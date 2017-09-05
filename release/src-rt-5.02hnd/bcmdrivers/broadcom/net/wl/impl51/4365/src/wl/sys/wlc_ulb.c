/**
 * @file
 * @brief
 * Ultra Low Bandwidth (ULB) functionality for Broadcom 802.11 Networking Driver
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_ulb.c 570838 2015-07-13 18:44:44Z $
 */

/**
 * @file
 * @brief
 * Twiki: http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/UltraLowBandMode
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <wlioctl.h>
#include <wlc.h>
#include <wlc_bmac.h>
#ifdef WL11ULB
#include <wlc_ulb.h>
#endif
#include <wlc_hw_priv.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_scb.h>
#include <wlc_scb_ratesel.h>
#include <wlc_tbtt.h>
#include <wlc_tx.h>
#include <wlc_pcb.h>
#include <wlc_scan.h>
#include <wlc_assoc.h>
#include <wlc_bsscfg.h>

#ifdef PROP_TXSTATUS
#include <wlc_p2p.h>
#include <wlfc_proto.h>
#endif /* PROP_TXSTATUS */
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif /* WLMCHAN */
#include <proto/bcmulb.h>
#include <hndpmu.h>


/* iovar table */
enum {
	IOV_ULB_MODE = 0,	/* Configure ULB Mode feature */
	IOV_ULB_BW = 1,		/* Configure ULB Mode BW for a BSSCFG */
	IOV_ULB_MODE_SW_REQ = 2
};

static const bcm_iovar_t ulb_iovars[] = {
	{"ulb_mode", IOV_ULB_MODE, (IOVF_SET_DOWN), IOVT_UINT8, 0},
	{"ulb_bw", IOV_ULB_BW, (IOVF_OPEN_ALLOW), IOVT_UINT8, 0},
	{"ulb_mode_sw_req", IOV_ULB_MODE_SW_REQ, (IOVF_SET_UP), IOVT_UINT32, 0},
	{NULL, 0, 0, 0, 0}
};

/* ULB module specific information */
struct wlc_ulb_info {
	wlc_info_t *wlc;	/* pointer to main wlc structure */
	wlc_pub_t *pub;
	osl_t *osh;
	uint8 ulb_mode;
	uint32 cfgh;
	uint16 prev_bw;
};

/* ULB Mode-SW specific information */
typedef struct ulb_mode_sw_info {
	bool   mode_sw_inprog;		/* Flag to indicate if ULB Mode-SW is in progress */
	bool   process_tbtt;		/* Flag to indicate TBTT processing is needed */
	uint8  mode_sw_bw;		/* Mode SW BW value (ulb_bw_type_t) */
	uint8  mode_sw_chnum;		/* Mode SW Channel Number value */
	uint8  mode_sw_cnt;		/* Mode SW Count value. Indicates number of TBTTs until
	 * the STA sending the Mode-SW Attribute switches to the new channel or is set to 0.
	 * A value of 1 indicates that the switch occurs immediately before the next TBTT.
	 * A value of 0 indicates that the switch occurs at any time after the frame containing
	 * the element is transmitted
	 */
	uint8  mode_sw_token;		/* Dialog Token received in Mode-SW Request.
	 * Currently, simulatneous processing more than one request is not supported.
	 * Incase we are planning to extend it to multiple req/resp then need to maintain a list
	 * of dailog-token/addr and perform the processing.
	 */
	uint8  mode_sw_af_rtcnt;	/* Mode SW Request/Response Retry Count value */
	void  *pkt;			/* Pointer to AF packet used during retry */
} ulb_mode_sw_info_t;

/* BSSCFG specific information */
typedef struct bss_ulb_info {
	uint16 min_bw;
	ulb_mode_sw_info_t mode_sw_info;
} bss_ulb_info_t;


#define DEFAULT_MODE_SW_BEACON_CNT	5 /* Number of TBTTs */
#define MAX_NUM_MODE_SW_AF_RETRY_CNT	1 /* Number of Retries allowed for Mode-SW Action Frame */
#define BRCM_ULB_IE_BUILD_FST_BMP	(FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP) | \
					FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP) | \
					FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ) | \
					FT2BMP(FC_PROBE_REQ))

#define BRCM_ULB_IE_SCAN_PARSE_FST_BMP	(FT2BMP(WLC_IEM_FC_SCAN_BCN) | \
					FT2BMP(WLC_IEM_FC_SCAN_PRBRSP))

#define BRCM_ULB_IE_PARSE_FST_BMP	(FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP) | \
						FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP) | \
						FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ))

/* bsscfg specific info access accessor */
#define ULB_BSSCFG_CUBBY(ulb_info, cfg) ((bss_ulb_info_t *)BSSCFG_CUBBY((cfg), (ulb_info)->cfgh))

#define WLC_ULB_INFO_SIZE		(sizeof(wlc_ulb_info_t))
#define ULB_MODE_GET(ulb_info)		(ulb_info->ulb_mode)
#define BSS_ULB_SET_MODE_SW_INPROG(bss_ulb, val)	(\
						(bss_ulb)->mode_sw_info.mode_sw_inprog = (val))
#define BSS_ULB_SET_MODE_SW_PROC_TBTT(bss_ulb, val)	((bss_ulb)->mode_sw_info.process_tbtt = \
							(val))
#define BSS_ULB_SET_MODE_SW_BW(bss_ulb, val)		((bss_ulb)->mode_sw_info.mode_sw_bw = \
							(val))
#define BSS_ULB_SET_MODE_SW_CHNUM(bss_ulb, val)		((bss_ulb)->mode_sw_info.mode_sw_chnum = \
							(val))
#define BSS_ULB_SET_MODE_SW_CNT(bss_ulb, val)		((bss_ulb)->mode_sw_info.mode_sw_cnt = \
							(val))
#define BSS_ULB_SET_MODE_SW_TOKEN(bss_ulb, val)		((bss_ulb)->mode_sw_info.mode_sw_token = \
							(val))
#define BSS_ULB_SET_MODE_SW_AF_RTCNT(bss_ulb, val)	((bss_ulb)->mode_sw_info.mode_sw_af_rtcnt\
							= (val))
#define BSS_ULB_SET_MODE_SW_PKT(bss_ulb, val)		((bss_ulb)->mode_sw_info.pkt = (val))
#define BSS_ULB_GET_MODE_SW_INPROG(bss_ulb)		((bss_ulb)->mode_sw_info.mode_sw_inprog)
#define BSS_ULB_GET_MODE_SW_PROC_TBTT(bss_ulb)		((bss_ulb)->mode_sw_info.process_tbtt)
#define BSS_ULB_GET_MODE_SW_BW(bss_ulb)			((bss_ulb)->mode_sw_info.mode_sw_bw)
#define BSS_ULB_GET_MODE_SW_CHNUM(bss_ulb)		((bss_ulb)->mode_sw_info.mode_sw_chnum)
#define BSS_ULB_GET_MODE_SW_CNT(bss_ulb)		((bss_ulb)->mode_sw_info.mode_sw_cnt)
#define BSS_ULB_GET_MODE_SW_TOKEN(bss_ulb)		((bss_ulb)->mode_sw_info.mode_sw_token)
#define BSS_ULB_GET_MODE_SW_AF_RTCNT(bss_ulb)		((bss_ulb)->mode_sw_info.mode_sw_af_rtcnt)
#define BSS_ULB_GET_MODE_SW_PKT(bss_ulb)		((bss_ulb)->mode_sw_info.pkt)

static int wlc_ulb_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static bool	wlc_update_hwcap_ulb(wlc_ulb_info_t *ulb_info);
static int wlc_ulb_cfg_cubby_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_ulb_cfg_cubby_deinit(void *ctx, wlc_bsscfg_t *cfg);
static uint conv_chspec_bw_2_ulb_bw(uint16 chspec_bw);
static uint conv_ulb_bw_2_chspec_bw(uint ulb_bw);
static void wlc_ulb_bsscfg_updn(void *ctx, bsscfg_up_down_event_data_t *evt);
static uint wlc_ulb_calc_brcm_ulb_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ulb_write_brcm_ulb_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_ulb_scan_parse_brcm_ulb_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ulb_parse_brcm_ulb_ie(void *ctx, wlc_iem_parse_data_t *data);
static ulb_prop_ie_t *wlc_ulb_read_brcm_ulb_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len);
static ulb_cap_attr_t *wlc_ulb_read_cap_attr(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len);
static ulb_opr_attr_t *wlc_ulb_read_opr_attr(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len);
static ulb_mode_sw_attr_t *wlc_ulb_read_mode_sw_attr(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len);
static void wlc_ulb_build_cap_attr(wlc_info_t *wlc, uint bandunit, ulb_cap_attr_t *cap_attr);
static void wlc_ulb_build_opr_attr(wlc_info_t *wlc, wlc_bsscfg_t *cfg, chanspec_t cur_chspec,
	ulb_opr_attr_t *opr_attr);
static void wlc_ulb_build_mode_sw_attr(wlc_info_t *wlc, bss_ulb_info_t *bss_ulb, uint8 ch_num,
	ulb_mode_sw_attr_t *mode_sw_attr);
static void wlc_ulb_scan_parse_cap_attr(wlc_info_t *wlc, ulb_prop_ie_t *ulb_ie,
	wlc_iem_ft_pparm_t *ftpparm);
static void wlc_ulb_scan_parse_opr_attr(wlc_info_t *wlc, ulb_prop_ie_t *ulb_ie,
	wlc_iem_ft_pparm_t *ftpparm);
static void wlc_ulb_process_cap_attr(scb_t *scb, ulb_cap_attr_t *cap_attr);
static bool wlc_ulb_process_opr_attr(wlc_ulb_info_t *ulb_info, wlc_iem_parse_data_t *data,
	chanspec_t cur_chspec, ulb_opr_attr_t *opr_attr);
static void wlc_ulb_process_mode_sw_attr(wlc_ulb_info_t *ulb_info, wlc_bsscfg_t *cfg,
	ulb_mode_sw_attr_t *mode_sw_attr);
static bool wlc_ulb_validate_opr_field(wlc_ulb_info_t *ulb_info, chanspec_t cur_chspec,
	ulb_opr_field_t *opr_field);
