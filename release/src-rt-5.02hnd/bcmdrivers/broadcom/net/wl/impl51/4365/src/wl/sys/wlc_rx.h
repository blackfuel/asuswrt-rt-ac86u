/*
 * wlc_rx.h
 *
 * Common headers for receive datapath components
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_rx.h 471768 2014-04-22 00:15:30Z $
 *
 */
#ifndef _wlc_rx_c
#define _wlc_rx_c

void BCMFASTPATH
wlc_recv(wlc_info_t *wlc, void *p);

void BCMFASTPATH
wlc_recvdata(wlc_info_t *wlc, osl_t *osh, wlc_d11rxhdr_t *wrxh, void *p);

void BCMFASTPATH
wlc_recvdata_ordered(wlc_info_t *wlc, struct scb *scb, struct wlc_frminfo *f);

void BCMFASTPATH
wlc_recvdata_sendup(wlc_info_t *wlc, struct scb *scb, bool wds, struct ether_addr *da, void *p);

void BCMFASTPATH
wlc_sendup(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb, void *pkt);


extern void wlc_tdls_reinsert_pkt(wlc_info_t *wlc, wlc_bsscfg_t *parent, struct scb *scb, void *p);

extern void wlc_tsf_adopt_bcn(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_bcn_prb *bcn);

extern void
wlc_bcn_ts_tsf_calc(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh,
	void *plcp, uint32 *tsf_h, uint32 *tsf_l);

extern void wlc_bcn_tsf_diff(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, void *plcp,
	struct dot11_bcn_prb *bcn, int32 *diff_h, int32 *diff_l);
extern int wlc_bcn_tsf_later(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, void *plcp,
	struct dot11_bcn_prb *bcn);

#ifdef WLC_HIGH
extern int
wlc_arq_pre_parse_frame(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *ies, uint ies_len, wlc_pre_parse_frame_t *ppf);
#endif

#ifdef BCMSPLITRX
extern void wlc_pktfetch_get_scb(wlc_info_t *wlc, wlc_frminfo_t *f,
	wlc_bsscfg_t **bsscfg, struct scb **scb, bool promisc_frame);
#endif

#endif /* _wlc_rx_c */
