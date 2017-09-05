/*
 * SES global common header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: ses.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _ses_h_
#define _ses_h_

#include <typedefs.h>


#define SES_MAX_SSID_LEN                32
#define SES_MAX_PASSPHRASE_LEN          63
#define SES_DEF_PASSPHRASE_LEN          16
#define SES_MAX_KEY_LEN                 255

#define SES_PE_MAX_INTERFACES		8

#define SES_MAX_TIMERS			16

#ifdef _WIN32
#define SES_IFNAME_SIZE			260 /* iptypes.h MAX_ADAPTER_NAME_LENGTH+4 */
#else
#define SES_IFNAME_SIZE			16
#endif

/* SES WDS configurator mode during OW */
#define SES_WDS_MODE_DISABLED           0 /* disabled */
#define SES_WDS_MODE_AUTO               1 /* dynamic cf/cl selection */
#define SES_WDS_MODE_ENABLED_ALWAYS     2 /* enabled always */
#define SES_WDS_MODE_ENABLED_EXCL       3 /* WDS-only i.e no regular STAs */
#define SES_WDS_MODE_CLIENT             4 /* always wds client mode */

#define SES_SECURITY_WPA_PSK		0x01
#define SES_SECURITY_WPA2_PSK		0x02

#define SES_ENCRYPTION_TKIP		0x01
#define SES_ENCRYPTION_AES		0x02
#define SES_ENCRYPTION_TKIP_AES		0x03

#define SES_VNDR_IE_TYPE                1
#define SES_VNDR_IE_SUBTYPE_CFG_AD      1
#define SES_VNDR_IE_VERSION             0x10

#define SES_VNDR_IE_FLAG_RWO            0x01
#define SES_VNDR_IE_FLAG_WDS_RWO        0x02
#define SES_VNDR_IE_FLAG_MASK           (SES_VNDR_IE_FLAG_RWO|SES_VNDR_IE_FLAG_WDS_RWO)

typedef struct ses_vndr_ie {
        uchar type;
        uchar subtype;
        uchar ses_ver;          /* 4-7: major, 0-3: minor */
        uchar ses_flags;        /* bit 0: window for regular client open */
                                /* bit 1: window for wds client open */
} ses_vndr_ie_t;

#endif /* _ses_h_ */
