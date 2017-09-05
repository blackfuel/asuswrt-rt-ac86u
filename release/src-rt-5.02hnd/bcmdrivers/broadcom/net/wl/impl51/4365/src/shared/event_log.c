/*
 * EVENT_LOG system
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: event_log.c 241182 2011-02-17 21:50:03Z $
 */


#include <typedefs.h>
#include <hnd_debug.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>

#ifdef LOGTRACE
#include <logtrace.h>
#endif

#include <event_log.h>
#include <rte_timer.h>

event_log_top_t *event_log_top;

#ifdef EVENT_LOG_COMPILE

uint8 *event_log_tag_sets;

extern uint32 logstrs_size;

/* Timestamp should be synced every 1-sec or more */
#define EL_SYNC_TIME 1000

int
BCMATTACHFN(event_log_init)(si_t *sih)
{
	int i;
	event_log_set_t **setpp;

	if ((event_log_top = MALLOC(si_osh(sih), sizeof(event_log_top_t))) ==
	    NULL) {
		return -1;
	}

	event_log_top->magic = EVENT_LOG_TOP_MAGIC;
	event_log_top->version = EVENT_LOG_VERSION;

	/* Init the sets */
	event_log_top->num_sets = NUM_EVENT_LOG_SETS;
	if ((event_log_top->sets = MALLOC(si_osh(sih),
	                                  sizeof(uint32) * NUM_EVENT_LOG_SETS)) == NULL) {
		goto err1;
	}

	setpp = event_log_top->sets;
	for (i = 0; i <= NUM_EVENT_LOG_SETS; i++) {
		*setpp++ = NULL;
	}

	event_log_top->logstrs_size = (uint32) &logstrs_size;

	/* Init the tag flags */
	if ((event_log_tag_sets = MALLOC(si_osh(sih),
	                                 sizeof(uint8) *
	                                 (EVENT_LOG_TAG_MAX + 1))) == NULL) {
		goto err2;
	}

	for (i = 0; i <= EVENT_LOG_TAG_MAX; i++) {
		*(event_log_tag_sets + i) = EVENT_LOG_TAG_FLAG_NONE;
	}

	/* Init the timestamp */
	event_log_time_sync(OSL_SYSUPTIME());

	/* Set the pointer in the debug area */
	 get_hnd_debug_info()->event_log_top = event_log_top;

	/* Put a dummy in the logstrs so that it isn't empty */
	EVENT_LOG(EVENT_LOG_TAG_MAX, " ");

	return 0;

err2:
	MFREE(si_osh(sih), event_log_top->sets,
	      sizeof(uint32) * NUM_EVENT_LOG_SETS);

err1:
	MFREE(si_osh(sih), event_log_top, sizeof(event_log_top_t));
	return -1;
}

int
event_log_set_init(si_t *sih, int set_num, int size)
{
	event_log_set_t *ts;
	event_log_set_t **setpp;

	if ((event_log_top == NULL) ||
	    (set_num > NUM_EVENT_LOG_SETS) ||
	    (event_log_top->sets == NULL)) {
		return -1;
	}

	/* See if the set has already been initialized */
	setpp = event_log_top->sets + set_num;
	if (*setpp != NULL) {
		return -1;
	}

	if ((ts = MALLOC(si_osh(sih), sizeof(event_log_set_t))) == NULL) {
		return -1;
	}

	/* Init to empty */
	ts->first_block = NULL;
	ts->last_block = NULL;
	ts->cur_block = NULL;
	ts->logtrace_block = NULL;
	ts->blockcount = 0;
	ts->timestamp = event_log_top->timestamp;
	ts->cyclecount = event_log_top->cyclecount;

	*setpp = ts;

	/* Expand the empty set */
	return (event_log_set_expand(sih, set_num, size));
}

