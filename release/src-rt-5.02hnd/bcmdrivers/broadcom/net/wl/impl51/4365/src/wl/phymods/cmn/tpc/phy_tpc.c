/*
 * TxPowerCtrl module implementation.
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
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include <phy_rstr.h>
#include "phy_type_tpc.h"
#include <phy_tpc.h>

#include <phy_utils_var.h>

/* module private states */
struct phy_tpc_info {
	phy_info_t *pi;
	phy_type_tpc_fns_t *fns;

	bool user_txpwr_at_rfport;
	uint8 ucode_tssi_limit_en;
};

/* module private states memory layout */
typedef struct {
	phy_tpc_info_t info;
	phy_type_tpc_fns_t fns;
/* add other variable size variables here at the end */
} phy_tpc_mem_t;

/* local function declaration */

/* attach/detach */
phy_tpc_info_t *
BCMATTACHFN(phy_tpc_attach)(phy_info_t *pi)
{
	phy_tpc_info_t *info;
	phy_type_tpc_fns_t *fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_tpc_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	fns = &((phy_tpc_mem_t *)info)->fns;
	info->fns = fns;

	/* user specified power is at the ant port */
#ifdef WLNOKIA_NVMEM
	info->user_txpwr_at_rfport = TRUE;
#else
	info->user_txpwr_at_rfport = FALSE;
#endif

	info->ucode_tssi_limit_en = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssilimucod, 1);

	/* Register callbacks */

	return info;

	/* error */
fail:
	phy_tpc_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_tpc_detach)(phy_tpc_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null tpc module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_tpc_mem_t));
}

/* recalc target txpwr and apply to h/w */
void
phy_tpc_recalc_tgt(phy_tpc_info_t *ti)
{
	phy_type_tpc_fns_t *fns = ti->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(fns->recalc != NULL);
	(fns->recalc)(fns->ctx);
}

/* read SROM for the given bandtype */
int
BCMATTACHFN(phy_tpc_read_srom)(phy_tpc_info_t *ti, int bandtype)
{
	phy_type_tpc_fns_t *fns = ti->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->read_srom == NULL)
		return BCME_OK;

	return (fns->read_srom)(fns->ctx, bandtype);
}

/* check limit? */
void
phy_tpc_check_limit(phy_info_t *pi)
{
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (!ti->ucode_tssi_limit_en)
		return;

	if (fns->check == NULL)
		return;

	(fns->check)(fns->ctx);
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_tpc_register_impl)(phy_tpc_info_t *ti, phy_type_tpc_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));


	*ti->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_tpc_unregister_impl)(phy_tpc_info_t *ti)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}
