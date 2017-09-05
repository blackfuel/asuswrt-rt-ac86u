/*
 * metric tab  statistics for visualization tool
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
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
 * $Id: vis_wlmetrics.c 625886 2016-03-18 03:10:37Z $
 */

#ifdef WIN32
#include <windows.h>
#endif

#if !defined(TARGETOS_nucleus)
#define CLMDOWNLOAD
#endif


#if defined(__NetBSD__)
#include <typedefs.h>
#endif

/* Because IL_BIGENDIAN was removed there are few warnings that need
 * to be fixed. Windows was not compiled earlier with IL_BIGENDIAN.
 * Hence these warnings were not seen earlier.
 * For now ignore the following warnings
 */
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4761)
#endif

#include <bcmendian.h>
#include "vis_common.h"
#if defined(WLPFN) && defined(linux)
#ifndef TARGETENV_android
#include <unistd.h>
#endif
#endif /* WLPFN */

#ifdef WLEXTLOG
#include <wlc_extlog_idstr.h>
#endif

#include <inttypes.h>
#include <errno.h>

#if defined SERDOWNLOAD || defined CLMDOWNLOAD
#include <sys/stat.h>
#include <trxhdr.h>
#ifdef SERDOWNLOAD
#include <usbrdl.h>
#endif
#include <stdio.h>
#include <errno.h>

#ifndef WIN32
#include <fcntl.h>
#endif /* WIN32 */
#endif /* SERDOWNLOAD || defined CLMDOWNLOAD */

#include <wlioctl_utils.h>
#include "vis_wlmetrics.h"
#include "vis_wl.h"

/* some OSes (FC4) have trouble allocating (kmalloc) 128KB worth of memory,
 * hence keeping WL_DUMP_BUF_LEN below that
 */
#if defined(BWL_SMALL_WLU_DUMP_BUF)
#define WL_DUMP_BUF_LEN (4 * 1024)
#else
#define WL_DUMP_BUF_LEN (127 * 1024)
#endif 

#define	PRVAL(name)	pbuf += sprintf(pbuf, "%s %u ", #name, dtoh32(cnt->name))
#define	PRVALSIX(name)	pbuf += sprintf(pbuf, "%s %u ", #name, dtoh32(cnt_six->name))
#define	PRNL()		pbuf += sprintf(pbuf, "\n")

#define WL_CNT_VERSION_SIX 6

/* As VHT rates 0 - 9 + prop 10-11. So both MCS and VHT can go max upto 12 */
#define MAX_AMPDU_DUMP_PER_LINE	12
/* As of now only 4 streams for MCS and VHT. But MPDUDensity supports max 64 MPDU's
 * In each line 8 entries so there can be max 8 lines
 */
#define MAX_AMPDU_DUMP_LINES	8
/* As we have 1 structure to hold all the AMPDU dump values, we have to store max of
 * MCS or VHT entries per line(VHT is max 12) and also Max number of lines in the dump
 * Which is max 8 for MPDUDensity
 */
#define MAX_AMPDU_ENTRIES	(MAX_AMPDU_DUMP_LINES * MAX_AMPDU_DUMP_PER_LINE)

extern long g_timestamp;
extern char *g_vis_cmdoutbuf;

#define MCSTCNT_LE10_FROM_CNTBUF(cntbuf) (wl_cnt_v_le10_mcst_t *) \
	bcm_get_data_from_xtlv_buf(((wl_cnt_info_t *)cntbuf)->data,     \
		((wl_cnt_info_t *)cntbuf)->datalen,                     \
		WL_CNT_XTLV_CNTV_LE10_UCODE, NULL,                      \
		BCM_XTLV_OPTION_ALIGN32)

#define MCSTCNT_LT40_FROM_CNTBUF(cntbuf) (wl_cnt_lt40mcst_v1_t *)\
	bcm_get_data_from_xtlv_buf(((wl_cnt_info_t *)cntbuf)->data,     \
		((wl_cnt_info_t *)cntbuf)->datalen,                     \
		WL_CNT_XTLV_LT40_UCODE_V1, NULL,                        \
		BCM_XTLV_OPTION_ALIGN32)

