/*
 * BlueToothCoExistence module implementation.
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
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_wd.h>
#include <phy_type_btcx.h>

#include <phy_utils_reg.h>

/* module private states */
struct phy_btcx_info {
	phy_info_t *pi;
	phy_type_btcx_fns_t		*fns;		/* PHY specific function ptrs */
};

/* local function declaration */
static bool phy_btcx_wd(phy_wd_ctx_t *ctx);

/* attach/detach */
phy_btcx_info_t *
BCMATTACHFN(phy_btcx_attach)(phy_info_t *pi)
{
	phy_btcx_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_btcx_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_btcx_wd, info,
	                  PHY_WD_PRD_FAST, PHY_WD_FAST_BTCX, PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

	return info;

	/* error */
fail:
	phy_btcx_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_btcx_detach)(phy_btcx_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null btcx module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_btcx_info_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_btcx_register_impl)(phy_btcx_info_t *cmn_info,
	phy_type_btcx_fns_t *fns)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	cmn_info->fns = fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_btcx_unregister_impl)(phy_btcx_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	cmn_info->fns = NULL;
}

/* watchdog callback */
static bool
phy_btcx_wd(phy_wd_ctx_t *ctx)
{
	phy_btcx_info_t *bi = (phy_btcx_info_t *)ctx;
	phy_info_t *pi = bi->pi;

	phy_utils_phyreg_enter(pi);
	wlapi_update_bt_chanspec(pi->sh->physhim, pi->radio_chanspec,
	                         SCAN_INPROG_PHY(pi), RM_INPROG_PHY(pi));
	phy_utils_phyreg_exit(pi);

	return TRUE;
}
