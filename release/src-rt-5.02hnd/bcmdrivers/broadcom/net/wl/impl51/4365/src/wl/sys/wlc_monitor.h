/*
 * Monitor moude interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_monitor.h 496606 2014-08-13 18:17:24Z $
 */


#ifndef _WLC_MONITOR_H_
#define _WLC_MONITOR_H_

#define MONITOR_PROMISC_ENAB(_ctxt_) \
	wlc_monitor_get_mctl_promisc_bits((_ctxt_))

extern wlc_monitor_info_t *wlc_monitor_attach(wlc_info_t *wlc);
extern void wlc_monitor_detach(wlc_monitor_info_t *ctxt);
extern void wlc_monitor_promisc_enable(wlc_monitor_info_t *ctxt, bool enab);
extern uint32 wlc_monitor_get_mctl_promisc_bits(wlc_monitor_info_t *ctxt);
extern void wlc_monitor_phy_cal(wlc_monitor_info_t *ctx, bool enable);
extern void wlc_monitor_phy_cal_timer_start(wlc_monitor_info_t *ctxt, uint32 tm);
#endif /* _WLC_MONITOR_H_ */