int
event_log_set_expand(si_t *sih, int set_num, int size)
{
	/* Add a event_log block before the first one */
	event_log_set_t *ts;
	event_log_block_t *tb;

	if ((event_log_top == NULL) ||
	    (set_num >= NUM_EVENT_LOG_SETS) ||
	    (event_log_top->sets == NULL)) {
		return -1;
	}

	ts = *(event_log_top->sets + set_num);
	if (ts == NULL) {
		return -1;
	}

	size = (size + 3) & ~3;		/* Word align */

	/* Allocate multiple blocks if greater than max blocksize */
	while (size != 0) {
		int this_size = (size > EVENT_LOG_MAX_BLOCK_SIZE) ?
			EVENT_LOG_MAX_BLOCK_SIZE : size;

		/* Allocate and the clear the block + logs in one chunk */
		tb = MALLOC(si_osh(sih), sizeof(event_log_block_t) + this_size);
		if (tb == NULL) {
			return -1;
		}

		/* Init all of the fields in the block */
		memset(tb, 0, sizeof(event_log_block_t) + this_size);

		/* Set the end ptr saving one slot for the end count */
		tb->end_ptr = (&tb->event_logs) + (this_size >> 2) - 1;
		tb->pktlen = (uint32) tb->end_ptr - (uint32) &tb->pktlen;

		/* This becomes the last block */
		if (ts->first_block == NULL) {
			/* This is the first one */
			ts->first_block = tb;
			tb->next_block = tb;
			tb->prev_block = tb;
			ts->last_block = tb;
		} else {
			ts->first_block->prev_block = tb;
			ts->last_block->next_block = tb;
			tb->next_block = ts->first_block;
			tb->prev_block = ts->last_block;
			ts->last_block = tb;
		}

		ts->blockcount++;

		size -= this_size;
	}

	/* Reset the set logging to the first block */
	ts->cur_block = ts->first_block;
	ts->cur_ptr = &(ts->first_block->event_logs);

	/* Reset the counts for logtrace */
	ts->blockfill_count = 0;
	ts->logtrace_block = ts->first_block;
	ts->logtrace_block->count = ts->blockfill_count++;
	ts->logtrace_count = ts->logtrace_block->count;

	return 0;
}

int
event_log_set_shrink(si_t *sih, int set_num, int size)
{
	/* Delete the last block(s) - even if it is the only one */
	event_log_set_t *ts;
	event_log_block_t *lb;


	if ((event_log_top == NULL) ||
	    (set_num >= NUM_EVENT_LOG_SETS)) {
		return -1;
	}

	ts = *(event_log_top->sets + set_num);
	if (ts == NULL) {
		return -1;
	}

	while (size > 0) {
		lb = ts->last_block;
		if (lb == NULL) {           /* Check for all empty */
			return -1;
		}

		size -= (uint32) lb->end_ptr - (uint32) lb + 1;

		if (lb == ts->first_block) {  /* Deleting last block */
			ts->cur_block = NULL;
			ts->last_block = NULL;
			ts->first_block = NULL;
			ts->cur_ptr = NULL;

		} else {
			/* Set up the circular list of sets */
			ts->last_block = lb->prev_block;
			ts->first_block->prev_block = ts->last_block;
			ts->last_block->next_block = ts->first_block;

			/* Start event_log over */
			ts->cur_block = ts->first_block;
			ts->cur_ptr = &ts->first_block->event_logs;

			/* Release the block */
			MFREE(si_osh(sih), lb, (uint32) lb->end_ptr -
			      (uint32) lb + 1);
		}
		ts->blockcount--;
	}

	/* Reset the set logging to the first block */
	ts->cur_block = ts->first_block;
	ts->cur_ptr = &(ts->first_block->event_logs);

	/* Reset the counts for logtrace */
	ts->blockfill_count = 0;
	ts->logtrace_block = ts->first_block;
	ts->logtrace_block->count = ts->blockfill_count++;
	ts->logtrace_count = ts->logtrace_block->count;

	return 0;
}

int
event_log_tag_start(int tag, int set_num, int flags)
{
	if ((event_log_top == NULL) ||
	    (set_num >= NUM_EVENT_LOG_SETS) ||
	    (tag > EVENT_LOG_TAG_MAX)) {
		return -1;
	}

	*(event_log_tag_sets + tag) = flags | set_num;

	return 0;
}

