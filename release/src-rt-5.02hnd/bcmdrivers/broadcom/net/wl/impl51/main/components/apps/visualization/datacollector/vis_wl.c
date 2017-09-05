/*
 * Site survey statistics for visualization tool
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
 * $Id: vis_wl.c 611403 2016-01-11 06:30:12Z $
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

#define TYPEDEF_FLOAT_T
#include <math.h>
#include <typedefs.h>
#include <bcmwifi_rates.h>

#include <bcmendian.h>
#include <bcmsrom_fmt.h>
#include "vis_common.h"
#include "vis_wl.h"
#if defined(WLPFN) && defined(linux)
#ifndef TARGETENV_android
#include <unistd.h>
#endif
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>
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
#include <unistd.h>
#endif /* WIN32 */
#endif /* SERDOWNLOAD || defined CLMDOWNLOAD */

#if LCNCONF || SSLPNCONF
#define MAX_CHUNK_LEN 1456  /* 8 * 7 * 26 */
#else
#define MAX_CHUNK_LEN 1408 /* 8 * 8 * 22 */
#endif

/* For backwards compatibility, the absense of the define 'NO_FILESYSTEM_SUPPORT'
 * implies that a filesystem is supported.
 */
#if !defined(BWL_NO_FILESYSTEM_SUPPORT)
#define BWL_FILESYSTEM_SUPPORT
#endif

/* some OSes (FC4) have trouble allocating (kmalloc) 128KB worth of memory,
 * hence keeping WL_DUMP_BUF_LEN below that
 */
#if defined(BWL_SMALL_WLU_DUMP_BUF)
#define WL_DUMP_BUF_LEN (4 * 1024)
#else
#define WL_DUMP_BUF_LEN (127 * 1024)
#endif 

#if !defined(WL_RATESET_SZ_VHT_MCS_P)
#define WL_RATESET_SZ_VHT_MCS_P	12
#endif /* WL_RATESET_SZ_VHT_MCS_P */

/* 802.11i/WPA RSN IE parsing utilities */
typedef struct {
	uint16 version;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *akm;
	uint8 *capabilities;
} rsn_parse_info_t;

static networks_list_t *dump_networks(void *wl, char *buf);
static void dump_bss_info(void *wl, wl_bss_info_t *bi, int idx, networks_list_t *networks_listin);
static void wl_dump_wpa_rsn_ies(uint8* cp, uint len, char *mcastrsntype,
	char *ucastrsntype, char *akmrsntype);
static void wl_rsn_ie_dump(bcm_tlv_t *ie, char *mcastrsntype,
	char *ucastrsntype, char *akmrsntype);

static int wl_rsn_ie_parse_info(uint8* buf, uint len, rsn_parse_info_t *rsn);

extern networks_list_t	*g_networks_list;
extern networks_list_t	*g_networks_list_1;

extern long g_timestamp;

extern char *g_vis_cmdoutbuf;

static void get_ucast_rsn_string(int type, char *ucastrsntype);
static void get_akm_rsn_string(int type, int rsn, char *akmrsntype);

void
syserr(char *s)
{
	VIS_ADAPTER("%s: ", g_wlu_av0);
	VIS_ADAPTER("Error in %s : %s\n", s, strerror(errno));
}

/* Checks whether driver is present or not */
int
wl_check(void *wl)
{
	int ret;
	int val = 0;

	if ((ret = wlu_get(wl, WLC_GET_MAGIC, &val, sizeof(int))) < 0)
		return ret;

	/* Detect if IOCTL swapping is necessary */
	if (val == (int)bcmswap32(WLC_IOCTL_MAGIC)) {
		val = bcmswap32(val);
		g_swap = TRUE;
	}
	if (val != WLC_IOCTL_MAGIC)
		return -1;
	if ((ret = wlu_get(wl, WLC_GET_VERSION, &val, sizeof(int))) < 0)
		return ret;
	ioctl_version = dtoh32(val);
	if (ioctl_version != WLC_IOCTL_VERSION &&
	    ioctl_version != 1) {
		VIS_ADAPTER("Version mismatch, please upgrade. Got %d, expected %d or 1\n",
		        ioctl_version, WLC_IOCTL_VERSION);
		return -1;
	}
	return 0;
}

int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
	struct ifreq *ifr = (struct ifreq *) wl;
	wl_ioctl_t ioc;
	int ret = 0;
	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	/* do it */
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	ioc.set = set;
	ifr->ifr_data = (caddr_t) &ioc;
	if ((ret = ioctl(s, SIOCDEVPRIVATE, ifr)) < 0) {
		VIS_ADAPTER("Error in ioctl : %s\n", strerror(errno));
		if (cmd != WLC_GET_MAGIC) {
			ret = IOCTL_ERROR;
		}
	}

	/* cleanup */
	close(s);

	return ret;
}

#ifndef ATE_BUILD
/* Returns the address in string format */
char
*wl_ether_etoa(const struct ether_addr *n)
{
	static char etoa_buf[ETHER_ADDR_LEN * 3];
	char *c = etoa_buf;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", n->octet[i] & 0xff);
	}
	return etoa_buf;
}
#endif /* ATE_BUILD */

/* Validates and parses the RSN or WPA IE contents into a rsn_parse_info_t structure
 * Returns 0 on success, or 1 if the information in the buffer is not consistant with
 * an RSN IE or WPA IE.
 * The buf pointer passed in should be pointing at the version field in either an RSN IE
 * or WPA IE.
 */
