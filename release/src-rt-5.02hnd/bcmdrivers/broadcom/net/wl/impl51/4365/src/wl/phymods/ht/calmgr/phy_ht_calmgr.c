/*
 * HTPHY Calibration Manager module implementation
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
#include <phy_mem.h>
#include "phy_type_calmgr.h"
#include <phy_ht.h>
#include <phy_ht_calmgr.h>

/* module private states */
struct phy_ht_calmgr_info {
	phy_info_t *pi;
	phy_ht_info_t *hti;
	phy_calmgr_info_t *ci;
};

/* local functions */
static int phy_ht_calmgr_prepare(phy_type_calmgr_ctx_t *ctx);
static void phy_ht_calmgr_cleanup(phy_type_calmgr_ctx_t *ctx);

/* register phy type specific implementation */
phy_ht_calmgr_info_t *
BCMATTACHFN(phy_ht_calmgr_register_impl)(phy_info_t *pi, phy_ht_info_t *hti,
	phy_calmgr_info_t *ci)
{
	phy_ht_calmgr_info_t *info;
	phy_type_calmgr_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_ht_calmgr_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_ht_calmgr_info_t));
	info->pi = pi;
	info->hti = hti;
	info->ci = ci;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.prepare = phy_ht_calmgr_prepare;
	fns.cleanup = phy_ht_calmgr_cleanup;
	fns.ctx = info;

	if (phy_calmgr_register_impl(ci, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_calmgr_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ht_calmgr_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ht_calmgr_unregister_impl)(phy_ht_calmgr_info_t *info)
{
	phy_info_t *pi;

	ASSERT(info);
	pi = info->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_calmgr_unregister_impl(info->ci);

	phy_mfree(pi, info, sizeof(phy_ht_calmgr_info_t));
}

static int
phy_ht_calmgr_prepare(phy_type_calmgr_ctx_t *ctx)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	return BCME_OK;
}

static void
phy_ht_calmgr_cleanup(phy_type_calmgr_ctx_t *ctx)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}
