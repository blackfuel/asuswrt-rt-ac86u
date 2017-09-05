/*
 * ACPHY PAPD CAL module interface (to other PHY modules).
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

#ifndef _phy_ac_papdcal_h_
#define _phy_ac_papdcal_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_papdcal.h>

/* forward declaration */
typedef struct phy_ac_papdcal_info phy_ac_papdcal_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_papdcal_info_t *phy_ac_papdcal_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_papdcal_info_t *mi);
void phy_ac_papdcal_unregister_impl(phy_ac_papdcal_info_t *info);


/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
extern uint16 papd_gainctrl_pga[];

/* used for PAPD cal */
typedef struct _acphy_txgains {
	uint16 txlpf;
	uint16 txgm;
	uint16 pga;
	uint16 pad;
	uint16 ipa;
} acphy_txgains_t;

#define ACPHY_PAPD_EPS_TBL_SIZE		64
extern uint32 acphy_papd_scaltbl[];

extern void wlc_phy_papd_phy_cleanup_acphy(phy_info_t *pi);
extern void wlc_phy_papd_cal_acphy(phy_info_t *pi, uint16 num_iter, uint8 core, uint16 startindex,
	uint16 yrefindex, uint16 stopindex);
extern void wlc_phy_papd_set_rfpwrlut(phy_info_t *pi);
extern void wlc_phy_txpwr_papd_cal_run_acphy(phy_info_t *pi, uint8 tx_pre_cal_pwr_ctrl_state);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
extern void wlc_phy_papd_dump_eps_trace_acphy(phy_info_t *pi, struct bcmstrbuf *b);
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */
extern void wlc_phy_papd_phy_setup_acphy(phy_info_t *pi);
extern void wlc_phy_tiny_papd_cal_run_acphy(phy_info_t *pi,
	uint8 tx_pre_cal_pwr_ctrl_state);

#endif /* _phy_ac_papdcal_h_ */
