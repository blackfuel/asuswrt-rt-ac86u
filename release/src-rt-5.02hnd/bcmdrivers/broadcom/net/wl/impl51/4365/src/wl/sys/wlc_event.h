/*
 * Event mechanism
 *
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_event.h 473826 2014-04-29 23:43:06Z $
 */


#ifndef _WLC_EVENT_H_
#define _WLC_EVENT_H_

#include <typedefs.h>

typedef struct wlc_eventq wlc_eventq_t;

typedef void (*wlc_eventq_cb_t)(void *arg);

extern wlc_eventq_t *wlc_eventq_attach(wlc_pub_t *pub, struct wlc_info *wlc,
	void *wl, wlc_eventq_cb_t cb);
extern int wlc_eventq_detach(wlc_eventq_t *eq);
extern int wlc_eventq_down(wlc_eventq_t *eq);
extern void wlc_event_free(wlc_eventq_t *eq, wlc_event_t *e);
extern wlc_event_t *wlc_eventq_next(wlc_eventq_t *eq, wlc_event_t *e);
extern int wlc_eventq_cnt(wlc_eventq_t *eq);
extern bool wlc_eventq_avail(wlc_eventq_t *eq);
extern wlc_event_t *wlc_eventq_deq(wlc_eventq_t *eq);
extern void wlc_eventq_enq(wlc_eventq_t *eq, wlc_event_t *e);
extern wlc_event_t *wlc_event_alloc(wlc_eventq_t *eq);

#ifdef WLNOEIND
#define wlc_eventq_register_ind_ext(a, b, c) 0
#define wlc_eventq_query_ind_ext(a, b, c, d) 0
#define wlc_eventq_test_ind(a, b) FALSE
#define wlc_eventq_handle_ind(a, b) do {} while (0)
#define wlc_eventq_set_ind(a, b, c) do {} while (0)
#define wlc_eventq_flush(eq) do {} while (0)
#define wlc_assign_event_msg(a, b, c, d, e) do {} while (0)
#else /* WLNOEIND */
extern int wlc_eventq_register_ind_ext(wlc_eventq_t *eq, eventmsgs_ext_t* iovar_msg, uint8 *mask);
extern int wlc_eventq_query_ind_ext(wlc_eventq_t *eq, eventmsgs_ext_t* in_iovar_msg,
	eventmsgs_ext_t* out_iovar_msg, uint8 *mask);
extern int wlc_eventq_test_ind(wlc_eventq_t *eq, int et);
extern int wlc_eventq_handle_ind(wlc_eventq_t* eq, wlc_event_t *e);
extern int wlc_eventq_set_ind(wlc_eventq_t* eq, uint et, bool on);
extern void wlc_eventq_flush(wlc_eventq_t *eq);
extern void wlc_assign_event_msg(wlc_info_t *wlc, wl_event_msg_t *msg, const wlc_event_t *e,
	uint8 *data, uint32 len);

#endif /* WLNOEIND */

#if defined(MSGTRACE) || defined(LOGTRACE)
#include <rte_dev.h>
extern void wlc_event_sendup_trace(struct wlc_info * wlc, hnd_dev_t * bus, uint8* hdr,
                                   uint16 hdrlen, uint8 *buf, uint16 buflen);
#endif

#endif  /* _WLC_EVENT_H_ */
