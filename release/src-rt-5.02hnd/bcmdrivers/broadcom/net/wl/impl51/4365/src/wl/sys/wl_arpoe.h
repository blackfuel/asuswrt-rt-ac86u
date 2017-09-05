/*
 * ARP Offload interface
 *
 *   Broadcom Proprietary and Confidential. Copyright (C) 2016,
 *   All Rights Reserved.
 *   
 *   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 *   the contents of this file may not be disclosed to third parties, copied
 *   or duplicated in any form, in whole or in part, without the prior
 *   written permission of Broadcom.
 *
 *   $Id: wl_arpoe.h 475379 2014-05-05 17:55:00Z $
 */

#ifndef _wl_arpoe_h_
#define _wl_arpoe_h_

/* Forward declaration */
typedef struct wl_arp_info wl_arp_info_t;

/* Return values */
#define ARP_REPLY_PEER 		0x1	/* Reply was sent to service ARP request from peer */
#define ARP_REPLY_HOST		0x2	/* Reply was sent to service ARP request from host */
#define ARP_REQ_SINK		0x4	/* Input packet should be discarded */
#define ARP_FORCE_FORWARD       0X5     /* ARP req should be forwarded to host,
					 * bypassing pktfilter
					 */

#ifdef ARPOE

#define NON_ARP				-1 /* received packet is not ARP packet */
#define TRUNCATED_ARP		-2 /* received packet is truncated ARP packet */

/*
 * Initialize ARP private context.
 * Returns a pointer to the ARP private context, NULL on failure.
 */
extern wl_arp_info_t *wl_arp_attach(wlc_info_t *wlc);

/* Cleanup ARP private context */
extern void wl_arp_detach(wl_arp_info_t *arpi);

/* Process frames in transmit direction */
extern bool wl_arp_send_pktfetch_required(wl_arp_info_t *arpi, void *sdu);
extern int wl_arp_send_proc(wl_arp_info_t *arpi, void *sdu);

/* Process frames in receive direction */
extern int wl_arp_recv_proc(wl_arp_info_t *arpi, void *sdu);

/* called when a new virtual IF is created.
 *	i/p: primary ARPIIF [arpi_p] and the new wlcif,
 *	o/p: new arpi structure populated with inputs and
 *		the global parameters duplicated from arpi_p
 *	side-effects: arpi for a new IF will inherit properties of arpi_p till
 *		the point new arpi is created. After that, for any change in
 *		arpi_p will NOT change the arpi corr to new IF. To change property
 *		of new IF, wl -i wl0.x has to be used.
*/
extern wl_arp_info_t *wl_arp_alloc_ifarpi(wl_arp_info_t *arpi_p,
	wlc_if_t *wlcif);
extern void wl_arp_clone_arpi(wl_arp_info_t *from_arpi, wl_arp_info_t *to_arpi);

extern void wl_arp_free_ifarpi(wl_arp_info_t *arpi);
#ifdef BCM_OL_DEV
extern void wl_arp_update_stats(wl_arp_info_t *arpi, bool suppressed);
extern void
wl_arp_proc_msg(wlc_dngl_ol_info_t * wlc_dngl_ol, wl_arp_info_t *arpi, void *buf);
#endif
#else	/* stubs */

#define wl_arp_attach(a)		(wl_arp_info_t *)0x0dadbeef
#define	wl_arp_detach(a)		do {} while (0)
#define wl_arp_send_pktfetch_required(a, b)		(0)
#define wl_arp_send_proc(a, b)		(-1)
#define wl_arp_recv_proc(a, b)		(-1)
#define wl_arp_alloc_ifarpi(a, b)	(0)
#define wl_arp_free_ifarpi(a)		do {} while (0)
#define wl_arp_clone_arpi(a, b)     do {} while (0)
#endif /* ARPOE */

#endif	/* _wl_arpoe_h_ */
