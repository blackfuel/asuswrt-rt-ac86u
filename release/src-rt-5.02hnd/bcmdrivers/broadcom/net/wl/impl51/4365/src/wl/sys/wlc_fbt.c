/*
 * wlc_fbt.c -- FBT module source.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_fbt.c 515999 2014-11-18 08:18:09Z $
 */

/**
 * @file
 * @brief
 * Fast BSS Transition (802.11r) - roaming / fast authentication related
 */


#ifdef BCMINTSUP
#include <wlc_cfg.h>
#endif /* BCMINTSUP */

#ifndef	STA
#error "STA must be defined for wlc_fbt.c"
#endif /* STA */
#if !defined(WLFBT)
#error "WLFBT must be defined"
#endif /* !defined(WLFBT) */

#ifdef BCMINTSUP
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <proto/eap.h>
#include <proto/eapol.h>
#include <bcmwpa.h>
#if defined(BCMSUP_PSK)
#include <bcmcrypto/prf.h>
#endif /* BCMSUP_PSK */

#include <proto/802.11.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_keymgmt.h>
#include <wlc_led.h>
#include <wlc_rm.h>
#include <wlc_assoc.h>
#include <wl_export.h>
#include <wlc_scb.h>
#include <wlc_scb_ratesel.h>
#include <wlc_obss.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
#if defined(BCMSUP_PSK) || defined(WLFBT)
#include <wlc_wpa.h>
#endif /* BCMSUP_PSK */
#include <wlc_sup.h>
#ifdef WOWL
#include <wlc_wowl.h>
#endif
#ifdef WLBTAMP
#include <proto/802.11_bta.h>
#endif /* WLBTAMP */

#else /* external supplicant */

#include <stdio.h>
#include <typedefs.h>
#include <wlioctl.h>
#include <proto/eapol.h>
#include <proto/eap.h>
#include <bcmwpa.h>
#include <sup_dbg.h>
#include <bcmutils.h>
#include <string.h>
#include <bcmendian.h>
#include <bcmcrypto/prf.h>
#include <proto/eapol.h>
#include <bcm_osl.h>
#include "bcm_supenv.h"
#include "wpaif.h"
#include "wlc_sup.h"
#include "wlc_wpa.h"
#endif /* BCMINTSUP */

#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif
#include "wlc_fbt.h"
#include "wlc_cac.h"
#include <wlc_tx.h>

typedef struct wlc_fbt_priv {
	/* references to driver `common' things */
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;			/* pointer to wlc public portion */
	void *wl;			/* per-port handle */
	osl_t *osh;			/* PKT* stuff wants this */

	uint16 bss_fbt_priv_offset; /* offset of priv cubby in bsscfg */
} wlc_fbt_priv_t;

struct wlc_fbt_info {
	wlc_fbt_pub_t	mod_pub;
	wlc_fbt_priv_t	mod_priv;
};

typedef struct bss_fbt_priv {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;			/* pointer to wlc public portion */
	void *wl;			/* per-port handle */
	osl_t *osh;			/* PKT* stuff wants this */
	wlc_bsscfg_t *cfg;		/* pointer to sup's bsscfg */
	wlc_fbt_info_t *m_handle;	/* module handle */

	wpapsk_t *wpa;			/* volatile, initialized in set_sup */
	wpapsk_info_t *wpa_info;		/* persistent wpa related info */

	struct ether_addr peer_ea;      /* peer's ea */

	uint16 mdid;		/* Mobility domain id */
	uint16 mdie_len;	/* FTIE len */
	uchar *mdie;		/* MDIE */
	uint16 ftie_len;	/* FTIE len */
	uchar *ftie;		/* FTIE */
	bcm_tlv_t *r0khid;
	bcm_tlv_t *r1khid;
	uchar pmkr0name[WPA2_PMKID_LEN];
	uchar pmkr1name[WPA2_PMKID_LEN];
	uint8 current_akm_type;		/* AKM suite used for current association */
	bool use_sup_wpa;		/* TRUE if idsup is enabled and owns wpa/wpa_info */
} bss_fbt_priv_t;

typedef struct bss_fbt_info {
	bss_fbt_pub_t	bss_pub;
	bss_fbt_priv_t	bss_priv;
} bss_fbt_info_t;

/* wlc_fbt_info_priv_t offset in module states */
static uint16 wlc_fbt_info_priv_offset = OFFSETOF(wlc_fbt_info_t, mod_priv);

#define WLC_FBT_PRIV_INFO(fbt_info)		((wlc_fbt_priv_t *)((uint8 *)fbt_info + \
	wlc_fbt_info_priv_offset))

#define FBT_BSSCFG_CUBBY_LOC(fbt, cfg) ((bss_fbt_info_t **)BSSCFG_CUBBY(cfg, (fbt)->mod_pub.cfgh))
#define FBT_BSSCFG_CUBBY(fbt, cfg) (*FBT_BSSCFG_CUBBY_LOC(fbt, cfg))

#define BSS_PRIV_OFFSET(fbt_info)	((WLC_FBT_PRIV_INFO(fbt_info))->bss_fbt_priv_offset)
#define FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg) ((bss_fbt_priv_t *)((uint8 *) \
	(FBT_BSSCFG_CUBBY(fbt_info, cfg))+ 	BSS_PRIV_OFFSET(fbt_info)))
#define FBT_BSSCFG_CUBBY_PUB(fbt_info, cfg) ((bss_fbt_pub_t *)FBT_BSSCFG_CUBBY(fbt_info, cfg))

#define UNIT(ptr)	((ptr)->pub->unit)
#define CUR_EA(ptr)	(((bss_fbt_priv_t *)ptr)->cfg->cur_etheraddr)
#define PEER_EA(ptr)	(((bss_fbt_priv_t *)ptr)->peer_ea)
#define BSS_EA(ptr)	(((bss_fbt_priv_t *)ptr)->cfg->BSSID)
#define BSS_SSID(ptr)	(((bss_fbt_priv_t *)ptr)->cfg->current_bss->SSID)
#define BSS_SSID_LEN(ptr)	(((bss_fbt_priv_t *)ptr)->cfg->current_bss->SSID_len)

/* Transaction Sequence Numbers for FT MIC calculation. */
#define FT_MIC_ASSOC_REQUEST_TSN	5	/* TSN for association request frames. */
#define FT_MIC_REASSOC_RESPONSE_TSN	6	/* TSN for reassociation request response frames. */

#ifdef BCMDBG
static void wlc_fbt_dump_fbt_keys(bss_fbt_priv_t *fbt_bss_priv, uchar *pmkr0, uchar *pmkr1);
#endif

static int wlc_fbt_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif);
static void wlc_fbt_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_fbt_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_fbt_handle_joinproc(void *ctx, bss_assoc_state_data_t *evt_data);
static void wlc_fbt_bsscfg_updown_callbk(void *ctx, bsscfg_up_down_event_data_t *evt);
static void wlc_fbt_free_ies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);

#ifdef BCMSUP_PSK
static void wlc_fbt_sup_updown_callbk(void *ctx, sup_init_event_data_t *evt);
static void wlc_fbt_sup_init(void *ctx, sup_init_event_data_t *evt);
static void wlc_fbt_sup_deinit(void *ctx, wlc_bsscfg_t *cfg);
#endif /* BCMSUP_PSK */

/* IE mgmt */
static uint wlc_fbt_auth_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbt_auth_write_rsn_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_fbt_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *data);
static uint wlc_fbt_calc_md_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbt_write_md_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_fbt_parse_md_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_fbt_scan_parse_md_ie(void *ctx, wlc_iem_parse_data_t *data);
static uint wlc_fbt_calc_ft_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbt_write_ft_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_fbt_parse_ft_ie(void *ctx, wlc_iem_parse_data_t *data);
static uint wlc_fbt_calc_rde_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbt_write_rde_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_fbt_parse_rde_ie(void *ctx, wlc_iem_parse_data_t *data);

enum {
	IOV_ALLOW_FBTOVERDS,
	IOV_FBT_CAP,
	IOV_LAST
};

static const bcm_iovar_t fbt_iovars[] = {
	{"fbtoverds", IOV_ALLOW_FBTOVERDS, (0), IOVT_UINT32, 0},
	{"fbt_cap", IOV_FBT_CAP, (IOVF_OPEN_ALLOW), IOVT_UINT32, 0},
	{NULL, 0, 0, 0, 0}
};


/* FBT Over-the-DS Action frame IEs' order */
static const uint8 BCMINITDATA(fbt_ie_tags)[] = {
	DOT11_MNG_RSN_ID,
	DOT11_MNG_MDIE_ID,
	DOT11_MNG_FTIE_ID,
};

/* FBT RIC IEs' order in reassoc request frame */
static const uint8 BCMINITDATA(ric_ie_tags)[] = {
	DOT11_MNG_RDE_ID,
	DOT11_MNG_VS_ID
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>


/* Allocate fbt context, squirrel away the passed values,
 * and return the context handle.
 */
wlc_fbt_info_t *
BCMATTACHFN(wlc_fbt_attach)(wlc_info_t *wlc)
{
	wlc_fbt_info_t *fbt_info;
	wlc_fbt_priv_t	*fbt_priv;

	uint16 mdiefstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ) | FT2BMP(FC_AUTH);
	uint16 ftiefstbmp = FT2BMP(FC_REASSOC_REQ) | FT2BMP(FC_AUTH);
	uint16 scanfstbmp = FT2BMP(WLC_IEM_FC_SCAN_BCN) | FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);
	uint16 ftfstbmp = FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP) | FT2BMP(FC_AUTH);

	WL_TRACE(("wl%d: wlc_fbt_attach\n", wlc->pub->unit));

	if (!(fbt_info = (wlc_fbt_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_fbt_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	fbt_priv->wlc = wlc;
	fbt_priv->pub = wlc->pub;
	fbt_priv->wl = wlc->wl;
	fbt_priv->osh = wlc->osh;
	fbt_priv->bss_fbt_priv_offset = OFFSETOF(bss_fbt_info_t, bss_priv);

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((fbt_info->mod_pub.cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_fbt_info_t *),
	                NULL /* wlc_fbt_init */, wlc_fbt_bsscfg_deinit, NULL,
	                (void *)fbt_info)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          UNIT(fbt_priv), __FUNCTION__));
		goto err;
	}

	/* assoc join-start/done callback */
	if (wlc_bss_assoc_state_register(wlc, (bss_assoc_state_fn_t)wlc_fbt_handle_joinproc,
		fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_assoc_state_register() failed\n",
		          UNIT(fbt_priv), __FUNCTION__));
		goto err;
	}

	if (wlc_bsscfg_updown_register(wlc, wlc_fbt_bsscfg_updown_callbk, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_up_down_register() failed\n",
			UNIT(fbt_priv), __FUNCTION__));
		goto err;
	}

