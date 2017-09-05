/*
 * Power Management Mode PM_FAST (PM2) interface
 *
 *   Broadcom Proprietary and Confidential. Copyright (C) 2016,
 *   All Rights Reserved.
 *   
 *   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 *   the contents of this file may not be disclosed to third parties, copied
 *   or duplicated in any form, in whole or in part, without the prior
 *   written permission of Broadcom.
 *
 *   $Id: wlc_pm.h 467328 2014-04-03 01:23:40Z $
 */

/** Twiki: [WlDriverPowerSave] */

#ifndef _wlc_pm_h_
#define _wlc_pm_h_

#ifdef STA

/* PM2 Fast Return to Sleep */
extern void wlc_pm2_enter_ps(wlc_bsscfg_t *cfg);
extern void wlc_pm2_sleep_ret_timeout_cb(void *arg);
extern void wlc_pm2_sleep_ret_timeout(wlc_bsscfg_t *cfg);
extern void wlc_pm2_ret_upd_last_wake_time(wlc_bsscfg_t *cfg, uint32* tsf_l);
extern void wlc_pm2_sleep_ret_timer_start(wlc_bsscfg_t *cfg);
extern void wlc_pm2_sleep_ret_timer_stop(wlc_bsscfg_t *cfg);
extern void wlc_pm2_sleep_ret_timeout_cb(void *arg);

/* Dynamic PM2 Fast Return To Sleep */
extern void wlc_dfrts_reset_counters(wlc_bsscfg_t *bsscfg);
extern void wlc_update_sleep_ret(wlc_bsscfg_t *bsscfg, bool inc_rx, bool inc_tx,
	uint rxbytes, uint txbytes);

/* PM2 Receive Throttle Duty Cycle */
#if defined(WL_PM2_RCV_DUR_LIMIT)
extern void wlc_pm2_rcv_timeout_cb(void *arg);
extern void wlc_pm2_rcv_timeout(wlc_bsscfg_t *cfg);
extern void wlc_pm2_rcv_timer_start(wlc_bsscfg_t *cfg);
extern void wlc_pm2_rcv_timer_stop(wlc_bsscfg_t *cfg);
#else
#define wlc_pm2_rcv_timer_stop(cfg)
#define wlc_pm2_rcv_timer_start(cfg)
#endif /* WL_PM2_RCV_DUR_LIMIT */

#ifdef WL_EXCESS_PMWAKE
extern uint32 wlc_get_roam_ms(wlc_info_t *wlc);
extern uint32 wlc_get_pfn_ms(wlc_info_t *wlc);
extern void wlc_generate_pm_alert_event(wlc_info_t *wlc, uint32 reason, void *data, uint32 datalen);
extern void wlc_check_roam_alert_thresh(wlc_info_t *wlc);
extern void wlc_check_excess_pm_awake(wlc_info_t *wlc);
extern void wlc_epm_roam_time_upd(wlc_info_t *wlc, uint32 connect_dur);
extern void wlc_reset_epm_ca(wlc_info_t *wlc);
extern void wlc_reset_epm_dur(wlc_info_t *wlc);
#endif /* WL_EXCESS_PMWAKE */
#endif	/* STA */

#endif	/* _wlc_pm_h_ */
