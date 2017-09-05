/*
 * LCNPHY TxPowerCtrl module implementation
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
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_tpc.h>
#include "phy_tpc_shared.h"
#include "phy_type_tpc.h"
#include <phy_lcn.h>
#include <phy_lcn_tpc.h>

#include <phy_utils_reg.h>
#include <wlc_phyreg_lcn.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn.h>
/* TODO: all these are going away... > */
#endif

/* module private states */
struct phy_lcn_tpc_info {
	phy_info_t *pi;
	phy_lcn_info_t *lcni;
	phy_tpc_info_t *ti;
};

#define wlc_lcnphy_set_target_tx_pwr(pi, target) \
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlTargetPwr, targetPwr0, \
		(uint16)MAX(pi->u.pi_lcnphy->tssi_minpwr_limit, \
		(MIN(pi->u.pi_lcnphy->tssi_maxpwr_limit, \
		(uint16)(target)))))

/* local functions */
static void phy_lcn_tpc_recalc_tgt(phy_type_tpc_ctx_t *ctx);
static int phy_lcn_tpc_read_srom(phy_type_tpc_ctx_t *ctx, int bandtype);
static void phy_lcn_tpc_check_limit(phy_type_tpc_ctx_t *ctx);

/* Register/unregister LCNPHY specific implementation to common layer. */
phy_lcn_tpc_info_t *
BCMATTACHFN(phy_lcn_tpc_register_impl)(phy_info_t *pi, phy_lcn_info_t *lcni, phy_tpc_info_t *ti)
{
	phy_lcn_tpc_info_t *info;
	phy_type_tpc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn_tpc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn_tpc_info_t));
	info->pi = pi;
	info->lcni = lcni;
	info->ti = ti;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.recalc = phy_lcn_tpc_recalc_tgt;
	fns.read_srom = phy_lcn_tpc_read_srom;
	fns.check = phy_lcn_tpc_check_limit;
	fns.ctx = info;

	phy_tpc_register_impl(ti, &fns);

	return info;

fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn_tpc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn_tpc_unregister_impl)(phy_lcn_tpc_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_tpc_info_t *ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_tpc_unregister_impl(ti);

	phy_mfree(pi, info, sizeof(phy_lcn_tpc_info_t));
}

/* recalc target txpwr and apply to h/w */
static void
phy_lcn_tpc_recalc_tgt(phy_type_tpc_ctx_t *ctx)
{
	phy_lcn_tpc_info_t *info = (phy_lcn_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	wlc_phy_txpower_recalc_target_lcnphy(pi);
}

static int
BCMATTACHFN(phy_lcn_tpc_read_srom)(phy_type_tpc_ctx_t *ctx, int bandtype)
{
	phy_lcn_tpc_info_t *info = (phy_lcn_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	return wlc_phy_txpwr_srom_read_lcnphy(pi, bandtype);
}

/* recalc target txpwr and apply to h/w */
static void
phy_lcn_tpc_check_limit(phy_type_tpc_ctx_t *ctx)
{
	phy_lcn_tpc_info_t *info = (phy_lcn_tpc_info_t *)ctx;
	phy_lcn_info_t *lcni = info->lcni;
	phy_info_t *pi = info->pi;
	int8 txpwrindex;
	int32 margin_qdBm = 4;
	int32 tssi_maxpwr_limit, tssi_minpwr_limit;
	uint8 wait_for_pwrctrl = 5;
	int32 a1 = 0, b0 = 0, b1 = 0;
	uint16 avgTssi_2C, idleTssi0_2C, idleTssi0_OB, avgTssi_OB, adjTssi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (!lcni->dynamic_pwr_limit_en)
		return;

	txpwrindex = wlc_lcnphy_get_current_tx_pwr_idx(pi);

	if ((txpwrindex != 0) && (txpwrindex != 127))
		return;

	wlc_phy_get_paparams_for_band(pi, &a1, &b0, &b1);

	avgTssi_2C = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusNew4, avgTssi);
	idleTssi0_2C = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlIdleTssi, idleTssi0);

	if (idleTssi0_2C >= 256)
		idleTssi0_OB = idleTssi0_2C - 256;
	else
		idleTssi0_OB = idleTssi0_2C + 256;

	if (avgTssi_2C >= 256)
		avgTssi_OB = avgTssi_2C - 256;
	else
		avgTssi_OB = avgTssi_2C + 256;

	adjTssi = (avgTssi_OB + (511-idleTssi0_OB))>>2;

	if (txpwrindex == 0) {
		lcni->idx0cnt++;
		if (lcni->idx0cnt < wait_for_pwrctrl)
			return;
		lcni->idx0cnt = 0;
		tssi_maxpwr_limit = (wlc_lcnphy_tssi2dbm(adjTssi, a1, b0, b1) >> 1) - margin_qdBm;
		if (tssi_maxpwr_limit >= lcni->tssi_minpwr_limit)
			lcni->tssi_maxpwr_limit = tssi_maxpwr_limit;
		else
			lcni->tssi_maxpwr_limit = lcni->tssi_minpwr_limit;
		wlc_lcnphy_set_target_tx_pwr(pi, lcni->tssi_maxpwr_limit);

	} else if (txpwrindex == 127) {
		lcni->idx127cnt++;
		if (lcni->idx127cnt < wait_for_pwrctrl)
			return;
		lcni->idx127cnt = 0;
		tssi_minpwr_limit = (wlc_lcnphy_tssi2dbm(adjTssi, a1, b0, b1) >> 1) + margin_qdBm;
		if (tssi_minpwr_limit <= lcni->tssi_maxpwr_limit)
			lcni->tssi_minpwr_limit = tssi_minpwr_limit;
		else
			lcni->tssi_minpwr_limit = lcni->tssi_maxpwr_limit;
		wlc_lcnphy_set_target_tx_pwr(pi, lcni->tssi_minpwr_limit);
	}
}
