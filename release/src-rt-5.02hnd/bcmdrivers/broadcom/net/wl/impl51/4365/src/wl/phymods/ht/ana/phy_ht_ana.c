/*
 * HTPHY ANAcore contorl module implementation
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
#include <phy_mem.h>
#include <phy_ana.h>
#include "phy_type_ana.h"
#include <phy_ht.h>
#include <phy_ht_ana.h>

#include <wlc_phyreg_ht.h>
#include <phy_utils_reg.h>

/* module private states */
struct phy_ht_ana_info {
	phy_info_t *pi;
	phy_ht_info_t *hti;
	phy_ana_info_t *ani;
};

/* local functions */
static int phy_ht_ana_switch(phy_type_ana_ctx_t *ctx, bool on);
static void phy_ht_ana_reset(phy_type_ana_ctx_t *ctx);

/* Register/unregister HTPHY specific implementation to common layer. */
phy_ht_ana_info_t *
BCMATTACHFN(phy_ht_ana_register_impl)(phy_info_t *pi, phy_ht_info_t *hti,
	phy_ana_info_t *ani)
{
	phy_ht_ana_info_t *info;
	phy_type_ana_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ht_ana_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_ht_ana_info_t));
	info->pi = pi;
	info->hti = hti;
	info->ani = ani;

#ifndef BCM_OL_DEV
	phy_ht_ana_switch(info, ON);
#endif

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctrl = phy_ht_ana_switch;
	fns.reset = phy_ht_ana_reset;
	fns.ctx = info;

	phy_ana_register_impl(ani, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ht_ana_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ht_ana_unregister_impl)(phy_ht_ana_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_ana_info_t *ani = info->ani;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_ana_unregister_impl(ani);

	phy_mfree(pi, info, sizeof(phy_ht_ana_info_t));
}

/* switch anacore on/off */
static int
phy_ht_ana_switch(phy_type_ana_ctx_t *ctx, bool on)
{
	phy_ht_ana_info_t *info = (phy_ht_ana_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint16 core;

	PHY_TRACE(("%s %d\n", __FUNCTION__, on));

	FOREACH_CORE(pi, core) {
		if (on) {
			phy_utils_write_phyreg(pi, HTPHY_Afectrl(core), 0xcd);
			phy_utils_write_phyreg(pi, HTPHY_AfectrlOverride(core), 0x0);
		} else {
			phy_utils_write_phyreg(pi, HTPHY_AfectrlOverride(core), 0x07ff);
			phy_utils_write_phyreg(pi, HTPHY_Afectrl(core), 0x0fd);
		}
	}

	return BCME_OK;
}

/* reset h/w */
static void
phy_ht_ana_reset(phy_type_ana_ctx_t *ctx)
{
	phy_ht_ana_switch(ctx, ON);
}
