/*
 * Linux Visualization System common xml utility implementation
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
 * $Id: vis_xmlshared.c 672659 2016-11-29 10:24:01Z $
 */

#include <string.h>
#include <time.h>
#include "vis_xmlshared.h"
#include "vis_sock_util.h"

/* Gets the integer element from the XML node */
int
get_int_element(xmlDocPtr doc, xmlNodePtr node, int *element)
{
	int	ret = -1;
	xmlChar	*str;

	str = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
	if (str != NULL) {
		*element = atoi((const char*)str);
		ret = 0;
		xmlFree(str);
	}

	return ret;
}

/* Gets the long element from the XML node */
int
get_long_element(xmlDocPtr doc, xmlNodePtr node, long *element)
{
	int	ret = -1;
	xmlChar	*str;

	str = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
	if (str != NULL) {
		*element = atol((const char*)str);
		ret = 0;
		xmlFree(str);
	}

	return ret;
}

/* Gets the string element from the XML node */
int
get_string_element(xmlDocPtr doc, xmlNodePtr node, char *element, int nelemlen)
{
	int	ret = -1;
	xmlChar	*str;

	str = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
	if (str != NULL) {
		snprintf(element, nelemlen, "%s", str);
		ret = 0;
		xmlFree(str);
	}

	return ret;
}

/* Gets the integer attribute from the XML node */
int
get_int_attribute(xmlNodePtr node, char *attribname, int *attribute)
{
	int ret = -1;

	xmlChar *str = xmlGetProp(node, (const xmlChar*)attribname);
	if (str) {
		*attribute = atoi((const char*)str);
		ret = 0;
		xmlFree(str);
	}

	return ret;
}

/* Gets the CDATA string from the XML node */
int
get_cdata_element(xmlNodePtr node, char *element, int nelemlen)
{
	int	ret = -1;
	xmlChar	*str;

	str = xmlNodeGetContent(node);
	if (str != NULL) {
		snprintf(element, nelemlen, "%s", str);
		ret = 0;
		xmlFree(str);
	}

	return ret;
}

/* add config node to root node */
void
add_config_node(xmlNodePtr *root_node, configs_t *configs)
{
	xmlNodePtr	node;
	char		tmpstr[10];

	node = xmlNewChild(*root_node, NULL, (const xmlChar*)TAG_CONFIG, NULL);

	snprintf(tmpstr, sizeof(tmpstr), "%d", configs->interval);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_SAMPLEINTERVAL, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", configs->isstart);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_ISSTART, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", configs->isremoteenab);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_ISREMOTEENAB, (const xmlChar*)tmpstr);

	xmlNewChild(node, NULL, (const xmlChar*)TAG_GATEWAYIP,
		(const xmlChar*)configs->gatewayip);

	snprintf(tmpstr, sizeof(tmpstr), "%d", configs->isautostart);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_ISAUTOSTART, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", configs->weekdays);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_WEEKDAYS, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", configs->fromtm);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_FROMTM, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", configs->totm);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TOTM, (const xmlChar*)tmpstr);
}

