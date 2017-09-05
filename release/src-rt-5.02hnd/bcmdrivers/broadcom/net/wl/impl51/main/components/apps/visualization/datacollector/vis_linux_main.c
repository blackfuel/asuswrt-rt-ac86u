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
 * $Id: vis_linux_main.c 616026 2016-01-29 11:14:36Z $
 */
#include "vis_linux_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#ifndef TARGETENV_android
#include <error.h>
#endif /* TARGETENV_android */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#ifdef TWO_CPU_ARCHITECTURE
#include <netdb.h>
#include <linux/rtnetlink.h>
#endif /* TWO_CPU_ARCHITECTURE */
#include <proto/ethernet.h>
#include <proto/bcmip.h>
#include <syslog.h>

#include <wlioctl.h>
#include <bcmutils.h>
#include <linux/types.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>

#include <sys/time.h>
#include <signal.h>
#include <bcmnvram.h>

#include "vis_wlmetrics.h"
#include "vis_socketclient.h"
#include "vis_wl.h"
#include "vis_wlcongestion.h"
#include "vis_wlmetrics.h"
#include "vis_xmlshared.h"
#include "vis_xmlutility.h"
#include "vis_rrmprocess.h"

/* #define TIMER_BASED */

#define DEV_TYPE_LEN					3 /* length for devtype 'wl'/'et' */
#define NO_ERROR						0

struct timeval g_tmstarted;

#ifdef TWO_CPU_ARCHITECTURE

#define BUFSIZE	8192

struct route_info
{
	struct in_addr dstAddr;
	struct in_addr srcAddr;
	struct in_addr gateWay;
	char ifName[IF_NAMESIZE];
};

enum vis_enum_weekdays {
	VIS_ENUM_SUNDAY = 1 << 0,
	VIS_ENUM_MONDAY = 1 << 1,
	VIS_ENUM_TUESDAY = 1 << 2,
	VIS_ENUM_WEDNESDAY = 1 << 3,
	VIS_ENUM_THURSDAY = 1 << 4,
	VIS_ENUM_FRIDAY = 1 << 5,
	VIS_ENUM_SATURDAY = 1 << 6
};

