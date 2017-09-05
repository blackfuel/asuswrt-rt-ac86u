/*
 * SW probe response module source file
 * disable ucode sending probe response,
 * driver will decide whether send probe response,
 * after check the received probe request.
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_probresp.c 552304 2015-04-27 02:49:55Z $
 */

#include <wlc_cfg.h>

#ifdef WLPROBRESP_SW

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_tpc.h>
#include <wlc_csa.h>
#include <wlc_quiet.h>
#include <wlc_bmac.h>
#include <wlc_probresp.h>

#define WLC_PROBRESP_MAXFILTERS		3	/* max filter functions */
#define WLC_PROBRESP_INVALID_INDEX	-1

#define WLC_PROBRESP_MEM_THRESHOLD_LOW(x)      \
	(((x)->wlc->pub->tunables->min_scballoc_mem) + 3)

typedef	struct probreqcb {
	void *hdl;
	probreq_filter_fn_t filter_fn;
} probreqcb_t;

/* IOVar table */
/* No ordering is imposed */
enum {
	IOV_PROBRESP_SW, /* SW probe response enable/dsiable */
	IOV_LAST
};

static const bcm_iovar_t wlc_probresp_iovars[] = {
	{"probresp_sw", IOV_PROBRESP_SW, (0), IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0}
};

/* SW probe response module info */
struct wlc_probresp_info {
	wlc_info_t *wlc;
	probreqcb_t probreq_filters[WLC_PROBRESP_MAXFILTERS];
	int p2p_index;
};

/* local functions */
/* module */
static int wlc_probresp_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);

