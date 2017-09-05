/*
 * WAPI (WLAN Authentication and Privacy Infrastructure) private header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_wapi_priv.h 467328 2014-04-03 01:23:40Z $
 */
#ifndef _wlc_wapi_priv_h_
#define _wlc_wapi_priv_h_

struct wlc_wapi_info {
	wlc_info_t *wlc;
	wlc_pub_t *pub;
	int cfgh;			/* bsscfg cubby handle to retrieve data from bsscfg */
	uint priv_offset;		/* offset of private bsscfg cubby */
};

#ifdef BCMWAPI_WAI
/* WAPI private bsscfg cubby structure and access macro */
typedef struct wlc_wapi_bsscfg_cubby_priv {
	/* non-critical data path security variables */
	bool	wai_preauth;	/* default is FALSE */
	uint8	*wapi_ie;	/* user plumbed wapi_ie */
	int	wapi_ie_len;	/* wapi_ie len */
} wlc_wapi_bsscfg_cubby_priv_t;


#define WAPI_BSSCFG_PRIV(wapi, cfg) \
	((wlc_wapi_bsscfg_cubby_priv_t *)BSSCFG_CUBBY((cfg), \
	((wapi)->cfgh + (wapi)->priv_offset)))
#endif /* BCMWAPI_WAI */

#endif /* _wlc_wapi_priv_h_ */
