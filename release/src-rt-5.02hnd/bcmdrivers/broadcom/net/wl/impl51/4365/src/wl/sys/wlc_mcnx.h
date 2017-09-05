/*
 * Multiple connection capable hardware/ucode management header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_mcnx.h 476506 2014-05-09 00:52:35Z $
 */

#ifndef _wlc_mcnx_h_
#define _wlc_mcnx_h_

/* TODO: include header files containing referenced data types here... */

#ifdef WLC_HIGH

/* module entries */
extern wlc_mcnx_info_t *wlc_mcnx_attach(wlc_info_t *wlc);
extern void wlc_mcnx_detach(wlc_mcnx_info_t *mcnx);

/* capability/enable */
extern bool wlc_mcnx_cap(wlc_mcnx_info_t *mcnx);
extern int wlc_mcnx_enab(wlc_mcnx_info_t *mcnx, bool enable);

/* h/w management */
extern int wlc_mcnx_ra_set(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern void wlc_mcnx_ra_unset(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern int wlc_mcnx_bssid_set(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern void wlc_mcnx_bssid_unset(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern void wlc_mcnx_adopt_bcn(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_bcn_prb *bcn);
extern void wlc_mcnx_adopt_bss(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	wlc_bss_info_t *bi);
extern void wlc_mcnx_reset_bss(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern void wlc_mcnx_assoc_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool assoc);
extern void wlc_mcnx_bss_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool up);
extern void wlc_mcnx_hps_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, uint8 what, bool enable);
extern void wlc_mcnx_hps_set(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool enable);
extern void wlc_mcnx_hps_force(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern void wlc_mcnx_tbtt_set(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 tsf_h, uint32 tsf_l, uint32 tbtt_h, uint32 tbtt_l);
extern void wlc_mcnx_tbtt_inv(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern void wlc_mcnx_tbtt_adj_all(wlc_mcnx_info_t *mcnx, int32 off_h, int32 off_l);
extern void wlc_mcnx_tbtt_adj(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, int adj);
extern void wlc_mcnx_tbtt_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool set);
extern void wlc_mcnx_unaligned_tbtt_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 tbtt_tsfo);
extern void wlc_mcnx_tbtt_calc(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
        wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_bcn_prb *bcn,
        uint32 *tsf_h, uint32 *tsf_l, uint32 *bcn_h, uint32 *bcn_l);
extern void wlc_mcnx_tbtt_calc_bcn(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_bcn_prb *bcn,
	uint32 *tsf_h, uint32 *tsf_l, uint32 *bcn_h, uint32 *bcn_l);
extern void wlc_mcnx_dtim_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool set);
extern void wlc_mcnx_st_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool set);
extern void wlc_mcnx_read_tsf64(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 *tsf_h, uint32 *tsf_l);
extern void wlc_mcnx_tsf_adopt(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 ltsf_h, uint32 ltsf_l, uint32 rtsf_h, uint32 rtsf_l);

/* conversions */
extern uint32 wlc_mcnx_r2l_tsf32(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, uint32 rtsf);
extern uint32 wlc_mcnx_l2r_tsf32(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, uint32 ltsf);
extern void wlc_mcnx_r2l_tsf64(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 rtsf_h, uint32 rtsf_l, uint32 *ltsf_h, uint32 *ltsf_l);
extern void wlc_mcnx_l2r_tsf64(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 ltsf_h, uint32 ltsf_l, uint32 *rtsf_h, uint32 *rtsf_l);

/* last/next tbtt */
extern void wlc_mcnx_next_l_tbtt64(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 ltsf_h, uint32 ltsf_l, uint32 *ltbtt_h, uint32 *ltbtt_l);
extern void wlc_mcnx_last_l_tbtt64(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 ltsf_h, uint32 ltsf_l, uint32 *ltbtt_h, uint32 *ltbtt_l);

