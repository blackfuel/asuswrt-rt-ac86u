/*
 * TEMPerature sense module implementation.
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
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_rstr.h>
#include "phy_type_temp.h"
#include "phy_temp_st.h"
#include <phy_temp.h>

#include <phy_utils_var.h>

/* module private states */
struct phy_temp_info {
	phy_info_t *pi;
	phy_type_temp_fns_t *fns;
	/* tempsense */
	phy_txcore_temp_t *temp;
};

/* module private states memory layout */
typedef struct {
	phy_temp_info_t info;
	phy_type_temp_fns_t fns;
	phy_txcore_temp_t temp;
/* add other variable size variables here at the end */
} phy_temp_mem_t;

/* local function declaration */

/* attach/detach */
phy_temp_info_t *
BCMATTACHFN(phy_temp_attach)(phy_info_t *pi)
{
	phy_temp_info_t *info;
	phy_txcore_temp_t *temp;
	uint8 init_txrxchain;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_temp_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->fns = &((phy_temp_mem_t *)info)->fns;

	/* Parameters for temperature-based fallback to 1-Tx chain */
	temp = &((phy_temp_mem_t *)info)->temp;
	info->temp = temp;

	init_txrxchain = (1 << PHYCORENUM(pi->pubpi.phy_corenum)) - 1;

	temp->disable_temp = (uint8)PHY_GETINTVAR(pi, rstr_tempthresh);
	if ((temp->disable_temp == 0) || (temp->disable_temp == 0xff)) {
		if (ISHTPHY(pi)) {
			temp->disable_temp = HTPHY_CHAIN_TX_DISABLE_TEMP;
		} else if (ISACPHY(pi)) {
			temp->disable_temp = ACPHY_CHAIN_TX_DISABLE_TEMP;
		} else {
			temp->disable_temp = PHY_CHAIN_TX_DISABLE_TEMP;
		}
	}

#if defined(BCM94360X51) && defined(BCM94360X52C)
	if (ISACPHY(pi) &&
	    (CHIPID(pi->sh->chip) == BCM4360_CHIP_ID) &&
	    ((pi->sh->boardtype == BCM94360X51) ||
	     (pi->sh->boardtype == BCM94360X51P3) ||
	     (pi->sh->boardtype == BCM94360X52C))) {
		temp->disable_temp = 120;
	}
#endif /* BCM94360X51 && BCM94360X52C */

	temp->disable_temp_max_cap = temp->disable_temp;

	temp->hysteresis = (uint8)PHY_GETINTVAR(pi, rstr_temps_hysteresis);
	if ((temp->hysteresis == 0) || (temp->hysteresis == 0xf)) {
		temp->hysteresis = PHY_HYSTERESIS_DELTATEMP;
	}

	temp->enable_temp =
		temp->disable_temp - temp->hysteresis;

	temp->heatedup = FALSE;
	temp->degrade1RXen = FALSE;

	temp->bitmap = (init_txrxchain << 4 | init_txrxchain);

	pi->phy_tempsense_offset = 0;

	/* Register callbacks */

	return info;

	/* error */
fail:
	phy_temp_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_temp_detach)(phy_temp_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null temp module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_temp_mem_t));
}

/*
 * Query the states pointer.
 */
phy_txcore_temp_t *
phy_temp_get_st(phy_temp_info_t *ti)
{
	return ti->temp;
}

/* temp. throttle */
uint8
phy_temp_throttle(phy_temp_info_t *ti)
{
	phy_type_temp_fns_t *fns = ti->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->throt == NULL)
		return 0;

	return (fns->throt)(fns->ctx);
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_temp_register_impl)(phy_temp_info_t *ti, phy_type_temp_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));


	*ti->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_temp_unregister_impl)(phy_temp_info_t *ti)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

#ifdef	WL_DYNAMIC_TEMPSENSE
#if defined(BCMDBG) || defined(WLTEST)
int
phy_temp_get_override(phy_temp_info_t *ti)
{
	phy_info_t *pi = ti->pi;

	return  pi->tempsense_override;
}
#endif /* BCMDBG || WLTEST */

int
phy_temp_get_thresh(phy_temp_info_t *ti)
{
	phy_txcore_temp_t *temp = ti->temp;

	return temp->disable_temp;
}

/*
 * Move this to wlc_phy_cmn.c and do it based on PHY
 * This function DOES NOT calculate temperature and it also DOES NOT
 * read any register from MAC and/or RADIO. This function returns
 * last recorded temperature that is stored in phy_info_t.
 * There are few types of phy that DO NOT record last temperature
 * but derives it based on various other mechnism. This function
 * returns BCME_RANGE for such type of PHY. So that the code would
 * continue as per old fashioned tempsense mechanism at every 10S.
 */
int
phy_temp_get_cur_temp(phy_temp_info_t *ti)
{
	phy_txcore_temp_t *temp = ti->temp;
	phy_type_temp_fns_t *fns = ti->fns;
	int ct = BCME_RANGE;

	if (temp->heatedup) {
		return BCME_RANGE; /* Indicate we are already heated up */
	}

	if (fns->get != NULL)
		ct = (fns->get)(fns->ctx);

	if (ct >= (temp->disable_temp))
		return BCME_RANGE;
	return ct;
}
#endif /* WL_DYNAMIC_TEMPSENSE */
