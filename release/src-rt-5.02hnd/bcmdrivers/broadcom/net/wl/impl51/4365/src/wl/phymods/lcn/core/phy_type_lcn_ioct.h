/*
 * LCNPHY Core module internal interface
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

#ifndef _phy_lcn_ioct_h_
#define _phy_lcn_ioct_h_

#include <phy_api.h>
#include "phy_type.h"
#include <wlc_iocv_types.h>

/* register LCNPHY specific ioctl tables/handlers to system */
int phy_lcn_register_ioct(phy_info_t *pi, phy_type_info_t *ti, wlc_iocv_info_t *ii);

#endif /* _phy_lcn_ioct_h_ */