#ifdef BCMSUP_PSK
	if (SUP_ENAB(wlc->pub)) {
		if (wlc_sup_up_down_register(wlc, wlc_fbt_sup_updown_callbk, fbt_info) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_sup_up_down_register() failed\n",
			 UNIT(fbt_priv), __FUNCTION__));
			goto err;
		}
	}
#endif /* BCMSUP_PSK */

	/* register module */
	if (wlc_module_register(wlc->pub, fbt_iovars, "fbt", fbt_info, wlc_fbt_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: auth wlc_module_register() failed\n", UNIT(fbt_priv)));
		goto err;
	}
	/* sort calc_len/build callbacks for Action frame */
	(void)wlc_ier_sort_cbtbl(wlc->ier_fbt, fbt_ie_tags, sizeof(fbt_ie_tags));

	/* sort calc_len/build callbacks for FBT RIC(Resource Request) registry */
	(void)wlc_ier_sort_cbtbl(wlc->ier_ric, ric_ie_tags, sizeof(ric_ie_tags));

	/* calc/build */
	/* authreq */
	if (wlc_iem_add_build_fn(wlc->iemi, FC_AUTH, DOT11_MNG_RSN_ID,
	      wlc_fbt_auth_calc_rsn_ie_len, wlc_fbt_auth_write_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, rsn in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* authreq/assocreq/reassocreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, mdiefstbmp, DOT11_MNG_MDIE_ID,
	      wlc_fbt_calc_md_ie_len, wlc_fbt_write_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, md ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* authreq/reassocreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, ftiefstbmp, DOT11_MNG_FTIE_ID,
	      wlc_fbt_calc_ft_ie_len, wlc_fbt_write_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, ft ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* ftreq */
	if (wlc_ier_add_build_fn(wlc->ier_fbt, DOT11_MNG_RSN_ID,
	      wlc_fbt_auth_calc_rsn_ie_len, wlc_fbt_auth_write_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, rsn ie in ftreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* ftreq */
	if (wlc_ier_add_build_fn(wlc->ier_fbt, DOT11_MNG_MDIE_ID,
	      wlc_fbt_calc_md_ie_len, wlc_fbt_write_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, mdie in ftreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* ftreq */
	if (wlc_ier_add_build_fn(wlc->ier_fbt, DOT11_MNG_FTIE_ID,
	      wlc_fbt_calc_ft_ie_len, wlc_fbt_write_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, ftie in ftreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* reassocreq */
	if (wlc_ier_add_build_fn(wlc->ier_ric, DOT11_MNG_RDE_ID,
	      wlc_fbt_calc_rde_ie_len, wlc_fbt_write_rde_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, rde ie in reassocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* parse */
	/* bcn/prbrsp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, scanfstbmp, DOT11_MNG_MDIE_ID,
	                             wlc_fbt_scan_parse_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, md in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* reassocresp */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_REASSOC_RESP, DOT11_MNG_RSN_ID,
	                             wlc_fbt_parse_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, rsn ie */\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* authresp/assocresp/reassocresp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, ftfstbmp, DOT11_MNG_MDIE_ID,
	                         wlc_fbt_parse_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, md ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* authresp/assocresp/reassocresp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, ftfstbmp, DOT11_MNG_FTIE_ID,
	                         wlc_fbt_parse_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, ft ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* reassocresp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, FC_REASSOC_RESP, DOT11_MNG_RDE_ID,
	                                wlc_fbt_parse_rde_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, rde ie in reassocresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* ftresp */
	if (wlc_ier_add_parse_fn(wlc->ier_fbt, DOT11_MNG_RSN_ID,
	                        wlc_fbt_parse_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, rsn ie in ftresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* ftresp */
	if (wlc_ier_add_parse_fn(wlc->ier_fbt, DOT11_MNG_MDIE_ID,
	                        wlc_fbt_parse_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, md ie in ftresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* ftresp */
	if (wlc_ier_add_parse_fn(wlc->ier_fbt, DOT11_MNG_FTIE_ID,
	                        wlc_fbt_parse_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, ft ie in ftresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	return fbt_info;
err:
	wlc_fbt_detach(fbt_info);
	return NULL;
}

void
BCMATTACHFN(wlc_fbt_detach)(wlc_fbt_info_t *fbt_info)
{
	wlc_fbt_priv_t *fbt_priv;

	if (!fbt_info)
		return;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	WL_TRACE(("wl%d: wlc_fbt_detach\n", UNIT(fbt_priv)));

	wlc_bss_assoc_state_unregister(fbt_priv->wlc, wlc_fbt_handle_joinproc, fbt_info);
#ifdef BCMSUP_PSK
	if (SUP_ENAB(fbt_priv->wlc->pub))
		wlc_sup_up_down_unregister(fbt_priv->wlc, wlc_fbt_sup_updown_callbk, fbt_info);
#endif /* BCMSUP_PSK */
	wlc_bsscfg_updown_unregister(fbt_priv->wlc, wlc_fbt_bsscfg_updown_callbk, fbt_info);

	wlc_module_unregister(fbt_priv->pub, "fbt", fbt_info);
	MFREE(fbt_priv->osh, fbt_info, sizeof(wlc_fbt_info_t));
}

static int
wlc_fbt_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)handle;
	wlc_fbt_priv_t	*fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc_info_t *wlc = fbt_priv->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	bool bool_val = FALSE;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);
	if (BSSCFG_AP(bsscfg)) {
		err = BCME_NOTSTA;
		goto exit;
	}

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;
	(void)bool_val;

	switch (actionid) {

	case IOV_GVAL(IOV_ALLOW_FBTOVERDS):
		*ret_int_ptr = ((bsscfg->flags & WLC_BSSCFG_ALLOW_FTOVERDS) != 0);
		break;

	case IOV_SVAL(IOV_ALLOW_FBTOVERDS):
		if (bool_val)
			bsscfg->flags |= WLC_BSSCFG_ALLOW_FTOVERDS;
		else
			bsscfg->flags &= ~WLC_BSSCFG_ALLOW_FTOVERDS;
		break;

	case IOV_GVAL(IOV_FBT_CAP):
		if (WLFBT_ENAB(wlc->pub)) {
			*ret_int_ptr = WLC_FBT_CAP_DRV_4WAY_AND_REASSOC;
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}
exit:
	return err;
}

void
wlc_fbt_clear_ies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	wlc_fbt_free_ies(fbt_info, cfg);

	wlc_wpapsk_free(fbt_bss_priv->wlc, fbt_bss_priv->wpa);
	fbt_bss_priv->wpa_info->pmk_len = 0;
}

void
wlc_fbt_calc_fbt_ptk(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	wpapsk_t *wpa;
	wpapsk_info_t *wpa_info;
	uchar pmkr0[PMK_LEN], pmkr1[PMK_LEN];

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (!fbt_bss_priv)
		return;

	if (fbt_bss_priv->ftie) {
		wpa = fbt_bss_priv->wpa;
		wpa_info = fbt_bss_priv->wpa_info;

		/* calc PMK-R0 */
		wpa_calc_pmkR0(BSS_SSID(fbt_bss_priv), BSS_SSID_LEN(fbt_bss_priv),
			fbt_bss_priv->mdid,
			fbt_bss_priv->r0khid->data, fbt_bss_priv->r0khid->len,
			&CUR_EA(fbt_bss_priv),	wpa_info->pmk, (uint)wpa_info->pmk_len,
			pmkr0, fbt_bss_priv->pmkr0name);

		/* calc PMK-R1 */
		wpa_calc_pmkR1((struct ether_addr *)fbt_bss_priv->r1khid->data,
			&CUR_EA(fbt_bss_priv), pmkr0, PMK_LEN, fbt_bss_priv->pmkr0name,
			pmkr1, fbt_bss_priv->pmkr1name);

		/* calc PTK */
		wpa_calc_ft_ptk(&PEER_EA(fbt_bss_priv), &CUR_EA(fbt_bss_priv),
			wpa->anonce, wpa->snonce, pmkr1, PMK_LEN,
			wpa->eapol_mic_key, (uint)wpa->ptk_len);
#ifdef BCMDBG
		wlc_fbt_dump_fbt_keys(fbt_bss_priv, pmkr0, pmkr1);
#endif
	}
}

#ifdef BCMDBG
static void
wlc_fbt_dump_fbt_keys(bss_fbt_priv_t *fbt_bss_priv, uchar *pmkr0, uchar *pmkr1)
{
	if (WL_WSEC_ON()) {
		uchar *ptk;
		int i;

		if (!fbt_bss_priv)
			return;

		prhex("PMK", fbt_bss_priv->wpa_info->pmk, (uint)fbt_bss_priv->wpa_info->pmk_len);

		prhex("PMK-R0", pmkr0, 32);

		printf("R0KHID len %d : \n", fbt_bss_priv->r0khid->len);
		prhex("R0KHID", fbt_bss_priv->r0khid->data, fbt_bss_priv->r0khid->len);

		printf("R1KHID len %d : \n", fbt_bss_priv->r1khid->len);
		prhex("R1KHID", fbt_bss_priv->r1khid->data, fbt_bss_priv->r1khid->len);

		prhex("PMK-R0name", fbt_bss_priv->pmkr0name, 16);

		prhex("PMK-R1", pmkr1, 32);

		prhex("PMK-R1name", fbt_bss_priv->pmkr1name, 16);

		prhex("Anonce", fbt_bss_priv->wpa->anonce, 32);
		prhex("Snonce", fbt_bss_priv->wpa->snonce, 32);

		printf("BSSID : \n");
		for (i = 0; i < 6; i++) {
			printf(" 0x%2x ", PEER_EA(fbt_bss_priv).octet[i]);
		}
		printf("\n");

		ptk = (uchar *)fbt_bss_priv->wpa->eapol_mic_key;
		prhex("PTK", ptk, WPA_MIC_KEY_LEN);
	}
}
#endif /* BCMDBG */

bool
wlc_fbt_is_cur_mdid(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, wlc_bss_info_t *bi)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	return ((fbt_bss_priv->mdie_len != 0) && (bi->flags & WLC_BSS_FBT) &&
		(bi->mdid == fbt_bss_priv->mdid));
}

/* Calculate the RIC IE endptr and IE count. */
static bool
wlc_fbt_parse_ric(uint8 *pbody, int len, uint8 **ricend, int *ric_ie_count)
{
	bool found = FALSE;
	uint8 *ricptr;
	uint8 *_ricend = NULL;
	uint8 *end = NULL;
	int _ric_ie_count = 0;

	ricptr = pbody;
	end = pbody + len;
	while (ricptr < end) {
		bcm_tlv_t *ie = (bcm_tlv_t*)ricptr;
		int ie_len = TLV_HDR_LEN + ie->len;
		ricptr += ie_len;
		if (ie->id == DOT11_MNG_RDE_ID) {
			int i;
			dot11_rde_ie_t *rde = (dot11_rde_ie_t*)ie;

			/* Include RDE in MIC calculations. */
			_ric_ie_count += 1;
			/* Include protected elements too. */
			_ric_ie_count += rde->rd_count;
			for (i = 0; i < rde->rd_count; i++) {
				bcm_tlv_t *ie = (bcm_tlv_t*)ricptr;

				ricptr += TLV_HDR_LEN + ie->len;
				_ricend = ricptr;
			}
			/* Set return val. */
			found = TRUE;
		}
	}

	if (found) {
		*ricend = _ricend;
		*ric_ie_count = _ric_ie_count;
	}
	return found;
}

/* Populates the MIC and MIC control fields of the supplied FTIE. */
static bool
wlc_fbt_ft_calc_mic(bss_fbt_priv_t *fbt_bss_priv, dot11_ft_ie_t *ftie,
	bcm_tlv_t *mdie, bcm_tlv_t *rsnie, uint8 *ricdata, int ricdata_len,
	int ric_ie_count, uint8 trans_seq_nbr)
{
	int total_len = 0;
	uint8 *micdata;
	uint8 *pos;
	uint8 mic[PRF_OUTBUF_LEN];
	uint prot_ie_len = 3 + ric_ie_count;

	/* See 802.11r 11A.8.4 & 11A.8.5 for details. */
	/* Total expected size of buffer needed to calculate the MIC. */
	total_len += 2 * ETHER_ADDR_LEN;	/* AP & STA MAC addresses. */
	total_len += 1;				/* Transaction sequence number byte. */
	total_len += TLV_HDR_LEN + rsnie->len;
	total_len += TLV_HDR_LEN + mdie->len;
	total_len += TLV_HDR_LEN + ftie->len;
	total_len += ricdata_len;

	micdata = MALLOC(fbt_bss_priv->osh, total_len);
	if (micdata == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
		return FALSE;
	}
	pos = micdata;

	/* calc mic */
	bcopy((uint8 *)&CUR_EA(fbt_bss_priv), pos, ETHER_ADDR_LEN);
	pos += ETHER_ADDR_LEN;
	bcopy((uint8 *)&PEER_EA(fbt_bss_priv), pos, ETHER_ADDR_LEN);
	pos += ETHER_ADDR_LEN;

	/* Transaction sequence number. The value is dependent on whether we're called for an
	 * (re)association request, (re)association response, or other.
	 */
	*pos++ = trans_seq_nbr;

	bcopy(rsnie, pos, rsnie->len + TLV_HDR_LEN);
	pos += rsnie->len + TLV_HDR_LEN;

	bcopy(mdie, pos, mdie->len + TLV_HDR_LEN);
	pos += mdie->len + TLV_HDR_LEN;

	/* Prepare the FTIE. Set protected frame count in MIC control and zero the MIC field. */
	ftie->mic_control = htol16(prot_ie_len << 8);
	bzero(ftie->mic, sizeof(ftie->mic));

	bcopy(ftie, pos, ftie->len + TLV_HDR_LEN);
	pos += ftie->len + TLV_HDR_LEN;

	/* Add any RIC IEs to MIC data. */
	if (ricdata && ricdata_len) {
		/* Include RDE and counted frames in protected IE calculations. */
		bcopy(ricdata, pos, ricdata_len);
		pos += ricdata_len;
	}

	aes_cmac_calc(micdata, pos - micdata, fbt_bss_priv->wpa->eapol_mic_key,
		EAPOL_WPA_KEY_MIC_LEN, mic, EAPOL_WPA_KEY_MIC_LEN);
	bcopy(mic, &ftie->mic[0], EAPOL_WPA_KEY_MIC_LEN);
	MFREE(fbt_bss_priv->osh, micdata, total_len);

	return TRUE;
}

static bool
wlc_fbt_auth_build_rsnie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, uint8 *pbody)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	wpa_pmkid_list_t *wpa_pmkid;
	wpapsk_t *wpa;

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	wpa = fbt_bss_priv->wpa;

	bcopy(wpa->sup_wpaie, pbody, wpa->sup_wpaie_len);
	if (wpa->sup_wpaie_len != 0) {
		/* Add pmkr0name to rsnie */
		pbody[1] += sizeof(wpa_pmkid_list_t);
		pbody += wpa->sup_wpaie_len;
		wpa_pmkid = (wpa_pmkid_list_t *)pbody;
		wpa_pmkid->count.low = 1;
		wpa_pmkid->count.high = 0;
		bcopy(fbt_bss_priv->pmkr0name, &wpa_pmkid->list[0], WPA2_PMKID_LEN);
	}
	return TRUE;
}

static bool
wlc_fbt_auth_build_mdie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, uint8 *pbody)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	/* Add mdie */
	if (fbt_bss_priv->mdie != NULL) {
		bcopy(fbt_bss_priv->mdie, pbody, fbt_bss_priv->mdie_len);
	}
	return TRUE;
}

static bool
wlc_fbt_auth_build_ftie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, uint8 *pbody)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	/* Add FTIE with snonce, r0kh-id */
	if (fbt_bss_priv->r0khid != NULL) {
		dot11_ft_ie_t *ftie = (dot11_ft_ie_t *)pbody;
		wpapsk_t *wpa = fbt_bss_priv->wpa;

		bzero(ftie, sizeof(dot11_ft_ie_t));
		ftie->id = DOT11_MNG_FTIE_ID;
		ftie->len = sizeof(dot11_ft_ie_t) + fbt_bss_priv->r0khid->len;
		++wpa->snonce[0];
		bcopy(wpa->snonce, ftie->snonce, EAPOL_WPA_KEY_NONCE_LEN);
		pbody += sizeof(dot11_ft_ie_t);
		ASSERT((fbt_bss_priv->ftie_len >= sizeof(dot11_ft_ie_t)) &&
			(fbt_bss_priv->ftie_len < 255));
		bcopy(fbt_bss_priv->r0khid, pbody, TLV_HDR_LEN + fbt_bss_priv->r0khid->len);
	}
	return TRUE;
}

/* Save a copy of RSN IE from Assoc req */
static bool
wlc_fbt_find_rsnie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	/* Do some initialisation that would normally be done by wpa_start()
	 * This is for the case where the external supplicant is performing the 4-way handshake but
	 * we'd still like to do reassociations within the ESS.
	 */
	if (fbt_bss_priv) {
		wpapsk_t *wpa = fbt_bss_priv->wpa;
		uint8 *sup_ies, *auth_ies;
		uint sup_ies_len, auth_ies_len;
		uchar *sup_wpaie;

		wlc_find_sup_auth_ies(NULL, cfg, &sup_ies, &sup_ies_len,
			&auth_ies, &auth_ies_len);

		sup_wpaie = (uchar *)bcm_parse_tlvs(sup_ies, sup_ies_len, DOT11_MNG_RSN_ID);
		if (sup_wpaie == NULL) {
			WL_ERROR(("wl%d: %s: STA WPA IE not found in sup_ies with len %d\n",
				UNIT(fbt_bss_priv), __FUNCTION__, sup_ies_len));
			return FALSE;
		}
		if (wpa->sup_wpaie)
			MFREE(fbt_bss_priv->osh, wpa->sup_wpaie, wpa->sup_wpaie_len);

		/* Save copy of STA's WPA IE */
		wpa->sup_wpaie_len = (uint16) (sup_wpaie[1] + TLV_HDR_LEN);
		wpa->sup_wpaie = (uchar *) MALLOC(fbt_bss_priv->osh, wpa->sup_wpaie_len);
		if (!wpa->sup_wpaie) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
			return FALSE;
		}
		bcopy((char *)sup_wpaie, (char *)wpa->sup_wpaie, wpa->sup_wpaie_len);

		wlc_getrand(fbt_bss_priv->wlc, wpa->snonce, EAPOL_WPA_KEY_NONCE_LEN);
		wpa->ptk_len = AES_PTK_LEN;
		wpa->tk_len = AES_TK_LEN;
		wpa->desc = WPA_KEY_DESC_V2;
	}

	return TRUE;
}

/* Parse auth resp frame for mdie */
static bool
wlc_fbt_auth_parse_mdie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, uint8 *body, int body_len)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (fbt_bss_priv->mdie_len == 0) {
		dot11_mdid_ie_t *mdie = (dot11_mdid_ie_t *)body;

		fbt_bss_priv->mdie_len = mdie->len + TLV_HDR_LEN;
		fbt_bss_priv->mdie = MALLOC(fbt_bss_priv->osh, fbt_bss_priv->mdie_len);
		if (!fbt_bss_priv->mdie) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			   UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
			return FALSE;
		}
		bcopy(mdie, fbt_bss_priv->mdie, fbt_bss_priv->mdie_len);
		fbt_bss_priv->mdid = ltoh16_ua(&mdie->mdid);
	}
	return TRUE;
}

/* Parse auth resp frame for ftie */
static bool
wlc_fbt_auth_parse_ftie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, uint8 *body, int body_len)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	dot11_ft_ie_t *ftie = (dot11_ft_ie_t *)body;
	wpapsk_t *wpa;
	uchar *tlvs;
	uint tlv_len;

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	wpa = fbt_bss_priv->wpa;

	if (fbt_bss_priv->ftie)
		MFREE(fbt_bss_priv->osh, fbt_bss_priv->ftie, fbt_bss_priv->ftie_len);

	fbt_bss_priv->ftie_len = ftie->len + TLV_HDR_LEN;
	fbt_bss_priv->ftie = MALLOC(fbt_bss_priv->osh, fbt_bss_priv->ftie_len);
	if (!fbt_bss_priv->ftie) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
		return FALSE;
	}
	bcopy(ftie, fbt_bss_priv->ftie, fbt_bss_priv->ftie_len);

	tlvs = (uchar *)((uintptr)fbt_bss_priv->ftie + sizeof(dot11_ft_ie_t));
	tlv_len = fbt_bss_priv->ftie_len - sizeof(dot11_ft_ie_t);
	fbt_bss_priv->r1khid = bcm_parse_tlvs(tlvs, tlv_len, 1);
	fbt_bss_priv->r0khid = bcm_parse_tlvs(tlvs, tlv_len, 3);
	bcopy(ftie->anonce, wpa->anonce, sizeof(wpa->anonce));
	ASSERT(fbt_bss_priv->r0khid && fbt_bss_priv->r1khid);
	wlc_fbt_calc_fbt_ptk(fbt_info, cfg);
	return TRUE;
}

