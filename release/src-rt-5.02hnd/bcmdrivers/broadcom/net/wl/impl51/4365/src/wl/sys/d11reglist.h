/*
 * D11reglist for Broadcom 802.11abgn
 * Networking Adapter Device Drivers.
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
 * $Id: d11reglist.h 599445 2015-11-13 20:32:54Z $
 */
#ifndef _d11reglist_h_
#define _d11reglist_h_

#include "d11reglist_proto.h"

#ifdef WLC_MINMACLIST
extern CONST d11regs_list_t d11regsmin_pre40[];
extern CONST d11regs_list_t d11regsmin_ge40[];
extern CONST uint d11regsmin_pre40sz;
extern CONST uint d11regsmin_ge40sz;
#else
extern CONST d11regs_list_t d11regs23[];
extern CONST d11regs_list_t d11regs42[];
extern CONST d11regs_list_t d11regs48[];
extern CONST d11regs_list_t d11regs49[];
extern CONST d11regs_list_t d11regs_pre40[];
extern CONST d11regs_list_t d11regs_ge40[];
extern CONST d11regs_list_t d11regs64[];
extern CONST d11regs_list_t d11regsx64[];
extern CONST d11regs_list_t d11regs65[];
extern CONST d11regs_list_t d11regsx65[];
extern CONST uint d11regs23sz;
extern CONST uint d11regs42sz;
extern CONST uint d11regs48sz;
extern CONST uint d11regs49sz;
extern CONST uint d11regs_pre40sz;
extern CONST uint d11regs_ge40sz;
extern CONST uint d11regs64sz;
extern CONST uint d11regsx64sz;
extern CONST uint d11regs65sz;
extern CONST uint d11regsx65sz;
#endif /* WLC_MINMACLIST */

#endif /* _d11reglist_h_ */