static int
readNlSock(int sockFd, char *bufPtr, int seqNum, int pId)
{
	struct nlmsghdr *nlHdr;
	int readLen = 0, msgLen = 0;

	do {
		/* Recieve response from the kernel */
		if ((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0) {
			VIS_SOCK("SOCK READ %s\n", strerror(errno));
			return -1;
		}

		nlHdr = (struct nlmsghdr *)bufPtr;

		/* Check if the header is valid */
		if ((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR)) {
			VIS_SOCK("Error in recieved packet %s\n", strerror(errno));
			return -1;
		}

		/* Check if the its the last message */
		if (nlHdr->nlmsg_type == NLMSG_DONE) {
			break;
		} else {
			/* Else move the pointer to buffer appropriately */
			bufPtr += readLen;
			msgLen += readLen;
		}

		/* Check if its a multi part message */
		if ((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0) {
			/* return if its not */
			break;
		}
	} while ((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));

	return msgLen;
}

/* parse the route info returned */
static int
parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo)
{
	struct rtmsg *rtMsg;
	struct rtattr *rtAttr;
	int rtLen, found = 0;

	rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

	/* If the route is not for AF_INET or does not belong to main routing table then return. */
	if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
		return found;

	/* get the rtattr field */
	rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
	rtLen = RTM_PAYLOAD(nlHdr);

	for (; RTA_OK(rtAttr, rtLen); rtAttr = RTA_NEXT(rtAttr, rtLen)) {
		switch (rtAttr->rta_type)
		{
			case RTA_OIF:
				if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
				break;

			case RTA_GATEWAY:
				memcpy(&rtInfo->gateWay, RTA_DATA(rtAttr), sizeof(rtInfo->gateWay));
				found = 1;
				break;

			case RTA_PREFSRC:
				memcpy(&rtInfo->srcAddr, RTA_DATA(rtAttr), sizeof(rtInfo->srcAddr));
				break;

			case RTA_DST:
				memcpy(&rtInfo->dstAddr, RTA_DATA(rtAttr), sizeof(rtInfo->dstAddr));
				found = 1;
				break;
		}
	}

	return found;
}

/* Get the gateway IP address */
static int
get_gatewayip(char *gatewayip, socklen_t size)
{
	int found_gatewayip = 0;

	struct nlmsghdr *nlMsg;
	struct route_info *rtInfo;
	char msgBuf[BUFSIZE]; /* pretty large buffer */

	int sock, len, msgSeq = 0;

	/* Create Socket */
	if ((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0) {
		VIS_SOCK("Socket Creation %s\n", strerror(errno));
		return (-1);
	}

	/* Initialize the buffer */
	memset(msgBuf, 0, BUFSIZE);

	/* point the header and the msg structure pointers into the buffer */
	nlMsg = (struct nlmsghdr *)msgBuf;

	/* Fill in the nlmsg header */
	nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); /* Length of message. */
	nlMsg->nlmsg_type = RTM_GETROUTE; /* Get the routes from kernel routing table */

	nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; /* The message is a request for dump */
	nlMsg->nlmsg_seq = msgSeq++; /* Sequence of the message packet */
	nlMsg->nlmsg_pid = getpid(); /* PID of process sending the request */

	/* Send the request */
	if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0) {
		VIS_SOCK("Write To Socket Failed %s\n", strerror(errno));
		return -1;
	}

	/* Read the response */
	if ((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0) {
		VIS_SOCK("Read From Socket Failed %s\n", strerror(errno));
		return -1;
	}

	/* Parse and print the response */
	rtInfo = (struct route_info *)malloc(sizeof(struct route_info));

	for (; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg, len)) {
		memset(rtInfo, 0, sizeof(struct route_info));
		if (parseRoutes(nlMsg, rtInfo) == 0)
			continue;

		/* Check if default gateway */
		if (strstr((char *)inet_ntoa(rtInfo->dstAddr), "0.0.0.0")) {
			/* copy it over */
			inet_ntop(AF_INET, &rtInfo->gateWay, gatewayip, size);
			found_gatewayip = 1;
			break;
		}
	}

	free(rtInfo);
	close(sock);

	return found_gatewayip;
}

#endif /* TWO_CPU_ARCHITECTURE */

/* Get the DUT info from driver for all wl interfaces */
static int
get_dut_info_from_driver()
{
	char strdutmac[ETHER_ADDR_LEN * 3];
	vis_wl_interface_t *curnode = g_wlinterface;

	while (curnode != NULL) {
		memset(strdutmac, 0x00, ETHER_ADDR_LEN*3);
		wl_get_mac(&curnode->ifr, strdutmac);

		wl_get_bss_info(&curnode->ifr, &curnode->dut_info);
		snprintf(curnode->dut_info.mac, (ETHER_ADDR_LEN * 3), "%s", strdutmac);
		if (strcmp(curnode->dut_info.bssid, "00:00:00:00:00:00") == 0)
			snprintf(curnode->dut_info.bssid, (ETHER_ADDR_LEN * 3),
				"%s", strdutmac);
		curnode->dut_info.isenabled = curnode->enabled;
		curnode = curnode->next;
	}

	return 0;
}

/* get all stats(scan result, associated sta's, chanim stats,
 * AMPDU, glitch and packet stats from driver
 */
