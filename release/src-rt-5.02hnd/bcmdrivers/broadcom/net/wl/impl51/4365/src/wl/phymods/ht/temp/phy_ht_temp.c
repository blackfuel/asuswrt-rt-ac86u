/*
 * HTPHY TEMPerature sense module implementation
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
#include "phy_type_temp.h"
#include "phy_temp_st.h"
#include <phy_ht.h>
#include <phy_ht_temp.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_ht.h>
/* TODO: all these are going away... > */
#endif

/* module private states */
struct phy_ht_temp_info {
	phy_info_t *pi;
	phy_ht_info_t *hti;
	phy_temp_info_t *ti;
};

/* local functions */
static uint8 phy_ht_temp_throttle(phy_type_temp_ctx_t *ctx);
static int phy_ht_temp_get(phy_type_temp_ctx_t *ctx);

/* Register/unregister HTPHY specific implementation to common layer */
phy_ht_temp_info_t *
BCMATTACHFN(phy_ht_temp_register_impl)(phy_info_t *pi, phy_ht_info_t *hti, phy_temp_info_t *ti)
{
	phy_ht_temp_info_t *info;
	phy_type_temp_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ht_temp_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_ht_temp_info_t));
	info->pi = pi;
	info->hti = hti;
	info->ti = ti;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.throt = phy_ht_temp_throttle;
	fns.get = phy_ht_temp_get;
	fns.ctx = info;

	phy_temp_register_impl(ti, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ht_temp_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ht_temp_unregister_impl)(phy_ht_temp_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_temp_info_t *ti = info->ti;


	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_temp_unregister_impl(ti);

	phy_mfree(pi, info, sizeof(phy_ht_temp_info_t));
}

static uint8
phy_ht_temp_throttle(phy_type_temp_ctx_t *ctx)
{
	phy_ht_temp_info_t *info = (phy_ht_temp_info_t *)ctx;
	phy_temp_info_t *ti = info->ti;
	phy_info_t *pi = info->pi;
	phy_txcore_temp_t *temp;
	uint8 txcore_shutdown_lut[] = {1, 1, 2, 1, 4, 1, 2, 5};
	uint8 phyrxchain = pi->sh->phyrxchain;
	uint8 phytxchain = pi->sh->phytxchain;
	uint8 new_phytxchain;
	int16 currtemp;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(phytxchain);

	temp = phy_temp_get_st(ti);
	ASSERT(temp != NULL);

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	currtemp = wlc_phy_tempsense_htphy(pi);
	wlapi_enable_mac(pi->sh->physhim);

#ifdef BCMDBG
	if (pi->tempsense_override)
		currtemp = pi->tempsense_override;
#endif

	if (!temp->heatedup) {
		if (currtemp >= temp->disable_temp) {
			new_phytxchain = txcore_shutdown_lut[phytxchain];
			temp->heatedup = TRUE;
			temp->bitmap = ((phyrxchain << 4) | new_phytxchain);
		}
	} else {
		if (currtemp <= temp->enable_temp) {
			new_phytxchain = pi->sh->hw_phytxchain;
			temp->heatedup = FALSE;
			temp->bitmap = ((phyrxchain << 4) | new_phytxchain);
		}
	}

	return temp->bitmap;
}

/* read the current temperature */
static int
phy_ht_temp_get(phy_type_temp_ctx_t *ctx)
{
	phy_ht_temp_info_t *info = (phy_ht_temp_info_t *)ctx;
	phy_ht_info_t *hti = info->hti;

	return hti->current_temperature;
}
