/*
 * HTPHY Core module implementation
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <wlc_iocv_types.h>
#include "phy_type_ht_ioct.h"
#include "phy_type_ht_ioct_high.h"

/* local functions */

/* register ioctl tables/handlers to IOC module */
int
BCMATTACHFN(phy_ht_high_register_ioct)(wlc_iocv_info_t *ii)
{
	return phy_ht_register_ioct(NULL, NULL, ii);
}