/* add DUT settings to root node */
void
add_dut_settings_node(xmlNodePtr *root_node, dut_settings_t *dutset)
{
	xmlNodePtr	node;
	char		tmpstr[10];

	node = xmlNewChild(*root_node, NULL, (const xmlChar*)TAG_DUTSETTINGS, NULL);

	if (dutset != NULL) {
		snprintf(tmpstr, sizeof(tmpstr), "%d", dutset->isscan);
		xmlNewChild(node, NULL, (const xmlChar*)TAG_DOSCAN, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", dutset->band);
		xmlNewChild(node, NULL, (const xmlChar*)TAG_BAND, (const xmlChar*)tmpstr);

		xmlNewChild(node, NULL, (const xmlChar*)TAG_MAC, (const xmlChar*)dutset->mac);
	}
}

/* Gets the records to fetch node */
int
get_records_to_fetch_node(xmlDocPtr doc, xmlNodePtr node, packet_header_t *pktheader)
{
	int ret = 0;

	if (node == NULL) /* If there is no node records to fetch */
		return -1;

	while (node != NULL) {
		if (strcmp((const char*)node->name, TAG_NAME) == 0) {
			pktheader->recordstofetch.ispresent = 1;
			get_string_element(doc, node, pktheader->recordstofetch.name,
				MAX_GRAPH_NAME);
		} else if (strcmp((const char*)node->name, TAG_NUMBER) == 0) {
			get_int_element(doc, node, (int*)&pktheader->recordstofetch.count);
		} else if (strcmp((const char*)node->name, TAG_RECORDS) == 0) {
			get_int_element(doc, node, (int*)&pktheader->recordstofetch.records);
		} else if (strcmp((const char*)node->name, TAG_FTIMESTAMP) == 0) {
			get_int_element(doc, node, (int*)&pktheader->recordstofetch.ftimestamp);
		} else if (strcmp((const char*)node->name, TAG_LTIMESTAMP) == 0) {
			get_int_element(doc, node, (int*)&pktheader->recordstofetch.ltimestamp);
		}

		node = node->next;
	}

	return ret;
}

/* Get the packet header from XML data and stores in structure */
int
get_packet_header(xmlDocPtr doc, xmlNodePtr node, packet_header_t *pktheader)
{
	int		ret = 0;
	xmlNodePtr	nodechild;

	if (node == NULL) /* If there is no node inside packet version */
		return -1;

	while (node != NULL) {
		if (strcmp((const char*)node->name, TAG_PACKETHEADER) == 0) {
			nodechild = node->children;
			while (nodechild != NULL) {
				if (strcmp((const char*)nodechild->name, TAG_PACKETTYPE) == 0) {
					get_int_element(doc, nodechild, (int*)&pktheader->type);
				} else if (strcmp((const char*)nodechild->name,
					TAG_PACKETFROM) == 0) {
					get_int_element(doc, nodechild, (int*)&pktheader->from);
				} else if (strcmp((const char*)nodechild->name,
					TAG_REQUESTRESPTYPE) == 0) {
					get_string_element(doc, nodechild, pktheader->reqresptype,
						MAX_REQUEST_NAME);
				} else if (strcmp((const char*)nodechild->name, TAG_RESULT) == 0) {
					get_int_element(doc, nodechild, (int*)&pktheader->result);
				} else if (strcmp((const char*)nodechild->name, TAG_REASON) == 0) {
					get_string_element(doc, nodechild, pktheader->reason,
						MAX_REASON_LEN);
				} else if (strcmp((const char*)nodechild->name,
					TAG_FREQBAND) == 0) {
					get_int_element(doc, nodechild, (int*)&pktheader->band);
				} else if (strcmp((const char*)nodechild->name,
					TAG_RECORDSTOFETCH) == 0) {
					get_records_to_fetch_node(doc,
						nodechild->children, pktheader);
				}
				nodechild = nodechild->next;
			}
			break;
		}
		node = node->next;
	}
	VIS_XML("Packet Type : %d\nPacket From : %d\nRequest Response Type : %s\n"
		"Result : %d\nReason : %s\nFrequencyBand : %d\n", pktheader->type, pktheader->from,
		pktheader->reqresptype, pktheader->result, pktheader->reason,
		pktheader->band);

	return ret;
}

/* Get the config TAG from XML data and stores in structure */
int
get_config_from_packet(xmlDocPtr doc, xmlNodePtr node, configs_t *conf)
{
	int ret = 0;

	if (node == NULL) /* If there is no node inside packet version */
		return -1;

	while (node != NULL) { /* search for tag config */
		if (strcmp((const char*)node->name, TAG_CONFIG) == 0) {
			break;
		}
		node = node->next;
	}

	if (node == NULL) /* no tag named Config */
		return -1;

	node = node->children;
	while (node != NULL) {
		if (strcmp((const char*)node->name, TAG_SAMPLEINTERVAL) == 0) {
			get_int_element(doc, node, (int*)&conf->interval);
		} else if (strcmp((const char*)node->name, TAG_DBSIZE) == 0) {
			get_int_element(doc, node, (int*)&conf->dbsize);
		} else if (strcmp((const char*)node->name, TAG_ISSTART) == 0) {
			get_int_element(doc, node, (int*)&conf->isstart);
		} else if (strcmp((const char*)node->name, TAG_ISREMOTEENAB) == 0) {
			get_int_element(doc, node, (int*)&conf->isremoteenab);
		} else if (strcmp((const char*)node->name, TAG_GATEWAYIP) == 0) {
			get_string_element(doc, node, conf->gatewayip, MAX_IP_LEN);
		} else if (strcmp((const char*)node->name, TAG_ISOVERWRITEDB) == 0) {
			get_int_element(doc, node, (int*)&conf->isoverwrtdb);
		} else if (strcmp((const char*)node->name, TAG_ISAUTOSTART) == 0) {
			get_int_element(doc, node, (int*)&conf->isautostart);
		} else if (strcmp((const char*)node->name, TAG_WEEKDAYS) == 0) {
			get_int_element(doc, node, (int*)&conf->weekdays);
		} else if (strcmp((const char*)node->name, TAG_FROMTM) == 0) {
			get_int_element(doc, node, (int*)&conf->fromtm);
		} else if (strcmp((const char*)node->name, TAG_TOTM) == 0) {
			get_int_element(doc, node, (int*)&conf->totm);
		}
		node = node->next;
	}

	VIS_XML("Sample Interval : %d\nDB SIZE : %d\n Is Start : %d\n Is Overwrite : %d\n"
		"Auto Start : %d\nWeekDays : %d\nFrom Time : %d\nTo Time : %d\n",
		conf->interval, conf->dbsize, conf->isstart, conf->isoverwrtdb,
		conf->isautostart, conf->weekdays, conf->fromtm, conf->totm);

	return ret;
}

/* Adds packet version to root node */
void
add_packet_version(xmlNodePtr *root_node, int version)
{
	char tmpstr[2];

	*root_node = xmlNewNode(NULL, (const xmlChar*)TAG_PACKETVERSION);
	snprintf(tmpstr, sizeof(tmpstr), "%d", version);
	xmlNewProp(*root_node, (const xmlChar*)ATTRIB_NAME, (const xmlChar*)tmpstr);
}

/* Adds packet header TAG node */
void
add_packet_header_node(xmlNodePtr *root_node, packet_header_t *pktheader)
{
	char		tmpstr[10];
	xmlNodePtr	node;

	node = xmlNewChild(*root_node, NULL, (const xmlChar*)TAG_PACKETHEADER, NULL);

	/* Add Packet Type */
	snprintf(tmpstr, sizeof(tmpstr), "%d", pktheader->type);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_PACKETTYPE, (const xmlChar*)tmpstr);

	/* Add From */
	snprintf(tmpstr, sizeof(tmpstr), "%d", pktheader->from);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_PACKETFROM, (const xmlChar*)tmpstr);

	/* Add Reuqest Response Type */
	xmlNewChild(node, NULL, (const xmlChar*)TAG_REQUESTRESPTYPE,
		(const xmlChar*)pktheader->reqresptype);

	/* Add Result */
	snprintf(tmpstr, sizeof(tmpstr), "%d", pktheader->result);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_RESULT, (const xmlChar*)tmpstr);

	if (strlen(pktheader->reason) > 0) {
		/* Add Reason */
		xmlNewChild(node, NULL, (const xmlChar*)TAG_REASON,
			(const xmlChar*)pktheader->reason);
	}
}

