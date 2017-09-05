/*
 * WL (per-port) event queue, for use by dongle offloads that need to process wl events
 * asynchronously. Not to be confused with wlc_eventq, which is used by the common code
 * to send events to the host.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wl_eventq.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wl_eventq_h_
#define _wl_eventq_h_

#include <wlc_types.h>


/* opaque private context */
typedef struct wl_eventq_info wl_eventq_info_t;

typedef void (*wl_eventq_cb_t)(void *ctx, uint32 event_type,
	wl_event_msg_t *wl_event, uint8 *data, uint32 length);

/*
 * Initialize wl eventq private context.
 * Returns a pointer to the wl eventq private context, NULL on failure.
 */
extern wl_eventq_info_t *wl_eventq_attach(wlc_info_t *wlc);

/* Cleanup wl event queue private context */
extern void wl_eventq_detach(wl_eventq_info_t *wlevtq);

/* register a callback fn to handle events */
extern int wl_eventq_register_event_cb(wl_eventq_info_t *wlevtq, uint32 event[], uint count,
	wl_eventq_cb_t cb, void *arg);

/* unregister a callback fn */
extern void wl_eventq_unregister_event_cb(wl_eventq_info_t *wlevtq, wl_eventq_cb_t cb);

/* duplicate event for wl event queue */
extern void wl_eventq_dup_event(wl_eventq_info_t *wlevtq, wlc_event_t *e);

#endif /* _wl_eventq_h_ */
