/*
 * HostFetch module interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_fetch.c 474540 2014-05-01 18:40:15Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <rte_fetch.h>

#ifdef BCMDBG
#define FETCH_MSG(x) printf x
#else
#define FETCH_MSG(x)
#endif

struct fetch_rqst_q_t {
	fetch_rqst_t *head;
	fetch_rqst_t *tail;
};

/* HostFetch module related info */
struct fetch_rqst_q_t *fetch_rqst_q = NULL;
struct fetch_module_info *fetch_info = NULL;

static void hnd_enque_fetch_rqst(struct fetch_rqst *fr, bool enque_to_head);
static int hnd_dispatch_fetch_rqst(struct fetch_rqst *fr);
static struct fetch_rqst *hnd_deque_fetch_rqst(void);
static struct fetch_rqst *hnd_remove_fetch_rqst_from_queue(struct fetch_rqst *fr);

void
BCMATTACHFN(hnd_fetch_module_init)(osl_t *osh)
{
	fetch_info = MALLOCZ(osh, sizeof(struct fetch_module_info));
	if (fetch_info == NULL) {
		FETCH_MSG(("%s: Unable to init fetch module info!\n", __FUNCTION__));
		return;
	}

	fetch_rqst_q = MALLOCZ(osh, sizeof(struct fetch_rqst_q_t));
	if (fetch_rqst_q == NULL) {
		FETCH_MSG(("%s : Unable to initialize fetch_rqst_q!\n", __FUNCTION__));
		MFREE(osh, fetch_info, sizeof(struct fetch_module_info));
		return;
	}

	fetch_info->pool = SHARED_POOL;
}

void
hnd_fetch_bus_dispatch_cb_register(bus_dispatch_cb_t cb, void *arg)
{
	if (fetch_info == NULL) {
		FETCH_MSG(("fetch info not inited! Cannot register callback!\n"));
		ASSERT(0);
		return;
	}
	fetch_info->cb = cb;
	fetch_info->arg = arg;
}

void
hnd_fetch_rqst(struct fetch_rqst *fr)
{
	if (hnd_fetch_rqstq_empty()) {
		if (hnd_dispatch_fetch_rqst(fr) == BCME_OK)
			return;
	}
	hnd_enque_fetch_rqst(fr, FALSE);
}

static void
hnd_enque_fetch_rqst(struct fetch_rqst *fr, bool enque_to_head)
{
	struct fetch_rqst_q_t *frq = fetch_rqst_q;

	fr->next = NULL;

	if (frq->head == NULL)
		frq->head = frq->tail = fr;
	else {
		if (!enque_to_head) {
			frq->tail->next = fr;
			frq->tail = fr;
		} else {
			fr->next = frq->head;
			frq->head = fr;
		}
	}
}

static int
hnd_dispatch_fetch_rqst(struct fetch_rqst *fr)
{
	int ret;

	/* Fetch module assumes a dest buffer is always allocated */
	ASSERT(fr->dest != NULL);

	/* Bus layer DMA dispatch */
	FETCH_RQST_FLAG_SET(fr, FETCH_RQST_IN_BUS_LAYER);
	if (fetch_info && fetch_info->cb)
		ret = fetch_info->cb(fr, fetch_info->arg);
	else {
		FETCH_MSG(("fetch_info or fetch_info callback is NULL!\n"));
		ASSERT(0);
		return BCME_ERROR;
	}

	return ret;
}

static struct fetch_rqst *
hnd_deque_fetch_rqst(void)
{
	struct fetch_rqst_q_t *frq = fetch_rqst_q;
	struct fetch_rqst *fr;

	if (frq->head == NULL)
		fr = NULL;
	else if (frq->head == frq->tail) {
		fr = frq->head;
		frq->head = NULL;
		frq->tail = NULL;
	} else {
		fr = frq->head;
		frq->head = frq->head->next;
		fr->next = NULL;
	}
	return fr;
}

bool
hnd_fetch_rqstq_empty(void)
{
	struct fetch_rqst_q_t *frq = fetch_rqst_q;

	return (frq->head == NULL);
}

void
hnd_wake_fetch_rqstq(void)
{
	struct fetch_rqst *fr;
	int ret;

	/* While frq not empty */
	while (!hnd_fetch_rqstq_empty())
	{
		/* Deque an fr */
		fr = hnd_deque_fetch_rqst();
		ret = hnd_dispatch_fetch_rqst(fr);

		if (ret != BCME_OK) {
			/* Reque fr to head of Q */
			hnd_enque_fetch_rqst(fr, TRUE);
			break;
		}
	}
}

void
hnd_flush_fetch_rqstq(void)
{
	struct fetch_rqst_q_t *frq = fetch_rqst_q;
	struct fetch_rqst *fr;

	while (frq->head != NULL) {
		fr = frq->head;
		frq->head = frq->head->next;
		if (fr->cb)
			fr->cb(fr, TRUE);
		else {
			FETCH_MSG(("%s: No callback registered for fetch_rqst!\n", __FUNCTION__));
			ASSERT(0);
		}
	}
	frq->tail = NULL;
}

/* Generic callback invoked from the bus layer signifying
 * that a DMA descr was freed up / space available in the DMA queue
 */
void
hnd_dmadesc_avail_cb(void)
{
	if ((fetch_rqst_q)->head != NULL)
		hnd_wake_fetch_rqstq();
}

static struct fetch_rqst *
hnd_remove_fetch_rqst_from_queue(struct fetch_rqst *fr)
{
	struct fetch_rqst_q_t *frq = fetch_rqst_q;
	struct fetch_rqst *cur_fr, *prev_fr;

	if (hnd_fetch_rqstq_empty())
		goto not_found;

	/* Single element case */
	if (frq->head == frq->tail)
	{
		if (frq->head == fr) {
			frq->head = frq->tail = NULL;
			return fr;
		} else
			goto not_found;
	}

	/* Atleast 2 elements in queue */
	for (cur_fr = frq->head, prev_fr = NULL;
		cur_fr != NULL; prev_fr = cur_fr, cur_fr = cur_fr->next)
		{
			if (cur_fr == fr) {
				if (prev_fr == NULL)
					frq->head = cur_fr->next;
				else if (cur_fr == frq->tail) {
					prev_fr->next = NULL;
					frq->tail = prev_fr;
				} else
					prev_fr->next = cur_fr->next;
				return fr;
			}
		}

not_found:
	FETCH_MSG(("%s: fetch_rqst not found in fetch_rqst queue!\n", __FUNCTION__));
	return NULL;
}

int
hnd_cancel_fetch_rqst(struct fetch_rqst *fr)
{
	if (hnd_remove_fetch_rqst_from_queue(fr)) {
		if (fr->cb)
			fr->cb(fr, TRUE);
		else {
			FETCH_MSG(("%s: No callback registered for fetch_rqst!\n", __FUNCTION__));
			ASSERT(0);
		}
		return BCME_OK;
	} else if (FETCH_RQST_FLAG_GET(fr, FETCH_RQST_IN_BUS_LAYER)) {
		FETCH_RQST_FLAG_SET(fr, FETCH_RQST_CANCELLED);
		return BCME_NOTFOUND;
	}
	return BCME_ERROR;
}