static void wlc_ulb_process_mode_sw_req_af(wlc_ulb_info_t* ulb_info, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
static void wlc_ulb_process_mode_sw_rsp_af(wlc_ulb_info_t* ulb_info, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
static int wlc_send_ulb_mode_sw_req(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	uint8 dia_token, ulb_opr_field_t *opr_field, bool reg_cb);
static int wlc_send_ulb_mode_sw_rsp(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	uint8 dia_token, ulb_opr_field_t *opr_field, uint8 status, bool reg_cb);
static bool wlc_ulb_bw_supp_all_scbs(wlc_ulb_info_t* ulb_info, wlc_bsscfg_t *cfg, uint8 ulb_bw);
static int wlc_ulb_get_num_ulb_scbs(wlc_ulb_info_t* ulb_info, wlc_bsscfg_t *cfg);
static bool wlc_ulb_is_ulb_bw_supp(scb_t* scb, uint8 ulb_bw);
static void wlc_ulb_do_ulb_mode_sw(wlc_ulb_info_t* ulb_info, wlc_bsscfg_t *cfg,
	ulb_opr_field_t *opr_field, bool block_data);
static void wlc_ulb_pretbtt_cb(void *ctx, wlc_tbtt_ent_data_t *notif_data);
static void wlc_ulb_tbtt_cb(void *ctx, wlc_tbtt_ent_data_t *notif_data);
static void wlc_ulb_ulb_mode_sw_req_cb(wlc_info_t *wlc, uint tx_status, void* arg);
static void wlc_ulb_ulb_mode_sw_rsp_cb(wlc_info_t *wlc, uint tx_status, void* arg);
static void wlc_ulb_abort_mode_sw(bss_ulb_info_t *bss_ulb);

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

wlc_ulb_info_t *
BCMATTACHFN(wlc_ulb_attach)(wlc_info_t *wlc)
{
	wlc_ulb_info_t *ulb_info;

	/* Allocate info structure */
	if (!(ulb_info = (wlc_ulb_info_t *)MALLOC(wlc->osh, WLC_ULB_INFO_SIZE))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	bzero((char *)ulb_info, WLC_ULB_INFO_SIZE);
	ulb_info->wlc = wlc;
	ulb_info->pub = wlc->pub;
	ulb_info->osh = wlc->osh;
	ulb_info->ulb_mode = ULB_MODE_DISABLED;


	/* register module */
	if (wlc_module_register(wlc->pub, ulb_iovars, "ulb",
		ulb_info, wlc_ulb_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: ULB module register failed\n",
			wlc->pub->unit));
		goto fail;
	}

	/* reserve cubby in the bsscfg container for private data */
	if ((ulb_info->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_ulb_info_t),
			wlc_ulb_cfg_cubby_init, wlc_ulb_cfg_cubby_deinit, NULL,
			(void *)ulb_info)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* bsscfg up/down callback */
	if (wlc_bsscfg_updown_register(wlc, wlc_ulb_bsscfg_updn, ulb_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/*
	* Update HW ULB capability based on Chip-ID and Revision.
	*/
	if (wlc_update_hwcap_ulb(ulb_info) == FALSE) {
		WL_ERROR(("wl%d: HW not ULB capable. ULB will be disabled on this WLC\n",
			wlc->pub->unit));
	}
	/* register IE mgmt callbacks */
	/* calc/build */
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, BRCM_ULB_IE_BUILD_FST_BMP,
		WLC_IEM_VS_IE_PRIO_ULB, wlc_ulb_calc_brcm_ulb_ie_len, wlc_ulb_write_brcm_ulb_ie,
		ulb_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, brcm ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* Parse Call back registering */
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, BRCM_ULB_IE_SCAN_PARSE_FST_BMP,
		WLC_IEM_VS_IE_PRIO_ULB, wlc_ulb_scan_parse_brcm_ulb_ie, ulb_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vsadd_parse_fn failed, brcm ie in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, BRCM_ULB_IE_PARSE_FST_BMP,
		WLC_IEM_VS_IE_PRIO_ULB, wlc_ulb_parse_brcm_ulb_ie, ulb_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vsadd_parse_fn failed, brcm ie in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	return ulb_info;

fail:
	wlc_ulb_detach(ulb_info);
	return NULL;
}

void
BCMATTACHFN(wlc_ulb_detach)(wlc_ulb_info_t *ulb_info)
{

	if (!ulb_info)
		return;

	si_pmu_set_ulbmode(ulb_info->wlc->pub->sih, ulb_info->osh, PMU_ULB_BW_NONE);

	wlc_module_unregister(ulb_info->pub, "ulb", ulb_info);

	MFREE(ulb_info->osh, ulb_info, WLC_ULB_INFO_SIZE);
}

static int
wlc_ulb_cfg_cubby_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_ulb_info_t *ulb_info = (wlc_ulb_info_t *)ctx;
	bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, cfg);

	/* In ULB Mode, min_bw is used to store the BW to be used for scan/join operations
	 * for this BSS. It is by default set to 20MHz
	 */
	if (wlc_ulb_set_bss_min_bw(ulb_info, cfg, WL_CHANSPEC_BW_20, FALSE)) {
		WL_ERROR(("wl%d: %s: wlc_ulb_set_bss_min_bw() failed.\n",
			ulb_info->wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	BSS_ULB_SET_MODE_SW_INPROG(bss_ulb, FALSE);
	BSS_ULB_SET_MODE_SW_TOKEN(bss_ulb, 0);
	BSS_ULB_SET_MODE_SW_AF_RTCNT(bss_ulb, 0);

	return BCME_OK;
exit:
	wlc_ulb_cfg_cubby_deinit(ctx, cfg);
	return BCME_ERROR;
}

static void
wlc_ulb_cfg_cubby_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	/* Do Nothing */
}

static void
wlc_ulb_bsscfg_updn(void *ctx, bsscfg_up_down_event_data_t *evt)
{
	wlc_ulb_info_t *ulb_info = (wlc_ulb_info_t *)ctx;
	wlc_info_t *wlc = ulb_info->wlc;
	wlc_bsscfg_t *cfg = evt->bsscfg;

	if (IS_ULB_DYN_MODE_ENABLED(wlc)) {
		if (evt->up) {
			/* In Dynamic Mode, TBTT/pre-TBTT processing call-back routines are
			 * registered
			 */
			if (wlc_tbtt_ent_fn_add(wlc->tbtt, cfg, wlc_ulb_pretbtt_cb,
				wlc_ulb_tbtt_cb, wlc->ulb_info) != BCME_OK) {
				WL_ERROR(("wl%d: %s: wlc_tbtt_ent_fn_add() failed\n",
				  wlc->pub->unit, __FUNCTION__));
				ASSERT(0);
			}
		} else {
			/* TBTT and pre-TBTT call-back handlers are de-registered,
			 * if registered
			 */
			wlc_tbtt_ent_fn_del(wlc->tbtt, cfg, wlc_ulb_pretbtt_cb,
				wlc_ulb_tbtt_cb, ulb_info);
		}
	}
}

#ifdef WL11ULB_NIC
int
wlc_tbtt_ent_fn_del(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg,
	wlc_tbtt_ent_fn_t pre_fn, wlc_tbtt_ent_fn_t fn, void *arg)
{
	return BCME_OK;
}

int
wlc_tbtt_ent_fn_add(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg,
	wlc_tbtt_ent_fn_t pre_fn, wlc_tbtt_ent_fn_t fn, void *arg)
{
	return BCME_OK;
}
#endif


static int
wlc_ulb_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_ulb_info_t *ulb_info = (wlc_ulb_info_t *)hdl;
	wlc_info_t *wlc = ulb_info->wlc;
	int32 int_val = 0;
	uint32 *ret_uint_ptr;
	int err = 0;
	wlc_bsscfg_t *bsscfg;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(ulb_info->wlc, wlcif);
	ASSERT(bsscfg != NULL);
	if (plen >= (int)sizeof(int_val))
		memcpy(&int_val, p, sizeof(int_val));

	ret_uint_ptr = (uint32 *)a;

	switch (actionid) {
		case IOV_GVAL(IOV_ULB_MODE):
			*ret_uint_ptr = (uint32)ulb_info->ulb_mode;
			break;

		case IOV_SVAL(IOV_ULB_MODE):
			if (int_val >= MAX_SUPP_ULB_MODES) {
				WL_ERROR(("wl%d: ULB Mode %d OOR\n", ulb_info->pub->unit,
					int_val));
				err = BCME_BADARG;
				break;
			}

			/* Allow enabling of ULB Mode only if it is currently supported in HW */
			if ((int_val != ULB_MODE_DISABLED) &&
				(wlc->hw->hw_ulb_cap == ULB_CAP_BW_NONE)) {
				WL_ERROR(("wl%d: ULB Mode not supported on this WLC\n",
					ulb_info->pub->unit));
				err = BCME_UNSUPPORTED;
				break;
			}

			ulb_info->ulb_mode = int_val;

			/* If ULB Mode is being disabled then min_bw is changed to 20MHz and
			 * ULB ENAB is disabled in all corresponding ULB bsscfgs
			 */
			if (ULB_MODE_GET(ulb_info) == ULB_MODE_DISABLED) {
				int idx = 0;
				wlc_bsscfg_t *cfg = NULL;
				FOREACH_ULB_ENAB_BSS(wlc, idx, cfg) {
					if (wlc_ulb_set_bss_min_bw(wlc->ulb_info, cfg,
						WL_CHANSPEC_BW_20, TRUE)) {
						err = BCME_ERROR;
						break;
					}
				}
			}
			/* Since in Dynamic Mode we perform all scan/join operations in 20MHz,
			 * we reset min_bw in all bsscfgs and set the ULB ENAB flag in bsscfg
			 * to TRUE, indicating that bsscfg is enabled for ULB operations,
			 * if needed.
			 */
			else if (ULB_MODE_GET(ulb_info) == ULB_MODE_DYN_MODE) {
				int idx = 0;
				wlc_bsscfg_t *cfg = NULL;
				FOREACH_BSS(wlc, idx, cfg) {
					if (wlc_ulb_set_bss_min_bw(wlc->ulb_info, cfg,
						WL_CHANSPEC_BW_20, FALSE)) {
						err = BCME_ERROR;
						break;
					}
					BSSCFG_SET_ULB_ENAB(cfg);
				}
			}
			break;
		case IOV_GVAL(IOV_ULB_BW): {
			bss_ulb_info_t *bss_ulb = NULL;

			if ((bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, bsscfg)) == NULL) {
				WL_ERROR(("wl%d: ULB BSS Cubby NULL\n", ulb_info->pub->unit));
				err = BCME_NORESOURCE;
				break;
			}

			*ret_uint_ptr = conv_chspec_bw_2_ulb_bw(bss_ulb->min_bw);
			break;
		}
		case IOV_SVAL(IOV_ULB_BW): {
			bss_ulb_info_t *bss_ulb = NULL;

			if (int_val >= MAX_SUPP_ULB_BW) {
				WL_ERROR(("wl%d: Invalid ULB BW %d OOR\n", ulb_info->pub->unit,
					int_val));
				err = BCME_BADARG;
				break;
			}

			if (!WLC_ULB_MODE_ENABLED(wlc)) {
				WL_ERROR(("wl%d: ULB Mode Not Enabled\n", ulb_info->pub->unit));
				err = BCME_NOTENABLED;
				break;
			}

			if (ULB_MODE_GET(ulb_info) == ULB_MODE_DYN_MODE) {
				WL_ERROR(("wl%d: Cannot force ULB BW in Dynamic Mode\n",
					ulb_info->pub->unit));
				err = BCME_ERROR;
				break;
			}

			if ((bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, bsscfg)) == NULL) {
				WL_ERROR(("wl%d: ULB BSS Cubby NULL\n", ulb_info->pub->unit));
				err = BCME_NORESOURCE;
				break;
			}

			/* Don't allow ULB BW settings in associated state;
			 * be it with any ULB or non-ULB BSS
			 */
			if (bsscfg->associated) {
				WL_ERROR(("wl%d: Not allowed in associated state\n",
					ulb_info->pub->unit));
				err = BCME_ASSOCIATED;
				break;
			}

			switch (int_val) {
				case ULB_BW_2P5MHZ:
					if (WLC_2P5MHZ_ULB_SUPP_HW(wlc))
						bss_ulb->min_bw = WL_CHANSPEC_BW_2P5;
					else
						err = BCME_UNSUPPORTED;
					break;
				case ULB_BW_5MHZ:
					if (WLC_5MHZ_ULB_SUPP_HW(wlc))
						bss_ulb->min_bw = WL_CHANSPEC_BW_5;
					else
						err = BCME_UNSUPPORTED;
					break;
				case ULB_BW_10MHZ:
					if (WLC_10MHZ_ULB_SUPP_HW(wlc))
						bss_ulb->min_bw = WL_CHANSPEC_BW_10;
					else
						err = BCME_UNSUPPORTED;
					break;
				default:
					bss_ulb->min_bw = WL_CHANSPEC_BW_20;
					break;
			}

			if (err == BCME_OK) {
				if (int_val != ULB_BW_DISABLED)
					BSSCFG_SET_ULB_ENAB(bsscfg);
				else
					BSSCFG_RESET_ULB_ENAB(bsscfg);
			}
			break;
		}
		case IOV_SVAL(IOV_ULB_MODE_SW_REQ): {
			ulb_opr_field_t opr_field;
			bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, bsscfg);

			if ((bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, bsscfg)) == NULL) {
				WL_ERROR(("wl%d: ULB BSS Cubby NULL\n", ulb_info->pub->unit));
				err = BCME_ERROR;
				break;
			}

			if (BSS_ULB_GET_MODE_SW_INPROG(bss_ulb)) {
				WL_ERROR(("wl%d: Multiple Request for ULB Mode-SW not supported\n",
					ulb_info->pub->unit));
				err = BCME_ERROR;
				break;
			}

			opr_field.cur_opr_bw = (int_val & 0xFF);
			opr_field.pri_opr_bw = ((int_val >> 8) & 0xFF);
			opr_field.pri_ch_num = ((int_val >> 16) & 0xFF);

			if ((opr_field.cur_opr_bw >= MAX_SUPP_ULB_BW) ||
			    (opr_field.pri_opr_bw >= MAX_SUPP_ULB_BW)) {
				WL_ERROR(("wl%d: Invalid BW Values %d, %d\n",
					ulb_info->pub->unit, opr_field.cur_opr_bw,
					opr_field.pri_opr_bw));
				err = BCME_ERROR;
				break;
			}

			if (opr_field.pri_ch_num >= MAXCHANNEL) {
				WL_ERROR(("wl%d: Invalid Channel Value %d\n",
					ulb_info->pub->unit, opr_field.pri_ch_num));
				err = BCME_ERROR;
				break;
			}

			BSS_ULB_SET_MODE_SW_BW(bss_ulb, opr_field.cur_opr_bw);
			BSS_ULB_SET_MODE_SW_CHNUM(bss_ulb, opr_field.pri_ch_num);
			BSS_ULB_SET_MODE_SW_AF_RTCNT(bss_ulb, 0);
			if (++BSS_ULB_GET_MODE_SW_TOKEN(bss_ulb) == 0)
				BSS_ULB_SET_MODE_SW_TOKEN(bss_ulb, 1);

			if (!wlc_send_ulb_mode_sw_req(wlc, bsscfg, &bsscfg->BSSID,
				BSS_ULB_GET_MODE_SW_TOKEN(bss_ulb), &opr_field, TRUE)) {
				WL_ERROR(("wl%d: wlc_send_ulb_mode_sw_req() failed\n",
					ulb_info->pub->unit));
				err = BCME_ERROR;
				break;
			}
			break;
		}
		default:
			err = BCME_UNSUPPORTED;
			break;
	}
	return err;
}

