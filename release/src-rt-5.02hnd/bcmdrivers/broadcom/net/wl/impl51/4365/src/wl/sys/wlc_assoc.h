/*
 * Association/Roam related routines
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_assoc.h 598298 2015-11-09 09:11:22Z $
 */

#ifndef __wlc_assoc_h__
#define __wlc_assoc_h__

#ifdef STA
extern int wlc_join(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *SSID, int len,
	wl_join_scan_params_t *scan_params,
	wl_join_assoc_params_t *assoc_params, int assoc_params_len);
extern void wlc_join_recreate(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);

extern void wlc_join_complete(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_bcn_prb *bcn, int bcn_len);
extern void wlc_join_recreate_complete(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_bcn_prb *bcn, int bcn_len);

extern int wlc_join_pref_parse(wlc_bsscfg_t *cfg, uint8 *pref, int len);
extern int wlc_join_pref_build(wlc_bsscfg_t *cfg, uint8 *pref, int len);
extern void wlc_join_pref_reset(wlc_bsscfg_t *cfg);
extern void wlc_join_pref_band_upd(wlc_bsscfg_t *cfg);

extern int wlc_reassoc(wlc_bsscfg_t *cfg, wl_reassoc_params_t *reassoc_params);

extern void wlc_roam_complete(wlc_bsscfg_t *cfg, uint status,
                              struct ether_addr *addr, uint bss_type);
extern int wlc_roam_scan(wlc_bsscfg_t *cfg, uint reason, chanspec_t *list, uint32 channum);
extern void wlc_roamscan_start(wlc_bsscfg_t *cfg, uint roam_reason);
extern void wlc_assoc_roam(wlc_bsscfg_t *cfg);
extern void wlc_txrate_roam(wlc_info_t *wlc, struct scb *scb, tx_status_t *txs, bool pkt_sent,
	bool pkt_max_retries, uint8 ac);
extern void wlc_build_roam_cache(wlc_bsscfg_t *cfg, wlc_bss_list_t *candidates);
extern void wlc_roam_motion_detect(wlc_bsscfg_t *cfg);
extern void wlc_roam_bcns_lost(wlc_bsscfg_t *cfg);
extern int wlc_roam_trigger_logical_dbm(wlc_info_t *wlc, wlcband_t *band, int val);
extern bool wlc_roam_scan_islazy(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool roam_scan_isactive);
extern bool wlc_lazy_roam_scan_suspend(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern bool wlc_lazy_roam_scan_sync_dtim(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_roam_prof_update(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool reset);
extern void wlc_roam_prof_update_default(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

extern int wlc_disassociate_client(wlc_bsscfg_t *cfg, bool send_disassociate);

extern int wlc_assoc_abort(wlc_bsscfg_t *cfg);
extern void wlc_assoc_timeout(void *cfg);
extern void wlc_assoc_change_state(wlc_bsscfg_t *cfg, uint newstate);
extern void wlc_authresp_client(wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, uint body_len, bool short_preamble);
extern void wlc_assocresp_client(wlc_bsscfg_t *cfg, struct scb *scb,
	struct dot11_management_header *hdr, uint8 *body, uint body_len);
extern void wlc_process_assocresp_decision(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	assoc_decision_t * dc);
extern void wlc_auth_tx_complete(wlc_info_t *wlc, uint txstatus, void *arg);
extern void wlc_auth_complete(wlc_bsscfg_t *cfg, uint status, struct ether_addr *addr,
	uint auth_status, uint auth_type);
extern void wlc_assocreq_complete(wlc_info_t *wlc, uint txstatus, void *arg);

extern void wlc_sta_assoc_upd(wlc_bsscfg_t *cfg, bool state);

extern void wlc_clear_hw_association(wlc_bsscfg_t *cfg, bool mute_mode);
#ifdef ROBUST_DISASSOC_TX
extern void wlc_disassoc_tx(wlc_bsscfg_t *cfg, bool send_disassociate);
#endif /* ROBUST_DISASSOC_TX */
extern void wlc_roam_timer_expiry(void *arg);
#if defined(WLTEST)
extern void wlc_assoc_auth_txstatus(wlc_info_t * wlc, wlc_bsscfg_t * cfg, uint16 fc);
#endif

#endif /* STA */

extern int wlc_mac_request_entry(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int req);

extern void wlc_roam_defaults(wlc_info_t *wlc, wlcband_t *band, int *roam_trigger, uint *rm_delta);

extern void wlc_disassoc_complete(wlc_bsscfg_t *cfg, uint status, struct ether_addr *addr,
	uint disassoc_reason, uint bss_type);
extern void wlc_deauth_complete(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint status,
	const struct ether_addr *addr, uint deauth_reason, uint bss_type);
typedef struct wlc_deauth_send_cbargs {
	struct ether_addr	ea;
	int8			_idx;
	void                    *pkt;
} wlc_deauth_send_cbargs_t;
extern void wlc_deauth_sendcomplete(wlc_info_t *wlc, uint txstatus, void *arg);
extern void wlc_disassoc_ind_complete(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint status,
	struct ether_addr *addr, uint disassoc_reason, uint bss_type,
	uint8 *body, int body_len);
extern void wlc_deauth_ind_complete(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint status,
	struct ether_addr *addr, uint deauth_reason, uint bss_type,
	uint8 *body, int body_len);

extern void wlc_assoc_bcn_mon_off(wlc_bsscfg_t *cfg, bool off, uint user);

extern void wlc_join_adopt_ibss_params(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern int wlc_join_start_ibss(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern int wlc_assoc_iswpaenab(wlc_bsscfg_t *cfg, bool wpa);

extern void wlc_join_start_prep(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_assoc_init(wlc_bsscfg_t *cfg, uint type);
extern bool wlc_assoc_check_aplost_ok(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_adopt_dtim_period(wlc_bsscfg_t *cfg, uint8 dtim_period);
extern uint32 wlc_bss_pref_score(wlc_bsscfg_t *cfg, wlc_bss_info_t *bi, bool band_rssi_boost,
	uint32 *prssi);
#endif /* __wlc_assoc_h__ */
