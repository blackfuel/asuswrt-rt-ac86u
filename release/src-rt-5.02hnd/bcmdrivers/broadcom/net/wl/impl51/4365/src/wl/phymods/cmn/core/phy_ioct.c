/*
 * PHY Core module implementation - IOCtlTable registration
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
#include <phy_api.h>
#include <wlc_iocv_types.h>

#include "phy_ioct.h"

/* local functions */

#ifndef ALL_NEW_PHY_MOD
int phy_legacy_register_ioct(phy_info_t *pi, wlc_iocv_info_t *ii);
#endif

/* Register all modules' ioctl tables/handlers */
int
BCMATTACHFN(phy_register_ioct)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	ASSERT(ii != NULL);

#ifndef ALL_NEW_PHY_MOD
	/* Register legacy ioctl table/handlers */
	if (phy_legacy_register_ioct(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_legacy_register_ioct failed\n", __FUNCTION__));
		goto fail;
	}
#endif


	/* Regiser other modules' common ioctl tables/dispatchers here ... */


	return BCME_OK;

fail:
	return BCME_ERROR;
}
