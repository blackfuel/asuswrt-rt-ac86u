/*
 * Secure WiFi through NFC
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wl_nfc.h 43960 2013-11-26 15:57:35 $
 */


#ifndef	_WL_NFC_H
#define	_WL_NFC_H

/* Forward declaration */
typedef struct wl_nfc_info wl_nfc_info_t;

#define NFC_BUF_SIZE			1024
#define ETHER_TYPE_NFC_SECURE_WIFI	0xC021

#ifdef WLNFC
extern wl_nfc_info_t * wl_nfc_attach(wlc_info_t *wlc);
extern void wl_nfc_detach(wl_nfc_info_t *nfci);
extern wl_nfc_info_t *wl_nfc_alloc_ifnfci(wl_nfc_info_t *nfci_p, wlc_if_t *wlcif);
extern void wl_nfc_free_ifnfci(wl_nfc_info_t *nfci);
extern int wl_nfc_recv_proc(wl_nfc_info_t *nfci, void *sdu);
extern int wl_nfc_send_proc(wl_nfc_info_t *nfci, void *sdu);
extern void wl_recv_from_nfc_uart(void *p, const char *s, uint16 len);
#endif /* WLNFC */

#endif /* _WL_NFC_H */