bool
wlc_ulb_mode_enabled(wlc_ulb_info_t *ulb_info)
{
	return (ULB_MODE_GET(ulb_info) != ULB_MODE_DISABLED);
}

bool
wlc_ulb_dyn_mode_enabled(wlc_ulb_info_t *ulb_info)
{
	return (ULB_MODE_GET(ulb_info) == ULB_MODE_DYN_MODE);
}

bool
wlc_ulb_bw_supp_hw(wlc_ulb_info_t *ulb_info, uint16 ulb_bw)
{
	wlc_info_t *wlc = ulb_info->wlc;

	if ((wlc_ulb_mode_enabled(ulb_info) == TRUE) &&
		(ulb_bw & wlc->hw->hw_ulb_cap))
		return TRUE;

	return FALSE;
}

bool
wlc_ulb_bw_supp_band(wlc_ulb_info_t *ulb_info, uint bandunit, uint16 ulb_bw)
{
	wlc_info_t *wlc = ulb_info->wlc;

	if (wlc_ulb_mode_enabled(ulb_info) && wlc_ulb_bw_supp_hw(ulb_info, ulb_bw)) {
		switch (ulb_bw) {
			case ULB_CAP_BW_2P5MHZ:
				return WL_BW_CAP_2P5MHZ(wlc->bandstate[bandunit]->bw_cap);
				break;
			case ULB_CAP_BW_5MHZ:
				return WL_BW_CAP_5MHZ(wlc->bandstate[bandunit]->bw_cap);
				break;
			case ULB_CAP_BW_10MHZ:
				return WL_BW_CAP_10MHZ(wlc->bandstate[bandunit]->bw_cap);
				break;
			default:
				return FALSE;
				break;
		}
	}

	return FALSE;
}

bool
wlc_update_hwcap_ulb(wlc_ulb_info_t *ulb_info)
{
	wlc_hw_info_t *wlc_hw = ulb_info->wlc->hw;
	/*
	* Currently ULB capability is detected based on PHY Rev-ID.
	* In future this should be done by reading some HW register.
	*/
	wlc_hw->hw_ulb_cap |= (WLC_2P5MHZ_CAP_PHY(ulb_info->wlc) ? ULB_CAP_BW_2P5MHZ :
		ULB_CAP_BW_NONE);
	wlc_hw->hw_ulb_cap |= (WLC_5MHZ_CAP_PHY(ulb_info->wlc) ? ULB_CAP_BW_5MHZ :
		ULB_CAP_BW_NONE);
	wlc_hw->hw_ulb_cap |= (WLC_10MHZ_CAP_PHY(ulb_info->wlc) ? ULB_CAP_BW_10MHZ :
		ULB_CAP_BW_NONE);

	if (wlc_hw->hw_ulb_cap == ULB_CAP_BW_NONE)
		return FALSE;
	else
		return TRUE;
}

/* Function returns the min_bw value configured for operation in specified BSS.
 * This in effect, gives the control channel bw for specified BSS. Value returned
 * by this function will always be less than or equal to 20MHz i.e 20/10/5/2.5 MHz.
 */
uint16
wlc_ulb_get_bss_min_bw(wlc_ulb_info_t *ulb_info, wlc_bsscfg_t *cfg)
{
	if (WLC_ULB_MODE_ENABLED(ulb_info->wlc)) {
		bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, cfg);

		if (bss_ulb == NULL) {
			WL_ERROR(("wl%d: ULB Cubby NULL %s\n", ulb_info->wlc->pub->unit,
				__FUNCTION__));
			return WL_CHANSPEC_BW_20;
		}

		return bss_ulb->min_bw;
	}

	return WL_CHANSPEC_BW_20;
}

/* Function is used to configure the min_bw value for operation in specified BSS.
 * This in effect, configures the control channel bw for specified BSS
 */
int
wlc_ulb_set_bss_min_bw(wlc_ulb_info_t *ulb_info, wlc_bsscfg_t *cfg, uint16 bw, bool update_enab)
{
	bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, cfg);

	if (bss_ulb == NULL) {
		WL_ERROR(("wl%d: ULB Cubby NULL %s\n",
			ulb_info->wlc->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	/* BW passed to this function must be <= 20MHz, if not then this is adjusted */
	if (!BW_LE20(bw))
		bw = WL_CHANSPEC_BW_20;

	/* If specified input BW is not supported by HW then BCME_ERROR is returned */
	switch (bw) {
		case WL_CHANSPEC_BW_2P5:
			if (!WLC_2P5MHZ_ULB_SUPP_HW(ulb_info->wlc))
				return BCME_ERROR;
			break;
		case WL_CHANSPEC_BW_5:
			if (!WLC_5MHZ_ULB_SUPP_HW(ulb_info->wlc))
				return BCME_ERROR;
			break;
		case WL_CHANSPEC_BW_10:
			if (!WLC_10MHZ_ULB_SUPP_HW(ulb_info->wlc))
				return BCME_ERROR;
			break;
		default:
			/* Nothing to do */
			break;
	}

	bss_ulb->min_bw = bw;

	/* There are scenarios where we reset min_bw to 20MHz mode but would still want to keep the
	 * cfg as ULB Enabled. This is being controled via update_enab
	 */
	if (update_enab) {
		if (bss_ulb->min_bw != WL_CHANSPEC_BW_20)
			BSSCFG_SET_ULB_ENAB(cfg);
		else
			BSSCFG_RESET_ULB_ENAB(cfg);
	}

	return BCME_OK;
}

static uint
conv_chspec_bw_2_ulb_bw(uint16 chspec_bw)
{
	uint retval = 0;
	switch (chspec_bw) {
		case WL_CHANSPEC_BW_2P5:
			retval = (uint)ULB_BW_2P5MHZ;
			break;
		case WL_CHANSPEC_BW_5:
			retval = (uint)ULB_BW_5MHZ;
			break;
		case WL_CHANSPEC_BW_10:
			retval = (uint)ULB_BW_10MHZ;
			break;
		default:
			retval = (uint32)ULB_BW_DISABLED;
			break;
	}

	return retval;
}

static uint
conv_ulb_bw_2_chspec_bw(uint ulb_bw)
{
	uint16 retval = 0;

	switch (ulb_bw) {
		case ULB_BW_2P5MHZ:
			retval = WL_CHANSPEC_BW_2P5;
			break;
		case ULB_BW_5MHZ:
			retval = WL_CHANSPEC_BW_5;
			break;
		case ULB_BW_10MHZ:
			retval = WL_CHANSPEC_BW_10;
			break;
		default:
			retval = WL_CHANSPEC_BW_20;
			break;
	}

	return retval;
}

static uint
wlc_ulb_calc_brcm_ulb_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ulb_info_t *ulb_info = (wlc_ulb_info_t *)ctx;
	bss_ulb_info_t *bss_ulb = NULL;
	bool b_cap_attr = FALSE;
	bool b_opr_attr = FALSE;
	bool b_mode_sw_attr = FALSE;
	uint ie_len = 0;

	if ((data->cfg == NULL) || (!BSSCFG_ULB_ENAB(ulb_info->wlc, data->cfg)) ||
		!wlc_ulb_dyn_mode_enabled(ulb_info))
		return ie_len;

	bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, data->cfg);

	switch (data->ft) {
		case FC_BEACON:
		case FC_PROBE_RESP:
			/* ULB IE with Capability & Operational Attribute is always included in
			 * Beacon/Probe-Response
			 */
			b_cap_attr = TRUE;
			b_opr_attr = TRUE;

			/* Mode-SW Attribute is included only if Mode-SW is currently in Progress */
			if (BSS_ULB_GET_MODE_SW_INPROG(bss_ulb))
				b_mode_sw_attr = TRUE;
			break;
		case FC_ASSOC_RESP:
		case FC_REASSOC_RESP:
			/* If peer is ULB capable then ULB IE is included with Capability and
			 * Operational Attribute
			 */
			if (IS_SCB_ULB_CAP(data->cbparm->ft->assocresp.scb)) {
				b_cap_attr = TRUE;
				b_opr_attr = TRUE;
			}
			break;
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ:
			/* If peer is ULB capable then ULB IE is included with Capability and
			 * Operational Attribute
			 */
			if (IS_BSS_ULB_CAP(data->cbparm->ft->assocreq.target)) {
				b_cap_attr = TRUE;
				b_opr_attr = TRUE;
			}
			break;
		case FC_PROBE_REQ:
			/* ULB IE with Capability Attribute is always included in Probe-Req */
			b_cap_attr = TRUE;
			break;
	}

	if (b_cap_attr || b_opr_attr || b_mode_sw_attr)
		ie_len = TLV_HDR_LEN + ULB_BRCM_PROP_IE_OUI_OVRHD;

	if (b_cap_attr)
		ie_len += ULB_CAP_ATTR_LEN;

	if (b_opr_attr)
		ie_len += ULB_OPR_ATTR_LEN;

	if (b_mode_sw_attr)
		ie_len += ULB_MODE_SW_ATTR_LEN;

	return ie_len;
}

