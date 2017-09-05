/*
 * TCP Keep-Alive & ICMP  Offload interface
 *
 *   Broadcom Proprietary and Confidential. Copyright (C) 2017,
 *   All Rights Reserved.
 *   
 *   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 *   the contents of this file may not be disclosed to third parties, copied
 *   or duplicated in any form, in whole or in part, without the prior
 *   written permission of Broadcom.
 *
 *   $Id: wl_tcpkoe.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wl_tcpkoe_h_
#define _wl_tcpkoe_h_

/* Forward declaration */
typedef struct wl_icmp_info wl_icmp_info_t;
typedef struct tcp_keep_info    wl_tcp_keep_info_t;
#define IP_DEFAULT_TTL     32

#ifdef TCPKAOE

extern wl_icmp_info_t *wl_icmp_attach(wlc_info_t *wlc);
extern wl_tcp_keep_info_t *wl_tcp_keep_attach(wlc_info_t *wlc);
extern void wl_tcp_keep_detach(wl_tcp_keep_info_t *tcp_keep_info);

extern void wl_icmp_detach(wl_icmp_info_t *icmpi);
extern int wl_icmp_recv_proc(wl_icmp_info_t *icmpi, void *sdu);
extern int wl_tcpkeep_recv_proc(wl_tcp_keep_info_t *tcp_keep_info, void *sdu);

#ifdef BCM_OL_DEV
extern void
wl_tcp_keepalive_proc_msg(wlc_dngl_ol_info_t * wlc_dngl_ol, wl_tcp_keep_info_t *tcpkeepi,
    void *buf);
#endif /* BCM_OL_DEV */

#else /* TCPKAOE */
#define wl_icmp_attach(a)		(wl_icmp_info_t *)0x0dadbeef
#define wl_icmp_detach(a)		do {} while (0)
#define wl_tcp_keep_attach(a)		(wl_tcp_keep_info_t *)0x0dadbeef
#ifdef BCM_OL_DEV
#define wl_tcp_keepalive_proc_msg(a, b, c)	do {} while (0)
#endif /* BCM_OL_DEV */
#endif /* TCPKAOE */

#endif	/* _wl_tcpkoe_h_ */
