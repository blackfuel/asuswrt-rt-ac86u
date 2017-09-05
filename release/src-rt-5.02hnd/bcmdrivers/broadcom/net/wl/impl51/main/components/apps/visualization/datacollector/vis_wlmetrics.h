/*
 * metric tab  statistics header for visualization tool
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
 * $Id: vis_wlmetrics.h 534859 2015-02-16 08:18:50Z $
 */

#ifndef _vis_wlcounters_h_
#define _vis_wlcounters_h_

#include "vis_struct.h"

#define STR_TX_MCS	"TX MCS  :"
#define STR_TX_VHT	"TX VHT  :"
#define STR_MPDUDENS	"MPDUdens:"
#define STR_TX_MCS_SGI	"TX MCS SGI:"
#define STR_TX_VHT_SGI	"TX VHT SGI:"
#define STR_RX_MCS	"RX MCS  :"
#define STR_RX_VHT	"RX VHT  :"
#define STR_RX_MCS_SGI	"RX MCS SGI:"
#define STR_RX_VHT_SGI	"RX VHT SGI:"

extern int wl_counters(void *wl, counters_t *countersout, counters_t *oldcountersout);

extern assoc_sta_list_t *wl_get_counters_for_all_stas(void *wl);

extern tx_ampdu_list_t *wl_get_ampdu_stats(void *wl);

#endif /* _vis_wlcounters_h_ */