static int
wlc_ulb_write_brcm_ulb_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ulb_info_t *ulb_info = (wlc_ulb_info_t *)ctx;
	wlc_info_t     *wlc = ulb_info->wlc;
	bss_ulb_info_t *bss_ulb = NULL;
	bool b_cap_attr = FALSE;
	bool b_opr_attr = FALSE;
	bool b_mode_sw_attr = FALSE;
	chanspec_t cur_chspec = 0;

	if ((data->cfg == NULL) || (!BSSCFG_ULB_ENAB(wlc, data->cfg)) ||
		!wlc_ulb_dyn_mode_enabled(ulb_info))
		return BCME_OK;

	if (BSSCFG_AP(data->cfg) || data->cfg->associated)
		cur_chspec = data->cfg->current_bss->chanspec;
	else
		cur_chspec = wlc->chanspec;

	bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, data->cfg);

	switch (data->ft) {
		case FC_BEACON:
		case FC_PROBE_RESP:
			/* ULB IE with Capability & Operational Attribute is always included in
			 * Beacon/Probe-Response
			 */
			b_cap_attr = TRUE;
			b_opr_attr = TRUE;

			/* Mode-SW Attribute is included only if Mode-SW is currently in Progress */
			if (BSS_ULB_GET_MODE_SW_INPROG(bss_ulb))
				b_mode_sw_attr = TRUE;
			break;
		case FC_ASSOC_RESP:
		case FC_REASSOC_RESP:
			/* If peer is ULB capable then ULB IE is included with Capability and
			 * Operational Attribute
			 */
			if (IS_SCB_ULB_CAP(data->cbparm->ft->assocresp.scb)) {
				b_cap_attr = TRUE;
				b_opr_attr = TRUE;
			}
			break;
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ:
			/* If peer is ULB capable then ULB IE is included with Capability and
			 * Operational Attribute
			 */
			if (IS_BSS_ULB_CAP(data->cbparm->ft->assocreq.target)) {
				b_cap_attr = TRUE;
				b_opr_attr = TRUE;
			}
			break;
		case FC_PROBE_REQ:
			/* ULB IE with Capability Attribute is always included in Probe-Req */
			b_cap_attr = TRUE;
			break;
	}

	if (b_cap_attr || b_opr_attr || b_mode_sw_attr) {
		ulb_prop_ie_t ulb_ie;
		uint8 *buf = data->buf;
		uint buf_len = data->buf_len;
		uint8 *ie_len_ptr = (data->buf + TLV_LEN_OFF);
		uint8 ie_len = ULB_BRCM_PROP_IE_OUI_OVRHD;

		/* First BRCM-Vendor IE, Length, OUI and OUI-Type are written. Note that length
		 * of full IE is not known write now, hence will be written at the end
		 */
		ulb_ie.id = DOT11_MNG_VS_ID;
		memcpy(&ulb_ie.oui[0], BRCM_PROP_OUI, DOT11_OUI_LEN);
		ulb_ie.type = ULB_BRCM_PROP_IE_TYPE;
		ulb_ie.len = ULB_BRCM_PROP_IE_OUI_OVRHD;
		buf = bcm_copy_tlv_safe(&ulb_ie, buf, buf_len);
		buf_len -= ULB_BRCM_PROP_IE_OUI_OVRHD;

		/* Build and Copy ULB Capability Attribute, if needed */
		if (b_cap_attr) {
			ulb_cap_attr_t cap_attr;

			wlc_ulb_build_cap_attr(wlc, CHSPEC_BANDUNIT(cur_chspec), &cap_attr);
			buf = bcm_copy_tlv_safe(&cap_attr, buf, buf_len);
			buf_len -= ULB_CAP_ATTR_LEN;
			ie_len += ULB_CAP_ATTR_LEN;
		}

		/* Build and Copy ULB Operations Attribute, if needed */
		if (b_opr_attr) {
			ulb_opr_attr_t opr_attr;

			wlc_ulb_build_opr_attr(wlc, data->cfg, cur_chspec, &opr_attr);
			buf = bcm_copy_tlv_safe(&opr_attr, buf, buf_len);
			buf_len -= ULB_OPR_ATTR_LEN;
			ie_len += ULB_OPR_ATTR_LEN;
		}

		/* Build and Copy ULB Mode SW Attribute, if needed */
		if (b_mode_sw_attr) {
			ulb_mode_sw_attr_t mode_sw_attr;

			wlc_ulb_build_mode_sw_attr(wlc, bss_ulb,
				CHSPEC_CHANNEL(wf_chspec_ctlchspec(cur_chspec)), &mode_sw_attr);
			buf = bcm_copy_tlv_safe(&mode_sw_attr, buf, buf_len);
			buf_len -= ULB_MODE_SW_ATTR_LEN;
			ie_len += ULB_MODE_SW_ATTR_LEN;
		}

		/* IE length is written IE-Length field of BRCM Vendor IE */
		*ie_len_ptr = ie_len;
	}

	return BCME_OK;
}

static int
wlc_ulb_scan_parse_brcm_ulb_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ulb_info_t *ulb_info = (wlc_ulb_info_t *)ctx;
	wlc_info_t     *wlc = ulb_info->wlc;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	ulb_prop_ie_t *ulb_ie = NULL;

	if ((data->ie == NULL) || (data->cfg == NULL) || !BSSCFG_ULB_ENAB(wlc, data->cfg) ||
		!wlc_ulb_dyn_mode_enabled(ulb_info))
		return BCME_OK;

	/* If ULB IE is present then we parse for Capability Attribute and update bssinfo in
	 * scan results.
	 */
	if ((ulb_ie = wlc_ulb_read_brcm_ulb_ie(wlc, data->ie, data->ie_len)) != NULL) {
		wlc_ulb_scan_parse_cap_attr(wlc, ulb_ie, ftpparm);
		wlc_ulb_scan_parse_opr_attr(wlc, ulb_ie, ftpparm);
	}

	return BCME_OK;
}

static int
wlc_ulb_parse_brcm_ulb_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ulb_info_t *ulb_info = (wlc_ulb_info_t *)ctx;
	wlc_info_t     *wlc = ulb_info->wlc;
	bss_ulb_info_t *bss_ulb = NULL;
	ulb_prop_ie_t *ulb_ie = NULL;
	ulb_cap_attr_t *cap_attr = NULL;
	ulb_opr_attr_t *opr_attr = NULL;
	ulb_mode_sw_attr_t *mode_sw_attr = NULL;
	chanspec_t cur_chspec = 0;

	if ((data->cfg == NULL) || (!BSSCFG_ULB_ENAB(wlc, data->cfg)) ||
		!wlc_ulb_dyn_mode_enabled(ulb_info))
		return BCME_OK;

	bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, data->cfg);

	if (BSSCFG_AP(data->cfg) || data->cfg->associated)
		cur_chspec = data->cfg->current_bss->chanspec;
	else
		cur_chspec = wlc->chanspec;

	if ((ulb_ie = wlc_ulb_read_brcm_ulb_ie(wlc, data->ie, data->ie_len)) != NULL) {
		if ((cap_attr = wlc_ulb_read_cap_attr(wlc, ulb_ie->data, ulb_ie->len)) != NULL) {
			if ((opr_attr = wlc_ulb_read_opr_attr(wlc, ulb_ie->data, ulb_ie->len))
				!= NULL) {
				mode_sw_attr = wlc_ulb_read_mode_sw_attr(wlc, ulb_ie->data,
						ulb_ie->len);
			}
		}
	}

	/* If ULB BRCM-Properitary IE or Capability Attribute or Operations Attribute are not
	 * found then there is no point in processing further.
	 */
	/* ULB_TBD: Will this function be called if ULB IE is not present? */
	if ((ulb_ie == NULL) || (cap_attr == NULL) || (opr_attr == NULL))
		return BCME_OK;

	switch (data->ft) {
		case FC_BEACON:
		case FC_PROBE_RESP:
			/* No Beacon and Probe-Response processing is done during scan. All the
			 * necessary ULB specific Scan processing is done in
			 * wlc_ulb_scan_parse_brcm_ulb_ie()
			 */
			if (SCAN_IN_PROGRESS(wlc->scan))
				break;

			if (!wlc_ulb_process_opr_attr(ulb_info, data, cur_chspec, opr_attr)) {
				WL_ERROR(("wl%d: %s: Processing of Opr-Attr Failed, Ft=0x%.2x\n",
					wlc->pub->unit, __FUNCTION__, data->ft));
			}


			if (mode_sw_attr)
				wlc_ulb_process_mode_sw_attr(ulb_info, data->cfg, mode_sw_attr);
			break;
		case FC_ASSOC_RESP:
		case FC_REASSOC_RESP:
			/* ULB capability is updated in SCB from Capability Attribute */
			wlc_ulb_process_cap_attr(data->pparm->ft->assocresp.scb, cap_attr);

			/* Statuc Code in (Re)Assoc-Response processing is changed to
			 * DOT11_SC_FAILURE for following conditions:
			 * - If Primary Channel Number specified is not valid or does not matches
			 *   with bssinfo chanspec or
			 * - Mismatch in primary and operational ULB BW (this should always be
			 *   same) or
			 * - If specified ULB operating BW is not supported
			 */
			 /* ULB_TBD: Will setting failure here help in getting disconnected??? */
			if (!wlc_ulb_process_opr_attr(ulb_info, data, cur_chspec, opr_attr)) {
				data->pparm->ft->assocresp.status = DOT11_SC_FAILURE;
				break;
			}
			break;
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ:
			/* ULB capability is updated in SCB from Capability Attribute */
			wlc_ulb_process_cap_attr(data->pparm->ft->assocreq.scb, cap_attr);

			/* Statuc Code in (Re)Assoc-Request processing is changed to
			 * DOT11_SC_FAILURE for following conditions:
			 * - If Primary Channel Number specified is not valid or does not matches
			 *   with bssinfo chanspec or
			 * - Mismatch in primary and operational ULB BW (this should always be
			 *   same) or
			 * - If specified ULB operating BW is not supported or
			 * - If specified ULB operating BW is not same as current ULB BW
			 */
			if (!wlc_ulb_validate_opr_field(ulb_info, cur_chspec,
				&opr_attr->ulb_opr_field) ||
				(conv_chspec_bw_2_ulb_bw(bss_ulb->min_bw) !=
				opr_attr->ulb_opr_field.cur_opr_bw)) {
				data->pparm->ft->assocreq.status = DOT11_SC_FAILURE;
				break;
			}
			break;
		default:
			break;
	}


	return BCME_OK;
}

