/*
 * ACPHY PHYTblInit module implementation - iovar handlers & registration
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
#include <phy_api.h>
#include <phy_tbl_iov.h>
#include <phy_ac_tbl_iov.h>

#include <wlc_iocv_types.h>
#include <wlc_iocv_reg.h>

#ifdef WLC_LOW
#include <phy_ac_info.h>


static int
phy_ac_tbl_doiovar(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsize)
{
#if defined(WLTEST)
	phy_info_t *pi = (phy_info_t *)ctx;
	int err = BCME_OK;

	switch (aid) {
	case IOV_GVAL(IOV_PHYTABLE):
		wlc_phy_table_read_acphy(pi, *(uint32 *)p, 1,
		                         *((uint32 *)p + 1), *((uint32 *)p + 2),
		                         (uint32 *)a);
		break;

	case IOV_SVAL(IOV_PHYTABLE):
		wlc_phy_table_write_acphy(pi, *(uint32 *)p, 1,
		                          *((uint32 *)p + 1), *((uint32 *)p + 2),
		                          (uint32 *)p + 3);
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
#else
	return BCME_UNSUPPORTED;
#endif 
}
#endif /* WLC_LOW */

/* register iovar table to the system */
int
BCMATTACHFN(phy_ac_tbl_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	wlc_iovt_desc_t iovd;

	ASSERT(ii != NULL);

	wlc_iocv_init_iovd(phy_tbl_iovars,
	                   NULL, NULL,
	                   phy_ac_tbl_doiovar, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
