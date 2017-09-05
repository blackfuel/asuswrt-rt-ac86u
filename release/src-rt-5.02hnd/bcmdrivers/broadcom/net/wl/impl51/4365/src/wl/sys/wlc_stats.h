/*
 * wlc/related statistics and test/verification counters for
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
 * $Id: wlc_stats.h 398374 2013-04-24 09:36:32Z  $
 */

#ifndef _wlc_stats_h_
#define _wlc_stats_h_

wlc_stats_info_t *
wlc_stats_attach(wlc_info_t *wlc);

void
wlc_stats_detach(wlc_stats_info_t *stats_info);

void
wlc_stats_update_bcnlenhist(wlc_stats_info_t *stats_info, int len);

#endif	/* _wlc_stats_h_ */