static ulb_prop_ie_t *
wlc_ulb_read_brcm_ulb_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len)
{
	ulb_prop_ie_t *prop_ie;
	uint8 type = ULB_BRCM_PROP_IE_TYPE;

	prop_ie = (ulb_prop_ie_t *)bcm_find_vendor_ie(tlvs, tlvs_len,
		BRCM_PROP_OUI, &type, sizeof(type));

	if (prop_ie) {
		if (prop_ie->len >= (MIN_ULB_BRCM_PROP_IE_LEN))
			return prop_ie;
		else
			WL_ERROR(("wl%d: %s: len %d too short\n",
			          wlc->pub->unit, __FUNCTION__, prop_ie->len));
	}

	return NULL;
}

static ulb_cap_attr_t *
wlc_ulb_read_cap_attr(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len)
{
	ulb_cap_attr_t *cap_attr;
	uint8 attr_id = ULB_CAP_ATTR_ID;

	cap_attr = (ulb_cap_attr_t *)bcm_parse_tlvs(tlvs, tlvs_len, attr_id);

	if (cap_attr) {
		if (cap_attr->len >= (ULB_CAP_ATTR_LEN - TLV_HDR_LEN))
			return cap_attr;
		else
			WL_ERROR(("wl%d: %s: len %d too short\n",
			          wlc->pub->unit, __FUNCTION__, cap_attr->len));
	}

	return NULL;
}

static ulb_opr_attr_t *
wlc_ulb_read_opr_attr(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len)
{
	ulb_opr_attr_t *opr_attr;
	uint8 attr_id = ULB_OPR_ATTR_ID;

	opr_attr = (ulb_opr_attr_t *)bcm_parse_tlvs(tlvs, tlvs_len, attr_id);

	if (opr_attr) {
		if (opr_attr->len >= (ULB_CAP_ATTR_LEN - TLV_HDR_LEN))
			return opr_attr;
		else
			WL_ERROR(("wl%d: %s: len %d too short\n",
			          wlc->pub->unit, __FUNCTION__, opr_attr->len));
	}

	return NULL;
}

static ulb_mode_sw_attr_t *
wlc_ulb_read_mode_sw_attr(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len)
{
	ulb_mode_sw_attr_t *mode_sw_attr;
	uint8 attr_id = ULB_MODE_SW_ATTR_ID;

	mode_sw_attr = (ulb_mode_sw_attr_t *)bcm_parse_tlvs(tlvs, tlvs_len, attr_id);

	if (mode_sw_attr) {
		if (mode_sw_attr->len >= (ULB_CAP_ATTR_LEN - TLV_HDR_LEN))
			return mode_sw_attr;
		else
			WL_ERROR(("wl%d: %s: len %d too short\n",
			          wlc->pub->unit, __FUNCTION__, mode_sw_attr->len));
	}

	return NULL;
}

static void
wlc_ulb_build_cap_attr(wlc_info_t *wlc, uint bandunit, ulb_cap_attr_t *cap_attr)
{
	cap_attr->id = ULB_CAP_ATTR_ID;
	cap_attr->len = ULB_CAP_ATTR_LEN - TLV_HDR_LEN;

	/* Standalone Capabaility is configured based on HW supported + configuration from
	 * bw_cap IOVAR
	 */
	cap_attr->ulb_stdaln_cap = ULB_CAP_BW_NONE;
	cap_attr->ulb_stdaln_cap |= (WLC_2P5MHZ_ULB_SUPP_BAND(wlc, bandunit) ?
		ULB_CAP_BW_2P5MHZ : 0);
	cap_attr->ulb_stdaln_cap |= (WLC_5MHZ_ULB_SUPP_BAND(wlc, bandunit) ?
		ULB_CAP_BW_5MHZ : 0);
	cap_attr->ulb_stdaln_cap |= (WLC_10MHZ_ULB_SUPP_BAND(wlc, bandunit) ?
		ULB_CAP_BW_10MHZ : 0);

	/* Currently setting Dynamic Coex Capability to not Supported always */
	cap_attr->ulb_dyn_coex_cap = ULB_CAP_BW_NONE;

}

static void
wlc_ulb_build_opr_attr(wlc_info_t *wlc, wlc_bsscfg_t *cfg, chanspec_t cur_chspec,
	ulb_opr_attr_t *opr_attr)
{
	uint16 min_bw = wlc_ulb_get_bss_min_bw(wlc->ulb_info, cfg);

	opr_attr->id = ULB_OPR_ATTR_ID;
	opr_attr->len = ULB_OPR_ATTR_LEN - TLV_HDR_LEN;

	opr_attr->ulb_opr_field.cur_opr_bw = conv_chspec_bw_2_ulb_bw(min_bw);
	opr_attr->ulb_opr_field.pri_opr_bw = conv_chspec_bw_2_ulb_bw(min_bw);
	opr_attr->ulb_opr_field.pri_ch_num = CHSPEC_CHANNEL(wf_chspec_ctlchspec(cur_chspec));
}

static void
wlc_ulb_build_mode_sw_attr(wlc_info_t *wlc, bss_ulb_info_t *bss_ulb, uint8 ch_num,
	ulb_mode_sw_attr_t *mode_sw_attr)
{
	mode_sw_attr->id = ULB_MODE_SW_ATTR_ID;
	mode_sw_attr->len = ULB_MODE_SW_ATTR_LEN - TLV_HDR_LEN;

	mode_sw_attr->ulb_opr_field.cur_opr_bw = BSS_ULB_GET_MODE_SW_BW(bss_ulb);
	mode_sw_attr->ulb_opr_field.pri_opr_bw = BSS_ULB_GET_MODE_SW_BW(bss_ulb);
	mode_sw_attr->ulb_opr_field.pri_ch_num = ch_num;
	mode_sw_attr->cnt = BSS_ULB_GET_MODE_SW_CNT(bss_ulb);
}

static void
wlc_ulb_scan_parse_cap_attr(wlc_info_t *wlc, ulb_prop_ie_t *ulb_ie, wlc_iem_ft_pparm_t *ftpparm)
{
	wlc_bss_info_t *bi = ftpparm->scan.result;
	ulb_cap_attr_t *cap_attr = NULL;

	/* Parse for ULB Capability Attribute and update the Standalone Mode Capability in bssinfo.
	 * Note that Dynamic Co-ex field is currently not parsed/handled.
	 */
	if ((cap_attr = wlc_ulb_read_cap_attr(wlc, ulb_ie->data, ulb_ie->len)) != NULL) {
		bi->flags2 |= ((cap_attr->ulb_stdaln_cap & ULB_CAP_BW_10MHZ) ?
				WLC_BSS_ULB_10_CAP : 0);
		bi->flags2 |= ((cap_attr->ulb_stdaln_cap & ULB_CAP_BW_5MHZ) ?
				WLC_BSS_ULB_5_CAP : 0);
		bi->flags2 |= ((cap_attr->ulb_stdaln_cap & ULB_CAP_BW_2P5MHZ) ?
				WLC_BSS_ULB_2P5_CAP : 0);
	}
}

static void
wlc_ulb_scan_parse_opr_attr(wlc_info_t *wlc, ulb_prop_ie_t *ulb_ie, wlc_iem_ft_pparm_t *ftpparm)
{
	wlc_bss_info_t *bi = ftpparm->scan.result;
	ulb_opr_attr_t *opr_attr = NULL;

	/* Parse for ULB Operations Attribute and update the chanspec in bssinfo */
	if ((opr_attr = wlc_ulb_read_opr_attr(wlc, ulb_ie->data, ulb_ie->len)) != NULL) {
		if (opr_attr->ulb_opr_field.cur_opr_bw != ULB_BW_DISABLED) {
			uint16 bw = conv_ulb_bw_2_chspec_bw(opr_attr->ulb_opr_field.cur_opr_bw);

			bi->chanspec = CHBW_CHSPEC(bw, opr_attr->ulb_opr_field.pri_ch_num);
		}
	}
}

static void
wlc_ulb_process_cap_attr(scb_t *scb, ulb_cap_attr_t *cap_attr)
{
	scb->flags3 |= ((cap_attr->ulb_stdaln_cap & ULB_CAP_BW_10MHZ) ?
			SCB3_IS_10 : 0);
	scb->flags3 |= ((cap_attr->ulb_stdaln_cap & ULB_CAP_BW_5MHZ) ?
			SCB3_IS_5 : 0);
	scb->flags3 |= ((cap_attr->ulb_stdaln_cap & ULB_CAP_BW_2P5MHZ) ?
			SCB3_IS_2P5 : 0);
}