int
event_log_tag_stop(int tag)
{
	if ((event_log_top == NULL) ||
	    (tag > EVENT_LOG_TAG_MAX)) {
		return -1;
	}

	*(event_log_tag_sets + tag) = EVENT_LOG_TAG_FLAG_NONE;

	return 0;
}

int
event_log_get(int set_num, int buflen, void *buf)
{
	int size;
	uint8 *pkt = event_log_next_logtrace(set_num);
	if (pkt == NULL) {
		*((uint32 *) buf) = 0;
		return -1;
	}

	/* Compute how much to move in */
	size = *((uint16 *) pkt);

	/* Copy as much as possible */
	bcopy(pkt, buf, (buflen < size) ? buflen : size);

	return 0;
}

uint8 *
event_log_next_logtrace(int set_num)
{
	event_log_set_t *ts;
	event_log_block_t *lb;

	/* Check if active, valid set */
	if ((event_log_top == NULL) ||
	    (set_num >= NUM_EVENT_LOG_SETS)) {
		return NULL;
	}

	/* See if this set has been initialized and has blocks */
	ts = *(event_log_top->sets + set_num);
	if ((ts == NULL) || (ts->logtrace_block == NULL)) {
		return NULL;
	}

	if (ts->blockcount < 2) {
		return NULL;	/* Single block is always current!! */
	}

	lb = ts->logtrace_block;
	if ((lb == ts->cur_block) &&
	    (ts->cur_block->count == ts->logtrace_count)) {
		/* Block still in use */
		return NULL;
	}

	if (ts->logtrace_count != lb->count) {
		/* Overran the block so things wrapped all the way
		 * around.  Start over with the first clean block
		 * which is the one after the current block.
		 */
		lb = ts->cur_block->next_block;
		ts->logtrace_block = lb;
		ts->logtrace_count = lb->count;
	}

	/* On to the next block */
	ts->logtrace_count++;
	ts->logtrace_block = lb->next_block;

	return (uint8 *) &(lb->pktlen);
}


/* Define the macros used to generate event log entries */

#ifdef LOGTRACE
#define LOGTRACE_TRIGGER logtrace_trigger();
#else
#define LOGTRACE_TRIGGER
#endif

#define _EVENT_LOG_END_CHECK(n)				\
	if ((ts->cur_ptr + (n)) >= cb->end_ptr) {	\
		do {					\
			/* Clear the rest */		\
			*ts->cur_ptr++ = 0;		\
		} while (ts->cur_ptr < cb->end_ptr);	\
		/* Mark the end for matching */		\
		*ts->cur_ptr = cb->count;		\
							\
		/* Advance to the next block */		\
		cb = cb->next_block;			\
		ts->cur_block = cb;			\
		ts->cur_ptr = &cb->event_logs;		\
		cb->count = ts->blockfill_count++;	\
		LOGTRACE_TRIGGER;			\
	}

#define _EVENT_LOG_STORE(el)						\
	*ts->cur_ptr++ = el

#define _EVENT_LOG_START(tag, fmt_num, num)				\
	event_log_set_t *ts = *(event_log_top->sets +			\
				(flag & EVENT_LOG_TAG_FLAG_MASK));	\
	event_log_block_t *cb;						\
									\
	if ((ts != NULL) && ((cb = ts->cur_block) != NULL)) {		\
		event_log_hdr_t th = {{tag, num + 1, fmt_num}};		\
		if (event_log_top->timestamp != ts->timestamp) {	\
			/* Must put a timestamp in the log */		\
			event_log_hdr_t tsh = {{EVENT_LOG_TAG_TS, 3, 0}}; \
			_EVENT_LOG_END_CHECK(4);			\
			*ts->cur_ptr++ = ts->timestamp;			\
			*ts->cur_ptr++ = ts->cyclecount;		\
			*ts->cur_ptr++ = get_arm_cyclecount();		\
			*ts->cur_ptr++ = tsh.t;				\
			ts->timestamp = event_log_top->timestamp;	\
			ts->cyclecount = event_log_top->cyclecount;	\
		}							\
	        _EVENT_LOG_END_CHECK(num + 2);				\
		/* Note missing close paren */

#define _EVENT_LOG_END							\
	        /* Note missing open paren */				\
	        *ts->cur_ptr++ = get_arm_cyclecount();			\
	        *ts->cur_ptr++ = th.t;					\
	}