static int
get_counter_from_driver()
{
	int ret = 0;
	int isscanned = 0;
	static int isfirsttime = 1;
	unsigned long diff;
	struct timeval curr_time;
	struct timeval prev_time;
	vis_wl_interface_t *curnode = g_wlinterface;

	g_timestamp = time(NULL);
	get_dut_info_from_driver();

	/* Start the scan only for first time or if the user selects scan in web UI */
	while (curnode != NULL) {
		if (curnode->enabled) {
			if (isfirsttime == 1 || curnode->dutset.isscan == 1) {
				wl_scan(&curnode->ifr);
				isscanned = 1;
			}
		}
		curnode = curnode->next;
	}
	gettimeofday(&prev_time, NULL);

	/* Now get all the remaining counters before getting the scan result */
	curnode = g_wlinterface;
	while (curnode != NULL) {
		if (curnode->enabled) {
			/* get the packet queue stats and associated STA list */
			curnode->stas_list = wl_get_counters_for_all_stas(&curnode->ifr);

			/* Get the RRM Stats for all the associated STA's */
			if (wl_is_rrm_enabled(&curnode->ifr)) {
				curnode->rrmstatslist = get_rrm_stats_for_all_sta(&curnode->ifr,
					curnode->stas_list, curnode->dut_info.mac,
					curnode->dut_info.ctrlch);
			}

			/* Get the Sta Stats for all the associated STA list */
			curnode->stastatslist = get_sta_stats_for_all_sta(&curnode->ifr,
				curnode->stas_list, curnode->dut_info.band);

			/* Get WL counters like glitch counter */
			wl_counters(&curnode->ifr, &curnode->counters, &curnode->oldcounters);

			/* Get the APMDU stats */
			curnode->ampdulist = wl_get_ampdu_stats(&curnode->ifr);

			/* Get the congestion statistics */
			curnode->congestion = wl_get_chanim_stats(&curnode->ifr);

			/* Copy the band and mac info to dut settings structure */
			curnode->dutset.band = curnode->dut_info.band;
			snprintf(curnode->dutset.mac, sizeof(curnode->dutset.mac), "%s",
				curnode->dut_info.mac);

		}
		curnode = curnode->next;
	}

	/* Get the current time and calculate the diff to wait for atleast 4 seconds
	 * to get the scan result after issuing scan. Note: only if it scanned
	 */
	if (isscanned) {
		gettimeofday(&curr_time, NULL);
		diff = curr_time.tv_sec - prev_time.tv_sec;
		if ((4 - diff) > 0)
			sleep(4 - diff);

		curnode = g_wlinterface;
		while (curnode != NULL) {
			if (curnode->enabled) {
				if (isfirsttime == 1 || curnode->dutset.isscan == 1)
					curnode->networks_list = wl_dump_networks(&curnode->ifr);

			}
			curnode = curnode->next;
		}
	}

	/* Now get the RRM data from the queue. wait for atleast 4 seconds to get the data */
	gettimeofday(&curr_time, NULL);
	diff = curr_time.tv_sec - prev_time.tv_sec;
	if ((4 - diff) > 0)
		sleep(4 - diff);

	curnode = g_wlinterface;
	while (curnode != NULL) {
		if (curnode->enabled && curnode->rrmstatslist) {
			get_rrm_report_from_queue(curnode->rrmstatslist);
		}
		curnode = curnode->next;
	}

	isfirsttime = 0;

	return ret;
}

/* Connects to server and sends data buffer */
static int
connect_and_send_data(char *xmldata, int sz)
{
	int sockfd	= INVALID_SOCKET;
	int ret		= 0;

	/* Connect to server */
	sockfd = connect_to_server(VISUALIZATION_SERVER_PORT, server_ip_address);
	if (sockfd != INVALID_SOCKET) {
		ret = add_length_and_send_data(sockfd, xmldata, sz);
		if (ret != INVALID_SOCKET) {
			unsigned char *rdata = NULL;
			int rsz = 0;
			rsz = on_receive(sockfd, &rdata);
			if (rsz > 0) {
				VIS_SOCK("Data recieved of size : %d is : %s\n", rsz, rdata);
				ret = parse_response(rdata, rsz);
			} else {
				ret = -1;
			}
			if (rdata != NULL)
				free(rdata);
		} else {
			ret = -1;
		}
	} else {
		VIS_SOCK("Failed to Connect to server : %s Port : %d\n",
			server_ip_address, VISUALIZATION_SERVER_PORT);
		ret = -1;
	}

	close_socket(&sockfd);

	return ret;
}

/* Creates and sends login packet to server */
static int
send_login_data()
{
	char *xmldata	= NULL;
	int sz		= 0;
	int ret		= 0;

	create_login_request(&xmldata, &sz);
	ret = connect_and_send_data(xmldata, sz);

	free_xmldata(&xmldata);

	return ret;
}

/* Creates and sends all dut info packet to server */
static int
send_all_dut_info()
{
	char *xmldata	= NULL;
	int sz		= 0;
	int ret		= 0;

	create_all_dut_info_request(&xmldata, &sz);
	ret = connect_and_send_data(xmldata, sz);

	free_xmldata(&xmldata);

	return ret;
}