static bool
wlc_fbt_reassoc_build_ftie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
   uint8 *buf, int buflen, bcm_tlv_t *mdie, bcm_tlv_t *rsnie,
   uint8 *ricdata, int ricdata_len, int ric_ie_count)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (fbt_bss_priv->ftie && fbt_bss_priv->ftie_len) {
		uint8 *ftptr;

		ftptr = MALLOC(fbt_bss_priv->osh, fbt_bss_priv->ftie_len);
		if (ftptr == NULL) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			   UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
			return FALSE;
		} else {
			bcopy(fbt_bss_priv->ftie, ftptr, fbt_bss_priv->ftie_len);
			/* Calculate MIC */
			if (wlc_fbt_ft_calc_mic(fbt_bss_priv, (dot11_ft_ie_t*)ftptr, mdie, rsnie,
				ricdata, ricdata_len, ric_ie_count, FT_MIC_ASSOC_REQUEST_TSN)) {
				bcm_copy_tlv_safe(ftptr, buf, buflen);
			}
			MFREE(fbt_bss_priv->osh, ftptr, fbt_bss_priv->ftie_len);
		}
	}
	return TRUE;
}

/*
 * wlc_fbt_reassoc_validate_ftie: Checks the validity of the FTIE (if provided) in the reassociation
 * request response frame.
 *
 * Returns TRUE if the reassociation request response frame is good, FALSE otherwise.
 * A FALSE return will mean that the frame contained an FTIE but was missing either an MDIE or an
 * RSNIE (shouldn't really happen) or that the FT MIC was invalid. In either case, the calling
 * function should discard the reassociation request response frame.
 */
