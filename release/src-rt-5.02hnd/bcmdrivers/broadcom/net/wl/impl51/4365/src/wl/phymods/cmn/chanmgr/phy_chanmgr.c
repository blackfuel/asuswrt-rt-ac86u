/*
 * CHanSPEC manipulation module implementation.
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
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include "phy_type_chanmgr.h"
#include <phy_chanmgr_notif.h>
#include <phy_chanmgr_notif_priv.h>
#include <phy_chanmgr_api.h>
#include <phy_chanmgr.h>

/* forward declaration */
typedef struct phy_chanmgr_mem phy_chanmgr_mem_t;

/* module private states */
struct phy_chanmgr_info {
	phy_info_t				*pi;		/* PHY info ptr */
	phy_type_chanmgr_fns_t	*fns;		/* PHY specific function ptrs */
	phy_chanmgr_mem_t		*mem;		/* Memory layout ptr */
};

/* module private states memory layout */
struct phy_chanmgr_mem {
	phy_chanmgr_info_t		info;
	phy_type_chanmgr_fns_t	fns;
/* add other variable size variables here at the end */
};

/* local function declaration */
static int phy_chanmgr_set_bw(phy_init_ctx_t *ctx);

/* attach/detach */
phy_chanmgr_info_t *
BCMATTACHFN(phy_chanmgr_attach)(phy_info_t *pi)
{
	phy_chanmgr_mem_t	*mem = NULL;
	phy_chanmgr_info_t	*cmn_info = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_chanmgr_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	/* Initialize infra params */
	cmn_info = &(mem->info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* Initialize chanmgr params */

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_chanmgr_set_bw,
		cmn_info, PHY_INIT_CHBW) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register callbacks */

	return cmn_info;

	/* error */
fail:
	phy_chanmgr_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_chanmgr_detach)(phy_chanmgr_info_t *cmn_info)
{
	phy_chanmgr_mem_t *mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	if (!cmn_info)
		return;

	/* Memory associated with cmn_info must be cleaned up. */
	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null chanmgr module\n", __FUNCTION__));
		return;
	}
	phy_mfree(cmn_info->pi, mem, sizeof(phy_chanmgr_mem_t));
}

/* init bandwidth in h/w */
static int
WLBANDINITFN(phy_chanmgr_set_bw)(phy_init_ctx_t *ctx)
{
	phy_chanmgr_info_t *ti = (phy_chanmgr_info_t *)ctx;
	phy_info_t *pi = ti->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* sanitize bw here to avoid later mess. wlc_set_bw will invoke phy_reset,
	 *  but phy_init recursion is avoided by using init_in_progress
	 */
	if (CHSPEC_BW(pi->radio_chanspec) != pi->bw)
		wlapi_bmac_bw_set(pi->sh->physhim, CHSPEC_BW(pi->radio_chanspec));

	return BCME_OK;
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_chanmgr_register_impl)(phy_chanmgr_info_t *ti, phy_type_chanmgr_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));


	*ti->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_chanmgr_unregister_impl)(phy_chanmgr_info_t *ti)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/*
 * Create/Destroy an operating chanspec context for 'chanspec'.
 */
int
phy_chanmgr_create_ctx(phy_info_t *pi, chanspec_t chanspec)
{
	phy_chanmgr_notif_data_t data;

	PHY_TRACE(("%s: chanspec 0x%x\n", __FUNCTION__, chanspec));

	data.event = PHY_CHANMGR_NOTIF_OPCHCTX_OPEN;
	data.new = chanspec;

	return phy_chanmgr_notif_signal(pi->chanmgr_notifi, &data, TRUE);
}

void
phy_chanmgr_destroy_ctx(phy_info_t *pi, chanspec_t chanspec)
{
	phy_chanmgr_notif_data_t data;

	PHY_TRACE(("%s: chanspec 0x%x\n", __FUNCTION__, chanspec));

	data.event = PHY_CHANMGR_NOTIF_OPCHCTX_CLOSE;
	data.new = chanspec;

	(void)phy_chanmgr_notif_signal(pi->chanmgr_notifi, &data, FALSE);
}

/*
 * Use the operating chanspec context associated with the 'chanspec' as
 * the current operating chanspec context.
 *
 * This function changes the current radio chanspec and applies
 * s/w properties to related h/w.
 */
int
phy_chanmgr_set_oper(phy_info_t *pi, chanspec_t chanspec)
{
	phy_chanmgr_notif_data_t data;

	PHY_TRACE(("%s: chanspec 0x%x\n", __FUNCTION__, chanspec));

	data.event = PHY_CHANMGR_NOTIF_OPCH_CHG;
	data.new = chanspec;
	data.old = pi->radio_chanspec;

	return phy_chanmgr_notif_signal(pi->chanmgr_notifi, &data, TRUE);
}

/*
 * Set the radio chanspec to the 'chanspec' and invalidate the current
 * operating chanspec context if any.
 *
 * This function only changes the current radio chanspec.
 */
int
phy_chanmgr_set(phy_info_t *pi, chanspec_t chanspec)
{
	phy_chanmgr_notif_data_t data;

	PHY_TRACE(("%s: chanspec 0x%x\n", __FUNCTION__, chanspec));

	data.event = PHY_CHANMGR_NOTIF_CH_CHG;
	data.new = chanspec;
	data.old = pi->radio_chanspec;

	return phy_chanmgr_notif_signal(pi->chanmgr_notifi, &data, FALSE);
}
