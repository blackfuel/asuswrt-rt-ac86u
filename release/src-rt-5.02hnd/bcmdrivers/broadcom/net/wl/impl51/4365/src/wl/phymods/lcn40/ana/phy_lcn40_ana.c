/*
 * LCN40PHY ANAcore contorl module implementation
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
#include <phy_ana.h>
#include "phy_type_ana.h"
#include <phy_lcn40.h>
#include <phy_lcn40_ana.h>

#include <wlc_phyreg_lcn40.h>
#include <phy_utils_reg.h>

/* module private states */
struct phy_lcn40_ana_info {
	phy_info_t *pi;
	phy_lcn40_info_t *lcn40i;
	phy_ana_info_t *ani;
};

/* local functions */
static int phy_lcn40_ana_switch(phy_type_ana_ctx_t *ctx, bool on);

/* Register/unregister LCN40PHY specific implementation to common layer */
phy_lcn40_ana_info_t *
BCMATTACHFN(phy_lcn40_ana_register_impl)(phy_info_t *pi, phy_lcn40_info_t *lcn40i,
	phy_ana_info_t *ani)
{
	phy_lcn40_ana_info_t *info;
	phy_type_ana_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn40_ana_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn40_ana_info_t));
	info->pi = pi;
	info->lcn40i = lcn40i;
	info->ani = ani;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctrl = phy_lcn40_ana_switch;
	fns.ctx = info;

	phy_ana_register_impl(ani, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn40_ana_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn40_ana_unregister_impl)(phy_lcn40_ana_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_ana_info_t *ani = info->ani;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_ana_unregister_impl(ani);

	phy_mfree(pi, info, sizeof(phy_lcn40_ana_info_t));
}

/* switch anacore on/off */
static int
phy_lcn40_ana_switch(phy_type_ana_ctx_t *ctx, bool on)
{
	phy_lcn40_ana_info_t *info = (phy_lcn40_ana_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: %d\n", __FUNCTION__, on));

	/* rxafe needs to be in reset before anacore operation */
	if ((LCN40REV_LE(pi->pubpi.phy_rev, 5)) || (LCN40REV_IS(pi->pubpi.phy_rev, 7)))
		phy_utils_or_phyreg(pi, LCN40PHY_resetCtrl, 0x0047);
	if (on) {
		phy_utils_and_phyreg(pi, LCN40PHY_AfeCtrlOvr1,
			~(LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_dac_pu_ovr_MASK));
	} else  {
		phy_utils_and_phyreg(pi, LCN40PHY_AfeCtrlOvr1Val,
			~(LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_dac_pu_ovr_val_MASK));
		phy_utils_or_phyreg(pi, LCN40PHY_AfeCtrlOvr1,
			LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_dac_pu_ovr_MASK);
	}

	return BCME_OK;
}
