/*
 * RadarDetect module implementation
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
#include <phy_api.h>
#include <phy_init.h>
#include <phy_radar_api.h>
#include "phy_type_radar.h"
#include "phy_radar_st.h"
#include "phy_radar_shared.h"
#include <phy_radar.h>

/* module private states */
struct phy_radar_info {
	phy_info_t *pi;
	phy_radar_st_t *st;		/* states pointer, also as "enable" flag */
	phy_type_radar_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_radar_info_t info;
	phy_type_radar_fns_t fns;
/* add other variable size variables here at the end */
} phy_radar_mem_t;

/* local function declaration */
static int _phy_radar_init(phy_init_ctx_t *ctx);

/* attach/detach */
phy_radar_info_t *
BCMATTACHFN(phy_radar_attach)(phy_info_t *pi)
{
	phy_radar_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_radar_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->fns = &((phy_radar_mem_t *)info)->fns;

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, _phy_radar_init, info, PHY_INIT_RADAR) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	return info;

	/* error */
fail:
	phy_radar_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_radar_detach)(phy_radar_info_t *ri)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ri == NULL) {
		PHY_INFORM(("%s: null radar module\n", __FUNCTION__));
		return;
	}

	pi = ri->pi;

	phy_mfree(pi, ri, sizeof(phy_radar_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_radar_register_impl)(phy_radar_info_t *ri, phy_type_radar_fns_t *fns)
{
	phy_info_t *pi = ri->pi;
	int ref_count = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));


	/* ===================== */
	/* INIT THE RADAR STATES */
	/* ===================== */

	/* OBJECT REGISTRY: check if shared key has value already stored */
	ri->st = (phy_radar_st_t *)wlapi_obj_registry_get(pi->sh->physhim, OBJR_PHY_CMN_RADAR_INFO);

	if (ri->st == NULL) {
		if ((ri->st = phy_malloc(pi, sizeof(phy_radar_st_t))) == NULL) {
			PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
			return BCME_NOMEM;
		}

		/* OBJECT REGISTRY: We are the first instance, store value for key */
		wlapi_obj_registry_set(pi->sh->physhim, OBJR_PHY_CMN_RADAR_INFO, ri->st);

		PHY_INFORM(("wl%d: %s: ptr(ri->st) = %p | sizeof(phy_radar_st_t) = %d\n",
			pi->sh->unit, __FUNCTION__, ri->st, (int)sizeof(phy_radar_st_t)));
	}
	/* OBJECT REGISTRY: Reference the stored value in both instances */
	ref_count = wlapi_obj_registry_ref(pi->sh->physhim, OBJR_PHY_CMN_RADAR_INFO);
	ASSERT(ref_count <= MAX_RSDB_MAC_NUM);
	BCM_REFERENCE(ref_count);

	*ri->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_radar_unregister_impl)(phy_radar_info_t *ri)
{
	phy_info_t *pi = ri->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ri->st == NULL)
		return;

	if (wlapi_obj_registry_unref(pi->sh->physhim, OBJR_PHY_CMN_RADAR_INFO) == 0) {
		wlapi_obj_registry_set(pi->sh->physhim, OBJR_PHY_CMN_RADAR_INFO, NULL);
		phy_mfree(pi, ri->st, sizeof(phy_radar_st_t));
		ri->st = NULL;
	}
}

/* return radar state structure to PHY type specific implementation */
phy_radar_st_t *
phy_radar_get_st(phy_radar_info_t *ri)
{
	return ri->st;
}

/* init h/w */
static int
phy_radar_init(phy_radar_info_t *ri, bool init)
{
#ifdef BAND5G
	phy_type_radar_fns_t *fns = ri->fns;
	phy_info_t *pi = ri->pi;
#endif

	PHY_TRACE(("%s: init %d\n", __FUNCTION__, init));

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		if (fns->init != NULL)
			return (fns->init)(fns->ctx, init);
	}
#endif

	return BCME_OK;
}

static int
WLBANDINITFN(_phy_radar_init)(phy_init_ctx_t *ctx)
{
	phy_radar_info_t *ri = (phy_radar_info_t *)ctx;
	phy_info_t *pi = ri->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	return phy_radar_init(ri, pi->sh->radar);
}

/* update h/w */
void
phy_radar_upd(phy_radar_info_t *ri)
{
	phy_type_radar_fns_t *fns = ri->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->update == NULL)
		return;

	(fns->update)(fns->ctx);
}

/* public APIs */
void
phy_radar_detect_enable(phy_info_t *pi, bool enab)
{
	phy_radar_info_t *ri = pi->radari;

	PHY_TRACE(("%s: enable %d\n", __FUNCTION__, enab));

	pi->sh->radar = enab;

	if (!pi->sh->up)
		return;

	phy_radar_init(ri, enab);
}

int
phy_radar_detect_run(phy_info_t *pi, int PLL_idx, int BW80_80_mode)
{
	phy_radar_info_t *ri = pi->radari;
	phy_type_radar_fns_t *fns = ri->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->run == NULL)
		return BCME_OK;

	return (fns->run)(fns->ctx, PLL_idx, BW80_80_mode);
}

void
phy_radar_detect_mode_set(phy_info_t *pi, phy_radar_detect_mode_t mode)
{
	phy_radar_info_t *ri = pi->radari;
	phy_type_radar_fns_t *fns = ri->fns;
	phy_radar_st_t *st = ri->st;

	PHY_TRACE(("wl%d: %s, Radar detect mode set done\n", pi->sh->unit, __FUNCTION__));

	if (mode != RADAR_DETECT_MODE_FCC && mode != RADAR_DETECT_MODE_EU) {
		PHY_TRACE(("wl%d: bogus radar detect mode: %d\n", pi->sh->unit, mode));
		return;
	}

	if (st == NULL) {
		PHY_ERROR(("%s: not supported\n", __FUNCTION__));
		return;
	}

	if (st->rdm == mode) {
		PHY_TRACE(("wl%d: radar detect mode unchanged: %d\n", pi->sh->unit, mode));
		return;
	}

	st->rdm = mode;

	/* Change radar params based on radar detect mode for
	 * both 20Mhz (index 0) and 40Mhz (index 1) aptly
	 * feature_mask bit-11 is FCC-enable
	 * feature_mask bit-12 is EU-enable
	 */
	if (mode == RADAR_DETECT_MODE_FCC) {
		st->rparams.radar_args.feature_mask =
			(st->rparams.radar_args.feature_mask & 0xefff) | 0x800;
	}
	else if (mode == RADAR_DETECT_MODE_EU) {
		st->rparams.radar_args.feature_mask =
			(st->rparams.radar_args.feature_mask & 0xf7ff) | 0x1000;
	}

	if (fns->mode == NULL)
		return;

	(fns->mode)(fns->ctx, mode);
}

void
phy_radar_first_indicator_set(phy_radar_info_t *ri)
{
	phy_radar_st_t *st = ri->st;

	if (st == NULL)
		return;

	/* indicate first time radar detection */
	st->first_radar_indicator = 1;
	st->first_radar_indicator_sc = 1;
}
