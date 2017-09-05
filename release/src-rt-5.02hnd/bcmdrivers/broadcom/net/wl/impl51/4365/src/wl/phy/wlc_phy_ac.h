/*
 * ACPHY module header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_phy_ac.h 537076 2015-02-25 02:31:49Z $
 */

#ifndef _wlc_phy_ac_h_
#define _wlc_phy_ac_h_

#include <typedefs.h>
#include <wlc_phy_int.h>

#include <phy_ac_info.h>

/*
 * ACPHY Core REV info and mapping to (Major/Minor)
 * http://confluence.broadcom.com/display/WLAN/ACPHY+Major+and+Minor+PHY+Revision+Mapping
 *
 * revid  | chip used | (major, minor) revision
 *  0     : 4360A0/A2 (3x3) | (0, 0)
 *  1     : 4360B0    (3x3) | (0, 1)
 *  2     : 4335A0    (1x1 + low-power improvment, no STBC/antdiv) | (1, 0)
 *  3     : 4350A0/B0 (2x2 + low-power 2x2) | (2, 0)
 *  4     : 4345TC    (1x1 + tiny radio)  | (3, 0)
 *  5     : 4335B0    (1x1, almost the same as rev2, txbf NDP stuck fix) | (1, 1)
 *  6     : 4335C0    (1x1, rev5 + bug fixes + stbc) | (1, 2)
 *  7     : 4345A0    (1x1 + tiny radio, rev4 improvment) | (3, 1)
 *  8     : 4350C0    (2x2 + low-power 2x2) | (2, 1)
 *  9     : 43602A0   (3x3 + fixme_info) | (2, 2)
 *  10    : 4349TC2A0
 *  11    : 43457A0   (Based on 4345A0 plus support A4WP (Wireless Charging)
 *  12    : 4349A0    (1x1, Reduced Area Tiny-2 Radio plus channel bonding 80+80 supported)
 *  13    : 4345B0/43457B0	(1x1 + tiny radio, phyrev 7 improvements) | (3, 3)
 *  20    : 4345C0    (1x1 + tiny radio) | (3, 4)
 */

#include "phy_api.h"
#include "phy_ac_ana.h"
#include "phy_ac_radio.h"
#include "phy_ac_tbl.h"
#include "phy_ac_tpc.h"
#include "phy_ac_radar.h"
#include "phy_ac_antdiv.h"
#include "phy_ac_temp.h"
#include "phy_ac_rssi.h"
#include "phy_ac_rxiqcal.h"
#include "phy_ac_txiqlocal.h"
#include "phy_ac_papdcal.h"
#include "phy_ac_vcocal.h"

/* ********************************************************************************** */
/* *** PLEASE DO NOT ADD ANY DEFINITION HERE. DO IT IN THE RELEVANT PHYMOD/MODULE DIR *** */
/* *** THIS FILE WILL BE REMOVED SOON *** */
/* ********************************************************************************** */

#if     defined(WLOFFLD) || defined(BCM_OL_DEV)
int8 wlc_phy_noise_sample_acphy(wlc_phy_t *pih);
void wlc_phy_noise_reset_ma_acphy(wlc_phy_t *pih);
#endif /* defined(WLOFFLD) || defined(BCM_OL_DEV) */

/* *********************** Remove ************************** */
void wlc_phy_get_initgain_dB_acphy(phy_info_t *pi, int16 *initgain_dB);
uint8 wlc_phy_rxgainctrl_encode_gain_acphy(phy_info_t *pi, uint8 clipgain, uint8 core,
	int8 gain_dB, bool trloss, bool lna1byp, uint8 *gidx);
void wlc_phy_farrow_setup_tiny(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_phy_write_tx_farrow_tiny(phy_info_t *pi, chanspec_t chanspec,
	chanspec_t chanspec_sc, int sc_mode);


#endif /* _wlc_phy_ac_h_ */