/* Adds DUTINFO TAG node */
void
add_dut_info_node(xmlDocPtr *doc, xmlNodePtr *root_node, dut_info_t *dut_info)
{
	xmlNodePtr	node, tmpchildnode, tmpcdataptr;
	char		tmpstr[10];

	node = xmlNewChild(*root_node, NULL, (const xmlChar*)TAG_DUT, NULL);

	snprintf(tmpstr, sizeof(tmpstr), "%d", dut_info->isenabled);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_ISENABLED, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", dut_info->isAP);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_ISAP, (const xmlChar*)tmpstr);

	xmlNewChild(node, NULL, (const xmlChar*)TAG_BSSID,
		(const xmlChar*)dut_info->bssid);

	/* Add string inside CDATA tag as it may contain special characters */
	tmpchildnode = xmlNewChild(node, NULL, (const xmlChar*)TAG_SSID, NULL);
	tmpcdataptr = xmlNewCDataBlock(*doc, (const xmlChar*)dut_info->ssid,
		strlen(dut_info->ssid));
	xmlAddChild(tmpchildnode, tmpcdataptr);

	xmlNewChild(node, NULL, (const xmlChar*)TAG_MAC,
		(const xmlChar*)dut_info->mac);

	snprintf(tmpstr, sizeof(tmpstr), "%d", dut_info->channel);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_CHANNEL, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", dut_info->bandwidth);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_BANDWIDTH, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", dut_info->rssi);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_RSSI, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", dut_info->noise);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_NOISE, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", dut_info->ctrlch);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_CONTROLCH, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", dut_info->band);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_BAND, (const xmlChar*)tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "%d", dut_info->maxrate);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_RATE, (const xmlChar*)tmpstr);

	xmlNewChild(node, NULL, (const xmlChar*)TAG_TYPE,
		(const xmlChar*)dut_info->networktype);

	if (strlen(dut_info->ucastrsntype) > 0) {
		xmlNewChild(node, NULL, (const xmlChar*)TAG_MCAST_RSN,
			(const xmlChar*)dut_info->mcastrsntype);
		xmlNewChild(node, NULL, (const xmlChar*)TAG_UCAST_RSN,
			(const xmlChar*)dut_info->ucastrsntype);
		xmlNewChild(node, NULL, (const xmlChar*)TAG_AKM,
			(const xmlChar*)dut_info->akmrsntype);
	}

	snprintf(tmpstr, sizeof(tmpstr), "%d", dut_info->errinfo);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_ERR_INFO, (const xmlChar*)tmpstr);
}

/* Adds Networks TAG node */
void
add_networks_node(xmlDocPtr *doc, xmlNodePtr *root_node, networks_list_t *networks_list)
{
	xmlNodePtr	node, node1, tmpchildnode, tmpcdataptr;
	char		timestamp[15];
	char		tmpstr[10];
	int		i;

	if (networks_list == NULL)
		return;
	/* Add the Networks element */
	node = xmlNewChild(*root_node, NULL, (const xmlChar*)TAG_NETWORKS, NULL);

	snprintf(tmpstr, sizeof(tmpstr), "%d", networks_list->length);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TOTAL, (const xmlChar*)tmpstr);

	snprintf(timestamp, sizeof(timestamp), "%ld", networks_list->timestamp);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TIMESTAMP, (const xmlChar*)timestamp);

	/* Now for every AP found add AP Tag and its info */
	for (i = 0; i < networks_list->length; i++) {
		/* AP Tag */
		node1 = xmlNewChild(node, NULL, (const xmlChar*)TAG_AP, NULL);

		/* Add string inside CDATA tag as it may contain special characters */
		tmpchildnode = xmlNewChild(node1, NULL, (const xmlChar*)TAG_SSID, NULL);
		tmpcdataptr = xmlNewCDataBlock(*doc, (const xmlChar*)networks_list->aps[i].ssid,
			strlen(networks_list->aps[i].ssid));
		xmlAddChild(tmpchildnode, tmpcdataptr);

		xmlNewChild(node1, NULL, (const xmlChar*)TAG_BSSID,
			(const xmlChar*)networks_list->aps[i].bssid);

		snprintf(tmpstr, sizeof(tmpstr), "%d", networks_list->aps[i].rssi);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_RSSI, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", networks_list->aps[i].noise);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_NOISE, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", networks_list->aps[i].channel);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_CHANNEL, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", networks_list->aps[i].ctrlch);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_CONTROLCH, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", networks_list->aps[i].band);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_BAND, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", networks_list->aps[i].bandwidth);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_BANDWIDTH, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", networks_list->aps[i].maxrate);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_RATE, (const xmlChar*)tmpstr);

		xmlNewChild(node1, NULL, (const xmlChar*)TAG_TYPE,
			(const xmlChar*)networks_list->aps[i].networktype);

		if (strlen(networks_list->aps[i].ucastrsntype) > 0) {
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_MCAST_RSN,
				(const xmlChar*)networks_list->aps[i].mcastrsntype);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_UCAST_RSN,
				(const xmlChar*)networks_list->aps[i].ucastrsntype);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_AKM,
				(const xmlChar*)networks_list->aps[i].akmrsntype);
		}
	}
}

