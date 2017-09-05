/*
 * ACPHY Core module internal interface (to other PHY modules).
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

#ifndef _phy_ac_h_
#define _phy_ac_h_

#include <phy.h>
#include <wlc_phy_int.h> /* *** !!! To be removed !!! *** */

#ifdef ALL_NEW_PHY_MOD
typedef struct phy_ac_info phy_ac_info_t;
#else
/* < TODO: all these are going away... */
typedef struct phy_info_acphy phy_ac_info_t;
/* TODO: all these are going away... > */
#endif /* ALL_NEW_PHY_MOD */
#define PHY_REG_READ_REV(pi, phy_type, reg_name, field, rev) \
		((phy_utils_read_phyreg(pi, phy_type##_##reg_name(rev)) & \
		phy_type##_##reg_name##_##field##_##MASK(rev)) >> \
		phy_type##_##reg_name##_##field##_##SHIFT(rev))
void phy_ac_update_phycorestate(phy_info_t *pi);
void phy_regaccess_war_acphy(phy_info_t *pi);

/* ************************************************************ */
/* ACPHY module							*/
/* function declarations / intermodule api's			*/
/* ************************************************************ */

/* ************************************************************ */

/* *** Needs to be moved to TPC header once the AC modules are created *** */
#define PHY_TXPWR_MIN_ACPHY	1	/* for acphy devices */
#define PHY_TXPWR_MIN_ACPHY1X1EPA	8	/* for acphy1x1 ipa devices */
#define PHY_TXPWR_MIN_ACPHY1X1IPA	1	/* for acphy1x1 ipa devices */
#define PHY_TXPWR_MIN_ACPHY2X2	5	/* for 2x2 acphy devices */
#ifdef POWPERCHANNL
#define CH20MHz_NUM_2G	14 /* Number of 20MHz channels in 2G band */
#define PWR_PER_CH_NORM_TEMP	0	/* Temp zone  in norm for power per channel  */
#define PWR_PER_CH_LOW_TEMP		1	/* Temp zone  in low for power per channel  */
#define PWR_PER_CH_HIGH_TEMP	2	/* Temp zone  in high for power per channel  */
#define PWR_PER_CH_TEMP_MIN_STEP	5	/* Min temprature step for sensing  */
#define PWR_PER_CH_NEG_OFFSET_LIMIT_QDBM 20 /* maximal power reduction offset: 5dB =20 qdBm */
#define PWR_PER_CH_POS_OFFSET_LIMIT_QDBM 12 /* maximal power increase offset: 3dB =12 qdBm */
#endif


#endif /* _phy_ac_h_ */