/* accessors */
extern int wlc_mcnx_BSS_idx(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern int wlc_mcnx_rcmta_bssid_idx(wlc_mcnx_info_t *mcnx, const wlc_bsscfg_t *cfg);
extern int wlc_mcnx_rcmta_ra_idx(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern bool wlc_mcnx_tbtt_valid(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern uint16 wlc_mcnx_hps_get(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern bool wlc_mcnx_hps_forced(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
extern void wlc_mcnx_ps_enab(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool enab);

/* wowl setup */
extern void wlc_mcnx_wowl_setup(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);

/* p2p SHM access */
extern uint16 wlc_mcnx_read_shm(wlc_mcnx_info_t *mcnx, uint offset);
extern void wlc_mcnx_write_shm(wlc_mcnx_info_t *mcnx, uint offset, uint16 value);

/* STA assoc update notification */
typedef struct {
	wlc_bsscfg_t *cfg;
	bool assoc;
} wlc_mcnx_assoc_upd_data_t;
typedef void (*wlc_mcnx_assoc_upd_fn_t)(void *ctx, wlc_mcnx_assoc_upd_data_t *notif_data);
extern int wlc_mcnx_assoc_upd_register(wlc_mcnx_info_t *mcnx,
	wlc_mcnx_assoc_upd_fn_t cb, void *arg);
extern int wlc_mcnx_assoc_upd_unregister(wlc_mcnx_info_t *mcnx,
	wlc_mcnx_assoc_upd_fn_t cb, void *arg);
/* AP bss up/down notification */
typedef struct {
	wlc_bsscfg_t *cfg;
	bool up;
} wlc_mcnx_bss_upd_data_t;
typedef void (*wlc_mcnx_bss_upd_fn_t)(void *ctx, wlc_mcnx_bss_upd_data_t *notif_data);
extern int wlc_mcnx_bss_upd_register(wlc_mcnx_info_t *mcnx,
	wlc_mcnx_bss_upd_fn_t cb, void *arg);
extern int wlc_mcnx_bss_upd_unregister(wlc_mcnx_info_t *mcnx,
	wlc_mcnx_bss_upd_fn_t cb, void *arg);
/* tsf change notification */
typedef struct {
	wlc_bsscfg_t *cfg;
} wlc_mcnx_tsf_upd_data_t;
typedef void (*wlc_mcnx_tsf_upd_fn_t)(void *ctx, wlc_mcnx_tsf_upd_data_t *notif_data);
extern int wlc_mcnx_tsf_upd_register(wlc_mcnx_info_t *mcnx,
	wlc_mcnx_tsf_upd_fn_t cb, void *arg);
extern int wlc_mcnx_tsf_upd_unregister(wlc_mcnx_info_t *mcnx,
	wlc_mcnx_tsf_upd_fn_t cb, void *arg);
/* intr notification */
typedef struct {
	wlc_bsscfg_t *cfg;
	int intr;
	uint32 tsf_h;
	uint32 tsf_l;
} wlc_mcnx_intr_data_t;
typedef void (*wlc_mcnx_intr_fn_t)(void *ctx, wlc_mcnx_intr_data_t *notif_data);
extern int wlc_mcnx_intr_register(wlc_mcnx_info_t *mcnx,
	wlc_mcnx_intr_fn_t cb, void *arg);
extern int wlc_mcnx_intr_unregister(wlc_mcnx_info_t *mcnx,
	wlc_mcnx_intr_fn_t cb, void *arg);

/* active window/client traffic window */
extern void wlc_mcnx_ctw_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool enab, uint32 aw);

/* absence schedule */
typedef struct {
	uint32 start;
	uint32 duration;
	uint32 interval;
	uint32 count;
} wlc_mcnx_abs_t;
extern void wlc_mcnx_abs_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	bool enab, wlc_mcnx_abs_t *sched);

/* suspend/resume mcnx h/w resources */
extern void wlc_mcnx_mac_suspend(wlc_mcnx_info_t *mcnx);
extern void wlc_mcnx_mac_resume(wlc_mcnx_info_t *mcnx);
extern void wlc_mcnx_shm_bss_idx_set(wlc_mcnx_info_t *mcnx, int bss_idx);

#endif /* WLC_HIGH */

/* interrupt handler */
extern void wlc_p2p_int_proc(wlc_info_t *wlc, uint8 *p2p_interrupts, uint32 tsf_l, uint32 tsf_h);
extern void wlc_tbtt_ucode_to_tbtt32(uint32 tsf_l, uint32 *tbtt_l);
extern void wlc_tbtt_ucode_to_tbtt64(uint32 tsf_h, uint32 tsf_l, uint32 *tbtt_h, uint32 *tbtt_l);

/* notifications */
#endif	/* !_wlc_mcnx_h_ */
