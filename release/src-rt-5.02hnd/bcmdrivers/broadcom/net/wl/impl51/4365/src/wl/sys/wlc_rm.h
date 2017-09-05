/*
 * Radio Measurement related header file
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: wlc_rm.h 467328 2014-04-03 01:23:40Z $
 *
*/


#ifndef _wlc_rm_h_
#define _wlc_rm_h_

typedef struct wlc_rm_req {
	int8	type;		/* type of measurement */
	int8	flags;
	int	token;		/* token for this particular measurement */
	chanspec_t chanspec;	/* channel for the measurement */
	uint32	tsf_h;		/* TSF high 32-bits of Measurement Request start time */
	uint32	tsf_l;		/* TSF low 32-bits */
	int	dur;		/* TUs */
} wlc_rm_req_t;

typedef struct wlc_rm_req_state {
	int	report_class;	/* type of report to generate */
	bool	broadcast;	/* was the request DA broadcast */
	int	token;		/* overall token for measurement set */
	uint	step;		/* current state of RM state machine */
	chanspec_t	chanspec_return;	/* channel to return to after the measurements */
	bool	ps_pending;	/* true if we need to wait for PS to be announced before
				 * off-channel measurement
				 */
	int	dur;		/* TUs, min duration of current parallel set measurements */
	uint32	actual_start_h;	/* TSF high 32-bits of actual start time */
	uint32	actual_start_l;	/* TSF low 32-bits */
	int	cur_req;	/* index of current measure request */
	int	req_count;	/* number of measure requests */
	wlc_rm_req_t*	req;	/* array of requests */
	/* CCA measurements */
	bool	cca_active;	/* true if measurement in progress */
	int	cca_dur;	/* TU, specified duration */
	int	cca_idle;	/* idle carrier time reported by ucode */
	uint8	cca_busy;	/* busy fraction */
	/* RPI measurements */
	bool	rpi_active;	/* true if measurement in progress */
	bool	rpi_end;	/* signal last sample */
	int	rpi_dur;	/* TU, specified duration */
	int	rpi_sample_num;	/* number of samples collected */
	uint16	rpi[WL_RPI_REP_BIN_NUM];	/* rpi/rssi measurement values */
	int	rssi_sample_num; /* count of samples in averaging total */
	int	rssi_sample;	/* current sample averaging total */
#ifdef BCMCCX
	ccx_rm_t	*ccx;	/* pointer to ccx rm variables */
#endif	/* BCMCCX */
	void *		cb;	/* completion callback fn: may be NULL */
	void *		cb_arg;	/* single arg to callback function */
} wlc_rm_req_state_t;

#if defined(WLRM)
struct rm_info {
	wlc_info_t *wlc;
	/* Radio Measurement support */
	wlc_rm_req_state_t *rm_state;           /* radio measurement state */
	wl_rm_rep_elt_t *rm_ioctl_rep;          /* saved measure reports for ioctl rm requests */
	int             rm_ioctl_rep_len;       /* length of rm_ioctl_rep block */
	struct wl_timer *rm_timer;              /* 11h radio measurement timer */
	struct wl_timer *rm_rpi_timer;          /* RPI sample timer */
};
extern rm_info_t *wlc_rm_attach(wlc_info_t *wlc);
extern void wlc_rm_detach(rm_info_t *rm_info);
extern void wlc_rm_pm_pending_complete(rm_info_t *rm_info);
extern void wlc_rm_terminate(rm_info_t *rm_info);

/* Radio Measurement states */
#define WLC_RM_IDLE                     0 /* Idle */
#define WLC_RM_ABORT                    1 /* Abort */
#define WLC_RM_WAIT_START_SET           2 /* Wait Start set */
#define WLC_RM_WAIT_PREP_CHANNEL        3 /* Wait Prep Channel */
#define WLC_RM_WAIT_TX_SUSPEND          4 /* Wait Tx Suspend */
#define WLC_RM_WAIT_PS_ANNOUNCE         5 /* Wait PS Announcement */
#define WLC_RM_WAIT_BEGIN_MEAS          6 /* Wait Begin Measurement */
#define WLC_RM_WAIT_END_MEAS            7 /* Wait End Measurement */
#define WLC_RM_WAIT_END_CCA             8 /* Wait End CCA */
#ifdef BCMCCX
#define WLC_RM_WAIT_END_SCAN            9  /* Wait End Scan */
#define WLC_RM_WAIT_END_FRAME           10 /* Wait End Frame Measurement */
#define WLC_RM_WAIT_END_PATHLOSS        11 /* Wait End Pathloss Measurement */
#endif  /* BCMCCX */

#define WLC_RM_MIN_TIMER   20 /* (ms) min time for a measure */
#define WLC_RM_PREP_MARGIN 30 /* (ms) time to prepare for a measurement on a different channel */
#define WLC_RM_HOME_TIME   40 /* (ms) min time on home channel between off-channel measurements */

#define WLC_RM_NOISE_SUPPORTED(rm_info) TRUE    /* except obsolete bphy, all current phy support */
#else
#define wlc_rm_attach(wlc)      (wlc_info_t)0xdeadbeef
#define wlc_rm_detach(rm_info)  do {} while (0)
#endif /* WLRM */

/* a radio measurement is in progress unless is it IDLE, ABORT, or
 * waiting to start or channel switch for a set
 */
#if defined(STA) && defined(WLRM)
#define WLC_RM_IN_PROGRESS(wlc) ((wlc)->rm_info && \
		(!(((wlc)->rm_info)->rm_state->step == WLC_RM_IDLE || \
		((wlc)->rm_info)->rm_state->step == WLC_RM_ABORT ||	\
		((wlc)->rm_info)->rm_state->step == WLC_RM_WAIT_PREP_CHANNEL || \
		((wlc)->rm_info)->rm_state->step == WLC_RM_WAIT_START_SET)))
#else
#define WLC_RM_IN_PROGRESS(wlc) FALSE
#endif /* defined(STA) && defined(WLRM) */

#endif	/* _wlc_rm_h_ */
