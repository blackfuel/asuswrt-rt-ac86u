/*
 * IOCV module interface.
 * For BMAC/PHY.
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

#ifndef _wlc_iocv_h_
#define _wlc_iocv_h_

#ifndef WLC_LOW
#error "WLC_LOW is not defined"
#endif

#include <typedefs.h>
#include <wlc_types.h>
#include <wlc_iocv_types.h>

/* attach/detach */
wlc_iocv_info_t *wlc_iocv_attach(wlc_hw_info_t *hw);
void wlc_iocv_detach(wlc_iocv_info_t *ii);

/*
 * Dispatch iovar with table id 'tid' and action id 'aid'
 * to a registered dispatch callback.
 *
 * Return BCME_XXXX.
 */
int wlc_iocv_dispatch_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid,
	uint8 *p, uint p_len, uint8 *a, uint a_len, uint var_sz);

/*
 * Dispatch ioctl with table id 'tid' and command id 'cid'
 * to a registered dispatch callback.
 *
 * Return BCME_XXXX.
 */
int wlc_iocv_dispatch_ioc(wlc_iocv_info_t *ii, uint16 tid, uint16 cid,
	uint8 *a, uint a_len, bool *ta);

#endif /* _wlc_iocv_h_ */
