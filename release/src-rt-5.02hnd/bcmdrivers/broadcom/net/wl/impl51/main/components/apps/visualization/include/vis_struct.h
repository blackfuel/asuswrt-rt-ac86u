/*
 * Linux Visualization System common structures header
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
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
 * $Id: vis_struct.h 672659 2016-11-29 10:24:01Z $
 */

#ifndef _VIS_STRUCT_H_
#define _VIS_STRUCT_H_

#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif /* WIN32 */

#include "defines.h"
#include "typedefs.h"

#include <sys/ioctl.h>
#include <net/if.h>
#include <proto/ethernet.h>
#include <arpa/inet.h>

/* generic IOVAR handler
 * Any New IOVAR added will have one entry in vis_common.c file
 */
typedef struct iovar_handler_t_ {
	char *name; /* Name of the graph */
	char *heading; /* Heading to be shown before plotting the graph. "None" if no heading */
	char *plotwith; /* If this counter has to be plotted with
			 * other graph give main graphs name else "None"
			 */
	int graphtype; /* Type of grap 1 for bar graph,
			* 2 bar against time,
			* 3 for line grap and
			* 4 for line against time
			*/
	int perSTA; /* The vale is 1 if, the counter is for per associated STA's */
	char *barheading; /* Heading of the bar in case of multiple counters in one grap */
	char *xaxisname; /* Name of x-axis in a graph */
	char *yaxisname; /* Name of y-axis in a graph */
	int isplotinexternal; /* 1 if we have to plot in external build */
} iovar_handler_t;

typedef struct configs_t_ {
	uint32 version;
	uint32 isstart; /* whether to start collecting data or not */
	uint32 interval; /* Interval between sample collection */
	uint32 dbsize; /* Database size limit */
	int isremoteenab; /* Is remote debugging enabled */
	int isoverwrtdb; /* Whether to over write the DB or stop data collection flag */
	int isautostart; /* Whether to start data collection automatically or not */
	int weekdays; /* Which of the weekdays the data collection should run */
	int fromtm; /* From what time the data collection should start automatically */
	int totm; /* To what time the data collection should run automatically */
	char gatewayip[MAX_IP_LEN]; /* Gateway IP address */
} configs_t;

typedef struct records_to_fetch_t_ {
	int ispresent;
	char name[MAX_GRAPH_NAME];
	int count;
	int records;
	uint32 ftimestamp;
	uint32 ltimestamp;
} records_to_fetch_t;

typedef struct packet_header_t_ {
	uint32 type;
	uint32 from;
	uint32 result;
	uint32 band;
	char reqresptype[MAX_REQUEST_NAME];
	char reason[MAX_REASON_LEN];
	records_to_fetch_t recordstofetch;
} packet_header_t;

typedef struct dut_info_t_ {
	int isenabled;
	uint32 channel;
	uint32 ctrlch;
	int rowid;
	uint32 isAP;
	uint32 bandwidth;
	uint32 band;
	int noise;
	int rssi;
	int maxrate;
	char bssid[ETHER_ADDR_LEN * 3];
	char ssid[SSID_FMT_BUF_LEN];
	char mac[ETHER_ADDR_LEN * 3];
	char stamac[ETHER_ADDR_LEN * 3]; /* STA MAC for dcon. The MAC per STA from web */
	char networktype[MAX_NETWORK_TYPE]; /* Network tyep a, b, g or n */
	char mcastrsntype[MAX_MULTICAST_RSN]; /* RSN multicast cipher */
	char ucastrsntype[MAX_UNICAST_RSN]; /* RSN unicast ciphers */
	char akmrsntype[MAX_AKM_TYPE]; /* RSN AKM Suites */
	uint32 errinfo;			/* Error information. */
} dut_info_t;

typedef struct dut_list_t_ {
	uint32 version;
	uint32 length;
	dut_info_t duts[1];
} dut_list_t;

typedef struct ap_info_t_ {
	char bssid[ETHER_ADDR_LEN * 3];
	char ssid[SSID_FMT_BUF_LEN];
	int16 rssi;
	int8 noise;
	uint32 channel;
	uint32 ctrlch;
	uint8 band;
	uint8 bandwidth;
	uint32 maxrate;
	char networktype[MAX_NETWORK_TYPE]; /* Network tyep a, b, g or n */
	char mcastrsntype[MAX_MULTICAST_RSN]; /* RSN multicast cipher */
	char ucastrsntype[MAX_UNICAST_RSN]; /* RSN unicast ciphers */
	char akmrsntype[MAX_AKM_TYPE]; /* RSN AKM Suites */
} ap_info_t;

