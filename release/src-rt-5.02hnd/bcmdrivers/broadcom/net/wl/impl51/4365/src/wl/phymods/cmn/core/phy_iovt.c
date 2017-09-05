/*
 * PHY Core module implementation - IOVarTable registration
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

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_api.h>
#include "phy_iovt.h"
#include <phy_radar_iov.h>
#include <phy_temp_iov.h>

#include <wlc_iocv_types.h>

/* local functions */

#ifndef ALL_NEW_PHY_MOD
int phy_legacy_register_iovt(phy_info_t *pi, wlc_iocv_info_t *ii);
#endif

/* Register all modules' iovar tables/handlers */
int
BCMATTACHFN(phy_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	ASSERT(ii != NULL);

#ifndef ALL_NEW_PHY_MOD
	/* Register legacy iovar table/handlers */
	if (phy_legacy_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_legacy_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#endif

#if defined(AP) && defined(RADAR)
	/* Register radar common iovar table/handlers */
	if (phy_radar_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_radar_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#endif

	/* Register TEMPerature sense common iovar table/handlers */
	if (phy_temp_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_temp_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}


	/* Regiser other modules' common iovar tables/dispatchers here ... */


	return BCME_OK;

fail:
	return BCME_ERROR;
}
