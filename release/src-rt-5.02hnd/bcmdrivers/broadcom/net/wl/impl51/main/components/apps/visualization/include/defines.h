/*
 * Linux Visualization System common defines header
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
 * $Id: defines.h 622375 2016-03-02 12:21:04Z $
 */

#ifndef _VIS_DEFINES_H_
#define _VIS_DEFINES_H_

#include <typedefs.h>

/* Debug Message defines */
/* #define SHOW_TIME_ELAPSED */
/* #define SHOW_FUN_START_END_DEBUG_MSG */

#define VIS_DEBUG_ERROR			0x0001
#define VIS_DEBUG_WARNING		0x0002
#define VIS_DEBUG_INFO			0x0004
#define VIS_DEBUG_DETAIL		0x0008
#define VIS_DEBUG_ADAPTER		0x0010
#define VIS_DEBUG_SCAN			0x0020
#define VIS_DEBUG_CHANIM		0x0040
#define VIS_DEBUG_METRICS		0x0080
#define VIS_DEBUG_SOCK			0x0100
#define VIS_DEBUG_XML			0x0200
#define VIS_DEBUG_JSON			0x0400
#define VIS_DEBUG_DB			0x0800
#define VIS_DEBUG_SYNCH			0x1000
#define VIS_DEBUG_SAVEXMLFILE		0x2000
#define VIS_DEBUG_RRMSTATS		0x4000
#define VIS_DEBUG_DB_RRMSTATS		0x8000

#define TWO_CPU_ARCHITECTURE
#define STORE_GRAPHS_IN_LIST
#define USE_MUTEXES

#define TIMER_BASED
/* #define COMMAND_LINE */

#define LOOPBACK_IP	"127.0.0.1"
#define VISUALIZATION_SERVER_PORT	8888
#define VISUALIZATION_SERVER_ADDRESS	"127.0.0.1"

/* Max Length defines */
#define MAX_TAB_NAME		30
#define MAX_GRAPH_NAME		50
#define MAX_TABLE_NAME		50
#define MAX_GRAPHS		20
#define MAX_GRAPH_ENTRY		10
#define MAX_REASON_LEN		255
#define MAX_DELETE_ROWS		10
#define MAX_REQUEST_NAME	50
#define MAX_YVALUE_LEN		11

#define MAX_VIS_MCS_INDEX	63

#define MAX_NETWORK_TYPE	8
#define MAX_MULTICAST_RSN	10
#define MAX_UNICAST_RSN		50
#define MAX_AKM_TYPE		50

#define MAX_IP_LEN			25

#define MAX_DUTS			5

#define MAX_PATH		256
#define VIS_LEN_FIELD_SZ	4 /* Number of bytes used to specify the length of the packet */

/* Graph Types */
#define GRAPH_TYPE_BAR			1
#define GRAPH_TYPE_BAR_AGAINST_TIME	2
#define GRAPH_TYPE_LINE			3
#define GRAPH_TYPE_LINE_AGAINST_TIME	4

/* Some default config settings */
#define DEFAULT_INTERVAL	5	/* 5 Seconds */
#define DEFAULT_DURATION	0	/* Indefinite */
#define DEFAULT_FREQUENCY	1800	/* 30 Minutes */
#define DEFAULT_DBSIZE		2	/* 2 MB */

/* From wlc_rate.[ch] */
#define MCS_TABLE_SIZE		33
#define MCS_ACTABLE_SIZE	40

/* Supported Frequency Idx */
#define MCS_20	1
#define MCS_40	2
#define MCS_80	3

#define VHT_CAP_MCS_MAP_0_7_MAX_IDX 8
#define VHT_CAP_MCS_MAP_0_8_MAX_IDX 9
#define VHT_CAP_MCS_MAP_0_9_MAX_IDX 10
#define VHT_CAP_MCS_MAP_0_11_MAX_IDX 12

#define NETWORK_TYPE_A	0x1
#define NETWORK_TYPE_B	0x2
#define NETWORK_TYPE_G	0x4
#define NETWORK_TYPE_N	0x8
#define NETWORK_TYPE_AC	0x10

