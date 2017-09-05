/*
 * ANTennaDIVersity module interface (to other PHY modules).
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

#ifndef _phy_antdiv_h_
#define _phy_antdiv_h_

#include <phy_api.h>

/* forward declaration */
typedef struct phy_antdiv_info phy_antdiv_info_t;

/* attach/detach */
phy_antdiv_info_t *phy_antdiv_attach(phy_info_t *pi);
void phy_antdiv_detach(phy_antdiv_info_t *di);

#ifdef WLC_SW_DIVERSITY
/* swctrl_mask */
uint16 phy_antdiv_get_swctrl_mask(phy_antdiv_info_t *di);
#endif

#endif /* _phy_antdiv_h_ */