typedef struct networks_list_t_ {
	uint32 version;
	uint32 length;
	long timestamp;
	ap_info_t aps[1];
} networks_list_t;

typedef struct assoc_sta {
	uint32 version;
	uint32 length;
	int32 rssi;
	int32 phyrate;
	char mac[ETHER_ADDR_LEN * 3];
	uint32 prequested;
	uint32 pstored;
	uint32 pdropped;
	uint32 pretried;
	uint32 putilization;
	uint32 pqlength;
	/* count of rts attempts that failed to receive cts */
	uint32 prtsfail;
	/* packets finally dropped after retry limit */
	uint32 prtrydrop;
	/* packets retried again (ps pretend) prior to moving power save mode */
	uint32 ppsretry;
	/* count of packets sent (acked) successfully */
	uint32 packed;
	float ptput;
	float pphyrate;
} assoc_sta_t;

typedef struct assoc_sta_list {
	uint32 version;
	uint32 length;
	long timestamp;
	assoc_sta_t sta[1];
} assoc_sta_list_t;

typedef struct counters_t_ {
	long timestamp;
	uint32 rxcrsglitch;
	uint32 rxbadfcs;
	uint32 rxbadplcp;
} counters_t;

typedef struct tx_ampdu {
	uint8 streams;
	uint8 mcs;
	uint32 txmcs;
	uint32 txmcspercent;
	uint32 txmcssgi;
	uint32 txmcssgipercent;
	uint32 rxmcs;
	uint32 rxmcspercent;
	uint32 rxmcssgi;
	uint32 rxmcssgipercent;
	uint32 txvht;
	uint32 txvhtper;
	uint32 txvhtpercent;
	uint32 txvhtsgi;
	uint32 txvhtsgiper;
	uint32 txvhtsgipercent;
	uint32 rxvht;
	uint32 rxvhtper;
	uint32 rxvhtpercent;
	uint32 rxvhtsgi;
	uint32 rxvhtsgiper;
	uint32 rxvhtsgipercent;
	uint32 mpdudens;
	uint32 mpdudenspercent;
} tx_ampdu_t;

typedef struct tx_ampdu_list {
	uint32 version;
	uint32 length;
	long timestamp;
	tx_ampdu_t ampdus[1];
} tx_ampdu_list_t;

typedef struct congestion_t_ {
	uint32 channel; /* Channel number */
	/* Following 9 counters are CCA stats */
	uint32 tx;
	uint32 inbss;
	uint32 obss;
	uint32 nocat;
	uint32 nopkt;
	uint32 doze;
	uint32 txop;
	uint32 goodtx;
	uint32 badtx;
	uint32 glitchcnt;	/* Glitch per second count */
	uint32 badplcp;	/* bad plcp per second count */
	int8 knoise;	/* background noise level (in dBm) */
	uint8 chan_idle; /* Idle */
	uint32 busy;
	uint32 availcap;
	uint32 wifi;
	uint32 nonwifi;
	long timestamp;
} congestion_t;

typedef struct congestion_list_t_ {
	uint32 length;
	long timestamp;
	congestion_t congest[1];
} congestion_list_t;

typedef struct graphs_t_ {
	long timestamp;
	int xvalue;
	char yvalue[MAX_YVALUE_LEN];
} graphs_t;

typedef struct graph_list_t_ {
	uint32 length;
	long timestamp;
	char mac[ETHER_ADDR_LEN * 3]; /* Only in case of perSTA graph */
	graphs_t graph[1];
} graph_list_t;

typedef struct graphdetails_t_ {
	int rowid;
	char tab[50];
	char table[50];
	int graphtype;
} graphdetails_t;

typedef struct mutexes_t_ {
	char *tablename;
	pthread_mutex_t dbmutex;
	struct mutexes_t_ *next;
} mutexes_t;

typedef struct table_t_ {
	uint32 rowid;
	char name[MAX_GRAPH_NAME];
	char tablename[MAX_TABLE_NAME];
	char tab[MAX_GRAPH_NAME];
} table_t;

typedef struct tablelist_t_ {
	uint32 alloclen;
	uint32 length;
	table_t tables[1];
} tablelist_t;

typedef struct newclient_t_ {
	int sockfd;
	char hostaddr[INET6_ADDRSTRLEN];
} newclient_t;

