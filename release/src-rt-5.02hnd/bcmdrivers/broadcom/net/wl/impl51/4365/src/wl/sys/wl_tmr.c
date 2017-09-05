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
 * $Id: wl_tmr.c 467328 2014-04-03 01:23:40Z $
 *
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmendian.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>

#include <wl_tmr.h>


/* create timer */
struct wl_tmr *wl_init_tmr(void *w, void (*fn)(void *arg), void *arg, const char *name)
{
	wlc_info_t *wlc = (wlc_info_t *)w;
	struct wl_tmr *tmr;

	tmr = (struct wl_tmr *)MALLOC(wlc->osh, sizeof(*tmr));
	if (tmr == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}
	tmr->wlc = wlc;
	tmr->timer = wl_init_timer(wlc->wl, fn, arg, name);
	return tmr;
}
