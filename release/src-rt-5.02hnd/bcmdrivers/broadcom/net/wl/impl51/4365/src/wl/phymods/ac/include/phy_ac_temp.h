/*
 * ACPHY TEMPerature sense module interface (to other PHY modules).
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

#ifndef _phy_ac_temp_h_
#define _phy_ac_temp_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_temp.h>

/* forward declaration */
typedef struct phy_ac_temp_info phy_ac_temp_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_temp_info_t *phy_ac_temp_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_temp_info_t *ti);
void phy_ac_temp_unregister_impl(phy_ac_temp_info_t *info);

typedef struct _acphy_tempsense_phyregs {
	bool   is_orig;

	uint16 RxFeCtrl1;
	uint16 RxSdFeConfig1;
	uint16 RxSdFeConfig6;
	uint16 RfctrlIntc[PHY_CORE_MAX];
	uint16 RfctrlOverrideAuxTssi[PHY_CORE_MAX];
	uint16 RfctrlCoreAuxTssi1[PHY_CORE_MAX];
	uint16 RfctrlOverrideRxPus[PHY_CORE_MAX];
	uint16 RfctrlCoreRxPus[PHY_CORE_MAX];
	uint16 RfctrlOverrideTxPus[PHY_CORE_MAX];
	uint16 RfctrlCoreTxPus[PHY_CORE_MAX];
	uint16 RfctrlOverrideLpfSwtch[PHY_CORE_MAX];
	uint16 RfctrlCoreLpfSwtch[PHY_CORE_MAX];
	uint16 RfctrlOverrideGains[PHY_CORE_MAX];
	uint16 RfctrlCoreLpfGain[PHY_CORE_MAX];
	uint16 RfctrlOverrideAfeCfg[PHY_CORE_MAX];
	uint16 RfctrlCoreAfeCfg1[PHY_CORE_MAX];
	uint16 RfctrlCoreAfeCfg2[PHY_CORE_MAX];
} acphy_tempsense_phyregs_t;

typedef struct _tempsense_radioregs {
	bool   is_orig;
	uint16 OVR18[PHY_CORE_MAX];
	uint16 OVR19[PHY_CORE_MAX];
	uint16 OVR5[PHY_CORE_MAX];
	uint16 OVR3[PHY_CORE_MAX];
	uint16 tempsense_cfg[PHY_CORE_MAX];
	uint16 testbuf_cfg1[PHY_CORE_MAX];
	uint16 auxpga_cfg1[PHY_CORE_MAX];
	uint16 auxpga_vmid[PHY_CORE_MAX];
} tempsense_radioregs_t;

typedef struct _tempsense_radioregs_tiny {
	uint16 tempsense_cfg[PHY_CORE_MAX];
	uint16 testbuf_cfg1[PHY_CORE_MAX];
	uint16 auxpga_cfg1[PHY_CORE_MAX];
	uint16 auxpga_vmid[PHY_CORE_MAX];
	uint16 tempsense_ovr1[PHY_CORE_MAX];
	uint16 testbuf_ovr1[PHY_CORE_MAX];
	uint16 auxpga_ovr1[PHY_CORE_MAX];
	uint16 tia_cfg9[PHY_CORE_MAX];
	uint16 adc_ovr1[PHY_CORE_MAX];
	uint16 adc_cfg10[PHY_CORE_MAX];
	uint16 tia_cfg5[PHY_CORE_MAX];
	uint16 rx_bb_2g_ovr_east[PHY_CORE_MAX];
	uint16 tia_cfg7[PHY_CORE_MAX];
} tempsense_radioregs_tiny_t;

typedef struct _acphy_tempsense_radioregs
{
	union {
		tempsense_radioregs_t acphy_tempsense_radioregs;
		tempsense_radioregs_tiny_t acphy_tempsense_radioregs_tiny;
	} u;
} acphy_tempsense_radioregs_t;

extern int16 wlc_phy_tempsense_acphy(phy_info_t *pi);
extern int16 wlc_phy_tempsense_acphy_tiny(phy_info_t *pi);
extern void phy_ac_update_tempsense_bitmap(phy_info_t *pi);
#endif /* _phy_ac_temp_h_ */
