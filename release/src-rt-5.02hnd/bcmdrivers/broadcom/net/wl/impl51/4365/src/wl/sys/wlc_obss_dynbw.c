/*
 * OBSS Dynamic bandwidth switch support
 * Broadcom 802.11 Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wlc_obss_dynbw.c 648489 2016-07-12 13:50:10Z $
 */

/**
 * @file
 * @brief
 * out of band BSS bandwidth switch
 */

#include <wlc_types.h>
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_export.h>
#include <wlc_assoc.h>
#include <wlc_bmac.h>
#include <wlc_vht.h>
#include <wlc_modesw.h>
#include <wlc_lq.h>
#include <wlc_obss_util.h>
#include <wlc_obss_dynbw.h>
#ifdef WLCSA
#include <wlc_csa.h>
#endif

#define OBSS_DYNBW_DELTASTATS(result, curr, prev) \
	result = (curr - prev);

/* OBSS protection trigger for RX CRS Primary */
#define OBSS_DUR_RXCRS_PRI_THRESHOLD_DEFAULT			20

/* '70' is to check whether ibss is less than 70% of rxcrs primary */
#define OBSS_RATIO_RXCRS_PRI_VS_IBSS_DEFAULT			70

#define OBSS_BWSW_NO_ACTIVITY_CFM_PERIOD_DEFAULT		(6)	/* in seconds */
#define OBSS_BWSW_NO_ACTIVITY_CFM_PERIOD_INCR_DEFAULT		(5)	/* in seconds,
				* see explanation of obss_bwsw_no_activity_cfm_count_cfg
				*/
#define OBSS_BWSW_NO_ACTIVITY_MAX_INCR_DEFAULT			(30) /* in seconds,
				* see explanation of obss_bwsw_no_activity_cfm_count_cfg
				*/

#define OBSS_BWSW_ACTIVITY_CFM_PERIOD_DEFAULT			(3)	/* in seconds */
#define OBSS_BWSW_PSEUDO_SENSE_PERIOD_DEFAULT			(500) /* in msecs */
#define OBSS_BWSW_DUR_THRESHOLD_DEFAULT				15	/* OBSS DYN BWSW
									* trigger for RX CRS Sec
									*/
/* txop limit to trigger bw downgrade */
#define OBSS_TXOP_THRESHOLD_DEFAULT				12

/*
 * module states layout
 */

struct wlc_obss_dynbw {
	uint8 dyn_bwsw_enabled;
	wlc_info_t *wlc;		/* pointer to main wlc structure */

	int cfgh;   /* bsscfg cubby handle */
	uint32 obss_active_cnt; /* global: non-zero value indicates that at least
							* one bss has obss active
							*/
	chanspec_t main_chanspec; /* value to save the original chanspec */
	/* Pointers to internal structures */
	wlc_obss_dynbwsw_config_t *cfg_dbs;
	/* ucode previous and current stat counters */
	wlc_bmac_obss_counts_t *prev_stats;
	wlc_bmac_obss_counts_t *curr_stats;
	/* Cummulative stat counters */
	wlc_bmac_obss_counts_t *total_stats;
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* For diagnostic measurements */
	wlc_bmac_obss_counts_t *msrmnt_stored;
	cca_stats_n_flags *results;
#endif 
};

#define WLC_OBSS_DYNBW_SIZE (sizeof(wlc_obss_dynbw_t))

#define WLC_OBSS_DYNBW_STATIC_SIZE (sizeof(wlc_obss_dynbwsw_config_t)\
		+  3 * sizeof(wlc_bmac_obss_counts_t))

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#define WLC_OBSS_DYNBW_DEBUG_SIZE (sizeof(wlc_bmac_obss_counts_t)\
		+ sizeof(cca_stats_n_flags))
#endif 

/* Private cubby struct for protOBSS */
typedef struct {
	uint8 obss_activity;	/* obss activity: inactive, ht or non-ht */
	chanspec_t orig_chanspec;  /* original assoc[STA]/start[AP] chanspec */
	uint16 bwswpsd_state;	/* possible states: BWSW_PSEUDOUPGD_XXX above */
	chanspec_t bwswpsd_cspec_for_tx;	/* If we are in pseudo phy
		* cspec upgraded state, tx pkts should use this overridden cspec.
		* Also, if we find that pseudo upgrade doesn't solve obss issue, we
		* need to return to this cspec.
		*/
	uint8 obss_bwsw_no_activity_cfm_count;	/* # of secs of OBSS inactivity
		* for bwswitch. [checked against obss_bwsw_no_activity_cfm_count_max]
		*/
	uint8 obss_bwsw_no_activity_cfm_count_max; /* initial value:
		* obss_bwsw_no_activity_cfm_count_cfg and later incremented in steps
		* of obss_bwsw_no_activity_cfm_count_incr_cfg, if required.
		*/
	uint8 obss_bwsw_activity_cfm_count; /* after we detect OBSS using stats, we
		* will confirm it by waiting for # of secs before we BWSW. This field
		* is to count that. [checked against obss_bwsw_activity_cfm_count_cfg]
		*/
	uint16 obss_bwsw_pseudo_sense_count; /* number of seconds/cnt to be in
		* pseudo state. This is used to sense/measure the stats from lq.
		*/
	bool is_obss_modesw_call; /* To identify if DBS issued modesw calls. Used to
		* decide if callbacks from modesw need to processed by DBS
		*/
	bool is_csa_lock;
} dynbw_bsscfg_cubby_t;

typedef struct wlc_dynbw_cb_ctx {
	uint16		connID;
} wlc_dynbw_cb_ctx_t;

/* bsscfg specific info access accessor */
#define DYNBW_BSSCFG_CUBBY_LOC(dynbwsw, cfg) \
	((dynbw_bsscfg_cubby_t **)BSSCFG_CUBBY((cfg), (dynbwsw)->cfgh))
#define DYNBW_BSSCFG_CUBBY(dynbwsw, cfg) (*(DYNBW_BSSCFG_CUBBY_LOC(dynbwsw, cfg)))

#define DYNBW_BSS_MATCH(wlc, cfg)			\
	(   /* for assoc STA's... */			\
		((BSSCFG_STA(cfg) && cfg->associated) ||	\
		/* ... or UP AP's ... */				\
		(BSSCFG_AP(cfg) && cfg->up)) &&		\
		/* ... AND chanspec matches currently measured chanspec */	\
		(cfg->current_bss->chanspec == wlc->chanspec))

enum {
	OBSS_INACTIVE = 0,
	OBSS_ACTIVE_HT = 1,
	OBSS_ACTIVE_NONHT = 2
};

enum bwsw_pseudoupgd_states {
	BWSW_PSEUDOUPGD_NOTACTIVE = 0,
	BWSW_PSEUDOUPGD_PENDING = 1,
	BWSW_PSEUDOUPGD_ACTIVE = 2
};

/* OBSS BWSW enable enums */
enum {
	OBSS_DYN_BWSW_DISABLE = 0,
	OBSS_DYN_BWSW_ENAB_RXCRS = 1,
	OBSS_DYN_BWSW_ENAB_TXOP = 2
};

static const bcm_iovar_t wlc_obss_dynbw_iovars[] = {
	{"obss_dyn_bw", IOV_OBSS_DYN_BWSW_ENAB,
	(IOVF_SET_DOWN), IOVT_UINT8, 0
	},
	{"dyn_bwsw_params", IOV_OBSS_DYN_BWSW_PARAMS,
	(0), IOVT_BUFFER, sizeof(obss_config_params_t),
	},
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	{"dump_obss_dyn_bwsw", IOV_OBSS_DYN_BWSW_DUMP,
	(IOVF_GET_UP), IOVT_BUFFER, WLC_IOCTL_MAXLEN,
	},
#endif 
	{NULL, 0, 0, 0, 0}
};