static bool
wlc_fbt_reassoc_validate_ftie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t* cfg, uint8 *ricie,
	int ricie_len, dot11_ft_ie_t *ftie, bcm_tlv_t *mdie, bcm_tlv_t *rsnie)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	bool is_valid = TRUE;

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (ftie != NULL) {
		uint8 *ricend = NULL;
		int ric_ie_count = 0;
		uint8 *ftptr;
		dot11_ft_ie_t *myftie;

		if (mdie == NULL) {
			WL_ERROR(("%s: MDIE not found and required\n", __FUNCTION__));
			return FALSE;
		}
		if (rsnie == NULL) {
			WL_ERROR(("%s: RSNIE not found and required\n", __FUNCTION__));
			return FALSE;
		}

		ftptr = MALLOC(fbt_bss_priv->osh, ftie->len + TLV_HDR_LEN);
		if (ftptr == NULL) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			   UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
			return FALSE;
		}
		if (ricie != NULL) {
			wlc_fbt_parse_ric(ricie, ricie_len, &ricend, &ric_ie_count);
		}
		/* Copy received FTIE into own buffer as MIC calculation will modify it. */
		bcopy(ftie, ftptr, ftie->len + TLV_HDR_LEN);
		wlc_fbt_ft_calc_mic(fbt_bss_priv, (dot11_ft_ie_t*)ftptr, mdie,
			rsnie, ricie, ricend - ricie, ric_ie_count, FT_MIC_REASSOC_RESPONSE_TSN);

		/* Now get the calculated MIC and compare to that in ftie. */
		myftie = (dot11_ft_ie_t*)ftptr;
		if (bcmp(myftie->mic, ftie->mic, EAPOL_WPA_KEY_MIC_LEN)) {
			/* MICs are different. */
			is_valid = FALSE;
			if (WL_ERROR_ON()) {
				WL_ERROR(("%s: FT-MICs do not match\n", __FUNCTION__));
				prhex("Recv MIC", ftie->mic, EAPOL_WPA_KEY_MIC_LEN);
				prhex("Calc MIC", myftie->mic, EAPOL_WPA_KEY_MIC_LEN);
			}
		}
		MFREE(fbt_bss_priv->osh, ftptr, ftie->len + TLV_HDR_LEN);
	}
	return is_valid;
}

/* Looks for 802.11r RIC response TLVs. */
static bool
wlc_fbt_reassoc_parse_rdeie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t* cfg, uint8 *tlvs, int tlvs_len)
{
	bool accepted = TRUE;
	bcm_tlv_t *ric_ie;

	ric_ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RDE_ID);
	while (ric_ie && accepted) {
		dot11_rde_ie_t *rdeptr = (dot11_rde_ie_t*)ric_ie;
		accepted = (rdeptr->status == 0);
		if (accepted) {
			/* May contain more than one RDE, so look for more. */
			tlvs += DOT11_MNG_RDE_IE_LEN;
			tlvs_len -= DOT11_MNG_RDE_IE_LEN;
			ric_ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RDE_ID);
		}
	}
	return accepted;
}

uint8 *
wlc_fbt_get_pmkr1name(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return NULL;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	return (fbt_bss_priv->pmkr1name);
}

static bool
wlc_fbt_process_reassoc_resp(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	wlc_assoc_t *as = cfg->assoc;
	dot11_ft_ie_t *ftie;
	uint8 *tlvs;
	uint tlv_len;
	dot11_gtk_ie_t *gtk;
	wpapsk_t *wpa;

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	wpa = fbt_bss_priv->wpa;

	tlvs = (uint8 *)&as->resp[1];
	tlv_len = as->resp_len - DOT11_ASSOC_RESP_FIXED_LEN;
	if ((ftie = (dot11_ft_ie_t *)bcm_parse_tlvs(tlvs, tlv_len, DOT11_MNG_FTIE_ID)) != NULL) {
		tlvs = (uint8 *)((uintptr)ftie + sizeof(dot11_ft_ie_t));
		tlv_len = ftie->len - sizeof(dot11_ft_ie_t) + TLV_HDR_LEN;
		/* plumb the keys here and set the scb authorized */
		if ((gtk = (dot11_gtk_ie_t *)bcm_parse_tlvs(tlvs, tlv_len, 2)) != NULL) {
			wpa->gtk_len = gtk->key_len;
			wpa->gtk_index = ltoh16_ua(&gtk->key_info) & 0x3;
			/* extract and plumb GTK */
			ASSERT(gtk->len == 35);
			if (aes_unwrap(WPA_ENCR_KEY_LEN, wpa->eapol_encr_key, gtk->len - 11,
				&gtk->data[0], wpa->gtk)) {
				WL_WSEC(("FBT reassoc: GTK decrypt failed\n"));
				return FALSE;
			}
			wlc_wpa_plumb_gtk(fbt_bss_priv->wlc, fbt_bss_priv->cfg, wpa->gtk,
				wpa->gtk_len, wpa->gtk_index, wpa->mcipher, gtk->rsc, TRUE);

			wlc_wpa_plumb_tk(fbt_bss_priv->wlc, fbt_bss_priv->cfg,
				(uint8*)wpa->temp_encr_key,
				wpa->tk_len, wpa->ucipher, &PEER_EA(fbt_bss_priv));

			wlc_ioctl(fbt_bss_priv->wlc, WLC_SCB_AUTHORIZE, &PEER_EA(fbt_bss_priv),
				ETHER_ADDR_LEN, fbt_bss_priv->cfg->wlcif);

			return TRUE;
		}
	}

	return FALSE;
}

uint16
wlc_fbt_getlen_eapol(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return 0;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	return (sizeof(wpa_pmkid_list_t) + fbt_bss_priv->ftie_len +	sizeof(dot11_mdid_ie_t));
}

/* Insert IEs in msg2 for 4-way eapol handshake */
void
wlc_fbt_addies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, eapol_wpa_key_header_t *wpa_key)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	wpa_pmkid_list_t *pmkid;
	wpapsk_t * wpa;

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	wpa = fbt_bss_priv->wpa;

	wpa_key->data[1] += sizeof(wpa_pmkid_list_t);

	pmkid = (wpa_pmkid_list_t *)&wpa_key->data[wpa->sup_wpaie_len];
	pmkid->count.low = 1;
	pmkid->count.high = 0;
	bcopy(fbt_bss_priv->pmkr1name, &pmkid->list[0], WPA2_PMKID_LEN);
	bcopy(fbt_bss_priv->mdie, &pmkid[1], sizeof(dot11_mdid_ie_t));
	bcopy(fbt_bss_priv->ftie, (uint8 *)&pmkid[1] + sizeof(dot11_mdid_ie_t),
		fbt_bss_priv->ftie_len);
}

