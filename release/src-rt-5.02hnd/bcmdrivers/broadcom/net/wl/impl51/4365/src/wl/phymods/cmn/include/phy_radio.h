/*
 * RADIO control module internal interface (to other PHY modules).
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

#ifndef _phy_radio_h_
#define _phy_radio_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_radio_info phy_radio_info_t;

/* attach/detach */
phy_radio_info_t *phy_radio_attach(phy_info_t *pi);
void phy_radio_detach(phy_radio_info_t *ri);

/* query radio idcode */
uint32 phy_radio_query_idcode(phy_radio_info_t *ri);

#endif /* _phy_radio_h_ */
