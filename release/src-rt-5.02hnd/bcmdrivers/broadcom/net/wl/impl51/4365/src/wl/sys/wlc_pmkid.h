/*
 * Exposed interfaces of wlc_pmkid.c
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_pmkid.h 487832 2014-06-27 05:13:40Z $
 */

/** Used for WPA(2) pre-authentication */

extern wlc_pmkid_info_t * wlc_pmkid_attach(wlc_info_t *wlc);
extern void wlc_pmkid_detach(wlc_pmkid_info_t *pmkid_info);

/* Gets called when RSN IE of assoc request has to be populated with PMKID
 * since the driver has PMKID store
 */
extern uint16
wlc_pmkid_putpmkid(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg,
	struct ether_addr *bssid, bcm_tlv_t *wpa2_ie, uint8 *fbt_pmkid, uint32 WPA_auth);

/* Clears currently stored PMKIDs both in PMKID module as well as supplicant */
extern void
wlc_pmkid_clear_store(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg);

/* identify candidate & add to the candidate list */
extern void
wlc_pmkid_prep_list(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg,
	struct ether_addr *bssid, uint8 wpa2_flags);

/* if candidate is part of supplicant, store PMKID as part of module main list
 * so that it can be used while preparing (re)assoc req
 */
extern void
wlc_pmkid_cache_req(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg);
#ifdef WL_OKC
extern void
wpa_calc_pmkid_for_okc(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg, struct ether_addr *auth_ea,
	struct ether_addr *sta_ea, uint8 *pmk, uint pmk_len, uint8 *data,
	uint8 *digest, int *index);
#endif
