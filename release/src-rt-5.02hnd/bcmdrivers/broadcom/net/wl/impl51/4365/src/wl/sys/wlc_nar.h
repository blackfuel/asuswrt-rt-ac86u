/*
 * wlc_nar.h
 *
 * This module contains the external definitions for the NAR transmit module.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_nar.h 597472 2015-11-05 08:17:54Z $
 *
 */

/**
 * Hooked in the transmit path at the same level as the A-MPDU transmit module, it provides balance
 * amongst MPDU and AMPDU traffic by regulating the number of in-transit packets for non-aggregating
 * stations.
 */

#if !defined(__WLC_NAR_H__)
#define __WLC_NAR_H__

/*
 * Module attach and detach functions.
 */
extern wlc_nar_info_t *wlc_nar_attach(wlc_info_t *);

extern int wlc_nar_detach(wlc_nar_info_t *);

#ifdef WLNAR
extern bool nar_check_aggregation(struct scb *scb, void *pkt, uint prec);
#endif
extern void wlc_nar_dotxstatus(wlc_nar_info_t *, struct scb *scb, void *sdu, tx_status_t *txs);

#ifdef PKTQ_LOG
extern struct pktq *wlc_nar_prec_pktq(wlc_info_t* wlc, struct scb* scb);
#endif

#ifdef PROP_TXSTATUS
extern void wlc_nar_flush_flowid_pkts(wlc_nar_info_t * nit, struct scb *scb, uint16 flowid);
#endif

#endif /* __WLC_NAR_H__ */
