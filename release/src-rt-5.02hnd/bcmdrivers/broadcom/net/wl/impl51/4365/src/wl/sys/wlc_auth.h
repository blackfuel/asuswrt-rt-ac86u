/*
 * Exposed interfaces of wlc_auth.c
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_auth.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_auth_h_
#define _wlc_auth_h_

/* Values for type parameter of wlc_set_auth() */
#define AUTH_UNUSED	0	/* Authenticator unused */
#define AUTH_WPAPSK	1	/* Used for WPA-PSK */

extern wlc_auth_info_t* wlc_auth_attach(wlc_info_t *wlc);
extern authenticator_t* wlc_authenticator_attach(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_auth_detach(wlc_auth_info_t *auth_info);
extern void wlc_authenticator_detach(authenticator_t *auth);
extern void wlc_authenticator_down(authenticator_t *auth);

/* Install WPA PSK material in authenticator */
extern int wlc_auth_set_pmk(authenticator_t *auth, wsec_pmk_t *psk);
extern bool wlc_set_auth(authenticator_t *auth, int type, uint8 *sup_ies, uint sup_ies_len,
                         uint8 *auth_ies, uint auth_ies_len, struct scb *scb);

extern bool wlc_auth_eapol(authenticator_t *auth, eapol_header_t *eapol_hdr,
                           bool encrypted, struct scb *scb);

extern void wlc_auth_join_complete(authenticator_t *auth_info, struct ether_addr *ea,
                                   bool initialize);

extern void wlc_auth_tkip_micerr_handle(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);

#endif	/* _wlc_auth_h_ */
