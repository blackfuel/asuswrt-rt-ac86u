/*
 * PAPD CAL module interface (to other PHY modules).
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

#ifndef _phy_papdcal_h_
#define _phy_papdcal_h_

#include <phy_api.h>

/* forward declaration */
typedef struct phy_papdcal_info phy_papdcal_info_t;

/* attach/detach */
phy_papdcal_info_t *phy_papdcal_attach(phy_info_t *pi);
void phy_papdcal_detach(phy_papdcal_info_t *cmn_info);

/* init */

#endif /* _phy_papdcal_h_ */