#define MCSTCNT_GE40_FROM_CNTBUF(cntbuf) (wl_cnt_ge40mcst_v1_t *)\
	bcm_get_data_from_xtlv_buf(((wl_cnt_info_t *)cntbuf)->data,     \
		((wl_cnt_info_t *)cntbuf)->datalen,                     \
		WL_CNT_XTLV_GE40_UCODE_V1, NULL,                        \
		BCM_XTLV_OPTION_ALIGN32)

/* Gets the counters from driver. Send the delta counter values */
int
wl_counters(void *wl, counters_t *countersout, counters_t *oldcountersout)
{
	counters_t tmpcounters = {0};
	int err = BCME_OK;
	uint16 ver;
	char *cntbuf;
	wl_cnt_info_t *cntinfo;
	uint32 corerev = 0;
	wl_cnt_v_le10_mcst_t *macstat_le10;
	wl_cnt_lt40mcst_v1_t *macstat_lt40;
	wl_cnt_ge40mcst_v1_t *macstat_ge40;

	cntbuf = malloc(WLC_IOCTL_MEDLEN);
	if (cntbuf == NULL) {
		VIS_METRICS("NOMEM\n");
		return BCME_NOMEM;
	}
	err = wlu_iovar_get(wl, "counters", cntbuf, WLC_IOCTL_MEDLEN);
	if (err < 0) {
		VIS_METRICS("wl counters failed (%d)\n", err);
		goto exit;
	}

	cntinfo = (wl_cnt_info_t *)cntbuf;
	cntinfo->version = dtoh16(cntinfo->version);
	cntinfo->datalen = dtoh16(cntinfo->datalen);
	ver = cntinfo->version;
	if (cntinfo->datalen + OFFSETOF(wl_cnt_info_t, data) > WLC_IOCTL_MEDLEN) {
	    printf("%s: IOVAR buffer short!\n", __FUNCTION__);
	}
	if (ver == WL_CNT_VERSION_11) {
		wlc_rev_info_t revinfo;
		memset(&revinfo, 0, sizeof(revinfo));
		err = wlu_get(wl, WLC_GET_REVINFO, &revinfo, sizeof(revinfo));
		if (err) {
			VIS_METRICS("WLC_GET_REVINFO failed %d\n", err);
			goto exit;
		}
		corerev = dtoh32(revinfo.corerev);
	}

	/* Translate traditional (ver <= 10) counters struct to new xtlv type struct */
	err = wl_cntbuf_to_xtlv_format(NULL, cntbuf, WLC_IOCTL_MEDLEN, corerev);
	if (err < 0) {
		VIS_METRICS("wl_cntbuf_to_xtlv_format failed (%d)\n", err);
		goto exit;
	}

	if ((macstat_le10 = MCSTCNT_LE10_FROM_CNTBUF(cntbuf)) == NULL) {
		if ((macstat_lt40 = MCSTCNT_LT40_FROM_CNTBUF(cntbuf)) == NULL) {
			if ((macstat_ge40 = MCSTCNT_GE40_FROM_CNTBUF(cntbuf)) == NULL) {
				VIS_METRICS("No Counters Found in TLV\n");
			} else {
				tmpcounters.rxcrsglitch = dtoh32(macstat_ge40->rxcrsglitch);
				tmpcounters.rxbadplcp = dtoh32(macstat_ge40->rxbadplcp);
				tmpcounters.rxbadfcs = dtoh32(macstat_ge40->rxbadfcs);
			}
		} else {
			tmpcounters.rxcrsglitch = dtoh32(macstat_lt40->rxcrsglitch);
			tmpcounters.rxbadplcp = dtoh32(macstat_lt40->rxbadplcp);
			tmpcounters.rxbadfcs = dtoh32(macstat_lt40->rxbadfcs);
		}
	} else {
		tmpcounters.rxcrsglitch = dtoh32(macstat_le10->rxcrsglitch);
		tmpcounters.rxbadplcp = dtoh32(macstat_le10->rxbadplcp);
		tmpcounters.rxbadfcs = dtoh32(macstat_le10->rxbadfcs);
	}

	/* summary stat counter line */
	VIS_METRICS("\tRx CRS Glitch Counter : %u\n", tmpcounters.rxcrsglitch);
	VIS_METRICS("\tBad FCS Counter : %u\n", tmpcounters.rxbadfcs);
	VIS_METRICS("\tBad PLCP Counter : %u\n", tmpcounters.rxbadplcp);

	countersout->timestamp = g_timestamp;
	/* For the first time send 0 to dcon and store the new value in old counters */
	if (oldcountersout->timestamp == 0) {
		oldcountersout->timestamp = countersout->timestamp;
		oldcountersout->rxcrsglitch = tmpcounters.rxcrsglitch;
		oldcountersout->rxbadplcp = tmpcounters.rxbadplcp;
		oldcountersout->rxbadfcs = tmpcounters.rxbadfcs;
		countersout->rxcrsglitch = 0;
		countersout->rxbadplcp = 0;
		countersout->rxbadfcs = 0;
	} else { /* Send the delta value to dcon and store the new value in old counter */
		countersout->rxcrsglitch = tmpcounters.rxcrsglitch - oldcountersout->rxcrsglitch;
		countersout->rxbadplcp = tmpcounters.rxbadplcp - oldcountersout->rxbadplcp;
		countersout->rxbadfcs = tmpcounters.rxbadfcs - oldcountersout->rxbadfcs;

		/* Store the new value to old counters */
		oldcountersout->rxcrsglitch = tmpcounters.rxcrsglitch;
		oldcountersout->rxbadplcp = tmpcounters.rxbadplcp;
		oldcountersout->rxbadfcs = tmpcounters.rxbadfcs;
	}

exit:
	free(cntbuf);
	return err;
}

