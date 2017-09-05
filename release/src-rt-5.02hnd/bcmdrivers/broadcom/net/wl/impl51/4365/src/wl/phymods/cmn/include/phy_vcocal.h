/*
 * VCO CAL module interface (to other PHY modules).
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

#ifndef _phy_vcocal_h_
#define _phy_vcocal_h_

#include <phy_api.h>

/* forward declaration */
typedef struct phy_vcocal_info phy_vcocal_info_t;

/* attach/detach */
phy_vcocal_info_t *phy_vcocal_attach(phy_info_t *pi);
void phy_vcocal_detach(phy_vcocal_info_t *cmn_info);

/* init */

#endif /* _phy_vcocal_h_ */
