/*
 * Common OS-independent driver header file for power selection
 * algorithm of Broadcom 802.11b DCF-only Networking Adapter.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * $Id: wlc_power_sel.h 569789 2015-07-09 02:09:43Z $
 */

#ifndef	_WLC_POWER_SEL_H_
#define	_WLC_POWER_SEL_H_

typedef struct lcb lcb_t;

struct rate_lcb_info {
	bool	tpr_good;
	bool	tpr_good_valid;
	bool	la_good;
	bool	la_good_valid;
	uint8	tpr_thresh;
	uint32	hi_rate_kbps;
#ifdef WL_LPC_DEBUG
	uint8	tpr_val;
#endif
};

/* Debug log settings */
#define LPC_MSG_INFO_VAL 0x01
#define LPC_MSG_MORE_VAL 0x02
extern uint8 lpc_msglevel;

#ifdef WL_LPC_DEBUG
#define LPC_INFO(args) \
	do {if (WL_LPC_ON() && (lpc_msglevel & LPC_MSG_INFO_VAL)) \
	printf args;} while (0)
#define LPC_MORE(args) \
	do {if (WL_LPC_ON() && (lpc_msglevel & LPC_MSG_MORE_VAL)) \
	printf args;} while (0)
#else
#define LPC_INFO(args)
#define LPC_MORE(args)
#endif

extern lpc_info_t *BCMATTACHFN(wlc_lpc_attach)(wlc_info_t *wlc);
extern void BCMATTACHFN(wlc_lpc_detach)(struct lpc_info *lpci);
#ifdef BCMDBG
extern void wlc_lpc_dump_lcb(lcb_t *lcb, int32 ac, struct bcmstrbuf *b);
#endif
extern bool wlc_lpc_capable_chip(wlc_info_t *wlc);
extern void wlc_lpc_init(lpc_info_t *lpci, lcb_t *state, struct scb *scb);
extern uint8 wlc_lpc_getcurrpwr(lcb_t *state);
extern void wlc_lpc_update_pwr(lcb_t *state, uint8 ac, uint16 phy_ctrl_word);
extern void wlc_lpc_store_pcw(lcb_t *state, uint16 phy_ctrl_word);
extern int wlc_lpc_lcb_sz(void);
#endif	/* _WLC_POWER_SEL_H_ */
