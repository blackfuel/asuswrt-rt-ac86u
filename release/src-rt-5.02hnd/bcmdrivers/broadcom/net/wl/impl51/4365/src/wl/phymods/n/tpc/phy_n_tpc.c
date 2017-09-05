/*
 * NPHY TxPowerCtrl module implementation
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
#include "phy_type_tpc.h"
#include <phy_n.h>
#include <phy_n_tpc.h>

#include <wlc_phyreg_n.h>
#include <phy_utils_reg.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
/* TODO: all these are going away... > */
#endif

/* module private states */
struct phy_n_tpc_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_tpc_info_t *ti;
};

/* local functions */
static void phy_n_tpc_recalc_tgt(phy_type_tpc_ctx_t *ctx);
static int phy_n_tpc_read_srom(phy_type_tpc_ctx_t *ctx, int bandtype);

/* Register/unregister NPHY specific implementation to common layer */
phy_n_tpc_info_t *
BCMATTACHFN(phy_n_tpc_register_impl)(phy_info_t *pi, phy_n_info_t *ni, phy_tpc_info_t *ti)
{
	phy_n_tpc_info_t *info;
	phy_type_tpc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_n_tpc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_n_tpc_info_t));
	info->pi = pi;
	info->ni = ni;
	info->ti = ti;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.recalc = phy_n_tpc_recalc_tgt;
	fns.read_srom = phy_n_tpc_read_srom;
	fns.ctx = info;

	phy_tpc_register_impl(ti, &fns);

	return info;

fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_n_tpc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_tpc_unregister_impl)(phy_n_tpc_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_tpc_info_t *ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_tpc_unregister_impl(ti);

	phy_mfree(pi, info, sizeof(phy_n_tpc_info_t));
}

/* recalc target txpwr and apply to h/w */
static void
phy_n_tpc_recalc_tgt(phy_type_tpc_ctx_t *ctx)
{
	phy_n_tpc_info_t *info = (phy_n_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	wlc_phy_txpower_recalc_target_nphy(pi);
}

/* read srom txpwr limits */
static int
BCMATTACHFN(phy_n_tpc_read_srom)(phy_type_tpc_ctx_t *ctx, int bandtype)
{
	phy_n_tpc_info_t *info = (phy_n_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	if (pi->sh->sromrev >= 9)
		return wlc_phy_txpwr_srom9_read(pi) ? BCME_OK : BCME_ERROR;
#ifndef WLC_DISABLE_SROM8
	else
		return wlc_phy_txpwr_srom8_read(pi) ? BCME_OK : BCME_ERROR;
#endif
	return BCME_ERROR;
}