/* Adds Channel STATS TAG */
void
add_congestion_node(xmlNodePtr *root_node, congestion_list_t *congestion)
{
	xmlNodePtr	node, node1;
	char		tmpstr[10];
	char		tmptime[15];
	int		i;

	if (congestion == NULL)
		return;
	node = xmlNewChild(*root_node, NULL, (const xmlChar*)TAG_CHANNELSTATS, NULL);

	snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->length);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TOTAL, (const xmlChar*)tmpstr);

	snprintf(tmptime, sizeof(tmptime), "%ld", congestion->timestamp);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TIMESTAMP, (const xmlChar*)tmptime);

	/* Now for every Channels add Stats Tag and its info */
	for (i = 0; i < congestion->length; i++) {
		/* STATS Tag */
		node1 = xmlNewChild(node, NULL, (const xmlChar*)TAG_STATS, NULL);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].channel);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_CHANNEL, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].tx);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_TX, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].inbss);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_INBSS, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].obss);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_OBSS, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].nocat);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_NOCAT, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].nopkt);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_NOPKT, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].doze);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_DOZE, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].txop);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXOP, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].goodtx);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_GOODTX, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].badtx);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_BADTX, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].glitchcnt);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_GLITCH, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].badplcp);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_BADPLCP, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].knoise);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_KNOISE, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", congestion->congest[i].chan_idle);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_IDLE, (const xmlChar*)tmpstr);
	}
}

/* Adds one graph node */
void
add_one_xtime_node_graph(xmlNodePtr *root_node, char *name,
	int timestamp, int value)
{
	iovar_handler_t	*handle;
	xmlNodePtr	node, node1;
	char		tmpstr[10];
	char		tmptime[15];

	handle = find_iovar(name);

	add_graph_header_details(root_node, &node, handle);

	/* Add Total */
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TOTAL, (const xmlChar*)"1");

	node1 = xmlNewChild(node, NULL, (const xmlChar*)TAG_VALUE, NULL);

	/* Add timestamp */
	snprintf(tmptime, sizeof(tmptime), "%d", timestamp);
	xmlNewChild(node1, NULL, (const xmlChar*)TAG_TIMESTAMP, (const xmlChar*)tmptime);

	xmlNewChild(node1, NULL, (const xmlChar*)TAG_X_VALUE, (const xmlChar*)tmptime);

	snprintf(tmpstr, sizeof(tmpstr), "%u", value);
	xmlNewChild(node1, NULL, (const xmlChar*)TAG_Y_VALUE, (const xmlChar*)tmpstr);
}

/* Adds one packet queue graph node */
void
add_one_packet_queue_graph(xmlNodePtr *root_node, char *name,
	char *mac, int timestamp, char *value)
{
	iovar_handler_t	*handle;
	xmlNodePtr	node, node1;
	char		tmptime[15];

	handle = find_iovar(name);

	add_graph_header_details(root_node, &node, handle);

	/* Add Total */
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TOTAL, (const xmlChar*)"1");

	xmlNewChild(node, NULL, (const xmlChar*)TAG_MAC, (const xmlChar*)mac);

	node1 = xmlNewChild(node, NULL, (const xmlChar*)TAG_VALUE, NULL);

	/* Add timestamp */
	snprintf(tmptime, sizeof(tmptime), "%d", timestamp);
	xmlNewChild(node1, NULL, (const xmlChar*)TAG_TIMESTAMP, (const xmlChar*)tmptime);

	xmlNewChild(node1, NULL, (const xmlChar*)TAG_X_VALUE, (const xmlChar*)tmptime);

	xmlNewChild(node1, NULL, (const xmlChar*)TAG_Y_VALUE, (const xmlChar*)value);
}

