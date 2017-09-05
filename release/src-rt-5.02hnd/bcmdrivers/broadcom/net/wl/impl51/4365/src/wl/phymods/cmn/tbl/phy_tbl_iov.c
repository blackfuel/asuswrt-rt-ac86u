/*
 * PHYTblInit module implementation - iovar table/handlers
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
/* iovar table */
#ifdef WLC_HIGH
#include <typedefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc.h>

#include "phy_tbl_iov.h"

const bcm_iovar_t phy_tbl_iovars[] = {
#if defined(WLTEST) || defined(DBG_PHY_IOV)
	{"phytable", IOV_PHYTABLE, IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG, IOVT_BUFFER, 4*4},
#endif
	{NULL, 0, 0, 0, 0}
};

#ifdef WLC_HIGH_ONLY
#include <bcm_xdr.h>

bool
phy_tbl_pack_iov(wlc_info_t *wlc, uint32 aid, void *p, uint p_len, bcm_xdr_buf_t *b)
{
#if defined(WLTEST) || defined(DBG_PHY_IOV)
	int err;

	/* Decide the buffer is 16-bit or 32-bit buffer */
	switch (IOV_ID(aid)) {
	case IOV_PHYTABLE:
		p_len &= ~3;
		err = bcm_xdr_pack_uint32(b, p_len);
		ASSERT(!err);
		err = bcm_xdr_pack_uint32_vec(b, p_len, p);
		ASSERT(!err);
		return TRUE;
	}
#endif 
	return FALSE;
}
#endif /* WLC_HIGH_ONLY */
#endif /* WLC_HIGH */
