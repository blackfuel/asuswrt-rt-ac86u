/*
 * Linux Visualization Data Concentrator json utility header
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
 * $Id: vis_jsonutility.h 606349 2015-12-15 07:15:11Z $
 */

#ifndef _VIS_JSON_UTILITY_H_
#define _VIS_JSON_UTILITY_H_

#include <stdio.h>
#include <json.h>
#include "vis_struct.h"

extern int send_networks(int sockfd, dut_info_t *dut_info, networks_list_t *networks_list);

extern int send_config_to_web(int sockfd, configs_t *configs, graphnamelist_t *graphnames);

extern int send_associated_sta(int sockfd, dut_info_t *dut_info, assoc_sta_list_t *stas_list);

extern int send_json_response(int sockfd, char *response);

extern int send_channel_capacity(int sockfd, dut_info_t *dut_info, congestion_list_t *congestion,
	int capacity, channel_maplist_t *chanmap, int freqband, assoc_sta_list_t *stas_list);

extern int send_ampdu_stats(int sockfd, tx_ampdu_list_t *ampdulist);

extern int send_chanim_metrics_stats(int sockfd, dut_info_t *dut_info,
	congestion_list_t *congestion, int freqband);

extern int send_all_duts(int sockfd, dut_list_t *dutlist);

extern int send_rrm_stastats(int sockfd, vis_sta_stats_t *stastats);
#endif /* _VIS_JSON_UTILITY_H_ */