static bool
wlc_fbt_parse_associe(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	wlc_assoc_t *as = cfg->assoc;
	bcm_tlv_t *mdie, *ftie;
	int ies_len;
	uchar * assoc_ies = (uchar *)&as->resp[1];

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	ies_len = as->resp_len - sizeof(struct dot11_assoc_resp);
	if ((mdie = bcm_parse_tlvs(assoc_ies, ies_len, DOT11_MNG_MDIE_ID)) != NULL) {
		fbt_bss_priv->mdie_len = mdie->len + TLV_HDR_LEN;
		fbt_bss_priv->mdie = MALLOC(fbt_bss_priv->osh, fbt_bss_priv->mdie_len);
		if (!fbt_bss_priv->mdie) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				fbt_bss_priv->pub->unit, __FUNCTION__,
				MALLOCED(fbt_bss_priv->osh)));
			return FALSE;
		}
		bcopy(mdie, fbt_bss_priv->mdie, fbt_bss_priv->mdie_len);
		fbt_bss_priv->mdid = ltoh16_ua(&mdie->data[0]);
	}
	if ((ftie = bcm_parse_tlvs(assoc_ies, ies_len, DOT11_MNG_FTIE_ID)) != NULL) {
		uchar *tlvs;
		uint tlv_len;

		fbt_bss_priv->ftie_len = ftie->len + TLV_HDR_LEN;
		fbt_bss_priv->ftie = MALLOC(fbt_bss_priv->osh, fbt_bss_priv->ftie_len);
		if (!fbt_bss_priv->ftie) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				fbt_bss_priv->pub->unit, __FUNCTION__,
				MALLOCED(fbt_bss_priv->osh)));
			return FALSE;
		}
		bcopy(ftie, fbt_bss_priv->ftie, fbt_bss_priv->ftie_len);
		tlvs = (uchar *)((uintptr)fbt_bss_priv->ftie + sizeof(dot11_ft_ie_t));
		tlv_len = fbt_bss_priv->ftie_len - sizeof(dot11_ft_ie_t);
		fbt_bss_priv->r1khid = bcm_parse_tlvs(tlvs, tlv_len, 1);
		fbt_bss_priv->r0khid = bcm_parse_tlvs(tlvs, tlv_len, 3);
	}

	BSS_FBT_INI_FBT(fbt_info, cfg) = (mdie && ftie) ? TRUE : FALSE;
	return TRUE;
}

void
wlc_fbt_set_ea(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, struct ether_addr *ea)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = NULL;

	if (fbt_info)
		fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	else
		WL_ERROR(("%s: fbt_info is null\n", __FUNCTION__));

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	bcopy(ea, &fbt_bss_priv->peer_ea, ETHER_ADDR_LEN);
}

static void
wlc_fbt_wpa_free(bss_fbt_priv_t *fbt_bss_priv)
{
	if (fbt_bss_priv->wpa) {
		MFREE(fbt_bss_priv->osh, fbt_bss_priv->wpa, sizeof(*fbt_bss_priv->wpa));
		fbt_bss_priv->wpa = NULL;
	}
	if (fbt_bss_priv->wpa_info) {
		MFREE(fbt_bss_priv->osh, fbt_bss_priv->wpa_info, sizeof(*fbt_bss_priv->wpa_info));
		fbt_bss_priv->wpa_info = NULL;
	}
}

static void
wlc_fbt_bsscfg_updown_callbk(void *ctx, bsscfg_up_down_event_data_t *evt)
{
	if (!evt->up)
		wlc_fbt_clear_ies(ctx, evt->bsscfg);
}

static bss_fbt_priv_t *
wlc_fbt_bsscfg_cubby_init(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	wlc_fbt_priv_t	*fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	bss_fbt_info_t **pfbt_cfg = FBT_BSSCFG_CUBBY_LOC(fbt_info, cfg);
	bss_fbt_info_t *fbt_bss = NULL;
	bss_fbt_priv_t *fbt_bss_priv = NULL;

	if (!(fbt_bss = (bss_fbt_info_t *)MALLOCZ(fbt_priv->osh, sizeof(bss_fbt_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(fbt_priv), __FUNCTION__, MALLOCED(fbt_priv->osh)));
	}

	fbt_bss_priv = (bss_fbt_priv_t *)((uint8 *)fbt_bss + BSS_PRIV_OFFSET(fbt_info));

	fbt_bss_priv->m_handle = fbt_info;
	fbt_bss_priv->cfg = cfg;
	fbt_bss_priv->wlc = fbt_priv->wlc;
	fbt_bss_priv->osh = fbt_priv->osh;
	fbt_bss_priv->wl = fbt_priv->wl;
	fbt_bss_priv->pub = fbt_priv->pub;

	*pfbt_cfg = fbt_bss;
	return fbt_bss_priv;
}

static bss_fbt_priv_t *
wlc_fbt_get_bsscfg(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = NULL;

	fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	if (fbt_bss == NULL)
		fbt_bss_priv = wlc_fbt_bsscfg_cubby_init(fbt_info, cfg);
	else
		fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	return fbt_bss_priv;
}

/* Create fbt bsscfg when idsup comes up (sup_wpa=1) and use
 * supplicant wpa/wpa_info elements.
 * If idsup is disabled/not used, create fbt bsscfg at the end of first
 * association and use local fbt wpa/wpa_info elements.
 */
static void
wlc_fbt_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_fbt_priv_t	*fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	bss_fbt_priv_t *fbt_bss_priv = NULL;

	fbt_bss_priv = wlc_fbt_get_bsscfg(fbt_info, cfg);
	if (fbt_bss_priv == NULL)
		goto err;

	/* Use fbt wpa/wpa_info only if idsup is not being used. */
	if (fbt_bss_priv->use_sup_wpa == FALSE) {
		/* Check if wpa already exists as this is called for every JOIN_START */
		if (!fbt_bss_priv->wpa) {
			fbt_bss_priv->wpa = MALLOCZ(fbt_priv->osh, sizeof(wpapsk_t));
			if (fbt_bss_priv->wpa == NULL) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				  UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
				goto err;
			}
		}

		/* Check if wpa-info already exists */
		if (!fbt_bss_priv->wpa_info) {
			fbt_bss_priv->wpa_info = MALLOCZ(fbt_priv->osh, sizeof(wpapsk_info_t));
			if (fbt_bss_priv->wpa_info == NULL) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				  UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
				goto err;
			}
		}
	}
	return;
err:
	wlc_fbt_bsscfg_deinit(fbt_info, cfg);
}

static
void wlc_fbt_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	bss_fbt_info_t **pfbt_cfg = FBT_BSSCFG_CUBBY_LOC(fbt_info, cfg);

	if (!fbt_bss)
		return;

	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	wlc_fbt_free_ies(fbt_info, cfg);

	if (fbt_bss_priv->use_sup_wpa == FALSE)
		wlc_fbt_wpa_free(fbt_bss_priv);

	MFREE(fbt_bss_priv->osh, fbt_bss, sizeof(bss_fbt_info_t));
	*pfbt_cfg = NULL;
}

static void
wlc_fbt_handle_joindone(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	bss_fbt_priv_t *fbt_bss_priv;

	if (!fbt_bss)
		return;

	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	ASSERT(fbt_bss_priv);

	if (cfg->WPA_auth & WPA2_AUTH_FT) {
		if (cfg->auth_atmptd == DOT11_FAST_BSS) {
			/* plumbs keys from response frame for FBT-lite mode */
			wlc_fbt_process_reassoc_resp(fbt_info, cfg);
		} else {
			WL_WSEC(("wl%d: FBT adopt bss \n",
				WLCWLUNIT(fbt_bss_priv->wlc)));

			if (!fbt_bss_priv->use_sup_wpa)
				wlc_fbt_clear_ies(fbt_info, cfg);
			else
				wlc_fbt_free_ies(fbt_info, cfg);

			/* Assoc response does not contain RSN IE. So save the RSN IE from
			 * assoc req for initial association. This is required for FBT-lite
			 * mode where idsup is disabled.
			 */
			wlc_fbt_find_rsnie(fbt_info, cfg);
			/* Parse assoc resp for mdie and ftie for initial fbt association */
			wlc_fbt_parse_associe(fbt_info, cfg);
			wlc_fbt_set_ea(fbt_info, cfg, &cfg->BSSID);

			if (!fbt_bss_priv->use_sup_wpa) {
				fbt_bss_priv->wpa->WPA_auth = cfg->WPA_auth;
				fbt_bss_priv->wpa_info->wlc = fbt_bss_priv->wlc;
			}
		}
	}
}

static void
wlc_fbt_handle_joinproc(void *ctx, bss_assoc_state_data_t *evt_data)
{
	if (evt_data) {
		if (evt_data->state == AS_JOIN_INIT) {
			wlc_fbt_bsscfg_init(ctx, evt_data->cfg);
		} else if (evt_data->state == AS_JOIN_ADOPT) {
			wlc_fbt_handle_joindone(ctx, evt_data->cfg);
		}
	}
}

static void
wlc_fbt_free_ies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (fbt_bss_priv->mdie != NULL) {
		MFREE(fbt_bss_priv->osh, fbt_bss_priv->mdie, fbt_bss_priv->mdie_len);
		fbt_bss_priv->mdie = NULL;
		fbt_bss_priv->mdie_len = 0;
	}
	if (fbt_bss_priv->ftie != NULL) {
		MFREE(fbt_bss_priv->osh, fbt_bss_priv->ftie, fbt_bss_priv->ftie_len);
		fbt_bss_priv->ftie = NULL;
		fbt_bss_priv->ftie_len = 0;
	}
	BSS_FBT_INI_FBT(fbt_info, cfg) = FALSE;
}

static uint
wlc_fbt_auth_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (WLFBT_ENAB(fbt_priv->wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
		(cfg->auth_atmptd == DOT11_FAST_BSS)) {
		bss_fbt_priv_t *fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
		return (fbt_bss_priv->wpa->sup_wpaie_len + sizeof(wpa_pmkid_list_t));
	}
	return 0;
}

/* RSN IE in auth req for fast transition */
static int
wlc_fbt_auth_write_rsn_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_fbt_priv_t *fbt_priv;
	wlc_bsscfg_t *cfg = data->cfg;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (WLFBT_ENAB(fbt_priv->wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
		(cfg->auth_atmptd == DOT11_FAST_BSS))
		wlc_fbt_auth_build_rsnie(fbt_info, cfg, data->buf);

	return BCME_OK;
}

