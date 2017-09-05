/*
 * ACPHY Calibration Cache module implementation
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
#include <phy_mem.h>
#include "phy_type_cache.h"
#include <phy_ac.h>
#include <phy_ac_cache.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_radioreg_20691.h>
#include <wlc_phy_radio.h>
#include <wlc_phyreg_ac.h>
#include <wlc_phy_int.h>

#include <phy_utils_reg.h>
#include <phy_ac_info.h>


/* module private states */
struct phy_ac_cache_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_cache_info_t *cmn_info;
};

/* local functions */

/* register phy type specific implementation */
phy_ac_cache_info_t *
BCMATTACHFN(phy_ac_cache_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_cache_info_t *cmn_info)
{
	phy_ac_cache_info_t *ac_info;
	phy_type_cache_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_cache_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = ac_info;

	if (phy_cache_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_cache_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_ac_cache_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_cache_unregister_impl)(phy_ac_cache_info_t *ac_info)
{
	phy_info_t *pi;
	phy_cache_info_t *cmn_info;

	ASSERT(ac_info);
	pi = ac_info->pi;
	cmn_info = ac_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_cache_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info, sizeof(phy_ac_cache_info_t));
}
