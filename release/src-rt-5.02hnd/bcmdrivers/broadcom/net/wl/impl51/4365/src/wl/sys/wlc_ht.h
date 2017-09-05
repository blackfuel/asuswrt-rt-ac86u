/*
 * Common (OS-independent) portion of
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
 * $Id: wlc_ht.h 624706 2016-03-14 07:16:25Z $
 */

/** 802.11n (High Throughput) */

#ifndef _wlc_ht_h_
#define _wlc_ht_h_
#include <wlc_types.h>

#define WLC_HT_WEP_RESTRICT	0x01 	/* restrict HT with WEP */
#define WLC_HT_TKIP_RESTRICT 0x02 	/* restrict HT with TKIP */

#define WLC_HT_FEATURES_PROPRATES_DISAB		0
#define WLC_HT_FEATURES_PROPRATES_ENAB		1
#define WLC_HT_FEATURES_PROPRATES_FORCE		2

#define HT_MCS_BIT6_SHIFT			6

#ifdef WL11N
#if defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED))
#define WL_HT_TXBW_OVERRIDE_ENAB 1
#endif

/* READ ONLY: Used in txpath, so performance sensitive... */
/* Therefore, use the following macros */

/* module entries */
extern wlc_ht_info_t *wlc_ht_attach(wlc_info_t *wlc);
extern void wlc_ht_detach(wlc_ht_info_t *hti);

/* API */
extern void
wlc_ht_mimops_handle_rxchain_update(wlc_ht_info_t *pub, uint8 mimops_mode);

extern void
wlc_ht_publicaction(wlc_ht_info_t *hti, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr,
	uint8 *body, int body_len, wlc_d11rxhdr_t *wrxh);

extern void
wlc_ht_update_scbstate(wlc_ht_info_t *hti, struct scb *scb,
	ht_cap_ie_t *cap_ie, ht_add_ie_t *add_ie, obss_params_t *obss_ie);

extern void wlc_ht_init_defaults(wlc_ht_info_t *hti);
extern void wlc_ht_nvm_overrides(wlc_ht_info_t *hti, uint n_disabled);

extern void wlc_frameaction_ht(wlc_ht_info_t *hti, uint action_id, struct scb *scb,
	struct dot11_management_header *hdr, uint8 *body, int body_len);

extern uint
wlc_ht_calc_frame_len(wlc_ht_info_t *hti, ratespec_t ratespec, uint8 preamble_type,
	uint dur);

extern uint
wlc_ht_calc_frame_time(wlc_ht_info_t *hti, ratespec_t ratespec, uint8 preamble_type,
	uint len);

extern uint
wlc_calc_ba_time(wlc_ht_info_t *hti, ratespec_t rate, uint8 preamble_type);

extern void
wlc_ht_update_rifs_mode(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg);

extern void
wlc_ht_update_txburst_limit(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg);

/* Get/Set functions */
/* For agg modules */
extern void
wlc_ht_get_scb_ampdu_params(wlc_ht_info_t *hti, struct scb *scb,
	uint8* peer_density, uint32* max_rxlen, uint8* max_rxlen_factor);

extern uint16
wlc_ht_get_scb_amsdu_mtu_pref(wlc_ht_info_t *hti, struct scb *scb);

extern bool
wlc_ht_is_scb_40MHZ_cap(wlc_ht_info_t *hti, struct scb *scb);

/* Do all that's necessary to shift from one nmode to another */
extern int
wlc_set_nmode(wlc_ht_info_t *hti, int32 nmode);

extern void
wlc_ht_update_ampdu_rx_cap_params(wlc_ht_info_t *pub,
	uint8 rx_factor, uint8 mpdu_density);

extern void
wlc_ht_fill_sta_fields(wlc_ht_info_t *pub, struct scb *scb, sta_info_t *sta);

extern bool
wlc_ht_is_40MHZ_cap(wlc_ht_info_t *pub);

extern void
wlc_ht_set_rx_stbc_cap(wlc_ht_info_t *hti, int val);

extern int8
wlc_ht_stbc_rx_get(wlc_ht_info_t *hti);

extern bool
wlc_ht_stbc_tx_set(wlc_ht_info_t *hti, int32 int_val);

extern void
wlc_ht_cap_enable_tx_stbc(wlc_ht_info_t *pub);

extern void
wlc_ht_checkadd_rifs_permitted(wlc_ht_info_t *hti, int8 n_cfg, uint8* byte1);
extern void
wlc_ht_set_add_ie_basic_mcs(wlc_ht_info_t *hti, uint8 *mcsset, int len);

extern uint8
wlc_ht_get_phy_membership(wlc_ht_info_t *hti);

extern int
wlc_ht_add_ie_verify_rates(wlc_ht_info_t *hti, uint8 *rates, int len);

extern uint8
wlc_ht_get_wsec_restriction(wlc_ht_info_t *hti);

extern uint8
wlc_ht_get_mimo_band_bwcap(wlc_ht_info_t *hti);