/* Adds packet queue TAG  graph nodes */
void
add_packet_queue_graph_nodes(xmlNodePtr *root_node, assoc_sta_list_t *stas_list)
{
	int		i;

	if (stas_list == NULL)
		return;

	/* For every Associated STA's add graph tag */
	for (i = 0; i < stas_list->length; i++) {
		char tmpstr[10];

		snprintf(tmpstr, sizeof(tmpstr), "%u", stas_list->sta[i].prequested);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_PACKETREQUESTED,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%u", stas_list->sta[i].pstored);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_PACKETSTORED,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%u", stas_list->sta[i].pdropped);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_PACKETDROPPED,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%u", stas_list->sta[i].pretried);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_PACKETRETRIED,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%u", stas_list->sta[i].putilization);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_QUEUEUTILIZATION,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%u", stas_list->sta[i].pqlength);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_QUEUELENGTH,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%8.2f", stas_list->sta[i].ptput);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_DATATHROUGHPUT,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%8.2f", stas_list->sta[i].pphyrate);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_PHYSICALRATE,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%u", stas_list->sta[i].prtsfail);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_RTSFAIL,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%u", stas_list->sta[i].prtrydrop);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_RTRYDROP,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%u", stas_list->sta[i].ppsretry);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_PSRETRY,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%u", stas_list->sta[i].packed);
		add_one_packet_queue_graph(root_node, IOVAR_NAME_ACKED,
			stas_list->sta[i].mac, stas_list->timestamp, tmpstr);
	}
}

/* Adds Associated STA TAG node */
void
add_associated_sta_node(xmlNodePtr *root_node, assoc_sta_list_t *stas_list)
{
	xmlNodePtr	node, node1;
	char		tmpstr[10];
	char		timestamp[15];
	int		i;

	if (stas_list == NULL)
		return;
	node = xmlNewChild(*root_node, NULL, (const xmlChar*)TAG_ASSOCIATEDSTA, NULL);

	snprintf(tmpstr, sizeof(tmpstr), "%d", stas_list->length);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TOTAL, (const xmlChar*)tmpstr);

	snprintf(timestamp, sizeof(timestamp), "%ld", stas_list->timestamp);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TIMESTAMP, (const xmlChar*)timestamp);

	/* Now for every Associated STA's add STA Tag and its info */
	for (i = 0; i < stas_list->length; i++) {
		/* STA Tag */
		node1 = xmlNewChild(node, NULL, (const xmlChar*)TAG_STA, NULL);

		xmlNewChild(node1, NULL, (const xmlChar*)TAG_MAC,
			(const xmlChar*)stas_list->sta[i].mac);

		snprintf(tmpstr, sizeof(tmpstr), "%d", stas_list->sta[i].rssi);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_RSSI, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", stas_list->sta[i].phyrate);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_PHYRATE, (const xmlChar*)tmpstr);
	}

	add_packet_queue_graph_nodes(root_node, stas_list);
}

/* Adds counters data to Graph TAG */
void
add_counters_node(xmlNodePtr *root_node, counters_t *counters)
{
	/* ***** RXCRS Glitch data Starts ***** */
	add_one_xtime_node_graph(root_node, IOVAR_NAME_RXCRSGLITCH,
		counters->timestamp, counters->rxcrsglitch);
	/* ***** RXCRS Glitch data ends ***** */

	/* ***** Bad PLCP Glitch data Starts ***** */
	add_one_xtime_node_graph(root_node, IOVAR_NAME_BADPLCP,
		counters->timestamp, counters->rxbadplcp);
	/* ***** Bad PLCP Glitch data ends ***** */

	/* ***** Bad FCS Glitch data Starts ***** */
	add_one_xtime_node_graph(root_node, IOVAR_NAME_BADFCS,
		counters->timestamp, counters->rxbadfcs);
	/* ***** Bad FCS Glitch data ends ***** */
}

/* Checks whether all ampdu values are zero */
int is_all_ampdu_values_zero(tx_ampdu_list_t *ampdulist, int idx)
{
	if (ampdulist->ampdus[idx].txmcs != 0)
		return 0;
	else if (ampdulist->ampdus[idx].txmcspercent != 0)
		return 0;
	else if (ampdulist->ampdus[idx].txmcssgi != 0)
		return 0;
	else if (ampdulist->ampdus[idx].txmcssgipercent != 0)
		return 0;
	else if (ampdulist->ampdus[idx].rxmcs != 0)
		return 0;
	else if (ampdulist->ampdus[idx].rxmcspercent != 0)
		return 0;
	else if (ampdulist->ampdus[idx].rxmcssgi != 0)
		return 0;
	else if (ampdulist->ampdus[idx].rxmcssgipercent != 0)
		return 0;
	else if (ampdulist->ampdus[idx].txvht != 0)
		return 0;
	else if (ampdulist->ampdus[idx].txvhtper != 0)
		return 0;
	else if (ampdulist->ampdus[idx].txvhtpercent != 0)
		return 0;
	else if (ampdulist->ampdus[idx].txvhtsgi != 0)
		return 0;
	else if (ampdulist->ampdus[idx].txvhtsgiper != 0)
		return 0;
	else if (ampdulist->ampdus[idx].txvhtsgipercent != 0)
		return 0;
	else if (ampdulist->ampdus[idx].rxvht != 0)
		return 0;
	else if (ampdulist->ampdus[idx].rxvhtper != 0)
		return 0;
	else if (ampdulist->ampdus[idx].rxvhtpercent != 0)
		return 0;
	else if (ampdulist->ampdus[idx].rxvhtsgi != 0)
		return 0;
	else if (ampdulist->ampdus[idx].rxvhtsgiper != 0)
		return 0;
	else if (ampdulist->ampdus[idx].rxvhtsgipercent != 0)
		return 0;
	else if (ampdulist->ampdus[idx].mpdudens != 0)
		return 0;
	else if (ampdulist->ampdus[idx].mpdudenspercent != 0)
		return 0;

	return 1;
}

