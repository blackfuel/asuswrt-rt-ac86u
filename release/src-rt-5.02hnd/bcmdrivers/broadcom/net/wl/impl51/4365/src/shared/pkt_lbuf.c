/*
 * Network interface packet buffer routines. These are used internally by the
 * driver to allocate/de-allocate packet buffers, and manipulate the buffer
 * contents and attributes.
 *
 * This implementation is specific to LBUF (linked buffer) packet buffers.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: pkt_lbuf.c 467150 2014-04-02 17:30:43Z $
 */

/* ---- Include Files ---------------------------------------------------- */

#include "typedefs.h"
#include "bcmdefs.h"
#include "osl.h"
#include "pkt_lbuf.h"
#include "lbuf.h"
#include <stdlib.h>


/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define PKT_LBUF_LOG(a)	printf a


/* ---- Private Variables ------------------------------------------------ */

static lbuf_info_t *g_lbuf_info;


/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

/* ----------------------------------------------------------------------- */
pkt_handle_t*
pkt_init(osl_t *osh)
{
	lbuf_info_t	*info;

	if (!(info = (lbuf_info_t *)MALLOC(osh, sizeof(lbuf_info_t)))) {
		PKT_LBUF_LOG(("malloc failed for lbuf_info_t\n"));
		ASSERT(0);
	}
	memset(info, 0, sizeof(lbuf_info_t));
	info->osh = osh;

	if (!lbuf_alloc_list(info, &(info->txfree), NTXBUF)) {
		PKT_LBUF_LOG(("lbuf_alloc_list for TX fail \n"));
		ASSERT(0);
	}

	if (!lbuf_alloc_list(info, &(info->rxfree), NRXBUF)) {
		PKT_LBUF_LOG(("lbuf_alloc_list for RX fail \n"));
		ASSERT(0);
	}

	g_lbuf_info = info;

	return ((pkt_handle_t*)info);
}


/* ----------------------------------------------------------------------- */
void
pkt_deinit(osl_t *osh, pkt_handle_t *pkt_info)
{
	lbuf_info_t	*info = (lbuf_info_t*)pkt_info;

	lbuf_free_list(info, &(info->rxfree));
	lbuf_free_list(info, &(info->txfree));

	MFREE(osh, info, sizeof(lbuf_info_t));
}


/* ----------------------------------------------------------------------- */
/* Converts a native (network interface) packet to driver packet.
 * Allocates a new lbuf and copies the contents
 */
void *
pkt_frmnative(osl_t *osh, void *native_pkt, int len)
{
	struct lbuf* lb;
	lbuf_info_t *info = g_lbuf_info;
	struct lbfree *list = &info->txfree;

	ASSERT(osh);

	/* Alloc driver packet (lbuf) */
	if ((lb = lbuf_get(list)) == NULL)
		return (NULL);

	/* Adjust for the head room requested */
	ASSERT(list->size > list->headroom);
	lb->data += list->headroom;
	lb->tail += list->headroom;

	/* Copy data from native to driver packet. */
	pkt_get_native_pkt_data(osh, native_pkt, PKTDATA(osh, lb), PKTTAILROOM(osh, lb));
	pktsetlen(osh, lb, len);

	/* Save pointer to native packet. This will be freed later by pktfree(). */
	lb->native_pkt = native_pkt;

	/* Return driver packet. */
	return ((void *)lb);
}


/* ----------------------------------------------------------------------- */
/* Converts a driver packet to a native (network interface) packet.
 */
void* pkt_tonative(osl_t *osh, void *drv_pkt)
{
	void *native_pkt;
	struct lbuf* lb = (struct lbuf*) drv_pkt;

	ASSERT(osh);

	/* Allocate network interface (native) packet. */
	native_pkt = pkt_alloc_native(osh, PKTLEN(osh, drv_pkt));

	/* Copy data from driver to network interface packet */
	pkt_set_native_pkt_data(osh, native_pkt, PKTDATA(osh, lb), PKTLEN(osh, lb));

	return (native_pkt);
}


/* ----------------------------------------------------------------------- */
void*
pktget(osl_t *osh, uint len, bool send)
{
	struct lbuf	*lb;
	lbuf_info_t *info = g_lbuf_info;

	ASSERT(osh);

	if (len > LBUFSZ)
		return (NULL);

	if (send)
		lb = lbuf_get(&info->txfree);
	else
		lb = lbuf_get(&info->rxfree);

	if (lb)
		lb->len = len;

	return ((void*) lb);
}


/* ----------------------------------------------------------------------- */
void
pktfree(osl_t *osh, struct lbuf	*lb, bool send)
{
	struct lbuf *next;
	void *native_pkt;

	native_pkt = lb->native_pkt;

	ASSERT(osh);
	ASSERT(lb);

	while (lb) {
		next = lb->next;
		lb->next = NULL;

		lbuf_put(lb->list, lb);

		lb = next;
	}

	/* Free network interface (native) packet. */
	if (native_pkt != NULL)
		pkt_free_native(osh, native_pkt);
}


/* ----------------------------------------------------------------------- */
void pkt_set_headroom(osl_t *osh, bool tx, unsigned int headroom)
{
	struct lbfree *list;

	if (tx)
		list = &g_lbuf_info->txfree;
	else
		list = &g_lbuf_info->rxfree;

	list->headroom = headroom;
}
