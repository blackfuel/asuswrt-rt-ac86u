/**
 * Required functions exported by the wlc_aibss.c to common (os-independent) driver code.
 *
 * Advanced IBSS mode
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_aibss.h 498934 2014-08-26 22:46:26Z $
 */

#ifndef _WLC_AIBSS_H_
#define _WLC_AIBSS_H_

#ifdef	WLAIBSS

typedef struct aibss_scb_info {
	int32	tx_noack_count;
	uint16	no_bcn_counter;
	uint16	bcn_count;
	bool	pkt_pend;
	bool	atim_acked;
	bool	atim_rcvd;
	uint16	atim_failure_count;
} aibss_scb_info_t;

struct wlc_aibss_info {
	int32 scb_handle;		/* SCB CUBBY OFFSET */
};

enum aibss_peer_txfail {
	AIBSS_TX_FAILURE = 0,
	AIBSS_BCN_FAILURE = 1,
	AIBSS_ATIM_FAILURE = 2
};

/* access the variables via a macro */
#define WLC_AIBSS_INFO_SCB_HDL(a) ((a)->scb_handle)

extern wlc_aibss_info_t *wlc_aibss_attach(wlc_info_t *wlc);
extern void wlc_aibss_detach(wlc_aibss_info_t *aibss);
extern void wlc_aibss_check_txfail(wlc_aibss_info_t *aibss, wlc_bsscfg_t *cfg, struct scb *scb);
extern void wlc_aibss_tbtt(wlc_aibss_info_t *aibss);
extern bool wlc_aibss_sendpmnotif(wlc_aibss_info_t *aibss, wlc_bsscfg_t *cfg,
	ratespec_t rate_override, int prio, bool track);
extern void wlc_aibss_atim_window_end(wlc_info_t *wlc);
extern void wlc_aibss_ps_start(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_aibss_ps_stop(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_aibss_ps_process_atim(wlc_info_t *wlc, struct ether_addr *ea);
#else
#define wlc_aibss_atim_window_end(a)	do {} while (0)
#define wlc_aibss_ps_start(a, b)	do {} while (0)
#define wlc_aibss_ps_stop(a, b)	do {} while (0)
#define wlc_aibss_ps_process_atim(a, b)	do {} while (0)
#endif /* WLAIBSS */
#endif /* _WLC_AIBSS_H_ */
