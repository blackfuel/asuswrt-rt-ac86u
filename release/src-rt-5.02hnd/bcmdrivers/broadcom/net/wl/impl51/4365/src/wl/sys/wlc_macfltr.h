/*
 * mac filter module header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_macfltr.h 594757 2015-10-23 12:35:49Z $
 */

#ifndef _wlc_macflter_h_
#define _wlc_macflter_h_

/* This module provides association control:
 * - for AP to decide if an association request is granted
 * - for STA to decide if a join target is considered
 * - for others to control the peer list
 * - ...
 */

#include <wlc_types.h>
#include <wlioctl.h>

/* module entries */
/* attach/detach */
extern wlc_macfltr_info_t *wlc_macfltr_attach(wlc_info_t *wlc);
extern void wlc_macfltr_detach(wlc_macfltr_info_t *mfi);

/* APIs */
/* check if the 'addr' is in the allow/deny list by the return code defined below */
extern int wlc_macfltr_addr_match(wlc_macfltr_info_t *mfi, wlc_bsscfg_t *cfg,
	const struct ether_addr *addr);

/* address match return code */
/* mac filter mode is DISABLE */
#define WLC_MACFLTR_DISABLED		0
/* mac filter mode is DENY */
#define WLC_MACFLTR_ADDR_DENY		1	/* addr is in mac list */
#define WLC_MACFLTR_ADDR_NOT_DENY	2	/* addr is not in mac list */
/* mac filter mode is ALLOW */
#define WLC_MACFLTR_ADDR_ALLOW		3	/* addr is in mac list */
#define WLC_MACFLTR_ADDR_NOT_ALLOW	4	/* addr is not in mac list */

/* set/get mac allow/deny list based on mode */
extern int wlc_macfltr_list_set(wlc_macfltr_info_t *mfi, wlc_bsscfg_t *cfg,
	struct maclist *maclist, uint len);
extern int wlc_macfltr_list_get(wlc_macfltr_info_t *mfi, wlc_bsscfg_t *cfg,
	struct maclist *maclist, uint len);
#ifdef ACKSUPR_MAC_FILTER
extern void wlc_macfltr_addrmatch_move(wlc_info_t *wlc);
extern int wlc_macfltr_find_and_add_addrmatch(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	struct ether_addr *addr, uint16 attr);
extern int wlc_macfltr_find_and_clear_addrmatch(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	struct ether_addr *addr, uint16 attr);
extern bool wlc_macfltr_acksupr_is_duplicate(wlc_info_t *wlc, struct ether_addr *ea);
#endif /* ACKSUPR_MAC_FILTER */
/* set/get mac list mode */
#define MFIWLC(mfi) (*(wlc_info_t **)mfi)	/* expect wlc to be the first field */
#define wlc_macfltr_mode_set(mfi, cfg, mode) \
	(wlc_ioctl(MFIWLC(mfi), WLC_SET_MACMODE, &(mode), sizeof(mode), (cfg)->wlcif))
#define wlc_macfltr_mode_get(mfi, cfg, mode) \
	(wlc_ioctl(MFIWLC(mfi), WLC_GET_MACMODE, mode, sizeof(*(mode)), (cfg)->wlcif))

#endif /* _wlc_macflter_h_ */
