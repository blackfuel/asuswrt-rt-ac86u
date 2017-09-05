/*
 * PHY Core module implementation - register all PHY type specific implementations'
 * iovar tables/handlers to IOCV module - used by high driver
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_cfg.h>
#include <phy_dbg.h>
#include "phy_type_disp_high.h"

#include <wlc_iocv_types.h>

#if NCONF
#include "phy_type_n_iovt_high.h"
#include "phy_type_n_ioct_high.h"
#endif
#if HTCONF
#include "phy_type_ht_iovt_high.h"
#include "phy_type_ht_ioct_high.h"
#endif
#if LCNCONF
#include "phy_type_lcn_iovt_high.h"
#include "phy_type_lcn_ioct_high.h"
#endif
#if LCN40CONF
#include "phy_type_lcn40_iovt_high.h"
#include "phy_type_lcn40_ioct_high.h"
#endif
#if ACCONF || ACCONF2
#include "phy_type_ac_iovt_high.h"
#include "phy_type_ac_ioct_high.h"
#endif


/* ============= PHY type implementation dispatch table ============= */

typedef struct {
	uint8 phy_type;
	int (*reg_iovt_all)(wlc_iocv_info_t *ii);
	int (*reg_ioct_all)(wlc_iocv_info_t *ii);
} phy_type_reg_tbl_t;

static phy_type_reg_tbl_t BCMATTACHDATA(phy_type_reg_tbl)[] = {
#if NCONF
	{PHY_TYPE_N, phy_n_high_register_iovt, phy_n_high_register_ioct},
#endif
#if HTCONF
	{PHY_TYPE_HT, phy_ht_high_register_iovt, phy_ht_high_register_ioct},
#endif
#if LCNCONF
	{PHY_TYPE_LCN, phy_lcn_high_register_iovt, phy_lcn_high_register_ioct},
#endif
#if LCN40CONF
	{PHY_TYPE_LCN40, phy_lcn40_high_register_iovt, phy_lcn40_high_register_ioct},
#endif
#if ACCONF || ACCONF2
	{PHY_TYPE_AC, phy_ac_high_register_iovt, phy_ac_high_register_ioct},
#endif
	/* *** ADD NEW PHY TYPE IMPLEMENTATION ENTRIES HERE *** */
};

/* register PHY type specific implementation iovar tables/handlers */
int
BCMATTACHFN(phy_type_high_register_iovt)(uint phytype, wlc_iocv_info_t *ii)
{
	uint i;

	/* Unregister PHY type specific implementation with common */
	for (i = 0; i < ARRAYSIZE(phy_type_reg_tbl); i ++) {
		if (phytype == phy_type_reg_tbl[i].phy_type) {
			if (phy_type_reg_tbl[i].reg_iovt_all != NULL)
				return (phy_type_reg_tbl[i].reg_iovt_all)(ii);
			return BCME_OK;
		}
	}

	return BCME_NOTFOUND;
}

/* register PHY type specific implementation ioctl tables/handlers */
int
BCMATTACHFN(phy_type_high_register_ioct)(uint phytype, wlc_iocv_info_t *ii)
{
	uint i;

	/* Unregister PHY type specific implementation with common */
	for (i = 0; i < ARRAYSIZE(phy_type_reg_tbl); i ++) {
		if (phytype == phy_type_reg_tbl[i].phy_type) {
			if (phy_type_reg_tbl[i].reg_ioct_all != NULL)
				return (phy_type_reg_tbl[i].reg_ioct_all)(ii);
			return BCME_OK;
		}
	}

	return BCME_NOTFOUND;
}
