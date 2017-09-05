/*
 * MU-MIMO phy module internal interface (to other PHY modules).
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: phy_mu.h 525162 2015-01-09 02:07:08Z $
 */

#ifndef _phy_mu_h_
#define _phy_mu_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_mu_info phy_mu_info_t;

/* attach/detach */
phy_mu_info_t *phy_mu_attach(phy_info_t *pi);
void phy_mu_detach(phy_mu_info_t *ri);

#endif /* _phy_mu_h_ */