/* buffer length needed for wl_format_ssid
 * 32 SSID chars, max of 4 chars for each SSID char "\xFF", plus NULL
 */
#define SSID_FMT_BUF_LEN (4*32+1)	/* Length for SSID format string */

/* Packet Structure XML file version */
#define PACKET_VERSION		1

/* Packet Type Request or Response */
#define PACKET_TYPE_REQ		1 /* Request */
#define PACKET_TYPE_RESP	2 /* Response */

/* Packet from Data collector, from WEB or From data concentrator */
#define PACKET_FROM_DC		1 /* Data is from Datacollector */
#define PACKET_FROM_WEB		2 /* Data is from Web */
#define PACKET_FROM_DCON	3 /* Data is from Dataconcentrator */

/* Response from the server */
#define PACKET_SENT_SUCCESS		0 /* Data Recieved or Sent Successfully */
#define PACKET_SENT_FAILED		1 /* Failed to send or Recieve the data */
#define PACKET_ERR_PARSE_PACKET		2
#define PACKET_ERR_DB_NOT_FOUND		3
#define PACKET_ERR_DB_READ		4
#define PACKET_ERR_DB_WRITE		5

/* RRM report bit flags */
#define RRM_BIT_CHLOAD		(1 << 0)
#define RRM_BIT_STASTAT0	(1 << 1)
#define RRM_BIT_STASTAT1	(1 << 2)
#define RRM_BIT_STASTAT2	(1 << 3)
#define RRM_BIT_STASTAT3	(1 << 4)
#define RRM_BIT_STASTAT4	(1 << 5)
#define RRM_BIT_STASTAT5	(1 << 6)
#define RRM_BIT_STASTAT6	(1 << 7)
#define RRM_BIT_STASTAT7	(1 << 8)
#define RRM_BIT_STASTAT8	(1 << 9)
#define RRM_BIT_STASTAT9	(1 << 10)
#define RRM_STA_READ		2046	/* includes all sta stats */

/* Request Response type */
#define PACKET_REQLOGIN			"ConfigSettings"
#define PACKET_REQCONFIG		"ConfigSettings"
#define PACKET_REQSETCONFIG		"SetConfig"
#define PACKET_REQNETWORKS		"ScanResults"
#define PACKET_REQCOUNTERS		"ReqCounters"
#define PACKET_ASSOC_STA		"AssociatedSTA"
#define PACKET_REQCHANNELCAPACITY	"ChannelCapacity"
#define PACKET_REQAMPDUSTATISTICS	"AMPDUStatistics"
#define PACKET_REQGETDBFILE			"GetDBFile"
#define PACKET_REQCHANIM			"Chanim Statistics"
#define PACKET_REQBANDS			"GetBands"
#define PACKET_REQALLDUT		"AllDUT"
#define PACKET_REQRRMSTASTATS		"RRMSTAStats"
#define PACKET_REQRRMADVSTASTATS	"AdvancedPerStationDetails"

