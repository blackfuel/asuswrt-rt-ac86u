/*
 * MAC based SW probe response module header file -
 * It uses the MAC filter list to influence the decision about
 * which MAC to send SW probe response.
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */


#ifndef _wlc_probresp_mac_filter_h_
#define _wlc_probresp_mac_filter_h_

/* function APIs */
#ifdef WLPROBRESP_MAC_FILTER

/* module entry and exit */
extern void *wlc_probresp_mac_filter_attach(wlc_info_t *wlc);
extern void wlc_probresp_mac_filter_detach(void *mprobresp_mac_filter);
#endif

#endif /* _wlc_probresp_mac_filter_h_ */
