/**
 * @file
 * @brief
 * MAC based SW probe response module source file -
 * It uses the MAC filter list to influence the decision about
 * which MAC to send SW probe response.
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */


#include <wlc_cfg.h>

#ifdef WLPROBRESP_MAC_FILTER

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_macfltr.h>
#include <wlc_bsscfg.h>
#include <wlc_probresp.h>
#include <wlc_probresp_mac_filter.h>

/* iovar table */
enum {
	IOV_PROB_RESP_MAC_FILTER = 1,
	IOV_LAST
};

static const bcm_iovar_t prb_iovars[] = {
	{"probresp_mac_filter", IOV_PROB_RESP_MAC_FILTER, (0), IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0}
};

/* module info */
struct wlc_probresp_mac_filter_info {
	wlc_info_t *wlc;
	int bsscfgh;
};

/* bsscfg private states */
typedef struct {
	bool	probresp_mac_filter_mode;
} bss_probresp_mac_filter_info_t;

/* macros to retrieve the pointer to module specific opaque data in bsscfg container */
#define BSS_PRBRSP_MAC_FILTER_CUBBY_LOC(probresp_mac_filter, cfg) \
	((bss_probresp_mac_filter_info_t **)BSSCFG_CUBBY(cfg, (probresp_mac_filter)->bsscfgh))
#define BSS_PRBRSP_MAC_FILTER_INFO(probresp_mac_filter, cfg) \
	(*BSS_PRBRSP_MAC_FILTER_CUBBY_LOC(probresp_mac_filter, cfg))


/* filter function */
static bool wlc_probresp_mac_filter_check_probe_req(void *handle, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr, uint8 *body,
	int body_len, bool *psendProbeResp);

/* IOVAR management */
static int
wlc_probresp_mac_filter_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
/* register dump routine */
static int wlc_probresp_mac_filter_dump(void *ctx, struct bcmstrbuf *b);
#endif

/* bss cubby */
static int wlc_probresp_mac_filter_bss_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_probresp_mac_filter_bss_deinit(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_probresp_mac_filter_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);

/* module entries */
void *
BCMATTACHFN(wlc_probresp_mac_filter_attach)(wlc_info_t *wlc)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter;

	if ((mprobresp_mac_filter = MALLOCZ(wlc->osh,
		sizeof(wlc_probresp_mac_filter_info_t))) == NULL)
	{
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n", wlc->pub->unit,
			__FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	mprobresp_mac_filter->wlc = wlc;

	/* reserve cubby in the bsscfg container for per-bsscfg data */
	if ((mprobresp_mac_filter->bsscfgh = wlc_bsscfg_cubby_reserve(wlc,
		sizeof(bss_probresp_mac_filter_info_t *), wlc_probresp_mac_filter_bss_init,
		wlc_probresp_mac_filter_bss_deinit, wlc_probresp_mac_filter_bss_dump,
		(void *)mprobresp_mac_filter)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n", wlc->pub->unit,
			__FUNCTION__));
		goto fail;
	}

	/* register module up/down, watchdog, and iovar callbacks */
	if (wlc_module_register(wlc->pub, prb_iovars, "probresp_mac_filter", mprobresp_mac_filter,
		wlc_probresp_mac_filter_doiovar, NULL, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n", wlc->pub->unit,
			__FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "probresp_mac_filter", wlc_probresp_mac_filter_dump,
		mprobresp_mac_filter);
#endif
	if (wlc_probresp_register(wlc->mprobresp, mprobresp_mac_filter,
		wlc_probresp_mac_filter_check_probe_req, FALSE) != 0)
		goto fail;

	return mprobresp_mac_filter;

fail:
	wlc_probresp_mac_filter_detach(mprobresp_mac_filter);
	return NULL;
}

void
BCMATTACHFN(wlc_probresp_mac_filter_detach)(void *mprobresp_mac_filter)
{
	wlc_info_t *wlc;

	if (mprobresp_mac_filter == NULL)
		return;

	wlc = ((wlc_probresp_mac_filter_info_t *)mprobresp_mac_filter)->wlc;

	wlc_probresp_unregister(wlc->mprobresp, mprobresp_mac_filter);
	wlc_module_unregister(wlc->pub, "probresp_mac_filter", mprobresp_mac_filter);

	MFREE(wlc->osh, mprobresp_mac_filter, sizeof(wlc_probresp_mac_filter_info_t));
}

/* handle related iovars */
static int
wlc_probresp_mac_filter_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)ctx;
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	bss_probresp_mac_filter_info_t *bpi;
	wlc_bsscfg_t *bsscfg;
	int32 *ret_int_ptr;
	bool bool_val;
	int32 int_val = 0;
	int err = BCME_OK;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	bpi = BSS_PRBRSP_MAC_FILTER_INFO(mprobresp_mac_filter, bsscfg);
	ASSERT(bpi != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
	case IOV_GVAL(IOV_PROB_RESP_MAC_FILTER):
		*ret_int_ptr = bpi->probresp_mac_filter_mode;
		break;

	case IOV_SVAL(IOV_PROB_RESP_MAC_FILTER):
		bpi->probresp_mac_filter_mode = bool_val;
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_probresp_mac_filter_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)ctx;

	bcm_bprintf(b, "bsscfgh %d \n", mprobresp_mac_filter->bsscfgh);

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

static bool
wlc_probresp_mac_filter_check_probe_req(void *handle, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr,
	uint8 *body, int body_len, bool *psendProbeResp)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)handle;
	bss_probresp_mac_filter_info_t *bpi = BSS_PRBRSP_MAC_FILTER_INFO(mprobresp_mac_filter, cfg);
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	int addr_match;

	if (bpi->probresp_mac_filter_mode == 0)
		return TRUE;

	addr_match = wlc_macfltr_addr_match(wlc->macfltr, cfg, &hdr->sa);
	if (addr_match == WLC_MACFLTR_ADDR_DENY ||
		addr_match == WLC_MACFLTR_ADDR_NOT_ALLOW) {
		return FALSE;
	}
	return TRUE;
}

static int
wlc_probresp_mac_filter_bss_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)ctx;
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	bss_probresp_mac_filter_info_t **pbpi =
		BSS_PRBRSP_MAC_FILTER_CUBBY_LOC(mprobresp_mac_filter, cfg);
	bss_probresp_mac_filter_info_t *bpi;

	if (!(bpi = (bss_probresp_mac_filter_info_t *)MALLOCZ(wlc->osh,
		sizeof(bss_probresp_mac_filter_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NOMEM;
	}

	*pbpi = bpi;
	return BCME_OK;
}

static void
wlc_probresp_mac_filter_bss_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)ctx;
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	bss_probresp_mac_filter_info_t **pbpi =
		BSS_PRBRSP_MAC_FILTER_CUBBY_LOC(mprobresp_mac_filter, cfg);
	bss_probresp_mac_filter_info_t *bpi;
	if (pbpi) {
		bpi = *pbpi;
		if (bpi) {
			MFREE(wlc->osh, bpi, sizeof(bss_probresp_mac_filter_info_t));
			*pbpi = NULL;
		}
	}
}

static void
wlc_probresp_mac_filter_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)ctx;
	bss_probresp_mac_filter_info_t *bpi = BSS_PRBRSP_MAC_FILTER_INFO(mprobresp_mac_filter, cfg);

	ASSERT(cfg != NULL);

	if (bpi == NULL)
		return;

	bcm_bprintf(b, "\tprobresp_mac_filter_mode %u\n", bpi->probresp_mac_filter_mode);
}
#endif /* WLPROBRESP_MAC_FILTER */