/* All XML tag names of Packet structure XML file */
#define TAG_PACKETVERSION	"PacketVersion"
#define TAG_TOTAL		"Total"
#define TAG_PACKETHEADER	"PacketHeader"
#define TAG_PACKETTYPE		"PacketType"
#define TAG_PACKETFROM		"From"
#define TAG_REQUESTRESPTYPE	"ReqRespType"
#define TAG_RESULT		"Result"
#define TAG_REASON		"Reason"
#define TAG_FREQBAND		"FreqBand"
#define TAG_RECORDSTOFETCH	"RecordsToFetch"
#define TAG_NUMBER		"Number"
#define TAG_RECORDS		"Records"
#define TAG_FTIMESTAMP		"fTimeStamp"
#define TAG_LTIMESTAMP		"lTimeStamp"
#define	TAG_CONFIG		"Config"
#define TAG_SAMPLEINTERVAL	"SampleInterval"
#define TAG_DBSIZE		"dbsize"
#define TAG_ISSTART		"startstop"
#define TAG_ISSTARTSCAN		"startscan"
#define TAG_ISSTARTCHANIM	"startchanim"
#define TAG_ISREMOTEENAB	"isremote"
#define TAG_GATEWAYIP		"gatewayip"
#define TAG_ENABLEDGRAPHS	"EnabledGraphs"
#define TAG_ISAUTOSTART		"autost"
#define TAG_WEEKDAYS		"wkdays"
#define TAG_FROMTM			"frmtm"
#define TAG_TOTM			"totm"
#define TAG_GRAPHNAME		"graphname"
#define TAG_ISOVERWRITEDB	"overwrtdb"
#define TAG_DUT			"DUT"
#define TAG_TIMESTAMP		"TimeStamp"
#define TAG_ISAP		"isAP"
#define TAG_BSSID		"BSSID"
#define TAG_SSID		"SSID"
#define TAG_MAC			"MAC"
#define TAG_STAMAC		"STAMAC"
#define TAG_VALUE		"Value"
#define TAG_NETWORKS		"Networks"
#define TAG_AP			"AP"
#define TAG_RSSI		"RSSI"
#define TAG_NOISE		"Noise"
#define TAG_CHANNEL		"Channel"
#define TAG_CONTROLCH		"ControlCH"
#define TAG_BAND		"Band"
#define TAG_BANDWIDTH		"Bandwidth"
#define TAG_RATE		"Rate"
#define TAG_TYPE		"Type"
#define TAG_MCAST_RSN		"MCastRSN"
#define TAG_UCAST_RSN		"UCastRSN"
#define TAG_AKM			"AKM"
#define TAG_ASSOCIATEDSTA	"AssociatedSTA"
#define TAG_STA			"STA"
#define TAG_PHYRATE		"PhyRate"
#define TAG_GRAPH		"Graph"
#define TAG_GRAPH_TYPE		"GraphType"
#define TAG_NAME		"Name"
#define TAG_HEADING		"Heading"
#define TAG_PLOTWITH		"PlotWith"
#define TAG_PERSTA		"PerSTA"
#define TAG_BAR_HEADING		"BarHeading"
#define TAG_X_AXIS		"XAxis"
#define TAG_Y_AXIS		"YAxis"
#define TAG_X_VALUE		"XValue"
#define TAG_Y_VALUE		"YValue"
#define TAG_CHANNELSTATS	"ChannelStats"
#define TAG_STATS	"Stats"
#define TAG_TX		"tx"
#define TAG_INBSS	"inbss"
#define TAG_OBSS	"obss"
#define TAG_NOCAT	"nocat"
#define TAG_NOPKT	"nopkt"
#define TAG_DOZE	"doze"
#define TAG_TXOP	"txop"
#define TAG_GOODTX	"gdtx"
#define TAG_BADTX	"badtx"
#define TAG_GLITCH	"glitch"
#define TAG_BADPLCP	"plcp"
#define TAG_KNOISE	"noise"
#define TAG_IDLE	"idle"
#define TAG_AMPDUSTATS		"AMPDUStats"
#define TAG_AMPDU			"AMPDU"
#define TAG_MCS				"MCS"
#define TAG_TXMCS			"TXMCS"
#define TAG_TXMCSPERCENT	"TXMCSPercent"
#define TAG_TXMCSSGI		"TXMCSSGI"
#define TAG_TXMCSSGIPERCENT	"TXMCSSGIPercent"
#define TAG_RXMCS			"RXMCS"
#define TAG_RXMCSPERCENT	"RXMCSPercent"
#define TAG_RXMCSSGI		"RXMCSSGI"
#define TAG_RXMCSSGIPERCENT	"RXMCSSGIPercent"
#define TAG_TXVHT			"TXVHT"
#define TAG_TXVHTPER		"TXVHTPER"
#define TAG_TXVHTPERCENT	"TXVHTPercent"
#define TAG_TXVHTSGI		"TXVHTSGI"
#define TAG_TXVHTSGIPER		"TXVHTSGIPER"
#define TAG_TXVHTSGIPERCENT	"TXVHTSGIPercent"
#define TAG_RXVHT			"RXVHT"
#define TAG_RXVHTPER		"RXVHTPER"
#define TAG_RXVHTPERCENT	"RXVHTPercent"
#define TAG_RXVHTSGI		"RXVHTSGI"
#define TAG_RXVHTSGIPER		"RXVHTSGIPER"
#define TAG_RXVHTSGIPERCENT	"RXVHTSGIPercent"
#define TAG_MPDUDENS		"MPDUDens"
#define TAG_MPDUDENSPERCENT	"MPDUDensPercent"
#define TAG_DUTSETTINGS		"DUTSet"
#define TAG_DOSCAN			"DoScan"
#define TAG_ISENABLED		"Enabled"
#define TAG_RRMSTASTATS		"RRMSTAStats"
#define TAG_RRMSTA		"RRMSTA"
#define TAG_STAINFOSTATS	"STAInfoStats"
#define TAG_STASTATS		"STAStats"
#define TAG_GENERATION		"generation"
#define TAG_TXSTREAM		"txstream"
#define TAG_RXSTREAM		"rxstream"
#define TAG_PKTREQUESTED	"pktreq"
#define TAG_PKTDROPPED		"pktdrop"
#define TAG_PKTSTORED		"pktstore"
#define TAG_PKTRETRIED		"pktretried"
#define TAG_PKTACKED		"pktacked"

