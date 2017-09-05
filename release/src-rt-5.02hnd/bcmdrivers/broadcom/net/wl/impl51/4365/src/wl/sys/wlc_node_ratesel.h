/*
 * Net80211 rate selection algorithm wrapper of Broadcom
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
 * $Id: wlc_node_ratesel.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef	_WLC_NODE_RATESEL_H_
#define	_WLC_NODE_RATESEL_H_

#include <wlc_rate_sel.h>
struct ieee80211_node;
struct ieee80211_key;
struct ieee80211_txparam;
struct ieee80211_channel;
struct ieee80211com;
struct wmeParams;
struct mbuf;

extern wlc_ratesel_info_t *wlc_node_ratesel_attach(wlc_info_t *wlc);
extern void wlc_node_ratesel_detach(wlc_ratesel_info_t *wrsi);


/* select transmit rate given per-scb state */
extern void wlc_node_ratesel_gettxrate(wlc_ratesel_info_t *wrsi, void *node_ratesel,
	uint16 *frameid, ratesel_txparams_t *cur_rate, uint16 *flags);

/* update per-scb state upon received tx status */
extern void wlc_node_ratesel_upd_txstatus_normalack(wlc_ratesel_info_t *wrsi, void *node_ratesel,
	tx_status_t *txs, uint16 sfbl, uint16 lfbl,
	uint8 mcs, uint8 antselid, bool fbr);

#ifdef WL11N
/* change the throughput-based algo parameters upon ACI mitigation state change */
extern void wlc_node_ratesel_aci_change(wlc_ratesel_info_t *wrsi, bool aci_state);

/* update per-scb state upon received tx status for ampdu */
extern void wlc_node_ratesel_upd_txs_blockack(wlc_ratesel_info_t *wrsi, void *node_ratesel,
	tx_status_t *txs, uint8 suc_mpdu, uint8 tot_mpdu,
	bool ba_lost, uint8 retry, uint8 fb_lim, bool tx_error,
	uint8 mcs, uint8 antselid);

#ifdef WLAMPDU_MAC
extern void wlc_node_ratesel_upd_txs_ampdu(wlc_ratesel_info_t *wrsi, void *node_ratesel,
	uint16 frameid, uint8 mrt, uint8 mrt_succ, uint8 fbr, uint8 fbr_succ,
	bool tx_error, uint8 tx_mcs, uint8 antselid);
#endif

/* update rate_sel if a PPDU (ampdu or a reg pkt) is created with probe values */
extern void wlc_node_ratesel_probe_ready(wlc_ratesel_info_t *wrsi, void *node_ratesel,
	uint16 frameid, bool is_ampdu, uint8 ampdu_txretry);

extern void wlc_node_ratesel_upd_rxstats(wlc_ratesel_info_t *wrsi, ratespec_t rx_rspec,
	uint16 rxstatus2);

/* get the fallback rate of the specified mcs rate */
extern ratespec_t wlc_node_ratesel_getmcsfbr(wlc_ratesel_info_t *wrsi, void *node_ratesel,
	uint16 frameid, uint8 mcs);
#endif /* WL11N */

extern bool wlc_node_ratesel_minrate(wlc_ratesel_info_t *wrsi, void *node_ratesel,
	tx_status_t *txs);

extern void wlc_node_ratesel_init(wlc_info_t *wlc, void *node_ratesel);
extern void wlc_scb_ratesel_init_all(wlc_info_t *wlc);

extern void *wlc_ratesel_node_alloc(wlc_info_t *wlc);
extern void wlc_ratesel_node_free(wlc_info_t *wlc, void *node_ratesel);
extern void wlc_ratesel_init_node(wlc_info_t *wlc, struct ieee80211_node *ni,
	void *node_ratesel, wlc_rateset_t *rateset);

#endif	/* _WLC_NODE_RATESEL_H_ */
