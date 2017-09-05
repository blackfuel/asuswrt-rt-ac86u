/*
 * Common (OS-independent) portion of
 * Broadcom 802.11 offload Driver
 *
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_eventlog_ol.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef	__WLC_OL_EVENTLOG__

#define	WLC_EL_MAX_EVENTS	64

typedef struct wlc_ol_eventlog {
	uint32	event_time;	/* timestamp in Milliseconds resolution */
	uint32	event_data;	/* Data */
	uint8	event_type;	/* enumerated upto 256 types */
} wlc_ol_eventlog_t;

typedef	struct ol_el_buf {
	uint16			count;			/* Indicate buffer is free or not */
	uint16			write_pos;		/* Current write pointer in buffer */
	uint16			read_pos;		/* Current read pointer in buffer */
	wlc_ol_eventlog_t	event_buffer[WLC_EL_MAX_EVENTS]; /* Log buffer */
} ol_el_buf_t;

enum {
	WLC_EL_BEACON_LOST,
	WLC_EL_BEACON_IE_CHANGED,
	WLC_EL_BEACON_RSSI_THRESHOLD,
	WLC_EL_RADIO_HW_DISABLED,
	WLC_EL_UNASSOC,
	WLC_EL_DEAUTH,
	WLC_EL_DISASSOC,
	WLC_EL_SCAN_BEGIN,
	WLC_EL_SCAN_END,
	WLC_EL_PREFSSID_FOUND,
	WLC_EL_CSA,
	WLC_EL_PME_ASSERTED,
#ifdef BCMDBG
	WLC_EL_TEST = 254, /* Dummy test event, useful only for testing internal builds */
#endif /* BCMDBG */
	WLC_EL_LAST
};

#define	WLC_EL_INC_READ_POS(eb) {\
	if (eb->read_pos < WLC_EL_MAX_EVENTS)\
		eb->read_pos++; \
	else\
		eb->read_pos = 0; \
	eb->count--; \
	}

#define	WLC_EL_INC_WRITE_POS(eb) {\
	if (eb->write_pos < WLC_EL_MAX_EVENTS)\
		eb->write_pos++; \
	else\
		eb->write_pos = 0; \
	eb->count++; \
	}

#define	WLC_EL_EMPTY(eb) ((eb->count == 0) ? 1 : 0)
#define	WLC_EL_FULL(eb) ((eb->count == WLC_EL_MAX_EVENTS) ? 1 : 0)

#ifdef BCM_OL_DEV
wlc_dngl_ol_eventlog_info_t *wlc_dngl_ol_eventlog_attach(wlc_dngl_ol_info_t *wlc_dngl_ol);
void	wlc_dngl_ol_eventlog_write(wlc_dngl_ol_eventlog_info_t *context, uint8 type, uint32 data);
void	wlc_dngl_ol_eventlog_send_proc(wlc_dngl_ol_eventlog_info_t *eventlog_ol, int cmd);
void	wlc_dngl_ol_eventlog_handler(wlc_dngl_ol_eventlog_info_t *eventlog_ol, uint32 event,
	void *event_data);
#endif


#endif	/* #ifndef __WLC_OL_EVENTLOG__ */