/* All the attribute names for packet structure XML file */
#define ATTRIB_NAME	"Name"
#define ATTRIB_TABLE	"Table"
#define ATTRIB_COLUMN	"Column"

/* Default TAB's Present in the UI */
#define TAB_NETWORKS		"Networks"
#define TAB_CHANNEL_STATISTICS	"Channel Statistics"
#define TAB_METRICS		"Metrics"

/* Names of IOVARS needs to be queried from driver */
#define IOVAR_NAME_CHANNELTSTATS		"Channel Statistics"
#define IOVAR_NAME_AMPDUTXWITHOUTSGI	"AMPDU TX Without SGI"
#define IOVAR_NAME_AMPDUTXWITHSGI		"AMPDU TX With SGI"
#define IOVAR_NAME_AMPDUTXVHTWITHOUTSGI	"AMPDU TX VHT Without SGI"
#define IOVAR_NAME_AMPDUTXVHTWITHSGI	"AMPDU TX VHT With SGI"
#define IOVAR_NAME_AMPDURXWITHOUTSGI	"AMPDU RX Without SGI"
#define IOVAR_NAME_AMPDURXWITHSGI		"AMPDU RX With SGI"
#define IOVAR_NAME_AMPDURXVHTWITHOUTSGI	"AMPDU RX VHT Without SGI"
#define IOVAR_NAME_AMPDURXVHTWITHSGI	"AMPDU RX VHT With SGI"
#define IOVAR_NAME_AMPDUMPDUDENSITY		"MPDU Density"
#define IOVAR_NAME_RXCRSGLITCH			"Rx CRS Glitches"
#define IOVAR_NAME_BADPLCP				"Bad PLCP"
#define IOVAR_NAME_BADFCS				"Bad FCS"
#define IOVAR_NAME_CHANIM				"Chanim Statistics"
#define IOVAR_NAME_PACKETREQUESTED		"Packet Requested"
#define IOVAR_NAME_PACKETSTORED			"Packet Stored"
#define IOVAR_NAME_PACKETDROPPED		"Packet Dropped"
#define IOVAR_NAME_PACKETRETRIED		"Packet Retried"
#define IOVAR_NAME_QUEUEUTILIZATION		"Queue Utilization"
#define IOVAR_NAME_QUEUELENGTH			"Queue Length Per Precedence"
#define IOVAR_NAME_DATATHROUGHPUT		"Data Throughput"
#define IOVAR_NAME_PHYSICALRATE			"Physical Rate"
#define IOVAR_NAME_RTSFAIL				"RTS Fail"
#define IOVAR_NAME_RTRYDROP				"Retry Drop"
#define IOVAR_NAME_PSRETRY				"PS Retry"
#define IOVAR_NAME_ACKED				"Acked"

