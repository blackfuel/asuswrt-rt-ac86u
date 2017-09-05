/*
 * ACPHY Link Power Control module interface (to other PHY modules).
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

#ifndef _phy_ac_lpc_h_
#define _phy_ac_lpc_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_lpc.h>

/* forward declaration */
typedef struct phy_ac_lpc_info phy_ac_lpc_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_lpc_info_t *phy_ac_lpc_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_lpc_info_t *cmn_info);
void phy_ac_lpc_unregister_impl(phy_ac_lpc_info_t *ac_info);

#ifdef WL_LPC
extern uint8 wlc_acphy_lpc_getminidx(void);
extern uint8 wlc_acphy_lpc_getoffset(uint8 index);
#ifdef WL_LPC_DEBUG
extern uint8 * wlc_acphy_lpc_get_pwrlevelptr(void);
#endif
#endif /* WL_LPC */

#endif /* _phy_ac_lpc_h_ */
