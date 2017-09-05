/*
 * LCNPHY RSSI Compute module interface (to other PHY modules)
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

#ifndef _phy_lcn_rssi_h_
#define _phy_lcn_rssi_h_

#include <phy_api.h>
#include <phy_lcn.h>
#include <phy_rssi.h>

/* forward declaration */
typedef struct phy_lcn_rssi_info phy_lcn_rssi_info_t;

/* register/unregister LCNPHY specific implementations to/from common */
phy_lcn_rssi_info_t *phy_lcn_rssi_register_impl(phy_info_t *pi,
	phy_lcn_info_t *lcni, phy_rssi_info_t *ri);
void phy_lcn_rssi_unregister_impl(phy_lcn_rssi_info_t *info);

extern int8 *phy_lcn_pkt_rssi_gain_index_offset;
#define phy_lcn_get_pkt_rssi_gain_index_offset(idx) phy_lcn_pkt_rssi_gain_index_offset[idx]

#endif /* _phy_lcn_rssi_h_ */