/* Get packet queue stats */
static int
wl_txq_prec_dump(wl_iov_pktq_log_t* iov_pktq_log, bool is_aqm, int i,
	assoc_sta_list_t **stas_listout)
{
	uint32 totrqstd = 0, totstored = 0, totdropped = 0, totretried = 0, totutlistn = 0;
	uint32 totqlength = 0, totrtsfail = 0, totrtrydrop = 0, totpsretry = 0;
	uint32 totacked = 0;
	float tottput = 0.00, totphyrate = 0.00;

	#define PREC_DUMPV(v4, v5)  ((iov_pktq_log->version == 4) ? (v4) : (v5))

	uint8  index;
	uint8  prec;
	uint32 prec_mask = 0;
	pktq_log_format_v05_t* logv05 = NULL;
	pktq_log_format_v04_t* logv04 = NULL;
	uint32 num_prec = 0;

	if (iov_pktq_log->version == 0x04) {
		logv04 = &iov_pktq_log->pktq_log.v04;
	} else if (iov_pktq_log->version == 0x05) {
		logv05 = &iov_pktq_log->pktq_log.v05;
	} else {
		VIS_METRICS("Unknown/unsupported binary format (%x)\n",
		        iov_pktq_log->version);
		return -1;
	}

	index = 0;

	prec_mask = PREC_DUMPV(logv04->counter_info[index], logv05->counter_info[index]);
	num_prec = PREC_DUMPV(logv04->num_prec[index], logv05->num_prec[index]);

	/* Note that this is zero if the data is invalid */
	if (!num_prec) {
		VIS_METRICS("Parameter %c:%s not valid\n",
			iov_pktq_log->params.addr_type[index] != 0 ?
			iov_pktq_log->params.addr_type[index] & 0x7F : ' ',
			wl_ether_etoa(&iov_pktq_log->params.ea[index]));
		return -1;
	}

	for (prec = 0; prec < num_prec; prec++) {
		float tput = 0.0;
		float txrate_succ = 0.0;
		pktq_log_counters_v05_t counters;

		if (!(prec_mask & (1 << prec))) {
			continue;
		}

		if (iov_pktq_log->version == 5) {
			counters = logv05->counters[index][prec];
		} else {
			/* the following is a trick - it is possible because
			 * V4 and V5 are both common except that V5 has extra fields
			 * at the end
			*/
			memcpy(&counters, &logv04->counters[index][prec],
				sizeof(pktq_log_counters_v04_t));
			counters.airtime = 0;
		}

		txrate_succ = (float)counters.txrate_succ * 0.5;

		if (counters.time_delta != 0) {
			/* convert bytes to bits */
			tput = (float)counters.throughput;
			tput *= 8.0;

			/* converts to rate of bits per us,
			   because time_delta is in micro-seconds
			*/
			tput /= (float)counters.time_delta;
		}

		if (!(is_aqm && (prec & 1))) {
			uint32 acked = counters.acked;

			if (is_aqm && (prec_mask & (1 << (prec + 1)))) {
				pktq_log_counters_v05_t hi;

				if (iov_pktq_log->version == 5) {
					hi = logv05->counters[index][prec + 1];
				} else {
					/* the following is a trick - it is possible
					 * fields V4 and V5 are both common except
					 * that V5 has extra fields at the end
					 */
					memcpy(&hi, &logv04->counters[index][prec + 1],
						sizeof(pktq_log_counters_v04_t));
				}

				acked += hi.acked;
			}
			if (acked) {
				txrate_succ /= (float) acked;
			} else {
				txrate_succ = 0;
			}
		}

		if (is_aqm && (prec & 1)) {
			totretried += 0;
			totphyrate += 0;
		} else {
			totretried += counters.retry;
			totphyrate += txrate_succ;
		}
		totrqstd += counters.requested;
		totstored += counters.stored;
		totdropped += counters.dropped;
		totutlistn += counters.max_used;
		totqlength = counters.queue_capacity;
		tottput += tput;
		totrtsfail += counters.rtsfail;
		totrtrydrop += counters.retry_drop;
		totpsretry += counters.ps_retry;
		totacked += counters.acked;
	}

	VIS_METRICS("\tMAC : %s\n", (*stas_listout)->sta[i].mac);
	VIS_METRICS("\tPackets Requested : %7u\n", totrqstd);
	VIS_METRICS("\tPackets Stored : %7u\n", totstored);
	VIS_METRICS("\tPackets Dropped : %7u\n", totdropped);
	VIS_METRICS("\tPackets Retried : %7u\n", totretried);
	VIS_METRICS("\tRTSFail : %7u\n", totrtsfail);
	VIS_METRICS("\tRetry Drop : %7u\n", totrtrydrop);
	VIS_METRICS("\tPS Retry : %7u\n", totpsretry);
	VIS_METRICS("\tAcked : %7u\n", totacked);
	VIS_METRICS("\tQueue Utilization : %7u\n", totutlistn);
	VIS_METRICS("\tQueue Length : %7u\n", totqlength);
	VIS_METRICS("\tData Throughput : %8.2f Mbits/s\n", tottput);
	VIS_METRICS("\tPhy Rate : %8.2f Mbits/s\n", totphyrate);

	(*stas_listout)->sta[i].prequested = totrqstd;
	(*stas_listout)->sta[i].pstored = totstored;
	(*stas_listout)->sta[i].pdropped = totdropped;
	(*stas_listout)->sta[i].pretried = totretried;
	(*stas_listout)->sta[i].prtsfail = totrtsfail;
	(*stas_listout)->sta[i].prtrydrop = totrtrydrop;
	(*stas_listout)->sta[i].ppsretry = totpsretry;
	(*stas_listout)->sta[i].packed = totacked;
	(*stas_listout)->sta[i].putilization = totutlistn;
	(*stas_listout)->sta[i].pqlength = totqlength;
	(*stas_listout)->sta[i].ptput = tottput;
	(*stas_listout)->sta[i].pphyrate = totphyrate;

	return 0;
}

