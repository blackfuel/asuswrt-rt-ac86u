/*
 * Linux Visualization Data Collector RRM(802.11k) processing header
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
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
 * $Id: vis_rrmprocess.h 617465 2016-02-05 10:52:38Z $
 */

#ifndef _VIS_RRM_PROCESS_H_
#define _VIS_RRM_PROCESS_H_

#include <stdio.h>
#include "vis_struct.h"

extern int vis_wl_get_sta_info(void *wl, vis_sta_stats_t *stastats, uint32 band);

extern vis_rrm_statslist_t *get_rrm_stats_for_all_sta(void *wl, assoc_sta_list_t *stas_list,
	char *bssid, uint32 ctrlch);

extern vis_sta_statslist_t *get_sta_stats_for_all_sta(void *wl, assoc_sta_list_t *stas_list,
	uint32 band);

extern void rrm_initialize_mutexes();

extern void rrm_destroy_mutexes();

extern void rrm_thread_close();

extern void get_rrm_report_from_queue(vis_rrm_statslist_t *rrmstatslist);

#endif /* _VIS_RRM_PROCESS_H_ */