/* Adds AMPDU stats to Graph TAG */
void
add_ampdu_stats_node(xmlNodePtr *root_node, tx_ampdu_list_t *ampdulist)
{
	xmlNodePtr	node, node1;
	char		tmpstr[10];
	char		timestamp[15];
	int		i, j = 0;

	node = xmlNewChild(*root_node, NULL, (const xmlChar*)TAG_AMPDUSTATS, NULL);
	/* Add timestamp */
	snprintf(timestamp, sizeof(timestamp), "%ld", ampdulist->timestamp);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TIMESTAMP, (const xmlChar*)timestamp);
	if (ampdulist == NULL)
		return;
	for (i = 0; i < ampdulist->length; i++) {
		if (is_all_ampdu_values_zero(ampdulist, i) == 1) {
			ampdulist->length--;
			continue;
		}
		node1 = xmlNewChild(node, NULL, (const xmlChar*)TAG_AMPDU, NULL);

		snprintf(tmpstr, sizeof(tmpstr), "%d%d", ampdulist->ampdus[i].streams,
			ampdulist->ampdus[i].mcs);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_MCS, (const xmlChar*)tmpstr);

		if (ampdulist->ampdus[i].txmcs > 0) {
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].txmcs);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXMCS, (const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].txmcspercent);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXMCSPERCENT,
				(const xmlChar*)tmpstr);
		}

		if (ampdulist->ampdus[i].txmcssgi > 0) {
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].txmcssgi);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXMCSSGI,
				(const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d",
				ampdulist->ampdus[i].txmcssgipercent);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXMCSSGIPERCENT,
				(const xmlChar*)tmpstr);
		}

		if (ampdulist->ampdus[i].rxmcs > 0) {
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].rxmcs);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_RXMCS, (const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].rxmcspercent);
			xmlNewChild(node1, NULL, (const xmlChar*) TAG_RXMCSPERCENT,
				(const xmlChar*)tmpstr);
		}

		if (ampdulist->ampdus[i].rxmcssgi > 0) {
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].rxmcssgi);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_RXMCSSGI,
				(const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d",
				ampdulist->ampdus[i].rxmcssgipercent);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_RXMCSSGIPERCENT,
				(const xmlChar*)tmpstr);
		}

		if (ampdulist->ampdus[i].txvht > 0) {
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].txvht);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXVHT, (const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].txvhtper);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXVHTPER,
				(const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].txvhtpercent);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXVHTPERCENT,
				(const xmlChar*)tmpstr);
		}

		if (ampdulist->ampdus[i].txvhtsgi > 0) {
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].txvhtsgi);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXVHTSGI,
				(const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].txvhtsgiper);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXVHTSGIPER,
				(const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d",
				ampdulist->ampdus[i].txvhtsgipercent);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXVHTSGIPERCENT,
				(const xmlChar*)tmpstr);
		}

		if (ampdulist->ampdus[i].rxvht > 0) {
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].rxvht);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_RXVHT, (const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].rxvhtper);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_RXVHTPER,
				(const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].rxvhtpercent);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_RXVHTPERCENT,
				(const xmlChar*)tmpstr);
		}

		if (ampdulist->ampdus[i].rxvhtsgi > 0) {
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].rxvhtsgi);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_RXVHTSGI,
				(const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].rxvhtsgiper);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_RXVHTSGIPER,
				(const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d",
				ampdulist->ampdus[i].rxvhtsgipercent);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_RXVHTSGIPERCENT,
				(const xmlChar*)tmpstr);
		}

		if (ampdulist->ampdus[i].mpdudens > 0) {
			snprintf(tmpstr, sizeof(tmpstr), "%d", ampdulist->ampdus[i].mpdudens);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_MPDUDENS,
				(const xmlChar*)tmpstr);
			snprintf(tmpstr, sizeof(tmpstr), "%d",
				ampdulist->ampdus[i].mpdudenspercent);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_MPDUDENSPERCENT,
				(const xmlChar*)tmpstr);
		}
		j++;
	}
	/* Add Length */
	snprintf(tmpstr, sizeof(tmpstr), "%d", j);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TOTAL, (const xmlChar*)tmpstr);
}

