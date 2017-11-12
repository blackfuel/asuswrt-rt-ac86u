/*
 * wlceventd
 *
 * ASUSTeK Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of ASUSTeK.
 *
 */

#include "hapdevent.h"

#ifdef RTCONFIG_NOTIFICATION_CENTER
#include <wlc_nt.h>
#endif

#ifdef RTCONFIG_CFGSYNC
#include <cfg_lib.h>
#include <cfg_event.h>
#endif

#if defined(RTCONFIG_SW_HW_AUTH) && defined(RTCONFIG_AMAS)
#include <auth_common.h>
#define APP_ID	"33716237"
#define APP_KEY	"g2hkhuig238789ajkhc"
#endif

int HAPDEVENT_msglevel = HAPDEVENT_DEBUG_EVENT;

#if 0
/* Msg dispatch */
static int hapdevent_validate_message(int bytes, uint8 *dpkt)
{
	bcm_event_t *pvt_data;

	/* the message should be at least the header to even look at it */
	if (bytes < sizeof(bcm_event_t) + 2) {
		HAPDEVENT_ERROR("Invalid length of message\n");
		return HAPDEVENT_FAIL;
	}
	pvt_data = (bcm_event_t *)dpkt;
	if (ntohs(pvt_data->bcm_hdr.subtype) != BCMILCP_SUBTYPE_VENDOR_LONG) {
		HAPDEVENT_ERROR("%s: not vendor specifictype\n",
			pvt_data->event.ifname);
		return HAPDEVENT_FAIL;
	}
	if (pvt_data->bcm_hdr.version != BCMILCP_BCM_SUBTYPEHDR_VERSION) {
		HAPDEVENT_ERROR("%s: subtype header version mismatch\n",
			pvt_data->event.ifname);
		return HAPDEVENT_FAIL;
	}
	if (ntohs(pvt_data->bcm_hdr.length) < BCMILCP_BCM_SUBTYPEHDR_MINLENGTH) {
		HAPDEVENT_ERROR("%s: subtype hdr length not even minimum\n",
			pvt_data->event.ifname);
		return HAPDEVENT_FAIL;
	}
	if (bcmp(&pvt_data->bcm_hdr.oui[0], BRCM_OUI, DOT11_OUI_LEN) != 0) {
		HAPDEVENT_ERROR("%s: wlceventd_validate_wlpvt_message: not BRCM OUI\n",
			pvt_data->event.ifname);
		return HAPDEVENT_FAIL;
	}
	/* check for wl dcs message types */
	switch (ntohs(pvt_data->bcm_hdr.usr_subtype)) {
		case BCMILCP_BCM_SUBTYPE_EVENT:
#ifdef DBG
			HAPDEVENT_EVENT("subtype: event\n");
#endif
			break;
		default:
			return HAPDEVENT_FAIL;
	}

	return HAPDEVENT_OK; /* good packet may be this is destined to us */
}
#endif

#ifdef DBG
void dump_data(char *data, int len)
{
	int i;

	printf("dump data (%d)", len);

	for (i = 0; i < len; i++) {
		if (i%16 == 0) printf("\n");
		printf("%02X ",*(data+i));
	}
	printf("\n");
}
#endif

static int hapdevent_proc_event(int argc, char *argv[])
{
	char *event_name;
#ifdef RTCONFIG_CFGSYNC
	char event_msg[384] = {0};
#endif
	if (argc < 2) {
		HAPDEVENT_EVENT("Invalid arguments!!\n");
		return HAPDEVENT_FAIL;
	}

	event_name = argv[1];
	if (strcmp(event_name, HAPD_EVENT_AP_STA_CONNECTED) == 0) {
		if (argc < 4) {
			HAPDEVENT_EVENT("Invalid arguments!!\n");
			return HAPDEVENT_FAIL;
		}

		HAPDEVENT_EVENT("%s %s\n", HAPD_EVENT_AP_STA_CONNECTED, argv[2]);
#ifdef RTCONFIG_CFGSYNC
		snprintf(event_msg, sizeof(event_msg), WEVENT_MAC_IFNAME_MSG,
			EID_WEVENT_DEVICE_CONNECTED, argv[2], argv[3]);
		send_cfgmnt_event(event_msg);
#endif
	} else if (strcmp(event_name, HAPD_EVENT_AP_STA_DISCONNECTED) == 0) {
		if (argc < 4) {
			HAPDEVENT_EVENT("Invalid arguments!!\n");
			return HAPDEVENT_FAIL;
		}

		HAPDEVENT_EVENT("%s %s\n", HAPD_EVENT_AP_STA_DISCONNECTED, argv[2]);
#ifdef RTCONFIG_CFGSYNC
		snprintf(event_msg, sizeof(event_msg), WEVENT_MAC_IFNAME_MSG,
			EID_WEVENT_DEVICE_DISCONNECTED, argv[2], argv[3]);
		send_cfgmnt_event(event_msg);
#endif
	} else if (strcmp(event_name, HAPD_EVENT_AP_STA_PROBREQ) == 0) {
		if (argc < 3) {
			HAPDEVENT_EVENT("Invalid arguments!!\n");
			return HAPDEVENT_FAIL;
		}
		HAPDEVENT_EVENT("%s %s\n", HAPD_EVENT_AP_STA_PROBREQ, argv[2]);
#ifdef RTCONFIG_CFGSYNC
		snprintf(event_msg, sizeof(event_msg), WEVENT_VSIE_MSG,
			EID_WEVENT_DEVICE_PROBE_REQ, argv[2]);
		send_cfgmnt_event(event_msg);
#endif
	} else {
		HAPDEVENT_EVENT("Unsupoort Event : %s\n", argv[1]);
		return HAPDEVENT_FAIL;
	}
	return HAPDEVENT_OK;
}

/* service main entry */
int main(int argc, char *argv[])
{
	int err = HAPDEVENT_OK;
	char *val;
	bool foreground = FALSE;

#if defined(RTCONFIG_SW_HW_AUTH) && defined(RTCONFIG_AMAS)
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

	if (foreground == FALSE) {
		if (daemon(1, 1) == -1) {
			HAPDEVENT_ERROR("err from daemonize.\n");
			goto done;
		}
	}

	val = nvram_safe_get("HAPDEVENT_msglevel");
	if (strcmp(val, ""))
		HAPDEVENT_msglevel = strtoul(val, NULL, 0);

	hapdevent_proc_event(argc, argv);

done:
	return err;
}
