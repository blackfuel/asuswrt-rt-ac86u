/*
 * MCHAN related header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_mchan.h 630085 2016-04-07 09:25:06Z $
 */


#ifndef _wlc_mchan_h_
#define _wlc_mchan_h_

#ifdef WLMCHAN

extern bool wlc_mchan_stago_is_disabled(mchan_info_t *mchan);
extern mchan_info_t *wlc_mchan_attach(wlc_info_t *wlc);
extern void wlc_mchan_detach(mchan_info_t *mchan);
extern uint16 wlc_mchan_get_pretbtt_time(mchan_info_t *mchan);
extern void wlc_mchan_recv_process_beacon(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, uint8 *body, int bcn_len);
extern int wlc_mchan_create_bss_chan_context(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	chanspec_t chanspec);
extern int wlc_mchan_delete_bss_chan_context(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern int wlc_mchan_update_bss_chan_context(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	chanspec_t chanspec, bool create);
extern void wlc_mchan_abs_proc(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint32 tsf_l);
extern void wlc_mchan_psc_proc(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint32 tsf_l);
extern void wlc_mchan_pm_pending_complete(mchan_info_t *mchan);
extern void wlc_mchan_client_noa_clear(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
#if defined(PROP_TXSTATUS)
extern int wlc_wlfc_mchan_interface_state_update(wlc_info_t *wlc,
	wlc_bsscfg_t *bsscfg,
	uint8 open_close, bool force_open);
#endif /* PROP_TXSTATUS */

extern 	wlc_bsscfg_t *wlc_mchan_get_cfg_frm_q(wlc_info_t *wlc, wlc_txq_info_t *qi);
extern 	wlc_bsscfg_t *wlc_mchan_get_other_cfg_frm_q(wlc_info_t *wlc, wlc_txq_info_t *qi);

void wlc_mchan_set_priq(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
void wlc_mchan_set_pmpending(mchan_info_t *mchan, wlc_bsscfg_t *cfg, bool pmpending);
void wlc_mchan_update_pmpending(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
bool _wlc_mchan_ovlp_chan(mchan_info_t *mchan, wlc_bsscfg_t *cfg, chanspec_t chspec,
	uint chbw);
bool wlc_mchan_ovlp_chan(mchan_info_t *mchan, wlc_bsscfg_t *cfg1, wlc_bsscfg_t *cfg2,
	uint chbw);
bool _wlc_mchan_same_chan(mchan_info_t *mchan, wlc_bsscfg_t *cfg, chanspec_t chspec);
bool wlc_mchan_shared_chanctx(mchan_info_t *mchan, wlc_bsscfg_t *cfg1, wlc_bsscfg_t *cfg2);
chanspec_t wlc_mchan_current_chanspec(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
bool wlc_mchan_has_chanctx(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
void wlc_mchan_config_go_chanspec(mchan_info_t *mchan, wlc_bsscfg_t *cfg, chanspec_t chspec);
chanspec_t wlc_mchan_configd_go_chanspec(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
bool wlc_mchan_ap_tbtt_setup(wlc_info_t *wlc, wlc_bsscfg_t *ap_cfg);
#ifdef WLTDLS
extern void wlc_mchan_stop_tdls_timer(mchan_info_t *mchan);
extern void wlc_mchan_start_tdls_timer(mchan_info_t *mchan, wlc_bsscfg_t *parent,
	struct scb *scb, bool force);
#endif
#else	/* stubs */
#define wlc_mchan_attach(a) (mchan_info_t *)0x0dadbeef
#define	wlc_mchan_detach(a) do {} while (0)
#endif /* WLMCHAN */

#endif /* _wlc_mchan_h_ */