/* IO variables that take MAC addresses */
static int
wl_get_pktq_stats(void *wl, assoc_sta_list_t **stas_listout, int i)
{
	int ret;
	char addrType[2] = {"A"};

	wl_iov_mac_params_t*  params = (wl_iov_mac_params_t*)g_vis_cmdoutbuf;

	memset(params, 0, sizeof(*params));

	/* is there a prefix character? */
	params->addr_type[params->num_addrs] = toupper((int)(addrType[0]));

	/* check for + additional info option, set top bit */
	params->addr_type[params->num_addrs]  |= 0x80;

	/* the prefix C: denotes no given MAC address (to refer to "common") */
	if (((params->addr_type[params->num_addrs] & 0x7F) == 'C') ||
		wl_ether_atoe((*stas_listout)->sta[i].mac, &params->ea[params->num_addrs])) {
		params->num_addrs ++;
	} else {
		params->addr_type[params->num_addrs] = 0;
		VIS_METRICS("Bad parameter '%s'\n", (*stas_listout)->sta[i].mac);
	}

	if ((ret = wlu_iovar_getbuf(wl, "pktq_stats", params,
			sizeof(*params), g_vis_cmdoutbuf, WLC_IOCTL_MAXLEN)) < 0) {
		VIS_METRICS("Error getting pktq_stats\n");
		return ret;
	}

	wl_txq_prec_dump((wl_iov_pktq_log_t*)g_vis_cmdoutbuf, FALSE, i, stas_listout);

	return 0;
}

