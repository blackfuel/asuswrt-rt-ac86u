/*
 * MU-MIMO phy module implementation
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: phy_mu.c 525162 2015-01-09 02:07:08Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_mu.h>
#include <phy_mu.h>
#include <phy_mu_api.h>

/* module private states */
struct phy_mu_info {
	phy_info_t *pi;
	phy_type_mu_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_mu_info_t mu_info;
	phy_type_mu_fns_t fns;
/* add other variable size variables here at the end */
} phy_mu_mem_t;

/* local function declaration */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int phy_mu_dump(void *ctx, struct bcmstrbuf *b);
#endif

/* attach/detach */
phy_mu_info_t *
BCMATTACHFN(phy_mu_attach)(phy_info_t *pi)
{
	phy_mu_info_t *mu_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mu_info = phy_malloc(pi, sizeof(phy_mu_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	mu_info->pi = pi;
	mu_info->fns = &((phy_mu_mem_t *)mu_info)->fns;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* register dump callback */
	phy_dbg_add_dump_fn(pi, "phymu", (phy_dump_fn_t)phy_mu_dump, mu_info);
#endif

	return mu_info;

	/* error */
fail:
	phy_mu_detach(mu_info);
	return NULL;
}

void
BCMATTACHFN(phy_mu_detach)(phy_mu_info_t *mu_info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (mu_info == NULL) {
		PHY_INFORM(("%s: null mu module\n", __FUNCTION__));
		return;
	}

	pi = mu_info->pi;

	phy_mfree(pi, mu_info, sizeof(phy_mu_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_mu_register_impl)(phy_mu_info_t *mu_info, phy_type_mu_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));


	*mu_info->fns = *fns;
	return BCME_OK;
}

void
BCMATTACHFN(phy_mu_unregister_impl)(phy_mu_info_t *mu_info)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

#ifdef WL_MU_RX
/* Set or clear MU-MIMO group membership for a given group. When this station
 * is a member of the given group, set the user position as well.
 * Inputs:
 *   pi       - phy info
 *   mu_group - MU-MIMO group ID [1, 62]
 *   user_pos - user position within this group (i.e., which of the NSTS fields
 *              within VHT-SIG-A specifies the number of spatial streams to demodulate.)
 *              Unused if is_member is 0.
 *   is_member - 1 if this station is a member of this group
 *               0 otherwise
 *
 * Returns:
 *   BCME_OK if hardware is successfully updated
*/
int
phy_mu_group_set(phy_info_t *pi, uint16 mu_group, uint8 user_pos, uint8 is_member)
{
	phy_mu_info_t *mu_info = pi->mui;
	phy_type_mu_fns_t *fns = mu_info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ISSIM_ENAB(pi->sh->sih)) {
		return BCME_UNSUPPORTED;
	}

	/* Check if phy is capable of acting as MU beamformee */
	if ((wlc_phy_cap_get((wlc_phy_t*)pi) & PHY_CAP_MU_BFE) == 0) {
		return BCME_UNSUPPORTED;
	}

	/* redirect the request to PHY type specific implementation */
	ASSERT(fns->mu_group_set != NULL);
	return (fns->mu_group_set)(fns->ctx, mu_group, user_pos, is_member);
}
#endif   /* WL_MU_RX */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
phy_mu_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_mu_info_t *info = (phy_mu_info_t *)ctx;
	phy_type_mu_fns_t *fns = info->fns;

	if (fns->dump == NULL)
		return BCME_UNSUPPORTED;

	return (fns->dump)(fns->ctx, b);
}
#endif  /* BCMDBG || BCMDBG_DUMP */
