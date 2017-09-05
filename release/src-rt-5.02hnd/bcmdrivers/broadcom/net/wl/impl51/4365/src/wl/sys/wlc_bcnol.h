/*
 * Beacon offload	definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_bcnol.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_bcnol_h_
#define _wlc_bcnol_h_

#include <proto/802.11.h>
extern wlc_dngl_ol_bcn_info_t *wlc_dngl_ol_bcn_attach(wlc_dngl_ol_info_t *wlc_dngl_ol);
extern bool
wlc_dngl_ol_bcn_process(wlc_dngl_ol_bcn_info_t *bcn_ol,
	void *p, struct dot11_management_header *hdr);
extern void wlc_dngl_ol_bcn_send_proc(wlc_dngl_ol_bcn_info_t *bcn_ol, void *buf, int len);
extern bool wlc_dngl_ol_bcn_delete(wlc_dngl_ol_bcn_info_t *bcn_ol);
extern void wlc_dngl_ol_bcn_clear(wlc_dngl_ol_bcn_info_t *bcn_ol, wlc_dngl_ol_info_t *wlc_dngl_ol);
extern void wlc_dngl_ol_bcn_watchdog(wlc_dngl_ol_bcn_info_t *bcn_ol);
extern void wlc_dngl_ol_reset_rssi_ma(wlc_dngl_ol_rssi_info_t *rssi_ol);
extern bool wlc_ol_check_rssi(wlc_dngl_ol_rssi_info_t *rssi_ol);

extern void wlc_dngl_ol_bcn_event_handler(
	wlc_dngl_ol_info_t	*wlc_dngl_ol,
	uint32			event,
	void			*event_data);

#endif /* _wlc_bcnol_h_ */
