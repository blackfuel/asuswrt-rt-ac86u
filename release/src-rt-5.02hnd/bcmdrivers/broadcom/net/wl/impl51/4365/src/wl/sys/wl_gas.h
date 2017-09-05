/*
 * Support bcm_gas 802.11u GAS (Generic Advertisement Service) state machine in the driver.
 * See bcm_gas for the API.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wl_gas.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wl_gas_h_
#define _wl_gas_h_

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
#include <wl_eventq.h>

typedef struct wl_gas_info wl_gas_info_t;

#define MAX_WLIF_NUM				(1)

/*
 * Initialize gas private context.
 * Returns a pointer to the gas private context, NULL on failure.
 */
extern wl_gas_info_t *wl_gas_attach(wlc_info_t *wlc, wl_eventq_info_t *wlevtq);

/* Cleanup gas private context */
extern void wl_gas_detach(wl_gas_info_t *gas);


/* get ethernet address */
#define wl_cur_etheraddr(wlc, idx, buf) wl_gas_get_etheraddr((wlc_info_t *)wlc, idx, buf)
extern int wl_gas_get_etheraddr(wlc_info_t *wlc, int bsscfg_idx, struct ether_addr *outbuf);

/* transmit an action frame */
#define wl_actframe wl_gas_tx_actframe
extern int wl_gas_tx_actframe(void *w, int bsscfg_idx,
	uint32 packet_id, uint32 channel, int32 dwell_time,
	struct ether_addr *BSSID, struct ether_addr *da,
	uint16 len, uint8 *data);

/* abort action frame */
#define wl_actframe_abort wl_gas_abort_actframe
extern int wl_gas_abort_actframe(void *w, int bsscfg_idx);

/* get wlcif */
#define wl_getifbybsscfgidx(wlc, bsscfgidx) \
	wl_gas_get_wlcif((wlc), (bsscfgidx))
extern struct wlc_if *wl_gas_get_wlcif(wlc_info_t *wlc, int bsscfgidx);

extern int wl_gas_start_eventq(wl_gas_info_t *gas);
extern void wl_gas_stop_eventq(wl_gas_info_t *gas);

extern void *wl_gas_malloc(void *w, size_t size);
extern void wl_gas_free(void *w, void* memblk);
#endif /* _wl_gas_h_ */
