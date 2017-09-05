/*
 * 11n protection module APIs
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
 * $Id: wlc_prot_n.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wlc_prot_n_h_
#define _wlc_prot_n_h_

/* data APIs */
/* module specific states - read only */
struct wlc_prot_n_info {
	int cfgh;			/* bsscfg cubby handle */
	int8 n_pam_override;
};
#define WLC_PROT_N_INFO_CFGH(prot)	((prot)->cfgh)
#define WLC_PROT_N_INFO_PAM(prot)	((prot)->n_pam_override)

/* bsscfg specific states - read only */
typedef struct wlc_prot_n_cfg {
	int8 n_cfg;
	bool nongf;			/* non-GF present protection */
} wlc_prot_n_cfg_t;
#define WLC_PROT_N_CFG(prot, cfg)	(*(wlc_prot_n_cfg_t **) \
					 BSSCFG_CUBBY(cfg, WLC_PROT_N_INFO_CFGH(prot)))
#define WLC_PROT_N_CFG_N(prot, cfg)	(WLC_PROT_N_CFG(prot, cfg)->n_cfg)
#define WLC_PROT_N_CFG_NONGF(prot, cfg)	(WLC_PROT_N_CFG(prot, cfg)->nongf)

/* function APIs */
/* module entries */
extern wlc_prot_n_info_t *wlc_prot_n_attach(wlc_info_t *wlc);
extern void wlc_prot_n_detach(wlc_prot_n_info_t *prot);

/* configuration change */
extern void wlc_prot_n_cfg_set(wlc_prot_n_info_t *prot, uint idx, int val);
/* wlc_prot_n_cfg_set() idx */
#define	WLC_PROT_N_USER		1	/* gmode specified by the user */
#define	WLC_PROT_N_PAM_OVR	2	/* n preamble override */
extern void wlc_prot_n_cfg_init(wlc_prot_n_info_t *prot, wlc_bsscfg_t *cfg);

/* condition change */
extern void wlc_prot_n_cond_upd(wlc_prot_n_info_t *prot, struct scb *scb);

/* timeout change */
extern void wlc_prot_n_ovlp_upd(wlc_prot_n_info_t *prot, chanspec_t chspec,
	ht_add_ie_t *add_ie, ht_cap_ie_t *cap_ie, bool is_erp, bool bw40);
extern void wlc_prot_n_to_upd(wlc_prot_n_info_t *prot, wlc_bsscfg_t *cfg,
	ht_add_ie_t *add_ie, ht_cap_ie_t *cap_ie, bool is_erp, bool bw40);

/* nmode change */
extern void wlc_prot_n_mode_reset(wlc_prot_n_info_t *prot, bool force);
extern bool wlc_prot_n_mode_upd(wlc_prot_n_info_t *prot, wlc_bsscfg_t *cfg);

/* IE change */
extern void wlc_prot_n_cap_upd(wlc_prot_n_info_t *prot, wlc_bsscfg_t *cfg,
	ht_cap_ie_t *ht_cap);
extern void wlc_prot_n_build_add_ie(wlc_prot_n_info_t *prot, wlc_bsscfg_t *cfg,
	ht_add_ie_t *add_ie);

/* accessors */
extern bool wlc_prot_n_ht40int(wlc_prot_n_info_t *prot, wlc_bsscfg_t *cfg);

/* protection initialization */
extern void wlc_prot_n_init(wlc_prot_n_info_t *prot, wlc_bsscfg_t *cfg);

extern uint8
wlc_prot_n_get_non11n_apsd_assoc(wlc_prot_n_info_t *prot, wlc_bsscfg_t *cfg);


#ifdef _inc_wlc_prot_n_preamble_
#include <bcmdevs.h>
/* Apply all N-related overrides to resolve preamble type */
static INLINE uint8
wlc_prot_n_preamble(wlc_info_t *wlc, struct scb *scb)
{
	uint8 preamble_type = WLC_GF_PREAMBLE;
	wlc_bsscfg_t *cfg;
	wlc_prot_n_info_t *prot = wlc->prot_n;
	int8 pam;
	uint8 n;
	bool nongf;
#ifndef BCMCHIPID
	uint chip = wlc->pub->sih->chip;
#endif

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	pam = WLC_PROT_N_INFO_PAM(prot);
	n = WLC_PROT_N_CFG_N(prot, cfg);
	nongf = WLC_PROT_N_CFG_NONGF(prot, cfg);

	if ((n == WLC_N_PROTECTION_MIXEDMODE) ||
	    (n == WLC_N_PROTECTION_OPTIONAL)) {
		/* additional protection for legacy dev */
		preamble_type = WLC_MM_PREAMBLE;
	} else {
	}

	/* use mixed mode preamble if dest is not GF capable or nongf associated */
	if (SCB_NONGF_CAP(scb) || nongf)
		preamble_type = WLC_MM_PREAMBLE;

	if (((CHIPID(chip) == BCM4716_CHIP_ID) ||
	     (CHIPID(chip) == BCM4748_CHIP_ID) ||
	     (CHIPID(chip) == BCM47162_CHIP_ID) ||
	     (CHIPID(chip) == BCM5357_CHIP_ID) ||
	     (CHIPID(chip) == BCM53572_CHIP_ID) ||
	     (CHIPID(chip) == BCM43236_CHIP_ID) ||
	     (CHIPID(chip) == BCM4331_CHIP_ID) ||
	     (CHIPID(chip) == BCM43431_CHIP_ID)) &&
	    (pam == WLC_N_PREAMBLE_GF_BRCM)) {
		if (preamble_type == WLC_GF_PREAMBLE) {
			if (!(scb->flags & SCB_BRCM))
				preamble_type = WLC_MM_PREAMBLE;
		}
	}

	if (CHIPID(chip) == BCM4313_CHIP_ID) {
		if (preamble_type == WLC_GF_PREAMBLE) {
			preamble_type = WLC_MM_PREAMBLE;
		}
	}

	if ((pam != AUTO) &&
	    (pam != WLC_N_PREAMBLE_GF_BRCM)) {
		if (pam == WLC_N_PREAMBLE_GF)
			preamble_type = WLC_GF_PREAMBLE;
		else
			preamble_type = WLC_MM_PREAMBLE;
	}

	return preamble_type;
}
#endif /* _inc_wlc_prot_n_preamble_ */

#endif /* _wlc_prot_n_h_ */
