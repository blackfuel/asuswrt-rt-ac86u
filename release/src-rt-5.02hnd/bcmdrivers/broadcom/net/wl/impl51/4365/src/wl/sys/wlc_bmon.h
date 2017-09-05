/**
 * Ucode monitored BSSID feature driver support API
 *
 * Ucode compares the incoming packet's a3 (if any) with the configured
 * monitored BSSID(s) (currently only one) and passes the packet up to
 * the driver if the a3 matches one of the BSSID(s).
 *
 * This driver code adds support for users to configure the BSSID(s) and
 * to register the callbacks to be invoked when a matching packet comes.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_bmon.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wlc_bmon_h_
#define _wlc_bmon_h_

/* _bmon is enabled whenever there is entry in the match table */
#ifdef WLBMON
#define BMON_ENAB(wlc)	((wlc)->bmon != NULL && (wlc)->pub->_bmon)
#else
#define BMON_ENAB(wlc)	FALSE
#endif

/* match entry user */
#define BMON_USER_NIC	0x1
#define BMON_USER_WLU	0x2

/* callback info */
typedef struct {
	int me;		/* match entry index returned from wlc_bmon_pktrx_match() */
	wlc_d11rxhdr_t *wrxh;
	uint8 *plcp;
	void *pkt;
} wlc_bmon_pktrx_data_t;
typedef void (*wlc_bmon_pktrx_fn_t)(void *arg, const wlc_bmon_pktrx_data_t *notif_data);

/* registration info */
typedef struct {
	uint user;
	struct ether_addr *bssid;
	wlc_bmon_pktrx_fn_t fn;
	void *arg;
} wlc_bmon_reg_info_t;

#ifdef WLBMON

/* module entries */
extern wlc_bmon_info_t *wlc_bmon_attach(wlc_info_t *wlc);
extern void wlc_bmon_detach(wlc_bmon_info_t *bmi);

/* Add/del address into the match table. */
extern int wlc_bmon_bssid_add(wlc_bmon_info_t *bmi, wlc_bmon_reg_info_t *reg);
extern int wlc_bmon_bssid_del(wlc_bmon_info_t *bmi, wlc_bmon_reg_info_t *reg);

/* determine if the given packet represented by the 'hdr' parameter
 * matches one of our entries and return the entry index if it does
 * or return -1 otherwise.
 */
extern int wlc_bmon_pktrx_match(wlc_bmon_info_t *bmi, struct dot11_header *hdr);

/* Notify interested parties of the given matching packet 'pkt'.
 * 'pkt' is shared among all parties and must not be freed nor be modified,
 * it points to the dot11 header, it has no pkttag info (not initialized)...
 */
extern void wlc_bmon_pktrx_notif(wlc_bmon_info_t *bmi, wlc_bmon_pktrx_data_t *notif_data);

#else /* WLBMON */

#define wlc_bmon_bssid_add(bmi, reg) ((void)(reg), BCME_OK)
#define wlc_bmon_bssid_del(bmi, reg) ((void)(reg), BCME_OK)

#define wlc_bmon_pktrx_match(bmi, hdr) ((void)(hdr), -1)

#define wlc_bmon_pktrx_notif(bmi, notif_data) do {(void)(notif_data);} while (0)

#endif /* !WLBMON */

#endif /* _wlc_bmon_h_ */
