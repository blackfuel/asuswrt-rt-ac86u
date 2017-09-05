/*
 * ACPHY MU-MIMO module interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: phy_ac_mu.h 525162 2015-01-09 02:07:08Z $
 */

#ifndef _phy_ac_mu_h_
#define _phy_ac_mu_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_mu.h>

/* forward declaration */
typedef struct phy_ac_mu_info phy_ac_mu_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_mu_info_t *phy_ac_mu_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_mu_info_t *ri);
void phy_ac_mu_unregister_impl(phy_ac_mu_info_t *info);

#endif /* _phy_ac_mu_h_ */
