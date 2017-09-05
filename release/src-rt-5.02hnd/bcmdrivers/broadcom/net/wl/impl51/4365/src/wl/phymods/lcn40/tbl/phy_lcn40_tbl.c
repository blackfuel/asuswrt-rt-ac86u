/*
 * LCN40PHY PHYTableInit module implementation
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
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_tbl.h"
#include <phy_lcn40.h>
#include <phy_lcn40_tbl.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn40.h>
/* TODO: all these are going away... > */
#endif

/* module private states */
struct phy_lcn40_tbl_info {
	phy_info_t *pi;
	phy_lcn40_info_t *lcn40i;
	phy_tbl_info_t *ii;
};

/* local functions */
static int phy_lcn40_tbl_init(phy_type_tbl_ctx_t *ctx);

/* Register/unregister LCN40PHY specific implementation to common layer */
phy_lcn40_tbl_info_t *
BCMATTACHFN(phy_lcn40_tbl_register_impl)(phy_info_t *pi, phy_lcn40_info_t *lcn40i,
	phy_tbl_info_t *ii)
{
	phy_lcn40_tbl_info_t *info;
	phy_type_tbl_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn40_tbl_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn40_tbl_info_t));
	info->pi = pi;
	info->lcn40i = lcn40i;
	info->ii = ii;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init = phy_lcn40_tbl_init;
	fns.ctx = info;

	phy_tbl_register_impl(ii, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn40_tbl_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn40_tbl_unregister_impl)(phy_lcn40_tbl_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_tbl_info_t *ii = info->ii;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_tbl_unregister_impl(ii);

	phy_mfree(pi, info, sizeof(phy_lcn40_tbl_info_t));
}

/* h/w init/down */
static int
WLBANDINITFN(phy_lcn40_tbl_init)(phy_type_tbl_ctx_t *ctx)
{
	phy_lcn40_tbl_info_t *lcn40ii = (phy_lcn40_tbl_info_t *)ctx;
	phy_info_t *pi = lcn40ii->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	wlc_phy_init_lcn40phy(pi);

	return BCME_OK;
}
