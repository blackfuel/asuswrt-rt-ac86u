/*
 * TCP offload support in tgAC MAC
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_tso.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_tso_h_
#define _wlc_tso_h_

#include <proto/802.3.h>
#include <proto/bcmip.h>
#include <proto/bcmudp.h>
#include <proto/bcmtcp.h>
#include <proto/ethernet.h>
#include <proto/vlan.h>

#define IPV6_HLEN(iph)		40

/* a structure to capture both the current pkt and data ptr in the pkt chain */
typedef struct pkt_data_s {
	void * cur_p;
	void * cur_d;
} pkt_data_ptr_t;

#ifdef WLCSO
extern bool wlc_tso_support(wlc_info_t* wlc);
extern void wlc_set_tx_csum(wlc_info_t* wlc, uint32 on_off);
#endif /* WLCSO */

extern void wlc_toe_add_hdr(wlc_info_t *wlc, void *p, struct scb *scb,
	const struct wlc_key_info *key_info, uint nfrags, uint16 *pushlen);
extern uint wlc_tso_hdr_length(d11ac_tso_t* tso);

#endif /* _wlc_tso_h_ */