/*
 * The first 5 event log variants have explicit variables.  This is
 * slightly more efficient than using va_args because it removes the
 * requirement that the parameters get saved in memory.  The tradeoff
 * is the size of the generated code.  Since only the first few
 * parameters get passed in registers anyways there isn't much point
 * beyond 4 parameters so we use a loop for 5 or more params
 */


void
event_log0(int tag, int fmt_num)
{
	uint8 flag = *(event_log_tag_sets + tag);
	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x\n", tag & EVENT_LOG_TAG_FLAG_MASK, fmt_num);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, 0);
		_EVENT_LOG_END;
	}
}

void
event_log1(int tag, int fmt_num, uint32 t1)
{
	uint8 flag = *(event_log_tag_sets + tag);
	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x %x\n", tag & EVENT_LOG_TAG_FLAG_MASK,
		       fmt_num, t1);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, 1);
		*ts->cur_ptr++ = t1;
		_EVENT_LOG_END;
	}
}

void
event_log2(int tag, int fmt_num, uint32 t1, uint32 t2)
{
	uint8 flag = *(event_log_tag_sets + tag);
	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x  %x  %x\n", tag & EVENT_LOG_TAG_FLAG_MASK,
		       fmt_num, t1, t2);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, 2);
		*ts->cur_ptr++ = t1;
		*ts->cur_ptr++ = t2;
		_EVENT_LOG_END;
	}
}

void
event_log3(int tag, int fmt_num, uint32 t1, uint32 t2, uint32 t3)
{
	uint8 flag = *(event_log_tag_sets + tag);
	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x %x %x %x\n", tag & EVENT_LOG_TAG_FLAG_MASK,
		       fmt_num, t1, t2, t3);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, 3);
		*ts->cur_ptr++ = t1;
		*ts->cur_ptr++ = t2;
		*ts->cur_ptr++ = t3;
		_EVENT_LOG_END;
	}
}

void
event_log4(int tag, int fmt_num, uint32 t1, uint32 t2, uint32 t3, uint32 t4)
{
	uint8 flag = *(event_log_tag_sets + tag);
	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x %x %x %x %x\n", tag & EVENT_LOG_TAG_FLAG_MASK,
		       fmt_num, t1, t2, t3, t4);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, 4);
		*ts->cur_ptr++ = t1;
		*ts->cur_ptr++ = t2;
		*ts->cur_ptr++ = t3;
		*ts->cur_ptr++ = t4;
		_EVENT_LOG_END;
	}
}

void
event_logn(int num_args, int tag, int fmt_num, ...)
{
	uint8 flag = *(event_log_tag_sets + tag);
	va_list ap;

	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x", tag & EVENT_LOG_TAG_FLAG_MASK, fmt_num);
		va_start(ap, fmt_num);
		while (num_args--) {
			printf(" %x", va_arg(ap, uint32));
		}
		printf("\n");
		va_end(ap);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, num_args);
		va_start(ap, fmt_num);
		while (num_args--) {
			*ts->cur_ptr++ = va_arg(ap, uint32);
		}
		_EVENT_LOG_END;
		va_end(ap);
	}
}

/* Sync the timestamp with the PMU timer */
void event_log_time_sync(uint32 ms)
{
	if ((ms - event_log_top->timestamp) >= EL_SYNC_TIME) {
		event_log_top->timestamp = ms;
		event_log_top->cyclecount = get_arm_cyclecount();
	}
}
#endif /* EVENT_LOG_COMPILE */