static int
wlc_fbt_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	if (WLFBT_ENAB(fbt_priv->wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
		(data->ft == FC_REASSOC_RESP)) {
		/* Save the RSN IE for MIC calculation */
		ftpparm->assocresp.wpa2_ie = data->ie;
	}
	return BCME_OK;
}

static uint
wlc_fbt_calc_md_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	bss_fbt_priv_t *fbt_bss_priv;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm;
	uint ielen = 0;

	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (WLFBT_ENAB(fbt_priv->wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
			if (cfg->auth_atmptd == DOT11_FAST_BSS) {
				ielen = fbt_bss_priv->mdie_len;
			}
			break;
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ: {
			wlc_bss_info_t *bi;
			uint8 *tlvs = NULL;
			uint tlvs_len = 0;
			bcm_tlv_t *md_ie;

			ftcbparm = data->cbparm->ft;
			ASSERT(ftcbparm != NULL);

			bi = ftcbparm->assocreq.target;

			/* find the MD IE */
			if (bi->wpa2.flags & RSN_FLAGS_FBT && bi->bcn_prb != NULL) {
				tlvs = (uint8 *)bi->bcn_prb + sizeof(struct dot11_bcn_prb);
				ASSERT(bi->bcn_prb_len >= sizeof(struct dot11_bcn_prb));
				tlvs_len = bi->bcn_prb_len - sizeof(struct dot11_bcn_prb);

				if ((md_ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_MDIE_ID))
					!= NULL) {
					ftcbparm->assocreq.md_ie = (uint8 *)md_ie;
					ielen = TLV_HDR_LEN + ftcbparm->assocreq.md_ie[TLV_LEN_OFF];
				}
			}
			break;
		}
		default:
			break;
		}
	}
	return ielen;
}

static int
wlc_fbt_write_md_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (WLFBT_ENAB(fbt_priv->wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			if (cfg->auth_atmptd == DOT11_FAST_BSS) {
				wlc_fbt_auth_build_mdie(fbt_info, cfg, data->buf);
			}
			break;
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ:
			ftcbparm = data->cbparm->ft;
			ASSERT(ftcbparm != NULL);
			if (ftcbparm->assocreq.target->wpa2.flags & RSN_FLAGS_FBT &&
				(ftcbparm->assocreq.md_ie != NULL))
				bcm_copy_tlv(ftcbparm->assocreq.md_ie, data->buf);
			break;
		default:
			break;
		}
	}
	return BCME_OK;
}

static int
wlc_fbt_parse_md_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	if (WLFBT_ENAB(fbt_priv->wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			if (cfg->auth_atmptd == DOT11_FAST_BSS) {
				if (!wlc_fbt_auth_parse_mdie(fbt_info, cfg, data->ie, data->ie_len))
					ftpparm->auth.status = DOT11_SC_FAILURE;
			}
			break;
		case FC_ASSOC_RESP:
			/* FBT-lite mode: pulls FTIE & MDIE from assocresp */
			if (!wlc_fbt_auth_parse_mdie(fbt_info, cfg, data->ie, data->ie_len)) {
				ftpparm->assocresp.status = DOT11_SC_FAILURE;
			}
			break;
		case FC_REASSOC_RESP:
			ftpparm->assocresp.md_ie = data->ie;
		default:
			break;
		}
	}
	return BCME_OK;
}

/* Parse MD IE from the beacon/probe resp */
static int
wlc_fbt_scan_parse_md_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;
	wlc_info_t *wlc;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (!WLFBT_ENAB(wlc->pub))
		return BCME_OK;

	if (data->ie != NULL) {
		dot11_mdid_ie_t * mdie = (dot11_mdid_ie_t *)data->ie;

		bi->mdid = ltoh16_ua(&mdie->mdid);
		bi->flags |= WLC_BSS_FBT;
		if ((mdie->cap & FBT_MDID_CAP_OVERDS) &&
			(wlc->cfg->flags & WLC_BSSCFG_ALLOW_FTOVERDS))
			bi->flags2 |= WLC_BSS_OVERDS_FBT;
	}
	return BCME_OK;
}

static uint
wlc_fbt_calc_ft_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm;
	bss_fbt_priv_t *fbt_bss_priv;
	wlc_info_t *wlc;
	uint ielen = 0;

	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (WLFBT_ENAB(fbt_priv->wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			if (cfg->auth_atmptd == DOT11_FAST_BSS)
				ielen = TLV_HDR_LEN + sizeof(dot11_ft_ie_t) +
					fbt_bss_priv->r0khid->len;
			break;
		case FC_REASSOC_REQ:
			ftcbparm = data->cbparm->ft;
			ASSERT(ftcbparm != NULL);
			/* Calculate the length of FT IE and the length of RIC IEs
			 * using RIC registry.
			 */
			if ((ftcbparm->assocreq.target->wpa2.flags & RSN_FLAGS_FBT) &&
			    ftcbparm->assocreq.md_ie != NULL) {
				ielen = fbt_bss_priv->ftie_len +
				  wlc_cac_calc_ric_len(wlc->cac, cfg);
			}
			break;
		default:
			break;
		}
	}
	return ielen;
}

static int
wlc_fbt_write_ft_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	bss_fbt_priv_t *fbt_bss_priv;
	wlc_info_t *wlc;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(fbt_priv->wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			if (cfg->auth_atmptd == DOT11_FAST_BSS)
				wlc_fbt_auth_build_ftie(fbt_info, cfg, data->buf);
			break;
		case FC_REASSOC_REQ: {
			uint ricies_len;
			uint8 *ricies = NULL;
			int ricie_count = 0;
			wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
			ASSERT(ftcbparm != NULL);

			if ((ftcbparm->assocreq.target->wpa2.flags & RSN_FLAGS_FBT) &&
			    ftcbparm->assocreq.wpa2_ie != NULL &&
			    ftcbparm->assocreq.md_ie != NULL) {
				/* Build the IEs in RIC request using RIC registry first
				 * and then build the FT IE as the RIC IEs have to be
				 * included for calculating the MIC which is part of FT IE.
				 */
				ricies_len = wlc_cac_calc_ric_len(wlc->cac, cfg);
				/* Point to the start of RIC buffer in reassoc req frame */
				if (ricies_len)
					ricies = data->buf + fbt_bss_priv->ftie_len;

				/* Build RIC IEs */
				wlc_cac_write_ric(wlc->cac, cfg, ricies, &ricie_count);

				/* Calculate MIC and generate FT IE */
				wlc_fbt_reassoc_build_ftie(fbt_info, cfg, data->buf,
					data->buf_len, (bcm_tlv_t *)ftcbparm->assocreq.md_ie,
					(bcm_tlv_t *)ftcbparm->assocreq.wpa2_ie,
					ricies, ricies_len, ricie_count);
			}
			break;
		}
		default:
			break;
		}
	}
	return BCME_OK;
}

static int
wlc_fbt_parse_ft_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	if (WLFBT_ENAB(fbt_priv->wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			if (cfg->auth_atmptd == DOT11_FAST_BSS) {
				if (!wlc_fbt_auth_parse_ftie(fbt_info, cfg, data->ie, data->ie_len))
					ftpparm->auth.status = DOT11_SC_FAILURE;
			}
			break;
		case FC_ASSOC_RESP:
			/* FBT-lite mode: pull FTIE & MDIE from assocresp */
			if (!wlc_fbt_auth_parse_ftie(fbt_info, cfg, data->ie, data->ie_len)) {
				ftpparm->assocresp.status = DOT11_SC_FAILURE;
			}
			break;
		case FC_REASSOC_RESP: {
			uint8 *tlvs;
			uint tlvs_len;
			wlc_assoc_t *as = cfg->assoc;
			uint8 *ricie = NULL;

			/* Check if RDE IE is present in reassoc response */
			tlvs = (uint8 *)&as->resp[1];
			tlvs_len = as->resp_len - DOT11_ASSOC_RESP_FIXED_LEN;
			ricie = (uint8*)bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RDE_ID);
			if (ricie != NULL) {
				/* RDE IE is present, save FT IE to do MIC calculation
				 * while processing RDE IE.
				 */
				ftpparm->assocresp.ft_ie = data->ie;
			}
			else {
				/* RDE IE is absent. Do the MIC calculation here without RDE */
				if (!wlc_fbt_reassoc_validate_ftie(fbt_info, cfg, NULL, 0,
					(dot11_ft_ie_t *)data->ie,
					(bcm_tlv_t *)ftpparm->assocresp.md_ie,
					(bcm_tlv_t *)ftpparm->assocresp.wpa2_ie)) {
					WL_ERROR(("wl%d: %s failed, ignoring reassoc response\n",
					   UNIT(fbt_priv), __FUNCTION__));
					ftpparm->assocresp.status = DOT11_SC_FAILURE;
				}
			}
			break;
		}
	    default:
			break;
		}
	}
	return BCME_OK;
}

static uint
wlc_fbt_calc_rde_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	uint len = 0;

	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (WLFBT_ENAB(fbt_priv->wlc->pub))
		len = DOT11_MNG_RDE_IE_LEN;
	return len;
}

/* Write RDE IE in reassoc req */
static int
wlc_fbt_write_rde_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (WLFBT_ENAB(fbt_priv->wlc->pub)) {
		dot11_rde_ie_t* rdptr;
		ftcbparm->fbtric.rde_count += 1;
		/* RDE. */
		rdptr = (dot11_rde_ie_t*)data->buf;
		rdptr->id = DOT11_MNG_RDE_ID;
		rdptr->length = DOT11_MNG_RDE_IE_LEN - TLV_HDR_LEN;
		rdptr->rde_id = ftcbparm->fbtric.rde_count;
		rdptr->rd_count = 1;
		rdptr->status = htol16(0);	/* Always 0 for STA. */
	}
	return BCME_OK;
}