typedef struct graphname_t_ {
	uint32 rowid;
	char tab[MAX_TAB_NAME];
	char name[MAX_GRAPH_NAME];
	char heading[MAX_TAB_NAME];
	char plotwith[MAX_GRAPH_NAME];
	uint32 type;
	uint32 persta;
	char barheading[MAX_GRAPH_NAME];
	char xaxisname[MAX_GRAPH_NAME];
	char yaxisname[MAX_GRAPH_NAME];
	char tablename[MAX_TABLE_NAME];
	int enable;
} graphname_t;

typedef struct graphnamelist_t_ {
	uint32 length;
	graphname_t graphname[1];
} graphnamelist_t;

typedef struct onlygraphname_t_ {
	char name[MAX_GRAPH_NAME];
} onlygraphname_t;

typedef struct onlygraphnamelist_t_ {
	uint32 length;
	onlygraphname_t graphname[1];
} onlygraphnamelist_t;

typedef struct channel_map_t_ {
	char ssid[SSID_FMT_BUF_LEN];
	uint32 ctrlch;
	uint32 channel;
	uint32 bandwidth;
	int rssi;
} channel_map_t;

typedef struct channel_maplist_t_ {
	uint32 length;
	channel_map_t channelmap[1];
} channel_maplist_t;

typedef struct dut_settings_t_ {
	uint32 rowid;
	uint32 isscan;
	uint32 band;
	char mac[ETHER_ADDR_LEN * 3];
} dut_settings_t;

typedef struct alldut_settings_t_ {
	uint32 length;
	uint32 ncount;
	dut_settings_t dutset[1];
} alldut_settings_t;

/* Individual RRM statistics of the STA */
typedef struct vis_rrm_stats_t_ {
	long timestamp;			/* Timestamp */
	char mac[ETHER_ADDR_LEN * 3];	/* MAC address of the STA */
	uint32 txop;			/* transmission opportunity */
	uint32 pktrequested;		/* Number of packets requested */
	uint32 pktdropped;		/* Number of packets dropped */
	uint32 pktstored;		/* Number of packets stored */
	uint32 pktretried;		/* Number of packets retried */
	uint32 pktacked;		/* Number of packets acked */
	unsigned long read_flag;	/* Which and all the reports have been processed */
} vis_rrm_stats_t;

/* RRM Statistics for all the STA's */
typedef struct vis_rrm_statslist_t_ {
	uint32 length;			/* Number of Stations */
	long timestamp;			/* Timestamp */
	vis_rrm_stats_t rrmstats[1];	/* RRM Stats for all STA's */
} vis_rrm_statslist_t;

/* Individual STA statistics of the STA */
typedef struct vis_sta_stats_t_ {
	long timestamp;			/* Timestamp */
	char mac[ETHER_ADDR_LEN * 3];	/* MAC address of the STA */
	int generation_flag;		/* WiFi devices generation bit flag (a/b/g/n or ac) */
	uint8 txstream;			/* MIMO tx Streams */
	uint8 rxstream;			/* MIMO rx streams */
	int32 rate;			/* Rate */
	uint32 idle;			/* idle from sta_info */
} vis_sta_stats_t;

/* RRM Statistics for all the STA's */
typedef struct vis_sta_statslist_t_ {
	uint32 length;			/* Number of Stations */
	long timestamp;			/* Timestamp */
	vis_sta_stats_t stastats[1];	/* RRM Stats for all STA's */
} vis_sta_statslist_t;

/* Linked list to hold all the wl interface details and IOVAR output data */
typedef struct vis_wl_interface_t_ {
	int			enabled;	/* 1 If the interface is enabled */
	struct ifreq		ifr;		/* wl driver adapter */
	dut_info_t		dut_info;	/* To store dut details */
	counters_t		counters;	/* To store output of counters IOVAR */
	counters_t		oldcounters;	/* To store old counters result */
	networks_list_t		*networks_list;	/* To store scan result result */
	assoc_sta_list_t	*stas_list;	/* To store associated STA result */
	tx_ampdu_list_t		*ampdulist;	/* To store AMPDU result */
	congestion_list_t	*congestion;	/* To store chanim result */
	dut_settings_t		dutset;		/* To store settings of the DUT */
	vis_rrm_statslist_t	*rrmstatslist;	/* To store RRM STA side advance statistics */
	vis_sta_statslist_t	*stastatslist;	/* To store STA info statistics */

	struct vis_wl_interface_t_	*next;
} vis_wl_interface_t;

#endif /* _VIS_STRUCT_H_ */
