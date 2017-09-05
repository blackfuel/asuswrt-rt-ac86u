/*
 * Exposed interfaces of wlc_sup_ccx.c
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_sup_ccx.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_ccxsup_h_
#define _wlc_ccxsup_h_


typedef struct wlc_ccxsup_pub {
	int cfgh;			/* bsscfg cubby handle */
} wlc_ccxsup_pub_t;


#define WLC_CCXSUP_INFO_CFGH(ccxsup_info) (((wlc_ccxsup_pub_t *)(ccxsup_info))->cfgh)
extern wlc_ccxsup_info_t * wlc_ccxsup_attach(wlc_info_t *wlc);
extern void wlc_ccxsup_detach(wlc_ccxsup_info_t *ccxsup_info);

/* Initiate supplicant private context */
extern int wlc_ccxsup_init(void *ctx, sup_init_event_data_t *evt);

/* Remove supplicant private context */
extern void wlc_ccxsup_deinit(void *ctx, wlc_bsscfg_t *cfg);

/* Return whether the given SSID matches on in the LEAP list. */
extern bool wlc_ccx_leap_ssid(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg,
	uchar SSID[], int len);

/* Time-out  LEAP authentication and presume the AP is a rogue */
extern void wlc_ccx_rogue_timer(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg,
	struct ether_addr *ap_mac);

/* Register a rogue AP report */
extern void wlc_ccx_rogueap_update(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg,
	uint16 reason, struct ether_addr *ap_mac);

/* Return whether the supplicant state indicates successful authentication */
extern bool wlc_ccx_authenticated(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg);

#if defined(BCMSUP_PSK) || !defined(BCMINTSUP)
/* Populate the CCKM reassoc req IE */
extern void wlc_cckm_gen_reassocreq_IE(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg,
	cckm_reassoc_req_ie_t *cckmie, uint32 tsf_h, uint32 tsf_l, struct ether_addr *bssid,
	wpa_ie_fixed_t *rsnie);

/* Check for, validate, and process the CCKM reassoc resp IE */
extern bool wlc_cckm_reassoc_resp(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg);
#endif /* BCMSUP_PSK || !BCMINTSUP */

extern void wlc_ccx_sup_init(struct wlc_ccxsup_info *ccxsup_info,
	struct wlc_bsscfg *cfg, int sup_type);

extern bool
wlc_sup_getleapauthpend(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);

#if defined BCMEXTCCX
/* For external supplicant */
extern void
wlc_cckm_set_assoc_resp(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg,
	uint8 *assoc_resp, int len);
extern void
wlc_cckm_set_rn(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, int rn);
#endif

extern sup_auth_status_t wlc_ccxsup_get_auth_status(wlc_ccxsup_info_t *ccxsup_info,
	wlc_bsscfg_t *cfg);
extern uint16 wlc_ccxsup_get_cipher(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg,
	wpapsk_t *wpa, uint16 key_info, uint16 key_len);
extern uint16 wlc_ccxsup_handle_joinstart(wlc_ccxsup_info_t *ccxsup_info,
	wlc_bsscfg_t *cfg, uint16 sup_type);
extern void wlc_ccxsup_handle_wpa_eapol_msg1(wlc_ccxsup_info_t *ccxsup_info,
	wlc_bsscfg_t *cfg, uint16 key_info);
extern void wlc_ccxsup_send_leap_rogue_report(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);
extern void wlc_ccxsup_set_leap_state_keyed(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);
extern void wlc_ccxsup_init_cckm_rn(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);
extern void wlc_ccxsup_start_negotimer(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);
extern bool wlc_ccx_leapsup(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg,
	eapol_header_t *rx_hdr);
/* Initiate supplicant LEAP authentication. */
extern bool wlc_leapsup_start(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);
#endif	/* _wlc_ccxsup_h_ */