static int wlc_obss_dynbw_manage_bw_switch(wlc_obss_dynbw_t *prot);
static int wlc_obss_dynbw_bssinfo_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_obss_dynbw_bssinfo_deinit(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_obss_dynbw_modesw_cb(void *ctx, wlc_modesw_notif_cb_data_t *notif_data);
static int wlc_obss_dynbw_init_psdstat_cb(wlc_obss_dynbw_t *prot, wlc_bsscfg_t *cfg);
static void wlc_obss_dynbw_set_obss_activity_info(wlc_obss_dynbw_t *prot,
	wlc_bsscfg_t * bsscfg, dynbw_bsscfg_cubby_t *po_bss);
static int wlc_obss_dynbw_bwswpsd_statscb(wlc_info_t *wlc, void *ctx,
	uint32 elapsed_time, void *vstats);
static void wlc_obss_dynbw_updown_cb(void *ctx, bsscfg_up_down_event_data_t *updown_data);
static void wlc_obss_dynbw_assoc_cxt_cb(void *ctx, bss_assoc_state_data_t *notif_data);
static bool wlc_obss_dynbw_interference_detected_for_bwsw(wlc_obss_dynbw_t *prot,
	wlc_bmac_obss_counts_t *delta_stats);
static int wlc_obss_dynbw_doiovar(void *hdl, const bcm_iovar_t *vi,
	uint32 actionid, const char *name, void *p, uint plen, void *a,
	int alen, int val_size, struct wlc_if *wlcif);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_obss_dynbw_dump(wlc_obss_dynbw_t *prot, void *input, int buf_len, void *output,
	wlc_bsscfg_t *bsscfg);
static int wlc_obss_dynbw_dump_statscb(wlc_info_t *wlc, void *ctx,
	uint32 elapsed_time, void *vstats);
#endif 

static int wlc_obss_dynbw_init(void *cntxt);
static void wlc_obss_dynbw_watchdog(void *cntxt);
static INLINE bool wlc_obss_dynbw_prim_interf_detection_logic(uint32 sample_dur,
	uint32 crs_prim, uint32 ibss, uint8 thresh_seconds);
static void wlc_obss_dynbw_restore_defaults(wlc_obss_dynbw_t *prot, wlc_bsscfg_t *bsscfg,
		chanspec_t chspec);
static INLINE bool wlc_obss_dynbw_sec_interf_detection_logic(uint32 sample_dur,
	uint32 crs_total, uint8 thresh_seconds);
static INLINE bool wlc_obss_dynbw_txop_detection_logic(uint32 sample_dur,
	uint32 slot_time_txop, uint32 txopp_stats, uint32 txdur, uint32 ibss, uint8 txop_threshold);
#ifdef WLCSA
static void wlc_obss_dynbw_csa_cb(void *ctx, wlc_csa_notif_cb_data_t *notif_data);
#endif
/* static inline function for  TXOP stats update */
static INLINE bool
wlc_obss_dynbw_txop_detection_logic(uint32 sample_dur,
		uint32 slot_time_txop, uint32 txopp_stats, uint32 txdur,
		uint32 ibss, uint8 txop_threshold)
{
	uint32 limit = (txop_threshold * sample_dur) / 100;

	/* Calculate slot time ifs */
	uint32 slot_time_ifs = ((slot_time_txop >> 8) & 0xFF) +
		(slot_time_txop & 0xFF);

	/* Calculate txop using slot time ifs  */
	uint32 txop = (txopp_stats)*(slot_time_ifs);
	return ((txop + txdur + ibss) <= limit) ? TRUE : FALSE;
}

static INLINE bool
wlc_obss_dynbw_sec_interf_detection_logic(uint32 sample_dur,
		uint32 crs_total, uint8 thresh_seconds)
{
	uint32 limit = ((thresh_seconds * sample_dur) / 100);
	return (crs_total >= limit) ? TRUE : FALSE;
}

static INLINE bool
wlc_obss_dynbw_prim_interf_detection_logic(uint32 sample_dur,
		uint32 crs_prim, uint32 ibss, uint8 thresh_seconds)
{
	uint32 limit = ((thresh_seconds * sample_dur) / 100);
	bool result = ((crs_prim > limit) &&
		(ibss < (crs_prim *
		OBSS_RATIO_RXCRS_PRI_VS_IBSS_DEFAULT) / 100)) ? TRUE : FALSE;
	if (result)
		WL_MODE_SWITCH(("PRI.DYNBW:sample_dur:%d,rxcrs_pri:%d,"
			"rxcrs_pri_limit:%d,ibss:%d \n",
			sample_dur, crs_prim, limit, ibss));
	return result;
}


static bool
wlc_obss_dynbw_interference_detected_for_bwsw(wlc_obss_dynbw_t *prot,
		wlc_bmac_obss_counts_t *delta_stats)
{
	uint32 sample_dur, txop, slot_time_ifs, txdur;
	bool prim_detected = FALSE, sec_detected = FALSE;
	uint32 rxcrs_sec40, rxcrs_sec20, rxcrs_sec, rxcrs_pri, ibss;
	bool result = FALSE;
	/* Use current stats during normal case */
	if (delta_stats == NULL)
	{
		/* Sample duration is the TSF difference (in usec) between two reads. */
		sample_dur = prot->curr_stats->usecs - prot->prev_stats->usecs;
		OBSS_DYNBW_DELTASTATS(rxcrs_sec40, prot->curr_stats->rxcrs_sec40,
			prot->prev_stats->rxcrs_sec40);
		OBSS_DYNBW_DELTASTATS(rxcrs_sec20, prot->curr_stats->rxcrs_sec20,
			prot->prev_stats->rxcrs_sec20);
		rxcrs_sec = rxcrs_sec40 + rxcrs_sec20;
		/* Check Primary OBSS */
		OBSS_DYNBW_DELTASTATS(rxcrs_pri, prot->curr_stats->rxcrs_pri,
			prot->prev_stats->rxcrs_pri);
		OBSS_DYNBW_DELTASTATS(ibss, prot->curr_stats->ibss,
			prot->prev_stats->ibss);
		slot_time_ifs = prot->curr_stats->slot_time_txop;
		OBSS_DYNBW_DELTASTATS(txop, prot->curr_stats->txopp,
			prot->prev_stats->txopp);
		OBSS_DYNBW_DELTASTATS(txdur, prot->curr_stats->txdur,
			prot->prev_stats->txdur);
	}
	else
	{
		/* Use Delta stats during pseudo Case */
		sample_dur = delta_stats->usecs;
		rxcrs_sec = delta_stats->rxcrs_sec40 + delta_stats->rxcrs_sec20;
		rxcrs_pri = delta_stats->rxcrs_pri;
		ibss = delta_stats->ibss;
		slot_time_ifs = delta_stats->slot_time_txop;
		txop = delta_stats->txopp;
		txdur = delta_stats->txdur;
	}

	/* RXCRS stats for OBSS detection */
	if (prot->dyn_bwsw_enabled == OBSS_DYN_BWSW_ENAB_RXCRS)
	{
		/* OBSS detected if RX CRS Secondary exceeds configured limit */
		sec_detected = wlc_obss_dynbw_sec_interf_detection_logic(sample_dur,
			rxcrs_sec,
			prot->cfg_dbs->obss_bwsw_dur_thres);
		prim_detected = wlc_obss_dynbw_prim_interf_detection_logic(sample_dur,
			rxcrs_pri, ibss,
			prot->cfg_dbs->obss_bwsw_rx_crs_threshold_cfg);
		WL_MODE_SWITCH(("OBSS Detected RXCRS: sample_dur:%d, rxcrs_sec:%d "
			"pri:%d, sec:%d\n", sample_dur, rxcrs_sec,
			prim_detected, sec_detected));

		/* If both Secondary and PRIMARY OBSS is still active , return TRUE */
		result = (sec_detected && prim_detected) ? TRUE: FALSE;
	}
	else if (prot->dyn_bwsw_enabled == OBSS_DYN_BWSW_ENAB_TXOP)
	/* TXOP  stats for OBSS detection
	 * TXOP and TXDUR calculate data Transmitted
	 * INBSS calculates the data received.
	 * When both are low, it means interference is
	 * occupying the medium and DBS needs to start.
	 */
	{
		result = wlc_obss_dynbw_txop_detection_logic(sample_dur,
			slot_time_ifs, txop, txdur, ibss,
			prot->cfg_dbs->obss_bwsw_txop_threshold_cfg);
		if (result) {
			WL_MODE_SWITCH(("OBSS Detected TXOP: sample_dur:%d ,"
				"txop:%d,txdur:%d\n", sample_dur, txop, txdur));
		}
	}
	return result;
}

static int
wlc_obss_dynbw_init(void *cntxt)
{
	return BCME_OK;
}

static void
wlc_obss_dynbw_watchdog(void *cntxt)
{

	wlc_obss_dynbw_t *prot = (wlc_obss_dynbw_t *) cntxt;
	wlc_info_t *wlc = prot->wlc;
	if (prot->dyn_bwsw_enabled) {
		if (prot->curr_stats->usecs == 0) {
			if (WLC_CCASTATS_CAP(wlc)) {
				wlc_obss_util_update(wlc, prot->curr_stats, prot->prev_stats,
					prot->total_stats, wlc->chanspec);
			}
		}
		else {
			if (WLC_CCASTATS_CAP(wlc)) {
				wlc_obss_util_update(wlc, prot->curr_stats, prot->prev_stats,
					prot->total_stats, wlc->chanspec);
				wlc_obss_dynbw_manage_bw_switch(prot);
			}
		}
	}
	else {
		prot->curr_stats->usecs = 0;
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)

static int
wlc_obss_dynbw_dump_statscb(wlc_info_t *wlc, void *ctx,
		uint32 elapsed_time, void *vstats)
{
	wlc_obss_dynbw_t *prot = (wlc_obss_dynbw_t *) wlc->obss_dynbw;
	wlc_bmac_obss_counts_t *delta = (wlc_bmac_obss_counts_t *)vstats;

	if (vstats == NULL) {
		/* if vstats is NULL, it implies that it was not able
		* to complete the measurement of stats. As a result,
		* msrmnt_done is still 0
		*/
		return 0;
	}

	memcpy(prot->msrmnt_stored, delta, sizeof(*delta));

	/* since, it was able to complete the measurement, set
	* msrmnt_done to 1 to reflect that the results can be displayed
	*/
	prot->results->msrmnt_done = 1;
	return 0;
}

static int
wlc_obss_dynbw_dump(wlc_obss_dynbw_t *prot, void *input, int buf_len, void *output,
		wlc_bsscfg_t *bsscfg)
{
	cca_msrmnt_query *q = (cca_msrmnt_query *) input;
	cca_stats_n_flags *results;
	wlc_info_t *wlc = prot->wlc;

	if (!q->msrmnt_query) {
		prot->results->msrmnt_done = 0;
		wlc_lq_register_dynbw_stats_cb(wlc,
			q->time_req, wlc_obss_dynbw_dump_statscb, bsscfg->ID, NULL);
		bzero(output, buf_len);
	} else {
		char *buf_ptr;
		struct bcmstrbuf b;
		wlc_bsscfg_t *cfg;
		int idx;

		results = (cca_stats_n_flags *) output;
		buf_ptr = results->buf;
		buf_len = buf_len - OFFSETOF(cca_stats_n_flags, buf);
		buf_len = (buf_len > 0) ? buf_len : 0;

		results->msrmnt_time = prot->results->msrmnt_time;
		results->msrmnt_done = prot->results->msrmnt_done;
		bcm_binit(&b, buf_ptr, buf_len);

		if (results->msrmnt_done) {
			wlc_obss_util_stats(prot->wlc, prot->msrmnt_stored, prot->prev_stats,
				prot->curr_stats, q->report_opt, &b);
		}
		FOREACH_AS_BSS(wlc, idx, cfg) {
			dynbw_bsscfg_cubby_t *po_bss = DYNBW_BSSCFG_CUBBY(prot, cfg);

			bcm_bprintf(&b, "\n\nInterface ID=%d\n", cfg->ID);
			bcm_bprintf(&b, "Obss_activity=%d\n", po_bss->obss_activity);
			bcm_bprintf(&b, "pseudo states=%d\n", po_bss->bwswpsd_state);
			bcm_bprintf(&b, "tx overridden cspec=%x\n", po_bss->bwswpsd_cspec_for_tx);
			bcm_bprintf(&b, "# of secs of OBSS inactivity for bwswitch=%d\n",
				po_bss->obss_bwsw_no_activity_cfm_count);
			bcm_bprintf(&b, "# of secs of OBSS activity for bwswitch=%d\n",
				po_bss->obss_bwsw_activity_cfm_count);
		}

	}
	return 0;
}
#endif 

/* macro for dyn_bwsw_params */
#define DYNBWSW_CONFIG_PARAMS(field, mask, cfg_flag, rst_flag, val, def) \
	(cfg_flag & mask) ? val : ((rst_flag & mask) ? def : field);

static INLINE void
update_dynbwsw_config_params(obss_config_params_t *params,
		wlc_obss_dynbwsw_config_t *config)
{
	uint32 config_flags, reset_flags;
	wlc_obss_dynbwsw_config_t *disp_params = &params->config_params;
	config_flags = params->config_mask;
	reset_flags = params->reset_mask;

	config->obss_bwsw_activity_cfm_count_cfg =
		DYNBWSW_CONFIG_PARAMS(config->obss_bwsw_activity_cfm_count_cfg,
			WL_OBSS_DYN_BWSW_FLAG_ACTIVITY_PERIOD, config_flags, reset_flags,
			disp_params->obss_bwsw_activity_cfm_count_cfg,
			OBSS_BWSW_ACTIVITY_CFM_PERIOD_DEFAULT);

	config->obss_bwsw_no_activity_cfm_count_cfg =
		DYNBWSW_CONFIG_PARAMS(config->obss_bwsw_no_activity_cfm_count_cfg,
			WL_OBSS_DYN_BWSW_FLAG_NOACTIVITY_PERIOD, config_flags, reset_flags,
			disp_params->obss_bwsw_no_activity_cfm_count_cfg,
			OBSS_BWSW_NO_ACTIVITY_CFM_PERIOD_DEFAULT);

	config->obss_bwsw_no_activity_cfm_count_incr_cfg =
		DYNBWSW_CONFIG_PARAMS(config->obss_bwsw_no_activity_cfm_count_incr_cfg,
			WL_OBSS_DYN_BWSW_FLAG_NOACTIVITY_INCR_PERIOD, config_flags, reset_flags,
			disp_params->obss_bwsw_no_activity_cfm_count_incr_cfg,
			OBSS_BWSW_NO_ACTIVITY_CFM_PERIOD_INCR_DEFAULT);

	config->obss_bwsw_pseudo_sense_count_cfg =
		DYNBWSW_CONFIG_PARAMS(config->obss_bwsw_pseudo_sense_count_cfg,
			WL_OBSS_DYN_BWSW_FLAG_PSEUDO_SENSE_PERIOD, config_flags, reset_flags,
			disp_params->obss_bwsw_pseudo_sense_count_cfg,
			OBSS_BWSW_PSEUDO_SENSE_PERIOD_DEFAULT);

	config->obss_bwsw_rx_crs_threshold_cfg =
		DYNBWSW_CONFIG_PARAMS(config->obss_bwsw_rx_crs_threshold_cfg,
			WL_OBSS_DYN_BWSW_FLAG_RX_CRS_PERIOD, config_flags, reset_flags,
			disp_params->obss_bwsw_rx_crs_threshold_cfg,
			OBSS_DUR_RXCRS_PRI_THRESHOLD_DEFAULT);

	config->obss_bwsw_dur_thres =
		DYNBWSW_CONFIG_PARAMS(config->obss_bwsw_dur_thres,
			WL_OBSS_DYN_BWSW_FLAG_DUR_THRESHOLD, config_flags, reset_flags,
			disp_params->obss_bwsw_dur_thres,
			OBSS_BWSW_DUR_THRESHOLD_DEFAULT);

	config->obss_bwsw_txop_threshold_cfg =
		DYNBWSW_CONFIG_PARAMS(config->obss_bwsw_txop_threshold_cfg,
			WL_OBSS_DYN_BWSW_FLAG_TXOP_PERIOD, config_flags, reset_flags,
			disp_params->obss_bwsw_txop_threshold_cfg,
			OBSS_TXOP_THRESHOLD_DEFAULT);
}
static int
wlc_obss_dynbw_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
		void *p, uint plen, void *a, int alen, int val_size, struct wlc_if *wlcif)
{
	wlc_obss_dynbw_t *prot = (wlc_obss_dynbw_t *) hdl;
	int32 int_val = 0;
	uint32 uint_val;
	int32 *ret_int_ptr;
	bool bool_val;
	int err = 0;
	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)a;
	BCM_REFERENCE(ret_int_ptr);

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	uint_val = (uint)int_val;
	BCM_REFERENCE(uint_val);
	bool_val = (int_val != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val);

	switch (actionid) {
		case IOV_GVAL(IOV_OBSS_DYN_BWSW_ENAB):
			*ret_int_ptr = (int32) prot->dyn_bwsw_enabled;
			break;

		case IOV_SVAL(IOV_OBSS_DYN_BWSW_ENAB):
			prot->dyn_bwsw_enabled = (uint8)int_val;
			break;

		case IOV_GVAL(IOV_OBSS_DYN_BWSW_PARAMS): {
			obss_config_params_t *params = (obss_config_params_t *)a;
			params->version = WL_PROT_OBSS_CONFIG_PARAMS_VERSION;
			params->config_params = *(prot->cfg_dbs);
			break;
		}

		case IOV_SVAL(IOV_OBSS_DYN_BWSW_PARAMS): {
			obss_config_params_t *params = (obss_config_params_t *)p;
			wlc_obss_dynbwsw_config_t *config = prot->cfg_dbs;

			if (params->version != WL_PROT_OBSS_CONFIG_PARAMS_VERSION) {
				WL_ERROR(("Driver version mismatch. expected = %d, got = %d\n",
					WL_PROT_OBSS_CONFIG_PARAMS_VERSION, params->version));
				return BCME_VERSION;
			}

			if (!prot->dyn_bwsw_enabled)
				return BCME_NOTUP;
			update_dynbwsw_config_params(params, config);
			break;
		}
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
		case IOV_GVAL(IOV_OBSS_DYN_BWSW_DUMP): {
			wlc_info_t *wlc = prot->wlc;
			wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
			ASSERT(cfg != NULL);
			err = wlc_obss_dynbw_dump(prot, p, alen, a, cfg);
			break;
		}
#endif 
		default:
			err = BCME_UNSUPPORTED;
			break;
	}
	return err;
}

