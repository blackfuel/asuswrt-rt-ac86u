/*
 * HTPHY Calibration Manager module interface (to other PHY modules).
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

#ifndef _phy_ht_calmgr_h_
#define _phy_ht_calmgr_h_

#include <phy_api.h>
#include <phy_ht.h>
#include <phy_calmgr.h>

#include <phy_utils_math.h>

/* forward declaration */
typedef struct phy_ht_calmgr_info phy_ht_calmgr_info_t;

/* register/unregister HTPHY specific implementations to/from common */
phy_ht_calmgr_info_t *phy_ht_calmgr_register_impl(phy_info_t *pi,
	phy_ht_info_t *hti, phy_calmgr_info_t *cmn_info);
void phy_ht_calmgr_unregister_impl(phy_ht_calmgr_info_t *info);

#endif /* _phy_ht_calmgr_h_ */
