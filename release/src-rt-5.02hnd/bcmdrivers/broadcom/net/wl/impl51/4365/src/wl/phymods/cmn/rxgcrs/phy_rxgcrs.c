/*
 * Rx Gain Control and Carrier Sense module implementation.
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
#include <phy_rxgcrs_api.h>
#include "phy_type_rxgcrs.h"
#include <phy_rstr.h>
#include <phy_rxgcrs.h>

/* forward declaration */
typedef struct phy_rxgcrs_mem phy_rxgcrs_mem_t;

/* module private states */
struct phy_rxgcrs_info {
	phy_info_t 				*pi;	/* PHY info ptr */
	phy_type_rxgcrs_fns_t	*fns;	/* PHY specific function ptrs */
	phy_rxgcrs_mem_t			*mem;	/* Memory layout ptr */
	uint8 region_group;
};

/* module private states memory layout */
struct phy_rxgcrs_mem {
	phy_rxgcrs_info_t		cmn_info;
	phy_type_rxgcrs_fns_t	fns;
/* add other variable size variables here at the end */
};

/* attach/detach */
phy_rxgcrs_info_t *
BCMATTACHFN(phy_rxgcrs_attach)(phy_info_t *pi)
{
	phy_rxgcrs_mem_t	*mem = NULL;
	phy_rxgcrs_info_t	*cmn_info = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_rxgcrs_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize infra params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* Initialize rxgcrs params */
	cmn_info->region_group = REGION_OTHER;

	return cmn_info;

	/* error */
fail:
	phy_rxgcrs_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_rxgcrs_detach)(phy_rxgcrs_info_t *cmn_info)
{
	phy_rxgcrs_mem_t	*mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	/* Malloc has failed. No cleanup is necessary here. */
	if (!cmn_info)
		return;

	/* Freeup memory associated with cmn_info. */
	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null rxgcrs module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_rxgcrs_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_rxgcrs_register_impl)(phy_rxgcrs_info_t *mi, phy_type_rxgcrs_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*mi->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_rxgcrs_unregister_impl)(phy_rxgcrs_info_t *mi)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* driver down processing */
int
phy_rxgcrs_down(phy_rxgcrs_info_t *mi)
{
	int callbacks = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));

	return callbacks;
}


/* When multiple PHY supported, then need to walk thru all instances of pi */
void
wlc_phy_set_locale(phy_info_t *pi, uint8 region_group)
{
	phy_rxgcrs_info_t *info = pi->rxgcrsi;
	phy_type_rxgcrs_fns_t *fns = info->fns;

	info->region_group = region_group;

	if (fns->set_locale != NULL)
		(fns->set_locale)(fns->ctx, region_group);
}

uint8
wlc_phy_get_locale(phy_rxgcrs_info_t *info)
{
	return info->region_group;
}
