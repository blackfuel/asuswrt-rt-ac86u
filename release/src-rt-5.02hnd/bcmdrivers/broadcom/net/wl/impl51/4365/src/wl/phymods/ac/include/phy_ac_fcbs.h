/*
 * ACPHY FCBS module interface (to other PHY modules).
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

#ifndef _phy_ac_fcbs_h_
#define _phy_ac_fcbs_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_fcbs.h>

/* forward declaration */
typedef struct phy_ac_fcbs_info phy_ac_fcbs_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_fcbs_info_t *phy_ac_fcbs_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_fcbs_info_t *cmn_info);
void phy_ac_fcbs_unregister_impl(phy_ac_fcbs_info_t *ac_info);

#ifdef ENABLE_FCBS
extern uint16 wlc_phy_channelindicator_obtain_acphy(phy_info_t *pi);
extern bool wlc_phy_fcbsinit_acphy(phy_info_t *pi, int chanidx, chanspec_t chanspec);
extern bool wlc_phy_postfcbsinit_acphy(phy_info_t *pi, int chanidx);
extern bool wlc_phy_fcbs_acphy(phy_info_t *pi, int chanidx);
extern bool wlc_phy_prefcbsinit_acphy(phy_info_t *pi, int chanidx);
extern bool wlc_phy_postfcbs_acphy(phy_info_t *pi, int chanidx);
extern bool wlc_phy_prefcbs_acphy(phy_info_t *pi, int chanidx);
#endif /* ENABLE_FCBS */
#endif /* _phy_ac_fcbs_h_ */
