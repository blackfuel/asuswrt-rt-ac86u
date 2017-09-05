/*
 * IOCV module interface - abstract interface
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

#ifndef _wlc_iocv_if_h_
#define _wlc_iocv_if_h_

/* iocv module interface */
#include "wlc_iocv_reg.h"
#include "wlc_iocv_types.h"

struct wlc_iocv_info {
	/* bmac/phy iovar/ioctl table/handlers registration fn */
	int (*iovt_reg_fn)(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd);
	int (*ioct_reg_fn)(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd);
	void *obj;	/* object pointer */
};

#endif /* _wlc_iocv_if_h_ */
