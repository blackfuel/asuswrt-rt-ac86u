/*
 * ACPHY TxPowerCtrl module interface (to other PHY modules).
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _phy_ac_tpc_h_
#define _phy_ac_tpc_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_tpc.h>

/* forward declaration */
typedef struct phy_ac_tpc_info phy_ac_tpc_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_tpc_info_t *phy_ac_tpc_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_tpc_info_t *ti);
void phy_ac_tpc_unregister_impl(phy_ac_tpc_info_t *info);

void phy_ac_tpc_shortwindow_upd(phy_info_t *pi, bool new_channel);

#define TSSI_DIVWAR_INDX (2)

/* #ifdef PREASSOC_PWRCTRL */
typedef struct phy_pwr_ctrl_save_acphy {
	bool status_idx_carry_2g[PHY_CORE_MAX];
	bool status_idx_carry_5g[PHY_CORE_MAX];
	uint8 status_idx_2g[PHY_CORE_MAX];
	uint8 status_idx_5g[PHY_CORE_MAX];
	uint16 last_chan_stored_2g;
	uint16 last_chan_stored_5g;
	int8   pwr_qdbm_2g[PHY_CORE_MAX];
	int8   pwr_qdbm_5g[PHY_CORE_MAX];
	bool   stored_not_restored_2g[PHY_CORE_MAX];
	bool   stored_not_restored_5g[PHY_CORE_MAX];

} phy_pwr_ctrl_s;
/* #endif */  /* PREASSOC_PWRCTRL */

extern uint8 wlc_phy_tssi2dbm_acphy(phy_info_t *pi, int32 tssi, int32 a1, int32 b0, int32 b1);
extern void wlc_phy_get_paparams_for_band_acphy(phy_info_t *pi, int16 *a1, int16 *b0, int16 *b1);
extern void wlc_phy_read_txgain_acphy(phy_info_t *pi);
extern void wlc_phy_txpwr_by_index_acphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex);
extern void wlc_phy_get_txgain_settings_by_index_acphy(phy_info_t *pi,
	txgain_setting_t *txgain_settings, int8 txpwrindex);
extern void wlc_phy_get_tx_bbmult_acphy(phy_info_t *pi, uint16 *bb_mult, uint16 core);
extern void wlc_phy_set_tx_bbmult_acphy(phy_info_t *pi, uint16 *bb_mult, uint16 core);
extern uint32 wlc_phy_txpwr_idx_get_acphy(phy_info_t *pi);
extern void wlc_phy_txpwrctrl_enable_acphy(phy_info_t *pi, uint8 ctrl_type);
extern void wlc_phy_txpwr_fixpower_acphy(phy_info_t *pi);
extern void wlc_phy_txpower_sromlimit_get_acphy(phy_info_t *pi,
	chanspec_t chanspec, ppr_t *max_pwr, uint8 core);
extern void wlc_phy_txpower_sromlimit_get_srom12_acphy(phy_info_t *pi,
	chanspec_t chanspec, ppr_t *max_pwr, uint8 core);
extern void wlc_phy_txpwr_est_pwr_acphy(phy_info_t *pi, uint8 *Pout, uint8 *Pout_adj);
extern uint16 * wlc_phy_get_tx_pwrctrl_tbl_2069(phy_info_t *pi);
extern int8 wlc_phy_tone_pwrctrl(phy_info_t *pi, int8 tx_idx, uint8 core);

#ifdef PREASSOC_PWRCTRL
extern void wlc_phy_store_tx_pwrctrl_setting_acphy(phy_info_t *pi, chanspec_t previous_channel);
#endif

extern void wlc_phy_txpwrctrl_set_target_acphy(phy_info_t *pi, uint8 pwr_qtrdbm, uint8 core);
extern void wlc_phy_txpwrctrl_config_acphy(phy_info_t *pi);
extern int wlc_phy_txpower_core_offset_set_acphy(phy_info_t *pi,
	struct phy_txcore_pwr_offsets *offsets);
extern int wlc_phy_txpower_core_offset_get_acphy(phy_info_t *pi,
	struct phy_txcore_pwr_offsets *offsets);

#if defined(WL_SARLIMIT) || defined(BCM_OL_DEV) || defined(WL_SAR_SIMPLE_CONTROL)
extern void wlc_phy_set_sarlimit_acphy(phy_info_t *pi);
#endif /* WL_SARLIMIT || BCM_OL_DEV || WL_SAR_SIMPLE_CONTROL */

#if defined(WLTEST)
extern void wlc_phy_iovar_patrim_acphy(phy_info_t *pi, int32 *ret_int_ptr);
#endif
extern int8 wlc_phy_txpwrctrl_update_minpwr_acphy(phy_info_t *pi);
#endif /* _phy_ac_tpc_h_ */
