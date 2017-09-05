/*
 * Exposed interfaces of wlc_fbt.c
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_fbt.h 664112 2016-10-10 13:27:40Z $
 */


#ifndef _wlc_fbt_h_
#include <proto/eapol.h>

#define _wlc_fbt_h_

typedef struct wlc_fbt_pub {
	int cfgh;			/* bsscfg cubby handle */
} wlc_fbt_pub_t;

typedef struct bss_fbt_pub {
	bool ini_fbt;		/* initial ft handshake */
} bss_fbt_pub_t;

#define WLC_FBT_INFO_CFGH(fbt_info) (((wlc_fbt_pub_t *)(fbt_info))->cfgh)

#define BSS_FBT_INFO(fbt_info, cfg) \
	(*(bss_fbt_pub_t **)BSSCFG_CUBBY(cfg, WLC_FBT_INFO_CFGH(fbt_info)))
#define BSS_FBT_INI_FBT(fbt_info, cfg) (BSS_FBT_INFO(fbt_info, cfg)->ini_fbt)

extern wlc_fbt_info_t * BCMATTACHFN(wlc_fbt_attach)(wlc_info_t *wlc);
extern void BCMATTACHFN(wlc_fbt_detach)(wlc_fbt_info_t *fbt_info);

#if defined(STA) && defined(FBT_STA)
extern int wlc_fbt_set_pmk(wlc_fbt_info_t *fbt_info, struct wlc_bsscfg *cfg,
	wsec_pmk_t *pmk, bool assoc);
/* verify with current mdid */
extern void wlc_fbt_clear_ies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
extern bool wlc_fbt_is_cur_mdid(wlc_fbt_info_t *fbt_info, struct wlc_bsscfg *cfg,
	wlc_bss_info_t *bi);
extern bool wlc_fbt_is_fast_reassoc(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
	wlc_bss_info_t *bi);
extern bool wlc_fbt_akm_match(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg,
	const wlc_bss_info_t *bi);
extern uint8 *
wlc_fbt_get_pmkr1name(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
extern void
wlc_fbt_calc_fbt_ptk(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
extern void
wlc_fbt_addies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, eapol_wpa_key_header_t *wpa_key);
extern uint16 wlc_fbt_getlen_eapol(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
extern void wlc_fbt_recv_overds_resp(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, uint body_len);
extern void *wlc_fbt_send_overds_req(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
	struct ether_addr *ea, struct scb *scb, bool short_preamble);
extern void wlc_fbt_save_current_akm(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg,
	const wlc_bss_info_t *bi);
extern void wlc_fbt_reset_current_akm(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg);
extern void wlc_fbt_get_kck_kek(wlc_fbt_info_t *fbt_info, struct wlc_bsscfg *cfg, uint8 *key);

#else /* STA && FBT_STA */
#define wlc_fbt_set_pmk(f, c, p, a)			0
#define wlc_fbt_clear_ies(f, c)				do {} while (0)
#define wlc_fbt_is_cur_mdid(f, c, b)			FALSE
#define wlc_fbt_is_fast_reassoc(f, c, b)		FALSE
#define wlc_fbt_akm_match(f, c, b)			FALSE
#define wlc_fbt_get_pmkr1name(f, c)			NULL
#define wlc_fbt_calc_fbt_ptk(f, b)			do {} while (0)
#define wlc_fbt_addies(f, c, w)				do {} while (0)
#define wlc_fbt_getlen_eapol(f, c)			0
#define wlc_fbt_recv_overds_resp(f, c, h, b, l)		do {} while (0)
#define wlc_fbt_send_overds_req(f, c, e, s, p)		NULL
#define wlc_fbt_save_current_akm(f, c, b)		do {} while (0)
#define wlc_fbt_reset_current_akm(f, c)			do {} while (0)
#define wlc_fbt_get_kck_kek(f, c, k)			do {} while (0)
#endif /* STA && FBT_STA */

extern void
wlc_fbt_set_ea(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, struct ether_addr *ea);

extern bool wlc_fbt_enabled(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
#ifdef AP
extern void wlc_fbt_handle_auth(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
extern void wlc_fbt_recv_overds_req(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, void *body, uint body_len);
#endif /* AP */
#endif	/* _wlc_fbt_h_ */
