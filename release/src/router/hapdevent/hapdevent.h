/*
 * wlceventd shared include file
 *
 * ASUSTek Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of ASUSTek.
 *
 */

#ifndef _hapdevent_h_
#define _hapdevent_h_

#include <proto/ethernet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <bcmnvram.h>
#include <bcmendian.h>
#include <shutils.h>
#include <wlioctl.h>
#include <security_ipc.h>

#include <net/if.h>
#include <arpa/inet.h>

#include <wlscan.h>
#include <mac_name_tab.h>

#define HAPD_EVENT_AP_STA_CONNECTED "AP-STA-CONNECTED"
#define HAPD_EVENT_AP_STA_DISCONNECTED "AP-STA-DISCONNECTED"
#define HAPD_EVENT_AP_STA_PROBREQ "AP-STA-PROBEREQ"

#define HAPDEVENT_OK		0
#define HAPDEVENT_FAIL		-1
//#define HAPDEVENT_DFLT_FD	-1
//#define HAPDEVENT_POLL_INTERVAL	1

/* Debug Print */
extern int HAPDEVENT_msglevel;
#define HAPDEVENT_DEBUG_ERROR	0x000001
#define HAPDEVENT_DEBUG_WARNING	0x000002
#define HAPDEVENT_DEBUG_INFO	0x000004
#define HAPDEVENT_DEBUG_EVENT	0x000008
#define HAPDEVENT_DEBUG_DEBUG	0x000010

#define HAPDEVENT_ERROR(fmt, arg...) \
	do { if (HAPDEVENT_msglevel & HAPDEVENT_DEBUG_ERROR) \
		printf("HAPDEVENT %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define HAPDEVENT_WARNING(fmt, arg...) \
	do { if (HAPDEVENT_msglevel & HAPDEVENT_DEBUG_WARNING) \
		printf("HAPDEVENT %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define HAPDEVENT_INFO(fmt, arg...) \
	do { if (HAPDEVENT_msglevel & HAPDEVENT_DEBUG_INFO) \
		printf("HAPDEVENT %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define HAPDEVENT_EVENT(fmt, arg...) \
	do { if (HAPDEVENT_msglevel & HAPDEVENT_DEBUG_EVENT) \
		printf("HAPDEVENT %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define HAPDEVENT_DEBUG(fmt, arg...) \
	do { if (HAPDEVENT_msglevel & HAPDEVENT_DEBUG_DEBUG) \
		printf("HAPDEVENT %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define HAPDEVENT_PRINT(fmt, arg...) \
	do { printf("HAPDEVENT %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#endif /*  _hapdevent_h_ */
