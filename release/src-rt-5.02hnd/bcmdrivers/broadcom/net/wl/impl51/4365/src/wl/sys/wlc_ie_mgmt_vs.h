/*
 * IE management module Vendor Specific IE utilities
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_ie_mgmt_vs.h 580000 2015-08-17 22:08:34Z $
 */

#ifndef _wlc_ie_mgmt_vs_h_
#define _wlc_ie_mgmt_vs_h_

#include <typedefs.h>
#include <wlc_types.h>

/*
 * Special id (250 - 255).
 */
#define WLC_IEM_VS_IE_ID_UNK		250

/*
 * Priority/id (0 - 249).
 *
 * - It is used as a Priority when registering a Vendor Specific IE's
 *   calc_len/build callback pair. The IE management's IE calc_len/build
 *   function invokes the callbacks in their priorities' ascending order.
 *
 * - It is used as an ID when registering a Vendor Specific IE's parse
 *   callback. The IE management's IE parse function queries the user
 *   supplied classifier callback (which may use the OUI plus some other
 *   information in the IE being parsed to decide the ID), and invokes the
 *   callback.
 */
/* !Please leave some holes in between priorities when possible! */
#define WLC_IEM_VS_IE_PRIO_VNDR		64
#define WLC_IEM_VS_IE_PRIO_BRCM_HT	80
#define WLC_IEM_VS_IE_PRIO_BRCM_EXT_CH	88
#define WLC_IEM_VS_IE_PRIO_BRCM_VHT	104
#define WLC_IEM_VS_IE_PRIO_BRCM_TPC	136
#define WLC_IEM_VS_IE_PRIO_BRCM_RMC	140
#define WLC_IEM_VS_IE_PRIO_BRCM		152
#define WLC_IEM_VS_IE_PRIO_BRCM_PSTA	156
#define WLC_IEM_VS_IE_PRIO_WPA		160
#define WLC_IEM_VS_IE_PRIO_WPS		164
#define WLC_IEM_VS_IE_PRIO_WME		168
#ifdef BCMCCX
#define WLC_IEM_VS_IE_PRIO_CCX_QOS	170
#define WLC_IEM_VS_IE_PRIO_CCX_IHV	171
#define WLC_IEM_VS_IE_PRIO_CCX_RM_CAP	173
#define WLC_IEM_VS_IE_PRIO_CCX_VER	174
#endif
#define WLC_IEM_VS_IE_PRIO_WME_TS	176
#ifdef BCMCCX
#define WLC_IEM_VS_IE_PRIO_CCX_TS_RS	178
#endif
#define WLC_IEM_VS_IE_PRIO_HS20		190
#define WLC_IEM_VS_IE_PRIO_P2P		192
#define WLC_IEM_VS_IE_PRIO_OSEN		196
#define WLC_IEM_VS_IE_PRIO_NAN		197

#define WLC_IEM_VS_IE_PRIO_ULB     	204

/*
 * Map Vendor Specific IE to an id
 */
extern uint8 wlc_iem_vs_get_id(wlc_iem_info_t *iem, uint8 *ie);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
extern int wlc_iem_vs_dump(void *ctx, struct bcmstrbuf *b);
#endif

#endif /* _wlc_ie_mgmt_vs_h_ */