/* To get the list of associated STA's and their RSSI and PHY Rate */
static assoc_sta_list_t*
wl_get_bss_peer_info(void *wl)
{
	bss_peer_list_info_t *info;
	bss_peer_info_t *peer_info;
	bss_peer_info_param_t param;
	int err, i;
	void *ptr;
	assoc_sta_list_t *stas_listout = NULL;
	int szalloclen = 0;

	memset(&param, 0, sizeof(bss_peer_info_param_t));
	param.version = htod16(BSS_PEER_INFO_PARAM_CUR_VER);

	if ((err = wlu_var_getbuf_med(wl, "bss_peer_info", &param, sizeof(bss_peer_info_param_t),
		&ptr)) < 0)
		return NULL;

	info = (bss_peer_list_info_t*)ptr;

	if ((dtoh16(info->version) != BSS_PEER_LIST_INFO_CUR_VER) ||
		(dtoh16(info->bss_peer_info_len) != sizeof(bss_peer_info_t))) {
		VIS_METRICS("BSS peer info version/structure size mismatch driver %d "
				"firmware %d \r\n", BSS_PEER_LIST_INFO_CUR_VER,
				dtoh16(info->version));
		return NULL;
	}

	if (WLC_IOCTL_MEDLEN < (BSS_PEER_LIST_INFO_FIXED_LEN +
		(dtoh32(info->count) * sizeof(bss_peer_info_t)))) {
		VIS_METRICS("ERROR : peer list received exceed the buffer size\r\n");
	}

	szalloclen = (sizeof(assoc_sta_list_t) + (sizeof(assoc_sta_t) * (dtoh32(info->count))));
	stas_listout = (assoc_sta_list_t*)malloc(szalloclen);
	if (stas_listout == NULL) {
		VIS_METRICS("Failed to allocate stas_listout buffer of sz  %d\n", szalloclen);
		return NULL;
	}
	memset(stas_listout, 0x00, szalloclen);
	stas_listout->version = 1;
	stas_listout->length = dtoh32(info->count);
	stas_listout->timestamp = g_timestamp;

	for (i = 0; i < (int)dtoh32(info->count); i++) {
		peer_info = &info->peer_info[i];
		snprintf(stas_listout->sta[i].mac, (ETHER_ADDR_LEN * 3), "%s",
			wl_ether_etoa(&peer_info->ea));
		stas_listout->sta[i].rssi = peer_info->rssi;
		stas_listout->sta[i].phyrate = (dtoh32(peer_info->tx_rate) / 1000);
		VIS_METRICS("MAC : %s\tRSSI : %d\tPhy Rate : %d\n\n",
			stas_listout->sta[i].mac, stas_listout->sta[i].rssi,
			stas_listout->sta[i].phyrate);
	}

	return stas_listout;
}

/* This will get the details for all the associated STA'S
 * This uses the wl bss_info to get the assocaiated STA's
 * and to get the phy rate and RSSI
 * wl pktq_stats to get the packet queue stats
 */
assoc_sta_list_t*
wl_get_counters_for_all_stas(void *wl)
{
	int i;
	assoc_sta_list_t *stas_listout = NULL;

	/* Get all the associated STA's from bss_peer_info IOVAR */
	stas_listout = wl_get_bss_peer_info(wl);
	if (stas_listout == NULL)
		return NULL;

	/* Now get the packet queue stats for all the associated STA's */
	for (i = 0; i < stas_listout->length; i++) {
		wl_get_pktq_stats(wl, &stas_listout, i);
	}

	return stas_listout;
}

