/*
 * OBSS Protection support
 * Broadcom 802.11 Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */


#ifndef _wlc_prot_obss_h_
#define _wlc_prot_obss_h_

/* Function: record secondary rssi histogram
 * three bins [hi, med, low] with
 * hi  : counting sec_rssi >= M_SECRSSI0_MIN (hi_thresh)
 * med : counting sec_rssi in [ M_SECRSSI1_MIN, M_SECRSSI0_MIN )
 * low : counting sec_rssi <= M_SECRSSI1_MIN (low_thresh)
 */
#define OBSS_SEC_RSSI_LIM0_DEFAULT				-50	/* in dBm */
#define OBSS_SEC_RSSI_LIM1_DEFAULT				-70	/* in dBm */
#define OBSS_INACTIVITY_PERIOD_DEFAULT				30	/* in seconds */
#define OBSS_DUR_THRESHOLD_DEFAULT				30	/* OBSS
* protection trigger for RX CRS Sec
*/

struct wlc_prot_obss_info {
	bool protection;	/* TRUE if full phy bw CTS2SELF */
};

wlc_prot_obss_info_t *wlc_prot_obss_attach(wlc_info_t *wlc);
void wlc_prot_obss_detach(wlc_prot_obss_info_t *prot);

#ifdef WL_PROT_OBSS
#define WLC_PROT_OBSS_PROTECTION(prot)	((prot)->protection)
#else
#define WLC_PROT_OBSS_PROTECTION(prot)	(0)
#endif /* WL_PROT_OBSS */

#endif /* _wlc_prot_obss_h_ */