static int
wlc_fbt_parse_rde_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_info_t *wlc;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
		(data->ft == FC_REASSOC_RESP)) {
		bool accepted;
		wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
		ASSERT(ftpparm != NULL);

		/* Do the MIC calculation to validate FT IE when RDE IE is present */
		if (wlc_fbt_reassoc_validate_ftie(fbt_info, cfg, data->ie,
			data->ie_len, (dot11_ft_ie_t *)ftpparm->assocresp.ft_ie,
			(bcm_tlv_t *)ftpparm->assocresp.md_ie,
			(bcm_tlv_t *)ftpparm->assocresp.wpa2_ie)) {
			/* Process any RIC responses. */
			if (CAC_ENAB(wlc->pub)) {
				accepted = wlc_fbt_reassoc_parse_rdeie(fbt_info,
				        cfg, data->ie, data->ie_len);
				if (accepted == FALSE) {
					WL_ERROR(("wl%d: %s: RIC request denied\n",
					   wlc->pub->unit, __FUNCTION__));
					ftpparm->assocresp.status = DOT11_SC_FAILURE;
				}
			}
		} else {
			WL_ERROR(("wl%d: %s failed, ignoring reassoc response.\n",
			   wlc->pub->unit, __FUNCTION__));
			ftpparm->assocresp.status = DOT11_SC_FAILURE;
		}
	}
	return BCME_OK;
}

/* Over-the-DS FBT Request Action frame */
void *
wlc_fbt_send_overds_req(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, struct ether_addr *ea,
	struct scb *scb, bool short_preamble)
{
	wlc_info_t *wlc;
	wlc_txq_info_t *qi;
	void *p = NULL;
	uint8 *pbody;
	dot11_ft_req_t *ft_req;
	uint16 fc;
	int body_len;
	wlc_bsscfg_t *parent;
	wlc_fbt_priv_t *fbt_priv;

	if (scb) {
	  parent = SCB_BSSCFG(scb);
	}
	else {
	  ASSERT(0);   /* Force debug images to crash to assist debugging */
	  return NULL; /* Error handing for release images */
	}

	if (!fbt_info)
		return NULL;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	ASSERT(cfg != NULL);

	/* assert:  (status == success) -> (scb is not NULL) */
	ASSERT(scb != NULL);

	/* calc IEs' length */
	body_len = DOT11_FT_REQ_FIXED_LEN;

	/* get a packet */
	fc = FC_ACTION;
	body_len += wlc_ier_calc_len(wlc->ier_fbt, parent, FC_ACTION, NULL);

	if ((p = wlc_frame_get_mgmt(wlc, fc, &cfg->BSSID, &cfg->cur_etheraddr,
	                            &cfg->BSSID, body_len, &pbody)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_frame_get_mgmt failed\n",
		          wlc->pub->unit, __FUNCTION__));
#ifdef STA
#endif /* STA */
		return NULL;
	}

	ft_req = (dot11_ft_req_t*)pbody;
	ft_req->category = DOT11_ACTION_CAT_FBT;
	ft_req->action = DOT11_FT_ACTION_FT_REQ;
	bcopy(&cfg->cur_etheraddr, ft_req->sta_addr, ETHER_ADDR_LEN);
	bcopy(&cfg->target_bss->BSSID, ft_req->tgt_ap_addr, ETHER_ADDR_LEN);

	pbody += DOT11_FT_REQ_FIXED_LEN;
	body_len -= DOT11_FT_REQ_FIXED_LEN;

	/* build IEs */
	if (wlc_ier_build_frame(wlc->ier_fbt, parent, FC_ACTION, NULL,
	                        pbody, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		PKTFREE(wlc->osh, p, TRUE);
		return NULL;
	}

	/*
	  Send using the bsscfg queue so that the FT Req will go out on the current
	  associated channel, not the roam target channel.
	 */
	qi = cfg->wlcif->qi;

	if (wlc_queue_80211_frag(wlc, p, qi, scb, scb->bsscfg,
		short_preamble, NULL, WLC_LOWEST_SCB_RSPEC(scb)))
		return p;

	return NULL;
}

static void
wlc_fbt_auth_nhdlr_cb(void *ctx, wlc_iem_nhdlr_data_t *data)
{
	if (WL_INFORM_ON()) {
		printf("%s: no parser\n", __FUNCTION__);
		prhex("IE", data->ie, data->ie_len);
	}
}

static uint8
wlc_fbt_auth_vsie_cb(void *ctx, wlc_iem_pvsie_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;

	return wlc_iem_vs_get_id(wlc->iemi, data->ie);
}

/* FT Response Action frame */
void
wlc_fbt_recv_overds_resp(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
     struct dot11_management_header *hdr, uint8 *body, uint body_len)
{
	wlc_info_t *wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	uint16 resp_status;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&hdr->sa, eabuf);
#endif /* BCMDBG_ERR */
	dot11_ft_res_t* ft_resp = (dot11_ft_res_t*)body;
	struct scb *scb;
	bool ft_band_changed = TRUE;
	wlc_iem_upp_t upp;
	wlc_iem_ft_pparm_t ftpparm;
	wlc_iem_pparm_t pparm;
	wlc_fbt_priv_t *fbt_priv;

	if (!fbt_info)
		return;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	/* Is this request for an AP config or a STA config? */
	if (!BSSCFG_STA(cfg) || (!cfg->BSS)) {
		WL_ASSOC(("wl%d: %s: FC_ACTION FT Response: unknown bsscfg _ap %d"
			" bsscfg->BSS %d\n", WLCWLUNIT(wlc), __FUNCTION__,
			cfg->_ap, cfg->BSS));
		return;  /* We couldn't match the incoming frame to a BSS config */
	}

	/* ignore ft_resp frames from other stations */
	if ((ft_resp->action != DOT11_FT_ACTION_FT_RES) ||
		(as->state != AS_SENT_FTREQ) ||
		bcmp((char*)&hdr->sa, (char*)&cfg->current_bss->BSSID, ETHER_ADDR_LEN) ||
		bcmp((char*)&ft_resp->tgt_ap_addr, (char*)&target_bss->BSSID, ETHER_ADDR_LEN) ||
		!wlc_scbfind(wlc, cfg, &cfg->current_bss->BSSID) ||
		!(scb = wlc_scbfind(wlc, cfg, &target_bss->BSSID))) {
		WL_ERROR(("wl%d.%d: unsolicited FT response from %s",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), sa));
		WL_ERROR((" for  %s\n", bcm_ether_ntoa((struct ether_addr *)&ft_resp->tgt_ap_addr,
			eabuf)));

		wlc_auth_complete(cfg, WLC_E_STATUS_UNSOLICITED, &target_bss->BSSID, 0, 0);
		return;
	}

	resp_status = ltoh16(ft_resp->status);

	/* authentication error */
	if (resp_status != DOT11_SC_SUCCESS) {
		wlc_auth_complete(cfg, WLC_E_STATUS_FAIL, &target_bss->BSSID, resp_status,
			DOT11_FAST_BSS);
		return;
	}

	body += DOT11_FT_RES_FIXED_LEN;
	body_len -= DOT11_FT_RES_FIXED_LEN;

	/* prepare IE mgmt calls */
	bzero(&upp, sizeof(upp));
	upp.notif_fn = wlc_fbt_auth_nhdlr_cb;
	upp.vsie_fn = wlc_fbt_auth_vsie_cb;
	upp.ctx = wlc;
	bzero(&ftpparm, sizeof(ftpparm));
	ftpparm.auth.status = (uint16)resp_status;
	bzero(&pparm, sizeof(pparm));
	pparm.ft = &ftpparm;

	/* parse IEs */
	if (wlc_ier_parse_frame(wlc->ier_fbt, cfg, FC_ACTION, &upp, &pparm,
	                        body, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_parse_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return;
	}
	resp_status = ftpparm.auth.status;

	if (resp_status != DOT11_SC_SUCCESS) {
		wlc_auth_complete(cfg, WLC_E_STATUS_FAIL, &target_bss->BSSID, resp_status,
			DOT11_FAST_BSS);
		return;
	}

	{
		/* authentication success */
		wlcband_t *band;
		chanspec_t chanspec = target_bss->chanspec;
		bool switch_scb_band;

		/* Set switch_scb_band before wlc_set_chanspec has a chance to change
		 * wlc->band->bandunit.
		 */
		switch_scb_band = wlc->band->bandunit != CHSPEC_WLCBANDUNIT(chanspec);

		band = wlc->bandstate[CHSPEC_IS2G(chanspec) ? BAND_2G_INDEX : BAND_5G_INDEX];

		/* Sanitize user setting for 8080MHz against current settings
		 * Reduce an 8080MHz chanspec to 80MHz if needed.
		 */
		if (CHSPEC_IS8080_UNCOND(chanspec) &&
		    (!VHT_ENAB_BAND(wlc->pub, band->bandtype) ||
		     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_160MHZ) ||
		     !WL_BW_CAP_160MHZ(band->bw_cap))) {
			/* select the 80MHz primary channel in case 80 is allowed */
			chanspec = wf_chspec_primary80_chspec(chanspec);
		}

		/* Sanitize user setting for 160MHz against current settings
		 * Reduce an 160MHz chanspec to 80MHz if needed.
		 */
		if (CHSPEC_IS160_UNCOND(chanspec) &&
		    (!VHT_ENAB_BAND(wlc->pub, band->bandtype) ||
		     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_160MHZ) ||
		     !WL_BW_CAP_160MHZ(band->bw_cap))) {
			/* select the 80MHz primary channel in case 80 is allowed */
			chanspec = wf_chspec_primary80_chspec(chanspec);
		}

		/* Sanitize user setting for 80MHz against current settings
		 * Reduce an 80MHz chanspec to 40MHz if needed.
		 */
		if (CHSPEC_IS80_UNCOND(chanspec) &&
		    (!VHT_ENAB_BAND(wlc->pub, band->bandtype) ||
		     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_80MHZ) ||
		     !WL_BW_CAP_80MHZ(band->bw_cap))) {
			/* select the 40MHz primary channel in case 40 is allowed */
			chanspec = wf_chspec_primary40_chspec(chanspec);
		}

		/* Check if we're going to change bands */
		if (CHSPEC_BAND(cfg->current_bss->chanspec) == CHSPEC_BAND(chanspec))
			ft_band_changed = FALSE;

		/* Convert a 40MHz AP channel to a 20MHz channel if we are not in NMODE or
		 * the locale does not allow 40MHz
		 * or the band is not configured for 40MHz operation
		 * Note that the unconditional version of the CHSPEC_IS40 is used so that
		 * code compiled without MIMO support will still recognize and convert
		 * a 40MHz chanspec.
		 */
		if (CHSPEC_IS40_UNCOND(chanspec) &&
			(!N_ENAB(wlc->pub) ||
			(wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) &
			WLC_NO_40MHZ) ||
		         !WL_BW_CAP_40MHZ(band->bw_cap) ||
			(BAND_2G(band->bandtype) && WLC_INTOL40_DET(wlc, cfg)))) {
			uint channel;
			channel = wf_chspec_ctlchan(chanspec);
			chanspec = CH20MHZ_CHSPEC(channel);
		}

		/* Change the radio channel to match the target_bss */
		if ((WLC_BAND_PI_RADIO_CHANSPEC != chanspec)) {
			/* clear the quiet bit on the dest channel */
			wlc_clr_quiet_chanspec(wlc->cmi, chanspec);
			wlc_suspend_mac_and_wait(wlc);
			wlc_set_chanspec(wlc, chanspec);
#ifdef WLMCHAN
			if (MCHAN_ENAB(wlc->pub)) {
				wlc_mchan_set_priq(wlc->mchan, cfg);
			}
#endif
			wlc_enable_mac(wlc);
		}

		wlc_rate_lookup_init(wlc, &target_bss->rateset);

		{wlc_phy_t *pi = WLC_PI(wlc);
		if (WLCISACPHY(wlc->band)) {
			wlc_full_phy_cal(wlc, cfg, PHY_PERICAL_JOIN_BSS);
			wlc_phy_interference_set(pi, TRUE);
			wlc_phy_acimode_noisemode_reset(pi,
				chanspec, FALSE, TRUE, FALSE);
		}
		else if ((WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band)) &&
			ft_band_changed) {
			wlc_full_phy_cal(wlc, cfg, PHY_PERICAL_JOIN_BSS);
			wlc_phy_interference_set(pi, TRUE);
			wlc_phy_acimode_noisemode_reset(pi,
				chanspec, FALSE, TRUE, FALSE);
		}}
		/* The scb for target_bss was created in wlc_join_BSS on the channel for the
		 * current_bss. We may need to switch the target_bss scb to the new band if we've
		 * successfully performed an FT over-the-DS reassoc.
		 */
		if (switch_scb_band) {
			wlc_scb_switch_band(wlc, scb, wlc->band->bandunit, cfg);
			wlc_rateset_filter(&wlc->band->hw_rateset /* src */,
				&scb->rateset /* dst */, FALSE,
				WLC_RATES_CCK_OFDM, RATE_MASK, BSS_N_ENAB(wlc, scb->bsscfg));
			wlc_scb_ratesel_init(wlc, scb);
		}
	}
	wlc_auth_complete(cfg, WLC_E_STATUS_SUCCESS, &target_bss->BSSID, resp_status,
		DOT11_FAST_BSS);

}

