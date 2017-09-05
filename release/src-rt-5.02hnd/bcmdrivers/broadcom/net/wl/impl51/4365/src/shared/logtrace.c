/*
 * Trace messages sent over HBUS
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: logtrace.c 287537 2011-10-03 23:43:46Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <logtrace.h>
#include <event_log.h>
#include <bcmsdpcm.h>
#include <rte_timer.h>

/* Retry timeout value to handle the retry of lost message */
#define RETRY_TIMEOUT_VALUE	500

/* Send timeout value to trigger immmediately the sending by leaving the context of caller */
#define SEND_TIMEOUT_VALUE	0

/* Definition of trace buffer for sending trace over host bus */
typedef struct logtrace {
	void	*hdl1;			/* hdl1 : handler used to send trace event */
	void	*hdl2;			/* hdl2 : handler used to send trace event */


	uint32	seqnum;			/* Sequence number of event sent */
	hnd_timer_t *timer;		/* Timer used to trigger the sending  of trace buffer and
					 * used to handle the retry of lost event
					 */
	bool	pending;		/* Msg sent but not ackd */
	bool	timer_active;		/* Timer is active value */
	msgtrace_func_send_t func_send; /* Function pointer to send trace event */
	bool 	event_trace_enabled;   	/* EVENT_TRACE enabled/disabled flag */
	uint8	*last_trace;		/* Pointer to last trace sent */
} logtrace_t;


static logtrace_t *logtrace = NULL;

void
logtrace_stop(void)
{
	if (logtrace->event_trace_enabled) {
		if (logtrace->timer_active) {
			hnd_timer_stop(logtrace->timer);
			logtrace->timer_active = FALSE;
		}
		logtrace->event_trace_enabled = FALSE;
	}
}

void
logtrace_start(void)
{
	if (!logtrace->event_trace_enabled) {
		logtrace->event_trace_enabled = TRUE;
	}
}

int logtrace_sent_call = 0;
int sent_call_add = 0;
int sent_call_cancel = 0;
/* Called when the trace has been sent over the HBUS. */
int
logtrace_sent(void)
{
	logtrace_sent_call++;
	if (logtrace->timer_active) {
		sent_call_cancel++;
		hnd_timer_stop(logtrace->timer);
		logtrace->timer_active = FALSE;
	}

	/* Clear indicators */
	logtrace->pending = FALSE;

	if (!logtrace->event_trace_enabled) {
		return 0;
	}

	/* Trigger again to see if thre is more to send */
	logtrace_trigger();

	return 1;
}

void
logtrace_trigger(void)
{
	if (logtrace->timer_active) {
		return;				/* Already pending */
	}

	if (logtrace->event_trace_enabled) {
		/* Trigger immediately the sending by setting the timer to 0 */
		hnd_timer_start(logtrace->timer, SEND_TIMEOUT_VALUE, FALSE);
		logtrace->timer_active = TRUE;
	}
}

static void
logtrace_timeout(hnd_timer_t *t)
{
	int set_num;
	msgtrace_hdr_t hdr;

	logtrace->timer_active = FALSE;		/* Just fired */

	if (!logtrace->event_trace_enabled) {
		return;
	}

	if (logtrace->pending == FALSE) {
		/* Look for a set with something to send.  We do not
		 * round-robin under the assumption that the sets are ordered
		 * from most active or most important to last.
		 */
		for (set_num = 0; set_num < NUM_EVENT_LOG_SETS; set_num++) {
			logtrace->last_trace = event_log_next_logtrace(set_num);
			if (logtrace->last_trace != NULL) {
				/* First send of the event. */
				logtrace->seqnum++;
				break;			/* Found one */
			}
		}
	}

	logtrace->pending = FALSE;		/* Assume not pending */

	if (logtrace->last_trace != NULL) {
		/* Fill the trace header */
		uint16 len       = *((uint16 *) logtrace->last_trace);
		hdr.version	     = MSGTRACE_VERSION;
		hdr.trace_type	     = MSGTRACE_HDR_TYPE_LOG;
		hdr.len              = hton16(len);
		hdr.seqnum           = hton32(logtrace->seqnum);

		logtrace->pending = TRUE;
		logtrace->func_send(logtrace->hdl1, logtrace->hdl2,
		                   (uint8*)&hdr, sizeof(msgtrace_hdr_t),
		                   logtrace->last_trace, len);

		hnd_timer_start(logtrace->timer, RETRY_TIMEOUT_VALUE, FALSE);
		logtrace->timer_active = TRUE;
	}
}

void
BCMATTACHFN(logtrace_init)(void *hdl1, void *hdl2, msgtrace_func_send_t func_send)
{
	ASSERT(func_send != NULL);

	logtrace->hdl1 = hdl1;
	logtrace->hdl2 = hdl2;
	logtrace->func_send = func_send;

	logtrace->pending = FALSE;
	logtrace->timer_active = FALSE;
	logtrace->last_trace = NULL;

	logtrace->timer  = hnd_timer_create(NULL, NULL, logtrace_timeout,  NULL);
}
