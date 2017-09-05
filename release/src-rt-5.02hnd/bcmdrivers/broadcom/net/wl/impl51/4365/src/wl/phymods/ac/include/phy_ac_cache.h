/*
 * ACPHY Calibration Cache module interface (to other PHY modules).
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

#ifndef _phy_ac_cache_h_
#define _phy_ac_cache_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_cache.h>

#include <bcmutils.h>
#include <wlc_phy_int.h> /* *** !!! To be removed !!! *** */

/* forward declaration */
typedef struct phy_ac_cache_info phy_ac_cache_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_cache_info_t *phy_ac_cache_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_cache_info_t *cmn_info);
void phy_ac_cache_unregister_impl(phy_ac_cache_info_t *ac_info);


/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
extern void wlc_phy_cal_coeffs_upd(phy_info_t *pi, txcal_coeffs_t *txcal_cache);


#endif /* _phy_ac_cache_h_ */