wlc_obss_dynbw_t *
BCMATTACHFN(wlc_obss_dynbw_attach)(wlc_info_t *wlc)
{
	wlc_obss_dynbw_t *prot;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	prot = (wlc_obss_dynbw_t *) MALLOCZ(wlc->osh, (WLC_OBSS_DYNBW_SIZE
			+ WLC_OBSS_DYNBW_STATIC_SIZE + WLC_OBSS_DYNBW_DEBUG_SIZE));
#else
	prot = (wlc_obss_dynbw_t *) MALLOCZ(wlc->osh, (WLC_OBSS_DYNBW_SIZE
			+ WLC_OBSS_DYNBW_STATIC_SIZE));
#endif 

	if (prot == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit,
			__FUNCTION__,
			MALLOCED(wlc->osh)));
		wlc->pub->_obss_dynbw = FALSE;
		return NULL;
	}

	prot->wlc = wlc;
	prot->cfg_dbs = (wlc_obss_dynbwsw_config_t *) (prot + 1);
	prot->prev_stats = (wlc_bmac_obss_counts_t *) (prot->cfg_dbs + 1);
	prot->curr_stats = (wlc_bmac_obss_counts_t *) (prot->prev_stats + 1);
	prot->total_stats = (wlc_bmac_obss_counts_t *) (prot->curr_stats + 1);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	prot->msrmnt_stored = (wlc_bmac_obss_counts_t *) (prot->total_stats + 1);
	prot->results = (cca_stats_n_flags *) (prot->msrmnt_stored + 1);