wlc_probresp_info_t *
BCMATTACHFN(wlc_probresp_attach)(wlc_info_t *wlc)
{
	wlc_probresp_info_t *mprobresp;

	if (!wlc)
		return NULL;

	mprobresp = MALLOCZ(wlc->osh, sizeof(wlc_probresp_info_t));
	if (mprobresp == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	mprobresp->wlc = wlc;
	mprobresp->p2p_index = WLC_PROBRESP_INVALID_INDEX;

	/* keep the module registration the last other add module unregistration
	 * in the error handling code below...
	 */
	if (wlc_module_register(wlc->pub, wlc_probresp_iovars, "probresp", mprobresp,
		wlc_probresp_doiovar, NULL, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

	return mprobresp;

	/* error handling */
fail:
	if (mprobresp != NULL)
		MFREE(wlc->osh, mprobresp, sizeof(wlc_probresp_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_probresp_detach)(wlc_probresp_info_t *mprobresp)
{
	wlc_info_t *wlc;

	if (!mprobresp)
		return;
	wlc = mprobresp->wlc;
	wlc_module_unregister(wlc->pub, "probresp", mprobresp);

	MFREE(wlc->osh, mprobresp, sizeof(wlc_probresp_info_t));
}

static int
wlc_probresp_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_probresp_info_t *mprobresp = (wlc_probresp_info_t *)ctx;
	wlc_info_t *wlc = mprobresp->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	bool bool_val;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0);

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
	case IOV_GVAL(IOV_PROBRESP_SW):
		*ret_int_ptr = (int32)wlc->pub->_probresp_sw;
		break;
	case IOV_SVAL(IOV_PROBRESP_SW):
		if (wlc->pub->_probresp_sw != bool_val) {
			wlc->pub->_probresp_sw = bool_val;

			wlc_enable_probe_req(wlc, PROBE_REQ_PROBRESP_MASK,
				bool_val ? PROBE_REQ_PROBRESP_MASK : 0);

			wlc_disable_probe_resp(wlc, PROBE_RESP_SW_MASK,
				bool_val ? PROBE_RESP_SW_MASK : 0);
		}
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static void
wlc_probresp_send_probe_resp(wlc_probresp_info_t *mprobresp, wlc_bsscfg_t *bsscfg,
	struct ether_addr *da)
{
	wlc_info_t *wlc;
	void *p;
	uint8 *pbody;
	int len;

	wlc = mprobresp->wlc;

	ASSERT(bsscfg != NULL);

	ASSERT(wlc->pub->up);

	len = wlc->pub->bcn_tmpl_len;

#ifdef BCMPCIEDEV
	 /* for full dongle operation, do not queue Probe response packets
	  * while channel switch is in progress
	  */
	if (BCMPCIEDEV_ENAB() && (wlc_bmac_tx_fifo_suspended(wlc->hw, TX_CTL_FIFO)))
		return;
#endif
	/* build response and send */
	if ((p = wlc_frame_get_mgmt(wlc, FC_PROBE_RESP, da, &bsscfg->cur_etheraddr,
	                            &bsscfg->BSSID, len, &pbody)) == NULL) {
		WL_ERROR(("wl%d.%d: %s: wlc_frame_get_mgmt failed\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
	}
	else {
		/* Generate probe response body */
		wlc_bcn_prb_body(wlc, FC_PROBE_RESP, bsscfg, pbody, &len, FALSE);
		PKTSETLEN(wlc->osh, p, len + DOT11_MGMT_HDR_LEN);
#ifdef BCMPCIEDEV
		/* For full dongle operation, do not queue probe response packets into psq */
		if (BCMPCIEDEV_ENAB())
			WLPKTTAG(p)->flags |= WLF_PSDONTQ;
#endif /* BCMPCIEDEV */

		wlc_sendmgmt(wlc, p, bsscfg->wlcif->qi, NULL);
	}
}

static bool
wlc_probresp_filter_probe_request(wlc_probresp_info_t *mprobresp, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr,
	uint8 *body, int body_len)
{
	int i;
	bool sendProbeResp = TRUE;

	/* if network type is closed then don't send prob response */
	if ((cfg->closednet_nobcprbresp) ||
		(OSL_MEM_AVAIL() <= WLC_PROBRESP_MEM_THRESHOLD_LOW(mprobresp)) ||
		(hnd_pktid_free_cnt() < 64)) {
		bcm_tlv_t *ssid;
		/* If it is a directed probe request, we need to send out the response */
		ssid = bcm_parse_tlvs(body, body_len, DOT11_MNG_SSID_ID);
		if (ssid == NULL)
			return FALSE;
		/* Check if the ssid in the probe request matches our bsscfg->SSID */
		if ((ssid->len == 0) || (ssid->len != cfg->SSID_len) ||
			(bcmp(ssid->data, cfg->SSID, ssid->len) != 0)) {
			if ((eacmp(&hdr->da, &cfg->cur_etheraddr) != 0) &&
				((eacmp(&hdr->bssid, &cfg->BSSID)) != 0))
				return FALSE;
		}
	}

	for (i = 0; i < WLC_PROBRESP_MAXFILTERS; i ++) {
		if ((i != mprobresp->p2p_index) && mprobresp->probreq_filters[i].hdl) {
			if (!(mprobresp->probreq_filters[i].filter_fn(
				mprobresp->probreq_filters[i].hdl, cfg,
				wrxh, plcp, hdr, body, body_len, NULL)))
				sendProbeResp = FALSE;
		}
	}

	/* call p2p filter function last */
	if (mprobresp->p2p_index != WLC_PROBRESP_INVALID_INDEX) {
		sendProbeResp = mprobresp->probreq_filters[mprobresp->p2p_index].filter_fn(
			mprobresp->probreq_filters[mprobresp->p2p_index].hdl, cfg,
			wrxh, plcp, hdr, body, body_len, &sendProbeResp);
	}

	if (sendProbeResp) {
		wlc_probresp_send_probe_resp(mprobresp, cfg, &hdr->sa);
	}

	return sendProbeResp;
}

/* register filter function */
int
BCMATTACHFN(wlc_probresp_register)(wlc_probresp_info_t *mprobresp, void *hdl,
	probreq_filter_fn_t filter_fn, bool p2p)
{
	int i;
	if (!mprobresp || !hdl || !filter_fn)
		return BCME_BADARG;

	/* find an empty entry and just add, no duplication check! */
	for (i = 0; i < WLC_PROBRESP_MAXFILTERS; i ++) {
		if (!mprobresp->probreq_filters[i].hdl) {
			mprobresp->probreq_filters[i].hdl = hdl;
			mprobresp->probreq_filters[i].filter_fn = filter_fn;
			if (p2p)
				mprobresp->p2p_index = i;
			return BCME_OK;
		}
	}

	/* it is time to increase the capacity */
	ASSERT(i < WLC_PROBRESP_MAXFILTERS);
	return BCME_NORESOURCE;
}
/* register filter function */
int
BCMATTACHFN(wlc_probresp_unregister)(wlc_probresp_info_t *mprobresp, void *hdl)
{
	int i;
	if (!mprobresp || !hdl)
		return BCME_BADARG;

	for (i = 0; i < WLC_PROBRESP_MAXFILTERS; i ++) {
		if (mprobresp->probreq_filters[i].hdl == hdl) {
			bzero(&mprobresp->probreq_filters[i], sizeof(probreqcb_t));
			return BCME_OK;
		}
	}

	/* table not found! */
	return BCME_NOTFOUND;
}

/* process probe request frame */
void
wlc_probresp_recv_process_prbreq(wlc_probresp_info_t *mprobresp, wlc_d11rxhdr_t *wrxh,
	uint8 *plcp, struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;
	wlc_bsscfg_t *bsscfg_hwaddr;
	bcm_tlv_t *ssid;

	if (!mprobresp)
		return;

	wlc = mprobresp->wlc;

	if (!WLPROBRESP_SW_ENAB(wlc))
		return;

	if ((ssid = bcm_parse_tlvs(body, body_len, DOT11_MNG_SSID_ID)) != NULL) {
		wlc_bsscfg_t *cfg;
		int idx;
		bsscfg_hwaddr = wlc_bsscfg_find_by_hwaddr(wlc, &hdr->da);
		bsscfg = wlc_bsscfg_find_by_bssid(wlc, &hdr->bssid);
		if (bsscfg || bsscfg_hwaddr) {
			cfg = bsscfg ? bsscfg : bsscfg_hwaddr;
			if (BSSCFG_AP(cfg) && cfg->up && cfg->enable &&
			            ((bsscfg == bsscfg_hwaddr) ||
			            ETHER_ISBCAST(&hdr->da) ||
			            ETHER_ISBCAST(&hdr->bssid)) &&
			            ((ssid->len == 0) ||
			            ((ssid->len == cfg->SSID_len) &&
			            (bcmp(ssid->data, cfg->SSID, ssid->len) == 0)))) {
				wlc_probresp_filter_probe_request(mprobresp, cfg,
					wrxh, plcp, hdr, body, body_len);
			}
		} else if (ETHER_ISBCAST(&hdr->da) && ETHER_ISBCAST(&hdr->bssid)) {
			FOREACH_UP_AP(wlc, idx, cfg) {
				if (cfg->enable && ((ssid->len == 0) ||
				            ((ssid->len == cfg->SSID_len) &&
				            (bcmp(ssid->data, cfg->SSID,
				            ssid->len) == 0)))) {
					wlc_probresp_filter_probe_request(mprobresp, cfg,
						wrxh, plcp, hdr, body, body_len);
				}
			}
		}
	}
}

#endif /* WLPROBRESP_SW */
