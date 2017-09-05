/*
 * ACPHY Link Power Control module implementation
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
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_lpc.h"
#include <phy_ac.h>
#include <phy_ac_lpc.h>

/* module private states */
struct phy_ac_lpc_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_lpc_info_t *cmn_info;
};

/* local functions */

/* register phy type specific implementation */
phy_ac_lpc_info_t *
BCMATTACHFN(phy_ac_lpc_register_impl)(phy_info_t *pi, phy_ac_info_t *aci, phy_lpc_info_t *cmn_info)
{
	phy_ac_lpc_info_t *ac_info;
	phy_type_lpc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_lpc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = ac_info;

	if (phy_lpc_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_lpc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_ac_lpc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_lpc_unregister_impl)(phy_ac_lpc_info_t *ac_info)
{
	phy_info_t *pi;
	phy_lpc_info_t *cmn_info;

	ASSERT(ac_info);
	pi = ac_info->pi;
	cmn_info = ac_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_lpc_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info, sizeof(phy_ac_lpc_info_t));
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
#ifdef WL_LPC

#define LPC_MIN_IDX 31
#define LPC_TOT_IDX (LPC_MIN_IDX + 1)
#define PWR_VALUE_BITS  0x3F

/*	table containing values of 0.5db difference,
	Each value is represented in S4.1 format
*/
static uint8 lpc_pwr_level[LPC_TOT_IDX] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
		0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
		0x1E, 0x1F /* Max = Target - 15.5db */
		};


/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */

uint8
wlc_acphy_lpc_getminidx(void)
{
	return LPC_MIN_IDX;
}

uint8
wlc_acphy_lpc_getoffset(uint8 index)
{
	return lpc_pwr_level[index];
	/* return lpc_pwr_level[index]; for PHYs which expect the actual offset
	 * for example, HT 4331.
	 */
}

#ifdef WL_LPC_DEBUG
uint8 *
wlc_acphy_lpc_get_pwrlevelptr(void)
{
	return lpc_pwr_level;
}
#endif
#endif /* WL_LPC */
