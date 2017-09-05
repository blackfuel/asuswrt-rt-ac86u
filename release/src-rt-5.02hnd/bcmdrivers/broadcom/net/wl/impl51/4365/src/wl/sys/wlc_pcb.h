/*
 * packet tx complete callback management module interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_pcb.h 515240 2014-11-13 22:37:34Z $
 */

#ifndef _wlc_pcb_h_
#define _wlc_pcb_h_

/* module entries */
extern wlc_pcb_info_t *wlc_pcb_attach(wlc_info_t *wlc);
extern void wlc_pcb_detach(wlc_pcb_info_t *pcbi);

/* move callbacks from pkt and append them to new_pkt (for AMSDU) */
extern void wlc_pcb_fn_move(wlc_pcb_info_t *pcbi, void *new_pkt, void *pkt);

/* Set packet callback for a given packet.
 * It is called at the time a packet is added to the tx path.
 */
typedef void (*pkcb_fn_t)(wlc_info_t *wlc, uint txs, void *arg);
extern int wlc_pcb_fn_register(wlc_pcb_info_t *pcbi, pkcb_fn_t fn, void *arg, void *pkt);
/* Determine whether a callback <fn,arg> had been registered and optionally
 * cancel all matching callbacks. Returns the number of matching callbacks.
 */
extern int wlc_pcb_fn_find(wlc_pcb_info_t *pcbi, pkcb_fn_t fn, void *arg,
	bool cancel_all);

/* Set packet class callback for a class of packet.
 * It is normally called in the attach function of a module.
 * A packet is classified for a class callback by assigning a number to WLF2_PCBx_MASK bits
 * in flags2 field of the packet tag (or via WLF2_PCBx_REG()/WLF2_PCBx_UNREG() macros).
 * Currently each packet can have maximum 2 class callbacks WLF2_PCB1 and WLF2_PCB2
 * and them are invoked in the order of WLF2_PCB1 and WLF2_PCB2.
 */
typedef void (*wlc_pcb_fn_t)(wlc_info_t *wlc, void *pkt, uint txs);
extern int wlc_pcb_fn_set(wlc_pcb_info_t *pcbi, int tbl, int cls, wlc_pcb_fn_t pcb);

/* Invoke packet callback(s).
 * It is called in the tx path when the tx status is processed.
 */
extern void wlc_pcb_fn_invoke(wlc_pcb_info_t *pcbi, void *pkt, uint txs);

/* move the packet class callback chain attached from 'pkt_from' to 'pkt_to' */
extern void wlc_pcb_cb_move(wlc_pcb_info_t *pcbi, void *pkt_from, void *pkt_to);

/* register or unregister
* callback for all pkts prior to when they are freed
*/
typedef void (*wlc_pcb_pktfree_fn_t)(void *ctxt, void* pkt);

extern int wlc_pcb_pktfree_cb_register(wlc_pcb_info_t *pcbi,
	wlc_pcb_pktfree_fn_t fn, void* ctxt);
extern int wlc_pcb_pktfree_cb_unregister(wlc_pcb_info_t *pcbi,
	wlc_pcb_pktfree_fn_t fn, void* ctxt);

#endif /* _wlc_pcb_h_ */
