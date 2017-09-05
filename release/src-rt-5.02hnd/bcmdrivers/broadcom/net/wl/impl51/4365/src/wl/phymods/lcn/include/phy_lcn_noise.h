/*
 * LCNPHY NOISEmeasure module interface (to other PHY modules).
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

#ifndef _phy_lcn_noise_h_
#define _phy_lcn_noise_h_

#include <phy_api.h>
#include <phy_lcn.h>
#include <phy_noise.h>

/* forward declaration */
typedef struct phy_lcn_noise_info phy_lcn_noise_info_t;

/* register/unregister LCNPHY specific implementation to/from common */
phy_lcn_noise_info_t *phy_lcn_noise_register_impl(phy_info_t *pi,
	phy_lcn_info_t *lcni, phy_noise_info_t *nxi);
void phy_lcn_noise_unregister_impl(phy_lcn_noise_info_t *info);

#endif /* _phy_lcn_noise_h_ */
