/*
 * HTPHY RSSI Compute module implementation
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

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <qmath.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_rssi.h"
#include <phy_ht.h>
#include <phy_ht_rssi.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_ht.h>
/* TODO: all these are going away... > */
#endif

/* module private states */
struct phy_ht_rssi_info {
	phy_info_t *pi;
	phy_ht_info_t *hti;
	phy_rssi_info_t *ri;
	int8 gain_err[PHY_CORE_MAX];
};

/* local functions */
static void phy_ht_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh);
static void _phy_ht_rssi_init_gain_err(phy_type_rssi_ctx_t *ctx);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int phy_ht_rssi_dump(phy_type_rssi_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_ht_rssi_dump NULL
#endif

/* register phy type specific functions */
phy_ht_rssi_info_t *
BCMATTACHFN(phy_ht_rssi_register_impl)(phy_info_t *pi, phy_ht_info_t *hti, phy_rssi_info_t *ri)
{
	phy_ht_rssi_info_t *info;
	phy_type_rssi_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ht_rssi_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_ht_rssi_info_t));
	info->pi = pi;
	info->hti = hti;
	info->ri = ri;

	/* register PHY type specific implementation */
	fns.compute = phy_ht_rssi_compute;
	fns.init_gain_err = _phy_ht_rssi_init_gain_err;
	fns.dump = phy_ht_rssi_dump;
	fns.ctx = info;

	phy_rssi_register_impl(ri, &fns);
	phy_rssi_enable_ma(ri, TRUE);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ht_rssi_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ht_rssi_unregister_impl)(phy_ht_rssi_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_rssi_info_t *ri = info->ri;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_rssi_unregister_impl(ri);

	phy_mfree(pi, info, sizeof(phy_ht_rssi_info_t));
}

/* calculate rssi and update wrxh */
/* Get Rx power on core 0 */
#define HTPHY_RXPWR_ANT0(rxs)	(((rxs)->PhyRxStatus_2 & PRXS2_HTPHY_RXPWR_ANT0) >> 8)
/* Get Rx power on core 1 */
#define HTPHY_RXPWR_ANT1(rxs)	((rxs)->PhyRxStatus_3 & PRXS3_HTPHY_RXPWR_ANT1)
/* Get Rx power on core 2 */
#define HTPHY_RXPWR_ANT2(rxs)	(((rxs)->PhyRxStatus_3 & PRXS3_HTPHY_RXPWR_ANT2) >> 8)

