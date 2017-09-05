/*
 * Channel manager internal interface (to other PHY modules).
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

#ifndef _phy_chanmgr_h_
#define _phy_chanmgr_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_chanmgr_info phy_chanmgr_info_t;

/* attach/detach */
phy_chanmgr_info_t *phy_chanmgr_attach(phy_info_t *pi);
void phy_chanmgr_detach(phy_chanmgr_info_t *ri);

#endif /* _phy_chanmgr_h_ */
