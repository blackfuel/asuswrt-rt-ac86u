/*
 * PHY Core module public interface (to HIGH MAC driver).
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

#ifndef _phy_high_api_h_
#define _phy_high_api_h_

#include <typedefs.h>
#include <wlc_iocv_types.h>

/* register all iovar/ioctl tables/handlers to/from system */
int phy_high_register_iovt_all(uint phytype, wlc_iocv_info_t *ii);
int phy_high_register_ioct_all(uint phytype, wlc_iocv_info_t *ii);

#endif /* _phy_high_api_h_ */