#endif 

	if (D11REV_GE(wlc->pub->corerev, 40)) {
		wlc->pub->_obss_dynbw = TRUE;
	}
	/* bwsw defaults. Note: by default: disabled., Use the iovar to enable */
	prot->dyn_bwsw_enabled = OBSS_DYN_BWSW_DISABLE;
	prot->cfg_dbs->obss_bwsw_activity_cfm_count_cfg =
		OBSS_BWSW_ACTIVITY_CFM_PERIOD_DEFAULT;
	prot->cfg_dbs->obss_bwsw_no_activity_cfm_count_cfg =
		OBSS_BWSW_NO_ACTIVITY_CFM_PERIOD_DEFAULT;
	prot->cfg_dbs->obss_bwsw_no_activity_cfm_count_incr_cfg =
		OBSS_BWSW_NO_ACTIVITY_CFM_PERIOD_INCR_DEFAULT;
	prot->cfg_dbs->obss_bwsw_pseudo_sense_count_cfg =
		OBSS_BWSW_PSEUDO_SENSE_PERIOD_DEFAULT;
	prot->cfg_dbs->obss_bwsw_rx_crs_threshold_cfg =
		OBSS_DUR_RXCRS_PRI_THRESHOLD_DEFAULT;
	prot->cfg_dbs->obss_bwsw_dur_thres =
		OBSS_BWSW_DUR_THRESHOLD_DEFAULT;
	prot->cfg_dbs->obss_bwsw_txop_threshold_cfg =
		OBSS_TXOP_THRESHOLD_DEFAULT;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((prot->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(dynbw_bsscfg_cubby_t *),
			wlc_obss_dynbw_bssinfo_init, wlc_obss_dynbw_bssinfo_deinit, NULL,
			(void *)prot)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_module_register(wlc->pub, wlc_obss_dynbw_iovars,
			"obss_dynbw", prot, wlc_obss_dynbw_doiovar,
			wlc_obss_dynbw_watchdog, wlc_obss_dynbw_init, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_register failed\n",
		    wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_bss_assoc_state_register(wlc, wlc_obss_dynbw_assoc_cxt_cb, prot) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_assoc_state_register() failed\n",
		        wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_bsscfg_updown_register(wlc, wlc_obss_dynbw_updown_cb, prot) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
			wlc->pub->unit, __FUNCTION__));

		goto fail;
	}
	if (WLC_MODESW_ENAB(wlc->pub)) {
		if ((wlc->modesw) && (wlc_modesw_notif_cb_register(wlc->modesw,
				wlc_obss_dynbw_modesw_cb, prot) != BCME_OK)) {
			/* if call bk fails, it just continues, but the dyn_bwsw
			* will be disabled.
			*/
			WL_ERROR(("%s: modesw notif callbk failed, but continuing\n",
				__FUNCTION__));
			prot->dyn_bwsw_enabled = OBSS_DYN_BWSW_DISABLE;
		}
	} else
	{
		prot->dyn_bwsw_enabled = OBSS_DYN_BWSW_DISABLE;
	}
#ifdef WLCSA
	if (wlc->csa) {
	    if (wlc_csa_obss_dynbw_notif_cb_register(
			wlc->csa, wlc_obss_dynbw_csa_cb, prot) != BCME_OK) {
		WL_ERROR(("%s: csa notif callbk failed, but continuing\n",
			__FUNCTION__));
		goto fail;
	    }
	}
#endif
	return prot;
fail:
	wlc_obss_dynbw_detach(prot);
	return NULL;
}

