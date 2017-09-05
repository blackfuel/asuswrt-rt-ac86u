/*
 *  OKC and RCC related header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * Currently what is implemented is OKC(WL_OKC).
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: wlc_okc.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wlc_okc_h_
#define _wlc_okc_h_

#ifdef WL_OKC
struct okc_info {
	wlc_info_t *wlc;
	bool okc_lock;
	uint16 pmk_len;
	uint8  pmk[32];
};

#define WLC_OKC_INFO(wlc) ((struct okc_info *) (wlc->okc_info))

extern int wlc_calc_pmkid_for_okc(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	struct ether_addr *BSSID, int *index);
#endif /* WL_OKC */

#ifdef WLRCC
#define RCC_MODE_FORCE		0xFF
#define RCC_MODE_AUTO		0

extern void rcc_update_channel_list(wlc_roam_t *roam, wlc_ssid_t *ssid,
	wl_join_assoc_params_t *assoc_param);
extern void rcc_add_chanspec(wlc_roam_t *roam, uint8 ch);
extern void rcc_update_from_join_targets(wlc_roam_t *roam, wlc_bss_list_t *targets);
#endif /* WLRCC */

extern void *wlc_okc_attach(wlc_info_t *wlc);
extern void wlc_okc_detach(void *hdl);
#endif /* _wlc_okc_h */
