/*
 * HTPHY VCO CAL module interface (to other PHY modules).
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

#ifndef _phy_ht_vcocal_h_
#define _phy_ht_vcocal_h_

#include <phy_api.h>
#include <phy_ht.h>
#include <phy_vcocal.h>

/* forward declaration */
typedef struct phy_ht_vcocal_info phy_ht_vcocal_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ht_vcocal_info_t *phy_ht_vcocal_register_impl(phy_info_t *pi,
	phy_ht_info_t *aci, phy_vcocal_info_t *mi);
void phy_ht_vcocal_unregister_impl(phy_ht_vcocal_info_t *info);

#endif /* _phy_ht_vcocal_h_ */
