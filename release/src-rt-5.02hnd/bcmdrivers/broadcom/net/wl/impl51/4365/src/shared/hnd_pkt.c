/*
 * HND generic packet operation primitives
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: $
 */

#include <typedefs.h>
#include <osl.h>
#include <hnd_lbuf.h>
#include <hnd_pktpool.h>
#include <hnd_pkt.h>

#ifdef BCMPKTPOOL
static uint32 pktget_failed_but_alloced_by_pktpool = 0;
static uint32 pktget_failed_not_alloced_by_pktpool = 0;

void *
hnd_pkt_frag_get(osl_t *osh, uint len, enum lbuf_type type)
{
	void *pkt = hnd_pkt_alloc(osh, len, type);

	if (pkt == NULL) {
		switch (type) {
		case lbuf_basic:
			if (POOL_ENAB(SHARED_POOL) && (len <= pktpool_plen(SHARED_POOL)))
				pkt = pktpool_get(SHARED_POOL);
			break;
		case lbuf_frag:
#ifdef BCMFRAGPOOL
			if (POOL_ENAB(SHARED_FRAG_POOL) &&
			    (len <= pktpool_plen(SHARED_FRAG_POOL)))
				pkt = pktpool_get(SHARED_FRAG_POOL);
#endif
			break;
		case lbuf_rxfrag:
#ifdef BCMRXFRAGPOOL
			if (POOL_ENAB(SHARED_RXFRAG_POOL) &&
			    (len <= pktpool_len(SHARED_RXFRAG_POOL)))
				pkt = pktpool_get(SHARED_RXFRAG_POOL);
#endif
			break;
		}

		if (pkt)
			pktget_failed_but_alloced_by_pktpool++;
		else
			pktget_failed_not_alloced_by_pktpool++;
	}

	return pkt;
}

void *
hnd_pkt_get(osl_t *osh, uint len)
{
	void *pkt = hnd_pkt_alloc(osh, len, lbuf_basic);

	if (pkt == NULL) {
		if (POOL_ENAB(SHARED_POOL) && (len <= pktpool_plen(SHARED_POOL))) {
			pkt = pktpool_get(SHARED_POOL);
			if (pkt)
				pktget_failed_but_alloced_by_pktpool++;
			else
				pktget_failed_not_alloced_by_pktpool++;
		}
	}

	return pkt;
}
#endif /* BCMPKTPOOL */

void *
hnd_pkt_alloc(osl_t *osh, uint len, enum lbuf_type type)
{
	void *pkt;

#if defined(MEM_LOWATLIMIT)
	if ((OSL_MEM_AVAIL() < MEM_LOWATLIMIT) && len >= PKTBUFSZ) {
		return (void *)NULL;
	}
#endif
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	pkt = (void *)lb_alloc(len, type, __FILE__, __LINE__);
#else
	pkt = (void *)lb_alloc(len, type);
#endif /* BCMDBG_MEM || BCMDBG_MEMFAIL */

	if (pkt)
		osh->cmn->pktalloced++;
	return pkt;
}

void
hnd_pkt_free(osl_t *osh, void* p, bool send)
{
	struct lbuf *nlbuf;

	if (send && osh->tx_fn) /* set using PKTFREESETCB() */
		osh->tx_fn(osh->tx_ctx, p, 0);

	for (nlbuf = (struct lbuf *)p; nlbuf; nlbuf = PKTNEXT(osh, nlbuf)) {
		if (!lb_pool(nlbuf)) {
			ASSERT(osh->cmn->pktalloced > 0);
			osh->cmn->pktalloced--;
		}
	}

	lb_free((struct lbuf *)p);
}

void *
hnd_pkt_clone(osl_t *osh, void *p, int offset, int len)
{
	void *pkt;

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	pkt = (void *)lb_clone(p, offset, len, __FILE__, __LINE__);
#else
	pkt = (void *)lb_clone(p, offset, len);
#endif
	if (pkt)
		osh->cmn->pktalloced++;

	return pkt;
}

void *
hnd_pkt_dup(osl_t *osh, void *p)
{
	void *pkt;

	if ((pkt = (void *)lb_dup((struct lbuf *)p)))
		osh->cmn->pktalloced++;

	return pkt;
}