/* Gets the MCS rate */
static int
get_mcs_rate(char *pch, int *rate, int *percent, int *per)
{
	int ret = 0;
	while (*pch != '(' && *pch != '/' && *pch != '\n') {
		if (!isdigit(*pch)) {
			pch++;
			ret++;
			continue;
		}
		(*rate) *= 10;
		(*rate) += *pch - '0';
		pch++;
		ret++;
		if (pch == NULL || *pch == '\0')
			break;
	}
	if (pch != NULL && *pch != '\0' && *pch == '/') {
		pch++;
		ret++;
		while (*pch != '(' && *pch != '\n') {
			if (!isdigit(*pch)) {
				pch++;
				ret++;
				continue;
			}
			(*per) *= 10;
			(*per) += *pch - '0';
			pch++;
			ret++;
			if (pch == NULL || *pch == '\0')
				break;
		}
	}
	if (pch == NULL || *pch == '\0' || *pch != '(')
		goto vis_ampdu_parse_err;
	pch++;
	ret++;

	while (*pch != '%' && *pch != '\n') {
		(*percent) *= 10;
		(*percent) += *pch - '0';
		pch++;
		ret++;
		if (pch == NULL || *pch == '\0')
			break;
	}

	return ret;

vis_ampdu_parse_err:
	*rate = 0;
	*percent = 0;
	*per = 0;

	return ret;
}

/* Print the parsed AMPDU counters */
static void
print_ampdu(tx_ampdu_list_t *ampdulistout)
{
	int i;

	for (i = 0; i < ampdulistout->length; i++)
	{
		printf("%d x %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",
			ampdulistout->ampdus[i].streams, ampdulistout->ampdus[i].mcs,
			ampdulistout->ampdus[i].txmcs, ampdulistout->ampdus[i].txmcspercent,
			ampdulistout->ampdus[i].txmcssgi, ampdulistout->ampdus[i].txmcssgipercent,
			ampdulistout->ampdus[i].rxmcs, ampdulistout->ampdus[i].rxmcspercent,
			ampdulistout->ampdus[i].rxmcssgi, ampdulistout->ampdus[i].rxmcssgipercent,
			ampdulistout->ampdus[i].txvht, ampdulistout->ampdus[i].txvhtper,
			ampdulistout->ampdus[i].txvhtpercent,
			ampdulistout->ampdus[i].txvhtsgi, ampdulistout->ampdus[i].txvhtsgiper,
			ampdulistout->ampdus[i].txvhtsgipercent);
		printf("%d %d %d %d %d %d %d %d\n",
			ampdulistout->ampdus[i].rxvht, ampdulistout->ampdus[i].rxvhtper,
			ampdulistout->ampdus[i].rxvhtpercent,
			ampdulistout->ampdus[i].rxvhtsgi, ampdulistout->ampdus[i].rxvhtsgiper,
			ampdulistout->ampdus[i].rxvhtsgipercent,
			ampdulistout->ampdus[i].mpdudens, ampdulistout->ampdus[i].mpdudenspercent);
	}
}

/* Store TX MCS rates in structure */
void
store_tx_mcs_rate(tx_ampdu_list_t **ampdulistout, int rate, int percent,
	int per, int idx)
{
	(*ampdulistout)->ampdus[idx].txmcs = rate;
	(*ampdulistout)->ampdus[idx].txmcspercent = percent;
}

/* Store TX VHT rates in structure */
void
store_tx_vht_rate(tx_ampdu_list_t **ampdulistout, int rate, int percent,
	int per, int idx)
{
	(*ampdulistout)->ampdus[idx].txvht = rate;
	(*ampdulistout)->ampdus[idx].txvhtper = per;
	(*ampdulistout)->ampdus[idx].txvhtpercent = percent;
}

/* Store TX MCS with SGI rates in structure */
void
store_tx_mcs_sgi_rate(tx_ampdu_list_t **ampdulistout, int rate, int percent,
	int per, int idx)
{
	(*ampdulistout)->ampdus[idx].txmcssgi = rate;
	(*ampdulistout)->ampdus[idx].txmcssgipercent = percent;
}

/* Store TX VHT with SGI rates in structure */
void
store_tx_vht_sgi_rate(tx_ampdu_list_t **ampdulistout, int rate, int percent,
	int per, int idx)
{
	(*ampdulistout)->ampdus[idx].txvhtsgi = rate;
	(*ampdulistout)->ampdus[idx].txvhtsgiper = per;
	(*ampdulistout)->ampdus[idx].txvhtsgipercent = percent;
}

