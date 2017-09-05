/*
 * HTPHY Radar Detect module interface (to other PHY modules).
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

#ifndef _phy_ht_radar_h_
#define _phy_ht_radar_h_

#include <phy_api.h>
#include <phy_ht.h>
#include <phy_radar.h>

/* forward declaration */
typedef struct phy_ht_radar_info phy_ht_radar_info_t;

/* register/unregister HTPHY specific implementations to/from common */
phy_ht_radar_info_t *phy_ht_radar_register_impl(phy_info_t *pi,
	phy_ht_info_t *hti, phy_radar_info_t *ri);
void phy_ht_radar_unregister_impl(phy_ht_radar_info_t *info);

void phy_ht_radar_upd(phy_ht_radar_info_t *ri);

#endif /* _phy_ht_radar_h_ */