#define	WF_40MHZ_L	"l"
#define WF_40MHZ_U	"u"
#define WF_80MHZ_L	"/80"
#define WF_80MHZ_U	"/80"
#define WF_160MHZ_L	"/160"
#define WF_160MHZ_U	"/160"

/* 40MHz channels in 2GHz band */
static const uint8 wf_2g_40m_chans[] =
{3, 4, 5, 6, 7, 8, 9, 10, 11};
#define WF_NUM_5G_40M_CHANS \
	(sizeof(wf_5g_40m_chans)/sizeof(uint8))

/* 40MHz channels in 5GHz band */
static const uint8 wf_5g_40m_chans[] =
{38, 46, 54, 62, 102, 110, 118, 126, 134, 142, 151, 159};
#define WF_NUM_5G_40M_CHANS \
	(sizeof(wf_5g_40m_chans)/sizeof(uint8))

/* 80MHz channels in 5GHz band */
static const uint8 wf_5g_80m_chans[] =
{42, 58, 106, 122, 138, 155};
#define WF_NUM_5G_80M_CHANS \
	(sizeof(wf_5g_80m_chans)/sizeof(uint8))

/* 160MHz channels in 5GHz band */
static const uint8 wf_5g_160m_chans[] =
{50, 114};
#define WF_NUM_5G_160M_CHANS \
	(sizeof(wf_5g_160m_chans)/sizeof(uint8))

/* DEBUG Message Printing Macros */
#ifdef SHOW_TIME_ELAPSED
#define SHOW_TIME_ELAPSE_START() time_t start, end; time(&start);

#define SHOW_TIME_ELAPSE_END() time(&end); \
	double dif = difftime(end, start); \
	fprintf(stdout, "Elasped time in %s is %.2lf seconds.\n", __FUNCTION__, dif);
#else
#define SHOW_TIME_ELAPSE_START()
#define SHOW_TIME_ELAPSE_END()
#endif /* SHOW_TIME_ELAPSED */

#ifdef SHOW_FUN_START_END_DEBUG_MSG
#define FUN_START_DEBUG_MSG() fprintf(stdout, "***** Function : %s Started *****\n", \
	__FUNCTION__);
#define FUN_END_DEBUG_MSG() fprintf(stdout, "***** Function : %s Ended *****\n", \
	__FUNCTION__);
#else
#define FUN_START_DEBUG_MSG()
#define FUN_END_DEBUG_MSG()
#endif /* SHOW_FUN_START_END_DEBUG_MSG */

extern int vis_debug_level;

#define VIS_ERROR(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_ERROR) \
			printf("VIS-ERROR >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_WARNING(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_WARNING) \
			printf("VIS-WARNING >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_INFO(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_INFO) \
			printf("VIS-INFO >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_DEBUG(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_DETAIL) \
			printf("VIS-DEBUG >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_ADAPTER(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_ADAPTER) \
			printf("VIS-ADAPTER >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_SCAN(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_SCAN) \
			printf("VIS-SCAN >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_CHANIM(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_CHANIM) \
			printf("VIS-CHANIM >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_METRICS(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_METRICS) \
			printf("VIS-METRICS >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_SOCK(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_SOCK) \
			printf("VIS-SOCK >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_XML(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_XML) \
			printf("VIS-XML >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_JSON(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_JSON) \
			printf("VIS-JSON >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_DB(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_DB) \
			printf("VIS-DB >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_SYNCH(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_SYNCH) \
			printf("VIS-SYNCH >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_RRM(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_RRMSTATS) \
			printf("VIS-RRM >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_DB_RRM(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_DB_RRMSTATS) \
			printf("VIS-RRM >> : "fmt, ##arg); } while (0)

#define VIS_PRINT(fmt, arg...) \
		do { printf("VIS-PRINT >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#endif /* _VIS_DEFINES_H_ */