/* Store MPDU density rates in structure */
void
store_mpdu_density(tx_ampdu_list_t **ampdulistout, int rate, int percent,
	int per, int idx)
{
	(*ampdulistout)->ampdus[idx].mpdudens = rate;
	(*ampdulistout)->ampdus[idx].mpdudenspercent = percent;
}

/* Store RX MCS rates in structure */
void
store_rx_mcs_rate(tx_ampdu_list_t **ampdulistout, int rate, int percent,
	int per, int idx)
{
	(*ampdulistout)->ampdus[idx].rxmcs = rate;
	(*ampdulistout)->ampdus[idx].rxmcspercent = percent;
}

/* Store RX VHT rates in structure */
void
store_rx_vht_rate(tx_ampdu_list_t **ampdulistout, int rate, int percent,
	int per, int idx)
{
	(*ampdulistout)->ampdus[idx].rxvht = rate;
	(*ampdulistout)->ampdus[idx].rxvhtper = per;
	(*ampdulistout)->ampdus[idx].rxvhtpercent = percent;
}

/* Store RX MCS with SGI rates in structure */
void
store_rx_mcs_sgi_rate(tx_ampdu_list_t **ampdulistout, int rate, int percent,
	int per, int idx)
{
	(*ampdulistout)->ampdus[idx].rxmcssgi = rate;
	(*ampdulistout)->ampdus[idx].rxmcssgipercent = percent;
}

/* Store RX VHT with SGI rates in structure */
void
store_rx_vht_sgi_rate(tx_ampdu_list_t **ampdulistout, int rate, int percent,
	int per, int idx)
{
	(*ampdulistout)->ampdus[idx].rxvhtsgi = rate;
	(*ampdulistout)->ampdus[idx].rxvhtsgiper = per;
	(*ampdulistout)->ampdus[idx].rxvhtsgipercent = percent;
}

/* Get the AMPDU values from the buffer */
void
get_ampdu_values(char **pch, tx_ampdu_list_t **ampdulistout,
	void (*store_rates)(tx_ampdu_list_t **, int, int, int, int))
{
	int idx = 0, found = 0;

	while (*pch != NULL && **pch != '\0') {
		int count_per_line = 0;
		found = 0;
		while (*pch != NULL && **pch != '\0' && **pch != '\n') {
			if (isdigit(**pch)) {
				int rate = 0, percent = 0, per = 0;
				int len = get_mcs_rate(*pch, &rate, &percent, &per);
				*pch += len;
				(*store_rates)(ampdulistout, rate, percent, per, idx);
				idx++;
				count_per_line++;
				if (idx >= MAX_AMPDU_ENTRIES)
					return;
				found = 1;
			}
			(*pch)++;
		}
		if (found == 0)
			break;
		(*pch)++;
		if (*pch == NULL || **pch == '\0')
			break;
		else if (isalpha(**pch))
			break;
		/* As the MCS index in structure is max 12 actually it will be less in some cases.
		 * So increment the idx to adjust to the next line
		 */
		if (count_per_line <= MAX_AMPDU_DUMP_PER_LINE)
			idx += (MAX_AMPDU_DUMP_PER_LINE - count_per_line);
	}
}

/* Checks whether is there any alphabetic character in the current line */
int
is_alphachar_in_line(char *pch)
{
	while (pch != NULL && *pch != '\0' && *pch != '\n') {
		if (isalpha(*pch))
			return 1;

		pch++;
	}

	return 0;
}

/* This will get the AMPDU stats for the DUT
 * This uses the wl dump ampdu command
 */