static bool
wlc_ulb_process_opr_attr(wlc_ulb_info_t *ulb_info, wlc_iem_parse_data_t *data,
	chanspec_t cur_chspec, ulb_opr_attr_t *opr_attr)
{
	uint16 peer_min_bw = conv_ulb_bw_2_chspec_bw(opr_attr->ulb_opr_field.cur_opr_bw);
	bool retval = FALSE;

	/* Handles processing of Operations Attribute received in Beacon/Probe-Response/
	 * (Re)Assoc-Response
	 */
	if ((data->ft != FC_BEACON) && (data->ft != FC_PROBE_RESP) &&
		(data->ft != FC_ASSOC_RESP) && (data->ft != FC_REASSOC_RESP))
		return retval;

	if ((retval = wlc_ulb_validate_opr_field(ulb_info, cur_chspec,
		&opr_attr->ulb_opr_field)) == FALSE)
		return retval;

	/* Should never be the case */
	if (peer_min_bw != wlc_ulb_get_bss_min_bw(ulb_info, data->cfg)) {
		WL_ERROR(("wl%d: %s: ULB Mode BW Sync Loss!! peer_bw=0x%x, cur_bw=0x%x\n",
			ulb_info->wlc->pub->unit, __FUNCTION__, peer_min_bw,
			wlc_ulb_get_bss_min_bw(ulb_info, data->cfg)));
		wlc_ulb_do_ulb_mode_sw(ulb_info, data->cfg, &opr_attr->ulb_opr_field, TRUE);
	}

	return retval;
}

static void
wlc_ulb_process_mode_sw_attr(wlc_ulb_info_t *ulb_info, wlc_bsscfg_t *cfg,
	ulb_mode_sw_attr_t *mode_sw_attr)
{
	bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, cfg);
	/* Mode-SW in-progress flag is updated if this is the first Mode-SW Attribute being
	 * processed.
	 */
	if (!BSS_ULB_GET_MODE_SW_INPROG(bss_ulb)) {
		BSS_ULB_SET_MODE_SW_INPROG(bss_ulb, TRUE);
		BSS_ULB_SET_MODE_SW_CNT(bss_ulb, mode_sw_attr->cnt);
		BSS_ULB_SET_MODE_SW_BW(bss_ulb, mode_sw_attr->ulb_opr_field.cur_opr_bw);
		BSS_ULB_SET_MODE_SW_CHNUM(bss_ulb, mode_sw_attr->ulb_opr_field.pri_ch_num);

		if (BSS_ULB_GET_MODE_SW_CNT(bss_ulb) == 0) {
			wlc_ulb_do_ulb_mode_sw(ulb_info, cfg, &mode_sw_attr->ulb_opr_field,
				TRUE);
		} else
			BSS_ULB_SET_MODE_SW_PROC_TBTT(bss_ulb, TRUE);

		/* ULB_TBD: Need to ensure that STA never sleeps */
	}
}

static bool
wlc_ulb_validate_opr_field(wlc_ulb_info_t *ulb_info, chanspec_t cur_chspec,
	ulb_opr_field_t *opr_field)
{
	uint8 pri_ch_num = opr_field->pri_ch_num;
	uint8 cur_opr_bw = opr_field->cur_opr_bw;
	uint8 pri_opr_bw = opr_field->pri_opr_bw;
	bool retval = TRUE;

	/* Validation is done against following conditions:
	 * - If Primary Channel Number specified is not valid or does not matches
	 *   with bssinfo chanspec or
	 * - Mismatch in primary and operational ULB BW (this should always be same) or
	 * - If specified ULB operating BW is not supported
	 */
	if ((pri_ch_num >= MAXCHANNEL) ||
		(pri_ch_num != wf_chspec_ctlchan(cur_chspec)) ||
		(cur_opr_bw != pri_opr_bw) ||
		((cur_opr_bw != ULB_CAP_BW_NONE) &&
		(wlc_ulb_bw_supp_band(ulb_info, CHANNEL_BANDUNIT(ulb_info->wlc, pri_ch_num),
			cur_opr_bw) == FALSE))) {
		retval = FALSE;
	}

	return retval;
}

/* wlc calls to receive the action frames */
int
wlc_ulb_recv_action_frames(wlc_ulb_info_t *ulb_info, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	wlc_info_t *wlc = ulb_info->wlc;
	dot11_action_vs_frmhdr_t *af = (dot11_action_vs_frmhdr_t *)body;

	ASSERT(body != NULL);
	ASSERT(BSSCFG_ULB_ENAB(wlc, cfg));
	BCM_REFERENCE(wlc);

	switch (af->subtype) {
	case BRCM_ULB_SWITCH_REQ_SUBTYPE:
		wlc_ulb_process_mode_sw_req_af(ulb_info, cfg, hdr, body, body_len);
		break;
	case BRCM_ULB_SWITCH_RSP_SUBTYPE:
		wlc_ulb_process_mode_sw_rsp_af(ulb_info, cfg, hdr, body, body_len);
		break;
	default:
		WL_ERROR(("wl%d: %s: subtype %d is not valid\n",
		        wlc->pub->unit, __FUNCTION__, af->subtype));
		break;
	}

	return BCME_OK;
}

static void
wlc_ulb_process_mode_sw_req_af(wlc_ulb_info_t* ulb_info, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	wlc_info_t *wlc = ulb_info->wlc;
	ulb_mode_sw_req_t *mode_sw_req = (ulb_mode_sw_req_t *)body;
	ulb_opr_field_t *opr_field = &mode_sw_req->opr_field;
	bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, cfg);
	struct scb *scb = NULL;
	uint16 rsn_code = 0;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	if (!BSSCFG_AP(cfg)) {
		rsn_code = 1;
		goto fail;
	}
	/* If ULB Mode SW is already in progress then frame is dropped */
	else if (BSS_ULB_GET_MODE_SW_INPROG(bss_ulb)) {
		rsn_code = 2;
		goto fail;
	} else if (!wlc_ulb_validate_opr_field(ulb_info, cfg->current_bss->chanspec, opr_field)) {
		rsn_code = 3;
		goto fail;
	}
	/* Transmitter of the action-frame must be ULB capable. Also all the associated STA must
	 * be ULB capable to initiate mode SW
	 */
	else if (((scb = wlc_scbfind(wlc, cfg, (struct ether_addr *) &hdr->sa)) == NULL) ||
		!IS_SCB_ULB_CAP(scb) ||
		(!wlc_ulb_bw_supp_all_scbs(ulb_info, cfg, opr_field->cur_opr_bw))) {
		rsn_code = 4;
		goto fail;
	}
	/* Further, mode-sw request is rejected if BW specified is same as current operating
	 * bandwidth
	 */
	else if (opr_field->cur_opr_bw == conv_chspec_bw_2_ulb_bw(bss_ulb->min_bw)) {
		rsn_code = 5;
		goto fail;
	}

	/* On passing sanity checks of received frame, Mode-SW is performed. Mode-SW Inprogress
	 * flag is set to TRUE, and all other parsed parameters are updated locally
	 */
	BSS_ULB_SET_MODE_SW_INPROG(bss_ulb, TRUE);
	BSS_ULB_SET_MODE_SW_BW(bss_ulb, opr_field->cur_opr_bw);
	BSS_ULB_SET_MODE_SW_CHNUM(bss_ulb, opr_field->pri_ch_num);
	BSS_ULB_SET_MODE_SW_TOKEN(bss_ulb, mode_sw_req->dia_token);
	BSS_ULB_SET_MODE_SW_AF_RTCNT(bss_ulb, 0);

	/* If there are more than one associated ULB STA then, we need to indicate the mode switch
	 * by including ULB Mode Switch Attribute in ULB-IE of Beacon/Probe-Response frames.
	 * Else mode switch is initiated immediately be sending a ULB Mode Switch Response Action
	 * frame
	 */
	if (wlc_ulb_get_num_ulb_scbs(ulb_info, cfg) > 1) {
		/* Mode-SW-Cnt gives number of TBTTs after which mode-sw will be performed.
		 * Here the value of count is set to (DEFAULT_MODE_SW_BEACON_CNT + 1) inorder
		 * to have uniform processing of the Cnt decrement in wlc_ulb_pretbtt_cb() for
		 * STA and AP case.
		 */
		BSS_ULB_SET_MODE_SW_CNT(bss_ulb, (DEFAULT_MODE_SW_BEACON_CNT + 1));
		if (BSS_ULB_GET_MODE_SW_CNT(bss_ulb) == 1)
			BSS_ULB_GET_MODE_SW_CNT(bss_ulb)++;
		/* Process-TBTT flag is set to true to handle decrementing of Mode-SW-Cnt and
		 * updating beacon and probe-response template
		 */
		BSS_ULB_SET_MODE_SW_PROC_TBTT(bss_ulb, TRUE);
	} else {
		BSS_ULB_SET_MODE_SW_CNT(bss_ulb, 0);
		BSS_ULB_SET_MODE_SW_PROC_TBTT(bss_ulb, FALSE);
		if (!wlc_send_ulb_mode_sw_rsp(wlc, cfg, &hdr->sa, mode_sw_req->dia_token,
			opr_field, ULB_MODE_SW_SC_SUCCESS, TRUE)) {
			BSS_ULB_SET_MODE_SW_INPROG(bss_ulb, FALSE);
			WL_ERROR(("wl%d: %s: wlc_send_ulb_mode_sw_rsp() with RC=%d failed\n",
				wlc->pub->unit, __FUNCTION__, ULB_MODE_SW_SC_SUCCESS));
		}
		/* Block all data transmissions, if not done already */
		if (!(wlc->block_datafifo & DATA_BLOCK_TX_SUPR))
			wlc_block_datafifo(wlc, DATA_BLOCK_TX_SUPR, DATA_BLOCK_TX_SUPR);
	}

	return;

fail:
	/* In all the failed cases, indicate failure be sending ModeSwitch Response frame with
	 * invalid status code
	 */
	if (!wlc_send_ulb_mode_sw_rsp(wlc, cfg, &hdr->sa, mode_sw_req->dia_token, opr_field,
		ULB_MODE_SW_SC_INVALID, FALSE)) {
		WL_ERROR(("wl%d: %s: wlc_send_ulb_mode_sw_rsp() with RC=%d failed\n",
			wlc->pub->unit, __FUNCTION__, ULB_MODE_SW_SC_INVALID));
	}

	BCM_REFERENCE(rsn_code);
	WL_ERROR(("wl%d: %s: Dropping Switch Req from STA:%s. RC=%d\n", wlc->pub->unit,
		__FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf), rsn_code));
}

