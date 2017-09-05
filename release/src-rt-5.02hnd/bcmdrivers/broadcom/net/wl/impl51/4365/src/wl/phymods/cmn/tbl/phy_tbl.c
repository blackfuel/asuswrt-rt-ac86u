/*
 * PHYTableInit module implementation
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
#include "phy_type_tbl.h"
#include <phy_init.h>
#include <phy_tbl.h>
#include <phy_utils_reg.h>

/* module private states */
struct phy_tbl_info {
	phy_info_t *pi;
	phy_type_tbl_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_tbl_info_t info;
	phy_type_tbl_fns_t fns;
} phy_tbl_mem_t;

/* local function declaration */
static int phy_tbl_init(phy_init_ctx_t *ctx);
#ifndef BCMNODOWN
static int phy_tbl_down(phy_init_ctx_t *ctx);
#endif
#if defined(BCMDBG_PHYDUMP)
static int phy_tbl_dump1(void *ctx, struct bcmstrbuf *b);
static int phy_tbl_dump2(void *ctx, struct bcmstrbuf *b);
static int phy_tbl_dump3(void *ctx, struct bcmstrbuf *b);
static int phy_tbl_dump4(void *ctx, struct bcmstrbuf *b);
#endif 

/* attach/detach */
phy_tbl_info_t *
BCMATTACHFN(phy_tbl_attach)(phy_info_t *pi)
{
	phy_tbl_info_t *info;
	phy_type_tbl_fns_t *fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_tbl_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	/* init the type specific fns */
	fns = &((phy_tbl_mem_t *)info)->fns;
	info->fns = fns;

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_tbl_init, info, PHY_INIT_PHYTBL) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

#ifndef BCMNODOWN
	/* register down fn */
	if (phy_init_add_down_fn(pi->initi, phy_tbl_down, info, PHY_DOWN_PHYTBL) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}
#endif

	/* register dump callback */
#if defined(BCMDBG_PHYDUMP)
	phy_dbg_add_dump_fn(pi, "phytbl1", (phy_dump_fn_t)phy_tbl_dump1, info);
	phy_dbg_add_dump_fn(pi, "phytbl2", (phy_dump_fn_t)phy_tbl_dump2, info);
	phy_dbg_add_dump_fn(pi, "phytbl3", (phy_dump_fn_t)phy_tbl_dump3, info);
	phy_dbg_add_dump_fn(pi, "phytbl4", (phy_dump_fn_t)phy_tbl_dump4, info);
#endif 

	return info;

	/* error */
fail:
	phy_tbl_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_tbl_detach)(phy_tbl_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null init module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_tbl_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_tbl_register_impl)(phy_tbl_info_t *ii, phy_type_tbl_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));


	*ii->fns = *fns;
	return BCME_OK;
}

void
BCMATTACHFN(phy_tbl_unregister_impl)(phy_tbl_info_t *ii)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* init PHY tables */
static int
WLBANDINITFN(phy_tbl_init)(phy_init_ctx_t *ctx)
{
	phy_tbl_info_t *ii = (phy_tbl_info_t *)ctx;
	phy_type_tbl_fns_t *fns = ii->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(fns->init != NULL);
	return (fns->init)(fns->ctx);
}

#ifndef BCMNODOWN
/* down the h/w */
static int
BCMUNINITFN(phy_tbl_down)(phy_init_ctx_t *ctx)
{
	phy_tbl_info_t *ii = (phy_tbl_info_t *)ctx;
	phy_type_tbl_fns_t *fns = ii->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->down == NULL)
		return 0;

	return (fns->down)(fns->ctx);
}
#endif

#if defined(BCMDBG_PHYDUMP)
static int
phy_tbl_dumptbl(phy_tbl_info_t *info, int idx, struct bcmstrbuf *b)
{
	phy_type_tbl_fns_t *fns = info->fns;
	phy_info_t *pi = info->pi;
	int ret = BCME_UNSUPPORTED;

	if (!pi->sh->clk)
		return BCME_NOCLK;

	if (fns->dump[idx]) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		ret = (fns->dump[idx])(fns->ctx, b);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
	}

	return ret;
}

static int
phy_tbl_dump1(void *ctx, struct bcmstrbuf *b)
{
	phy_tbl_info_t *info = (phy_tbl_info_t *)ctx;

	return phy_tbl_dumptbl(info, 0, b);
}

static int
phy_tbl_dump2(void *ctx, struct bcmstrbuf *b)
{
	phy_tbl_info_t *info = (phy_tbl_info_t *)ctx;

	return phy_tbl_dumptbl(info, 1, b);
}

static int
phy_tbl_dump3(void *ctx, struct bcmstrbuf *b)
{
	phy_tbl_info_t *info = (phy_tbl_info_t *)ctx;

	return phy_tbl_dumptbl(info, 2, b);
}

static int
phy_tbl_dump4(void *ctx, struct bcmstrbuf *b)
{
	phy_tbl_info_t *info = (phy_tbl_info_t *)ctx;

	return phy_tbl_dumptbl(info, 3, b);
}

void
phy_tbl_do_dumptbl(phy_tbl_info_t *info, phy_table_info_t *ti, struct bcmstrbuf *b)
{
	phy_type_tbl_fns_t *fns = info->fns;
	phy_info_t *pi = info->pi;
	uint16 qval[3];
	uint16 addr, val = 0;

	qval[0] = 0;
	qval[1] = 0;
	qval[2] = 0;

	ASSERT(ti != NULL);
	while (ti->table != 0xff) {
#if defined(WLTEST)
		if (pi->nphy_tbldump_minidx >= 0) {
			if (ti->table < (uint) pi->nphy_tbldump_minidx) {
				ti++;
				continue;
			}
		}

		if (pi->nphy_tbldump_maxidx >= 0) {
			if (ti->table > (uint) pi->nphy_tbldump_maxidx) {
				break;
			}
		}
#endif 

		if (fns->tblfltr != NULL &&
		    !(fns->tblfltr)(pi, ti)) {
			ti++;
			continue;
		}

		for (addr = 0; addr < ti->max; addr++) {
			if (fns->addrfltr != NULL &&
			    !(fns->addrfltr)(pi, ti, addr)) {
				continue;
			}

			ASSERT(fns->readtbl != NULL);
			(fns->readtbl)(pi, ti, addr, &val, &qval[0]);

			if (PHY_INFORM_ON() && si_taclear(pi->sh->sih, FALSE)) {
				PHY_INFORM(("%s: TA reading aphy table 0x%x:0x%x\n", __FUNCTION__,
				            ti->table, addr));
				bcm_bprintf(b, "0x%x: 0x%03x tabort\n", ti->table, addr);
			} else if (!si_taclear(pi->sh->sih, FALSE)) {
				if (ti->q == 1) {
					bcm_bprintf(b, "0x%x: 0x%03x 0x%04x 0x%04x\n", ti->table,
					            addr, val, qval[0]);
			    } else if (ti->q == 2) {
					bcm_bprintf(b, "0x%x: 0x%03x 0x%04x 0x%04x 0x%04x\n",
					    ti->table, addr, val, qval[1], qval[0]);
			    } else if (ti->q == 3) {
					bcm_bprintf(b, "0x%x: 0x%03x 0x%04x 0x%04x 0x%04x 0x%04x\n",
					     ti->table, addr, val, qval[2], qval[1], qval[0]);
				} else {
					bcm_bprintf(b, "0x%x: 0x%03x 0x%04x\n", ti->table, addr,
					            val);
				}
			}
		}
		ti++;
	}
}
#endif 