static int
wl_rsn_ie_parse_info(uint8* rsn_buf, uint len, rsn_parse_info_t *rsn)
{
	uint16 count;

	memset(rsn, 0, sizeof(rsn_parse_info_t));

	/* version */
	if (len < sizeof(uint16))
		return 1;

	rsn->version = ltoh16_ua(rsn_buf);
	len -= sizeof(uint16);
	rsn_buf += sizeof(uint16);

	/* Multicast Suite */
	if (len < sizeof(wpa_suite_mcast_t))
		return 0;

	rsn->mcast = (wpa_suite_mcast_t*)rsn_buf;
	len -= sizeof(wpa_suite_mcast_t);
	rsn_buf += sizeof(wpa_suite_mcast_t);

	/* Unicast Suite */
	if (len < sizeof(uint16))
		return 0;

	count = ltoh16_ua(rsn_buf);

	if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
		return 1;

	rsn->ucast = (wpa_suite_ucast_t*)rsn_buf;
	len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
	rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

	/* AKM Suite */
	if (len < sizeof(uint16))
		return 0;

	count = ltoh16_ua(rsn_buf);

	if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
		return 1;

	rsn->akm = (wpa_suite_auth_key_mgmt_t*)rsn_buf;
	len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
	rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

	/* Capabilites */
	if (len < sizeof(uint16))
		return 0;

	rsn->capabilities = rsn_buf;

	return 0;
}

/*
 * Traverse a string of 1-byte tag/1-byte length/variable-length value
 * triples, returning a pointer to the substring whose first element
 * matches tag
 */
static uint8
*wlu_parse_tlvs(uint8 *tlv_buf, int buflen, uint key)
{
	uint8 *cp;
	int totlen;

	cp = tlv_buf;
	totlen = buflen;

	/* find tagged parameter */
	while (totlen >= 2) {
		uint tag;
		int len;

		tag = *cp;
		len = *(cp +1);

		/* validate remaining totlen */
		if ((tag == key) && (totlen >= (len + 2)))
			return (cp);

		cp += (len + 2);
		totlen -= (len + 2);
	}

	return NULL;
}

/* Does a memcompare */
static int
wlu_bcmp(const void *b1, const void *b2, int len)
{
	return (memcmp(b1, b2, len));
}

/* Is this body of this tlvs entry a WPA entry? If */
/* not update the tlvs buffer pointer/length */
static bool
wlu_is_wpa_ie(uint8 **wpaie, uint8 **tlvs, uint *tlvs_len)
{
	uint8 *ie = *wpaie;

	/* If the contents match the WPA_OUI and type=1 */
	if ((ie[1] >= 6) && !wlu_bcmp(&ie[2], WPA_OUI "\x01", 4)) {
		return TRUE;
	}

	/* point to the next ie */
	ie += ie[1] + 2;
	/* calculate the length of the rest of the buffer */
	*tlvs_len -= (int)(ie - *tlvs);
	/* update the pointer to the start of the buffer */
	*tlvs = ie;

	return FALSE;
}

/* Parses WPA RSN IE */
static void
wl_dump_wpa_rsn_ies(uint8* cp, uint len, char *mcastrsntype,
	char *ucastrsntype, char *akmrsntype)
{
	uint8 *parse = cp;
	uint parse_len = len;
	uint8 *wpaie;
	uint8 *rsnie;

	while ((wpaie = wlu_parse_tlvs(parse, parse_len, DOT11_MNG_WPA_ID)))
		if (wlu_is_wpa_ie(&wpaie, &parse, &parse_len))
			break;
	if (wpaie)
		wl_rsn_ie_dump((bcm_tlv_t*)wpaie, mcastrsntype, ucastrsntype, akmrsntype);

	rsnie = wlu_parse_tlvs(cp, len, DOT11_MNG_RSN_ID);
	if (rsnie)
		wl_rsn_ie_dump((bcm_tlv_t*)rsnie, mcastrsntype, ucastrsntype, akmrsntype);

	return;
}

/* Parses RSN IE */
static void
wl_rsn_ie_dump(bcm_tlv_t *ie, char *mcastrsntype, char *ucastrsntype, char *akmrsntype)
{
	int i;
	int rsn;
	wpa_ie_fixed_t *wpa = NULL;
	rsn_parse_info_t rsn_info;
	wpa_suite_t *suite;
	uint8 std_oui[3];
	int unicast_count = 0;
	int akm_count = 0;
	int err;

	if (ie->id == DOT11_MNG_RSN_ID) {
		rsn = TRUE;
		memcpy(std_oui, WPA2_OUI, WPA_OUI_LEN);
		err = wl_rsn_ie_parse_info(ie->data, ie->len, &rsn_info);
	} else {
		rsn = FALSE;
		memcpy(std_oui, WPA_OUI, WPA_OUI_LEN);
		wpa = (wpa_ie_fixed_t*)ie;
		err = wl_rsn_ie_parse_info((uint8*)&wpa->version, wpa->length - WPA_IE_OUITYPE_LEN,
			&rsn_info);
	}

	if (err || rsn_info.version != WPA_VERSION)
		return;

	/* Check for unicast suite(s) */
	if (rsn_info.ucast) {
		unicast_count = ltoh16_ua(&rsn_info.ucast->count);
		for (i = 0; i < unicast_count; i++) {
			suite = &rsn_info.ucast->list[i];
			if (!wlu_bcmp(suite->oui, std_oui, 3)) {
				get_ucast_rsn_string(suite->type, ucastrsntype);
			}
		}
	}
	/* Authentication Key Management */
	if (rsn_info.akm) {
		akm_count = ltoh16_ua(&rsn_info.akm->count);
		for (i = 0; i < akm_count; i++) {
			suite = &rsn_info.akm->list[i];
			if (!wlu_bcmp(suite->oui, std_oui, 3)) {
				get_akm_rsn_string(suite->type, rsn, akmrsntype);
			}
		}
	}
}

