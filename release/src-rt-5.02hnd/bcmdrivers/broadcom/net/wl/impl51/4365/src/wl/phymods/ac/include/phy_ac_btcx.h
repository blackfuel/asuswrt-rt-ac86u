/*
 * ACPHY BT Coex module interface (to other PHY modules).
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

#ifndef _phy_ac_btcx_h_
#define _phy_ac_btcx_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_btcx.h>

/* forward declaration */
typedef struct phy_ac_btcx_info phy_ac_btcx_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_btcx_info_t *phy_ac_btcx_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_btcx_info_t *cmn_info);
void phy_ac_btcx_unregister_impl(phy_ac_btcx_info_t *info);

extern void wlc_phy_set_bt_on_core1_acphy(phy_info_t *pi, uint8 bt_fem_val, uint16 gpioen);
extern void wlc_phy_bt_on_gpio4_acphy(phy_info_t *pi);
extern void wlc_phy_set_femctrl_bt_wlan_ovrd_acphy(phy_info_t *pi, int8 state);
extern int8 wlc_phy_get_femctrl_bt_wlan_ovrd_acphy(phy_info_t *pi);

#endif /* _phy_ac_btcx_h_ */