extern uint16
wlc_ht_get_cap(wlc_ht_info_t *hti);

/* For monitor code */
extern void
wlc_ht_monitor(wlc_ht_info_t *hti, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	ratespec_t rspec, struct wl_rxsts *sts);
extern void
wlc_ht_prep_rate_info(wlc_ht_info_t *hti, wlc_d11rxhdr_t *wrxh,
	ratespec_t rspec, struct wl_rxsts *sts);

#if defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED))
/* #define WL_HT_TXBW_OVERRIDE_ENAB 1 */
#define WL_HT_TXBW_OVERRIDE_IDX(hti, rspec, txbw_override_idx) \
	(txbw_override_idx) = -1; \
	if ((hti)->txbw_override) { \
		/* Take care of TXBW overrides */ \
		if (RSPEC_ISHT((rspec)) || RSPEC_ISVHT((rspec))) { \
			if ((hti)->mimo_40txbw != AUTO) { \
				(txbw_override_idx) = (hti)->mimo_40txbw; \
			} \
		} else if (IS_OFDM((rspec))) { \
			if ((hti)->ofdm_40txbw != AUTO) { \
				(txbw_override_idx) = (hti)->ofdm_40txbw; \
			} \
		} else { \
			ASSERT(IS_CCK((rspec))); \
			if ((hti)->cck_40txbw != AUTO) { \
				(txbw_override_idx) = (hti)->cck_40txbw; \
			} \
		} \
	}

#define WLC_HT_GET_MIMO_40TXBW(hti) ((hti)->mimo_40txbw)
#define WLC_HT_GET_OFDM_40TXBW(hti) ((hti)->ofdm_40txbw)
#define WLC_HT_GET_CCK_40TXBW(hti) ((hti)->cck_40txbw)

#else
#define WL_HT_TXBW_OVERRIDE_ENAB 0
#define WLC_HT_GET_MIMO_40TXBW(hti) (AUTO)
#define WLC_HT_GET_OFDM_40TXBW(hti) (AUTO)
#define WLC_HT_GET_CCK_40TXBW(hti) (AUTO)
#define WL_HT_TXBW_OVERRIDE_IDX (-1)
#endif /* defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED)) */

/* READ ONLY: Used in txpath, so performance sensitive... */
/* Therefore, use the following macros */
struct wlc_ht_info {
	/* fields are READ ONLY */
	int scbh;
	int bssh;
	int8		cck_40txbw; 	/* 11N, cck tx b/w override when in 40MHZ mode */
	int8		ofdm_40txbw;	/* 11N, ofdm tx b/w override when in 40MHZ mode */
	int8		mimo_40txbw;	/* 11N, mimo tx b/w override when in 40MHZ mode */
	int8		txbw_override;	/* TRUE iff the above three fields are non-AUTO */
	bool		frameburst;		/* enable per-packet framebursting */
	int8		sgi_tx;			/* sgi tx */
	int8		txburst_limit_override; /* tx burst limit override */
	bool		_rifs;			/* enable per-packet rifs */
	bool		ampdu_rts;		/* use RTS for AMPDU */

	/* above are READ ONLY */
};

#define WLC_HT_GET_TXBURST_LIMIT_OVERRIDE(hti) ((hti)->txburst_limit_override)
#define WLC_HT_GET_SGI_TX(hti) ((hti)->sgi_tx)
#define WLC_HT_GET_FRAMEBURST(hti) ((hti)->frameburst)
#define WLC_HT_GET_RIFS(hti) ((hti)->_rifs)
#define WLC_HT_GET_AMPDU_RTS(hti) ((hti)->ampdu_rts)

/* READ ONLY */
typedef struct wlc_ht_scb_info_pub {
	/* fields are READ ONLY */
	bool rts_enab; /* rts on or not */
	bool ht_mimops_enabled;	/* cached state: a mimo ps mode is enabled */
	bool ht_mimops_rtsmode;	/* cached state: TRUE=RTS mimo, FALSE=no mimo */
	/* above READ ONLY */
} wlc_ht_scb_info_pub_t;

#define WLC_HT_SCB(hti, scb)	((wlc_ht_scb_info_pub_t *) \
					 SCB_CUBBY((scb), (hti)->scbh))

#define WLC_HT_SCB_RTS_ENAB(hti, scb) (WLC_HT_SCB((hti), (scb))->rts_enab)
#define WLC_HT_GET_SCB_MIMOPS_ENAB(hti, scb) \
	(WLC_HT_SCB((hti), (scb))->ht_mimops_enabled)
#define WLC_HT_GET_SCB_MIMOPS_RTS_ENAB(hti, scb) \
	(WLC_HT_SCB((hti), (scb))->ht_mimops_rtsmode)

/* READ ONLY */
typedef struct wlc_ht_bss_info_pub {
	/* fields are READ ONLY */
	uint16		txburst_limit;	/* tx burst limit value */
	/* above READ ONLY */
} wlc_ht_bss_info_pub_t;
#define WLC_HT_BSS(hti, cfg)	((wlc_ht_bss_info_pub_t *) \
					 BSSCFG_CUBBY((cfg), (hti)->bssh))

