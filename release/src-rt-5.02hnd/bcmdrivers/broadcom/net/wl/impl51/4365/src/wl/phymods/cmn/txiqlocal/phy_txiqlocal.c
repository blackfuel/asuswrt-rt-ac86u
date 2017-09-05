/*
 * TXIQLO CAL module implementation.
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
#include "phy_type_txiqlocal.h"
#include <phy_rstr.h>
#include <phy_txiqlocal.h>

/* forward declaration */
typedef struct phy_txiqlocal_mem phy_txiqlocal_mem_t;

/* module private states */
struct phy_txiqlocal_info {
	phy_info_t 					*pi;		/* PHY info ptr */
	phy_type_txiqlocal_fns_t	*fns;		/* Function ptr */
	phy_txiqlocal_mem_t			*mem;		/* Memory layout ptr */
};

/* module private states memory layout */
struct phy_txiqlocal_mem {
	phy_txiqlocal_info_t		cmn_info;
	phy_type_txiqlocal_fns_t	fns;
/* add other variable size variables here at the end */
};

/* local function declaration */

/* attach/detach */
phy_txiqlocal_info_t *
BCMATTACHFN(phy_txiqlocal_attach)(phy_info_t *pi)
{
	phy_txiqlocal_mem_t	*mem;
	phy_txiqlocal_info_t *cmn_info = NULL;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_txiqlocal_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* init the txiqlocal states */

	/* Register callbacks */

	return cmn_info;

	/* error */
fail:
	phy_txiqlocal_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_txiqlocal_detach)(phy_txiqlocal_info_t *cmn_info)
{
	phy_txiqlocal_mem_t	*mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	if (cmn_info == NULL) {
		PHY_INFORM(("The NULL argument was passed to the function %s\n",
			__FUNCTION__));
		return;
	}

	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null txiqlocal module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_txiqlocal_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_txiqlocal_register_impl)(phy_txiqlocal_info_t *cmn_info,
	phy_type_txiqlocal_fns_t *fns)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	*cmn_info->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_txiqlocal_unregister_impl)(phy_txiqlocal_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	cmn_info->fns = NULL;
}
