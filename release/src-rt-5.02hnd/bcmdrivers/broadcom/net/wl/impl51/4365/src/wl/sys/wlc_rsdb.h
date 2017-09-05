/*
 * WLC RSDB API definition
 * Broadcom 802.11abg Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_rsdb.h 543330 2015-03-24 07:15:54Z $
 */


#ifndef _wlc_rsdb_h_
#define _wlc_rsdb_h_
#ifdef WLRSDB
enum wlc_rsdb_modes {
	WLC_RSDB_MODE_AUTO = AUTO,
	WLC_RSDB_MODE_2X2,
	WLC_RSDB_MODE_RSDB,
	WLC_RSDB_MODE_80P80,
	WLC_RSDB_MODE_MAX
};

#define WLC_RSDB_DUAL_MAC_MODE(mode)	((mode) == WLC_RSDB_MODE_RSDB)
#define WLC_RSDB_SINGLE_MAC_MODE(mode)	(((mode) == WLC_RSDB_MODE_2X2) ||	\
	((mode) == WLC_RSDB_MODE_80P80))
#define WLC_RSDB_GET_PRIMARY_WLC(wlc)	((wlc)->cmn->wlc[0])

#define SCB_MOVE /* Added flag temporarily to enable scb move during bsscfg clone */
#define WLC_RSDB_CURR_MODE(wlc) WLC_RSDB_EXTRACT_MODE((wlc)->cmn->rsdb_mode)
extern int wlc_rsdb_assoc_mode_change(wlc_bsscfg_t **cfg, wlc_bss_info_t *bi);
extern int wlc_rsdb_change_mode(wlc_info_t *wlc, int8 to_mode);
#ifdef WL_MODESW
extern uint8 wlc_rsdb_downgrade_wlc(wlc_info_t *wlc);
extern uint8 wlc_rsdb_upgrade_wlc(wlc_info_t *wlc);
#endif
int wlc_rsdb_get_wlcs(wlc_info_t *wlc, wlc_info_t **wlc_2g, wlc_info_t **wlc_5g);
wlc_info_t * wlc_rsdb_get_other_wlc(wlc_info_t *wlc);
wlc_info_t* wlc_rsdb_find_wlc_for_chanspec(wlc_info_t *wlc, chanspec_t chanspec);
wlc_bsscfg_t* wlc_rsdb_cfg_for_chanspec(wlc_info_t *wlc, wlc_bsscfg_t *cfg, chanspec_t chanspec);
void wlc_rsdb_update_wlcif(wlc_info_t *wlc, wlc_bsscfg_t *from, wlc_bsscfg_t *to);
int wlc_rsdb_join_prep_wlc(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *SSID, int len,
	wl_join_scan_params_t *scan_params,
	wl_join_assoc_params_t *assoc_params, int assoc_params_len);
wlc_bsscfg_t*
wlc_rsdb_bsscfg_clone(wlc_info_t *from_wlc, wlc_info_t *to_wlc, wlc_bsscfg_t *cfg, int *ret);
int wlc_rsdb_attach(wlc_info_t* wlc);
void wlc_rsdb_detach(wlc_info_t* wlc);
bool wlc_rsdb_update_active(wlc_info_t *wlc, bool *old_state);
extern uint16 wlc_rsdb_mode(void *hdl);
bool wlc_rsdb_chkiovar(const bcm_iovar_t  *vi_ptr, uint32 actid, int32 wlc_indx);
#else /* WLRSDB */
#define wlc_rsdb_mode(hdl) (PHYMODE_MIMO)
#endif /* WLRSDB */

#ifdef RSDB_DFS_SCAN
extern int wlc_rsdb_dfs_scan_prep(wlc_info_t * wlc);
#endif

#if defined(WLRSDB) && !defined(WL_DUALNIC_RSDB)
void wlc_rsdb_bmc_smac_template(void *wlc, int tplbuf, uint32 bufsize);
extern void wlc_rsdb_set_phymode(void *hdl, uint32 phymode);
#else
#define wlc_rsdb_bmc_smac_template(hdl, tplbuf, bufsize)  do {} while (0)
#define wlc_rsdb_set_phymode(a, b) do {} while (0)
#endif /* defined(WLRSDB) && !defined(WL_DUALNIC_RSDB) */
#endif /* _wlc_rsdb_h_ */