/* Frees all the global structures used to hold data collected from driver */
static void
free_global_data_struct()
{
	vis_wl_interface_t *curnode = g_wlinterface;

	while (curnode != NULL) {
		if (curnode->networks_list != NULL) {
			free(curnode->networks_list);
			curnode->networks_list = NULL;
		}

		if (curnode->stas_list != NULL) {
			free(curnode->stas_list);
			curnode->stas_list = NULL;
		}

		if (curnode->ampdulist != NULL) {
			free(curnode->ampdulist);
			curnode->ampdulist = NULL;
		}

		if (curnode->congestion != NULL) {
			free(curnode->congestion);
			curnode->congestion = NULL;
		}

		if (curnode->rrmstatslist != NULL) {
			free(curnode->rrmstatslist);
			curnode->rrmstatslist = NULL;
		}

		if (curnode->stastatslist != NULL) {
			free(curnode->stastatslist);
			curnode->stastatslist = NULL;
		}

		memset(&curnode->counters, 0x00, sizeof(counters_t));

		curnode = curnode->next;
	}
}

/* Sends all the driver stats to server */
static int
send_counters_data(vis_wl_interface_t *curnode)
{
	char *xmldata	= NULL;
	int sz		= 0;
	int sockfd	= INVALID_SOCKET;
	int ret		= 0;

	/* Create the request to send the data to data collector */
	create_counters_request(curnode, &xmldata, &sz);

	if (sz > 0 && xmldata != NULL) {
		sockfd = connect_to_server(VISUALIZATION_SERVER_PORT, server_ip_address);
		if (sockfd != INVALID_SOCKET) {
			int ret = add_length_and_send_data(sockfd, xmldata, sz);
			free_xmldata(&xmldata);
			if (ret > 0) {
				unsigned char *data = NULL;
				int rsz = 0;
				rsz = on_receive(sockfd, &data);
				if (rsz > 0 && data != NULL) {
					VIS_SOCK("Data recieved of size : %d is : %s\n", rsz, data);
					parse_response(data, rsz);
					if (data != NULL) {
						free(data);
						data = NULL;
					}
				}
			}
		}
	}

	close_socket(&sockfd);

	/* Free all the allocated memory */
	if (xmldata != NULL) {
		free_xmldata(&xmldata);
		xmldata = NULL;
	}

	return ret;
}

static int
ioctl_queryinformation_fe(void *wl, int cmd, void* input_buf, unsigned long *input_len)
{
	int error = NO_ERROR;

	error = wl_ioctl(wl, cmd, input_buf, *input_len, FALSE);

	return error;
}

static int
ioctl_setinformation_fe(void *wl, int cmd, void* buf, unsigned long *input_len)
{
	int error = 0;

	error = wl_ioctl(wl,  cmd, buf, *input_len, TRUE);

	return error;
}

int
wl_get(void *wl, int cmd, void *buf, int len)
{
	int error = 0;
	unsigned long input_len = len;

	error = (int)ioctl_queryinformation_fe(wl, cmd, buf, &input_len);

	if (error == BCME_NODEVICE)
		return BCME_NODEVICE;
	else if (error != 0)
		return IOCTL_ERROR;

	return 0;
}

int
wl_set(void *wl, int cmd, void *buf, int len)
{
	int error = 0;
	unsigned long input_len = len;

	error = (int)ioctl_setinformation_fe(wl, cmd, buf, &input_len);

	if (error == BCME_NODEVICE)
		return BCME_NODEVICE;
	else if (error != 0)
		return IOCTL_ERROR;

	return 0;
}

static int
wl_get_dev_type(char *name, void *buf, int len)
{
	int s;
	int ret;
	struct ifreq ifr;
	struct ethtool_drvinfo info;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	/* get device type */
	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t)&info;
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {

		/* print a good diagnostic if not superuser */
		if (errno == EPERM)
			syserr("wl_get_dev_type");

		*(char *)buf = '\0';
	} else {
		strncpy(buf, info.driver, len);
	}

	close(s);
	return ret;
}

