/*
 * NPHY TxPowerCtrl module interface (to other PHY modules).
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

#ifndef _phy_n_tpc_h_
#define _phy_n_tpc_h_

#include <phy_api.h>
#include <phy_n.h>
#include <phy_tpc.h>

/* forward declaration */
typedef struct phy_n_tpc_info phy_n_tpc_info_t;

/* register/unregister NPHY specific implementations to/from common */
phy_n_tpc_info_t *phy_n_tpc_register_impl(phy_info_t *pi,
	phy_n_info_t *ni, phy_tpc_info_t *ri);
void phy_n_tpc_unregister_impl(phy_n_tpc_info_t *info);

#endif /* _phy_n_tpc_h_ */
