/*
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
* $Id: wl_wlfc.h 501429 2014-09-09 01:32:20Z $
*
*/
#ifndef __wlfc_wl_definitions_h__
#define __wlfc_wl_definitions_h__



#define WLFC_PENDING_TRIGGER_WATERMARK 48

#define WLFC_SENDUP_TIMER_INTERVAL 10

typedef struct wlfc_fw_stats {
	uint32	packets_from_host;
	uint32	txstatus_count;
	uint32	txstats_other;
} wlfc_fw_stats_t;

typedef struct wlfc_fw_dbgstats {
	/* how many header only packets are allocated */
	uint32	nullpktallocated;
	uint32	realloc_in_sendup;
	uint32	wlfc_wlfc_toss;
	uint32	wlfc_wlfc_sup;
	uint32	wlfc_to_D11;
	uint32	wlfc_pktfree_except;
	uint32	creditupdates;
	uint32	creditin;
	uint32	nost_from_host;
	uint32	sig_from_host;
	uint32	credits[NFIFO];
} wlfc_fw_dbgstats_t;

#ifdef PROP_TXSTATUS_DEBUG
#define WLFC_COUNTER_TXSTATUS_TOD11(wlc)	do \
	{(wlfc_state_get((wlc)->wl))->dbgstats->wlfc_to_D11++;} while (0)
#define WLFC_COUNTER_TXSTATUS_WLCTOSS(wlc)	do \
	{(wlfc_state_get((wlc)->wl))->dbgstats->wlfc_wlfc_toss++;} while (0)
#else
#define WLFC_COUNTER_TXSTATUS_TOD11(wlc)	do {} while (0)
#define WLFC_COUNTER_TXSTATUS_WLCTOSS(wlc)	do {} while (0)
#endif

#define WLFC_COUNTER_TXSTATUS_COUNT(wlc)	do \
	{(wlfc_state_get((wlc)->wl))->stats.txstatus_count++;} while (0)
#define WLFC_COUNTER_TXSTATUS_OTHER(wlc)	do \
	{(wlfc_state_get((wlc)->wl))->stats.txstats_other++;} while (0)

/* used at the firmware */
typedef struct wlfc_mac_desc_handle_map {
	/* max 32 spaces, each bit indicates
	availability, taken if set to 1
	*/
	uint32	bitmap;
	/* only 3 bits are used */
	uint8	replay_counter;
} wlfc_mac_desc_handle_map_t;

struct wl_info;

typedef struct wlfc_info_state {
	uint16	pending_datalen;
	uint8	data[WLFC_MAX_PENDING_DATALEN];
	uint8	fifo_credit[WLFC_CTL_VALUE_LEN_FIFO_CREDITBACK];
	uint8	fifo_credit_threshold[WLFC_CTL_VALUE_LEN_FIFO_CREDITBACK];
	uint8	fifo_credit_back[WLFC_CTL_VALUE_LEN_FIFO_CREDITBACK];
	uint8	fifo_credit_in[WLFC_CTL_VALUE_LEN_FIFO_CREDITBACK];
	uint8	fifo_credit_back_pending;
	wlfc_fw_stats_t	stats;
	uint8	timer_started;
	struct wl_timer*	fctimer;
	struct wl_info*		wl_info;
	uint32	compressed_stat_cnt;
	uint8	total_credit;
	uint8	wlfc_trigger;
	uint8	wlfc_fifo_bo_cr_ratio;
	uint8	wlfc_comp_txstatus_thresh;
	uint16	pending_datathresh;
	uint8	txseqtohost;
	uint8	totalcredittohost;
	wlfc_fw_dbgstats_t *dbgstats;
} wlfc_info_state_t;

#define WLFC_FLAGS_XONXOFF_MASK \
	((1 << WLFC_CTL_TYPE_MAC_OPEN) | \
	(1 << WLFC_CTL_TYPE_MAC_CLOSE) | \
	(1 << WLFC_CTL_TYPE_MACDESC_ADD) | \
	(1 << WLFC_CTL_TYPE_MACDESC_DEL) | \
	(1 << WLFC_CTL_TYPE_INTERFACE_OPEN) | \
	(1 << WLFC_CTL_TYPE_INTERFACE_CLOSE))

#define WLFC_FLAGS_CREDIT_STATUS_MASK \
	((1 << WLFC_CTL_TYPE_FIFO_CREDITBACK) | \
	(1 << WLFC_CTL_TYPE_MAC_REQUEST_CREDIT) | \
	(1 << WLFC_CTL_TYPE_MAC_REQUEST_PACKET) | \
	(1 << WLFC_CTL_TYPE_TXSTATUS))

#define WLFC_FLAGS_PKT_STAMP_MASK \
	((1 << WLFC_CTL_TYPE_RX_STAMP) | \
	(1 << WLFC_CTL_TYPE_TX_ENTRY_STAMP))

#define WLFC_DEFAULT_FWQ_DEPTH			1

#define WLFC_CREDIT_TRIGGER			1
#define WLFC_TXSTATUS_TRIGGER			2

#endif /* __wlfc_wl_definitions_h__ */