void
BCMATTACHFN(wlc_obss_dynbw_detach)(wlc_obss_dynbw_t *prot)
{
	wlc_info_t *wlc;

	if (!prot)
		return;

	wlc = prot->wlc;
	wlc->pub->_obss_dynbw = FALSE;

	wlc_module_unregister(wlc->pub, "obss_dynbw", prot);

	if (wlc_bss_assoc_state_unregister(wlc,
			wlc_obss_dynbw_assoc_cxt_cb, prot) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_assoc_state_unregister() failed\n",
			wlc->pub->unit, __FUNCTION__));
	}
	if (wlc_bsscfg_updown_unregister(wlc,
			wlc_obss_dynbw_updown_cb, prot) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_unregister() failed\n",
			wlc->pub->unit, __FUNCTION__));
	}
	if (WLC_MODESW_ENAB(wlc->pub)) {
		if (wlc_modesw_notif_cb_unregister(wlc->modesw,
				wlc_obss_dynbw_modesw_cb, prot) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_modesw_notif_cb_unregister() failed\n",
				wlc->pub->unit, __FUNCTION__));
		}
	}
#ifdef WLCSA
	if (wlc->csa) {
	    if (wlc_csa_obss_dynbw_notif_cb_unregister(
			wlc->csa, wlc_obss_dynbw_csa_cb, prot) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_csa_notif_cb_unregister() failed\n",
			wlc->pub->unit, __FUNCTION__));
	    }
	}
#endif /* WLCSA */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	MFREE(wlc->osh, prot, (WLC_OBSS_DYNBW_SIZE + WLC_OBSS_DYNBW_STATIC_SIZE
			+ WLC_OBSS_DYNBW_DEBUG_SIZE));
#else
	MFREE(wlc->osh, prot, (WLC_OBSS_DYNBW_SIZE + WLC_OBSS_DYNBW_STATIC_SIZE));
#endif 
}

