/*
 * ACPHY Noise module interface (to other PHY modules).
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _phy_ac_noise_h_
#define _phy_ac_noise_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_noise.h>

#include <phy_ac_rxspur.h>

/* forward declaration */
typedef struct phy_ac_noise_info phy_ac_noise_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_noise_info_t *phy_ac_noise_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_noise_info_t *cmn_info);
void phy_ac_noise_unregister_impl(phy_ac_noise_info_t *ac_info);


/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

/* Number of tones(spurwar+nvshp) to be written */
#define ACPHY_SPURWAR_NV_NTONES                  32

/* ACI (start) */
#define ACPHY_ACI_CHAN_LIST_SZ 3

#define ACPHY_ACI_MAX_DESENSE_BPHY_DB 24
#define ACPHY_ACI_MAX_DESENSE_OFDM_DB 48
#define ACPHY_ACI_COARSE_DESENSE_UP 4
#define ACPHY_ACI_COARSE_DESENSE_DN 4

#define ACPHY_ACI_NUM_MAX_GLITCH_AVG 2
#define ACPHY_ACI_WAIT_POST_MITIGATION 1
#define ACPHY_ACI_OFDM_HI_GLITCH_THRESH 600
#define ACPHY_ACI_OFDM_LO_GLITCH_THRESH 300
#define ACPHY_ACI_BPHY_HI_GLITCH_THRESH 300
#define ACPHY_ACI_BPHY_LO_GLITCH_THRESH 100
#define ACPHY_ACI_BORDER_GLITCH_SLEEP 12
#define ACPHY_ACI_MD_GLITCH_SLEEP 8
#define ACPHY_ACI_LO_GLITCH_SLEEP 4
#define ACPHY_ACI_GLITCH_BUFFER_SZ 4

#define ACPHY_ACI_OFDM_HI_GLITCH_THRESH_TINY 300
#define ACPHY_ACI_OFDM_LO_GLITCH_THRESH_TINY 100

/* hw aci */
#define ACPHY_HWACI_MAX_STATES 5       /* min 1 for default */
#define ACPHY_HWACI_NOACI_WAIT_TIME  8 /* sec */
#define ACPHY_HWACI_SLEEP_TIME 2       /* After a change, sleep for 2s for hwaci stats refresh */

#define HWACI_DISABLE		0
#define HWACI_AUTO_FCBS		1
#define HWACI_FORCED_MITOFF	2
#define HWACI_FORCED_MITON	3
#define HWACI_AUTO_SW		4

/* ACI (end) */

/*
 * ACPHY_ENABLE_FCBS_HWACI  enables  HW ACI detection and HW mitigation thru use of the FCBS
 * This allows for the AGC to be in two states ACI and Normal.
 * This mode of operation is not compatible with the pre-exisiting
 * schemes in particular SW based desense.
 * ACPHY_ENABLE_FCBS_HWACI also disables a selection of existing ACI code.
 */
#ifndef WLC_DISABLE_ACI
#define ACPHY_ENABLE_FCBS_HWACI(pi) \
	(ACMAJORREV_3((pi)->pubpi.phy_rev) || ACMAJORREV_4((pi)->pubpi.phy_rev))
#define ACPHY_HWACI_WITH_DESENSE_ENG(pi) (ACMAJORREV_4((pi)->pubpi.phy_rev))
#define ACPHY_HWACI_HWTBL_MITIGATION(pi) (ACMAJORREV_33((pi)->pubpi.phy_rev))
#else
#define ACPHY_ENABLE_FCBS_HWACI(pi) 0
#define ACPHY_HWACI_WITH_DESENSE_ENG(pi) (0)
#define ACPHY_HWACI_HWTBL_MITIGATION(pi) (0)
#endif

typedef struct acphy_desense_values
{
	uint8 clipgain_desense[4]; /* in dBs */
	uint8 ofdm_desense, bphy_desense;      /* in dBs */
	uint8 lna1_tbl_desense, lna2_tbl_desense;   /* in ticks */
	uint8 lna1_gainlmt_desense, lna2_gainlmt_desense;   /* in ticks */
	uint8 lna1rout_gainlmt_desense;
	uint8 elna_bypass;
	uint8 mixer_setting_desense;
	uint8 nf_hit_lna12; /* mostly to adjust nb/w1 clip for bt cases */
	bool on;
	bool forced;
	uint8 analog_gain_desense_ofdm, analog_gain_desense_bphy; /* in dBs */
	uint8 lna1_idx_min, lna1_idx_max;   /* in ticks */
	uint8 lna2_idx_min, lna2_idx_max;   /* in ticks */
	uint8 mix_idx_min, mix_idx_max;   /* in ticks */
}  acphy_desense_values_t;

