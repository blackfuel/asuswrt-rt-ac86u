/*
 * LCNPHY ANTennaDIVersity module interface (to other PHY modules).
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

#ifndef _phy_lcn_antdiv_h_
#define _phy_lcn_antdiv_h_

#include <phy_api.h>
#include <phy_lcn.h>
#include <phy_antdiv.h>

/* forward declaration */
typedef struct phy_lcn_antdiv_info phy_lcn_antdiv_info_t;

/* register/unregister phy type specific implementation */
phy_lcn_antdiv_info_t *phy_lcn_antdiv_register_impl(phy_info_t *pi,
	phy_lcn_info_t *lcni, phy_antdiv_info_t *di);
void phy_lcn_antdiv_unregister_impl(phy_lcn_antdiv_info_t *di);

#endif /* _phy_lcn_antdiv_h_ */
