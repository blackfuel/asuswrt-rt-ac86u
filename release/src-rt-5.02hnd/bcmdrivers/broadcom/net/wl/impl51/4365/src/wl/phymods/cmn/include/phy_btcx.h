/*
 * BlueToothCoExistence module internal interface (to other PHY modules).
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

#ifndef _phy_btcx_h_
#define _phy_btcx_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_btcx_info phy_btcx_info_t;

/* attach/detach */
phy_btcx_info_t *phy_btcx_attach(phy_info_t *pi);
void phy_btcx_detach(phy_btcx_info_t *ri);

#endif /* _phy_btcx_h_ */
