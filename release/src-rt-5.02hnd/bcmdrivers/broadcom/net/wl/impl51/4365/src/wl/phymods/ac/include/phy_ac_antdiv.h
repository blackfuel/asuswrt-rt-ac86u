/*
 * ACPHY ANTennaDIVersity module interface (to other PHY modules).
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

#ifndef _phy_ac_antdiv_h_
#define _phy_ac_antdiv_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_antdiv.h>

/* forward declaration */
typedef struct phy_ac_antdiv_info phy_ac_antdiv_info_t;

/* register/unregister phy type specific implementation */
phy_ac_antdiv_info_t *phy_ac_antdiv_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_antdiv_info_t *di);
extern void phy_ac_antdiv_unregister_impl(phy_ac_antdiv_info_t *di);
extern bool wlc_phy_check_antdiv_enable_acphy(phy_info_t *pi);
extern void wlc_phy_antdiv_acphy(phy_info_t *pi, uint8 val);

#endif /* _phy_ac_antdiv_h_ */
