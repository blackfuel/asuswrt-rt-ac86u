/*
 * LCNPHY Core module implementation
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <wlc_iocv_types.h>
#include "phy_type_lcn.h"
#include "phy_type_lcn_iovt.h"
#include "phy_type_lcn_iovt_high.h"

/* local functions */

/* register iovar tables/handlers to IOC module */
int
BCMATTACHFN(phy_lcn_high_register_iovt)(wlc_iocv_info_t *ii)
{
	return phy_lcn_register_iovt(NULL, NULL, ii);
}
