/*
 * ACPHY Sample Collect module interface (to other PHY modules).
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

#ifndef _phy_ac_samp_h_
#define _phy_ac_samp_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_samp.h>

/* forward declaration */
typedef struct phy_ac_samp_info phy_ac_samp_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_samp_info_t *phy_ac_samp_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_samp_info_t *cmn_info);
void phy_ac_samp_unregister_impl(phy_ac_samp_info_t *ac_info);

/* ************************************** */
/* ACPHY sample collect intermodule api's */
/* ************************************** */
#ifdef WL_PROXDETECT
void acphy_set_sc_startptr(phy_info_t *pi, uint32 start_idx);
void acphy_set_sc_stopptr(phy_info_t *pi, uint32 stop_idx);
#endif /* WL_PROXDETECT */
extern void phy_ac_lpf_hpc_override(phy_ac_info_t *aci, bool setup);
#ifdef VASIP_HW_SUPPORT
extern void phy_ac_svmp_sampcol_params_get(phy_info_t *pi, wl_svmp_sampcol_t* psampcol);
extern void phy_ac_svmp_sampcol_params_set(phy_info_t *pi, wl_svmp_sampcol_t* psampcol);
#endif /* VASIP_HW_SUPPORT */
#endif /* _phy_ac_samp_h_ */
