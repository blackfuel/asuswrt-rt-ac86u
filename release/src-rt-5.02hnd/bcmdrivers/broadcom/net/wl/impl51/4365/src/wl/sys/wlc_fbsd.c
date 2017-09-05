/*
 * FreeBSD-specific portion of
 * Broadcom 802.11abgn Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_fbsd.c 467328 2014-04-03 01:23:40Z $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <proto/802.11.h>
#include <bcmwifi_channels.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <sbconfig.h>
#include <sbchipc.h>
#include <pcicfg.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_pio.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_phy_hal.h>
#include <wlc_led.h>
#include <wl_export.h>
#include "d11ucode.h"
#include "wlc.h"

bool
wlc_dotxstatus(wlc_info_t *wlc, tx_status_t *txs, uint32 frm_tx2)
{
	WL_ERROR(("%s:  not implemented\n", __FUNCTION__));
	return FALSE;
}

void
wlc_high_dpc(wlc_info_t *wlc, uint32 macintstatus)
{
	WL_ERROR(("%s:  not implemented\n", __FUNCTION__));
}

void
wlc_recv(wlc_info_t *wlc, void *p)
{
	WL_ERROR(("%s:  not implemented\n", __FUNCTION__));
}