static int
wlc_obss_dynbw_bssinfo_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_obss_dynbw_t *prot = (wlc_obss_dynbw_t *) ctx;
	wlc_info_t *wlc = prot->wlc;
	dynbw_bsscfg_cubby_t **pdynbw_bss = DYNBW_BSSCFG_CUBBY_LOC(prot, cfg);
	dynbw_bsscfg_cubby_t *dynbw_bss = NULL;

	/* allocate memory and point bsscfg cubby to it */
	if ((dynbw_bss = MALLOCZ(wlc->osh, sizeof(dynbw_bsscfg_cubby_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}
	dynbw_bss->orig_chanspec = INVCHANSPEC;
	dynbw_bss->bwswpsd_cspec_for_tx = INVCHANSPEC;
	prot->main_chanspec = INVCHANSPEC;
	*pdynbw_bss = dynbw_bss;
	return BCME_OK;
}

static void
wlc_obss_dynbw_bssinfo_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_obss_dynbw_t *prot = (wlc_obss_dynbw_t *) ctx;
	wlc_info_t *wlc = prot->wlc;
	dynbw_bsscfg_cubby_t **pdynbw_bss = DYNBW_BSSCFG_CUBBY_LOC(prot, cfg);
	dynbw_bsscfg_cubby_t *dynbw_bss = NULL;

	/* free the Cubby reserve allocated memory  */
	dynbw_bss = *pdynbw_bss;
	if (dynbw_bss) {
		/* if some obss is still active, but we are releasing bsscfg, reduce
		* the global obss active count
		*/
		if (dynbw_bss->obss_activity != OBSS_INACTIVE) {
			prot->obss_active_cnt--;
		}

		MFREE(wlc->osh, dynbw_bss, sizeof(dynbw_bsscfg_cubby_t));
		*pdynbw_bss = NULL;
	}
}


/* Funtion returns TRUE when it can find for than 1 Associated
* BSSCFG. This is especially needed for non-MCHAN PSTA and MBSS
* scenarios
*/
static bool
wlc_prot_obss_check_for_multi_ifs(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;
	uint cnt = 0;

	if (!PSTA_ENAB(wlc->pub) && !MBSS_ENAB(wlc->pub)) {
		WL_MODE_SWITCH(("Non PSTA and Non MBSS return FALSE\n"));
		return FALSE;
	}

	FOREACH_AS_BSS(wlc, idx, cfg) {
		cnt++;
		if (cnt > 1)
			return TRUE;
	}
	return FALSE;
}
static int
wlc_obss_dynbw_manage_bw_switch(wlc_obss_dynbw_t *prot)
{
	int idx;
	wlc_bsscfg_t *cfg;
	dynbw_bsscfg_cubby_t *po_bss = NULL;
	int err = BCME_OK;
	wlc_info_t *wlc = prot->wlc;

	/* if both secondary and primary are interfering */
	if (wlc_obss_dynbw_interference_detected_for_bwsw(prot, NULL)) {
		chanspec_t original_chspec;
		FOREACH_AS_BSS(wlc, idx, cfg) {
			po_bss = DYNBW_BSSCFG_CUBBY(prot, cfg);
			WL_MODE_SWITCH(("cfg:%p, bwswpsd:%d\n", cfg,
				po_bss->bwswpsd_state));
			if (WL11H_ENAB(wlc) &&
				wlc_radar_chanspec(wlc->cmi, cfg->current_bss->chanspec) &&
				!wlc_is_edcrs_eu(wlc)) {
			    continue;
			}

			if (po_bss->is_csa_lock) {
				/* Since channel change is  in progress
				* we donot run OBSS_DBS on old channel
				*/
				continue;
			}
			if (po_bss->is_obss_modesw_call == TRUE)
					continue;

			if (po_bss->bwswpsd_state == BWSW_PSEUDOUPGD_NOTACTIVE)
			{
				po_bss->obss_bwsw_activity_cfm_count++;
			}
			po_bss->obss_bwsw_no_activity_cfm_count = 0;
			WL_MODE_SWITCH(("%s: IF+: %d of %d; \n", __FUNCTION__,
				po_bss->obss_bwsw_activity_cfm_count,
				prot->cfg_dbs->obss_bwsw_activity_cfm_count_cfg));

			if (po_bss->obss_bwsw_activity_cfm_count <
					prot->cfg_dbs->obss_bwsw_activity_cfm_count_cfg) {
				continue;
			}

			/* pseudoug active/pending, don't do anything, let the
			* timer decide
			*/
			if (po_bss->bwswpsd_state != BWSW_PSEUDOUPGD_NOTACTIVE) {
				continue;
			}

			original_chspec = cfg->current_bss->chanspec;
			WL_MODE_SWITCH(("Orig Chanspec = %x\n", original_chspec));
			if (WLC_MODESW_ENAB(wlc->pub)) {
					uint32 ctrl_flags = MODESW_CTRL_AP_ACT_FRAMES |
					MODESW_CTRL_NO_ACK_DISASSOC;
				if (wlc_prot_obss_check_for_multi_ifs(wlc) == TRUE) {
					/* For PSTA and MBSS cases dynamic bwswitch
					* needs to happen for all IFs together
					*/
					ctrl_flags = ctrl_flags | MODESW_CTRL_HANDLE_ALL_CFGS;
				}
				po_bss->is_obss_modesw_call = TRUE;
				err = wlc_modesw_bw_switch(wlc->modesw,
					cfg->current_bss->chanspec,
					BW_SWITCH_TYPE_DNGRADE, cfg,
					ctrl_flags);
			}
			else
				err = BCME_UNSUPPORTED;

			po_bss->obss_bwsw_no_activity_cfm_count = 0;

			if ((err == BCME_OK) && (po_bss->obss_activity == OBSS_INACTIVE)) {
				/* If this is the first downg, store the orig BW
				* for use later.
				*/
				if (prot->main_chanspec == INVCHANSPEC) {
					prot->main_chanspec = original_chspec;
				}

				po_bss->orig_chanspec = prot->main_chanspec;
				po_bss->obss_bwsw_no_activity_cfm_count_max =
					prot->cfg_dbs->obss_bwsw_no_activity_cfm_count_cfg;
			}
			if (err != BCME_OK) {
				po_bss->is_obss_modesw_call = FALSE;
			}
			po_bss->obss_bwsw_activity_cfm_count = 0;
		}
	} else {
		/* OBSS POSSIBLY GONE NOW... if any one of the cfg's were affected? */
		if (prot->obss_active_cnt == 0) {
			if (prot->main_chanspec != INVCHANSPEC)
				prot->main_chanspec = INVCHANSPEC;
		}

		FOREACH_AS_BSS(wlc, idx, cfg) {
			po_bss = DYNBW_BSSCFG_CUBBY(prot, cfg);
			po_bss->obss_bwsw_activity_cfm_count = 0;

			if (!po_bss->obss_activity)
			    continue;
			if (po_bss->is_obss_modesw_call == TRUE)
				continue;
			po_bss->obss_bwsw_no_activity_cfm_count++;
			WL_MODE_SWITCH(("%s:IF-:%d.of.%d;oacnt:%d\n", __FUNCTION__,
				po_bss->obss_bwsw_no_activity_cfm_count,
				po_bss->obss_bwsw_no_activity_cfm_count_max,
				prot->obss_active_cnt));
			if (po_bss->obss_bwsw_no_activity_cfm_count <
					po_bss->obss_bwsw_no_activity_cfm_count_max) {
				continue;
			}

			if (po_bss->bwswpsd_state ==
					BWSW_PSEUDOUPGD_NOTACTIVE) {
				chanspec_t prsnt_chspec = cfg->current_bss->chanspec;
				po_bss->bwswpsd_state = BWSW_PSEUDOUPGD_PENDING;
				if (WLC_MODESW_ENAB(wlc->pub)) {
					uint32 ctrl_flags = MODESW_CTRL_UP_SILENT_UPGRADE;
					if (wlc_prot_obss_check_for_multi_ifs(wlc) ==
							TRUE) {
						/* For PSTA and MBSS cases dynamic bwswitch
						* needs to happen for all IFs together
						*/
						ctrl_flags = ctrl_flags |
						MODESW_CTRL_HANDLE_ALL_CFGS;
					}
					po_bss->is_obss_modesw_call = TRUE;
					/* save prsnt cspec to come bk if req */
					po_bss->bwswpsd_cspec_for_tx =
						prsnt_chspec;
					err = wlc_modesw_bw_switch(wlc->modesw,
						cfg->current_bss->chanspec,
						BW_SWITCH_TYPE_UPGRADE, cfg,
						ctrl_flags);
				}
				else
					err = BCME_UNSUPPORTED;
				if (err == BCME_OK) {

					WL_MODE_SWITCH(("pseudo:UP:txchanSpec:x%x!,"
						"pNOTACTIVE->pPENDING\n",
						po_bss->bwswpsd_cspec_for_tx));
				} else {
					/* Since pseudo up failed, reset to pseudo not active and
					* reset no activity count to zero and let it start again
					*/
					WL_MODE_SWITCH(("pseudo:UP:pst:%d,ERR!\n",
						po_bss->bwswpsd_state));
					po_bss->bwswpsd_state = BWSW_PSEUDOUPGD_NOTACTIVE;
					po_bss->obss_bwsw_no_activity_cfm_count_max =
						prot->cfg_dbs->obss_bwsw_no_activity_cfm_count_cfg;
					po_bss->is_obss_modesw_call = FALSE;
					po_bss->bwswpsd_cspec_for_tx = INVCHANSPEC;
				}
			}
			po_bss->obss_bwsw_no_activity_cfm_count = 0;
		}
	}
	return BCME_OK;
}

/* used by external world to get actual BW for tx, if BW is in pseudo state */
void
wlc_obss_dynbw_tx_bw_override(wlc_obss_dynbw_t *prot,
		wlc_bsscfg_t *bsscfg, uint32 *rspec_bw)
{
	dynbw_bsscfg_cubby_t *po_bss;
	if (bsscfg == NULL)
		return;
	po_bss = DYNBW_BSSCFG_CUBBY(prot, bsscfg);
	if (po_bss->bwswpsd_cspec_for_tx != INVCHANSPEC) {
		*rspec_bw = chspec_to_rspec(CHSPEC_BW(po_bss->bwswpsd_cspec_for_tx));
		WL_MODE_SWITCH(("BWSW:chspecORd:st:%d, chanspec:%x,rspec_bw:%x\n",
			po_bss->bwswpsd_state,
			po_bss->bwswpsd_cspec_for_tx,
			*rspec_bw));
	}
	else if (WLC_MODESW_ENAB(prot->wlc->pub)) {
		wlc_modesw_dynbw_tx_bw_override(prot->wlc->modesw, bsscfg,
			rspec_bw);
	}
}


static int
wlc_obss_dynbw_bwswpsd_statscb(wlc_info_t *wlc, void *ctx,
		uint32 elapsed_time, void *vstats)
{
	wlc_dynbw_cb_ctx_t *cb_ctx = (wlc_dynbw_cb_ctx_t *)ctx;
	wlc_bsscfg_t *bsscfg = wlc_bsscfg_find_by_ID(wlc, cb_ctx->connID);
	dynbw_bsscfg_cubby_t *po_bss;
	wlc_bmac_obss_counts_t *stats = (wlc_bmac_obss_counts_t *)vstats;
	int err = BCME_OK;
	wlc_obss_dynbw_t *prot = (wlc_obss_dynbw_t *) wlc->obss_dynbw;

	WL_INFORM(("%s:enter\n", __FUNCTION__));
	/* in case bsscfg is freed before this callback is invoked */
	if (bsscfg == NULL) {
		WL_ERROR(("wl%d: %s: unable to find bsscfg by ID %p\n",
				wlc->pub->unit, __FUNCTION__, bsscfg));
		err = BCME_ERROR;
		goto fail;
	}

	if (vstats == NULL) {
		WL_ERROR(("wl%d: %s: bsscfg is down\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	po_bss = DYNBW_BSSCFG_CUBBY(prot, bsscfg);

	if (po_bss->bwswpsd_state != BWSW_PSEUDOUPGD_ACTIVE) {
		/* just return */
		goto fail;
	}
	if (po_bss->is_obss_modesw_call == TRUE)
		goto fail;
	/* is interference still present in higher BW's? */
	if (wlc_obss_dynbw_interference_detected_for_bwsw(prot, stats)) {
		uint32 ctrl_flags = MODESW_CTRL_DN_SILENT_DNGRADE;
		if (wlc_prot_obss_check_for_multi_ifs(wlc) == TRUE) {
			/* For PSTA and MBSS cases dynamic bwswitch needs to happen
			* for all IFs together
			*/
			ctrl_flags = ctrl_flags | MODESW_CTRL_HANDLE_ALL_CFGS;
		}
		/* interference still present... silently dngrade */
		po_bss->bwswpsd_state = BWSW_PSEUDOUPGD_PENDING;
		po_bss->is_obss_modesw_call = TRUE;
		err = wlc_modesw_bw_switch(wlc->modesw,
			po_bss->bwswpsd_cspec_for_tx, BW_SWITCH_TYPE_DNGRADE, bsscfg,
			ctrl_flags);
		if ((err == BCME_OK) || (err == BCME_BUSY)) {
			WL_MODE_SWITCH(("pseudo:statscb:INTF_STILL_PRESENT!so,"
				"DN:txchanSpec:x%x!, pACTIVE->pPENDING\n",
				po_bss->bwswpsd_cspec_for_tx));
		} else {
			WL_MODE_SWITCH(("BWSW:statscb:DN:pst:%d, ERROR!\n",
				po_bss->bwswpsd_state));
			po_bss->bwswpsd_state = BWSW_PSEUDOUPGD_NOTACTIVE;
			po_bss->is_obss_modesw_call = FALSE;
		}
	} else {
		uint32 ctrl_flags = MODESW_CTRL_UP_ACTION_FRAMES_ONLY |
			MODESW_CTRL_AP_ACT_FRAMES | MODESW_CTRL_NO_ACK_DISASSOC;
		if (wlc_prot_obss_check_for_multi_ifs(wlc) == TRUE) {
			/* For PSTA and MBSS cases dynamic bwswitch needs to happen
			* for all IFs together
			*/
			ctrl_flags = ctrl_flags | MODESW_CTRL_HANDLE_ALL_CFGS;
		}
		/* upgrade. TBD: a counter required? */
		if (BSSCFG_AP(bsscfg)) {
			po_bss->bwswpsd_cspec_for_tx = INVCHANSPEC;
		}
		po_bss->is_obss_modesw_call = TRUE;
		err = wlc_modesw_bw_switch(wlc->modesw,
			bsscfg->current_bss->chanspec, BW_SWITCH_TYPE_UPGRADE, bsscfg,
			ctrl_flags);
		if (err == BCME_OK) {
			WL_MODE_SWITCH(("pseudo:statscb:INTF_GONE!so,UP:txchanSpec:"
					"x%x! actionsending!\n",
					po_bss->bwswpsd_cspec_for_tx));
		} else {
			WL_MODE_SWITCH(("BWSW:statscb:DN:pst:%d, ERROR!\n",
				po_bss->bwswpsd_state));
			po_bss->is_obss_modesw_call = FALSE;
		}
	}
fail:
	MFREE(prot->wlc->osh, cb_ctx, sizeof(wlc_dynbw_cb_ctx_t));
	return err;
}

static int
wlc_obss_dynbw_init_psdstat_cb(wlc_obss_dynbw_t *prot, wlc_bsscfg_t *cfg)
{
	wlc_dynbw_cb_ctx_t *cb_ctx = NULL;

	cb_ctx = (wlc_dynbw_cb_ctx_t *)MALLOCZ(prot->wlc->osh,
		sizeof(wlc_dynbw_cb_ctx_t));

	if (!cb_ctx) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			prot->wlc->pub->unit, __FUNCTION__,
			MALLOCED(prot->wlc->osh)));
		return BCME_NOMEM;
	}
	cb_ctx->connID = cfg->ID;

	WL_MODE_SWITCH(("%s:register_lq_cb!\n", __FUNCTION__));

	return wlc_lq_register_dynbw_stats_cb(prot->wlc,
		prot->cfg_dbs->obss_bwsw_pseudo_sense_count_cfg,
		wlc_obss_dynbw_bwswpsd_statscb, cfg->ID, cb_ctx);
}

static void
wlc_obss_dynbw_set_obss_activity_info(wlc_obss_dynbw_t *prot,
		wlc_bsscfg_t * bsscfg, dynbw_bsscfg_cubby_t *po_bss)
{
	if (po_bss->bwswpsd_state == BWSW_PSEUDOUPGD_NOTACTIVE) {
		WL_MODE_SWITCH(("\n orig = %x curr = %x\n",
			po_bss->orig_chanspec, bsscfg->current_bss->chanspec));
		if (bsscfg->current_bss->chanspec == po_bss->orig_chanspec) {
			po_bss->obss_activity = OBSS_INACTIVE;
			po_bss->orig_chanspec = INVCHANSPEC;
			prot->obss_active_cnt--;
			WL_INFORM(("%s:oa:%d\n", __FUNCTION__, prot->obss_active_cnt));
			WL_MODE_SWITCH(("%s:OBSS inactive\n", __FUNCTION__));
		} else {
			po_bss->obss_activity =
				wlc_modesw_is_connection_vht(prot->wlc, bsscfg) ?
				OBSS_ACTIVE_NONHT : OBSS_ACTIVE_HT;
			WL_MODE_SWITCH(("Value of the actvity in %s = %d\n",
				__FUNCTION__, po_bss->obss_activity));
		}
	}
}

static void
wlc_obss_dynbw_modesw_cb(void *ctx, wlc_modesw_notif_cb_data_t *notif_data)
{
	wlc_obss_dynbw_t *prot = (wlc_obss_dynbw_t *) ctx;
	dynbw_bsscfg_cubby_t *po_bss = DYNBW_BSSCFG_CUBBY(prot, notif_data->cfg);

	if (prot->dyn_bwsw_enabled == OBSS_DYN_BWSW_DISABLE ||
			!po_bss->is_obss_modesw_call) {
		/* Do not handle modesw callbacks as they are not meant for
		 * OBSS DBS module
		 */
		return;
	}
	WL_MODE_SWITCH(("%s: signal:%d\n", __FUNCTION__, notif_data->signal));
	if (po_bss->is_obss_modesw_call) {
		po_bss->is_obss_modesw_call = FALSE;
	}
	switch (notif_data->signal) {
	case (MODESW_PHY_UP_COMPLETE):
		WL_MODE_SWITCH(("Got the PHY_UP status\n"));
		if (po_bss->bwswpsd_state == BWSW_PSEUDOUPGD_PENDING) {
			po_bss->bwswpsd_state = BWSW_PSEUDOUPGD_ACTIVE;
			WL_MODE_SWITCH(("pseudo:UP:pPENDING->pACTIVE, NOT sending "
				"ACTION\n"));
			/* register callback for the stats to come for full Bw.
			* TBD: also check return value & act
			*/
			if (BCME_OK != wlc_obss_dynbw_init_psdstat_cb(prot, notif_data->cfg))
				WL_ERROR(("%s: failed to register psd cb!\n",
					__FUNCTION__));
		}
		break;
	case (MODESW_DN_AP_COMPLETE):
		WL_MODE_SWITCH(("Caught Signal = MODESW_DN_AP_COMPLETE\n"));
		WL_MODE_SWITCH(("value of the psd state = %d\n",
			po_bss->bwswpsd_state));
		WL_MODE_SWITCH(("Value of obss activity = %d\n",
			po_bss->obss_activity));
	/* Fall through */
	case (MODESW_DN_STA_COMPLETE):
		if (po_bss->bwswpsd_state == BWSW_PSEUDOUPGD_PENDING) {
			po_bss->bwswpsd_state = BWSW_PSEUDOUPGD_NOTACTIVE;
			WL_MODE_SWITCH(("pseudo:DN:NOACT:pPENDING->pNOTACTIVE"
				"oc:%x, cs:%x, txcspec:%x\n",
				po_bss->orig_chanspec, prot->wlc->chanspec,
				po_bss->bwswpsd_cspec_for_tx));
			po_bss->bwswpsd_cspec_for_tx = INVCHANSPEC;
			/* increment max number of seconds to find the OBSS gone */
			if ((po_bss->obss_bwsw_no_activity_cfm_count_max +
					prot->cfg_dbs->obss_bwsw_no_activity_cfm_count_incr_cfg) <
					OBSS_BWSW_NO_ACTIVITY_MAX_INCR_DEFAULT) {
				po_bss->obss_bwsw_no_activity_cfm_count_max +=
					prot->cfg_dbs->obss_bwsw_no_activity_cfm_count_incr_cfg;
			} else {
				po_bss->obss_bwsw_no_activity_cfm_count_max =
					OBSS_BWSW_NO_ACTIVITY_MAX_INCR_DEFAULT;
			}
		} else {
			/* reset counter which used to detect that obss is gone */
			po_bss->obss_bwsw_no_activity_cfm_count_max =
				prot->cfg_dbs->obss_bwsw_no_activity_cfm_count_cfg;
		}
		if (po_bss->obss_activity == OBSS_INACTIVE) {
			wlc_obss_dynbw_set_obss_activity_info(prot, notif_data->cfg,
				po_bss);
			prot->obss_active_cnt++;
		}
		break;
	case (MODESW_ACTION_FAILURE):
		/* The flag MODESW_CTRL_NO_ACK_DISASSOC will ensure
		* that action frame failures are handled in modesw module
		* by way of disassoc
		*/
		break;
	case (MODESW_UP_AP_COMPLETE) :
		WL_MODE_SWITCH(("Caught Signal = MODESW_UP_AP_COMPLETE\n"));
		WL_MODE_SWITCH(("value of the psd state = %d\n",
			po_bss->bwswpsd_state));
		WL_MODE_SWITCH(("Value of obss activity = %d\n",
			po_bss->obss_activity));
	/* Fall through */
	case (MODESW_UP_STA_COMPLETE):
		if (po_bss->bwswpsd_state == BWSW_PSEUDOUPGD_ACTIVE) {
			WL_MODE_SWITCH(("pseudo:ACTCmplt:st:pACTIVE->"
				"pNOTACTIVE:\n"));
			po_bss->bwswpsd_state = BWSW_PSEUDOUPGD_NOTACTIVE;
		} else {
			WL_MODE_SWITCH(("ACT_COMPLETE:non-pseudo"
				"UP-done.,state:%d!,evt:%d\n",
				po_bss->bwswpsd_state, notif_data->signal));
		}
		wlc_obss_dynbw_set_obss_activity_info(prot, notif_data->cfg,
			po_bss);
		po_bss->bwswpsd_cspec_for_tx = INVCHANSPEC;
		/* reset counter which used to detect that obss is gone */
		po_bss->obss_bwsw_no_activity_cfm_count_max =
			prot->cfg_dbs->obss_bwsw_no_activity_cfm_count_cfg;
		break;
	default:
		po_bss->is_obss_modesw_call = TRUE;
	}
}

chanspec_t
wlc_obss_dynbw_ht_chanspec_override(wlc_obss_dynbw_t *prot,
		wlc_bsscfg_t *bsscfg, chanspec_t beacon_chanspec)
{
	dynbw_bsscfg_cubby_t *po_bss = DYNBW_BSSCFG_CUBBY(prot, bsscfg);
	uint16 bw = WL_CHANSPEC_BW_20;
	uint16 bw_bcn = CHSPEC_BW(beacon_chanspec);
	uint8 ctl_chan = 0;

	/* When ULB operations are enabled no processing is needed based on BSS`s operating
	 * mode as expectation is beacon_chanspec is < 20MHz.
	 */
	if (WLC_MODESW_ENAB(prot->wlc->pub) &&
#ifdef WL11ULB_FIXME
		!BSSCFG_ULB_ENAB(prot->wlc, bsscfg) &&
#endif /* WL11ULB_FIXME */
		TRUE) {
		if (po_bss->obss_activity == OBSS_ACTIVE_HT) {
			bw = wlc_modesw_get_bw_from_opermode(bsscfg->oper_mode, bw_bcn);
			bw = MIN(bw, bw_bcn);

			/* if bw is 20 and bw_bcn is 40, we need to get ctrl channel
			 * and use it for making chanspec
			 */
			if (bw == WL_CHANSPEC_BW_20) {
				ctl_chan = wf_chspec_ctlchan(beacon_chanspec);
				return CH20MHZ_CHSPEC(ctl_chan);
			}
		}
	}
	return beacon_chanspec;
}

/* Function restores all prot_obss variables and states to
* default values
*/
static void
wlc_obss_dynbw_restore_defaults(wlc_obss_dynbw_t *prot, wlc_bsscfg_t *bsscfg,
		chanspec_t chspec)
{
	dynbw_bsscfg_cubby_t *po_bss = DYNBW_BSSCFG_CUBBY(prot, bsscfg);

	if (prot->dyn_bwsw_enabled == OBSS_DYN_BWSW_DISABLE) {
		return;
	}

	po_bss->orig_chanspec = chspec;
	WL_MODE_SWITCH(("Orig chanspec = %x\n", po_bss->orig_chanspec));

	po_bss->obss_activity = OBSS_INACTIVE;
	if (prot->obss_active_cnt != 0)
		prot->obss_active_cnt--;
	po_bss->bwswpsd_state = BWSW_PSEUDOUPGD_NOTACTIVE;

	if (prot->obss_active_cnt == 0)
		prot->main_chanspec = INVCHANSPEC;
	po_bss->bwswpsd_cspec_for_tx = INVCHANSPEC;
	po_bss->obss_bwsw_no_activity_cfm_count_max =
		prot->cfg_dbs->obss_bwsw_no_activity_cfm_count_cfg;
	po_bss->obss_bwsw_activity_cfm_count = 0;
	po_bss->obss_bwsw_no_activity_cfm_count = 0;
	po_bss->obss_bwsw_pseudo_sense_count = 0;
}

/* Callback from assoc. This Function will reset
* the bsscfg variables when STA association state gets updated.
*/
static void
wlc_obss_dynbw_assoc_cxt_cb(void *ctx, bss_assoc_state_data_t *notif_data)
{
	wlc_obss_dynbw_t *prot = (wlc_obss_dynbw_t *)ctx;
	wlc_bsscfg_t *bsscfg = notif_data->cfg;

	ASSERT(notif_data->cfg != NULL);
	ASSERT(ctx != NULL);
	WL_MODE_SWITCH(("%s:Got Callback from Assoc. resetting prot_obss\n",
		__FUNCTION__));

	if (BSSCFG_AP(bsscfg))
		return;

	wlc_obss_dynbw_restore_defaults(prot, bsscfg,
			bsscfg->current_bss->chanspec);
}

/* Callback from wldown. This Function will reset
* the bsscfg variables on wlup event.
*/
static void
wlc_obss_dynbw_updown_cb(void *ctx, bsscfg_up_down_event_data_t *updown_data)
{
	wlc_obss_dynbw_t *prot = (wlc_obss_dynbw_t *)ctx;
	wlc_bsscfg_t *bsscfg = updown_data->bsscfg;

	ASSERT(bsscfg != NULL);
	ASSERT(prot != NULL);

	if (updown_data->up == FALSE)
		return;

	WL_MODE_SWITCH(("%s:got callback from updown. intf up resetting prot_obss\n",
		__FUNCTION__));

	wlc_obss_dynbw_restore_defaults(prot, bsscfg,
			bsscfg->current_bss->chanspec);
}

/* Update Chanspec to Original Chanspec if Pseudo State is in Progress.
* So that update beacon op_ie willnot update the chanspec back in beacon
*/
void
wlc_obss_dynbw_beacon_chanspec_override(wlc_obss_dynbw_t *prot,
		wlc_bsscfg_t *bsscfg, chanspec_t *chanspec)
{
	dynbw_bsscfg_cubby_t *po_bss;

	po_bss = DYNBW_BSSCFG_CUBBY(prot, bsscfg);

	if (po_bss->bwswpsd_cspec_for_tx != INVCHANSPEC) {
		*chanspec = po_bss->bwswpsd_cspec_for_tx;
		WL_MODE_SWITCH(("%s,bwswpsd_state %d ,bwswpsd_cspec_for_tx = %d,"
			"chanspec:%x\n",
			__FUNCTION__, po_bss->bwswpsd_state,
			po_bss->bwswpsd_cspec_for_tx,
			*chanspec));
	}
}
#ifdef WLCSA
static void
wlc_obss_dynbw_csa_cb(void *ctx, wlc_csa_notif_cb_data_t *notif_data)
{
	wlc_obss_dynbw_t *prot = (wlc_obss_dynbw_t *)ctx;
	wlc_bsscfg_t *bsscfg = notif_data->cfg;
	dynbw_bsscfg_cubby_t *po_bss = DYNBW_BSSCFG_CUBBY(prot, bsscfg);

	ASSERT(notif_data->cfg != NULL);
	ASSERT(ctx != NULL);

	switch (notif_data->signal) {
		case CSA_CHANNEL_CHANGE_START:
			if (!po_bss->is_csa_lock) {
				/* csa started */
				po_bss->is_csa_lock = TRUE;
				WL_MODE_SWITCH(("%s:Got Callback from CSA,"
						"resetting prot_obss to 0x%x\n", __FUNCTION__,
						notif_data->chanspec));
				wlc_obss_dynbw_restore_defaults(prot, bsscfg, notif_data->chanspec);
			}
			break;
		case CSA_CHANNEL_CHANGE_END:
			/* csa done */
			WL_MODE_SWITCH(("%s:Got Callback from CSA,"
			   " CSA switch end\n", __FUNCTION__));
			po_bss->is_csa_lock = FALSE;
			break;
	}
}
#endif /* WLCSA */
/* end of file */
