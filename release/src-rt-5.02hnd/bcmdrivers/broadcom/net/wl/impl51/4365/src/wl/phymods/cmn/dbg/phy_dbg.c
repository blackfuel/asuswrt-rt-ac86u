/*
 * PHY module debug utilities
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
#include <wlc_dump_reg.h>
#include <phy_mem.h>
#include <phy_dbg_api.h>
#include <phy_dbg.h>

/* module private states */
struct phy_dump_info {
	phy_info_t *pi;
	wlc_dump_reg_info_t *dump;
};

/* default registries sizes */
#ifndef PHY_DUMP_REG_SZ
#define PHY_DUMP_REG_SZ 32
#endif

#if defined(BCMDBG_ERR) || defined(BCMDBG)
uint32 phyhal_msg_level = PHYHAL_ERROR;
#else
uint32 phyhal_msg_level = 0;
#endif

/* accessor functions */
uint32
BCMRAMFN(get_phyhal_msg_level)(void)
{
	return phyhal_msg_level;
}

void
BCMRAMFN(set_phyhal_msg_level)(uint32 sval)
{
	phyhal_msg_level = sval;
}

/* module attach/detach */
phy_dump_info_t *
BCMATTACHFN(phy_dump_attach)(phy_info_t *pi)
{
	phy_dump_info_t *di;

	if ((di = phy_malloc(pi, sizeof(phy_dump_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	di->pi = pi;
	di->dump = wlc_dump_reg_create(pi->sh->osh, PHY_DUMP_REG_SZ);
	if (di->dump == NULL) {
		PHY_ERROR(("%s: wlc_dump_reg_create failed\n", __FUNCTION__));
		goto fail;
	}

	return di;

fail:
	phy_dump_detach(di);
	return NULL;
}

void
BCMATTACHFN(phy_dump_detach)(phy_dump_info_t *di)
{
	phy_info_t *pi;

	if (di == NULL)
		return;

	pi = di->pi;

	if (di->dump)
		wlc_dump_reg_destroy(pi->sh->osh, di->dump);

	phy_mfree(pi, di, sizeof(phy_dump_info_t));
}

/* add a dump fn */
int
phy_dbg_add_dump_fn(phy_info_t *pi, char *name, phy_dump_fn_t fn, void *ctx)
{
	phy_dump_info_t *di = pi->dumpi;

	return wlc_dump_reg_add_fn(di->dump, name, fn, ctx);
}

/*
 * invoke dump function for name.
 */
int phy_dbg_dump(phy_info_t *pi, char *name, struct bcmstrbuf *b)
{
	phy_dump_info_t *di = pi->dumpi;

	return wlc_dump_reg_invoke_fn(di->dump, name, b);
}
