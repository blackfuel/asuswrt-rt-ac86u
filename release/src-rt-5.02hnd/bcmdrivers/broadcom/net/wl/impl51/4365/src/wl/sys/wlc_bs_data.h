/*
 * wlc_bs_data.h
 *
 * This module provides definitions for the Band Steering Daemon "bs_data" IOVAR functionality.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */


#if !defined(__wlc_bs_data_h__)
#define __wlc_bs_data_h__

#if defined(SCB_BS_DATA)
/*
 * BS_DATA Counter structure.
 *
 * This structure holds a number of counters which are of use to the Band Steering Daemon.
 *
 * It is allocated for any particular SCB on demand, that is, on the first invocation of the
 * bs_data IOVAR which queries the SCB BS_DATA counters.
 */
typedef struct {
	/* The following counters are a subset of what pktq_stats provides per precedence. */
	uint32 retry;          /* packets re-sent because they were not received */
	uint32 retry_drop;     /* packets finally dropped after retry limit */
	uint32 rtsfail;        /* count of rts attempts that failed to receive cts */
	uint32 acked;          /* count of packets sent (acked) successfully */
	uint32 txrate_succ;    /* running total of phy rate of packets sent successfully */
	uint32 txrate_main;    /* running total of phy 'main' rate */
	uint32 throughput;     /* actual data transferred successfully */
	uint32 time_delta;     /* time difference since last pktq_stats */
	uint32 airtime;        /* cumulative total medium access delay in useconds */
} wlc_bs_data_counters_t;

extern wlc_bs_data_info_t *BCMATTACHFN(wlc_bs_data_attach)(wlc_info_t *);
extern int BCMATTACHFN(wlc_bs_data_detach)(wlc_bs_data_info_t *);

extern wlc_bs_data_counters_t *wlc_bs_data_counters(struct wlc_info *, struct scb *);

#endif /* SCB_BS_DATA */

#endif /* __wlc_bs_data_h__ */
