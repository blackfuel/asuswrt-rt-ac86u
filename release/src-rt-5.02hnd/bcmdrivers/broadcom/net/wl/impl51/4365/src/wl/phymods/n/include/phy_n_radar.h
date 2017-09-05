/*
 * NPHY Radar Detect module interface (to other PHY modules).
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

#ifndef _phy_n_radar_h_
#define _phy_n_radar_h_

#include <phy_api.h>
#include <phy_n.h>
#include <phy_radar.h>

/* forward declaration */
typedef struct phy_n_radar_info phy_n_radar_info_t;

/* register/unregister NPHY specific implementations to/from common */
phy_n_radar_info_t *phy_n_radar_register_impl(phy_info_t *pi,
	phy_n_info_t *ai, phy_radar_info_t *ri);
void phy_n_radar_unregister_impl(phy_n_radar_info_t *info);

void phy_n_radar_upd(phy_n_radar_info_t *ri);

#endif /* _phy_n_radar_h_ */
