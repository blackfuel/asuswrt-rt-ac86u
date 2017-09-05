/*
 * MU-MIMO transmit module for Broadcom 802.11 Networking Adapter Device Drivers
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_mutx.h 654068 2016-08-11 01:23:43Z $
 */

#ifndef _wlc_mutx_h_
#define _wlc_mutx_h_

#include <typedefs.h>
#include <wlc_types.h>
#include <wlc_mumimo.h>

/* Standard attach and detach functions */
wlc_mutx_info_t* BCMATTACHFN(wlc_mutx_attach)(wlc_info_t *wlc);
void BCMATTACHFN(wlc_mutx_detach)(wlc_mutx_info_t *mu_info);

/* Other public APIs. */
uint16 wlc_mutx_sta_client_index(wlc_mutx_info_t *mu_info, struct scb *scb);
bool wlc_mutx_is_group_member(wlc_mutx_info_t *mu_info, struct scb *scb, uint16 group_id);
bool wlc_mutx_is_muclient(wlc_mutx_info_t *mu_info, struct scb *scb);
void wlc_mutx_update_vht_cap(wlc_mutx_info_t *mu_info, struct scb *scb);

/* Call this API when a configuration or state change may affect whether MU TX can be active */
void wlc_mutx_active_update(wlc_mutx_info_t *mu_info);

/* APIs to set and get MU group membership */
void wlc_mutx_membership_clear(wlc_mutx_info_t *mu_info, uint16 client_index);
int wlc_mutx_membership_get(wlc_mutx_info_t *mu_info, uint16 client_index,
	uint8 *membership, uint8 *position);
int wlc_mutx_membership_set(wlc_mutx_info_t *mu_info, uint16 client_index,
                            uint8 *membership, uint8 *position);

#ifdef WL_MUPKTENG
extern uint8 wlc_mutx_pkteng_on(wlc_mutx_info_t *mu_info);
#endif
#ifdef WLCNT
void wlc_mutx_update_txcounters(wlc_mutx_info_t *mu_info, struct scb *scb,
      ratespec_t rspec, bool is_mu, bool txs_mu, uint16 ncons, uint16 nlost,
      uint8 gid, tx_status_t *txs);
#endif /* WLCNT */
void wlc_mutx_sta_txfifo(wlc_mutx_info_t *mu_info, struct scb *scb, uint *pfifo);
int wlc_mutx_ntxd_adj(wlc_info_t *wlc, uint fifo, uint *out_ntxd, uint *out_ntxd_aqm);

#if defined(BCMDBG) || defined(BCMDBG_MU)
extern uint8 wlc_mutx_on(wlc_mutx_info_t *mu_info);
#endif
void wlc_mutx_update(wlc_info_t *wlc, bool enable);
int wlc_mutx_switch(wlc_info_t *wlc, bool mutx_feature, bool is_iov);
#ifdef WL_PSMX
void wlc_mutx_hostflags_update(wlc_info_t *wlc);
#endif /* WL_PSMX */
uint32 wlc_mutx_bw_policy_update(wlc_mutx_info_t *mu_info, wlc_bsscfg_t *bsscfg, bool force);
bool wlc_mutx_sta_on_hold(wlc_mutx_info_t *mu_info, struct scb *scb);
bool wlc_mutx_sta_mu_link_permit(wlc_mutx_info_t *mu_info, struct scb *scb);
bool wlc_mutx_sta_ac_check(wlc_mutx_info_t *mu_info, struct scb *scb);
#if defined(WL_MU_TX)
#ifdef WLCNT
void BCMFASTPATH wlc_mutx_upd_interm_counters(wlc_mutx_info_t *mu_info,
    struct scb *scb, tx_status_t *txs);
#endif
extern void wlc_mutx_txfifo_complete(wlc_info_t *wlc);
extern uint8 wlc_mutx_get_muclient_nrx(wlc_mutx_info_t *mu_info);
#endif /* defined(WL_MU_TX) */

/* mutx state bitfield */
#define MUTX_OFF	0
#define MUTX_ON		1

typedef struct {
	uint8	state;
} mutx_state_upd_data_t;

#ifdef WL_MU_TX
extern int wlc_mutx_state_upd_register(wlc_info_t *wlc, bcm_notif_client_callback fn, void *arg);
extern int wlc_mutx_state_upd_unregister(wlc_info_t *wlc, bcm_notif_client_callback fn, void *arg);
#else
#define wlc_mutx_state_upd_register(wlc, fn, arg) (0)
#define wlc_mutx_state_upd_unregister(wlc, fn, arg) (0)
#endif

#endif   /* _wlc_mutx_h_ */
