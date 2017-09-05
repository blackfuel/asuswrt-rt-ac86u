/*
 * Broadcom 802.11 host offload module
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_offloads.h 474262 2014-04-30 17:03:14Z $
 */


#ifndef _WL_OFFLOADS_H_
#define _WL_OFFLOADS_H_

#ifdef WLOFFLD
#include <bcm_ol_msg.h>

#define OL_CFG_MASK         0x1	/* For configuration */
#define OL_SCAN_MASK        0x2	/* For SCANNING */
#define OL_TBTT_MASK        0x4	/* For unaligned TBTT */
#define OL_AWAKEBCN_MASK    0x8	/* Stay Awake for Beacon */
#define OL_CSA_MASK         0x10 /* CSA IE Mask */
#define OL_RM_SCAN_MASK     0x20 /* Rm scan */

/* Whether offload capable or not */
extern bool wlc_ol_cap(wlc_info_t *wlc);

/* Offload module attach */
extern wlc_ol_info_t * wlc_ol_attach(wlc_info_t *wlc);

/* Offload module detach */
extern void wlc_ol_detach(wlc_ol_info_t *ol);
extern int wlc_ol_up(void *hdl);
extern int wlc_ol_down(void *hdl);
extern void wlc_ol_clear(wlc_ol_info_t *ol);
extern void wlc_ol_restart(wlc_ol_info_t *ol);

/* Returns true of the interrupt is from CR4 */
extern bool wlc_ol_intstatus(wlc_ol_info_t *ol);
/* DPC */
extern void wlc_ol_dpc(wlc_ol_info_t *ol);
extern void wlc_ol_mb_poll(wlc_ol_info_t *ol);
extern void wlc_ol_enable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg);
extern void wlc_ol_disable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg);
extern void wlc_ol_rx_deferral(wlc_ol_info_t *ol, uint32 mask, uint32 val);
extern bool wlc_ol_bcn_is_enable(wlc_ol_info_t *ol);
extern uint16 wlc_ol_get_state(wlc_ol_info_t *ol);
extern bool wlc_ol_time_since_bcn(wlc_ol_info_t *ol);
extern bool wlc_ol_arp_nd_enabled(wlc_ol_info_t *ol);
extern void wlc_ol_enable_intrs(wlc_ol_info_t *ol, bool enable);
extern bool wlc_ol_chkintstatus(wlc_ol_info_t *ol);
extern void wlc_ol_armtx(wlc_ol_info_t *ol, uint8 txEnable);
extern void wlc_ol_armtxdone(wlc_ol_info_t *ol, void *msg);
extern void wlc_set_tsf_update(wlc_ol_info_t *ol, bool value);
extern bool wlc_get_tsf_update(wlc_ol_info_t *ol);
extern bool wlc_ol_get_arm_txtstatus(wlc_ol_info_t *ol);
extern int wlc_ol_wowl_enable_start(
		    wlc_ol_info_t *ol,
		    wlc_bsscfg_t *cfg,
		    olmsg_wowl_enable_start *wowl_enable_start,
		    uint wowl_enable_len);

extern void wlc_ol_arm_halt(wlc_ol_info_t *ol);
extern bool wlc_ol_is_arm_halted(wlc_ol_info_t *ol);
extern int wlc_ol_wowl_enable_complete(wlc_ol_info_t *ol);
extern int wlc_ol_wowl_disable(wlc_ol_info_t *ol);
extern int wlc_ol_l2keepalive_enable(wlc_ol_info_t *ol);
extern int
wlc_ol_gtk_enable(wlc_ol_info_t *ol,
	rsn_rekey_params *rkey, struct scb *scb, int wpaauth);
void wlc_ol_rscupdate(wlc_ol_info_t *ol, void *rkey, void *msg);

void wlc_ol_update_sec_info(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg,
    scb_t *scb, ol_sec_info *info);

#ifdef WL_LTR
extern void wlc_ol_ltr(wlc_ol_info_t *ol, wlc_ltr_info_t *ltr_info);
#endif /* WL_LTR */

#ifdef UCODE_SEQ
#if defined(WLOFFLD) || defined(SCANOL)
/* This function tell whether the BK tid seq/iv needs to be
 * copied from shared memory (frmShm)
 */
extern void wlc_ol_update_seq_iv(wlc_info_t *wlc, bool frmShm, struct scb *scb);
#endif
#endif /* UCODE_SEQ */

extern int8 wlc_ol_rssi_get_value(wlc_ol_info_t *ol);
extern int8 wlc_ol_rssi_get_ant(wlc_ol_info_t *ol, uint32 ant_idx);
extern void wlc_ol_inc_rssi_cnt_arm(wlc_ol_info_t *ol);
extern void wlc_ol_inc_rssi_cnt_host(wlc_ol_info_t *ol);
extern void wlc_ol_inc_rssi_cnt_events(wlc_ol_info_t *ol);
extern void wlc_ol_curpwr_upd(wlc_ol_info_t *ol, int8 target_max_txpwr, chanspec_t chanspec);
extern void wlc_ol_print_cons(wlc_ol_info_t *ol);

/* Event log mechanism during sleep mode handlers and registration */
typedef void (*wlc_eventlog_print_handler_fn_t)(wlc_ol_info_t *ol, struct bcmstrbuf *b,
	uint8 type, uint32 time, uint32 data);
void	wlc_eventlog_register_print_handler(wlc_ol_info_t *ol, uint8 type,
	wlc_eventlog_print_handler_fn_t fn);

/* Definitions for chip memory space allocation */
#define OL_RAM_BASE_4360        0
#define OL_RAM_BASE_4350        0x180000
#define OL_RAM_BASE_43602       0x180000
#define OL_TEXT_START_4360      0
#define OL_TEXT_START_4350      0x180800
#define OL_TEXT_START_43602     0x180800

#endif /* WLOFFLD */

#endif /* _WL_OFFLOADS_H_ */