tx_ampdu_list_t*
wl_get_ampdu_stats(void *wl)
{
	int ret;
	char *dump_buf;
	char *pch = NULL;
	int spstreams = 0;
	int mcsindex = 0;
	int szalloclen = 0, i;
	tx_ampdu_list_t *ampdulistout = NULL;

	dump_buf = malloc(WL_DUMP_BUF_LEN + 1);
	if (dump_buf == NULL) {
		VIS_METRICS("Failed to allocate dump buffer of %d bytes\n", WL_DUMP_BUF_LEN);
		return NULL;
	}
	memset(dump_buf, 0, (WL_DUMP_BUF_LEN + 1));

	strcat(dump_buf, "ampdu");

	/* This is a "space" added at end */
	strcat(dump_buf, " ");

	ret = wlu_iovar_getbuf(wl, "dump", dump_buf, strlen(dump_buf),
		dump_buf, WL_DUMP_BUF_LEN);

	if (ret) {
		free(dump_buf);
		return NULL;
	}

	VIS_METRICS("%s\n\n", dump_buf);

	/* Parse AMPDU Statistics */
	szalloclen = (sizeof(tx_ampdu_list_t) + (sizeof(tx_ampdu_t) * (MAX_AMPDU_ENTRIES)));
	ampdulistout = (tx_ampdu_list_t*)malloc(szalloclen);
	if (ampdulistout == NULL) {
		VIS_METRICS("Failed to allocate ampdulistout buffer of sz %d\n", szalloclen);
		if (dump_buf)
			free(dump_buf);
		return NULL;
	}
	memset(ampdulistout, 0x00, szalloclen);
	ampdulistout->version = 1;
	ampdulistout->length = MAX_AMPDU_ENTRIES;
	ampdulistout->timestamp = g_timestamp;

	for (i = 0; i < MAX_AMPDU_ENTRIES; i++) {
		if ((i % MAX_AMPDU_DUMP_PER_LINE) == 0) {
			mcsindex = 0;
			spstreams++;
		}
		ampdulistout->ampdus[i].streams = spstreams;
		ampdulistout->ampdus[i].mcs = mcsindex++;
	}

	if ((pch = strstr(dump_buf, STR_TX_MCS)) != NULL) {
		if (!is_alphachar_in_line((pch + strlen(STR_TX_MCS)))) {
			get_ampdu_values(&pch, &ampdulistout, store_tx_mcs_rate);
		}
	}

	if ((pch = strstr(dump_buf, STR_TX_VHT)) != NULL) {
		if (!is_alphachar_in_line((pch + strlen(STR_TX_VHT)))) {
			get_ampdu_values(&pch, &ampdulistout, store_tx_vht_rate);
		}
	}

	if ((pch = strstr(dump_buf, STR_MPDUDENS)) != NULL) {
		if (!is_alphachar_in_line((pch + strlen(STR_MPDUDENS)))) {
			get_ampdu_values(&pch, &ampdulistout, store_mpdu_density);
		}
	}

	if ((pch = strstr(dump_buf, STR_TX_MCS_SGI)) != NULL) {
		if (!is_alphachar_in_line((pch + strlen(STR_TX_MCS_SGI)))) {
			get_ampdu_values(&pch, &ampdulistout, store_tx_mcs_sgi_rate);
		}
	}

	if ((pch = strstr(dump_buf, STR_TX_VHT_SGI)) != NULL) {
		if (!is_alphachar_in_line((pch + strlen(STR_TX_VHT_SGI)))) {
			get_ampdu_values(&pch, &ampdulistout, store_tx_vht_sgi_rate);
		}
	}

	if ((pch = strstr(dump_buf, STR_RX_MCS)) != NULL) {
		if (!is_alphachar_in_line((pch + strlen(STR_RX_MCS)))) {
			get_ampdu_values(&pch, &ampdulistout, store_rx_mcs_rate);
		}
	}

	if ((pch = strstr(dump_buf, STR_RX_VHT)) != NULL) {
		if (!is_alphachar_in_line((pch + strlen(STR_RX_VHT)))) {
			get_ampdu_values(&pch, &ampdulistout, store_rx_vht_rate);
		}
	}

	if ((pch = strstr(dump_buf, STR_RX_MCS_SGI)) != NULL) {
		if (!is_alphachar_in_line((pch + strlen(STR_RX_MCS_SGI)))) {
			get_ampdu_values(&pch, &ampdulistout, store_rx_mcs_sgi_rate);
		}
	}

	if ((pch = strstr(dump_buf, STR_RX_VHT_SGI)) != NULL) {
		if (!is_alphachar_in_line((pch + strlen(STR_RX_VHT_SGI)))) {
			get_ampdu_values(&pch, &ampdulistout, store_rx_vht_sgi_rate);
		}
	}

	if (vis_debug_level & VIS_DEBUG_METRICS)
		print_ampdu(ampdulistout);

	free(dump_buf);

	return ampdulistout;
}