static void
wlc_ulb_process_mode_sw_rsp_af(wlc_ulb_info_t* ulb_info, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	wlc_info_t *wlc = ulb_info->wlc;
	ulb_mode_sw_rsp_t *mode_sw_rsp = (ulb_mode_sw_rsp_t *)body;
	ulb_opr_field_t *opr_field = &mode_sw_rsp->opr_field;
	bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, cfg);
	uint16 rsn_code = 0;
	struct scb *scb = NULL;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	if (!BSSCFG_STA(cfg)) {
		rsn_code = 1;
		goto fail;
	} else if (mode_sw_rsp->dia_token != BSS_ULB_GET_MODE_SW_TOKEN(bss_ulb)) {
		rsn_code = 2;
		goto fail;
	} else if ((mode_sw_rsp->status != ULB_MODE_SW_SC_SUCCESS)) {
		rsn_code = 3;
		goto fail;
	} else if (((scb = wlc_scbfind(wlc, cfg, (struct ether_addr *) &hdr->sa)) == NULL) ||
		!IS_SCB_ULB_CAP(scb)) {
		rsn_code = 4;
		goto fail;
	} else if ((opr_field->cur_opr_bw == conv_chspec_bw_2_ulb_bw(bss_ulb->min_bw)) ||
		!wlc_ulb_is_ulb_bw_supp(scb, opr_field->cur_opr_bw)) {
		rsn_code = 5;
		goto fail;
	}

	/* On passing sanity checks of received frame, Mode-SW is performed. Mode-SW Inprogress
	 * flag is set to TRUE
	 */
	BSS_ULB_SET_MODE_SW_INPROG(bss_ulb, TRUE);
	wlc_ulb_do_ulb_mode_sw(ulb_info, cfg, opr_field, TRUE);
	return;
fail:
	BCM_REFERENCE(rsn_code);
	WL_ERROR(("wl%d: %s: Dropping Switch Rsp from AP:%s. Reason-Code=%d\n", wlc->pub->unit,
		__FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf), rsn_code));
	return;
}

static int
wlc_send_ulb_mode_sw_req(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	uint8 dia_token, ulb_opr_field_t *opr_field, bool reg_cb)
{
	void *p;
	uint8* pbody;
	uint body_len;
	ulb_mode_sw_req_t *mode_sw_req;
	bss_ulb_info_t *bss_ulb = NULL;
	int err = BCME_OK;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_INFORM */

	WL_INFORM(("wl%d: %s: sending Mode Switch Req (token %d) to %s\n",
		wlc->pub->unit, __FUNCTION__, dia_token, bcm_ether_ntoa(da, eabuf)));

	ASSERT(cfg != NULL);
	bss_ulb = ULB_BSSCFG_CUBBY(wlc->ulb_info, cfg);
	body_len = ULB_MODE_SW_REQ_AF_LEN;

	p = wlc_frame_get_mgmt(wlc, FC_ACTION, da, &cfg->cur_etheraddr, &cfg->BSSID,
	                       body_len, &pbody);
	if (p == NULL) {
		WL_ERROR(("wl%d: %s: no memory for Mode Switch Request\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	mode_sw_req = (ulb_mode_sw_req_t *)pbody;
	mode_sw_req->category = DOT11_ACTION_CAT_VS;
	mode_sw_req->type = BRCM_ULB_AF_TYPE;
	mode_sw_req->subtype = BRCM_ULB_SWITCH_REQ_SUBTYPE;
	mode_sw_req->dia_token = dia_token;

	memcpy(&mode_sw_req->OUI, BRCM_PROP_OUI, DOT11_OUI_LEN);
	memcpy(&mode_sw_req->opr_field, opr_field, sizeof(ulb_opr_field_t));

	if (reg_cb) {
		BSS_ULB_SET_MODE_SW_PKT(bss_ulb, p);
		err = wlc_pcb_fn_register(wlc->pcb,
			wlc_ulb_ulb_mode_sw_req_cb, (void*)cfg, p);
	}

	if (err == BCME_OK) {
		if (wlc_sendmgmt(wlc, p, cfg->wlcif->qi, NULL) == FALSE) {
			err = BCME_ERROR;
			WL_ERROR(("wl%d: %s: wlc_sendmgmt() failed\n",
				wlc->pub->unit, __FUNCTION__));
		}
	} else
		WL_ERROR(("wl%d: %s: Pkt-CB register failed\n",
			wlc->pub->unit, __FUNCTION__));

	return err;
}

static int
wlc_send_ulb_mode_sw_rsp(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	uint8 dia_token, ulb_opr_field_t *opr_field, uint8 status, bool reg_cb)
{
	void *p;
	uint8* pbody;
	uint body_len;
	ulb_mode_sw_rsp_t *mode_sw_rsp;
	bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(wlc->ulb_info, cfg);
	int err = BCME_OK;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_INFORM */

	WL_INFORM(("wl%d: %s: sending Mode Switch Rsp (token %d) to %s\n",
		wlc->pub->unit, __FUNCTION__, dia_token, bcm_ether_ntoa(da, eabuf)));

	ASSERT(cfg != NULL);
	body_len = ULB_MODE_SW_RSP_AF_LEN;

	p = wlc_frame_get_mgmt(wlc, FC_ACTION, da, &cfg->cur_etheraddr, &cfg->BSSID,
	                       body_len, &pbody);
	if (p == NULL) {
		WL_ERROR(("wl%d: %s: no memory for Mode Switch Response\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	mode_sw_rsp = (ulb_mode_sw_rsp_t *)pbody;
	mode_sw_rsp->category = DOT11_ACTION_CAT_VS;
	mode_sw_rsp->type = BRCM_ULB_AF_TYPE;
	mode_sw_rsp->subtype = BRCM_ULB_SWITCH_RSP_SUBTYPE;
	mode_sw_rsp->dia_token = dia_token;
	mode_sw_rsp->status = status;

	memcpy(&mode_sw_rsp->OUI, BRCM_PROP_OUI, DOT11_OUI_LEN);
	memcpy(&mode_sw_rsp->opr_field, opr_field, sizeof(ulb_opr_field_t));


	if (reg_cb) {
		BSS_ULB_SET_MODE_SW_PKT(bss_ulb, p);
		err = wlc_pcb_fn_register(wlc->pcb,
			wlc_ulb_ulb_mode_sw_rsp_cb, (void*)cfg, p);
	}

	if (err == BCME_OK) {
		if (wlc_sendmgmt(wlc, p, cfg->wlcif->qi, NULL) == FALSE) {
			err = BCME_ERROR;
			WL_ERROR(("wl%d: %s: wlc_sendmgmt() failed\n",
				wlc->pub->unit, __FUNCTION__));
		}
	} else
		WL_ERROR(("wl%d: %s: Pkt-CB register failed\n",
			wlc->pub->unit, __FUNCTION__));

	return err;
}

/* Function checks if the specified ulb_bw is supported by all the SCBs in specified bsscfg */
static bool
wlc_ulb_bw_supp_all_scbs(wlc_ulb_info_t* ulb_info, wlc_bsscfg_t *cfg, uint8 ulb_bw)
{
	struct scb_iter scbiter;
	scb_t *scb;
	wlc_info_t *wlc = ulb_info->wlc;
	bool retval = FALSE;

	if (ulb_bw > MAX_SUPP_ULB_BW)
		return retval;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		if (!IS_SCB_ULB_CAP(scb) || !wlc_ulb_is_ulb_bw_supp(scb, ulb_bw)) {
			retval = FALSE;
			break;
		} else
			retval = TRUE;
	}

	return retval;
}

/* Function returns number of SCBs that are ULB capable in specified bsscfg */
static int
wlc_ulb_get_num_ulb_scbs(wlc_ulb_info_t* ulb_info, wlc_bsscfg_t *cfg)
{
	struct scb_iter scbiter;
	scb_t *scb;
	wlc_info_t *wlc = ulb_info->wlc;
	int num_scbs = 0;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		if (IS_SCB_ULB_CAP(scb))
			num_scbs++;
	}

	return num_scbs;
}

/* Function checks if specified ulb_bw is supported in specified SCB */
static bool
wlc_ulb_is_ulb_bw_supp(scb_t* scb, uint8 ulb_bw)
{
	bool retval = FALSE;

	if ((ulb_bw > MAX_SUPP_ULB_BW) || !IS_SCB_ULB_CAP(scb)) {
		return retval;
	}

	switch (ulb_bw) {
		case ULB_BW_2P5MHZ:
			retval = (IS_SCB_ULB_2P5_CAP(scb) ? TRUE : FALSE);
			break;
		case ULB_BW_5MHZ:
			retval = (IS_SCB_ULB_5_CAP(scb) ? TRUE : FALSE);
			break;
		case ULB_BW_10MHZ:
			retval = (IS_SCB_ULB_10_CAP(scb) ? TRUE : FALSE);
			break;
		default:
			retval = TRUE;
			break;
	}

	return retval;
}

/* Function handles performing ULB Mode Switch to specified BW. */
static void
wlc_ulb_do_ulb_mode_sw(wlc_ulb_info_t* ulb_info, wlc_bsscfg_t *cfg, ulb_opr_field_t *opr_field,
	bool block_data)
{
	wlc_info_t *wlc = ulb_info->wlc;
	bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(wlc->ulb_info, cfg);
	uint16 new_chspec = CHBW_CHSPEC(conv_ulb_bw_2_chspec_bw(opr_field->cur_opr_bw),
		opr_field->pri_ch_num);

	if (!new_chspec || new_chspec == INVCHANSPEC) {
		return;
	}

#if defined(PROP_TXSTATUS) && defined(WLMCHAN)
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		wlc_wlfc_mchan_interface_state_update(wlc, cfg, WLFC_CTL_TYPE_INTERFACE_CLOSE,
			FALSE);
	}
#endif

	if (block_data)
		wlc_block_datafifo(wlc, DATA_BLOCK_TX_SUPR, DATA_BLOCK_TX_SUPR);

	if (wlc->chanspec != new_chspec) {
		wlc_bsscfg_set_current_bss_chan(cfg, new_chspec);
		wlc_ulb_set_bss_min_bw(ulb_info, cfg,
			conv_ulb_bw_2_chspec_bw(opr_field->cur_opr_bw),
			FALSE);
#ifdef WLMCHAN
		/* Update mchan context for new_chspec with current chanspec */
		if (MCHAN_ENAB(wlc->pub)) {
			wlc_mchan_update_bss_chan_context(wlc, cfg, new_chspec, FALSE);
		}
#endif /* WLMCHAN */
		wlc_set_home_chanspec(wlc, new_chspec);
		/* This is done to ensure that we detach the primary queue and perform a
		 * wlc_bmac_tx_fifo_sync()
		 */
		wlc_primary_queue_set(wlc, wlc->primary_queue);

		wlc_suspend_mac_and_wait(wlc);
#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(wlc->pub))
			wlc_wlfc_flush_pkts_to_host(wlc, cfg);
#endif /* PROP_TXSTATUS */
		wlc_set_chanspec(wlc, new_chspec);
		wlc_enable_mac(wlc);

		/* Updated the PHY chanspec, calibrate the channel */
		wlc_full_phy_cal(wlc, cfg, PHY_PERICAL_UP_BSS);
	}

	wlc_scb_ratesel_init_bss(wlc, cfg);

	if (block_data)
		wlc_block_datafifo(wlc, DATA_BLOCK_TX_SUPR, 0);

#if defined(PROP_TXSTATUS) && defined(WLMCHAN)
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		wlc_wlfc_mchan_interface_state_update(wlc, cfg,
			WLFC_CTL_TYPE_INTERFACE_OPEN, TRUE);
	}
#endif

	BSS_ULB_SET_MODE_SW_INPROG(bss_ulb, FALSE);
}

/* All mode sw count processing is done as part of pre-tbtt. The Concept of Mode-SW Cnt is
 * analogous to channel-sw-cnt in CSA-IE. Count value basicallly indicate number of TBTTs
 */
static void
wlc_ulb_pretbtt_cb(void *ctx, wlc_tbtt_ent_data_t *notif_data)
{
	wlc_ulb_info_t *ulb_info = (wlc_ulb_info_t *)ctx;
	wlc_info_t *wlc = ulb_info->wlc;
	wlc_bsscfg_t *cfg = notif_data->cfg;
	bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, cfg);

	/* Process-TBTT flag is set on following occassions:
	 * - When AP receives Mode-SW request AF and there are more than 1 associated ULB STA
	 * - When STA receives Mode-SW attribute in Beacon/Probe Response frames
	 */
	if (IS_ULB_DYN_MODE_ENABLED(wlc) && BSS_ULB_GET_MODE_SW_PROC_TBTT(bss_ulb)) {

		/* Block all data transmissions, if not done already */
		if (!(wlc->block_datafifo & DATA_BLOCK_TX_SUPR))
			wlc_block_datafifo(wlc, DATA_BLOCK_TX_SUPR, DATA_BLOCK_TX_SUPR);
		/* Mode-SW Count indicates number of TBTTs after which switch will be performed.
		 * Value of 1 indicates switch just before next TBTT and value of 0 indicate
		 * immediate switch. Here the value is decremented before any further processing
		 * in order handle case of STA and AP processing in a single place.
		 */
		if (--BSS_ULB_GET_MODE_SW_CNT(bss_ulb) == 0) {
			ulb_opr_field_t opr_field = {0};

			opr_field.cur_opr_bw = BSS_ULB_GET_MODE_SW_BW(bss_ulb);
			opr_field.pri_opr_bw = BSS_ULB_GET_MODE_SW_BW(bss_ulb);
			opr_field.pri_ch_num = BSS_ULB_GET_MODE_SW_CHNUM(bss_ulb);
			wlc_ulb_do_ulb_mode_sw(ulb_info, cfg, &opr_field, FALSE);

			if (wlc->block_datafifo & DATA_BLOCK_TX_SUPR)
				wlc_block_datafifo(wlc, DATA_BLOCK_TX_SUPR, 0);
		}

		if (BSSCFG_AP(cfg)) {
			wlc_bss_update_beacon(wlc, cfg);
			wlc_bss_update_probe_resp(wlc, cfg, TRUE);
		}

		/* Process-TBTT flag is reset if Mode-SW is not in Progress */
		if (!BSS_ULB_GET_MODE_SW_INPROG(bss_ulb)) {
			BSS_ULB_SET_MODE_SW_PROC_TBTT(bss_ulb, FALSE);
		}
	}
}

static void
wlc_ulb_tbtt_cb(void *ctx, wlc_tbtt_ent_data_t *notif_data)
{
	/* Nothing to be done for now */
}

static void
wlc_ulb_ulb_mode_sw_req_cb(wlc_info_t *wlc, uint tx_status, void* arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)arg;
	wlc_ulb_info_t *ulb_info = wlc->ulb_info;
	bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, cfg);

	if (tx_status & TX_STATUS_ACK_RCV) {
		BSS_ULB_SET_MODE_SW_AF_RTCNT(bss_ulb, 0);
	} else if ((tx_status & TX_STATUS_MASK) == TX_STATUS_NO_ACK) {
		if (++BSS_ULB_GET_MODE_SW_AF_RTCNT(bss_ulb) <= MAX_NUM_MODE_SW_AF_RETRY_CNT) {
			struct scb *scb = WLPKTTAGSCBGET(BSS_ULB_GET_MODE_SW_PKT(bss_ulb));
			ulb_opr_field_t opr_field = {0};

			/* make sure the scb still exists */
			if (scb == NULL) {
				WL_ERROR(("wl%d: %s: unable to find scb from the pkt %p\n",
					wlc->pub->unit, __FUNCTION__,
					BSS_ULB_GET_MODE_SW_PKT(bss_ulb)));
				wlc_ulb_abort_mode_sw(bss_ulb);
				return;
			}
			opr_field.cur_opr_bw = BSS_ULB_GET_MODE_SW_BW(bss_ulb);
			opr_field.pri_opr_bw = BSS_ULB_GET_MODE_SW_BW(bss_ulb);
			opr_field.pri_ch_num = BSS_ULB_GET_MODE_SW_CHNUM(bss_ulb);

			if (!wlc_send_ulb_mode_sw_req(wlc, cfg, &scb->ea,
				BSS_ULB_GET_MODE_SW_TOKEN(bss_ulb), &opr_field, TRUE)) {
				WL_ERROR(("wl%d: %s: wlc_send_ulb_mode_sw_req()\n",
					wlc->pub->unit, __FUNCTION__));
				wlc_ulb_abort_mode_sw(bss_ulb);
				return;
			}
		} else {
			WL_ERROR(("wl%d: %s: Retry Limit (%d) Reached. Aborting ULB Mode-SW\n",
				ulb_info->pub->unit, __FUNCTION__, MAX_NUM_MODE_SW_AF_RETRY_CNT));
			wlc_ulb_abort_mode_sw(bss_ulb);
			return;
		}
	} else {
		WL_ERROR(("wl%d: %s: Unknown TX Status=0x%x\n", ulb_info->pub->unit,
			__FUNCTION__, tx_status));
		wlc_ulb_abort_mode_sw(bss_ulb);
	}
}

