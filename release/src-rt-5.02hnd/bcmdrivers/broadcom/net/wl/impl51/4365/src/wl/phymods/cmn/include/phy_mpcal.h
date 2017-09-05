/*
 * MultiPhaseCAL module interface (to other PHY modules).
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

#ifndef _phy_mpcal_h_
#define _phy_mpcal_h_

#include <phy_api.h>

/* forward declaration */
typedef struct phy_mpcal_info phy_mpcal_info_t;

/* attach/detach */
phy_mpcal_info_t *phy_mpcal_attach(phy_info_t *pi);
void phy_mpcal_detach(phy_mpcal_info_t *mi);

/* up/down */
int phy_mpcal_init(phy_mpcal_info_t *mi);
int phy_mpcal_down(phy_mpcal_info_t *mi);

#endif /* _phy_mpcal_h_ */
