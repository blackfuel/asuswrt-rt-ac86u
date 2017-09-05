/*
 * TEMPerature sense module internal interface - iovar table registration
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

#ifndef _phy_temp_iov_t_
#define _phy_temp_iov_t_

#include <phy_api.h>

#include <wlc_iocv_types.h>

/* register iovar table/handlers */
int phy_temp_register_iovt(phy_info_t *pi, wlc_iocv_info_t *ii);

#endif /* _phy_temp_iov_t_ */
