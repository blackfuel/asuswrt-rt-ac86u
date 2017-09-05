/*
 * MU-MIMO receive module for Broadcom 802.11 Networking Adapter Device Drivers
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_murx.h 537259 2015-02-25 13:29:29Z $
 */

#ifndef _wlc_murx_h_
#define _wlc_murx_h_

#include <typedefs.h>
#include <wlc_types.h>
#include <wlc_mumimo.h>

#ifdef WLCNT
#define WLC_UPDATE_MURX_INPROG(wlc, bval) wlc_murx_update_murx_inprog((wlc)->murx, bval)
#define WLC_GET_MURX_INPROG(wlc) wlc_murx_get_murx_inprog((wlc)->murx)
#endif /* WLCNT */

#define WLC_MURX_BI_MU_BFR_CAP(wlc, bi) (MU_RX_ENAB(wlc) && \
	wlc_murx_is_bi_mu_bfr_cap((wlc)->murx, (bi)))

wlc_murx_info_t *(wlc_murx_attach)(wlc_info_t *wlc);
void (wlc_murx_detach)(wlc_murx_info_t *mu_info);
void wlc_murx_filter_bfe_cap(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg, uint32 *cap);
bool wlc_murx_is_bi_mu_bfr_cap(wlc_murx_info_t *mu_info, wlc_bss_info_t *bi);
int wlc_murx_gid_update(wlc_info_t *wlc, struct scb *scb,
                        uint8 *membership_status, uint8 *user_position);
#if defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU) || \
	defined(BCMDBG_DUMP))
void wlc_murx_update_rxcounters(wlc_murx_info_t *mu_info, uint32 ft, struct scb *scb,
	struct dot11_header *h);
#endif
bool wlc_murx_active(wlc_murx_info_t *mu_info);
#ifdef WLRSDB
bool wlc_murx_anymurx_active(wlc_murx_info_t *mu_info);
#endif /* WLRSDB */
#ifdef WL_MODESW
void wlc_murx_sync_oper_mode(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg, wlc_bss_info_t *bi);
#endif /* WL_MODESW */
#ifdef WLCNT
void wlc_murx_update_murx_inprog(wlc_murx_info_t *mu_info, bool bval);
bool wlc_murx_get_murx_inprog(wlc_murx_info_t *mu_info);
#endif /* WLCNT */
#endif /* _wlc_murx_h_ */
