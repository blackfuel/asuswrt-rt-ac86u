/*
 * Linux Visualization System common utility function implementation
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
 * $Id: vis_shared_util.c 555336 2015-05-08 09:52:04Z $
 */

#include "vis_shared_util.h"

/* generic IOVAR handler
 * Any New IOVAR added will have one entry here
 */
iovar_handler_t iovar_handler[] = {
	{ IOVAR_NAME_CHANNELTSTATS, "Channel Statistics", IOVAR_NAME_CHANNELTSTATS,
	GRAPH_TYPE_BAR, 0,
	"Congestion(Wi-Fi)", "Channels", "Percentage", 1},

	{ IOVAR_NAME_AMPDUTXWITHOUTSGI, "AMPDU Statistics", IOVAR_NAME_AMPDUTXWITHOUTSGI,
	GRAPH_TYPE_BAR, 0,
	"AMPDU Tx MCS Without SGI", "Tx MCS Rates", "% Packets", 0},

	{ IOVAR_NAME_CHANIM, "Chanim Statistics", IOVAR_NAME_CHANIM,
	GRAPH_TYPE_LINE_AGAINST_TIME, 0,
	IOVAR_NAME_CHANIM, "Time", "Count", 1},

	{ IOVAR_NAME_RXCRSGLITCH, "Glitch Counter Statistics", IOVAR_NAME_RXCRSGLITCH,
	GRAPH_TYPE_LINE_AGAINST_TIME, 0,
	"Rx CRS Glitches", "Time", "Count", 1},

	{ IOVAR_NAME_BADPLCP, "Glitch Counter Statistics", IOVAR_NAME_RXCRSGLITCH,
	GRAPH_TYPE_LINE_AGAINST_TIME, 0,
	"Bad PLCP", "Time", "Count", 1},

	{ IOVAR_NAME_BADFCS, "Glitch Counter Statistics", IOVAR_NAME_RXCRSGLITCH,
	GRAPH_TYPE_LINE_AGAINST_TIME, 0,
	"Bad FCS", "Time", "Count", 1},

	{ IOVAR_NAME_PACKETREQUESTED, "Packet Queue Statistics", IOVAR_NAME_PACKETREQUESTED,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_PACKETREQUESTED, "Time", "Count", 1},

	{ IOVAR_NAME_PACKETSTORED, "Packet Queue Statistics", IOVAR_NAME_PACKETREQUESTED,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_PACKETSTORED, "Time", "Count", 1},

	{ IOVAR_NAME_PACKETDROPPED, "Packet Queue Statistics", IOVAR_NAME_PACKETREQUESTED,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_PACKETDROPPED, "Time", "Count", 1},

	{ IOVAR_NAME_PACKETRETRIED, "Packet Queue Statistics", IOVAR_NAME_PACKETREQUESTED,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_PACKETRETRIED, "Time", "Count", 1},

	{ IOVAR_NAME_QUEUEUTILIZATION, "Packet Queue Statistics", IOVAR_NAME_QUEUEUTILIZATION,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_QUEUEUTILIZATION, "Time", "Count", 1},

	{ IOVAR_NAME_QUEUELENGTH, "Packet Queue Statistics", IOVAR_NAME_QUEUEUTILIZATION,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_QUEUELENGTH, "Time", "Count", 1},

	{ IOVAR_NAME_DATATHROUGHPUT, "Packet Queue Statistics", IOVAR_NAME_DATATHROUGHPUT,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	"Data Throughput", "Time", "Mbits/s", 1},

	{ IOVAR_NAME_PHYSICALRATE, "Packet Queue Statistics", IOVAR_NAME_DATATHROUGHPUT,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	"Physical Rate", "Time", "Mbits/s", 1},

	{ IOVAR_NAME_RTSFAIL, "Packet Queue Statistics", IOVAR_NAME_RTSFAIL,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_RTSFAIL, "Time", "Count", 1},

	{ IOVAR_NAME_RTRYDROP, "Packet Queue Statistics", IOVAR_NAME_RTSFAIL,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_RTRYDROP, "Time", "Count", 1},

	{ IOVAR_NAME_PSRETRY, "Packet Queue Statistics", IOVAR_NAME_PSRETRY,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_PSRETRY, "Time", "Count", 1},

	{ IOVAR_NAME_ACKED, "Packet Queue Statistics", IOVAR_NAME_PSRETRY,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_ACKED, "Time", "Count", 1},
};

/* Gets the iovar structure handle from the iovar name */
iovar_handler_t*
find_iovar(const char *name)
{
	iovar_handler_t *tmpiovar;

	for (tmpiovar = iovar_handler; tmpiovar->name && strcmp(tmpiovar->name, name); tmpiovar++);
	if (tmpiovar->name == NULL) {
		tmpiovar = NULL;
	}

	return tmpiovar;
}

/* Get the number of iovar's */
int
get_iovar_count()
{
	return (sizeof(iovar_handler)/sizeof(iovar_handler_t));
}

/* get IOVAR handle given the index */
iovar_handler_t*
get_iovar_handler(int idx)
{
	iovar_handler_t		*tmpiovar;
	int			i = 0;

	for (tmpiovar = iovar_handler; tmpiovar->name && i != idx; tmpiovar++, i++);
	if (tmpiovar->name == NULL) {
		tmpiovar = NULL;
	}

	return tmpiovar;
}
