/*
 * Miscellaneous modules interface (to other PHY modules).
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

#ifndef _phy_ac_misc_h_
#define _phy_ac_misc_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_misc.h>


/* forward declaration */
typedef struct phy_ac_misc_info phy_ac_misc_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_misc_info_t *phy_ac_misc_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_misc_info_t *cmn_info);
void phy_ac_misc_unregister_impl(phy_ac_misc_info_t *ac_info);

extern void wlc_phy_update_rxldpc_acphy(phy_info_t *pi, bool ldpc);
extern void wlc_phy_force_rfseq_acphy(phy_info_t *pi, uint8 cmd);
extern uint16 wlc_phy_classifier_acphy(phy_info_t *pi, uint16 mask, uint16 val);
extern void wlc_phy_deaf_acphy(phy_info_t *pi, bool mode);
extern bool wlc_phy_get_deaf_acphy(phy_info_t *pi);
extern void wlc_phy_gpiosel_acphy(phy_info_t *pi, uint16 sel, uint8 word_swap);
#if defined(BCMDBG) || defined(WLTEST)
extern int wlc_phy_freq_accuracy_acphy(phy_info_t *pi, int channel);
#endif
#if defined(WLTEST)
extern void wlc_phy_test_scraminit_acphy(phy_info_t *pi, int8 init);
#endif 

/* !!! This has been redeclared in wlc_phy_hal.h. Shoul dbe removed from there. !!! */
extern void wlc_acphy_set_scramb_dyn_bw_en(wlc_phy_t *pi, bool enable);

#endif /* _phy_ac_misc_h_ */
