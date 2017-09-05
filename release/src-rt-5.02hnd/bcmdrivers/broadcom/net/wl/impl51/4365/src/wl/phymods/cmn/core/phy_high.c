/*
 * PHY Core module implementation (for HIGH MAC driver)
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

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <wlioctl.h>
#include <wl_dbg.h>
#include <wlc_iocv_types.h>

#include <phy_api.h>
#include <phy_high_api.h>

#include "phy_iovt.h"
#include "phy_ioct.h"
#include "phy_type_disp_high.h"

/* local functions */

/* Register all iovar tables to/from system */
int
BCMATTACHFN(phy_high_register_iovt_all)(uint phytype, wlc_iocv_info_t *ii)
{
	int err;

	/* Register all common layer's iovar tables/handlers */
	if ((err = phy_register_iovt(NULL, ii)) != BCME_OK) {
		WL_ERROR(("%s: phy_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register PHY type implementation layer's iovar tables/handlers */
	if ((err = phy_type_high_register_iovt(phytype, ii)) != BCME_OK) {
		WL_ERROR(("%s: phy_type_high_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	return BCME_OK;

fail:
	return err;
}

/* Register all ioctl tables to/from system */
int
BCMATTACHFN(phy_high_register_ioct_all)(uint phytype, wlc_iocv_info_t *ii)
{
	int err;

	/* Register all common layer's ioctl tables/handlers */
	if ((err = phy_register_ioct(NULL, ii)) != BCME_OK) {
		WL_ERROR(("%s: phy_register_ioct failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register PHY type implementation layer's ioctl tables/handlers */
	if ((err = phy_type_high_register_ioct(phytype, ii)) != BCME_OK) {
		WL_ERROR(("%s: phy_type_high_register_ioct failed\n", __FUNCTION__));
		goto fail;
	}

	return BCME_OK;

fail:
	return err;
}
