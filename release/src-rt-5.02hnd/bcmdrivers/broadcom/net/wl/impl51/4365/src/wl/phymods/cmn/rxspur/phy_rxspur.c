/*
 * Rx Spur canceller module implementation.
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
#include "phy_type_rxspur.h"
#include <phy_rstr.h>
#include <phy_rxspur.h>

/* forward declaration */
typedef struct phy_rxspur_mem phy_rxspur_mem_t;

/* module private states */
struct phy_rxspur_info {
	phy_info_t				*pi;	/* PHY info ptr */
	phy_type_rxspur_fns_t	*fns;	/* PHY specific function ptrs */
	phy_rxspur_mem_t			*mem;	/* Memory layout ptr */
};

/* module private states memory layout */
struct phy_rxspur_mem {
	phy_rxspur_info_t		cmn_info;
	phy_type_rxspur_fns_t	fns;
/* add other variable size variables here at the end */
};

/* local function declaration */

/* attach/detach */
phy_rxspur_info_t *
BCMATTACHFN(phy_rxspur_attach)(phy_info_t *pi)
{
	phy_rxspur_mem_t	*mem = NULL;
	phy_rxspur_info_t	*cmn_info = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_rxspur_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize infra params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* Initialize rxspur params */

	/* Register callbacks */

	return cmn_info;

	/* error */
fail:
	phy_rxspur_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_rxspur_detach)(phy_rxspur_info_t *cmn_info)
{
	phy_rxspur_mem_t *mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */

	if (!cmn_info)
		return;

	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null rxspur module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_rxspur_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_rxspur_register_impl)(phy_rxspur_info_t *mi, phy_type_rxspur_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*mi->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_rxspur_unregister_impl)(phy_rxspur_info_t *mi)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* driver down processing */
int
phy_rxspur_down(phy_rxspur_info_t *mi)
{
	int callbacks = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));

	return callbacks;
}