typedef struct desense_history {
	uint32 glitches[ACPHY_ACI_GLITCH_BUFFER_SZ];
	uint8 hi_glitch_dB;
	uint8 lo_glitch_dB;
	uint8 no_desense_change_time_cnt;
} desense_history_t;

typedef struct acphy_aci_params {
	/* array is indexed by chan/bw */
	uint8 chan;
	uint16 bw;
	uint64 last_updated;

	acphy_desense_values_t desense;
	int8 weakest_rssi;

	desense_history_t bphy_hist;
	desense_history_t ofdm_hist;
	uint8 glitch_buff_idx, glitch_upd_wait;

	uint8 hwaci_setup_state, hwaci_desense_state;
	uint8 hwaci_noaci_timer;
	uint8 hwaci_sleep;
} acphy_aci_params_t;

typedef struct {
	uint16 sample_time;
	uint16 energy_thresh;
	uint16 detect_thresh;
	uint16 wait_period;
	uint8 sliding_window;
	uint8 samp_cluster;
	uint8 nb_lo_th;
	uint8 w3_lo_th;
	uint8 w3_md_th;
	uint8 w3_hi_th;
	uint8 w2;
} acphy_hwaci_setup_t;

typedef struct
{
	uint16 energy_thresh;
	uint8 lna1_pktg_lmt, lna2_pktg_lmt, lna1rout_pktg_lmt;
	uint8 w2_sel, w2_thresh, nb_thresh;
	uint8 lna1_idx_min, lna1_idx_max;
	uint8 lna2_idx_min, lna2_idx_max;
	uint8 mix_idx_min, mix_idx_max;
} acphy_hwaci_state_t;

/* Monitor for the Modified Entries - nvshapingtbl */
typedef struct _acphy_nshapetbl_mon {
	uint8 mod_flag;
	uint8 offset[ACPHY_SPURWAR_NV_NTONES];
} acphy_nshapetbl_mon_t;

extern void wlc_phy_hwaci_init_acphy(phy_info_t *pi);
extern void wlc_phy_hwaci_setup_acphy(phy_info_t *pi, bool on, bool init);
extern uint8 wlc_phy_disable_hwaci_fcbs_trig(phy_info_t *pi);
extern void wlc_phy_restore_hwaci_fcbs_trig(phy_info_t *pi, uint8 trig_disable);
extern void wlc_phy_hwaci_mitigation_enable_acphy_tiny(phy_info_t *pi, uint8 hwaci_mode, bool init);
extern acphy_aci_params_t* wlc_phy_desense_aci_getset_chanidx_acphy(phy_info_t *pi,
	chanspec_t chanspec, bool create);
extern void wlc_phy_desense_aci_engine_acphy(phy_info_t *pi);
extern void wlc_phy_reset_noise_var_shaping_acphy(phy_info_t *pi);
extern void wlc_phy_hwaci_engine_acphy(phy_info_t *pi);
extern void wlc_phy_hwaci_mitigation_acphy_tiny(phy_info_t *pi, int8 desense_state);
extern void wlc_phy_upd_mix_gains_acphy(phy_info_t *pi);
extern void wlc_phy_desense_aci_reset_params_acphy(phy_info_t *pi,
	bool call_gainctrl, bool all2g, bool all5g);
extern void wlc_phy_desense_aci_upd_chan_stats_acphy(phy_info_t *pi,
	chanspec_t chanspec, int8 rssi);

extern void wlc_phy_set_aci_regs_acphy(phy_info_t *pi);
extern void wlc_phy_reset_noise_var_shaping_acphy(phy_info_t *pi);
extern void wlc_phy_noise_var_shaping_acphy(phy_info_t *pi, uint8 core_nv, uint8 core_sp,
	int8 *tone_id, uint8 noise_var[][ACPHY_SPURWAR_NV_NTONES], uint8 reset);

#endif /* _phy_ac_noise_h_ */
