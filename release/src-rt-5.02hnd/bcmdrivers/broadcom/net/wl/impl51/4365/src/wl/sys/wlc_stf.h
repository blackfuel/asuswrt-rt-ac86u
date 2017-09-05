/*
 * Code that controls the antenna/core/chain
 * Broadcom 802.11bang Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_stf.h 635740 2016-05-05 02:51:23Z $
 */

#ifndef _wlc_stf_h_
#define _wlc_stf_h_

#define AUTO_SPATIAL_EXPANSION	-1
#define MIN_SPATIAL_EXPANSION	0
#define MAX_SPATIAL_EXPANSION	1

#define PWRTHROTTLE_CHAIN	1
#define PWRTHROTTLE_DUTY_CYCLE	2

extern int wlc_stf_attach(wlc_info_t* wlc);
extern void wlc_stf_detach(wlc_info_t* wlc);
extern void wlc_stf_chain_init(wlc_info_t *wlc);

#if defined(WLC_LOW) && defined(WLC_HIGH) && defined(WL11N) && !defined(WLC_NET80211)
extern void wlc_stf_pwrthrottle_upd(wlc_info_t *wlc);
#else
#define wlc_stf_pwrthrottle_upd(a) do {} while (0)
#endif

#ifdef WL11N
extern void wlc_stf_txchain_set_complete(wlc_info_t *wlc);
#ifdef WL11AC
extern void wlc_stf_chanspec_upd(wlc_info_t *wlc);
#endif /* WL11AC */
extern void wlc_stf_tempsense_upd(wlc_info_t *wlc);
extern void wlc_stf_ss_algo_channel_get(wlc_info_t *wlc, uint16 *ss_algo_channel,
	chanspec_t chanspec);
extern int wlc_stf_ss_update(wlc_info_t *wlc, wlcband_t *band);
extern void wlc_stf_phy_txant_upd(wlc_info_t *wlc);
extern int wlc_stf_txchain_set(wlc_info_t* wlc, int32 int_val, bool force, uint16 id);
extern int wlc_stf_txchain_subval_get(wlc_info_t* wlc, uint id, uint *txchain);
extern int wlc_stf_rxchain_set(wlc_info_t* wlc, int32 int_val, bool update);
extern bool wlc_stf_rxchain_ishwdef(wlc_info_t* wlc);
extern bool wlc_stf_txchain_ishwdef(wlc_info_t* wlc);
extern bool wlc_stf_stbc_rx_set(wlc_info_t* wlc, int32 int_val);
extern uint8 wlc_stf_txchain_get(wlc_info_t *wlc, ratespec_t rspec);
extern uint8 wlc_stf_txcore_get(wlc_info_t *wlc, ratespec_t rspec);
extern void wlc_stf_spatialpolicy_set_complete(wlc_info_t *wlc);
extern void wlc_stf_txcore_shmem_write(wlc_info_t *wlc, bool forcewr);
extern uint16 wlc_stf_spatial_expansion_get(wlc_info_t *wlc, ratespec_t rspec);
extern uint8 wlc_stf_get_pwrperrate(wlc_info_t *wlc, ratespec_t rspec,
	uint16 spatial_map);
extern int wlc_stf_get_204080_pwrs(wlc_info_t *wlc, ratespec_t rspec, txpwr204080_t* pwrs,
        wl_tx_chains_t txbf_chains);
extern void wlc_set_pwrthrottle_config(wlc_info_t *wlc);
extern int wlc_stf_duty_cycle_set(wlc_info_t *wlc, int duty_cycle, bool isOFDM, bool writeToShm);
extern void wlc_stf_chain_active_set(wlc_info_t *wlc, uint8 active_chains);
#ifdef RXCHAIN_PWRSAVE
extern uint8 wlc_stf_enter_rxchain_pwrsave(wlc_info_t *wlc);
extern void wlc_stf_exit_rxchain_pwrsave(wlc_info_t *wlc, uint8 ht_cap_rx_stbc);
#endif
#else
#define wlc_stf_spatial_expansion_get(a, b) 0
#define wlc_stf_get_pwrperrate(a, b, c) 0
#define wlc_stf_get_204080_pwrs(a, b, c) 0
#endif /* WL11N */

extern int wlc_stf_ant_txant_validate(wlc_info_t *wlc, int8 val);
extern void wlc_stf_phy_txant_upd(wlc_info_t *wlc);
extern void wlc_stf_phy_chain_calc(wlc_info_t *wlc);
extern uint16 wlc_stf_phytxchain_sel(wlc_info_t *wlc, ratespec_t rspec);
extern uint16 wlc_stf_d11hdrs_phyctl_txant(wlc_info_t *wlc, ratespec_t rspec);
extern uint16 wlc_stf_d11hdrs_phyctl_txcore_80p80phy(wlc_info_t *wlc, uint16 phyctl);
extern void wlc_stf_wowl_upd(wlc_info_t *wlc);
extern void wlc_stf_shmem_base_upd(wlc_info_t *wlc, uint16 base);
extern void wlc_stf_wowl_spatial_policy_set(wlc_info_t *wlc, int policy);
extern void wlc_update_txppr_offset(wlc_info_t *wlc, ppr_t *txpwr);
extern int wlc_stf_spatial_policy_set(wlc_info_t *wlc, int val);

typedef uint16 wlc_stf_txchain_st;
extern void wlc_stf_txchain_get_perrate_state(wlc_info_t *wlc, wlc_stf_txchain_st *state,
	wlc_stf_txchain_evt_notify func);
extern void
wlc_stf_txchain_restore_perrate_state(wlc_info_t *wlc, wlc_stf_txchain_st *state);
extern bool wlc_stf_saved_state_is_consistent(wlc_info_t *wlc, wlc_stf_txchain_st *state);
#ifdef WL_BEAMFORMING
extern void wlc_stf_set_txbf(wlc_info_t *wlc, bool enable);
#endif
#if !(defined(WLC_LOW) && !defined(WLTXPWR_CACHE))
extern int wlc_stf_txchain_pwr_offset_set(wlc_info_t *wlc, wl_txchain_pwr_offsets_t *offsets);
#endif
#if defined(WL_EXPORT_CURPOWER)
uint8 get_pwr_from_targets(wlc_info_t *wlc, ratespec_t rspec, ppr_t *txpwr);
#endif
#ifdef WLRSDB
extern void wlc_stf_phy_chain_calc_set(wlc_info_t *wlc);
#else
extern void BCMATTACHFN(wlc_stf_phy_chain_calc_set)(wlc_info_t *wlc);
#endif

#endif /* _wlc_stf_h_ */