/* Add the RRM STA side statistics TAG node */
void
add_rrm_sta_stats_node(xmlNodePtr *root_node, vis_rrm_statslist_t *rrmstatslist)
{
	xmlNodePtr	node, node1;
	char		tmpstr[10];
	char		timestamp[15];
	int		i, total = 0;

	if (rrmstatslist == NULL)
		return;
	for (i = 0; i < rrmstatslist->length; i++) {
		if ((rrmstatslist->rrmstats[i].read_flag & RRM_STA_READ) == RRM_STA_READ) {
			total++;
		}
	}
	node = xmlNewChild(*root_node, NULL, (const xmlChar*)TAG_RRMSTASTATS, NULL);

	snprintf(tmpstr, sizeof(tmpstr), "%d", total);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TOTAL, (const xmlChar*)tmpstr);

	snprintf(timestamp, sizeof(timestamp), "%ld", rrmstatslist->timestamp);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TIMESTAMP, (const xmlChar*)timestamp);

	/* Now for every Associated STA add RRM stats Tag and its info */
	for (i = 0; i < rrmstatslist->length; i++) {
		if ((rrmstatslist->rrmstats[i].read_flag & RRM_STA_READ) == RRM_STA_READ) {
			/* RRM STats */
			node1 = xmlNewChild(node, NULL, (const xmlChar*)TAG_RRMSTA, NULL);

			xmlNewChild(node1, NULL, (const xmlChar*)TAG_MAC,
				(const xmlChar*)rrmstatslist->rrmstats[i].mac);
			/*
			 * Since channel load rpt takes a little bit longer time
			 * so most of the times it might not be valid value
			 * So check for channel load rpt flag if set use the txop val
			 * otherwise -1 as invalid.
			 */
			rrmstatslist->rrmstats[i].txop =
				rrmstatslist->rrmstats[i].read_flag & RRM_BIT_CHLOAD ?
				rrmstatslist->rrmstats[i].txop : -1;

			snprintf(tmpstr, sizeof(tmpstr), "%d",
				rrmstatslist->rrmstats[i].txop);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXOP,
				(const xmlChar*)tmpstr);

			snprintf(tmpstr, sizeof(tmpstr), "%d",
				rrmstatslist->rrmstats[i].pktrequested);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_PKTREQUESTED,
				(const xmlChar*)tmpstr);

			snprintf(tmpstr, sizeof(tmpstr), "%d",
				rrmstatslist->rrmstats[i].pktdropped);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_PKTDROPPED,
				(const xmlChar*)tmpstr);

			snprintf(tmpstr, sizeof(tmpstr), "%d",
				rrmstatslist->rrmstats[i].pktstored);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_PKTSTORED,
				(const xmlChar*)tmpstr);

			snprintf(tmpstr, sizeof(tmpstr), "%d",
				rrmstatslist->rrmstats[i].pktretried);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_PKTRETRIED,
				(const xmlChar*)tmpstr);

			snprintf(tmpstr, sizeof(tmpstr), "%d",
				rrmstatslist->rrmstats[i].pktacked);
			xmlNewChild(node1, NULL, (const xmlChar*)TAG_PKTACKED,
				(const xmlChar*)tmpstr);
		}
	}
}

/* Add the STA side statistics TAG node */
void
add_sta_stats_node(xmlNodePtr *root_node, vis_sta_statslist_t *stastatslist)
{
	xmlNodePtr	node, node1;
	char		tmpstr[10];
	char		timestamp[15];
	int		i;

	if (stastatslist == NULL)
		return;
	node = xmlNewChild(*root_node, NULL, (const xmlChar*)TAG_STAINFOSTATS, NULL);

	snprintf(tmpstr, sizeof(tmpstr), "%d", stastatslist->length);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TOTAL, (const xmlChar*)tmpstr);

	snprintf(timestamp, sizeof(timestamp), "%ld", stastatslist->timestamp);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TIMESTAMP, (const xmlChar*)timestamp);

	/* Now for every Associated STA add STA stats Tag and its info */
	for (i = 0; i < stastatslist->length; i++) {
		/* STA STats */
		node1 = xmlNewChild(node, NULL, (const xmlChar*)TAG_STASTATS, NULL);

		xmlNewChild(node1, NULL, (const xmlChar*)TAG_MAC,
			(const xmlChar*)stastatslist->stastats[i].mac);

		snprintf(tmpstr, sizeof(tmpstr), "%d", stastatslist->stastats[i].generation_flag);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_GENERATION, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", stastatslist->stastats[i].txstream);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_TXSTREAM, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", stastatslist->stastats[i].rxstream);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_RXSTREAM, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", stastatslist->stastats[i].rate);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_RATE, (const xmlChar*)tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%d", stastatslist->stastats[i].idle);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_IDLE, (const xmlChar*)tmpstr);
	}
}

/* Sends XML data to server */
int
send_xml_data(int sockfd, xmlDocPtr *doc)
{
	int	ret = -1;
	char	*data = NULL;
	int	sz = 0;

	xmlDocDumpMemory(*doc, (xmlChar**)&data, &sz);

	if (data != NULL) {
		VIS_XML("XML Data Size is : %d\n\n Data : %s\n", sz, data);
		ret = add_length_and_send_data(sockfd, data, sz);

		free_xmldata(&data);
		ret = 0;
	}

	xmlFreeDoc(*doc);

	return ret;
}

