/*
 * Neighbor Advertisement Offload interface
 *
 *   Broadcom Proprietary and Confidential. Copyright (C) 2017,
 *   All Rights Reserved.
 *   
 *   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 *   the contents of this file may not be disclosed to third parties, copied
 *   or duplicated in any form, in whole or in part, without the prior
 *   written permission of Broadcom.
 *
 * $Id: wl_ndoe.h 476140 2014-05-08 01:19:15Z $
 */


#ifndef _wl_ndoe_h_
#define _wl_ndoe_h_

#include <proto/bcmipv6.h>

/* Forward declaration */
typedef struct wl_nd_info wl_nd_info_t;

#ifdef WLNDOE
extern wl_nd_info_t * wl_nd_attach(wlc_info_t *wlc);
extern void wl_nd_detach(wl_nd_info_t *ndi);
extern int wl_nd_recv_proc(wl_nd_info_t *ndi, void *sdu);
extern wl_nd_info_t * wl_nd_alloc_ifndi(wl_nd_info_t *ndi_p, wlc_if_t *wlcif);
extern void wl_nd_clone_ifndi(wl_nd_info_t *from_ndi, wl_nd_info_t *to_ndi);
extern int wl_nd_send_proc(wl_nd_info_t *ndi, void *sdu);
extern wl_nd_info_t * wl_nd_alloc_ifndi(wl_nd_info_t *ndi_p, wlc_if_t *wlcif);
extern void wl_nd_free_ifndi(wl_nd_info_t *ndi);
#ifdef WLNDOE_RA
extern int wl_nd_ra_filter_clear_cache(wl_nd_info_t *ndi);
#endif /* WLNDOE_RA */
#ifdef BCM_OL_DEV
extern void wl_nd_proc_msg(wlc_dngl_ol_info_t *wlc_dngl_ol, wl_nd_info_t *ndi, void *buf);
extern void wl_nd_update_stats(wl_nd_info_t *ndi, bool suppressed);
#endif
#endif /* WLNDOE */

#endif /* _WL_NDOE_H_ */
