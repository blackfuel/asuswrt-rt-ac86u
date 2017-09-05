/*
 * 802.11k definitions for
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
 * $Id: wlc_rrm.h 635565 2016-05-04 09:35:47Z $
 */


#ifndef _wlc_rrm_h_
#define _wlc_rrm_h_

extern wlc_rrm_info_t *wlc_rrm_attach(wlc_info_t *wlc);
extern void wlc_rrm_detach(wlc_rrm_info_t *rrm_info);
extern void wlc_frameaction_rrm(wlc_rrm_info_t *rrm_info, wlc_bsscfg_t *cfg, struct scb *scb,
	uint action_id, uint8 *body, int body_len, int8 rssi, ratespec_t rspec);
extern void wlc_rrm_pm_pending_complete(wlc_rrm_info_t *rrm_info);
extern void wlc_rrm_terminate(wlc_rrm_info_t *rrm_info);
extern bool wlc_rrm_inprog(wlc_info_t *wlc);
extern void wlc_rrm_stop(wlc_info_t *wlc);
extern bool wlc_rrm_wait_tx_suspend(wlc_info_t *wlc);
extern void wlc_rrm_start_timer(wlc_info_t *wlc);
extern bool wlc_rrm_enabled(wlc_rrm_info_t *rrm_info, wlc_bsscfg_t *cfg);
#ifdef WLSCANCACHE
extern void wlc_rrm_update_cap(wlc_rrm_info_t *rrm_info, wlc_bsscfg_t *bsscfg);
#endif /* WLSCANCACHE */
extern bool wlc_rrm_in_progress(wlc_info_t *wlc);
extern void wlc_rrm_upd_data_activity_ts(wlc_rrm_info_t *ri);
extern void wlc_rrm_stat_qos_counter(struct scb *scb, int tid, uint cnt_offset);
extern void wlc_rrm_stat_bw_counter(wlc_info_t *wlc, struct scb *scb, bool tx);
extern void wlc_rrm_stat_chanwidthsw_counter(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_rrm_tscm_upd(struct scb *scb, int tid, uint cnt_offset, uint cnt_val);
extern void wlc_rrm_delay_upd(struct scb *scb, uint8 tid, uint32 delay);
extern void rrm_add_pilot_timer(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

#endif	/* _wlc_rrm_h_ */
