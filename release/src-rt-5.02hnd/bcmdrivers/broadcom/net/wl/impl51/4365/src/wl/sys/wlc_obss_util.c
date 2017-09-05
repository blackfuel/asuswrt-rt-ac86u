
/*
 * OBSS and bandwidth switch utilities
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
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wlc_obss_util.c 552478 2015-04-27 11:54:40Z $
 */

/**
 * @file
 * @brief
 * Out of Band BSS
 * Banwidth switch utilities
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_scan.h>
#include <wlc_bmac.h>
#include <wlc_obss_util.h>

void
wlc_obss_util_update(wlc_info_t *wlc, wlc_bmac_obss_counts_t *curr,
		wlc_bmac_obss_counts_t *prev, wlc_bmac_obss_counts_t *o_total, chanspec_t chanspec)
{
	uint16 delta;
	if (SCAN_IN_PROGRESS(wlc->scan)) {
		return;
	}

	BCM_REFERENCE(delta);

	/* Save a copy of previous counters */
	memcpy(prev, curr, sizeof(*prev));

	/* Read current ucode counters */
	wlc_bmac_obss_stats_read(wlc->hw, curr);

	/*
	 * Calculate the total counts.
	 */

	/* CCA stats */
	o_total->usecs += curr->usecs - prev->usecs;
	o_total->txdur += curr->txdur - prev->txdur;
	o_total->ibss += curr->ibss - prev->ibss;
	o_total->obss += curr->obss - prev->obss;
	o_total->noctg += curr->noctg - prev->noctg;
	o_total->nopkt += curr->nopkt - prev->nopkt;
	o_total->PM += curr->PM - prev->PM;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	o_total->txopp += curr->txopp - prev->txopp;
	o_total->gdtxdur += curr->gdtxdur - prev->gdtxdur;
	o_total->bdtxdur += curr->bdtxdur - prev->bdtxdur;
	o_total->slot_time_txop = curr->slot_time_txop;
#endif

#ifdef ISID_STATS
	delta = curr->crsglitch - prev->crsglitch;
	o_total->crsglitch += delta;

	delta = curr->badplcp - prev->badplcp;
	o_total->badplcp += delta;

	delta = curr->bphy_crsglitch - prev->bphy_crsglitch;
	o_total->bphy_crsglitch += delta;

	delta = curr->bphy_badplcp - prev->bphy_badplcp;
	o_total->bphy_badplcp += delta;
#endif

	if (!WLC_PROT_OBSS_ENAB(wlc->pub)) {
		return;	/* OBSS stats unsupported */
	}

	/* OBSS stats */
	delta = curr->rxdrop20s - prev->rxdrop20s;
	o_total->rxdrop20s += delta;

	delta = curr->rx20s - prev->rx20s;
	o_total->rx20s += delta;

	o_total->rxcrs_pri += curr->rxcrs_pri - prev->rxcrs_pri;
	o_total->rxcrs_sec20 += curr->rxcrs_sec20 - prev->rxcrs_sec20;
	o_total->rxcrs_sec40 += curr->rxcrs_sec40 - prev->rxcrs_sec40;

	delta = curr->sec_rssi_hist_hi - prev->sec_rssi_hist_hi;
	o_total->sec_rssi_hist_hi += delta;
	delta = curr->sec_rssi_hist_med - prev->sec_rssi_hist_med;
	o_total->sec_rssi_hist_med += delta;
	delta = curr->sec_rssi_hist_low - prev->sec_rssi_hist_low;
	o_total->sec_rssi_hist_low += delta;

	o_total->suspend += curr->suspend - prev->suspend;
	o_total->suspend_cnt += curr->suspend_cnt - prev->suspend_cnt;
	o_total->txfrm += curr->txfrm - prev->txfrm;
	o_total->rxstrt += curr->rxstrt - prev->rxstrt;
	o_total->rxglitch += curr->rxglitch - prev->rxglitch;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#define DIV_QUO(num, div) ((num)/div)  /* Return the quotient of division to avoid floats */
