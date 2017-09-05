/**
 * 802.11d (support for additional regulatory domains) module header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_11d.h 580000 2015-08-17 22:08:34Z $
*/


#ifndef _wlc_11d_h_
#define _wlc_11d_h_

/* APIs */
#ifdef WL11D

#if defined(CNTRY_DEFAULT) && defined(WLC_HIGH)
#define WLC_CNTRY_DEFAULT_ENAB(wlc) \
	((wlc)->m11d != NULL && wlc_11d_cntry_default_enabled((wlc)->m11d))
#else
#define WLC_CNTRY_DEFAULT_ENAB(wlc) FALSE
#endif /* CNTRY_DEFAULT */

/* module */
extern wlc_11d_info_t *wlc_11d_attach(wlc_info_t *wlc);
extern void wlc_11d_detach(wlc_11d_info_t *m11d);

extern void wlc_11d_scan_complete(wlc_11d_info_t *m11d, int status);

/* actions */
extern void wlc_11d_adopt_country(wlc_11d_info_t *m11d, char *country_str, bool adopt_country);
extern void wlc_11d_reset_all(wlc_11d_info_t *m11d);

/* accessors */
extern bool wlc_11d_autocountry_adopted(wlc_11d_info_t *m11d);
extern void wlc_11d_set_autocountry_default(wlc_11d_info_t *m11d, const char *country_abbrev);
extern const char *wlc_11d_get_autocountry_default(wlc_11d_info_t *m11d);

#else /* !WL11D */

#define wlc_11d_attach(wlc) NULL
#define wlc_11d_detach(m11d) do {} while (0)

#define wlc_11d_scan_complete(m11d, status) do {} while (0)

#define wlc_11d_adopt_country(m11d, country_str, adopt_country) BCM_REFERENCE(adopt_country)
#define wlc_11d_reset_all(m11d) do {} while (0)

#define wlc_11d_autocountry_adopted(m11d) FALSE
#define wlc_11d_set_autocountry_default(m11d, country_abbrev) BCM_REFERENCE(country_abbrev)
#define wlc_11d_get_autocountry_default(m11d) NULL

#endif /* !WL11D */

#endif /* _wlc_11d_h_ */
