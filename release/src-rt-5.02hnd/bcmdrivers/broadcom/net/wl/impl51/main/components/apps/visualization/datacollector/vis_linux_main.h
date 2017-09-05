/*
 * Linux Visualization Data Collector
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
 * $Id: vis_linux_main.h 555336 2015-05-08 09:52:04Z $
 */
#ifndef _VIS_LINUX_MAIN_H_
#define _VIS_LINUX_MAIN_H_
#include "vis_struct.h"

int vis_debug_level; /* Indicates the level of debug message to be printed on the console */

vis_wl_interface_t *g_wlinterface; /* Head pointer of all the WL interface adapters info */
configs_t	g_configs; /* Global config */
long		g_timestamp; /* Global timestamp. when the data collection started */

char server_ip_address[MAX_IP_LEN]; /* Server's IP address */
char gateway_ip_address[MAX_IP_LEN]; /* Gateway IP address */

#endif /* _VIS_LINUX_MAIN_H_ */
