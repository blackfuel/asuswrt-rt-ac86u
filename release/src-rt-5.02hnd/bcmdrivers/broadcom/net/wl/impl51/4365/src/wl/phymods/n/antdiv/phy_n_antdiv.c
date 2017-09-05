/*
 * NPHY ANTennaDIVersity module implementation
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
#include "phy_type_antdiv.h"
#include <phy_n.h>
#include <phy_n_antdiv.h>

#include <wlc_phyreg_n.h>
#include <phy_utils_reg.h>

/* module private states */
struct phy_n_antdiv_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_antdiv_info_t *di;
};

/* local functions */
static void phy_n_antdiv_set_rx(phy_type_antdiv_ctx_t *ctx, uint8 ant);

/* register phy type specific implementation */
phy_n_antdiv_info_t *
BCMATTACHFN(phy_n_antdiv_register_impl)(phy_info_t *pi, phy_n_info_t *ni,
	phy_antdiv_info_t *di)
{
	phy_n_antdiv_info_t *info;
	phy_type_antdiv_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_n_antdiv_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_n_antdiv_info_t));
	info->pi = pi;
	info->ni = ni;
	info->di = di;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.setrx = phy_n_antdiv_set_rx;
	fns.ctx = info;

	phy_antdiv_register_impl(di, &fns);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_n_antdiv_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_antdiv_unregister_impl)(phy_n_antdiv_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_antdiv_info_t *di = info->di;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_antdiv_unregister_impl(di);

	phy_mfree(pi, info, sizeof(phy_n_antdiv_info_t));
}

/* Setup */
static void
phy_n_antdiv_set_rx(phy_type_antdiv_ctx_t *ctx, uint8 ant)
{
	phy_n_antdiv_info_t *info = (phy_n_antdiv_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: ant 0x%x\n", __FUNCTION__, ant));

	if (D11REV_IS(pi->sh->corerev, 16))
		return;

	if (ant > ANT_RX_DIV_FORCE_1)
		wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, MHF1_ANTDIV,
			WLC_BAND_ALL);
	else
		wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, 0, WLC_BAND_ALL);
}