/* Prepares for SCAN */
static int
wl_scan_prep(void *wl, wl_scan_params_t *params, int *params_size)
{
	int err = 0;
	int nchan = 0;
	int nssid = 0;
	char *p;

	UNUSED_PARAMETER(wl);

	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->bss_type = DOT11_BSSTYPE_ANY;
	params->scan_type = 0;
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = -1;
	params->home_time = -1;
	params->channel_num = 0;

	params->nprobes = htod32(params->nprobes);
	params->active_time = htod32(params->active_time);
	params->passive_time = htod32(params->passive_time);
	params->home_time = htod32(params->home_time);

	p = (char*)params->channel_list + nchan * sizeof(uint16);

	params->channel_num = htod32((nssid << WL_SCAN_PARAMS_NSSID_SHIFT) |
	                             (nchan & WL_SCAN_PARAMS_COUNT_MASK));
	*params_size = p - (char*)params + nssid * sizeof(wlc_ssid_t);

	return err;
}

/* Do a scan for all the BSSID */
int
wl_scan(void *wl)
{
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + WL_NUMCHANNELS * sizeof(uint16);
	wl_scan_params_t *params;
	int err = 0;

	params_size += WL_SCAN_PARAMS_SSID_MAX * sizeof(wlc_ssid_t);
	params = (wl_scan_params_t*)malloc(params_size);
	if (params == NULL) {
		VIS_SCAN("Error allocating %d bytes for scan params\n", params_size);
		return -1;
	}
	memset(params, 0, params_size);

	err = wl_scan_prep(wl, params, &params_size);

	if (err) {
		free(params);
		return err;
	}

	err = wlu_set(wl, WLC_SCAN, params, params_size);

	free(params);

	return err;
}

/* Gets the scan result */
static int
wl_get_scan(void *wl, int opc, char *scan_buf, uint buf_len)
{
	wl_scan_results_t *list = (wl_scan_results_t*)scan_buf;
	int ret;

	list->buflen = htod32(buf_len);
	ret = wlu_get(wl, opc, scan_buf, buf_len);
	if (ret < 0)
		return ret;
	ret = 0;

	list->buflen = dtoh32(list->buflen);
	list->version = dtoh32(list->version);
	list->count = dtoh32(list->count);
	if (list->buflen == 0) {
		list->version = 0;
		list->count = 0;
	} else if (list->version != WL_BSS_INFO_VERSION &&
	           list->version != LEGACY2_WL_BSS_INFO_VERSION &&
	           list->version != LEGACY_WL_BSS_INFO_VERSION) {
		VIS_SCAN("Sorry, your driver has bss_info_version %d "
			"but this program supports only version %d.\n",
			list->version, WL_BSS_INFO_VERSION);
		list->buflen = 0;
		list->count = 0;
	}

	return ret;
}

/* Gets the scan result */
networks_list_t*
wl_dump_networks(void *wl)
{
	int ret;
	char *dump_buf, *dump_buf_orig;
	networks_list_t *tmp_network = NULL;

	dump_buf_orig = dump_buf = malloc(WL_DUMP_BUF_LEN);
	if (dump_buf == NULL) {
		VIS_SCAN("Failed to allocate dump buffer of %d bytes\n",
			WL_DUMP_BUF_LEN);
		return NULL;
	}

	ret = wl_get_scan(wl, WLC_SCAN_RESULTS, dump_buf, WL_DUMP_BUF_LEN);

	if (ret == 0) {
		tmp_network = dump_networks(wl, dump_buf);
	}

	free(dump_buf_orig);
	return tmp_network;
}

/* Pretty print the BSS list */
static networks_list_t*
dump_networks(void *wl, char *network_buf)
{
	wl_scan_results_t *list = (wl_scan_results_t*)network_buf;
	wl_bss_info_t *bi;
	networks_list_t *tmp_network = NULL;
	uint i;
	int szalloclen = (sizeof(networks_list_t) + (sizeof(ap_info_t) * list->count));

	tmp_network = (networks_list_t*)malloc(szalloclen);
	if (tmp_network == NULL) {
		VIS_SCAN("Failed to allocate networks_list buffer of sz : %d\n",
			szalloclen);
		return NULL;
	}
	memset(tmp_network, 0x00, szalloclen);
	tmp_network->length = list->count;
	tmp_network->timestamp = g_timestamp;
	if (list->count == 0)
		return tmp_network;
	else if (list->version != WL_BSS_INFO_VERSION &&
	         list->version != LEGACY2_WL_BSS_INFO_VERSION &&
	         list->version != LEGACY_WL_BSS_INFO_VERSION) {
		VIS_SCAN("Sorry, your driver has bss_info_version %d "
			"but this program supports only version %d.\n",
			list->version, WL_BSS_INFO_VERSION);
		tmp_network->length = 0;
		return tmp_network;
	}

	bi = list->bss_info;
	for (i = 0; i < list->count; i++, bi = (wl_bss_info_t*)((int8*)bi + dtoh32(bi->length))) {
		dump_bss_info(wl, bi, i, tmp_network);
	}

	return tmp_network;
}

/* Formats the SSID */
static int
wl_format_ssid(char* ssid_buf, uint8* ssid, int ssid_len)
{
	int i, c;
	char *p = ssid_buf;

	if (ssid_len > 32) ssid_len = 32;

	for (i = 0; i < ssid_len; i++) {
		c = (int)ssid[i];
		if (c == '\\') {
			*p++ = '\\';
			*p++ = '\\';
		} else if (isprint((uchar)c)) {
			*p++ = (char)c;
		} else {
			p += sprintf(p, "\\x%02X", c);
		}
	}
	*p = '\0';

	return p - ssid_buf;
}

/* Checks whether the type is B or G */
static void
is_b_g(int rate, int *has_b, int *has_g)
{
	switch (rate) {
		case 1:
		case 2:
		case 5:
		case 11:
			*has_b = TRUE;
		break;
		default:
			*has_g = TRUE;
		break;
	}
}

/* Dumps the Unicast RSN string from the type to structure */
static void
get_ucast_rsn_string(int type, char *ucastrsntype)
{
	switch (type) {
		case WPA_CIPHER_NONE:
			snprintf(ucastrsntype+strlen(ucastrsntype),
				MAX_UNICAST_RSN-strlen(ucastrsntype), "NONE ");
			break;
		case WPA_CIPHER_WEP_40:
			snprintf(ucastrsntype+strlen(ucastrsntype),
				MAX_UNICAST_RSN-strlen(ucastrsntype), "WEP64 ");
			break;
		case WPA_CIPHER_WEP_104:
			snprintf(ucastrsntype+strlen(ucastrsntype),
				MAX_UNICAST_RSN-strlen(ucastrsntype), "WEP128 ");
			break;
		case WPA_CIPHER_TKIP:
			snprintf(ucastrsntype+strlen(ucastrsntype),
				MAX_UNICAST_RSN-strlen(ucastrsntype), "TKIP ");
			break;
		case WPA_CIPHER_AES_OCB:
			snprintf(ucastrsntype+strlen(ucastrsntype),
				MAX_UNICAST_RSN-strlen(ucastrsntype), "AES-OCB ");
			break;
		case WPA_CIPHER_AES_CCM:
			snprintf(ucastrsntype+strlen(ucastrsntype),
				MAX_UNICAST_RSN-strlen(ucastrsntype), "AES-CCMP ");
			break;
	}
}

/* Dumps the RSN AKM string from the type to structure */
static void
get_akm_rsn_string(int type, int rsn, char *akmrsntype)
{
	switch (type) {
		case RSN_AKM_NONE:
			snprintf(akmrsntype+strlen(akmrsntype), MAX_AKM_TYPE-strlen(akmrsntype),
				"NONE ");
			break;
		case RSN_AKM_UNSPECIFIED:
			snprintf(akmrsntype+strlen(akmrsntype), MAX_AKM_TYPE-strlen(akmrsntype),
				"%s ", rsn ? "WPA2" : "WPA");
			break;
		case RSN_AKM_PSK:
			snprintf(akmrsntype+strlen(akmrsntype), MAX_AKM_TYPE-strlen(akmrsntype),
				"%s ", rsn ? "WPA2-PSK" : "WPA-PSK");
			break;
		case RSN_AKM_FBT_1X:
			snprintf(akmrsntype+strlen(akmrsntype), MAX_AKM_TYPE-strlen(akmrsntype),
				"FT-802.1X ");
			break;
		case RSN_AKM_FBT_PSK:
			snprintf(akmrsntype+strlen(akmrsntype), MAX_AKM_TYPE-strlen(akmrsntype),
				"FT-PSK ");
			break;
	}

}

struct d11_mcs_rate_info {
	uint8 constellation_bits;
	uint8 coding_q;
	uint8 coding_d;
};

static const struct d11_mcs_rate_info wlu_mcs_info[] = {
	{ 1, 1, 2 }, /* MCS  0: MOD: BPSK,   CR 1/2 */
	{ 2, 1, 2 }, /* MCS  1: MOD: QPSK,   CR 1/2 */
	{ 2, 3, 4 }, /* MCS  2: MOD: QPSK,   CR 3/4 */
	{ 4, 1, 2 }, /* MCS  3: MOD: 16QAM,  CR 1/2 */
	{ 4, 3, 4 }, /* MCS  4: MOD: 16QAM,  CR 3/4 */
	{ 6, 2, 3 }, /* MCS  5: MOD: 64QAM,  CR 2/3 */
	{ 6, 3, 4 }, /* MCS  6: MOD: 64QAM,  CR 3/4 */
	{ 6, 5, 6 }, /* MCS  7: MOD: 64QAM,  CR 5/6 */
	{ 8, 3, 4 }, /* MCS  8: MOD: 256QAM, CR 3/4 */
	{ 8, 5, 6 }  /* MCS  9: MOD: 256QAM, CR 5/6 */
};

static uint
wl_mcs2rate(uint mcs, uint nss, uint bw, int sgi)
{
	const int ksps = 250; /* kilo symbols per sec, 4 us sym */
	const int Nsd_20MHz = 52;
	const int Nsd_40MHz = 108;
	const int Nsd_80MHz = 234;
	const int Nsd_160MHz = 468;
	uint rate;

	if (mcs == 32) {
		/* just return fixed values for mcs32 instead of trying to parametrize */
		rate = (sgi == 0) ? 6000 : 6700;
	} else if (mcs < WL_RATESET_SZ_VHT_MCS_P) {
		/* This calculation works for 11n HT and 11ac VHT if the HT mcs values
		 * are decomposed into a base MCS = MCS % 8, and Nss = 1 + MCS / 8.
		 * That is, HT MCS 23 is a base MCS = 7, Nss = 3
		 */

		/* find the number of complex numbers per symbol */
		if (bw == 20) {
			rate = Nsd_20MHz;
		} else if (bw == 40) {
			rate = Nsd_40MHz;
		} else if (bw == 80) {
			rate = Nsd_80MHz;
		} else if (bw == 160) {
			rate = Nsd_160MHz;
		} else {
			rate = 1;
		}

		/* multiply by bits per number from the constellation in use */
		rate = rate * wlu_mcs_info[mcs].constellation_bits;

		/* adjust for the number of spatial streams */
		rate = rate * nss;

		/* adjust for the coding rate given as a quotient and divisor */
		rate = (rate * wlu_mcs_info[mcs].coding_q) / wlu_mcs_info[mcs].coding_d;

		/* multiply by Kilo symbols per sec to get Kbps */
		rate = rate * ksps;

		/* adjust the symbols per sec for SGI
		 * symbol duration is 4 us without SGI, and 3.6 us with SGI,
		 * so ratio is 10 / 9
		 */
		if (sgi) {
			/* add 4 for rounding of division by 9 */
			rate = ((rate * 10) + 4) / 9;
		}
	} else {
		rate = 0;
	}

	return rate;
}

/* get the Max rate */
static void
get_max_rate(void *wl, wl_bss_info_t *bi, bool ntypefound, int *has_b, int *has_g,
	int *outmaxrate, bool currentdut)
{
	int rate = 0, maxrate = 0, tmprate = 0;
	int i = 0, mcs_idx = 0;
	int nbw = 0;
	int mcs = 0, sgi = 0, isht = 0;

	if (CHSPEC_IS80(bi->chanspec))
		nbw = 80;
	else if (CHSPEC_IS40(bi->chanspec))
		nbw = 40;
	else if (CHSPEC_IS20(bi->chanspec))
		nbw = 20;
	if (currentdut == TRUE) {
		if (wlu_iovar_get(wl, "sgi_tx", &sgi, sizeof(sgi)) < 0) {
			sgi = 0;
		}
		if (sgi != 0) {
			sgi = 1;
		}
	}
	/* Copy the supported rates */
	for (i = 0; i < bi->rateset.count; i++) {
		if (bi->rateset.rates[i]) {
			tmprate = ((bi->rateset.rates[i])&0x7F)/2;
			if (tmprate > maxrate)
				maxrate = tmprate;

			if (ntypefound == FALSE) {
				is_b_g(tmprate, has_b, has_g);
			}
		}
	}

	if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap) {
		if (bi->vht_cap) {
			int nss = 0, rate;
			uint mcs_cap, mcs_cap_map;

			for (i = 1; i <= VHT_CAP_MCS_MAP_NSS_MAX; i++) {
				mcs_cap = VHT_MCS_MAP_GET_MCS_PER_SS(i,
					ltoh16(bi->vht_txmcsmap));
				if (mcs_cap != VHT_CAP_MCS_MAP_NONE) {
					nss++; /* Calculate the number of streams */
					mcs_cap_map = mcs_cap; /* Max valid mcs cap map */
				}
			}

			if (nss) {
				if (mcs_cap_map == VHT_CAP_MCS_MAP_0_7)
					mcs = VHT_CAP_MCS_MAP_0_7_MAX_IDX - 1;
				else if (mcs_cap_map == VHT_CAP_MCS_MAP_0_8)
					mcs = VHT_CAP_MCS_MAP_0_8_MAX_IDX - 1;
				else if (mcs_cap_map == VHT_CAP_MCS_MAP_0_9)
					mcs = VHT_CAP_MCS_MAP_0_9_MAX_IDX - 1;

				if ((currentdut == FALSE) && (bi->nbss_cap & VHT_BI_SGI_80MHZ)) {
					sgi = 1;
				}
				rate = wl_mcs2rate(mcs, nss, nbw, sgi);
				tmprate = rate/1000;
				if (tmprate > maxrate)
					maxrate = tmprate;
				if (ntypefound == FALSE) {
					is_b_g(tmprate, has_b, has_g);
				}
			}
		}
		if (currentdut == FALSE) {
			sgi = 0;
		}
		/* For 802.11n networks, use MCS table */
		for (mcs_idx = 0; mcs_idx < (MCSSET_LEN * 8); mcs_idx++) {
			if (isset(bi->basic_mcs, mcs_idx) && mcs_idx < MCS_TABLE_SIZE) {
				mcs = mcs_idx;
				/* Check for short guard interval support
				 */
				if (currentdut == FALSE) {
					if ((nbw && (bi->nbss_cap & HT_CAP_SHORT_GI_40)) ||
						(!nbw && (bi->nbss_cap & HT_CAP_SHORT_GI_20))) {
						sgi = 1;
					}
				}
				tmprate = rate/1000;
				if (tmprate > maxrate)
					maxrate = tmprate;
				if (ntypefound == FALSE) {
					is_b_g(tmprate, has_b, has_g);
				}
				isht = 1;
			}
		}

		if (isht) {
			int nss = 0;

			if (mcs > 32) {
				VIS_SCAN("MCS is Out of range \n");
			} else if (mcs == 32) {
				rate = wl_mcs2rate(mcs, 1, nbw, sgi);
			} else {
				nss = 1 + (mcs / 8);
				mcs = mcs % 8;
				rate = wl_mcs2rate(mcs, nss, nbw, sgi);
			}
			tmprate = rate/1000;
			if (tmprate > maxrate)
				maxrate = tmprate;
		}
	}
	*outmaxrate = maxrate;
}

/* Dumps the scan result into a structure */
static void
dump_bss_info(void *wl, wl_bss_info_t *bi, int idx, networks_list_t *networks_listin)
{
	char ssidbuf[SSID_FMT_BUF_LEN];
	char chspec_str[CHANSPEC_STR_LEN];
	wl_bss_info_107_t *old_bi;
	int networktype;
	int isABand, controlch = 0;
	bool ntypefound = FALSE;
	int has_b = FALSE, has_g = FALSE;
	int maxrate = 0;

	/* Convert version 107 to 109 */
	if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
		old_bi = (wl_bss_info_107_t *)bi;
		bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
		bi->ie_length = old_bi->ie_length;
		bi->ie_offset = sizeof(wl_bss_info_107_t);
	} else {
		/* do endian swap and format conversion for chanspec if we have
		 * not created it from legacy bi above
		 */
		bi->chanspec = wl_chspec_from_driver(bi->chanspec);
	}

	wl_format_ssid(ssidbuf, bi->SSID, bi->SSID_len);
	snprintf(networks_listin->aps[idx].ssid, SSID_FMT_BUF_LEN, "%s", ssidbuf);

	networks_listin->aps[idx].rssi = (int16)(dtoh16(bi->RSSI));

	networks_listin->aps[idx].noise = bi->phy_noise;

	snprintf(networks_listin->aps[idx].bssid, (ETHER_ADDR_LEN * 3),
		"%s", wl_ether_etoa(&bi->BSSID));

	if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap) {
		controlch = bi->ctl_ch;
		networks_listin->aps[idx].band = CHSPEC_IS2G(bi->chanspec)?2:5;
		networks_listin->aps[idx].bandwidth = (CHSPEC_IS80(bi->chanspec) ?
		        80 : (CHSPEC_IS40(bi->chanspec) ?
		              40 : (CHSPEC_IS20(bi->chanspec) ? 20 : 10)));
	} else {
		controlch = atoi(wf_chspec_ntoa(bi->chanspec, chspec_str));
		networks_listin->aps[idx].bandwidth = (CHSPEC_IS80(bi->chanspec) ?
			80 : (CHSPEC_IS40(bi->chanspec) ?
			40 : (CHSPEC_IS20(bi->chanspec) ? 20 : 10)));
	}

	if (networks_listin->aps[idx].bandwidth > 20) {
		networks_listin->aps[idx].ctrlch = controlch;

		get_center_channels(bi->chanspec, controlch, &networks_listin->aps[idx].channel,
			networks_listin->aps[idx].bandwidth);
	} else {
		networks_listin->aps[idx].channel = controlch;
		networks_listin->aps[idx].ctrlch = controlch;
	}

	VIS_SCAN("SSID: %s BSSID : %s Channel : %d Center CH : %d Bandwidth : %d\n",
		networks_listin->aps[idx].ssid, networks_listin->aps[idx].bssid,
		networks_listin->aps[idx].ctrlch, networks_listin->aps[idx].channel,
		networks_listin->aps[idx].bandwidth);

	isABand = networks_listin->aps[idx].channel > 14;
	if (bi->n_cap) { /* 802.11n */
		networktype = NETWORK_TYPE_N;
		if (isABand)
			networktype |= (NETWORK_TYPE_A);
		else
			networktype |= (NETWORK_TYPE_B|NETWORK_TYPE_G);
		ntypefound = TRUE;
	} else if (isABand) {
		networktype = NETWORK_TYPE_A;
		ntypefound = TRUE;
	}

	get_max_rate(wl, bi, ntypefound, &has_b, &has_g, &maxrate, FALSE);
	networks_listin->aps[idx].maxrate = maxrate;
	if (ntypefound == FALSE) {
		if (has_b && has_g)
			networktype = NETWORK_TYPE_B|NETWORK_TYPE_G; /* _T("802.11b & 802.11g"); */
		else if (has_b)
			networktype = NETWORK_TYPE_B; /* _T("802.11b"); */
		else if (has_g)
			networktype = NETWORK_TYPE_G; /* _T("802.11g"); */
		else
			networktype = 0; /* TranslateString(_T("<unknown>")); */
	}
	if (networktype & NETWORK_TYPE_A)
		strcat(networks_listin->aps[idx].networktype, "a");
	if (networktype & NETWORK_TYPE_B)
		strcat(networks_listin->aps[idx].networktype, "b");
	if (networktype & NETWORK_TYPE_G)
		strcat(networks_listin->aps[idx].networktype, "g");
	if (networktype & NETWORK_TYPE_N)
		strcat(networks_listin->aps[idx].networktype, "n");
	if (networks_listin->aps[idx].bandwidth >= 80)
		strcat(networks_listin->aps[idx].networktype, " ac");
	/* Get RSN Details */
	if (dtoh32(bi->ie_length)) {
		wl_dump_wpa_rsn_ies((uint8 *)(((uint8 *)bi) + dtoh16(bi->ie_offset)),
			dtoh32(bi->ie_length),
			networks_listin->aps[idx].mcastrsntype,
			networks_listin->aps[idx].ucastrsntype,
			networks_listin->aps[idx].akmrsntype);
	}
}

#ifndef ATE_BUILD
/* Checks is AP */
int
wl_is_AP(void *wl, dut_info_t *dut_info)
{
	int ret;
	int val = 0;

	if ((ret = wlu_get(wl, WLC_GET_AP, &val, sizeof(int))) < 0)
		return ret;

	val = dtoh32(val);
	dut_info->isAP = (uint32)val;

	return ret;
}
#endif /* !ATE_BUILD */

/* Gets the DUT info */
static int
get_dut_info_from_bss(void *wl, wl_bss_info_t *bi, dut_info_t *dut_info)
{
	char chspec_str[CHANSPEC_STR_LEN];
	wl_bss_info_107_t *old_bi;
	uint32 isABand, controlch;
	int maxrate = 0;
	int networktype;
	bool ntypefound = FALSE;
	int has_b = FALSE, has_g = FALSE;

	memset(dut_info, 0x00, sizeof(dut_info_t));

	/* Convert version 107 to 109 */
	if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
		old_bi = (wl_bss_info_107_t *)bi;
		bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
		bi->ie_length = old_bi->ie_length;
		bi->ie_offset = sizeof(wl_bss_info_107_t);
	} else {
		/* do endian swap and format conversion for chanspec if we have
		 * not created it from legacy bi above
		 */
		bi->chanspec = wl_chspec_from_driver(bi->chanspec);
	}

	wl_format_ssid(dut_info->ssid, bi->SSID, bi->SSID_len);

	dut_info->rssi = (int16)(dtoh16(bi->RSSI));

	dut_info->noise = bi->phy_noise;

	snprintf(dut_info->bssid, (ETHER_ADDR_LEN * 3), "%s", wl_ether_etoa(&bi->BSSID));

	if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap) {
		controlch = bi->ctl_ch;
		dut_info->band = CHSPEC_IS2G(bi->chanspec)?2:5;
	} else {
		controlch = atoi(wf_chspec_ntoa(bi->chanspec, chspec_str));
		dut_info->band = 2;
	}

	dut_info->bandwidth = (CHSPEC_IS80(bi->chanspec) ?
		80 : (CHSPEC_IS40(bi->chanspec) ?
		40 : (CHSPEC_IS20(bi->chanspec) ? 20 : 10)));

	if (dut_info->bandwidth > 20) {
		dut_info->ctrlch = controlch;

		get_center_channels(bi->chanspec, controlch, &dut_info->channel,
			dut_info->bandwidth);
	} else {
		dut_info->channel = controlch;
		dut_info->ctrlch = controlch;
	}

	isABand = dut_info->channel > 14;
	if (bi->n_cap) { /* 802.11n */
		networktype = NETWORK_TYPE_N;
		if (isABand)
			networktype |= (NETWORK_TYPE_A);
		else
			networktype |= (NETWORK_TYPE_B|NETWORK_TYPE_G);
		ntypefound = TRUE;
	} else if (isABand) {
		networktype = NETWORK_TYPE_A;
		ntypefound = TRUE;
	}

	get_max_rate(wl, bi, ntypefound, &has_b, &has_g, &maxrate, TRUE);
	dut_info->maxrate = maxrate;
	if (ntypefound == FALSE) {
		if (has_b && has_g)
			networktype = NETWORK_TYPE_B|NETWORK_TYPE_G; /* _T("802.11b & 802.11g"); */
		else if (has_b)
			networktype = NETWORK_TYPE_B; /* _T("802.11b"); */
		else if (has_g)
			networktype = NETWORK_TYPE_G; /* _T("802.11g"); */
		else
			networktype = 0; /* TranslateString(_T("<unknown>")); */
	}
	memset(dut_info->networktype, 0x00, MAX_NETWORK_TYPE);
	if (networktype & NETWORK_TYPE_A)
		strcat(dut_info->networktype, "a");
	if (networktype & NETWORK_TYPE_B)
		strcat(dut_info->networktype, "b");
	if (networktype & NETWORK_TYPE_G)
		strcat(dut_info->networktype, "g");
	if (networktype & NETWORK_TYPE_N)
		strcat(dut_info->networktype, "n");
	if (dut_info->bandwidth >= 80)
		strcat(dut_info->networktype, " ac");
	memset(dut_info->mcastrsntype, 0x00, MAX_MULTICAST_RSN);
	memset(dut_info->ucastrsntype, 0x00, MAX_UNICAST_RSN);
	memset(dut_info->akmrsntype, 0x00, MAX_AKM_TYPE);

	VIS_SCAN("SSID : %s\tChannel : %d\tBandwidth : %d\n",
		dut_info->bssid, dut_info->channel,
		dut_info->bandwidth);

	return 0;
}

static int
vis_get_wsec(void *wl, char *ucastrsntype)
{
	int ret = 0, i = 0, curpos = 0;
	uint32 wsec = 0;
	static struct {
		int val;
		const char *name;
	} wsec_mode[] =
		{{WEP_ENABLED,	"WEP"},
		{TKIP_ENABLED,	"TKIP"},
		{AES_ENABLED,	"AES"}};

	if ((ret = wlu_get(wl, WLC_GET_WSEC, &wsec, sizeof(uint32))) == 0) {
		for (i = 0; i < (int)ARRAYSIZE(wsec_mode); i++) {
			if (wsec & wsec_mode[i].val)
				curpos += snprintf(ucastrsntype + curpos, MAX_UNICAST_RSN - curpos,
					"%s ", wsec_mode[i].name);
		}
	}

	return ret;
}

#if !defined(WPA2_AUTH_1X_SHA256)
#define WPA2_AUTH_1X_SHA256     0x1000  /* 1X with SHA256 key derivation */
#endif /* WPA2_AUTH_1X_SHA256 */

