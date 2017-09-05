/*
 * Common (OS-independent) portion of
 * Broadcom 802.11bang Networking Device Driver
 *
 * BMAC driver - AMT/RCMTA interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_addrmatch.h 594757 2015-10-23 12:35:49Z $
 */

/* This interface provides support for manipulating address match
 * or rcmta (pre-corerev 40) entries, independent of the core, althouth
 * certain functionality - AMT attributes are not available for older
 * core revisions.
 */

#ifndef _wlc_addrmatch_h_
#define _wlc_addrmatch_h_

#include <wlc_types.h>
#include <d11.h>

enum {
	WLC_ADDRMATCH_IDX_MAC 		= -1,
	WLC_ADDRMATCH_IDX_BSSID 	= -2
};

/* Add the address match entry. For AMT, the input attributes are OR'd
 * with the current ones, if any, if AMT_ATTR_VALID is set in input.
 * Otherwise, they are cleared. Based on idx, the entry is selected as follows
 *  WLC_ADDRMATCH_IDX_MAC - rxe MAC or AMT_IDX_MAC
 *  WLC_ADDRMATCH_IDX_BSSID - rxe BSSID or AMT_IDX_BSSID
 *  otherwise entry corresponds to the idx  - rcmta or AMT
 * Previous attributes are returned for AMT, or 0 for RCMTA
 */
uint16 wlc_set_addrmatch(wlc_info_t *wlc, int idx,
	const struct ether_addr *addr, uint16 attr);

/* Clears the address match entry - both address and attributes
 * if applicable, are cleared. Previous attributes are returned for AMT,
 * 0 for RCMTA
 */
uint16 wlc_clear_addrmatch(wlc_info_t *wlc, int idx);

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WL_BEAMFORMING) || \
	defined(ACKSUPR_MAC_FILTER)
/* get info for the address match entry. For AMT, both address and
 * the attributes are returned. For RCMTA, address is returned and
 * attributes are set to AMT_ATTR_VALID for non-NULL ether address
 */
void wlc_get_addrmatch(wlc_info_t *wlc, int idx, struct ether_addr *addr,
	uint16 *attr);
#endif 

#if defined(BCMDBG_DUMP)
/* dump the address match entry */
int wlc_dump_addrmatch(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif 

#ifdef ACKSUPR_MAC_FILTER
int wlc_addrmatch_info_alloc(wlc_info_t *wlc, int max_entry_num);
void wlc_addrmatch_info_free(wlc_info_t *wlc, int max_entry_num);
#endif /* ACKSUPR_MAC_FILTER */
#endif /* _wlc_addrmatch_h_ */
