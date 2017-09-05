/*
 * tmr module
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wl_tmr.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wl_tmr_h_
#define _wl_tmr_h_

#include <wlc.h>
#include <wl_export.h>


typedef struct wl_tmr {
	wlc_info_t		*wlc;
	struct wl_timer		*timer;
} tmrT;


/* create timer */
#ifndef tmrCreate
#define tmrCreate(wlc, fn, arg, name) \
	wl_init_tmr((wlc), (fn), (arg), (name))
#endif
extern struct wl_tmr * wl_init_tmr(void *w,
	void (*fn)(void *arg), void *arg, const char *name);

/* destroy timer */
#ifndef tmrDestroy
#define tmrDestroy(tmr) \
	do { \
		ASSERT(tmr); \
		wl_free_timer((tmr)->wlc->wl, (tmr)->timer); \
		MFREE((tmr)->wlc->osh, (tmr), sizeof(tmrT)); \
	} while (0);
#endif

/* start timer */
#ifndef tmrStart
#define tmrStart(tmr, ms, is_periodic) \
	do { \
		ASSERT(tmr); \
		wl_add_timer((tmr)->wlc->wl, (tmr)->timer, (ms), (is_periodic)); \
	} while (0);
#endif

/* stop timer */
#ifndef tmrStop
#define tmrStop(tmr) \
	do { \
		ASSERT(tmr); \
		wl_del_timer((tmr)->wlc->wl, (tmr)->timer); \
	} while (0);
#endif

#endif /* _wl_tmr_h_ */
