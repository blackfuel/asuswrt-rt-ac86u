/*
 * Rx Gain Control and Carrier Sense module interface (to other PHY modules).
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

#ifndef _phy_rxgcrs_h_
#define _phy_rxgcrs_h_

#include <phy_api.h>

/* forward declaration */
typedef struct phy_rxgcrs_info phy_rxgcrs_info_t;

/* attach/detach */
phy_rxgcrs_info_t *phy_rxgcrs_attach(phy_info_t *pi);
void phy_rxgcrs_detach(phy_rxgcrs_info_t *cmn_info);

/* down */
int phy_rxgcrs_down(phy_rxgcrs_info_t *cmn_info);

uint8 wlc_phy_get_locale(phy_rxgcrs_info_t *info);

#endif /* _phy_rxgcrs_h_ */