static void
wlc_ulb_ulb_mode_sw_rsp_cb(wlc_info_t *wlc, uint tx_status, void* arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)arg;
	wlc_ulb_info_t *ulb_info = wlc->ulb_info;
	bss_ulb_info_t *bss_ulb = ULB_BSSCFG_CUBBY(ulb_info, cfg);

	if (tx_status & TX_STATUS_ACK_RCV) {
		ulb_opr_field_t opr_field = {0};

		opr_field.cur_opr_bw = BSS_ULB_GET_MODE_SW_BW(bss_ulb);
		opr_field.pri_opr_bw = BSS_ULB_GET_MODE_SW_BW(bss_ulb);
		opr_field.pri_ch_num = BSS_ULB_GET_MODE_SW_CHNUM(bss_ulb);
		wlc_ulb_do_ulb_mode_sw(ulb_info, cfg, &opr_field, FALSE);

		if (wlc->block_datafifo & DATA_BLOCK_TX_SUPR)
			wlc_block_datafifo(wlc, DATA_BLOCK_TX_SUPR, 0);

		if (BSSCFG_AP(cfg)) {
			wlc_bss_update_beacon(wlc, cfg);
			wlc_bss_update_probe_resp(wlc, cfg, TRUE);
		}
	} else if ((tx_status & TX_STATUS_MASK) == TX_STATUS_NO_ACK) {
		if (++BSS_ULB_GET_MODE_SW_AF_RTCNT(bss_ulb) <= MAX_NUM_MODE_SW_AF_RETRY_CNT) {
			struct scb *scb = WLPKTTAGSCBGET(BSS_ULB_GET_MODE_SW_PKT(bss_ulb));
			ulb_opr_field_t opr_field = {0};

			/* make sure the scb still exists */
			if (scb == NULL) {
				WL_ERROR(("wl%d: %s: unable to find scb from the pkt %p\n",
					wlc->pub->unit, __FUNCTION__,
					BSS_ULB_GET_MODE_SW_PKT(bss_ulb)));
				wlc_ulb_abort_mode_sw(bss_ulb);
				return;
			}
			opr_field.cur_opr_bw = BSS_ULB_GET_MODE_SW_BW(bss_ulb);
			opr_field.pri_opr_bw = BSS_ULB_GET_MODE_SW_BW(bss_ulb);
			opr_field.pri_ch_num = BSS_ULB_GET_MODE_SW_CHNUM(bss_ulb);

			if (!wlc_send_ulb_mode_sw_rsp(wlc, cfg, &scb->ea,
				BSS_ULB_GET_MODE_SW_TOKEN(bss_ulb), &opr_field,
				ULB_MODE_SW_SC_SUCCESS, TRUE)) {
				WL_ERROR(("wl%d: %s: wlc_send_ulb_mode_sw_rsp() failed\n",
					ulb_info->pub->unit, __FUNCTION__));
				wlc_ulb_abort_mode_sw(bss_ulb);
				return;
			}
		} else {
			WL_ERROR(("wl%d: %s: Retry Limit (%d) Reached. Aborting ULB Mode-SW\n",
				ulb_info->pub->unit, __FUNCTION__, MAX_NUM_MODE_SW_AF_RETRY_CNT));
			wlc_ulb_abort_mode_sw(bss_ulb);
			return;
		}
	} else {
		WL_ERROR(("wl%d: %s: Unknown TX Status=0x%x\n", ulb_info->pub->unit,
			__FUNCTION__, tx_status));
		wlc_ulb_abort_mode_sw(bss_ulb);
	}
}

static void
wlc_ulb_abort_mode_sw(bss_ulb_info_t *bss_ulb)
{
	BSS_ULB_SET_MODE_SW_INPROG(bss_ulb, FALSE);
	BSS_ULB_SET_MODE_SW_AF_RTCNT(bss_ulb, 0);
}

/* Function handles setting of min_bw field in bsscfg during configuration of chanspec IOVAR */
int
wlc_ulb_hdl_set_ulb_chspec(wlc_ulb_info_t *ulb_info, wlc_bsscfg_t *bsscfg, uint16 chspec)
{
	wlc_info_t *wlc = ulb_info->wlc;
	int err = BCME_OK;

	/* Set the bsscfg min_bw to value specified in an ULB/20MHz chanspec */
	if (CHSPEC_BW_LE20(chspec)) {
		/* Configuring of ULB Chanspec when in Dynamic Operation is Blocked
		 */
		if (IS_ULB_DYN_MODE_ENABLED(wlc)) {
			if (CHSPEC_IS_ULB(wlc, chspec) ||
			    wlc_ulb_set_bss_min_bw(wlc->ulb_info, bsscfg,
			    CHSPEC_BW(chspec), FALSE)) {
				err = BCME_BADCHAN;
			}
		} else if (wlc_ulb_set_bss_min_bw(wlc->ulb_info, bsscfg,
				CHSPEC_BW(chspec), TRUE)) {
			err = BCME_BADCHAN;
		}
	}
	ulb_info->prev_bw = CHSPEC_BW(chspec);
	return err;
}
