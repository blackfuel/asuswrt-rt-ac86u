/*
 * Broadcom 802.11 L2 keepalive offload Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_l2keepaliveol.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wlc_l2keepaliveol_h_
#define _wlc_l2keepaliveol_h_

extern wlc_dngl_ol_l2keepalive_info_t *
wlc_dngl_ol_l2keepalive_attach(wlc_dngl_ol_info_t *wlc_dngl_ol);
extern void wlc_dngl_ol_l2keepalive_send_proc(wlc_dngl_ol_l2keepalive_info_t *l2keepalive_ol,
	void *buf, int len);
extern void wlc_l2_keepalive_timer_start(wlc_dngl_ol_info_t *wlc_dngl_ol, bool restart);
extern void wlc_l2_keepalive_event_handler(wlc_dngl_ol_info_t *wlc_dngl_ol, uint32 event,
	void *event_data);
extern uint8 wlc_l2_keepalive_get_flags(wlc_dngl_ol_info_t *wlc_dngl_ol);
#endif /* L2KEEPALIVE_OFFLOADS */
