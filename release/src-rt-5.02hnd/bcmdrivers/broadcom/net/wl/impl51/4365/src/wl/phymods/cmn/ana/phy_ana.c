/*
 * ANAcore module implementation.
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

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include "phy_type_ana.h"
#include <phy_ana_api.h>
#include <phy_ana.h>

/* module private states */
struct phy_ana_info {
	phy_info_t *pi;
	phy_type_ana_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_ana_info_t info;
	phy_type_ana_fns_t fns;
/* add other variable size variables here at the end */
} phy_ana_mem_t;

/* local function declaration */
static int phy_ana_init(phy_init_ctx_t *ctx);

/* attach/detach */
phy_ana_info_t *
BCMATTACHFN(phy_ana_attach)(phy_info_t *pi)
{
	phy_ana_info_t *info;
	phy_type_ana_fns_t *fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_ana_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	fns = &((phy_ana_mem_t *)info)->fns;
	info->fns = fns;

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_ana_init, info, PHY_INIT_ANA) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register callbacks */

	return info;

	/* error */
fail:
	phy_ana_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_ana_detach)(phy_ana_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null ana module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_ana_mem_t));
}

/* switch on/off anacore */
void
phy_ana_switch(phy_info_t *pi, bool on)
{
	phy_ana_info_t *ani = pi->anai;
	phy_type_ana_fns_t *fns = ani->fns;

	PHY_TRACE(("%s: %d\n", __FUNCTION__, on));

	ASSERT(fns->ctrl != NULL);
	(fns->ctrl)(fns->ctx, on);
}

/* reset anacore */
void
phy_ana_reset(phy_info_t *pi)
{
	phy_ana_info_t *ani = pi->anai;
	phy_type_ana_fns_t *fns = ani->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->reset == NULL)
		return;

	(fns->reset)(fns->ctx);
}

static int
phy_ana_init(phy_init_ctx_t *ctx)
{
	phy_ana_info_t *ani = (phy_ana_info_t *)ctx;
	phy_info_t *pi = ani->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_ana_switch(pi, ON);
	return BCME_OK;
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_ana_register_impl)(phy_ana_info_t *ani, phy_type_ana_fns_t *fns)
{

	PHY_TRACE(("%s\n", __FUNCTION__));

	*ani->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_ana_unregister_impl)(phy_ana_info_t *ani)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}