void *
hnd_pkt_frmnative(osl_t *osh, struct lbuf *lbuf)
{
	struct lbuf *nlbuf;

	for (nlbuf = lbuf; nlbuf; nlbuf = PKTNEXT(osh, nlbuf)) {
		if (!lb_pool(nlbuf))
			osh->cmn->pktalloced++;
	}

	return ((void *)lbuf);
}

struct lbuf *
hnd_pkt_tonative(osl_t *osh, void *p)
{
	struct lbuf *nlbuf;

	for (nlbuf = (struct lbuf *)p; nlbuf; nlbuf = PKTNEXT(osh, nlbuf)) {
		if (!lb_pool(nlbuf)) {
			ASSERT(osh->cmn->pktalloced > 0);
			osh->cmn->pktalloced--;
		}
	}

	return ((struct lbuf *)p);
}

void *
hnd_pkt_shrink(osl_t *osh, void *p)
{
	void *pkt;

	pkt = (void *)lb_shrink((struct lbuf *)p);
	if (pkt != p)
		osh->cmn->pktalloced++;

	return pkt;
}

/* Util function for lfrags and legacy lbufs to return individual fragment pointers and length */
/* Takes in pkt pointer and frag idx as inputs */
/* fragidx will be updated by this function itself. */
uint8 *
hnd_pkt_params(osl_t * osh, void **pkt, uint32 *len, uint32 *fragix, uint16 *fragtot, uint8* fbuf,
	uint32 *lo_data, uint32 *hi_data)
{
	uint8 * data;
	struct lbuf * lbuf = *(struct lbuf **)pkt;

	*fbuf = 0;
	*hi_data = 0;

	if (lbuf != NULL) {
		if (lb_is_frag(lbuf)) {	/* Lbuf with fraglist walk */
			uint32 ix = *fragix;

			*fragtot = PKTFRAGTOTNUM(osh, lbuf);

			if (ix != 0) {
				*lo_data = PKTFRAGDATA_LO(osh, lbuf, ix);
				*hi_data = PKTFRAGDATA_HI(osh, lbuf, ix);
				data = (uint8 *)lo_data;
				*fbuf = 1;
				*len = PKTFRAGLEN(osh, lbuf, ix);
				if (ix == PKTFRAGTOTNUM(osh, lbuf)) {	/* last frag */
					*fragix = 1U;		/* skip inlined data in next */
					*pkt = PKTNEXT(osh, lbuf);	/* next lbuf */
				} else {
					*fragix = ix + 1;
				}
			} else { /* ix == 0 */
				data = PKTDATA(osh, lbuf);
				*len = PKTLEN(osh, lbuf);
				*fragix = 1U;
			}
		} else {
			*fragtot = 0;
			data = PKTDATA(osh, lbuf);
			*len = PKTLEN(osh, lbuf);
			*pkt = PKTNEXT(osh, lbuf);
		}
		return data;
	}
	return NULL;
}

/* Takes in a lbuf/lfrag and no of bytes to be trimmed from tail */
/* trim bytes  could be spread out in below 3 formats */
/*	1. entirely in dongle
	2. entirely in host
	3. split between host-dongle
*/
void
hnd_pkt_lfrag_trim_tailbytes(osl_t * osh, void* p, uint16 trim_len, uint8 type)
{
	uint16 tcmseg_len = PKTLEN(osh, p);	/* TCM segment length */
	uint16 hostseg_len = PKTFRAGUSEDLEN(osh, p);	/* HOST segment length */

	/* if header conv is on, there is no fcs at the end */
	if ((HDR_CONV() && PKTISRXFRAG(osh, p) && (type == TAIL_BYTES_TYPE_FCS)) ||
		(trim_len == 0))
		return;

	if (PKTFRAGUSEDLEN(osh, p) >= trim_len) {
		/* TRIM bytes entirely in host */
		ASSERT(PKTISRXFRAG(osh, p));

		PKTSETFRAGUSEDLEN(osh, p, (hostseg_len - trim_len));
	} else {
		/* trim bytes either in dongle or split between dongle-host */
		PKTSETLEN(osh, p, (tcmseg_len - (trim_len - hostseg_len)));

		/* No more contents in host; reset length to zero */
		if (PKTFRAGUSEDLEN(osh, p))
			PKTSETFRAGUSEDLEN(osh, p, 0);
	}
}
