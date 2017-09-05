/*
 * IOCV module implementation - ioctl/iovar registry switcher.
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

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include "wlc_iocv_if.h"
#include <wlc_iocv_types.h>
#include <wlc_iocv_reg.h>

/* forward iovar table & callbacks registration to implementation */
int
wlc_iocv_register_iovt(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd)
{
	ASSERT(ii->iovt_reg_fn != NULL);

	return (ii->iovt_reg_fn)(ii, iovd);
}

/* forward ioctl table & callbacks registration implementation */
int
wlc_iocv_register_ioct(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd)
{
	ASSERT(ii->ioct_reg_fn != NULL);

	return (ii->ioct_reg_fn)(ii, iocd);
}