#define DIV_REM(num, div) (((num%div) * 10)/div) /* Return the remainder of division */

void
wlc_obss_util_stats(wlc_info_t *wlc,
		wlc_bmac_obss_counts_t *msrmnt_stored, wlc_bmac_obss_counts_t *prev_stats,
		wlc_bmac_obss_counts_t *curr_stats, uint8 report_opt, struct bcmstrbuf *b)
{
	wlc_bmac_obss_counts_t *stats = msrmnt_stored;
	uint32 usecs = stats->usecs;
	uint32 active = usecs - stats->PM - stats->suspend * 100;
	uint32 rxdur = stats->obss + stats->ibss + stats->noctg + stats->nopkt;
	const char *c0 = "";
	char c1[10], c2[10], c3[10], c4[10];

	if (usecs == 0) {
	/* This means the TSF (usecs) is not updating. Just return to prevent divide by 0 */
		bcm_bprintf(b, "\nInvalid usecs 0x%x prev 0x%x curr 0x%x\n",
			stats->usecs,
			prev_stats->usecs,
			curr_stats->usecs);
		return;
	}

	bcm_bprintf(b, "total %d (time unit ms)\n",
	    usecs / 1000);
	snprintf(c1, sizeof(c1), " (%d.%d%%)", DIV_QUO(active * 100, usecs),
	    DIV_REM(active * 100, usecs));
	if (report_opt == 0) {
		/* default version */
		uint32 seccrs = stats->rxcrs_sec20 + stats->rxcrs_sec40;

		snprintf(c2, sizeof(c2), " (%d.%02d%%)", DIV_QUO(stats->txdur * 100, usecs),
		    DIV_REM(stats->txdur * 100, usecs));
		snprintf(c3, sizeof(c3), " (%d.%02d%%)", DIV_QUO(rxdur * 100, usecs),
		    DIV_REM(rxdur * 100, usecs));
		bcm_bprintf(b, "active %d%s txdur %d%s rxdur %d%s\n",
		    active / 1000, (active ? c1 : c0),
		    stats->txdur / 1000, (stats->txdur ? c2 : c0),
		    rxdur / 1000, (rxdur ? c3 : c0));

		snprintf(c2, sizeof(c2), "(%d.%02d%%)", DIV_QUO(stats->rxcrs_pri * 100, usecs),
		     DIV_REM(stats->rxcrs_pri * 100, usecs));
		snprintf(c3, sizeof(c3), "(%d.%02d%%)", DIV_QUO(seccrs * 100, usecs),
		     DIV_REM(seccrs * 100, usecs));
		bcm_bprintf(b, "rxcrs : pri %d%s sec %d%s\n", stats->rxcrs_pri / 1000,
		    (stats->rxcrs_pri ? c2:c0), seccrs / 1000, (seccrs ? c3 : c0));
	} else {
		/* detail version */
		uint32 slot2time = ((stats->slot_time_txop >> 8) & 0xFF)
				 + (stats->slot_time_txop & 0xFF);
		uint32 txop = stats->txopp * slot2time;

		snprintf(c2, sizeof(c2), " (%d.%d%%)", DIV_QUO(stats->PM * 100, usecs),
		     DIV_REM(stats->PM * 100, usecs));
		snprintf(c3, sizeof(c3), " (%d.%d%%)", DIV_QUO(stats->suspend * 10000, usecs),
		     DIV_REM(stats->suspend * 10000, usecs));
		bcm_bprintf(b, "active %d%s sleep %d%s suspend %d%s\n",
		    active / 1000, (active ? c1 : c0),
		    stats->PM / 1000, (stats->PM ? c2 : c0),
		    stats->suspend / 10, (stats->suspend ? c3 : c0));

		snprintf(c2, sizeof(c2), " (%d.%d%%)", DIV_QUO(txop * 100, usecs),
		     DIV_REM(txop * 100, usecs));
		bcm_bprintf(b, "txop  : %d (%d slots)%s\n",
		    txop / 1000, stats->txopp, (txop ? c2 : c0));

		snprintf(c2, sizeof(c2), " (%d.%d%%)", DIV_QUO(stats->gdtxdur * 100, usecs),
		     DIV_REM(stats->gdtxdur * 100, usecs));
		snprintf(c3, sizeof(c3), " (%d.%d%%)", DIV_QUO(stats->bdtxdur * 100, usecs),
		     DIV_REM(stats->bdtxdur * 100, usecs));
		bcm_bprintf(b, "txdur : good %d%s norsp %d%s\n",
		    stats->gdtxdur / 1000, (stats->gdtxdur ? c2 : c0),
		    stats->bdtxdur / 1000, (stats->bdtxdur ? c3 : c0));

		snprintf(c1, sizeof(c1), " (%d.%d%%)", DIV_QUO(stats->ibss * 100, usecs),
		     DIV_REM(stats->gdtxdur * 100, usecs));
		snprintf(c2, sizeof(c2), " (%d.%d%%)", DIV_QUO(stats->obss * 100, usecs),
		     DIV_REM(stats->bdtxdur * 100, usecs));
		snprintf(c3, sizeof(c3), " (%d.%d%%)", DIV_QUO(stats->noctg * 100, usecs),
		     DIV_REM(stats->gdtxdur * 100, usecs));
		snprintf(c4, sizeof(c4), " (%d.%d%%)", DIV_QUO(stats->nopkt * 100, usecs),
		     DIV_REM(stats->bdtxdur * 100, usecs));
		bcm_bprintf(b, "rxdur : ibss %d%s obss %d%s noctg %d%s nopkt %d%s\n",
		    stats->ibss / 1000, (stats->ibss ? c1 : c0),
		    stats->obss / 1000, (stats->obss ? c2 : c0),
		    stats->noctg / 1000, (stats->noctg ? c3 : c0),
		    stats->nopkt / 1000, (stats->nopkt ? c4 : c0));

		if (report_opt == 2) {
			snprintf(c1, sizeof(c1), " (%d.%d%%)",
			    DIV_QUO(stats->rxcrs_pri * 100, usecs),
			    DIV_REM(stats->rxcrs_pri * 100, usecs));
			snprintf(c2, sizeof(c2), " (%d.%d%%)",
			    DIV_QUO(stats->rxcrs_sec20 * 100, usecs),
			    DIV_REM(stats->rxcrs_sec20 * 100, usecs));
			snprintf(c3, sizeof(c3), " (%d.%d%%)",
			    DIV_QUO(stats->rxcrs_sec40 * 100, usecs),
			    DIV_REM(stats->rxcrs_sec40 * 100, usecs));
			bcm_bprintf(b, "rxcrs : pri %d%s sec20 %d%s sec40 %d%s\n",
			    stats->rxcrs_pri / 1000, (stats->rxcrs_pri ? c1 : c0),
			    stats->rxcrs_sec20 / 1000, (stats->rxcrs_sec20 ? c2 : c0),
			    stats->rxcrs_sec40 / 1000, (stats->rxcrs_sec40 ? c3 : c0));

			bcm_bprintf(b, "\tsec20 drop cnt: %d out of %d\n",
			    stats->rxdrop20s, stats->rx20s);
			bcm_bprintf(b, "\tsec20 rssi cnt: hi %d  med %d  low %d\n",
			    stats->sec_rssi_hist_hi, stats->sec_rssi_hist_med,
			    stats->sec_rssi_hist_low);
		}

		bcm_bprintf(b, "cnt   : txfrm %d rxstrt %d rxglitch %d suspend %d \n",
		    stats->txfrm, stats->rxstrt, stats->rxglitch, stats->suspend_cnt);
	}
}
#endif 
