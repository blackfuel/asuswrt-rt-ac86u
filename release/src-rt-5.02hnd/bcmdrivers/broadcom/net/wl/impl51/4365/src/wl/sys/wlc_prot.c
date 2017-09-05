/*
 * 11g/n shared protection module source
 * Broadcom 802.11abg Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_prot.c 467328 2014-04-03 01:23:40Z $
 */


#include <wlc_cfg.h>

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_dbg.h>
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif
#include <wlc_scb.h>
#include <wlc_prot.h>
#include <wlc_obss.h>

/* module private states are defined in the private header */
#include <wlc_prot_priv.h>

/* ioctl table */
static const wlc_ioctl_cmd_t prot_ioctls[] = {
	{WLC_GET_PROTECTION_CONTROL, 0, 0},
	{WLC_SET_PROTECTION_CONTROL, 0, 0}
};

/* wlc_prot_info_priv_t offset in module states struct */
uint16 wlc_prot_info_priv_offset = sizeof(wlc_prot_info_t);

/* local functions */

/* module entries */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_prot_dump(void *ctx, struct bcmstrbuf *b);
#endif
static int wlc_prot_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif);

/* bsscfg cubby */
static int wlc_prot_bss_init(void *ctx, wlc_bsscfg_t *cfg);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void wlc_prot_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_prot_bss_dump NULL
#endif

/* protection module code starts here... */

/* module entries */
wlc_prot_info_t *
BCMATTACHFN(wlc_prot_attach)(wlc_info_t *wlc)
{
	wlc_prot_info_t *prot;
	wlc_prot_info_priv_t *priv;

	/* sanity check */
	ASSERT(wlc != NULL);

	/* allocate module states */
	if ((prot = MALLOCZ(wlc->osh, WLC_PROT_SIZE)) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	wlc_prot_info_priv_offset = OFFSETOF(wlc_prot_t, priv);
	priv = WLC_PROT_INFO_PRIV(prot);
	priv->wlc = wlc;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((prot->cfgh = wlc_bsscfg_cubby_reserve(wlc, BSS_PROT_SIZE,
	                wlc_prot_bss_init, NULL, wlc_prot_bss_dump,
	                prot)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	priv->cfg_offset = OFFSETOF(bss_prot_t, priv);

	if (wlc_module_add_ioctl_fn(wlc->pub, prot, wlc_prot_doioctl,
	                            ARRAYSIZE(prot_ioctls), prot_ioctls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_add_ioctl_fn() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "prot", wlc_prot_dump, (void *)prot);
#endif

	return prot;

	/* error handling */
fail:
	wlc_prot_detach(prot);
	return NULL;
}

void
BCMATTACHFN(wlc_prot_detach)(wlc_prot_info_t *prot)
{
	wlc_prot_info_priv_t *priv;
	wlc_info_t *wlc;

	if (prot == NULL)
		return;

	priv = WLC_PROT_INFO_PRIV(prot);
	wlc = priv->wlc;

	wlc_module_remove_ioctl_fn(wlc->pub, prot);

	MFREE(wlc->osh, prot, WLC_PROT_SIZE);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_prot_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_prot_info_t *prot = (wlc_prot_info_t *)ctx;
	wlc_prot_info_priv_t *priv = WLC_PROT_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	int idx;
	wlc_bsscfg_t *cfg;

	bcm_bprintf(b, "prot: priv_offset %d cfgh %d\n",
	            wlc_prot_info_priv_offset, prot->cfgh);

	FOREACH_AS_BSS(wlc, idx, cfg) {
		bcm_bprintf(b, "bsscfg %d >\n", WLC_BSSCFG_IDX(cfg));
	        wlc_prot_bss_dump(prot, cfg, b);
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

static int
wlc_prot_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	wlc_prot_info_t *prot = (wlc_prot_info_t *)ctx;
	wlc_prot_info_priv_t *priv = WLC_PROT_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	int val = 0, *pval;
	int err = BCME_OK;
	wlc_bsscfg_t *cfg;
	bss_prot_cfg_t *pc;

	/* default argument is generic integer */
	pval = (int *)arg;

	/* This will prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));

	/* update bsscfg pointer */
	cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(cfg != NULL);

	pc = BSS_PROT_CFG(prot, cfg);
	ASSERT(pc != NULL);

	switch (cmd) {
	case WLC_GET_PROTECTION_CONTROL:
		*pval = pc->overlap;
		break;

	case WLC_SET_PROTECTION_CONTROL:
		if ((val != WLC_PROTECTION_CTL_OFF) &&
		    (val != WLC_PROTECTION_CTL_LOCAL) &&
		    (val != WLC_PROTECTION_CTL_OVERLAP)) {
			err = BCME_RANGE;
			break;
		}
		wlc_prot_cfg_upd(prot, cfg, WLC_PROT_OVERLAP, val);

		/* Current g_protection will sync up to the specified control alg in watchdog
		 * if the driver is up and associated.
		 * If the driver is down or not associated, the control setting has no effect.
		 */
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* bsscfg cubby */
static int
wlc_prot_bss_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_prot_info_t *prot = (wlc_prot_info_t *)ctx;

	/* initialize CCK preamble mode to unassociated state */
	wlc_prot_cfg_upd(prot, cfg, WLC_PROT_SHORTPREAMBLE, FALSE);
	wlc_prot_cfg_upd(prot, cfg, WLC_PROT_OVERLAP, WLC_PROTECTION_CTL_OVERLAP);

	return BCME_OK;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_prot_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_prot_info_t *prot = (wlc_prot_info_t *)ctx;
	wlc_prot_info_priv_t *priv = WLC_PROT_INFO_PRIV(prot);
	wlc_prot_cfg_t *wpc;
	bss_prot_cfg_t *pc;

	ASSERT(cfg != NULL);

	wpc = WLC_PROT_CFG(prot, cfg);
	ASSERT(wpc != NULL);

	pc = BSS_PROT_CFG(prot, cfg);
	ASSERT(pc != NULL);

	bcm_bprintf(b, "\tcfg_offset %d\n", priv->cfg_offset);
	bcm_bprintf(b, "\toverlap %d shortpreamble %d\n",
	            pc->overlap, wpc->shortpreamble);
}
#endif /* BCMDBG || BCMDBG_DUMP */

/* centralized protection config change function to simplify debugging, no consistency checking
 * this should be called only on changes to avoid overhead in periodic function
 */
void
wlc_prot_cfg_upd(wlc_prot_info_t *prot, wlc_bsscfg_t *cfg, uint idx, int val)
{
	wlc_prot_info_priv_t *priv;
	wlc_prot_cfg_t *wpc;
	bss_prot_cfg_t *pc;

	ASSERT(cfg != NULL);

	wpc = WLC_PROT_CFG(prot, cfg);
	ASSERT(wpc != NULL);

	pc = BSS_PROT_CFG(prot, cfg);
	ASSERT(pc != NULL);

	priv = WLC_PROT_INFO_PRIV(prot);

	WL_PROT(("wl%d.%d %s: idx %d, val %d\n",
	         priv->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, idx, val));
	BCM_REFERENCE(priv);

	switch (idx) {
	case WLC_PROT_OVERLAP:
		pc->overlap = (int8)val;
		break;
	case WLC_PROT_SHORTPREAMBLE:
		wpc->shortpreamble = (bool)val;
		break;

	default:
		ASSERT(0);
		break;
	}
}

void
wlc_prot_cfg_init(wlc_prot_info_t *prot, wlc_bsscfg_t *cfg)
{
	/* unassociated CCK preamble mode */
	wlc_prot_cfg_upd(prot, cfg, WLC_PROT_SHORTPREAMBLE, FALSE);
}

void
wlc_prot_init(wlc_prot_info_t *prot, wlc_bsscfg_t *cfg)
{
	uint16 cap;

	ASSERT(cfg != NULL);

	cap = cfg->current_bss->capability;

	/* update the CCK preamble mode for STAs joining an AP */
	if (BSSCFG_STA(cfg) && cfg->BSS)
		wlc_prot_cfg_upd(prot, cfg, WLC_PROT_SHORTPREAMBLE, (cap & DOT11_CAP_SHORT) != 0);
}

/* ======= shared code between different protection modules ======== */

/* protection condition owner is the BSS who caused the condition being set.
 * non condition owner is the BSS whose condition set is a result of the condition propagation
 * from a condition owner.
 */
#define BSS_PROT_COND_OWNER	0x80	/* protection condition owner when condition is set */

void
wlc_prot_cond_set(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint coff, bool set,
	uint8 *(*cb)(void *prot, wlc_bsscfg_t *cfg, uint coff), void *prot)
{
	uint8 *pd = cb(prot, cfg, coff);
	uint8 *pd2;
	int i;
	wlc_bsscfg_t *bc;

	ASSERT(cfg != NULL);
	ASSERT(cb != NULL);

	pd = cb(prot, cfg, coff);

	/* set: propagate to all adjacent BSSs */
	if (set) {
		*pd = (BSS_PROT_COND_OWNER | 1);

		FOREACH_UP_AP(wlc, i, bc) {
			if (bc == cfg)
				continue;
#ifdef WLMCHAN
			if (MCHAN_ENAB(wlc->pub) &&
			    !wlc_mchan_ovlp_chan(wlc->mchan, bc, cfg, CH_20MHZ_APART))
				continue;
#endif
			pd2 = cb(prot, bc, coff);
			*pd2 |= 1;
		}
	}
	/* clear: propagate to all adjacent BSSs */
	else if (*pd & BSS_PROT_COND_OWNER) {
		*pd = 0;

		FOREACH_UP_AP(wlc, i, bc) {
			if (bc == cfg)
				continue;
#ifdef WLMCHAN
			if (MCHAN_ENAB(wlc->pub) &&
			    !wlc_mchan_ovlp_chan(wlc->mchan, bc, cfg, CH_20MHZ_APART))
				continue;
#endif
			pd2 = cb(prot, bc, coff);
			*pd2 &= ~1;
		}
	}

	/* clear: re-set adjacent BSSs for other owners. */
	if (!set) {
		FOREACH_UP_AP(wlc, i, bc) {
			pd2 = cb(prot, bc, coff);
			if (*pd2 & BSS_PROT_COND_OWNER)
				wlc_prot_cond_set(wlc, bc, coff, TRUE, cb, prot);
		}
	}
}

/* return TRUE if there are any associated STAs with matching flag value, otherwise FALSE */
bool
wlc_prot_scb_scan(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint32 flagmask, uint32 flagvalue)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_ASSOCIATED(scb) &&
		    SCB_BSSCFG(scb) == cfg &&
		    (scb->flags & flagmask) == flagvalue)
			return TRUE;
	}

	return FALSE;
}
