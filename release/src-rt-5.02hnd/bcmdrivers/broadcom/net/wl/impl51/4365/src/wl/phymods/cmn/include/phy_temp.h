/*
 * TEMPerature sense module internal interface (to other PHY modules).
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

#ifndef _phy_temp_h_
#define _phy_temp_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_temp_info phy_temp_info_t;

/* attach/detach */
phy_temp_info_t *phy_temp_attach(phy_info_t *pi);
void phy_temp_detach(phy_temp_info_t *ri);

/* temp. throttle */
uint8 phy_temp_throttle(phy_temp_info_t *ti);

#ifdef	WL_DYNAMIC_TEMPSENSE
int phy_temp_get_cur_temp(phy_temp_info_t *ti);
int phy_temp_get_temp_thresh(phy_temp_info_t *ti);
#if defined(BCMDBG) || defined(WLTEST)
int phy_tem_get_override(phy_temp_info_t *ti);
#endif 
#endif /* WL_DYNAMIC_TEMPSENSE */

#endif /* _phy_temp_h_ */