/* Add's a wl adapter node to the global linked list */
static void
add_interface_node(vis_wl_interface_t *tmpnode)
{
	vis_wl_interface_t *curnode = NULL;

	/* If head node is NULL make the adding node as head node */
	if (!g_wlinterface) {
		g_wlinterface = tmpnode;
		return;
	}

	/* else add it at the end */
	curnode = g_wlinterface;
	while (curnode->next != NULL)
		curnode = curnode->next;
	curnode->next = tmpnode;
}

/* Creates a empty wl adapter interface node */
static vis_wl_interface_t*
create_empty_interface_node()
{
	vis_wl_interface_t *tmpnode = NULL;

	tmpnode = (vis_wl_interface_t*)malloc(sizeof(vis_wl_interface_t));
	if (tmpnode == NULL) {
		VIS_ADAPTER("Failed to allocate memory for interface node\n");
		return NULL;
	}
	memset(tmpnode, 0x00, sizeof(vis_wl_interface_t));
	tmpnode->next = NULL;

	return tmpnode;
}

/* Free the global wl interface list */
static void
free_interface_nodes()
{
	vis_wl_interface_t *curnode = g_wlinterface;

	while (curnode != NULL) {
		g_wlinterface = g_wlinterface->next;
		free(curnode);
		curnode = g_wlinterface;
	}
}

/* Get's all the wl adapter and adds it to the linked list */
static void
get_all_wl_interfaces()
{
	char proc_net_dev[] = "/proc/net/dev";
	FILE *fp;
	char buf[1000], *c, *name;
	char dev_type[DEV_TYPE_LEN];

	if (!(fp = fopen(proc_net_dev, "r")))
		return;

	/* eat first two lines */
	if (!fgets(buf, sizeof(buf), fp) ||
	    !fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		return;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		vis_wl_interface_t *tmpnode;

		c = buf;
		while (isspace(*c))
			c++;
		if (!(name = strsep(&c, ":")))
			continue;
		tmpnode = create_empty_interface_node();
		if (tmpnode == NULL)
			continue;
		strncpy(tmpnode->ifr.ifr_name, name, IFNAMSIZ);
		if (wl_get_dev_type(name, dev_type, DEV_TYPE_LEN) >= 0 &&
			!strncmp(dev_type, "wl", 2)) {
			if (wl_check((void *) &tmpnode->ifr) == 0) {
				add_interface_node(tmpnode);
			} else { /* Not a valid adapter free the tmpnode */
				VIS_ADAPTER("%s is not a valid adapter\n", name);
				free(tmpnode);
			}
		} else { /* Not a wl adapter free the tmpnode */
			free(tmpnode);
		}
	}

	fclose(fp);
}

/* For all the wl adapter found, check's whether the adapter is up or not */
static void
vis_check_interfaces_are_up()
{
	vis_wl_interface_t *curnode = g_wlinterface;

	while (curnode != NULL) {
		wl_is_up(&curnode->ifr, &curnode->enabled);
		VIS_DEBUG("Interface %s's Enabled flag : %d\n", curnode->ifr.ifr_name,
			curnode->enabled);
		curnode = curnode->next;
	}
}

/* Initializes the global config settings structure */
static void
initialize_config()
{
	memset(&g_configs, 0x00, sizeof(configs_t));
	g_configs.isremoteenab = 0;
}

/* Get IP address and remote debug enabled flag from nvram */
static int
get_nvram_config_settings(char *ipaddress, size_t nip, int *isenabled)
{
	char *tmpipaddress = nvram_get("vis_dcon_ipaddr");
	if (tmpipaddress == NULL)
		return -1;
	snprintf(ipaddress, nip, "%s", tmpipaddress);

	char *value = nvram_get("vis_do_remote_dcon");
	if (value)
		*isenabled = atoi(value);

	VIS_DEBUG("IP address : %s\tIs Remote Enabled : %d\n", ipaddress, *isenabled);

	return 0;
}

/* Get debug level flag from nvram */
static int
get_nvram_debug_settings()
{
	char *tmpdebugflag = nvram_get("vis_debug_level");
	if (tmpdebugflag)
		vis_debug_level = strtoul(tmpdebugflag, NULL, 0);

	return 0;
}


