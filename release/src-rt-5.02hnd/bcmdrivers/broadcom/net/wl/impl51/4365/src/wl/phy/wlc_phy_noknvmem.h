/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11 Networking Device Driver.
 * code to handle the Nokia nvmem speciifc details
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_phy_noknvmem.h 241182 2011-02-17 21:50:03Z $:
 */

#ifndef _wlc_phy_noknvmem_h_
#define _wlc_phy_noknvmem_h_

extern void *wlc_phy_noknvmem_attach(osl_t *osh, phy_info_t *pi);
extern void wlc_phy_noknvmem_detach(osl_t *osh, void *noknvmem);
extern int8 wlc_phy_noknvmem_antport_to_rfport_offset(phy_info_t *pi, uint chan,
	uint32 band, uint8 rate);
extern void wlc_phy_noknvmem_env_txpwrlimit_upd(phy_info_t *pi, int8 vbat, int8 temp, uint32 band);
extern void wlc_phy_noknvmem_get_pwrdet_offsets(phy_info_t *pi, int8 *cckoffset, int8 *ofdmoffset);
extern uint8 txp_rateindex2nokconstellation(uint8 rate_idx);
extern bool wlc_phy_noknvmem_env_check(phy_info_t *pi, int8 vbat, int8 temp);
extern int8 wlc_phy_noknvmem_modify_rssi(phy_info_t *pi, int8 rssi, chanspec_t chanspec);
extern void wlc_phy_nokia_brdtxpwr_limits(phy_info_t *pi, uint chan, int txp, uint8 *minpwr,
	uint8 *maxpwr);
#endif /* _wlc_phy_noknvmem_h_ */