/* Adds GRAPH Header details */
int
add_graph_header_details(xmlNodePtr *root_node, xmlNodePtr *node, iovar_handler_t *handle)
{
	char tmpstr[10];
	char tmptime[15];

	*node = xmlNewChild(*root_node, NULL, (const xmlChar*)TAG_GRAPH, NULL);

	/* Add Graph Type */
	snprintf(tmpstr, sizeof(tmpstr), "%d", handle->graphtype);
	xmlNewChild(*node, NULL, (const xmlChar*)TAG_GRAPH_TYPE, (const xmlChar*)tmpstr);

	/* Add Name */
	xmlNewChild(*node, NULL, (const xmlChar*)TAG_NAME, (const xmlChar*)handle->name);

	/* Add Heading */
	xmlNewChild(*node, NULL, (const xmlChar*)TAG_HEADING, (const xmlChar*)handle->heading);

	/* Add PlotWith */
	xmlNewChild(*node, NULL, (const xmlChar*)TAG_PLOTWITH, (const xmlChar*)handle->plotwith);

	/* Add perSTA */
	snprintf(tmpstr, sizeof(tmpstr), "%d", handle->perSTA);
	xmlNewChild(*node, NULL, (const xmlChar*)TAG_PERSTA, (const xmlChar*)tmpstr);

	/* Add Bar Heading */
	xmlNewChild(*node, NULL, (const xmlChar*)TAG_BAR_HEADING,
		(const xmlChar*)handle->barheading);

	/* Add x-axis name */
	xmlNewChild(*node, NULL, (const xmlChar*)TAG_X_AXIS, (const xmlChar*)handle->xaxisname);

	/* Add y-axis name */
	xmlNewChild(*node, NULL, (const xmlChar*)TAG_Y_AXIS, (const xmlChar*)handle->yaxisname);

	/* Add TimeStamp */
	snprintf(tmptime, sizeof(tmptime), "%ld", (long)time(NULL));
	xmlNewChild(*node, NULL, (const xmlChar*)TAG_TIMESTAMP, (const xmlChar*)tmptime);

	return 0;
}

/* Adds bar graph details to Graph TAG */
int
add_bargraph_details(xmlNodePtr *root_node, graph_list_t *graphdata, char *iovar_name)
{
	iovar_handler_t	*handle;
	xmlNodePtr	node, node1;
	char		tmpstr[10];
	char		tmptime[15];
	int		i = 0;

	handle = find_iovar(iovar_name);
	if (handle == NULL)
		return -1;

	add_graph_header_details(root_node, &node, handle);

	/* Add Total */
	snprintf(tmpstr, sizeof(tmpstr), "%d", graphdata->length);
	xmlNewChild(node, NULL, (const xmlChar*)TAG_TOTAL, (const xmlChar*)tmpstr);

	/* Add mac address if the perSTA is 1 */
	if (handle->perSTA == 1) {
		xmlNewChild(node, NULL, (const xmlChar*)TAG_MAC, (const xmlChar*)graphdata->mac);
	}

	for (i = 0; i < graphdata->length; i++) {
		node1 = xmlNewChild(node, NULL, (const xmlChar*)TAG_VALUE, NULL);

		/* Add timestamp */
		snprintf(tmptime, sizeof(tmptime), "%ld", graphdata->graph[i].timestamp);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_TIMESTAMP, (const xmlChar*)tmptime);

		/* Add X-axis value */
		snprintf(tmpstr, sizeof(tmpstr), "%d", graphdata->graph[i].xvalue);
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_X_VALUE, (const xmlChar*)tmpstr);

		/* Add Y-axis value */
		xmlNewChild(node1, NULL, (const xmlChar*)TAG_Y_VALUE,
			(const xmlChar*)graphdata->graph[i].yvalue);
	}

	return 0;
}

/* Get the DUT Settings TAG from XML data and stores in structure */
int
get_dut_settings_tag(xmlDocPtr doc, xmlNodePtr node, dut_settings_t *dutset)
{
	int ret = 0;

	if (node == NULL) /* If there is no node inside packet version */
		return -1;

	while (node != NULL) { /* search for tag TAG_DUTSETTINGS */
		if (strcmp((const char*)node->name, TAG_DUTSETTINGS) == 0) {
			break;
		}
		node = node->next;
	}

	if (node == NULL) /* no tag named TAG_DUTSETTINGS */
		return -1;

	node = node->children;
	while (node != NULL) {
		if (strcmp((const char*)node->name, TAG_DOSCAN) == 0) {
			get_int_element(doc, node, (int*)&dutset->isscan);
		} else if (strcmp((const char*)node->name, TAG_BAND) == 0) {
			get_int_element(doc, node, (int*)&dutset->band);
		} else if (strcmp((const char*)node->name, TAG_MAC) == 0) {
			get_string_element(doc, node, dutset->mac, (ETHER_ADDR_LEN * 3));
		}
		node = node->next;
	}

	VIS_XML("IS Scan : %d\nBand : %d\nMac : %s\n", dutset->isscan,
		dutset->band, dutset->mac);

	return ret;
}

/* Frees the XML data */
void
free_xmldata(char **data)
{
	if (*data)
		xmlFree(*data);
	*data = NULL;
}
