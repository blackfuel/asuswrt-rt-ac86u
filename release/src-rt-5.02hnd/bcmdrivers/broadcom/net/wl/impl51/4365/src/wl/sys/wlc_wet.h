/*
 * Wireless Ethernet (WET) interface
 *
 *   Broadcom Proprietary and Confidential. Copyright (C) 2016,
 *   All Rights Reserved.
 *   
 *   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 *   the contents of this file may not be disclosed to third parties, copied
 *   or duplicated in any form, in whole or in part, without the prior
 *   written permission of Broadcom.
 *
 *   $Id: wlc_wet.h 607219 2015-12-18 07:03:46Z $
 */


#ifndef _wlc_wet_h_
#define _wlc_wet_h_

/* RCMTA engine can have a maximum of 50 entries. Reserve slot 25 for the
 * primary. Slots 26 to 49 are available for use by WET RAs. Slots 0 to 24
 * are available for use for the downstream STAs TAs.
 */
#define WET_TA_STRT_INDX	0
#define WET_RA_PRIM_INDX	(RCMTA_SIZE / 2)
#define WET_RA_STRT_INDX	((RCMTA_SIZE / 2) + 1)

/* forward declaration */
typedef struct wlc_wet_info wlc_wet_info_t;

/*
 * Initialize wet private context.It returns a pointer to the
 * wet private context if succeeded. Otherwise it returns NULL.
 */
extern wlc_wet_info_t *wlc_wet_attach(wlc_info_t *wlc);

/* Cleanup wet private context */
extern void wlc_wet_detach(wlc_wet_info_t *weth);

/* Process frames in transmit direction */
extern int wlc_wet_send_proc(wlc_wet_info_t *weth, void *sdu, void **new);

/* Process frames in receive direction */
extern int wlc_wet_recv_proc(wlc_wet_info_t *weth, void *sdu);

#ifdef BCMDBG
extern int wlc_wet_dump(wlc_wet_info_t *weth, struct bcmstrbuf *b);
#endif /* BCMDBG */

#endif	/* _wlc_wet_h_ */
