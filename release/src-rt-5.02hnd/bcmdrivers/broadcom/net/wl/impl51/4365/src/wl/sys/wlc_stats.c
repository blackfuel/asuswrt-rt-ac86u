/*
 * wlc/related statistics and test/verification counters for
 * Broadcom 802.11bang Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_stats.c 405986 2013-07-05 15:22:22Z   $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wl_dbg.h>

#include <wlc_stats.h>

/* iovar table */
enum {
	IOV_STATS_BCNS_INFO,
	IOV_STATS_LAST
};

static const bcm_iovar_t wlc_stats_iovars[] = {
	{"bcnlenhist", IOV_STATS_BCNS_INFO, (0), IOVT_BUFFER, 0},
	{NULL, 0, 0, 0, 0}
};

struct wlc_stats_info {
	wlc_info_t *wlc;
	wlc_bcn_len_hist_t	*bcn_len_hist;
};

#define	MAX_BCNS_LEN_STORED	10

static int
wlc_stats_get_bcnlenhist(wlc_bcn_len_hist_t *, void *, int32);

static void
wlc_stats_clear_bcnlenhist(wlc_bcn_len_hist_t *);

static int
wlc_stats_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *p, uint plen, void *arg, int alen, int vsize,
	struct wlc_if *wlcif);

wlc_stats_info_t *
BCMATTACHFN(wlc_stats_attach)(wlc_info_t *wlc)
{
	wlc_stats_info_t *stats_info = NULL;

	if (!(stats_info = (wlc_stats_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_stats_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}
	stats_info->wlc = wlc;

	/* register module */
	if (wlc_module_register(wlc->pub, wlc_stats_iovars, "stats",
		stats_info, wlc_stats_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* Memory allocation for structure wlc_bcn_len_hist_t */
	stats_info->bcn_len_hist = (wlc_bcn_len_hist_t *)MALLOCZ(wlc->osh,
		WL_LAST_BCNS_INFO_FIXED_LEN + (MAX_BCNS_LEN_STORED * sizeof(uint32)));
	if (!stats_info->bcn_len_hist) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	/* Ring Buffer length */
	stats_info->bcn_len_hist->ringbuff_len = MAX_BCNS_LEN_STORED;

	/* Min_bcn_len needs to be initialised with max int value, to find min bcns len */
	stats_info->bcn_len_hist->min_bcnlen = 0x7fffffff;

	/* Enable 'STATS' mode */
	wlc->pub->_wlstats = TRUE;

	return stats_info;

fail:
	wlc_stats_detach(stats_info);
	return NULL;
}

void
BCMATTACHFN(wlc_stats_detach)(wlc_stats_info_t *stats_info)
{
	wlc_module_unregister(stats_info->wlc->pub, "stats", stats_info);

	MFREE(stats_info->wlc->osh, stats_info->bcn_len_hist, sizeof(wlc_bcn_len_hist_t));
	MFREE(stats_info->wlc->osh, stats_info, sizeof(wlc_stats_info_t));
}

static int
wlc_stats_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_stats_info_t *stats_info = hdl;
	wlc_info_t *wlc = stats_info->wlc;
	int32 int_val = 0;
	wlc_bsscfg_t *cfg;
	int err = BCME_OK;

	cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(cfg != NULL);
	UNUSED_PARAMETER(cfg);

	/* Get IOVAR paramter */
	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	switch (actionid) {
		case IOV_GVAL(IOV_STATS_BCNS_INFO):
		{
			err = wlc_stats_get_bcnlenhist(stats_info->bcn_len_hist, arg, int_val);

			break;
		}

		default:
		{
			err = BCME_UNSUPPORTED;
			break;
		}
	}

	return (err);
}

void
wlc_stats_update_bcnlenhist(wlc_stats_info_t *stats_info, int len)
{
	wlc_bcn_len_hist_t *bcns_info = stats_info->bcn_len_hist;
	uint32	*bcns_len = bcns_info->bcnlen_ring;

	/* Received beacon lengths are into ring buffer */
	bcns_len[bcns_info->cur_index++] = len;
	if (bcns_info->cur_index >= MAX_BCNS_LEN_STORED)
		bcns_info->cur_index = 0;

	/* Chech for the Maximum beacon length received after scan/join/reset */
	if (bcns_info->max_bcnlen < len)
		bcns_info->max_bcnlen = len;

	/* Chech for the Minimum beacon length received after scan/join/reset */
	if (bcns_info->min_bcnlen > len)
		bcns_info->min_bcnlen = len;
}

void
wlc_stats_clear_bcnlenhist(wlc_bcn_len_hist_t *bcns_info)
{
	uint32	ringbuff_len = 0;

	/* save 'Ring Buffer Length' before clearing 'wlc_bcn_len_hist_t' structure data */
	ringbuff_len = bcns_info->ringbuff_len;

	bzero(bcns_info,
		(WL_LAST_BCNS_INFO_FIXED_LEN + MAX_BCNS_LEN_STORED * sizeof(uint32)));

	/* Retrive the 'Ring Buffer Length' stored before clearing 'wlc_bcn_len_hist_t' */
	bcns_info->ringbuff_len = ringbuff_len;

	/* Reinitialize 'Min_bcn_len' with max int value, to find min bcns len */
	bcns_info->min_bcnlen = (int)0x7fffffff;
}

static int
wlc_stats_get_bcnlenhist(wlc_bcn_len_hist_t *recvd_bcns, void *arg, int32 int_val)
{
	wlc_bcn_len_hist_t *bcns_info = (wlc_bcn_len_hist_t *)arg;

	/* If received parameter is not '0', then inform WL Utility about Bad Argument */
	if (int_val && (0x30 != int_val))
		return BCME_BADARG;

	/* Send complete 'wlc_bcn_len_hist_t' structure data to WL Utility */
	memcpy(bcns_info, recvd_bcns,
		(WL_LAST_BCNS_INFO_FIXED_LEN + MAX_BCNS_LEN_STORED * sizeof(uint32)));

	/* If received paramter is '0', then clear 'wlc_bcn_len_hist_t' structure data */
	if (int_val == 0x30)
		wlc_stats_clear_bcnlenhist(recvd_bcns);

	return BCME_OK;
}