/* Save the akm type being used for the current association */
void
wlc_fbt_save_current_akm(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg,
	const wlc_bss_info_t *bi)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED) {
		if ((cfg->WPA_auth & WPA2_AUTH_FT) && (bi->wpa2.flags & RSN_FLAGS_FBT))
			fbt_bss_priv->current_akm_type = RSN_AKM_FBT_1X;
		else
			fbt_bss_priv->current_akm_type = RSN_AKM_UNSPECIFIED;
	}
	else if (cfg->WPA_auth & WPA2_AUTH_PSK) {
		if ((cfg->WPA_auth & WPA2_AUTH_FT) && (bi->wpa2.flags & RSN_FLAGS_FBT))
			fbt_bss_priv->current_akm_type = RSN_AKM_FBT_PSK;
		 else
			fbt_bss_priv->current_akm_type = RSN_AKM_PSK;
	}
}

/* Reset the current state of the variable */
void
wlc_fbt_reset_current_akm(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!(cfg->WPA_auth & WPA2_AUTH_FT))
		return;

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (fbt_bss_priv)
		fbt_bss_priv->current_akm_type = RSN_AKM_NONE;

	return;
}

/* Fast transition allowed only if the target AP supports the same FT AKM
 * suites as the currently associated AP.
 */
bool
wlc_fbt_is_fast_reassoc(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, wlc_bss_info_t *bi)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!(cfg->WPA_auth & WPA2_AUTH_FT))
		return FALSE;

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (!wlc_fbt_is_cur_mdid(fbt_info, cfg, bi))
		return FALSE;

	if (((cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED) &&
		(fbt_bss_priv->current_akm_type == RSN_AKM_FBT_1X)) ||
		((cfg->WPA_auth & WPA2_AUTH_PSK) &&
		(fbt_bss_priv->current_akm_type == RSN_AKM_FBT_PSK))) {
		WL_WSEC(("wl%d: JOIN: Fast bss transition\n", WLCWLUNIT(fbt_bss_priv->wlc)));
		return TRUE;
	}
	else
		return FALSE;
}

/* Check if the AKM selected for the current bss is supported by the target AP */
bool
wlc_fbt_akm_match(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg, const wlc_bss_info_t *bi)
{
	wlc_fbt_priv_t *fbt_priv;
	bool akm_match = FALSE;
	int i;

	if (!fbt_info)
		return FALSE;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (bcmwpa_is_wpa2_auth(cfg->WPA_auth) && (bi->wpa2.flags & RSN_FLAGS_SUPPORTED)) {
		if (WLFBT_ENAB(fbt_priv->wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
			for (i = 0; i < bi->wpa2.acount && (akm_match == FALSE); i++) {
				if (((bi->wpa2.auth[i] == RSN_AKM_FBT_1X) &&
					(cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED)) ||
					((bi->wpa2.auth[i] == RSN_AKM_FBT_PSK) &&
					(cfg->WPA_auth & WPA2_AUTH_PSK))) {
					WL_ASSOC(("wl%d: JOIN: FBT AKM match\n",
						WLCWLUNIT(fbt_priv->wlc)));
					akm_match = TRUE;
				}
			}
		}
		else if (!(cfg->WPA_auth & WPA2_AUTH_FT)) {
			for (i = 0; i < bi->wpa2.acount && (akm_match == FALSE); i++) {
				if (((bi->wpa2.auth[i] == RSN_AKM_UNSPECIFIED) &&
					(cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED)) ||
					((bi->wpa2.auth[i] == RSN_AKM_PSK) &&
					(cfg->WPA_auth & WPA2_AUTH_PSK))) {
					WL_ASSOC(("wl%d: JOIN: WPA AKM match\n",
						WLCWLUNIT(fbt_priv->wlc)));
					akm_match = TRUE;
				}
#ifdef BCMCCX
				else if (CCX_ENAB(fbt_priv->wlc->pub) &&
					(cfg->WPA_auth & WPA2_AUTH_CCKM) &&
					((bi->wpa2.auth[i] == RSN_AKM_NONE) ||
					(bi->wpa2.auth[i] == RSN_AKM_UNSPECIFIED))) {
					WL_ASSOC(("wl%d: JOIN: CCKM AKM match\n",
						WLCWLUNIT(fbt_priv->wlc)));
					akm_match = TRUE;
				}
#endif /* BCMCCX */
			}
		}
	}
	return akm_match;
}

#ifdef BCMSUP_PSK
static
void wlc_fbt_sup_updown_callbk(void *ctx, sup_init_event_data_t *evt)
{
	if (evt->up)
		wlc_fbt_sup_init(ctx, evt);
	else
		wlc_fbt_sup_deinit(ctx, evt->bsscfg);
}

static
void wlc_fbt_sup_init(void *ctx, sup_init_event_data_t *evt)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	bss_fbt_priv_t *fbt_bss_priv = NULL;

	fbt_bss_priv = wlc_fbt_get_bsscfg(fbt_info, evt->bsscfg);
	if (fbt_bss_priv) {
		/* If sup comes up, free up FBT local wpa & wpa_info elements. */
		if (fbt_bss_priv->use_sup_wpa == FALSE) {
			wlc_fbt_wpa_free(fbt_bss_priv);
			fbt_bss_priv->use_sup_wpa = TRUE;
		}
		fbt_bss_priv->wpa = evt->wpa;
		fbt_bss_priv->wpa_info = evt->wpa_info;
	} else {
		wlc_fbt_sup_deinit(fbt_info, evt->bsscfg);
	}
}

static
void wlc_fbt_sup_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	bss_fbt_priv_t *fbt_bss_priv;

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	if (fbt_bss_priv->use_sup_wpa == TRUE) {
		/* wpa/wpa_info are owned by sup module so only remove fbt reference to
		 * them.
		 */
		fbt_bss_priv->wpa = NULL;
		fbt_bss_priv->wpa_info = NULL;
		fbt_bss_priv->use_sup_wpa = FALSE;
	}
}
#endif /* BCMSUP_PSK */

int
wlc_fbt_set_pmk(wlc_fbt_info_t *fbt_info, struct wlc_bsscfg *cfg, wsec_pmk_t *pmk, bool assoc)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss;

	if (fbt_info == NULL || cfg == NULL || pmk == NULL) {
		WL_ERROR(("%s: missing fbt_info or bsscfg or pmk\n", __FUNCTION__));
		return BCME_BADARG;
	}

	fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	if (fbt_bss == NULL) {
		WL_ERROR(("%s: fbt_bss cubby not set\n", __FUNCTION__));
		return BCME_BADARG;
	}

	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	ASSERT(fbt_bss_priv);

	return wlc_set_pmk(fbt_bss_priv->wlc, fbt_bss_priv->wpa_info, fbt_bss_priv->wpa, cfg, pmk,
		assoc);
}

void
wlc_fbt_get_kck_kek(wlc_fbt_info_t *fbt_info, struct wlc_bsscfg *cfg, uint8 *key)
{
	bss_fbt_priv_t *fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	ASSERT(fbt_bss_priv && fbt_bss_priv->wpa && key);
	memcpy(key, fbt_bss_priv->wpa->eapol_mic_key, WPA_MIC_KEY_LEN);
	memcpy(key + WPA_MIC_KEY_LEN, fbt_bss_priv->wpa->eapol_encr_key, WPA_ENCR_KEY_LEN);
}
