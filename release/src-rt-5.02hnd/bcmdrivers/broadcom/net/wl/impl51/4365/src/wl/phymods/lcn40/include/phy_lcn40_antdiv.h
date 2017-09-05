/*
 * LCN40PHY ANTennaDIVersity module interface (to other PHY modules).
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

#ifndef _phy_lcn40_antdiv_h_
#define _phy_lcn40_antdiv_h_

#include <phy_api.h>
#include <phy_lcn40.h>
#include <phy_antdiv.h>

/* forward declaration */
typedef struct phy_lcn40_antdiv_info phy_lcn40_antdiv_info_t;

/* register/unregister phy type specific implementation */
phy_lcn40_antdiv_info_t *phy_lcn40_antdiv_register_impl(phy_info_t *pi,
	phy_lcn40_info_t *lcn40i, phy_antdiv_info_t *di);
void phy_lcn40_antdiv_unregister_impl(phy_lcn40_antdiv_info_t *di);

/* set ant */
void phy_lcn40_antdiv_set_rx(phy_lcn40_antdiv_info_t *di, uint8 ant);

/* enable/disable? */
void phy_lcn40_swdiv_epa_pd(phy_lcn40_antdiv_info_t *di, bool diable);

#endif /* _phy_lcn40_antdiv_h_ */
