/*
 * LCN40PHY RADIO control module interface (to other PHY modules).
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

#ifndef _phy_lcn40_radio_h_
#define _phy_lcn40_radio_h_

#include <phy_api.h>
#include <phy_lcn40.h>
#include <phy_radio.h>

/* forward declaration */
typedef struct phy_lcn40_radio_info phy_lcn40_radio_info_t;

/* register/unregister LCN40PHY specific implementations to/from common */
phy_lcn40_radio_info_t *phy_lcn40_radio_register_impl(phy_info_t *pi,
	phy_lcn40_info_t *lcn40i, phy_radio_info_t *ri);
void phy_lcn40_radio_unregister_impl(phy_lcn40_radio_info_t *info);

void phy_lcn40_radio_switch(phy_lcn40_radio_info_t *info, bool on);

uint32 phy_lcn40_radio_query_idcode(phy_info_t *pi);

#endif /* _phy_lcn40_radio_h_ */
