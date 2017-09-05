/*
 * Wrapper to scb rate selection algorithm of Broadcom
 * algorithm of Broadcom 802.11b DCF-only Networking Adapter.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * $Id: wlc_scb_ratesel.h 636568 2016-05-09 21:18:34Z $
 */


#ifndef	_WLC_SCB_RATESEL_H_
#define	_WLC_SCB_RATESEL_H_

#include <wlc_rate_sel.h>

#define WME_MAX_AC(wlc, scb) ((WME_PER_AC_MAXRATE_ENAB((wlc)->pub) && SCB_WME(scb)) ? \
				AC_COUNT : 1)

extern wlc_ratesel_info_t *wlc_scb_ratesel_attach(wlc_info_t *wlc);
extern void wlc_scb_ratesel_detach(wlc_ratesel_info_t *wrsi);

#if defined(BCMDBG_DUMP)
extern int wlc_scb_ratesel_scbdump(void *ctx, struct scb *scb, struct bcmstrbuf *b);
extern int wlc_scb_ratesel_get_fixrate(void *ctx, struct scb *scb, struct bcmstrbuf *b);
extern int wlc_scb_ratesel_set_fixrate(void *ctx, struct scb *scb, int ac, uint8 val);
#endif

/* get primary rate */
extern ratespec_t wlc_scb_ratesel_get_primary(wlc_info_t *wlc, struct scb *scb, void *p);

/* select transmit rate given per-scb state */
extern void wlc_scb_ratesel_gettxrate(wlc_ratesel_info_t *wrsi, struct scb *scb,
	uint16 *frameid, ratesel_txparams_t *cur_rate, uint16 *flags);

/* update per-scb state upon received tx status */
extern void wlc_scb_ratesel_upd_txstatus_normalack(wlc_ratesel_info_t *wrsi, struct scb *scb,
	tx_status_t *txs, uint16 sfbl, uint16 lfbl, uint8 mcs,
	bool sgi, uint8 antselid, bool fbr, uint8 ac);

#ifdef WL11N
/* change the throughput-based algo parameters upon ACI mitigation state change */
extern void wlc_scb_ratesel_aci_change(wlc_ratesel_info_t *wrsi, bool aci_state);

/* update per-scb state upon received tx status for ampdu */
extern void wlc_scb_ratesel_upd_txs_blockack(wlc_ratesel_info_t *wrsi, struct scb *scb,
	tx_status_t *txs, uint8 suc_mpdu, uint8 tot_mpdu,
	bool ba_lost, uint8 retry, uint8 fb_lim, bool tx_error,
	uint8 mcs, bool sgi, uint8 antselid, uint8 ac);

#ifdef WLAMPDU_MAC
extern void wlc_scb_ratesel_upd_txs_ampdu(wlc_ratesel_info_t *wrsi, struct scb *scb,
	ratesel_txs_t *rs_txs,
	tx_status_t *txs,
	uint16 tx_flags);
#endif /* WLAMPDU_MAC */

/* update rate_sel if a PPDU (ampdu or a reg pkt) is created with probe values */
extern void wlc_scb_ratesel_probe_ready(wlc_ratesel_info_t *wrsi, struct scb *scb,
	uint16 frameid, bool is_ampdu, uint8 ampdu_txretry, uint8 ac);

extern void wlc_scb_ratesel_upd_rxstats(wlc_ratesel_info_t *wrsi, ratespec_t rx_rspec,
	uint16 rxstatus2);

/* get the fallback rate of the specified mcs rate */
extern ratespec_t wlc_scb_ratesel_getmcsfbr(wlc_ratesel_info_t *wrsi, struct scb *scb,
	uint8 ac, uint8 mcs);
extern bool wlc_scb_ratesel_sync(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac,
	uint now, int rssi);
#endif /* WL11N */

extern bool wlc_scb_ratesel_minrate(wlc_ratesel_info_t *wrsi, struct scb *scb, tx_status_t *txs,
	uint8 ac);

extern void wlc_scb_ratesel_init(wlc_info_t *wlc, struct scb *scb);
extern void wlc_scb_ratesel_init_all(struct wlc_info *wlc);
extern void wlc_scb_ratesel_init_bss(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_scb_ratesel_rfbr(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac);
extern void wlc_scb_ratesel_rfbr_bss(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

#ifdef WL_LPC
/* External functions used by wlc_power_sel.c */
void wlc_scb_ratesel_get_info(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac,
	uint8 rate_stab_thresh, uint32 *new_rate_kbps, bool *rate_stable,
	rate_lcb_info_t *lcb_info);
void wlc_scb_ratesel_reset_vals(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac);
void wlc_scb_ratesel_clr_cache(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac);
#endif /* WL_LPC */

/* Update CLM enabled rates bitmap if condition has changed (e.g. OLPC kicked in) */
extern void wlc_scb_ratesel_ppr_upd(wlc_info_t *wlc);

#ifdef WLATF
/* get ratesel control block pointer */
extern rcb_t *wlc_scb_ratesel_getrcb(wlc_info_t *wlc, struct scb *scb, uint ac);
#endif /*  WLATF */

extern void wlc_scb_ratesel_get_ratecap(wlc_ratesel_info_t * wrsi, struct scb *scb, uint8 *sgi,
	uint16 mcs_bitmap[], uint8 ac);
#endif	/* _WLC_SCB_RATESEL_H_ */