static void BCMFASTPATH
phy_ht_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh)
{
	phy_ht_rssi_info_t *info = (phy_ht_rssi_info_t *)ctx;
	phy_info_t *pi = info->pi;
	d11rxhdr_t *rxh = &wrxh->rxhdr;
	int16 rxpwr;
	int16 rxpwr_core[PHY_CORE_MAX];
	int i;
	int16 gain_err_temp_adj_for_rssi;


	/* mode = 0: rxpwr = max(rxpwr0, rxpwr1)
	 * mode = 1: rxpwr = min(rxpwr0, rxpwr1)
	 * mode = 2: rxpwr = (rxpwr0+rxpwr1)/2
	 */
	rxpwr_core[0] = HTPHY_RXPWR_ANT0(rxh);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
		rxpwr_core[1] = HTPHY_RXPWR_ANT1(rxh);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
		rxpwr_core[2] = HTPHY_RXPWR_ANT2(rxh);

	/* Sign extend */
	FOREACH_CORE(pi, i) {
		if (rxpwr_core[i] > 127)
			rxpwr_core[i] -= 256;
	}

	wlc_phy_upd_gain_wrt_temp_phy(pi, &gain_err_temp_adj_for_rssi);

	/* Apply gain-error correction with temperature compensation: */
	FOREACH_CORE(pi, i) {
		int16 tmp;
	        tmp = info->gain_err[i]*2 - gain_err_temp_adj_for_rssi;
	        tmp = ((tmp >= 0) ? ((tmp + 2) >> 2) : -1*((-1*tmp + 2) >> 2));
	        rxpwr_core[i] -= tmp;
	}


	/* only 3 antennas are valid for now */
	FOREACH_CORE(pi, i) {
		 rxpwr_core[i] = MAX(-128, rxpwr_core[i]);
		 wrxh->rxpwr[i] = (int8)rxpwr_core[i];
	}
	wrxh->do_rssi_ma = 0;

	/* legacy interface */
	if (PHYCORENUM(pi->pubpi.phy_corenum) == 1) {
		rxpwr = rxpwr_core[0];
	} else {
		uint8 num_activecores = 0;

		rxpwr = 0;
		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, i) {
			if (num_activecores++ == 0) {
				rxpwr = rxpwr_core[i];
			} else {
				switch (pi->sh->rssi_mode) {
				case RSSI_ANT_MERGE_MAX:
					rxpwr = MAX(rxpwr, rxpwr_core[i]);
					break;
				case RSSI_ANT_MERGE_MIN:
					rxpwr = MIN(rxpwr, rxpwr_core[i]);
					break;
				case RSSI_ANT_MERGE_AVG:
					rxpwr += rxpwr_core[i];
					break;
				default:
					ASSERT(0);
				}
			}
		}

		if (pi->sh->rssi_mode == RSSI_ANT_MERGE_AVG) {
			int16 qrxpwr;

			ASSERT(num_activecores > 0);

			rxpwr = (int8)qm_div16(rxpwr, num_activecores, &qrxpwr);
		}
	}

	wrxh->rssi = (int8)rxpwr;
	wrxh->rssi_qdb = 0;

	PHY_TRACE(("%s: rssi %d\n", __FUNCTION__, (int8)rxpwr));
}

/* init gain error table. */
static void
_phy_ht_rssi_init_gain_err(phy_type_rssi_ctx_t *ctx)
{
	phy_ht_rssi_info_t *info = (phy_ht_rssi_info_t *)ctx;
	phy_info_t *pi = info->pi;
	int16 gainerr[PHY_CORE_MAX];
	int16 initgain_dB[PHY_CORE_MAX];
	int16 rxiqest_gain;
	uint8 core;
	bool srom_isempty = FALSE;

	/* Retrieve rxiqest gain error: */
	srom_isempty = wlc_phy_get_rxgainerr_phy(pi, gainerr);

	if (srom_isempty) {
		FOREACH_CORE(pi, core) {
			info->gain_err[core] = 0;
		}
		return;
	}

	/* Retrieve rxiqest gain: */
	wlc_phy_get_rxiqest_gain_htphy(pi, &rxiqest_gain);

	/* Retrieve initgains in dB */
	wlc_phy_get_initgain_dB_htphy(pi, initgain_dB);

	/* Compute correction */
	FOREACH_CORE(pi, core) {
		int16 tmp;
		/* gainerr is in 0.5dB steps; round to nearest dB */
		tmp = gainerr[core];
		tmp = ((tmp >= 0) ? ((tmp + 1) >> 1) : -1*((-1*tmp + 1) >> 1));
		/* report rssi gainerr in 0.5dB steps */
		info->gain_err[core] =
			(int8)((rxiqest_gain << 1) - (initgain_dB[core] << 1) + gainerr[core]);
	}
}

void
phy_ht_rssi_init_gain_err(phy_ht_rssi_info_t *info)
{
	_phy_ht_rssi_init_gain_err(info);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
phy_ht_rssi_dump(phy_type_rssi_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_ht_rssi_info_t *info = (phy_ht_rssi_info_t *)ctx;
	uint i;

	/* DUMP gain_err... */

	bcm_bprintf(b, "gain err:\n");
	for (i = 0; i < ARRAYSIZE(info->gain_err); i ++)
		bcm_bprintf(b, "  core %u: %u\n", i, info->gain_err[i]);

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */
