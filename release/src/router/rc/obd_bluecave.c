/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2012, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <wlutils.h>
#include <shutils.h>
#include <shared.h>
#include <wlioctl.h>
#include <rc.h>

#if defined(RTCONFIG_LANTIQ)
#define OUI_LEN 3
#define VS_ID 221
#else
#define OUI_LEN DOT11_OUI_LEN
#define VS_ID DOT11_MNG_VS_ID
#endif

#if defined(RTCONFIG_AMAS) && !defined(RTCONFIG_DISABLE_REPEATER_UI)

#ifdef RTCONFIG_SW_HW_AUTH
#include <auth_common.h>
#define APP_ID	"33716237"
#define APP_KEY	"g2hkhuig238789ajkhc"
#endif

#include <wlscan.h>
#include <bcmendian.h>
#if defined(RTCONFIG_BCM7) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
#include <bcmutils.h>
#include <security_ipc.h>
#endif

#include <sys/reboot.h>

/* Debug Print */
#define OBD_DEBUG_ERROR		0x000001
#define OBD_DEBUG_WARNING	0x000002
#define OBD_DEBUG_INFO		0x000004
#define OBD_DEBUG_EVENT		0x000008
#define OBD_DEBUG_DBG		0x000010

#define NEW_RSSI_INFO	1

int msglevel = OBD_DEBUG_DBG;

#define OBD_ERROR(fmt, arg...) \
	do { if (msglevel & OBD_DEBUG_ERROR) \
		dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define OBD_WARNING(fmt, arg...) \
	do { if (msglevel & OBD_DEBUG_WARNING) \
		dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define OBD_INFO(fmt, arg...) \
	do { if (msglevel & OBD_DEBUG_INFO) \
		dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define OBD_EVENT(fmt, arg...) \
	do { if (msglevel & OBD_DEBUG_EVENT) \
		dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define OBD_DBG(fmt, arg...) \
	do { if (msglevel & OBD_DEBUG_DBG) \
		dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define OBD_PRINT(fmt, arg...) \
	do { dbg("OBD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define NORMAL_PERIOD		1		/* second */
#define RUSHURGENT_PERIOD	50 * 1000	/* microsecond */
#define	NVRAM_BUFSIZE	100

#define WLC_SCAN_RETRY_TIMES		5
#define NUMCHANS			64
#define MAX_SSID_LEN			32

#define OBD_TIMEOUT			300

//static char scan_result[WLC_SCAN_RESULT_BUF_LEN];
static char hexdata_g[256];
static int len_hexdata_g;
static time_t time_ref;
static int status_g = 0;

struct cap_rssi {
	struct ether_addr BSSID;
	unsigned char RSSI;
};

struct rssi_info_s {
	unsigned char cap_count;
	struct cap_rssi caprssi[64];
} rssi_info;

#define SECURITY_DOWNGRADE

#ifndef SECURITY_DOWNGRADE
struct cap_info_s {
	wlc_ssid_t ssid;
	struct ether_addr BSSID;
	time_t timestamp;
} cap_info[64];

static unsigned char cap_count;
#endif

static void obd_exit(int sig);

/* The below macro handle endian mis-matches between wl utility and wl driver. */
//static bool g_swap = FALSE;
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))

#if defined(RTCONFIG_LANTIQ)
#else
#if defined(RTCONFIG_BCM7) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
typedef struct escan_wksp_s {
	uint8 packet[4096];
	int event_fd;
} escan_wksp_t;

static escan_wksp_t *d_info;

/* open a UDP packet to event dispatcher for receiving/sending data */
static int
escan_open_eventfd()
{
	int reuse = 1;
	struct sockaddr_in sockaddr;
	int fd = -1;

	d_info->event_fd = -1;

	/* open loopback socket to communicate with event dispatcher */
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sockaddr.sin_port = htons(EAPD_WKSP_DCS_UDP_SPORT);

	if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		OBD_DBG("Unable to create loopback socket\n");
		goto exit;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
		OBD_DBG("Unable to setsockopt to loopback socket %d.\n", fd);
		goto exit;
	}

	if (bind(fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
		OBD_DBG("Unable to bind to loopback socket %d\n", fd);
		goto exit;
	}

	d_info->event_fd = fd;

	return 0;

	/* error handling */
exit:
	if (fd != -1) {
		close(fd);
	}

	return errno;
}

static bool escan_swap = FALSE;
#define htod16(i) (escan_swap?bcmswap16(i):(uint16)(i))
#define WL_EVENT_TIMEOUT 10

struct escan_bss {
	struct escan_bss *next;
	wl_bss_info_t bss[1];
};
#define ESCAN_BSS_FIXED_SIZE 4

/* listen to sockets and receive escan results */
static int
get_scan_escan(char *scan_buf, uint buf_len)
{
	fd_set fdset;
	int fd;
	struct timeval tv;
	uint8 *pkt;
	int len;
	int retval;
	wl_escan_result_t *escan_data;
	struct escan_bss *escan_bss_head = NULL;
	struct escan_bss *escan_bss_tail = NULL;
	struct escan_bss *result;

	d_info = (escan_wksp_t*)malloc(sizeof(escan_wksp_t));

	escan_open_eventfd();

	if (d_info->event_fd == -1) {
		return -1;
	}

	fd = d_info->event_fd;

	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	pkt = d_info->packet;
	len = sizeof(d_info->packet);

	tv.tv_sec = WL_EVENT_TIMEOUT;
	tv.tv_usec = 0;

	/* listen to data availible on all sockets */
	while ((retval = select(fd+1, &fdset, NULL, NULL, &tv)) > 0) {
		bcm_event_t *pvt_data;
		uint32 evt_type;
		uint32 status;

		if (recv(fd, pkt, len, 0) <= 0)
			continue;

		pvt_data = (bcm_event_t *)(pkt + IFNAMSIZ);
		evt_type = ntoh32(pvt_data->event.event_type);

		if (evt_type == WLC_E_ESCAN_RESULT) {
			escan_data = (wl_escan_result_t*)(pvt_data + 1);
			status = ntoh32(pvt_data->event.status);

			if (status == WLC_E_STATUS_PARTIAL) {
				wl_bss_info_t *bi = &escan_data->bss_info[0];
				wl_bss_info_t *bss = NULL;

				/* check if we've received info of same BSSID */
				for (result = escan_bss_head; result; result = result->next) {
					bss = result->bss;

					if (!memcmp(bi->BSSID.octet, bss->BSSID.octet,
						ETHER_ADDR_LEN) &&
						CHSPEC_BAND(bi->chanspec) ==
						CHSPEC_BAND(bss->chanspec) &&
						bi->SSID_len == bss->SSID_len &&
						!memcmp(bi->SSID, bss->SSID, bi->SSID_len))
						break;
				}

				if (!result) {
					/* New BSS. Allocate memory and save it */
					struct escan_bss *ebss = (struct escan_bss *)malloc(
						OFFSETOF(struct escan_bss, bss)	+ bi->length);

					if (!ebss) {
						OBD_DBG("can't allocate memory for bss");
						goto exit;
					}

					ebss->next = NULL;
					memcpy(&ebss->bss, bi, bi->length);
					if (escan_bss_tail) {
						escan_bss_tail->next = ebss;
					}
					else {
						escan_bss_head = ebss;
					}
					escan_bss_tail = ebss;
				}
				else if (bi->RSSI != WLC_RSSI_INVALID) {
					/* We've got this BSS. Update rssi if necessary */
					if (((bss->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL) ==
						(bi->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL)) &&
					    ((bss->RSSI == WLC_RSSI_INVALID) ||
						(bss->RSSI < bi->RSSI))) {
						/* preserve max RSSI if the measurements are
						 * both on-channel or both off-channel
						 */
						bss->RSSI = bi->RSSI;
						bss->SNR = bi->SNR;
						bss->phy_noise = bi->phy_noise;
					} else if ((bi->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL) &&
						(bss->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL) == 0) {
						/* preserve the on-channel rssi measurement
						 * if the new measurement is off channel
						*/
						bss->RSSI = bi->RSSI;
						bss->SNR = bi->SNR;
						bss->phy_noise = bi->phy_noise;
						bss->flags |= WL_BSS_FLAGS_RSSI_ONCHANNEL;
					}
				}
			}
			else if (status == WLC_E_STATUS_SUCCESS) {
				/* Escan finished. Let's go dump the results. */
				break;
			}
			else {
				OBD_DBG("sync_id: %d, status:%d, misc. error/abort\n",
					escan_data->sync_id, status);
				goto exit;
			}
		}
	}

	if (retval > 0) {
		wl_scan_results_t* s_result = (wl_scan_results_t*)scan_buf;
		wl_bss_info_t *bi = s_result->bss_info;
		wl_bss_info_t *bss;

		s_result->count = 0;
		len = buf_len - WL_SCAN_RESULTS_FIXED_SIZE;

		for (result = escan_bss_head; result; result = result->next) {
			bss = result->bss;
			if (buf_len < bss->length) {
				OBD_DBG("Memory not enough for scan results\n");
				break;
			}
			memcpy(bi, bss, bss->length);
			bi = (wl_bss_info_t*)((int8*)bi + bss->length);
			len -= bss->length;
			s_result->count++;
		}
	} else if (retval == 0) {
		OBD_DBG("Scan timeout!\n");
	} else {
		OBD_DBG("Receive scan results failed!\n");
	}

exit:
	if (d_info) {
		if (d_info->event_fd != -1) {
			close(d_info->event_fd);
			d_info->event_fd = -1;
		}

		free(d_info);
	}

	/* free scan results */
	result = escan_bss_head;
	while (result) {
		struct escan_bss *tmp = result->next;
		free(result);
		result = tmp;
	}

	return (retval > 0) ? BCME_OK : BCME_ERROR;
}

static char *
wl_get_scan_results_escan(char *scan_result)
{
	wl_scan_results_t *list = (wl_scan_results_t*)scan_result;
	int unit = 0;
	int ret, retry_times = 0;
	wl_escan_params_t *params = NULL;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + OFFSETOF(wl_escan_params_t, params) + NUMCHANS * sizeof(uint16);
	int org_scan_time = 20, scan_time = 40;
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	params = (wl_escan_params_t*)malloc(params_size);
	if (params == NULL) {
		return NULL;
	}

	memset(params, 0, params_size);
	params->params.bss_type = DOT11_BSSTYPE_ANY;
	memcpy(&params->params.bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->params.scan_type = WL_SCANFLAGS_PASSIVE;
	params->params.nprobes = -1;
	params->params.active_time = -1;
	params->params.passive_time = -1;
	params->params.home_time = -1;
	params->params.channel_num = 0;

	params->version = htod32(ESCAN_REQ_VERSION);
	params->action = htod16(WL_SCAN_ACTION_START);

	srand((unsigned int)uptime());
	params->sync_id = htod16(rand() & 0xffff);

	params_size += OFFSETOF(wl_escan_params_t, params);

	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
	/* extend scan channel time to get more AP probe resp */
	wl_ioctl(ifname, WLC_GET_SCAN_CHANNEL_TIME, &org_scan_time, sizeof(org_scan_time));
	if (org_scan_time < scan_time)
		wl_ioctl(ifname, WLC_SET_SCAN_CHANNEL_TIME, &scan_time,	sizeof(scan_time));

	while ((ret = wl_iovar_set(ifname, "escan", params, params_size)) < 0 &&
				retry_times++ < WLC_SCAN_RETRY_TIMES) {
		OBD_DBG("set escan command failed, retry %d\n", retry_times);
		sleep(1);
	}

	free(params);

	/* restore original scan channel time */
	wl_ioctl(ifname, WLC_SET_SCAN_CHANNEL_TIME, &org_scan_time, sizeof(org_scan_time));

	if (ret == 0) {
		ret = get_scan_escan(scan_result, WLC_SCAN_RESULT_BUF_LEN);
		if (ret < 0) {
			OBD_DBG("get escan result failed, retry %d\n", retry_times);
		}
	}

	if (ret < 0)
		return NULL;

	// check WL_BSS_INFO version
	if (list->version != WL_BSS_INFO_VERSION &&
		list->version != LEGACY_WL_BSS_INFO_VERSION &&
		list->version != LEGACY2_WL_BSS_INFO_VERSION) {
		OBD_DBG("Sorry, your driver has bss_info_version %d "
		    "but this program supports only version %d.\n",
		    list->version, WL_BSS_INFO_VERSION);
		return NULL;
	}

	return scan_result;
}

#else

static char *
wl_get_scan_results(char *scan_result)
{
	int unit = 0;
	int ret, retry_times = 0;
	wl_scan_params_t *params;
	wl_scan_results_t *list = (wl_scan_results_t*)scan_result;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + NUMCHANS * sizeof(uint16);
	int org_scan_time = 20, scan_time = 40;
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	params = (wl_scan_params_t*)malloc(params_size);
	if (params == NULL) {
		return NULL;
	}

	memset(params, 0, params_size);
	params->bss_type = DOT11_BSSTYPE_ANY;
	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->scan_type = WL_SCANFLAGS_PASSIVE;
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = -1;
	params->home_time = -1;
	params->channel_num = 0;

	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	/* extend scan channel time to get more AP probe resp */
	wl_ioctl(ifname, WLC_GET_SCAN_CHANNEL_TIME, &org_scan_time, sizeof(org_scan_time));
	if (org_scan_time < scan_time)
		wl_ioctl(ifname, WLC_SET_SCAN_CHANNEL_TIME, &scan_time,	sizeof(scan_time));

	while ((ret = wl_ioctl(ifname, WLC_SCAN, params, params_size)) < 0 &&
				retry_times++ < WLC_SCAN_RETRY_TIMES) {
		OBD_DBG("set scan command failed, retry %d\n", retry_times);
		sleep(1);
	}

	free(params);

	/* restore original scan channel time */
	wl_ioctl(ifname, WLC_SET_SCAN_CHANNEL_TIME, &org_scan_time, sizeof(org_scan_time));

	sleep(2);

	if (ret == 0) {
		list->buflen = WLC_SCAN_RESULT_BUF_LEN;
		ret = wl_ioctl(ifname, WLC_SCAN_RESULTS, scan_result, WLC_SCAN_RESULT_BUF_LEN);
		if (ret < 0) {
			OBD_DBG("get scan result failed, retry %d\n", retry_times);
		}
	}

	if (ret < 0)
		return NULL;

	return scan_result;
}
#endif
#endif

static int vsie_setbuf(int type, unsigned char *dst, unsigned char *data)
{
	int len = 0;
	unsigned char c;

	switch (type) {
	case 1:
		len = 1;
		break;
	case 3:
		len = 20;
		break;
	case 4:
		len = 6;
		break;
	case 5:
#if defined(RTCONFIG_DBG_BLUECAVE_OBD)
		len = strlen("BLUECAVE");
#else
		len = strlen(get_productid());
#endif
		break;
	case 6:
		len = 1 + rssi_info.cap_count * 7;
		break;
	}

	if (len) {
		c = type;
		memcpy(dst, &c, 1);
		c = len;
		memcpy(dst+1, &c, 1);
		memcpy(dst+2, data, len);
	}

	return len + 2;
}

#if 0
static void set_temp_id(unsigned char *buf)
{
	time_t now;
	char now_byte[4];
	char str_groupid[] = "DC1A2BFCBCAE839DF23EF0F3B676C0DB";
	unsigned char groupid[16];
	char hexstr[3];
	char *src, *dest, *pp;
	int idx;
	unsigned char val;

	time(&now);
	src = str_groupid;
	dest = (char *) groupid;
	for (idx = 0; idx < sizeof(groupid); idx++) {
		hexstr[0] = src[0];
		hexstr[1] = src[1];
		hexstr[2] = '\0';

		val = (unsigned char) strtoul(hexstr, NULL, 16);

		*dest++ = val;
		src += 2;
	}

	pp = (char *) &now;
	for (idx = 0; idx < sizeof(now_byte); idx++)
		now_byte[idx] = pp[sizeof(now_byte) - 1 -idx];

	for (idx = 0; idx < sizeof(groupid); idx++)
		groupid[idx] = groupid[idx] & now_byte[idx % sizeof(now_byte)];

	f_read("/dev/urandom", buf, 16);
	memcpy(buf + 16, now_byte, 4);
}
#endif

static void
add_ie()
{
	int unit = 0;
	unsigned char value[256];
	char hexdata[256];
	unsigned char *p = NULL;
	unsigned char status[] = { 0x3 };
	unsigned char ea[ETHER_ADDR_LEN];
#if 0
	unsigned char ID[20];
#endif
	int len, i;
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;
	char length[3]/*, oui_asus_str[9]*/;

	memset(value, 0, sizeof(value));
	p = value;
	p += vsie_setbuf(1, p, status);
	ether_atoe(get_lan_hwaddr(), ea);
	p += vsie_setbuf(4, p, ea);
#if 0
	set_temp_id(ID);
	p += vsie_setbuf(3, p, ID);
#endif
#if defined(RTCONFIG_DBG_BLUECAVE_OBD)
	p += vsie_setbuf(5, p, (unsigned char *) "BLUECAVE");
#else
	p += vsie_setbuf(5, p, (unsigned char *) get_productid());
#endif
	p += vsie_setbuf(6, p, (unsigned char *) &rssi_info);

	len = p - value;

	for (i = 0; i < len; i++)
		sprintf(&hexdata[2 * i], "%02x", value[i]);
	hexdata[2 * len] = 0;

	len += OUI_LEN;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	/*memset(oui_asus_str, 0, sizeof(oui_asus_str));
	sprintf(oui_asus_str, "%02X:%02X:%02X", OUI_ASUS[0], OUI_ASUS[1], OUI_ASUS[2]);*/

	if (memcmp(hexdata_g, "\x0", 1)) {
		/*snprintf(length, sizeof(length), "%d", len_hexdata_g);
		eval("wl", "-i", ifname, "del_ie", "16", length, oui_asus_str, hexdata_g);*/
		del_obd_probe_req_vsie(hexdata_g);
	}

	if (strlen(hexdata)) {
		/*snprintf(length, sizeof(length), "%d", len);
		eval("wl", "-i", ifname, "add_ie", "16", length, oui_asus_str, hexdata);*/
		add_obd_probe_req_vsie(hexdata);
		memcpy(hexdata_g, hexdata, sizeof(hexdata_g));
		nvram_set("amesh_hexdata", hexdata_g);
		len_hexdata_g = len;
	}
}

/* Use to store scanned bss which contains vsie information. */
#define MAX_VSIE_LEN 1024
struct scanned_bss {
	struct scanned_bss *next;
	int idx;
	uint8 channel;
	struct ether_addr BSSID;
	uint8 vsie_len;
	uint8 vsie[MAX_VSIE_LEN];
	unsigned char RSSI;
};

static int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}


int hex2byte(const char *hex)
{
	int a, b;
	a = hex2num(*hex++);
	if (a < 0)
		return -1;
	b = hex2num(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}

/**
 * hexstr2bin - Convert ASCII hex string into binary data
 * @hex: ASCII hex string (e.g., "01ab")
 * @buf: Buffer for the binary data
 * @len: Length of the text to convert in bytes (of buf); hex will be double
 * this size
 * Returns: 0 on success, -1 on failure (invalid hex string)
 */
int hexstr2bin(const char *hex, uint8 *buf, size_t len)
{
	size_t i;
	int a;
	const char *ipos = hex;
	uint8 *opos = buf;

	for (i = 0; i < len; i++) {
		a = hex2byte(ipos);
		if (a < 0)
			return -1;
		*opos++ = a;
		ipos += 2;
	}
	return 0;
}

static void free_bss_list(struct scanned_bss *list) {
	struct scanned_bss *bss_list = list;
	struct scanned_bss *next_bss, *tmp;
	char eaddr[18];
	if (bss_list) {
		next_bss = bss_list->next;
		while (next_bss) {
			tmp = next_bss->next;
			ether_etoa((const unsigned char *) (uint8 *)&next_bss->BSSID, eaddr);
			//OBD_DBG("free_bss_list %s vsie_len=%d\n", eaddr, next_bss->vsie_len);
			free(next_bss);
			next_bss = NULL;
			next_bss = tmp;
		}
		ether_etoa((const unsigned char *) (uint8 *)&bss_list->BSSID, eaddr);
		//OBD_DBG("free_bss_list %s vsie_len=%d\n", eaddr, bss_list->vsie_len);
		free(bss_list);
		bss_list = NULL;
	}
}

#if defined(RTCONFIG_LANTIQ)
void start_active_scan()
{
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = get_wififname(0);
	OBD_DBG("Send probe-req\n\n");
#if defined(RTCONFIG_DBG_BLUECAVE_OBD)
	eval("wl", "-i", "wlan1", "scan");
#else
	eval("wl", "-i", ifname, "scan");
#endif
}

struct scanned_bss *get_bss_scan_result()
{
#define ACTIVE_SCANNED_BSS "scanned_bss"
#define PASSIVE_SCANNED_BSS "passive_scanned_bss"
#define KEY_BSSID "bssid="
#define KEY_BSSID_LEN 6
#define KEY_SIGNAL_LEVEL "signal_level="
#define KEY_SIGNAL_LEVEL_LEN 13
#define KEY_VSIE "vsie="
#define KEY_VSIE_LEN 5
	char *bss_result_file[2] = {ACTIVE_SCANNED_BSS, PASSIVE_SCANNED_BSS};
	struct scanned_bss *bss_list = NULL, *current_bss = NULL;
	char *bss_entry[256];
	int i;
	for (i=0; i<sizeof(bss_result_file)/sizeof(char *); i++) {
		FILE *fp = NULL;
		if ((fp = fopen(bss_result_file[i], "r")) != NULL) {
			memset(bss_entry, 0, sizeof(bss_entry));
			while(fgets(bss_entry, sizeof(bss_entry) , fp) != NULL) {
				int match_1 = 0, match_2, match_3 = 0;
				uint8 vsie_len = 0;
				uint8 vsie[MAX_VSIE_LEN];
				int RSSI = 0;
				struct ether_addr BSSID;

				char *tmp;

				memset(&vsie[0], 0, sizeof(vsie));
				memset(&BSSID, 0, sizeof(BSSID));

				// skip entry which have no vsie.
				if ((tmp = strstr(bss_entry, KEY_VSIE)) == NULL)
					continue;

				// vsie
				if (strlen(tmp) > KEY_VSIE_LEN) {
					vsie_len = (strlen(tmp) - KEY_VSIE_LEN);
					vsie_len /= 2;
					hexstr2bin(tmp+KEY_VSIE_LEN+(OUI_LEN*2), vsie, (vsie_len-OUI_LEN));
					OBD_DBG("%s vsie_len=%d\n", tmp+KEY_VSIE_LEN+(OUI_LEN*2), vsie_len);
					match_1 = 1;
				}

				// signal_level
				if ((tmp = strstr(bss_entry, KEY_SIGNAL_LEVEL)) == NULL)
					continue;

				if (strlen(tmp) > KEY_SIGNAL_LEVEL_LEN) {
					sscanf(tmp+KEY_SIGNAL_LEVEL_LEN, "%d", &RSSI);
					OBD_DBG("SIGNAL_LEVEL=%d\n", RSSI);
					match_2 = 1;
				}

				// bssid
				if ((tmp = strstr(bss_entry, KEY_BSSID)) == NULL)
					continue;

				if (strlen(tmp) > KEY_BSSID_LEN) {
					ether_aton_r(tmp+KEY_BSSID_LEN, &BSSID);
					OBD_DBG("BSSID=%.*s\n", 17, tmp+KEY_BSSID_LEN);
					match_3 = 1;
				}

				// create bss entry
				if (match_1 && match_3) {
					struct scanned_bss *bss = malloc(sizeof(struct scanned_bss));
					memset(bss, 0, sizeof(struct scanned_bss));

					bss->vsie_len = vsie_len;
					memcpy(&bss->vsie[0], &vsie[0], bss->vsie_len);
					bss->RSSI = (unsigned char)RSSI;
					memcpy(&bss->BSSID, &BSSID, sizeof(struct ether_addr));

					if (current_bss) {
						current_bss->next = bss;
					}
					current_bss = bss;

					if (bss_list == NULL)
						bss_list = bss;
				}

			}
			if (fp)
				fclose(fp);
		} else {
			perror(bss_result_file[i]);
		}
	}
	return bss_list;
}
#else
void start_active_scan()
{
	int unit = 0;
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
	OBD_DBG("Send probe-req\n\n");
	eval("wl", "-i", ifname, "scan");
}

struct scanned_bss *get_bss_scan_result()
{
	struct scanned_bss *bss_list = NULL, *current_bss = NULL;
	uint i, left;
	uint8 channel;
	char scan_result[WLC_SCAN_RESULT_BUF_LEN];
	wl_scan_results_t *list = (wl_scan_results_t*)scan_result;
	wl_bss_info_t *bi;
	wl_bss_info_107_t *old_bi;
	struct bss_ie_hdr *ie;
	struct vndr_ie *ie_vs;

#if defined(RTCONFIG_BCM7) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
	if (wl_get_scan_results_escan(scan_result) == NULL)
#else
	if (wl_get_scan_results(scan_result) == NULL)
#endif
		return NULL;

	bi = list->bss_info;

	for (i = 0; i < list->count; i++) {
		/* Convert version 107 to 109 */
		if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
			old_bi = (wl_bss_info_107_t *)bi;
			bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
			bi->ie_length = old_bi->ie_length;
			bi->ie_offset = sizeof(wl_bss_info_107_t);
		}

		if (bi->ie_length) {
			//ether_etoa((const unsigned char *) (uint8 *)&bi->BSSID, eaddr);

			if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap)
				channel= bi->ctl_ch;
			else
				channel= (bi->chanspec & WL_CHANSPEC_CHAN_MASK);

			if (status_g == 0)
				OBD_DBG("%-4d%-33s\n", i+1, bi->SSID);
		}
		else
			continue;

		ie = (struct bss_ie_hdr *)((unsigned char *) bi + bi->ie_offset);
		for (left = bi->ie_length; left > 0;
			left -= (ie->len + 2), ie = (struct bss_ie_hdr *) ((unsigned char *) ie + 2 + ie->len)) {

			char vsie_str[MAX_VSIE_LEN];
			char BSSID[ETHER_ADDR_LEN];
			struct scanned_bss *bss;

			if (ie->elem_id != VS_ID)
				continue;

			if (memcmp(ie->oui, OUI_ASUS, 3))
				continue;

			ie_vs = (struct vndr_ie *) ie;

			bss = malloc(sizeof(struct scanned_bss));
			memset(bss, 0, sizeof(struct scanned_bss));

			// channel
			bss->channel = channel;
			
			// vsie
			bss->vsie_len = ie_vs->len;
			memcpy((void *)&bss->vsie[0], (void *)&ie_vs->data[0], bss->vsie_len);
			OBD_DBG("%s vsie_len=%d\n", vsie_str, bss->vsie_len);

			// BSSID
			memcpy(&bss->BSSID, &bi->BSSID, ETHER_ADDR_LEN);
			ether_ntoa_r(&bss->BSSID, &BSSID);
			OBD_DBG("BSSID=%s\n", BSSID);

			memset(vsie_str, 0, sizeof(vsie_str));
			hex2str(&ie_vs->data[0], vsie_str, bss->vsie_len);

			if (current_bss) {
				current_bss->next = bss;
			}
			current_bss = bss;

			if (bss_list == NULL)
				bss_list = bss;

			goto NEXT_BI;
		}
NEXT_BI:
		bi = (wl_bss_info_t*)((int8*)bi + bi->length);
	}
	return bss_list;
}
#endif

#if 0
static int
wl_scan()
{
	int unit = 0;
	wl_scan_results_t *list = (wl_scan_results_t*)scan_result;
	wl_bss_info_t *bi;
	wl_bss_info_107_t *old_bi;
	struct bss_ie_hdr *ie;
	struct vndr_ie *ie_vs;
	struct tlvbase *tlv;
#ifndef SECURITY_DOWNGRADE
	struct tlvbase *tlv_timestamp;
#endif
	uint i, j, left, left2;
	char eaddr[18];
	uint8 channel;
	int match_1, match_2, match_3, match_4;
#ifndef SECURITY_DOWNGRADE
	int match_7;
#endif
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;
	uint8 status = 0, cost;
	unsigned char ID[20], ea[ETHER_ADDR_LEN];
	int count_available;
	int ob_locked;

#if defined (RTCONFIG_LANTIQ)
	if (get_passive_bss_scan_result() == NULL)
		return 0;
#else
#if defined(RTCONFIG_BCM7) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
	if (wl_get_scan_results_escan() == NULL)
#else
	if (wl_get_scan_results() == NULL)
#endif
		return 0;

	if (list->count == 0)
		return 0;
	
#if !(defined(RTCONFIG_BCM7) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER))
	else if (list->version != WL_BSS_INFO_VERSION &&
			list->version != LEGACY_WL_BSS_INFO_VERSION &&
			list->version != LEGACY2_WL_BSS_INFO_VERSION) {
		OBD_DBG("Sorry, your driver has bss_info_version %d "
		    "but this program supports only version %d.\n",
		    list->version, WL_BSS_INFO_VERSION);
		return 0;
	}
#endif
#endif

	bi = list->bss_info;

	if (status_g == 0)
		OBD_DBG("%-4s%-4s%-33s%-18s\n", "idx", "Ch", "SSID", "BSSID");

	count_available = ob_locked = 0;
	ether_atoe(get_lan_hwaddr(), ea);
	for (i = 0; i < list->count; i++) {
		/* Convert version 107 to 109 */
		if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
			old_bi = (wl_bss_info_107_t *)bi;
			bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
			bi->ie_length = old_bi->ie_length;
			bi->ie_offset = sizeof(wl_bss_info_107_t);
		}

		if (bi->ie_length) {
			ether_etoa((const unsigned char *) (uint8 *)&bi->BSSID, eaddr);

			if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap)
				channel= bi->ctl_ch;
			else
				channel= (bi->chanspec & WL_CHANSPEC_CHAN_MASK);

			if (status_g == 0)
				OBD_DBG("%-4d%-4d%-33s%-18s\n", i+1, channel, bi->SSID, eaddr);
		} else continue;

		match_1 = match_2 = match_3 = match_4 = 0;
#ifndef SECURITY_DOWNGRADE
		match_7 = 0;
#endif
		ie = (struct bss_ie_hdr *)((unsigned char *) bi + bi->ie_offset);
		for (left = bi->ie_length; left > 0;
			left -= (ie->len + 2), ie = (struct bss_ie_hdr *) ((unsigned char *) ie + 2 + ie->len)) {

			if (ie->elem_id != VS_ID)
				continue;

			if (memcmp(ie->oui, OUI_ASUS, 3))
				continue;

			if (status_g == 1)
				OBD_DBG("%-4d%-4d%-33s%-18s\n", i+1, channel, bi->SSID, eaddr);

			ie_vs = (struct vndr_ie *) ie;
			tlv = (struct tlvbase *) &(ie_vs->data[0]);
#ifndef SECURITY_DOWNGRADE
			tlv_timestamp = NULL;
#endif

			for (left2 = ie->len - OUI_LEN; left2 > 0;
				left2 -= (tlv->len + 2), tlv = (struct tlvbase *) ((unsigned char *) tlv + 2 + tlv->len)) {
				switch (tlv->type) {
				case 1:
					if (tlv->len != 1) break;
					match_1 = 1;
					status = tlv->data[0];
					OBD_DBG("Status: %x\n", status);
					break;
				case 2:
					if (tlv->len != 1) break;
					match_2 = 1;
					cost = tlv->data[0];
					OBD_DBG("Cost: %x\n", cost);
					break;
				case 3:
					if (tlv->len != 20) break;
					match_3 = 1;
					memcpy(ID, &tlv->data[0], 20);
					OBD_DBG("ID: ");
					if (msglevel & OBD_DEBUG_DBG) {
						for (j = 0; j < tlv->len; j++)
							dbg("%02X ", tlv->data[j]);
						dbg("\n");
					}
					break;
				case 4:
					if (tlv->len != 6) break;
					if (!memcmp(ea, &tlv->data[0], ETHER_ADDR_LEN))
						match_4 = 1;
					else
						match_4 = -1;
					OBD_DBG("MAC address: %s\n", ether_etoa(&tlv->data[0], eaddr));
					break;
				case 7:
#ifndef SECURITY_DOWNGRADE
					if (tlv->len != 4) break;
					match_7 = 1;
					tlv_timestamp = tlv;
					OBD_DBG("Timestamp: ");
					if (msglevel & OBD_DEBUG_DBG) {
						for (j = 0; j < tlv->len; j++)
							dbg("%02X ", tlv->data[j]);
						dbg("\n");
					}
#else
					break;
#endif
				default:
					OBD_DBG("Malformed TLV detected! Vendor Specific IE without ASUS OUI: ");
					ie_vs = (struct vndr_ie *) ie;
					if (msglevel & OBD_DEBUG_DBG) {
						for (j = 0; j < ie->len - OUI_LEN; j++)
							dbg("%02X ", ie_vs->data[j]);
						dbg("\n");
					}
					goto NEXT_BI;
				}
			}

#ifndef SECURITY_DOWNGRADE
			if (!(match_1 && match_2 && match_3 && match_7))
#else
			if (!(match_1 && match_2 && match_3))
#endif
				break;

			if (status_g == 0 && status == 2) {
				count_available++;

				if (count_available == 1)
					nvram_set_int("amesh_found_cap", 1);

				memcpy(&rssi_info.caprssi[count_available - 1].BSSID, &bi->BSSID, ETHER_ADDR_LEN);
				rssi_info.caprssi[count_available - 1].RSSI = bi->RSSI;

#ifndef SECURITY_DOWNGRADE
				memcpy(&cap_info[count_available - 1].BSSID, &bi->BSSID, ETHER_ADDR_LEN);
				strncpy((char *)cap_info[count_available - 1].ssid.SSID, (char *)bi->SSID, bi->SSID_len);
				cap_info[count_available - 1].ssid.SSID[bi->SSID_len] = '\0';
				cap_info[count_available - 1].ssid.SSID_len = bi->SSID_len;
				memcpy(&cap_info[count_available - 1].timestamp, &tlv_timestamp->data[0], sizeof(time_t));
#endif

				goto NEXT_BI;
			} else if (status_g == 1) {
				if (status == 2) {
					count_available++;

					memcpy(&rssi_info.caprssi[count_available - 1].BSSID, &bi->BSSID, ETHER_ADDR_LEN);
					rssi_info.caprssi[count_available - 1].RSSI = bi->RSSI;

					if (match_4 == 1) {
						nvram_set_int("amesh_led", 1);
						kill_pidfile_s("/var/run/watchdog.pid", SIGUSR1);
					} else if (match_4 == -1) {
						kill_pidfile_s("/var/run/watchdog.pid", SIGUSR2);
						nvram_set_int("amesh_led", 0);
					}

					goto NEXT_BI;
				} else if (status == 4) {
					if (match_4 == -1) {
						OBD_DBG("Reset due to mismatch of RE MAC address\n\n");
						status_g = 0;

						time_ref = uptime();

						nvram_set_int("amesh_found_cap", 0);

						return 0;
					} else if (match_4 == 1) {
#ifndef SECURITY_DOWNGRADE
						for (j = 0; j < cap_count; j++) {
							if (    !memcmp(&bi->BSSID, &cap_info[j].BSSID, ETHER_ADDR_LEN) &&
								(bi->SSID_len == cap_info[j].ssid.SSID_len) &&
								!memcmp(bi->SSID, cap_info[j].ssid.SSID, cap_info[j].ssid.SSID_len) &&
								!memcmp(&tlv_timestamp->data[0], &cap_info[j].timestamp, sizeof(time_t))) {
#endif
								ob_locked = 1;

								nvram_set_int("amesh_found_cap", 0);

								goto ACTION;
#ifndef SECURITY_DOWNGRADE
							}
						}
#endif
					}
				}
			}

			goto NEXT_BI;
		}
NEXT_BI:
		bi = (wl_bss_info_t*)((int8*)bi + bi->length);
	}

ACTION:
	if ((uptime() - time_ref) > OBD_TIMEOUT) {
		OBD_DBG("Reset due to timeout\n\n");
		status_g = 0;

		time_ref = uptime();

		nvram_set_int("amesh_found_cap", 0);
	} else if (ob_locked && status_g == 1) {
		if (nvram_get_int("amesh_led") == 1) {
			kill_pidfile_s("/var/run/watchdog.pid", SIGUSR2);
			nvram_set_int("amesh_led", 0);
		}

		OBD_DBG("Start WPS Enroll\n\n");
		notify_rc("start_wps_enr");

		sleep(3);

		status_g = 2;
	} else if (count_available && (status_g == 0 || status_g == 1)) {
		rssi_info.cap_count = count_available;
		wl_add_ie();

		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		OBD_DBG("Send probe-req\n\n");
		eval("wl", "-i", ifname, "scan");

		if (!status_g) {
#ifndef SECURITY_DOWNGRADE
			cap_count = count_available;
#endif
			status_g = 1;
		}
	}

	return 0;
}
#endif

static int
cap_scan()
{
	int unit = 0;
	struct scanned_bss *bss_list = NULL;
	struct scanned_bss *bss;
	struct tlvbase *tlv;
#ifndef SECURITY_DOWNGRADE
	struct tlvbase *tlv_timestamp;
#endif
	uint i, j, left2;
	char eaddr[18];
	int match_1, match_2, match_3, match_4;
#ifndef SECURITY_DOWNGRADE
	int match_7;
#endif
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;
	uint8 status = 0, cost;
	unsigned char ID[20], ea[ETHER_ADDR_LEN];
	int count_available;
	int ob_locked;

	if ((bss_list = get_bss_scan_result()) == NULL)
		return 0;

	bss = bss_list;

	if (status_g == 0)
		OBD_DBG("%-4s%-18s\n", "idx", "BSSID");
	
	/*for (i=0, bss = bss_list; bss; i++, bss = bss->next) {
		OBD_DBG("bss->vsie_len=%d\n", bss->vsie_len);
		if (bss->vsie_len) {
		}
	}

	free_bss_list(bss_list);
	return 0;*/

	count_available = ob_locked = 0;
	ether_atoe(get_lan_hwaddr(), ea);
	for (i=0, bss = bss_list; bss; i++, bss = bss->next) {
		/* Convert version 107 to 109 */
		/*if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
			old_bi = (wl_bss_info_107_t *)bi;
			bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
			bi->ie_length = old_bi->ie_length;
			bi->ie_offset = sizeof(wl_bss_info_107_t);
		}*/

		OBD_DBG("bss->vsie_len=%d\n", bss->vsie_len);
		if (bss->vsie_len) {
			ether_etoa((const unsigned char *) (uint8 *)&bss->BSSID, eaddr);

			/*if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap)
				channel= bi->ctl_ch;
			else
				channel= (bi->chanspec & WL_CHANSPEC_CHAN_MASK);*/

			if (status_g == 0)
				OBD_DBG("%-4d%-18s\n", i+1, eaddr);
		} else continue;

		match_1 = match_2 = match_3 = match_4 = 0;
#ifndef SECURITY_DOWNGRADE
		match_7 = 0;
#endif
		/*ie = (struct bss_ie_hdr *)((unsigned char *) bi + bi->ie_offset);
		for (left = bi->ie_length; left > 0;
			left -= (ie->len + 2), ie = (struct bss_ie_hdr *) ((unsigned char *) ie + 2 + ie->len)) {

			if (ie->elem_id != VS_ID)
				continue;

			if (memcmp(ie->oui, OUI_ASUS, 3))
				continue;

			if (status_g == 1)
				OBD_DBG("%-4d%-4d%-33s%-18s\n", i+1, channel, bi->SSID, eaddr);

			ie_vs = (struct vndr_ie *) ie;*/
			tlv = (struct tlvbase *) &(bss->vsie[0]);
#ifndef SECURITY_DOWNGRADE
			tlv_timestamp = NULL;
#endif

			for (left2 = bss->vsie_len - OUI_LEN; left2 > 0;
				left2 -= (tlv->len + 2), tlv = (struct tlvbase *) ((unsigned char *) tlv + 2 + tlv->len)) {
				switch (tlv->type) {
				case 1:
					if (tlv->len != 1) break;
					match_1 = 1;
					status = tlv->data[0];
					OBD_DBG("Status: %x\n", status);
					break;
				case 2:
					if (tlv->len != 1) break;
					match_2 = 1;
					cost = tlv->data[0];
					OBD_DBG("Cost: %x\n", cost);
					break;
				case 3:
					if (tlv->len != 20) break;
					match_3 = 1;
					memcpy(ID, &tlv->data[0], 20);
					OBD_DBG("ID: ");
					if (msglevel & OBD_DEBUG_DBG) {
						for (j = 0; j < tlv->len; j++)
							dbg("%02X ", tlv->data[j]);
						dbg("\n");
					}
					break;
				case 4:
					if (tlv->len != 6) break;
					if (!memcmp(ea, &tlv->data[0], ETHER_ADDR_LEN))
						match_4 = 1;
					else
						match_4 = -1;
					OBD_DBG("MAC address: %s\n", ether_etoa(&tlv->data[0], eaddr));
					break;
				case 7:
#ifndef SECURITY_DOWNGRADE
					if (tlv->len != 4) break;
					match_7 = 1;
					tlv_timestamp = tlv;
					OBD_DBG("Timestamp: ");
					if (msglevel & OBD_DEBUG_DBG) {
						for (j = 0; j < tlv->len; j++)
							dbg("%02X ", tlv->data[j]);
						dbg("\n");
					}
#else
					break;
#endif
				default:
					OBD_DBG("Malformed TLV detected! Vendor Specific IE without ASUS OUI: ");
					//ie_vs = (struct vndr_ie *) ie;
					if (msglevel & OBD_DEBUG_DBG) {
						for (j = 0; j < bss->vsie_len - OUI_LEN; j++)
							dbg("%02X ", bss->vsie[j]);
						dbg("\n");
					}
					goto NEXT_BSS;
				}
			}

#ifndef SECURITY_DOWNGRADE
			if (!(match_1 && match_2 && match_3 && match_7))
#else
			if (!(match_1 && match_2 && match_3))
#endif
				break;

			if (status_g == 0 && status == 2) {
				count_available++;

				if (count_available == 1)
					nvram_set_int("amesh_found_cap", 1);

				memcpy(&rssi_info.caprssi[count_available - 1].BSSID, &bss->BSSID, ETHER_ADDR_LEN);
				rssi_info.caprssi[count_available - 1].RSSI = bss->RSSI;

#ifndef SECURITY_DOWNGRADE
				memcpy(&cap_info[count_available - 1].BSSID, &bi->BSSID, ETHER_ADDR_LEN);
				strncpy((char *)cap_info[count_available - 1].ssid.SSID, (char *)bi->SSID, bi->SSID_len);
				cap_info[count_available - 1].ssid.SSID[bi->SSID_len] = '\0';
				cap_info[count_available - 1].ssid.SSID_len = bi->SSID_len;
				memcpy(&cap_info[count_available - 1].timestamp, &tlv_timestamp->data[0], sizeof(time_t));
#endif

				goto NEXT_BSS;
			} else if (status_g == 1) {
				if (status == 2) {
					count_available++;

					memcpy(&rssi_info.caprssi[count_available - 1].BSSID, &bss->BSSID, ETHER_ADDR_LEN);
					rssi_info.caprssi[count_available - 1].RSSI = bss->RSSI;

					if (match_4 == 1) {
						nvram_set_int("amesh_led", 1);
						kill_pidfile_s("/var/run/watchdog.pid", SIGUSR1);
					} else if (match_4 == -1) {
						kill_pidfile_s("/var/run/watchdog.pid", SIGUSR2);
						nvram_set_int("amesh_led", 0);
					}

					goto NEXT_BSS;
				} else if (status == 4) {
					if (match_4 == -1) {
						OBD_DBG("Reset due to mismatch of RE MAC address\n\n");
						status_g = 0;

						time_ref = uptime();

						nvram_set_int("amesh_found_cap", 0);

						free_bss_list(bss_list);
						return 0;
					} else if (match_4 == 1) {
#ifndef SECURITY_DOWNGRADE
						for (j = 0; j < cap_count; j++) {
							if (    !memcmp(&bi->BSSID, &cap_info[j].BSSID, ETHER_ADDR_LEN) &&
								(bi->SSID_len == cap_info[j].ssid.SSID_len) &&
								!memcmp(bi->SSID, cap_info[j].ssid.SSID, cap_info[j].ssid.SSID_len) &&
								!memcmp(&tlv_timestamp->data[0], &cap_info[j].timestamp, sizeof(time_t))) {
#endif
								ob_locked = 1;

								nvram_set_int("amesh_found_cap", 0);

								goto ACTION;
#ifndef SECURITY_DOWNGRADE
							}
						}
#endif
					}
				}
			}

			//goto NEXT_BSS;
		//}
NEXT_BSS:
		continue;
	}

ACTION:
	if ((uptime() - time_ref) > OBD_TIMEOUT) {
		OBD_DBG("Reset due to timeout\n\n");
		status_g = 0;

		time_ref = uptime();

		nvram_set_int("amesh_found_cap", 0);
	} else if (ob_locked && status_g == 1) {
		if (nvram_get_int("amesh_led") == 1) {
			kill_pidfile_s("/var/run/watchdog.pid", SIGUSR2);
			nvram_set_int("amesh_led", 0);
		}

		OBD_DBG("Start WPS Enroll\n\n");
		notify_rc("start_wps_enr");

		sleep(3);

		status_g = 2;
	} else if (count_available && (status_g == 0 || status_g == 1)) {
		rssi_info.cap_count = count_available;
		add_ie();

		start_active_scan();

		if (!status_g) {
#ifndef SECURITY_DOWNGRADE
			cap_count = count_available;
#endif
			status_g = 1;
		}
	}

	free_bss_list(bss_list);
	return 0;
}

static struct itimerval itv;
static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void
obd(int sig)
{
	int wps_proc_status;

	if (sig == SIGALRM)
	{
		if (status_g == 0 || status_g == 1) {
			cap_scan();

			alarm(NORMAL_PERIOD);
		} else if (status_g == 2) {	// WPS enrolling
			if (is_wps_stopped()) {
				wps_proc_status = nvram_get_int("wps_proc_status_x");
				if (wps_proc_status == 2 || wps_proc_status == 7)
					status_g = 3;
				else
					status_g = 4;

				alarm(NORMAL_PERIOD);
			} else
				alarmtimer(0, RUSHURGENT_PERIOD);
		} else if (status_g == 3) {	// WPS success
			if (nvram_get_int("w_Setting") == 1) {
				nvram_set("sw_mode", "3");
				nvram_set("wlc_psta", "2");
				nvram_set("wlc_dpsta", "1");
				nvram_set("lan_proto", "dhcp");
				nvram_set("lan_dnsenable_x", "1");
				nvram_set("x_Setting", "1");
				nvram_set("re_mode", "1");
				nvram_unset("cfg_group");
				nvram_commit();

				reboot(RB_AUTOBOOT);
				exit(0);
			} else {
				alarm(NORMAL_PERIOD);
			}
		} else if (status_g == 4) {	// WPS failure
			OBD_DBG("Exit due to WPS failure\n\n");
			notify_rc("restart_wireless");

			obd_exit(SIGTERM);
		}
	}
}

static void
obd_exit(int sig)
{
	/*char length[3], oui_asus_str[9];
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;*/

	if (sig == SIGTERM)
	{
		alarmtimer(0, 0);

		if (memcmp(hexdata_g, "\x0", 1)) {
			/*snprintf(prefix, sizeof(prefix), "wl%d_", 0);
			ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
			snprintf(oui_asus_str, sizeof(oui_asus_str), "%02X:%02X:%02X", OUI_ASUS[0], OUI_ASUS[1], OUI_ASUS[2]);
			snprintf(length, sizeof(length), "%d", len_hexdata_g);
			eval("wl", "-i", ifname, "del_ie", "16", length, oui_asus_str, hexdata_g);*/
			del_obd_probe_req_vsie(hexdata_g);
		}

		remove("/var/run/obd.pid");
		exit(0);
	}
}

int
obd_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;
	/*char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;
	char length[3], oui_asus_str[9];*/
	char *val;

#ifdef RTCONFIG_SW_HW_AUTH
	time_t timestamp = time(NULL);
	char in_buf[48];
	char out_buf[65];
	char hw_out_buf[65];
	char *hw_auth_code = NULL;

	// initial
	memset(in_buf, 0, sizeof(in_buf));
	memset(out_buf, 0, sizeof(out_buf));
	memset(hw_out_buf, 0, sizeof(hw_out_buf));

	// use timestamp + APP_KEY to get auth_code
	snprintf(in_buf, sizeof(in_buf)-1, "%ld|%s", timestamp, APP_KEY);

	hw_auth_code = hw_auth_check(APP_ID, get_auth_code(in_buf, out_buf, sizeof(out_buf)), timestamp, hw_out_buf, sizeof(hw_out_buf));

	// use timestamp + APP_KEY + APP_ID to get auth_code
	snprintf(in_buf, sizeof(in_buf)-1, "%ld|%s|%s", timestamp, APP_KEY, APP_ID);

	// if check fail, return
	if (strcmp(hw_auth_code, get_auth_code(in_buf, out_buf, sizeof(out_buf))))
		return 0;
#endif

	if (!is_router_mode() || (nvram_get_int("x_Setting") == 1))
		return 0;

	/* write pid */
	if ((fp = fopen("/var/run/obd.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	memset(hexdata_g, 0, sizeof(hexdata_g));
	len_hexdata_g = 0;

	time_ref = uptime();

	nvram_set_int("amesh_found_cap", 0);
	nvram_set_int("amesh_led", 0);
	if (!nvram_match("amesh_hexdata", "")) {
		/*snprintf(prefix, sizeof(prefix), "wl%d_", 0);
		ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		snprintf(oui_asus_str, sizeof(oui_asus_str), "%02X:%02X:%02X", OUI_ASUS[0], OUI_ASUS[1], OUI_ASUS[2]);
		snprintf(length, sizeof(length), "%d", (strlen(nvram_safe_get("amesh_hexdata"))/2) + OUI_LEN);
		eval("wl", "-i", ifname, "del_ie", "16", length, oui_asus_str, nvram_safe_get("amesh_hexdata"));*/
		del_obd_probe_req_vsie(nvram_safe_get("amesh_hexdata"));
		nvram_set("amesh_hexdata", "");
	}

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGALRM, obd);
	signal(SIGTERM, obd_exit);

	alarm(NORMAL_PERIOD);

	/* Most of time it goes to sleep */
	while (1)
	{
		val = nvram_safe_get("obd_msglevel");
		if (strcmp(val, ""))
			msglevel = strtoul(val, NULL, 0);

		pause();
	}

	return 0;
}
#endif
