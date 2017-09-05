/*
 * PHYTblInit module internal interface - iovar table/handlers
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

#ifndef _phy_tbl_iov_h_
#define _phy_tbl_iov_h_

enum {
	IOV_PHYTABLE = 1
};

/* iovar table */
#ifdef WLC_HIGH
#include <typedefs.h>
#include <bcmutils.h>
extern const bcm_iovar_t phy_tbl_iovars[];

/* fixup function */
#ifdef WLC_HIGH_ONLY
#include <wlc_types.h>
#include <bcm_xdr.h>
bool phy_tbl_pack_iov(wlc_info_t *wlc, uint32 aid, void *p, uint p_len, bcm_xdr_buf_t *b);
#endif
#endif /* WLC_HIGH */

#endif /* _phy_tbl_iov_h_ */