#define WLC_HT_CFG_TXBURST_LIMIT(hti, cfg) \
	(WLC_HT_BSS((hti), (cfg))->txburst_limit)

#ifdef WLTXMONITOR
extern INLINE void
wlc_ht_txmon_agg_ft(wlc_ht_info_t *hti, void *p, struct dot11_header *h,
	uint8 frametype, struct wl_txsts *sts);

extern INLINE void
wlc_ht_txmon_htflags(wlc_ht_info_t *hti,
	uint16 phytxctl, uint16 phytxctl1, uint8 *plcp,
	uint16 chan_bw, uint16 chan_band, uint16 chan_num, ratespec_t *rspec,
	struct wl_txsts *sts);

extern INLINE void
wlc_ht_txmon_chspec(wlc_ht_info_t *pub, uint16 phytxctl, uint16 phytxctl1,
	uint16 chan_band, uint16 chan_num,
	struct wl_txsts *sts, uint16 *chan_bw);
#endif /* WLTXMONITOR */

/* Ie mgmt */
extern ht_add_ie_t *wlc_read_brcm_ht_add_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len);
extern ht_cap_ie_t *wlc_read_brcm_ht_cap_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len);
extern obss_params_t *wlc_ht_read_obss_scanparams_ie(wlc_info_t *wlc, uint8 *tlv, int tlv_len);
extern ht_cap_ie_t *wlc_read_ht_cap_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len);
extern ht_add_ie_t *wlc_read_ht_add_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len);
extern ht_cap_ie_t *wlc_read_ht_cap_ies(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len);
extern ht_add_ie_t *wlc_read_ht_add_ies(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len);
#else
/* empty macros to avoid having to use WL11N compile flags everywhere */
#define WL_HT_TXBW_OVERRIDE_ENAB 0
#define WLC_HT_GET_FRAMEBURST(hti) FALSE
#define WLC_HT_GET_SGI_TX(a) (OFF)

#define wlc_ht_init_defaults(a)
#define wlc_ht_nvm_overrides(a, b)
#define wlc_ht_fill_sta_fields(a, b, c)

#define wlc_ht_attach(wlc) (NULL)
#define wlc_ht_detach(hti)
#define WLC_HT_GET_RIFS(hti) (0)
#define wlc_ht_update_txburst_limit(a, b)
#define wlc_ht_get_mimo_band_bwcap(hti) (0)
#define wlc_ht_get_phy_membership(hti) FALSE

#define wlc_read_brcm_ht_add_ie(a, b, c) (NULL)
#define wlc_read_brcm_ht_cap_ie(a, b, c) (NULL)

#define wlc_read_ht_cap_ie(a, b, c) (NULL)
#define wlc_read_ht_cap_ies(a, b, c) (NULL)

#define wlc_read_ht_add_ie(a, b, c) (NULL)
#define wlc_read_ht_add_ies(a, b, c) (NULL)

#define wlc_ht_update_scbstate(w, b, c, d, e)
#define wlc_ht_add_ie_verify_rates(a, b, c) (BCME_ERROR)
#define wlc_ht_get_wsec_restriction(a) (WLC_HT_TKIP_RESTRICT | WLC_HT_WEP_RESTRICT)
#define wlc_ht_update_rifs_mode(a, b)
#define wlc_ht_checkadd_rifs_permitted(a, b, c)
#define wlc_frameaction_ht(a, b, c, d, e, f)
#define wlc_ht_read_obss_scanparams_ie(a, b, c) (NULL)
#define wlc_ht_publicaction(a, b, c, d, e, f)

#define wlc_ht_is_40MHZ_cap(a) (FALSE)
#define wlc_ht_monitor(a, b, c, d, e)
#define wlc_ht_prep_rate_info(a, b, c, d)

/* similar to VHT calc when 11N, HT not on */
#define wlc_ht_calc_frame_len(a, b, c, d) ((d + RSPEC2RATE((b)) - 1)/(d))
#define wlc_ht_calc_frame_time(a, b, c, d) ((RSPEC2RATE((b)) * (d))/8000)
#define WLC_HT_GET_TXBURST_LIMIT_OVERRIDE(hti) (OFF)
#define wlc_ht_set_add_ie_basic_mcs(a, b, c)(BCM_REFERENCE(a))
#endif /* WL11N */
#ifdef WL11ULB
extern chanspec_t wlc_ht_chanspec(wlc_info_t *wlc, uint8 chan, uint8 extch, wlc_bsscfg_t *cfg);
#else /* WL11ULB */
extern chanspec_t wlc_ht_chanspec(wlc_info_t *wlc, uint8 chan, uint8 extch);
#endif /* WL11ULB */
extern void wlc_ht_upd_txbf_cap(wlc_bsscfg_t *cfg, uint8 bfr, uint8 bfe, uint32 *cap);
#endif /* _wlc_ht_h_ */
