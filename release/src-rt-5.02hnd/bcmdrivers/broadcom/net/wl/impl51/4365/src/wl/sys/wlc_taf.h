/*
 * wlc_taf.h
 *
 * This module contains the external definitions for the taf transmit module.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 *
 */
#if !defined(__wlc_taf_h__)
#define __wlc_taf_h__

#ifdef WLTAF
/*
 * Module attach and detach functions. This is the tip of the iceberg, visible from the outside.
 * All the rest is hidden under the surface.
 */
extern wlc_taf_info_t *wlc_taf_attach(wlc_info_t *);
extern int wlc_taf_detach(wlc_taf_info_t *);

#define TAF_PKTTAG_NUM_BITS			14
#define TAF_PKTTAG_MAX				((1 << TAF_PKTTAG_NUM_BITS) - 1)
#define TAF_MICROSEC_NUM_BITS			16
#define TAF_MICROSEC_MAX			((1 << TAF_MICROSEC_NUM_BITS) - 1)
#define TAF_MICROSEC_TO_PKTTAG_SHIFT		(TAF_MICROSEC_NUM_BITS - TAF_PKTTAG_NUM_BITS)

#define TAF_UNITS_TO_MICROSEC(a)		((a + 1) / 2)
#define TAF_MICROSEC_TO_UNITS(a)		(a * 2)

#define TAF_MICROSEC_TO_PKTTAG(a)		((a) >> TAF_MICROSEC_TO_PKTTAG_SHIFT)
#define TAF_PKTTAG_TO_MICROSEC(a)		((a) << TAF_MICROSEC_TO_PKTTAG_SHIFT)
#define TAF_PKTTAG_TO_UNITS(a)			TAF_MICROSEC_TO_UNITS(TAF_PKTTAG_TO_MICROSEC(a))
#define TAF_UNITS_TO_PKTTAG(a)			TAF_MICROSEC_TO_PKTTAG(TAF_UNITS_TO_MICROSEC(a))

#define TAF_PKTBYTES_COEFF			4096
#define TAF_PKTBYTES_TO_TIME(len, p, b) \
	(((p) + ((len) * (b)) + (TAF_PKTBYTES_COEFF / 2)) / TAF_PKTBYTES_COEFF)
#define TAF_PKTBYTES_TO_UNITS(len, p, b) \
	TAF_MICROSEC_TO_UNITS(TAF_PKTBYTES_TO_TIME(len, p, b))

typedef struct {
	uint32  emptied;
	uint32  max_did_rel_delta;
	uint32  ready;
	uint32  release_frcount;
	uint32  release_pcount;
	uint32  release_time;
	uint32  did_rel_delta;
	uint32  did_rel_time;
} taf_scheduler_tid_stats_t;

typedef struct  taf_scheduler_public {
	bool    was_emptied;
	bool    is_ps_mode;
	uint8   index;
	uint8   actual_release;
	uint16  last_release_pkttag;
	uint32  time_limit_units;
	uint32  released_units;
	uint32  released_bytes;
	uint32  total_released_units;
	uint32  byte_rate;
	uint32  pkt_rate;
#ifdef BCMDBG
	taf_scheduler_tid_stats_t* tidstats;
#endif
} taf_scheduler_public_t;

extern bool wlc_taf_enabled(wlc_taf_info_t* taf_info);
extern bool wlc_taf_rawfb(wlc_taf_info_t* taf_info);
extern uint32 wlc_taf_schedule_period(wlc_taf_info_t* taf_info, int tid);

extern uint16 wlc_taf_traffic_active(wlc_taf_info_t* taf_info, struct scb* scb);

extern bool wlc_taf_schedule(wlc_taf_info_t* taf_info,  int tid,  struct scb* scb,
                             bool force);

extern bool wlc_taf_handle_star(wlc_taf_info_t* taf_info, int tid, uint16 pkttag, uint8 index);
extern bool wlc_taf_reset_scheduling(wlc_taf_info_t* taf_info, int tid);

#endif /* WLTAF */

#endif /* __wlc_taf_h__ */