#if !defined(WPA2_AUTH_PSK_SHA256)
#define WPA2_AUTH_PSK_SHA256    0x8000  /* PSK with SHA256 key derivation */
#endif /* WPA2_AUTH_PSK_SHA256 */

static int
vis_get_wpa_auth(void *wl, char *akmrsntype)
{
	int wpa_auth = 0;
	int ret = 0;
	int i, curpos = 0;
	static struct {
		int val;
		const char *name;
	} auth_mode[] =
		{{WPA_AUTH_NONE,	"WPA-NONE"},
		{WPA_AUTH_UNSPECIFIED,	"WPA-802.1x"},
		{WPA_AUTH_PSK,		"WPA-PSK"},
		{WPA2_AUTH_UNSPECIFIED, "WPA2-802.1x"},
		{WPA2_AUTH_PSK,	"WPA2-PSK"},
		{WPA2_AUTH_1X_SHA256,	"1X-SHA256"},
		{WPA2_AUTH_FT,		"FT"},
		{WPA2_AUTH_PSK_SHA256,	"PSK-SHA256"},
		{WPA_AUTH_DISABLED,	"disabled"}};

	if ((ret = wlu_iovar_get(wl, "wpa_auth", &wpa_auth, sizeof(uint32))) == 0) {
		for (i = 0; i < (int)ARRAYSIZE(auth_mode); i++) {
			if (wpa_auth & auth_mode[i].val)
				curpos += snprintf(akmrsntype + curpos, MAX_AKM_TYPE - curpos,
					"%s ", auth_mode[i].name);
		}
	}

	return ret;
}

/* Gets DUT info */
int
wl_get_bss_info(void *wl, dut_info_t *dut_info)
{
	int ret;
	struct ether_addr bssid;
	wl_bss_info_t *bi;

	if ((ret = wlu_get(wl, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0) {
		/* The adapter is associated. */
		*(uint32*)g_vis_cmdoutbuf = htod32(WLC_IOCTL_MAXLEN);
		if ((ret = wlu_get(wl, WLC_GET_BSS_INFO, g_vis_cmdoutbuf, WLC_IOCTL_MAXLEN)) < 0)
			return ret;

		bi = (wl_bss_info_t*)(g_vis_cmdoutbuf + 4);
		if (dtoh32(bi->version) == WL_BSS_INFO_VERSION ||
			dtoh32(bi->version) == LEGACY2_WL_BSS_INFO_VERSION ||
			dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
			get_dut_info_from_bss(wl, bi, dut_info);
			vis_get_wsec(wl, dut_info->ucastrsntype);
			vis_get_wpa_auth(wl, dut_info->akmrsntype);
		}
		else
			VIS_SCAN("Sorry, your driver has bss_info_version %d "
				"but this program supports only version %d.\n",
				bi->version, WL_BSS_INFO_VERSION);
	}

	wl_is_AP(wl, dut_info);

	return 0;
}

/* To get the current MAC address of DUT */
int
wl_get_mac(void *wl, char *strmac)
{
	int ret;
	struct ether_addr ea = {{0, 0, 0, 0, 0, 0}};

	if ((ret = wlu_iovar_get(wl, "cur_etheraddr", &ea, ETHER_ADDR_LEN)) < 0) {
		VIS_SCAN("Error getting variable mac address of DUT\n");
		return ret;
	}
	snprintf(strmac, (ETHER_ADDR_LEN * 3), "%s", wl_ether_etoa(&ea));

	return 0;
}

/*
 * check whether rrm is enabled /not for the interface
 * WI-FI Insight uses two reports Channel_load_Measurement and Statistics_Measurement
 * So we are checking the corresponding capabilities bits which are 9 and 11 respectively.
 */
int
wl_is_rrm_enabled(void *wl)
{
	int err, len, isenabled = 0;
	uint low = 0, rrm_cap_clm_bit = 9, rrm_cap_sm_bit = 11;
	const char *cmdname = "rrm";
	void *ptr = NULL;
	dot11_rrm_cap_ie_t rrm_cap, *reply;

	memset(g_vis_cmdoutbuf, 0, WLC_IOCTL_SMLEN);
	strncpy(g_vis_cmdoutbuf, cmdname, WLC_IOCTL_SMLEN - 1);
	g_vis_cmdoutbuf[WLC_IOCTL_SMLEN -1] = '\0';

	len = strlen(cmdname) + 1;
	memcpy(&g_vis_cmdoutbuf[len], &rrm_cap, sizeof(rrm_cap));

	ptr = g_vis_cmdoutbuf;
	err = wlu_get(wl, WLC_GET_VAR, &g_vis_cmdoutbuf[0], WLC_IOCTL_SMLEN);

	if (err < 0)
		return isenabled;

	reply = (dot11_rrm_cap_ie_t *)ptr;
	if (reply) {
		low = reply->cap[0] | (reply->cap[1] << 8) |
			(reply->cap[2] << 16) | (reply->cap[3] << 24);
	}

	if ((low & (1 << rrm_cap_clm_bit)) &&
		(low & (1 << rrm_cap_sm_bit))) {
		isenabled = 1;
	}

	return isenabled;
}

#ifndef ATE_BUILD
int
wl_is_up(void *wl, int *isup)
{
	int ret;
	int val = 0;

	if ((ret = wlu_get(wl, WLC_GET_UP, &val, sizeof(int))) < 0)
		return ret;

	val = dtoh32(val);
	*isup = val;
	VIS_SCAN("val : %d and *isup : %d\n", val, *isup);

	return ret;
}
#endif /* !ATE_BUILD */
