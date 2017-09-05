/*
 * Support WiFi-Direct discovery state machine in the driver
 * for the P2P ofload (p2po).
 * See bcm_p2p_disc and wlc_p2po for the APIs.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wl_p2po_disc.h 468737 2014-04-08 17:59:20Z $
 */


#ifndef _wl_disc_h_
#define _wl_disc_h_

#include <wlc_cfg.h>
#include <d11.h>
#include <wlc_types.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <wlioctl.h>
#include <proto/bcmevent.h>
#include <siutils.h>
#include <wlc_pub.h>
#include <osl.h>
#include <wlc.h>
#include <wl_export.h>
#include <wl_tmr.h>

typedef struct wl_disc_info wl_disc_info_t;

/*
 * Initialize disc private context.
 * Returns a pointer to the disc private context, NULL on failure.
 */
extern wl_disc_info_t *wl_disc_attach(wlc_info_t *wlc);

/* Cleanup disc private context */
extern void wl_disc_detach(wl_disc_info_t *disc);


/* get device bsscfg index */
#define wl_p2p_dev(wlc, bsscfgIndex) \
	wl_disc_get_p2p_devcfg_idx((wlc), (bsscfgIndex))
extern int32 wl_disc_get_p2p_devcfg_idx(void *w, int32 *idx);

/* set p2p discovery state */
#define wl_p2p_state(wlc, state, chspec, dwell) \
	wl_disc_p2p_state((wlc), (state), (chspec), (dwell))
extern int wl_disc_p2p_state(void *w,
	uint8 state, chanspec_t chspec, uint16 dwell);

/* do p2p scan */
#define wl_p2p_scan(wlc, sync_id, is_active, num_probes, \
		active_dwell_time, passive_dwell_time, home_time, \
		num_channels, channels, flags) \
	wl_disc_p2p_scan((wlc), (sync_id), (is_active), (num_probes), \
		(active_dwell_time), (passive_dwell_time), (int)(home_time), \
		(num_channels), (channels), (flags))
extern int wl_disc_p2p_scan(void *w, uint16 sync_id, int is_active,
	int num_probes, int active_dwell_time, int passive_dwell_time,
	int home_time, int num_channels, uint16 *channels, uint8 flags);

/* get discovery bsscfg */
extern wlc_bsscfg_t * wl_disc_bsscfg(wl_disc_info_t *disc);

#endif /* _wl_disc_h_ */
