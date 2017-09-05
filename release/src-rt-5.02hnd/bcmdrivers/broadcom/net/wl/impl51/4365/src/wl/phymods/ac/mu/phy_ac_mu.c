/*
 * ACPHY MU-MIMO module implementation
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: phy_ac_mu.c 532720 2015-02-06 22:33:58Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <qmath.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_mu.h"
#include <phy_ac.h>
#include <phy_ac_info.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <phy_ac_mu.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_ac.h>
/* TODO: all these are going away... > */
#endif

/* The following code assumes the registers ACPHY_GidLutVal0_3 through ACPHY_GidLutVal60_63
 * are in a contiguous block. If adding support for a new AC PHY rev, please verify that
 * the registers are still in a block. If they are contiguous, or the feature does not exist
 * for the rev, adjust the #if condition below to allow the compile. If the feature is
 * implemented, but the registers are not contiguous, update the code to handle the new GID
 * register layout.
 */
#if ACCONF_GT(40) || ACCONF_MSK(0xF0E80000) || ACCONF2_MSK(0x6c)
#error "Verify that ACPHY_GidLutVal60_63(phy_rev) = ACPHY_GidLutVal0_3(phy_rev) + 15"
#endif

/* Shift for MU_MIMO user position value within an MU group nibble */
#define MU_USER_POS_SHIFT  1

/* Number of MU groups specified in each register */
#define MU_GROUPS_PER_REG  4

/* Number of bits used to define each group */
#define MU_REG_GROUP_SHIFT 4

/* module private states */
struct phy_ac_mu_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_mu_info_t *mu_info;
};

/* local functions */
#ifdef WL_MU_RX
static int phy_ac_mu_group_set(phy_type_mu_ctx_t *ctx,
	uint16 mu_group, uint8 user_pos, uint8 is_member);
#endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int phy_ac_mu_dump(phy_type_mu_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_ac_mu_dump NULL
#endif

/* register phy type specific implementation */
phy_ac_mu_info_t *
BCMATTACHFN(phy_ac_mu_register_impl)(phy_info_t *pi, phy_ac_info_t *aci, phy_mu_info_t *mi)
{
	phy_ac_mu_info_t *ac_mu_info;
	phy_type_mu_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_mu_info = phy_malloc(pi, sizeof(phy_ac_mu_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	ac_mu_info->pi = pi;
	ac_mu_info->aci = aci;
	ac_mu_info->mu_info = mi;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
#ifdef WL_MU_RX
	fns.mu_group_set = phy_ac_mu_group_set;
#endif
	fns.dump = phy_ac_mu_dump;
	fns.ctx = ac_mu_info;

	phy_mu_register_impl(mi, &fns);

	return ac_mu_info;

	/* error handling */
fail:
	if (ac_mu_info != NULL)
		phy_mfree(pi, ac_mu_info, sizeof(phy_ac_mu_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_mu_unregister_impl)(phy_ac_mu_info_t *ac_mu_info)
{
	phy_info_t *pi = ac_mu_info->pi;
	phy_mu_info_t *mi = ac_mu_info->mu_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_mu_unregister_impl(mi);

	phy_mfree(pi, ac_mu_info, sizeof(phy_ac_mu_info_t));
}

#ifdef WL_MU_RX
/* Set MU group */
static int
phy_ac_mu_group_set(phy_type_mu_ctx_t *ctx, uint16 mu_group, uint8 user_pos, uint8 is_member)
{
	phy_ac_mu_info_t *info = (phy_ac_mu_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint16 val = is_member | (user_pos << MU_USER_POS_SHIFT);
	uint16 reg_addr;
	uint16 reg_mask;
	uint8 shift;

	if ((mu_group == VHT_SIGA1_GID_TO_AP) || (mu_group >= VHT_SIGA1_GID_MAX_GID)) {
		PHY_ERROR(("%s: Invalid MU-MIMO group %u\n", __FUNCTION__, mu_group));
		return BCME_ERROR;
	}
	if (user_pos > VHT_SIGA1_MAX_USERPOS) {
		PHY_ERROR(("%s: Invalid MU-MIMO user position %u\n", __FUNCTION__, user_pos));
		return BCME_ERROR;
	}

	/* Register address is the base register address plus an offset determined
	 * by the group ID.
	 */
	reg_addr = ACPHY_GidLutVal0_3(pi->pubpi.phy_rev);
	if (reg_addr == INVALID_ADDRESS) {
		return BCME_UNSUPPORTED;
	}
	reg_addr += (mu_group / MU_GROUPS_PER_REG);
	shift = MU_REG_GROUP_SHIFT * (mu_group % MU_GROUPS_PER_REG);
	val = val << shift;
	reg_mask = 0xf << shift;
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	_PHY_REG_MOD(pi, reg_addr, reg_mask, val);
	wlapi_enable_mac(pi->sh->physhim);

	return BCME_OK;
}
#endif /* WL_MU_RX */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
phy_ac_mu_dump(phy_type_mu_ctx_t *ctx, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "ACPHY MU-MIMO:\n");

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */
