/*
 * ANQP Offload
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wl_anqpo.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wl_anqpo_h_
#define _wl_anqpo_h_

#include <wlc_cfg.h>
#include <d11.h>
#include <wlc_types.h>
#include <wl_gas.h>

typedef struct wl_anqpo_info wl_anqpo_info_t;

/*
 * Initialize anqpo private context.
 * Returns a pointer to the anqpo private context, NULL on failure.
 */
extern wl_anqpo_info_t *wl_anqpo_attach(wlc_info_t *wlc, wl_gas_info_t *gas);

/* Cleanup anqpo private context */
extern void wl_anqpo_detach(wl_anqpo_info_t *anqpo);

/* initialize on scan start */
extern void wl_anqpo_scan_start(wl_anqpo_info_t *anqpo);

/* deinitialize on scan stop */
extern void wl_anqpo_scan_stop(wl_anqpo_info_t *anqpo);

/* process scan result */
extern void wl_anqpo_process_scan_result(wl_anqpo_info_t *anqpo,
	wlc_bss_info_t *bi, uint8 *ie, uint32 ie_len);

#endif /* _wl_anqpo_h_ */
