/*
 * LCNPHY RSSI Compute module implementation
 *
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_rssi.h"
#include <phy_lcn.h>
#include <phy_lcn_rssi.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn.h>
/* TODO: all these are going away... > */
#endif

/* module private states */
struct phy_lcn_rssi_info {
	phy_info_t *pi;
	phy_lcn_info_t *lcni;
	phy_rssi_info_t *ri;
};

/* local functions */
static void phy_lcn_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh);

/* register phy type specific functions */
phy_lcn_rssi_info_t *
BCMATTACHFN(phy_lcn_rssi_register_impl)(phy_info_t *pi, phy_lcn_info_t *lcni, phy_rssi_info_t *ri)
{
	phy_lcn_rssi_info_t *info;
	phy_type_rssi_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn_rssi_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn_rssi_info_t));
	info->pi = pi;
	info->lcni = lcni;
	info->ri = ri;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.compute = phy_lcn_rssi_compute;
	fns.ctx = info;

	phy_rssi_register_impl(ri, &fns);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn_rssi_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn_rssi_unregister_impl)(phy_lcn_rssi_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_rssi_info_t *ri = info->ri;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_rssi_unregister_impl(ri);

	phy_mfree(pi, info, sizeof(phy_lcn_rssi_info_t));
}

/* calculate rssi */
static void BCMFASTPATH
phy_lcn_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh)
{
	phy_lcn_rssi_info_t *info = (phy_lcn_rssi_info_t *)ctx;
	phy_info_t *pi = info->pi;
	phy_lcn_info_t *lcni = info->lcni;
	d11rxhdr_t *rxh = &wrxh->rxhdr;
	int rssi = rxh->PhyRxStatus_1 & PRXS1_JSSI_MASK;
	uint16 board_atten = (rxh->PhyRxStatus_0 >> 11) & 0x1;
	uint8 gidx = (rxh->PhyRxStatus_2 & 0xFC00) >> 10;
	int8 ant = (int8)((rxh->PhyRxStatus_0 >> 14) & 0x1);

	if (rssi > 127)
		rssi -= 256;

	/* RSSI adjustment */
	rssi = rssi + phy_lcn_get_pkt_rssi_gain_index_offset(gidx);
	if (board_atten)
		rssi = rssi + pi->rssi_corr_boardatten;
	else
		rssi = rssi + pi->rssi_corr_normal;

	/* temperature compensation */
	rssi = rssi + lcni->lcnphy_pkteng_rssi_slope;

	/* Sign extend */
	if (rssi > 127)
		rssi -= 256;

	wrxh->rxpwr[0] = (int8)rssi;

	wrxh->rssi = (int8)rssi;
	wrxh->rssi_qdb = 0;
	wrxh->do_rssi_ma = 0;

	PHY_TRACE(("%s: rssi %d\n", __FUNCTION__, (int8)rssi));

	wlc_phy_lcn_updatemac_rssi(pi, (int8)rssi, ant);
}

static const int8 lcnphy_gain_index_offset_for_pkt_rssi[] = {
	8,	/* 0 */
	8,	/* 1 */
	8,	/* 2 */
	8,	/* 3 */
	8,	/* 4 */
	8,	/* 5 */
	8,	/* 6 */
	9,	/* 7 */
	10,	/* 8 */
	8,	/* 9 */
	8,	/* 10 */
	7,	/* 11 */
	7,	/* 12 */
	1,	/* 13 */
	2,	/* 14 */
	2,	/* 15 */
	2,	/* 16 */
	2,	/* 17 */
	2,	/* 18 */
	2,	/* 19 */
	2,	/* 20 */
	2,	/* 21 */
	2,	/* 22 */
	2,	/* 23 */
	2,	/* 24 */
	2,	/* 25 */
	2,	/* 26 */
	2,	/* 27 */
	2,	/* 28 */
	2,	/* 29 */
	2,	/* 30 */
	2,	/* 31 */
	1,	/* 32 */
	1,	/* 33 */
	0,	/* 34 */
	0,	/* 35 */
	0,	/* 36 */
	0	/* 37 */
};

int8 *phy_lcn_pkt_rssi_gain_index_offset = (int8 *)lcnphy_gain_index_offset_for_pkt_rssi;
