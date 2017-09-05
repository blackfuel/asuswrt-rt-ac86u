/*
 * LCN40PHY RSSI Compute module interface (to other PHY modules)
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

#ifndef _phy_lcn40_rssi_h_
#define _phy_lcn40_rssi_h_

#include <phy_api.h>
#include <phy_lcn40.h>
#include <phy_rssi.h>

/* forward declaration */
typedef struct phy_lcn40_rssi_info phy_lcn40_rssi_info_t;

/* register/unregister LCN40PHY specific implementations to/from common */
phy_lcn40_rssi_info_t *phy_lcn40_rssi_register_impl(phy_info_t *pi,
	phy_lcn40_info_t *lcn40i, phy_rssi_info_t *ri);
void phy_lcn40_rssi_unregister_impl(phy_lcn40_rssi_info_t *info);

extern int8 *phy_lcn40_pkt_rssi_gain_index_offset_2g;
#define phy_lcn40_get_pkt_rssi_gain_index_offset_2g(idx) \
	phy_lcn40_pkt_rssi_gain_index_offset_2g[idx]
#ifdef BAND5G
extern int8 *phy_lcn40_pkt_rssi_gain_index_offset_5g;
#define phy_lcn40_get_pkt_rssi_gain_index_offset_5g(idx) \
	phy_lcn40_pkt_rssi_gain_index_offset_5g[idx]
#endif

#endif /* _phy_lcn40_rssi_h_ */
