/*
 * WAPI (WLAN Authentication and Privacy Infrastructure) public header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_wapi.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wlc_wapi_h_
#define _wlc_wapi_h_

/*
 * We always have BCMWAPI_WPI enabled we don't have
 * other cipher alternation instead of WPI (SMS4)
 */
#if defined(BCMWAPI_WPI) || defined(BCMWAPI_WAI)

#ifdef BCMWAPI_WAI

/* WAPI public bsscfg cubby structure and access macro */
typedef struct wlc_wapi_bsscfg_cubby_pub wlc_wapi_bsscfg_cubby_pub_t;

/* Use wlc instead of wapi to save one dereference instruction time */
#define WAPI_BSSCFG_PUB(wlc, cfg) \
	((wlc_wapi_bsscfg_cubby_pub_t *)BSSCFG_CUBBY((cfg), (wlc)->wapi_cfgh))

struct wlc_wapi_bsscfg_cubby_pub {
	/* data path variables provide macro access */
	bool	wai_restrict;	/* restrict data until WAI auth succeeds */
};
#define WAPI_WAI_RESTRICT(wlc, cfg)	(WAPI_BSSCFG_PUB((wlc), (cfg))->wai_restrict)

/* Macros for lookup the unicast/multicast ciphers for SMS4 in RSN */
#define WAPI_RSN_UCAST_LOOKUP(prsn)	(wlc_rsn_ucast_lookup((prsn), WAPI_CIPHER_SMS4))
#define WAPI_RSN_MCAST_LOOKUP(prsn)	((prsn)->multicast == WAPI_CIPHER_SMS4)

/*
 * WAI LLC header,
 * DSAP/SSAP/CTL = AA:AA:03
 * OUI = 00:00:00
 * Ethertype = 0x88b4 (WAI Port Access Entity)
 */
#define WAPI_WAI_HDR	"\xAA\xAA\x03\x00\x00\x00\x88\xB4"
#define WAPI_WAI_SNAP(pbody)	(bcmp(WAPI_WAI_HDR, (pbody), DOT11_LLC_SNAP_HDR_LEN) == 0)
#endif /* BCMWAPI_WAI */

/* module */
extern wlc_wapi_info_t *wlc_wapi_attach(wlc_info_t *wlc);
extern void wlc_wapi_detach(wlc_wapi_info_t *wapi);

#ifdef BCMWAPI_WAI
extern void wlc_wapi_station_event(wlc_wapi_info_t* wapi, wlc_bsscfg_t *bsscfg,
	const struct ether_addr *addr, void *ie, uint8 *gsn, uint16 msg_type);
#if !defined(WLNOEIND)
extern void wlc_wapi_bss_wai_event(wlc_wapi_info_t *wapi, wlc_bsscfg_t * bsscfg,
	const struct ether_addr *ea, uint8 *data, uint32 len);
#endif /* !WLNOEIND */
#endif /* BCMWAPI_WAI */

#endif /* BCMWAPI_WPI || BCMWAPI_WAI */
#endif /* _wlc_wapi_h_ */