/* Gets the dcon server IP address */
static void
get_server_ip_address()
{
#ifdef TWO_CPU_ARCHITECTURE
	char tmpserverip[MAX_IP_LEN] = {0};
	int isremoteenabled = 0;

	if (get_nvram_config_settings(tmpserverip, sizeof(tmpserverip), &isremoteenabled) == -1) {
		snprintf(server_ip_address, sizeof(server_ip_address), "%s", LOOPBACK_IP);
	} else {
		if (isremoteenabled == 1)
			snprintf(server_ip_address, sizeof(server_ip_address), "%s", tmpserverip);
		else
			snprintf(server_ip_address, sizeof(server_ip_address), "%s", LOOPBACK_IP);
	}
	g_configs.isremoteenab = isremoteenabled;
#else
	snprintf(server_ip_address, sizeof(server_ip_address), "%s", LOOPBACK_IP);
#endif /* TWO_CPU_ARCHITECTURE */
	VIS_DEBUG("ServerIP : %s\n", server_ip_address);
}

/* Gets the gateway Ip and updates the config structure */
static int
get_gateway_config_settings()
{
	int ret = 0;
#ifdef TWO_CPU_ARCHITECTURE
	ret = get_gatewayip(gateway_ip_address, MAX_IP_LEN);
	if (ret == 1) {
		VIS_DEBUG("GatewayIP : %s\n", gateway_ip_address);
	}
	snprintf(g_configs.gatewayip, sizeof(g_configs.gatewayip), "%s", gateway_ip_address);
#else
	ret = 1;
#endif /* TWO_CPU_ARCHITECTURE */

	return ret;
}

/* Checks whether the weekday is selected */
inline static int
is_weekday(int weekdays, enum vis_enum_weekdays curweekday)
{
	return ((weekdays & curweekday) > 0);
}

/* Gets the weekday as enum element from current date */
inline static enum vis_enum_weekdays
get_enum_weekday_from_tm(const int tm_wday)
{
	enum vis_enum_weekdays curday = VIS_ENUM_SUNDAY;

	switch (tm_wday) {
		case 0:
			curday = VIS_ENUM_SUNDAY;
			break;
		case 1:
			curday = VIS_ENUM_MONDAY;
			break;
		case 2:
			curday = VIS_ENUM_TUESDAY;
			break;
		case 3:
			curday = VIS_ENUM_WEDNESDAY;
			break;
		case 4:
			curday = VIS_ENUM_THURSDAY;
			break;
		case 5:
			curday = VIS_ENUM_FRIDAY;
			break;
		case 6:
			curday = VIS_ENUM_SATURDAY;
			break;
	}
	return curday;
}

/* Checks whether to start data collection or not */
static int
is_auto_start_datacollection()
{
	time_t currTime;
	struct tm *localTime;
	int curhr, curminute, tm_wday;
	int totcursecs;

	/* If auto data collection is not enabled */
	if (g_configs.isautostart == 0)
		return FALSE;

	/* Get current day and time */
	currTime = time(NULL);
	localTime = localtime(&currTime);
	curhr = localTime->tm_hour;
	curminute = localTime->tm_min;
	tm_wday = localTime->tm_wday;
	totcursecs = ((curhr * 3600) + (curminute * 60));

	VIS_DEBUG("AUto Start : %d\tWeekDays : %d\tFrom Time : %d\tTo Time : %d\n\n",
		g_configs.isautostart, g_configs.weekdays, g_configs.fromtm, g_configs.totm);
	VIS_DEBUG("Local Hour: %d\tMinute : %d\tDay : %d\tTotsecs : %d\n",
		curhr, curminute, tm_wday, totcursecs);

	/* Check if the day is selected for auto data collection */
	if (is_weekday(g_configs.weekdays, get_enum_weekday_from_tm(tm_wday))) {
		if (totcursecs >= g_configs.fromtm && totcursecs <= g_configs.totm) {
			VIS_DEBUG("*** Collect Data ***\n");
			return TRUE;
		}
	}

	return FALSE;
}

