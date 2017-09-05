/*
 * IOCV module interface - data type declarations.
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

#ifndef _wlc_iocv_types_h_
#define _wlc_iocv_types_h_

#include <typedefs.h>

/* forward declarations */
typedef struct wlc_iocv_info wlc_iocv_info_t;

/* iovar dispatch function */
typedef int (*wlc_iov_disp_fn_t)(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsz);
/* ioctl dispatch function */
typedef int (*wlc_ioc_disp_fn_t)(void *ctx, uint16 cid,
	void *a, uint alen, bool *ta);

#endif /* _wlc_iocv_types_h_ */
