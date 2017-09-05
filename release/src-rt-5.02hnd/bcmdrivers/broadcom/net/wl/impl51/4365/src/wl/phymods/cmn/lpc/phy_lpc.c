/*
 * Link Power Control module implementation.
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
#include "phy_type_lpc.h"
#include <phy_rstr.h>
#include <phy_lpc.h>

/* forward declaration */
typedef struct phy_lpc_mem phy_lpc_mem_t;

/* module private states */
struct phy_lpc_info {
	phy_info_t			*pi;	/* PHY info ptr */
	phy_type_lpc_fns_t	*fns;	/* PHY specific function ptrs */
	phy_lpc_mem_t		*mem;	/* Memory layout ptr */
};

/* module private states memory layout */
struct phy_lpc_mem {
	phy_lpc_info_t		cmn_info;
	phy_type_lpc_fns_t	fns;
/* add other variable size variables here at the end */
};

/* attach/detach */
phy_lpc_info_t *
BCMATTACHFN(phy_lpc_attach)(phy_info_t *pi)
{
	phy_lpc_mem_t	*mem = NULL;
	phy_lpc_info_t	*cmn_info = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_lpc_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize infra params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* Initialize lpc params */

	/* Register callbacks */

	return cmn_info;

	/* error */
fail:
	phy_lpc_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_lpc_detach)(phy_lpc_info_t *cmn_info)
{
	phy_lpc_mem_t	*mem;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	/* Malloc has failed. No cleanup is necessary here. */
	if (!cmn_info)
		return;

	/* Freeup memory associated with cmn_info */
	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null lpc module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_lpc_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_lpc_register_impl)(phy_lpc_info_t *cmn_info, phy_type_lpc_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*cmn_info->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_lpc_unregister_impl)(phy_lpc_info_t *cmn_info)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* driver down processing */
int
phy_lpc_down(phy_lpc_info_t *cmn_info)
{
	int callbacks = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));

	return callbacks;
}
