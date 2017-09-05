/**
 * Beacon Coalescing related source file
 * Broadcom 802.11abg Networking Device Driver
 * http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/BeaconOffload
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_bcn_clsg.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wlc_bcn_clsg_h_
#define _wlc_bcn_clsg_h_

#ifdef WL_BCN_COALESCING
extern wlc_bcn_clsg_info_t *wlc_bcn_clsg_attach(wlc_info_t *bc);
extern void wlc_bcn_clsg_detach(wlc_bcn_clsg_info_t *bc);
extern bool wlc_bcn_clsg_in_ucode(wlc_bcn_clsg_info_t *bc, bool time_since_bcn, bool *flushed);
extern void wlc_bcn_clsg_store_rxh(wlc_bcn_clsg_info_t *bc, wlc_d11rxhdr_t *wrxh);
extern void wlc_bcn_clsg_update_rxh(wlc_bcn_clsg_info_t *bc, wlc_d11rxhdr_t *wrxh);
extern uint32 wlc_get_len_from_plcp(wlc_d11rxhdr_t *wrxh, uint8 *plcp);

#define BCN_CLSG_CONFIG_MASK    0x01	/* For disabled by config/user */
#define BCN_CLSG_ASSOC_MASK     0x02	/* For associated station */
#define BCN_CLSG_BSS_MASK       0x08	/* For more than 1 BSS is Active */
#define BCN_CLSG_SCAN_MASK      0x10	/* For SCANNING */
#define BCN_CLSG_UATBTT_MASK    0x20	/* For unaligned TBTT */
#define BCN_CLSG_PRMS_MASK      0x40	/* For promiscous mode */
#define BCN_CLSG_CORE_MASK      0x80	/* For not supported by device core rev */
#define BCN_CLSG_UPDN_MASK      0x100	/* For up/down mode */
#define BCN_CLSG_PM_MASK        0x200	/* Power Save Mode Mask */

extern void wlc_bcn_clsg_disable(wlc_bcn_clsg_info_t *bc, uint32 mask, uint32 val);


#endif /* WL_BCN_COALESCING */

#endif /* _wlc_bcn_clsg_h_ */
