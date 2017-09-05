/*
 * TxPowerCtrl module internal interface (to other PHY modules).
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

#ifndef _phy_tpc_h_
#define _phy_tpc_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_tpc_info phy_tpc_info_t;

/* attach/detach */
phy_tpc_info_t *phy_tpc_attach(phy_info_t *pi);
void phy_tpc_detach(phy_tpc_info_t *ri);

/* ******** interface for TPC module ******** */
/* tx gain settings */
typedef struct {
	uint16 rad_gain; /* Radio gains */
	uint16 rad_gain_mi; /* Radio gains [16:31] */
	uint16 rad_gain_hi; /* Radio gains [32:47] */
	uint16 dac_gain; /* DAC attenuation */
	uint16 bbmult;   /* BBmult */
} txgain_setting_t;

/* recalc target txpwr and apply to h/w */
void phy_tpc_recalc_tgt(phy_tpc_info_t *ti);

/* read srom for the bandtype */
int phy_tpc_read_srom(phy_tpc_info_t *ti, int bandtype);

/* check limit */
void phy_tpc_check_limit(phy_info_t *pi);

#endif /* _phy_tpc_h_ */