int
vis_daemon(int nochdir, int noclose)
{
	pid_t pid, sid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		return -1;
	}
	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		return -1;
	}

	/* Change the current working directory */
	if (!nochdir) {
		if ((chdir("/")) < 0) {
			return -1;
		}
	}

	/* Close out the standard file descriptors */
	if (!noclose) {
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}

	return 0;
}

/* Visualization Linux Main function */
int
main(int argc, char **argv)
{
	if (vis_daemon(1, 1) == -1) {
		perror("daemon");
		exit(errno);
	}

	int ret			= 0;
	int ngetipcount = 0;
	int ischeck_rrm_thread = 0;

	vis_debug_level = 1;
	get_nvram_debug_settings();
	VIS_DEBUG("Visualization Datacollector Started\n");


	initialize_config();

	g_wlinterface = NULL;
	memset(server_ip_address, 0x00, MAX_IP_LEN);
	memset(gateway_ip_address, 0x00, MAX_IP_LEN);

	/* Get all the wl adapter info */
	while (1) {
		get_all_wl_interfaces();
		if (!g_wlinterface) /* If there is no interface query again after 10 seconds */
			sleep(10);
		else /* Any one found break the loop */
			break;
	}
	vis_check_interfaces_are_up();

	/* Get DUT info deosnot reflect proper values in the boot time so getting after delay */
	sleep(5);
	get_dut_info_from_driver();

	get_server_ip_address();
	get_gateway_config_settings();

	/* Loop every 10 seconds till the dut info send is completed. 10 is taken randomly */
	while (1) {
		ret = send_all_dut_info();
		if (ret == -1) {
			sleep(10);
		} else {
			break;
		}
	}
	VIS_DEBUG("DUT info send successfull\n");

	/* Loop every 10 seconds till the login is completed. 10 is taken randomly */
	while (1) {
		ret = send_login_data();
		if (ret == -1) {
			sleep(10);
			/*
			 * As the gateway settings takes time. we are getting the gateway IP
			 * for some 10 times and expecting that at some point of time we get
			 * the IP address
			 */
			if (ngetipcount < 10) {
				get_gateway_config_settings();
				ngetipcount++;
			}
		} else
			break;
	}
	VIS_DEBUG("Login Completed\n");

	rrm_initialize_mutexes();

#ifdef TIMER_BASED
	int cur_count = 0;

	while (1) {
		struct timeval curr_time;
		struct timeval prev_time;
		unsigned long diff;
		vis_wl_interface_t *curnode;

		/*
		 * As the gateway settings takes time. we are getting the gateway IP
		 * for some 10 times and expecting that at some point of time we get
		 * the IP address
		 */
		if (ngetipcount < 10) {
			get_gateway_config_settings();
			ngetipcount++;
		}

		/* If start data collection is not set check for auto data collection */
		if (g_configs.isstart == 0) {
			if (is_auto_start_datacollection() == FALSE) { /* No auto data collection */
				/* Thread is not running so close it */
				if (ischeck_rrm_thread) {
					rrm_thread_close();
					ischeck_rrm_thread = 0;
				}
				sleep(5);
				send_login_data();
				continue;
			}
		}
		gettimeofday(&prev_time, NULL);

		ischeck_rrm_thread = 1; /* Thread will be running */
		get_counter_from_driver();
		curnode = g_wlinterface;
		while (curnode != NULL) {
			if (curnode->enabled) {
				send_counters_data(curnode);
			}
			curnode = curnode->next;
		}
		free_global_data_struct();
		cur_count++;

		gettimeofday(&curr_time, NULL);
		diff = curr_time.tv_sec - prev_time.tv_sec;

		if ((g_configs.interval - diff) > 0)
			sleep(g_configs.interval - diff);
	}
	rrm_destroy_mutexes();
	free_interface_nodes();

	return 0;
#else
	get_counter_from_driver();

	vis_wl_interface_t *curnode = g_wlinterface;
	while (curnode != NULL) {
		if (curnode->enabled) {
			send_counters_data(curnode);
		}
		curnode = curnode->next;
	}
	free_global_data_struct();
	rrm_destroy_mutexes();
	free_interface_nodes();

	return 0;
#endif /* TIMER_BASED */

	return 0;
}
