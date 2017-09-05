/*
 * HND HIGH driver RPC Tx module
 * Broadcom 802.11abgn Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_rpctx.h 570958 2015-07-14 01:53:00Z $
 */
#ifndef _wlc_rpctx_h_
#define _wlc_rpctx_h_

/* forward declaration */
#include <wlc_types.h>

struct wlc_pub;
struct rpc_info;

/* This controls how many packets are given to the dongle. This is required as
 * NTXD needs to be power of 2 but we may not have enough memory to absorb that
 * large number of frames
 */
#ifndef NRPCTXBUFPOST
#define NRPCTXBUFPOST NTXD
#endif

#if defined(WLC_HIGH_ONLY)

struct wlc_rpc_phy {
	struct rpc_info *rpc;
};

#define RPCTX_ENAB(pub)	(TRUE)

extern rpctx_info_t *wlc_rpctx_attach(struct wlc_pub *pub, wlc_info_t *wlc);
extern int wlc_rpctx_fifoinit(rpctx_info_t *rpctx, uint fifo, uint ntxd);
extern void wlc_rpctx_detach(rpctx_info_t *rpctx);
extern int wlc_rpctx_dump(rpctx_info_t *rpctx, struct bcmstrbuf *b);
extern void *wlc_rpctx_getnexttxp(rpctx_info_t *rpctx, uint fifo);
extern void wlc_rpctx_txreclaim(rpctx_info_t *rpctx);
extern uint wlc_rpctx_txavail(rpctx_info_t *rpctx, uint fifo);
extern int wlc_rpctx_pktenq(rpctx_info_t *rpctx, uint fifo, void *p);
extern int wlc_rpctx_tx(rpctx_info_t *rpctx, uint fifo, void *p, bool commit, uint16 frameid,
                        uint8 txpktpend);
extern uint wlc_rpctx_fifo_enabled(rpctx_info_t *rpctx, uint fifo);
extern void wlc_rpctx_map_pkts(rpctx_info_t *rpctx, map_pkts_cb_fn cb, void *ctx);
#else

#define RPCTX_ENAB(pub)                         (FALSE)
#define wlc_rpctx_attach(pub, wlc)              (NULL)
#define wlc_rpctx_fifoinit(rpctx, fifo, ntxd)   (0)
#define wlc_rpctx_detach(rpctx)                 ASSERT(0)
#define wlc_rpctx_txavail(rpctx, f)             (FALSE)
#define wlc_rpctx_dump(rpctx, b)                (0)
#define wlc_rpctx_getnexttxp(rpctx, f)          (NULL)
#define wlc_rpctx_txreclaim(rpctx)              ASSERT(0)
#define wlc_rpctx_pktenq(rpctx, fifo, p)        do { } while (0)
#define wlc_rpctx_tx(rpctx, f, p, c, fid, t)    (0)
#define wlc_rpctx_fifo_enabled(rpctx, f)        (FALSE)

#endif	/* WLC_HIGH_ONLY */

#endif /* _wlc_rpctx_h_ */
