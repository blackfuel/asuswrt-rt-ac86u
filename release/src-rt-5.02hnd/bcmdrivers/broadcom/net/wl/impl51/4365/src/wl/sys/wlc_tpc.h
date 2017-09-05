/*
 * 802.11h TPC and wl power control module header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_tpc.h 467328 2014-04-03 01:23:40Z $
*/


#ifndef _wlc_tpc_h_
#define _wlc_tpc_h_

/* APIs */
#ifdef WLTPC

/* module */
extern wlc_tpc_info_t *wlc_tpc_attach(wlc_info_t *wlc);
extern void wlc_tpc_detach(wlc_tpc_info_t *tpc);

/* utilities */
extern void wlc_tpc_rep_build(wlc_info_t *wlc, int8 rssi, ratespec_t rspec,
	dot11_tpc_rep_t *rpt);

/* action frame recv/send */
extern void wlc_recv_tpc_request(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len,
	int8 rssi, ratespec_t rspec);
extern void wlc_recv_tpc_report(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len,
	int8 rssi, ratespec_t rspec);

extern void wlc_send_tpc_request(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg, struct ether_addr *da);
extern void wlc_send_tpc_report(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	uint8 token, int8 rssi, ratespec_t rspec);

#ifdef WL_AP_TPC
extern void wlc_ap_tpc_assoc_reset(wlc_tpc_info_t *tpc, struct scb *scb);
extern void wlc_ap_bss_tpc_setup(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg);
#else
#define wlc_ap_tpc_assoc_reset(tpc, scb) do {} while (0)
#define wlc_ap_bss_tpc_setup(tpc, cfg) do {} while (0)
#endif

/* power management */
extern void wlc_tpc_reset_all(wlc_tpc_info_t *tpc);
extern uint8 wlc_tpc_get_local_constraint_qdbm(wlc_tpc_info_t *tpc);

/* accessors */
extern void wlc_tpc_set_local_max(wlc_tpc_info_t *tpc, uint8 pwr);

#else /* !WLTPC */

#define wlc_tpc_attach(wlc) NULL
#define wlc_tpc_detach(tpc) do {} while (0)

#define wlc_tpc_rep_build(wlc, rssi, rspec, tpc_rep) do {} while (0)

#define wlc_recv_tpc_request(tpc, cfg, hdr, body, body_len, rssi, rspec) do {} while (0)
#define wlc_recv_tpc_report(tpc, cfg, hdr, body, body_len, rssi, rspec) do {} while (0)

#define wlc_send_tpc_request(tpc, cfg, da) do {} while (0)
#define wlc_send_tpc_report(tpc, cfg, da, token, rssi, rspec) do {} while (0)

#define wlc_ap_tpc_assoc_reset(tpc, scb) do {} while (0)

#define wlc_tpc_reset_all(tpc) do {} while (0)
#define wlc_tpc_set_local_constraint(tpc, pwr) do {} while (0)
#define wlc_tpc_get_local_constraint_qdbm(tpc) 0

#define wlc_tpc_set_local_max(tpc, pwr) do {} while (0)

#endif /* !WLTPC */

#endif /* _wlc_tpc_h_ */
