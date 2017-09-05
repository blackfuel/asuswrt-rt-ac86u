/**
 * @file
 * @brief
 * Support P2P discovery state machine in the driver
 * for the P2P ofload (p2po).
 * See bcm_p2p_disc.c and wlc_p2po for the APIs.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wl_p2po_disc.c 468737 2014-04-08 17:59:20Z $
 *
 */


#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmendian.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scan.h>

#include <bcm_p2p_disc.h>
#include <wl_p2po_disc.h>


/* disc private info structure */
struct wl_disc_info {
	wlc_info_t		*wlc;			/* Pointer back to wlc structure */
};

/* wlc_pub_t struct access macros */
#define WLCUNIT(x)	((x)->wlc->pub->unit)
#define WLCOSH(x)	((x)->wlc->osh)


/* get P2P device/discovery bsscfg index */
int32
wl_disc_get_p2p_devcfg_idx(void *w, int32 *idx)
{
	wlc_info_t *wlc = (wlc_info_t *)w;

	return wlc_iovar_op(wlc, "p2p_dev", NULL, 0,
		(void *)idx, sizeof(*idx), IOV_GET, NULL);
}

/* get P2P discovery bsscfg */
static wlc_bsscfg_t *
wl_disc_get_p2p_devcfg(wlc_info_t *wlc)
{
	wlc_bsscfg_t *bsscfg;
	int32 idx;
	int err;

	if (wl_disc_get_p2p_devcfg_idx(wlc, &idx) == BCME_ERROR)
		return NULL;

	bsscfg = wlc_bsscfg_find(wlc, idx, &err);

	if (err != BCME_NOTFOUND)
		return bsscfg;
	else
		/* device bsscfg has not been created yet */
		return NULL;
}

/* set p2p discovery state */
int
wl_disc_p2p_state(void *w, uint8 state, chanspec_t chspec, uint16 dwell)
{
	wlc_info_t *wlc = (wlc_info_t *)w;
	wl_p2p_disc_st_t disc_state;
	wlc_bsscfg_t *bsscfg;

	/* get device bsscfg first */
	bsscfg = wl_disc_get_p2p_devcfg(wlc);

	/* device bsscfg has not been created yet */
	if (!bsscfg)
		return BCME_ERROR;

	disc_state.state = state;
	disc_state.chspec = chspec;
	disc_state.dwell = dwell;
	return wlc_iovar_op(wlc, "p2p_state", NULL, 0,
		(void *)&disc_state, sizeof(disc_state), IOV_SET,
		bsscfg->wlcif);
}

/* get discovery bsscfg */
wlc_bsscfg_t *
wl_disc_bsscfg(wl_disc_info_t *disc)
{
	wlc_info_t *wlc = disc->wlc;

	return wl_disc_get_p2p_devcfg(wlc);
}

/* do p2p scan */
int
wl_disc_p2p_scan(void *w, uint16 sync_id, int is_active,
	int num_probes, int active_dwell_time, int passive_dwell_time, int home_time,
	int num_channels, uint16 *channels, uint8 flags)
{
	wlc_info_t *wlc = (wlc_info_t *)w;
	wl_p2p_scan_t *params = NULL;
	int malloc_size = 0;
	int nssid = 0;
	int err = 0;
	wl_escan_params_t *eparams;
	wlc_bsscfg_t *bsscfg;

	/* get device bsscfg first */
	bsscfg = wl_disc_get_p2p_devcfg(wlc);

	/* device bsscfg has not been created yet */
	if (!bsscfg)
		return BCME_ERROR;

	malloc_size = sizeof(wl_p2p_scan_t);
	malloc_size += OFFSETOF(wl_escan_params_t, params) +
		WL_SCAN_PARAMS_FIXED_SIZE + num_channels * sizeof(uint16);
	malloc_size += nssid * sizeof(wlc_ssid_t);
	params = (wl_p2p_scan_t *)MALLOC(wlc->osh, malloc_size);
	if (params == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
		          WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}
	memset(params, 0, malloc_size);

	eparams = (wl_escan_params_t *)(params+1);

	params->type = 'E';

	eparams->version = ESCAN_REQ_VERSION;
	eparams->action = WL_SCAN_ACTION_START;
	eparams->sync_id = sync_id;

	memcpy(&eparams->params.bssid, &ether_bcast, ETHER_ADDR_LEN);
	eparams->params.bss_type = DOT11_BSSTYPE_ANY;
	eparams->params.scan_type = is_active ? 0 : WL_SCANFLAGS_PASSIVE;
	eparams->params.nprobes = num_probes;
	eparams->params.active_time = active_dwell_time;
	eparams->params.passive_time = passive_dwell_time;
	eparams->params.home_time = home_time;
	if ((flags & P2PO_FIND_FLAG_SCAN_ALL_APS) == 0)
		err = wlc_iovar_op(wlc, "p2p_ssid", NULL, 0, &eparams->params.ssid,
			sizeof(eparams->params.ssid), IOV_GET, bsscfg->wlcif);

	if (num_channels)
		memcpy(eparams->params.channel_list, channels, num_channels * sizeof(uint16));

	eparams->params.channel_num = (nssid << WL_SCAN_PARAMS_NSSID_SHIFT) |
		(num_channels & WL_SCAN_PARAMS_COUNT_MASK);

	err = wlc_iovar_op(wlc, "p2p_scan", NULL, 0,
		(void *)params, malloc_size, IOV_SET, bsscfg->wlcif);

	MFREE(wlc->osh, params, malloc_size);
	return err;
}


/*
 * initialize disc private context.
 * returns a pointer to the disc private context, NULL on failure.
 */
wl_disc_info_t *
BCMATTACHFN(wl_disc_attach)(wlc_info_t *wlc)
{
	wl_disc_info_t *disc;

	/* allocate disc private info struct */
	disc = MALLOC(wlc->osh, sizeof(wl_disc_info_t));
	if (!disc) {
		WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
		          WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* init disc private info struct */
	bzero(disc, sizeof(wl_disc_info_t));
	disc->wlc = wlc;

	return disc;
}

/* cleanup disc private context */
void
BCMATTACHFN(wl_disc_detach)(wl_disc_info_t *disc)
{
	WL_INFORM(("wl%d: disc_detach()\n", WLCUNIT(disc)));

	if (!disc)
		return;

	MFREE(WLCOSH(disc), disc, sizeof(wl_disc_info_t));

	disc = NULL;
}
