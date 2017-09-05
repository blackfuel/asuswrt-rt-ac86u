/*
 * Linux Visualization Data Collector xml utility implementation
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
 * $Id: vis_xmlutility.c 606349 2015-12-15 07:15:11Z $
 */

#include <string.h>
#include "vis_xmlutility.h"
#include "vis_xmlshared.h"

extern vis_wl_interface_t *g_wlinterface;
extern configs_t		g_configs;

/* Parse XML response from server */
int
parse_response(unsigned char *data, unsigned int size)
{
	xmlDocPtr	doc;
	xmlNodePtr	node;
	int		ret = 0;
	int		version;
	packet_header_t	pktheader;
	dut_settings_t tmpdutset;
	vis_wl_interface_t *curnode = g_wlinterface;

	memset(&pktheader, 0x00, sizeof(pktheader));
	doc = xmlParseMemory((const char*)data, size);
	if (doc == NULL)
		return -1;

	node = doc->children;
	if (node == NULL) { /* if there is no packet version */
		xmlFreeDoc(doc);
		return -1;
	}

	if (strcmp((const char*)node->name, TAG_PACKETVERSION) == 0) {
		get_int_attribute(node, ATTRIB_NAME, &version);
	}

	get_packet_header(doc, node->children, &pktheader);

	if (pktheader.result == PACKET_SENT_SUCCESS) {
		/* fprintf(stdout, "Data sent successfully to server\n"); */;
	} else {
		VIS_XML("Data NOT sent successfully to server. Reason : %s\n",
			pktheader.reason);
	}

	get_config_from_packet(doc, node->children, &g_configs);

	/* get the current DUT settings */
	memset(&tmpdutset, 0x00, sizeof(tmpdutset));
	get_dut_settings_tag(doc, node->children, &tmpdutset);
	while (curnode != NULL) {
		if ((strcmp(tmpdutset.mac, curnode->dutset.mac) == 0)) {
			curnode->dutset.isscan = tmpdutset.isscan;
			break;
		}
		curnode = curnode->next;
	}

	xmlFreeDoc(doc);

	return ret;
}

/* Creates the XML login request */
int
create_login_request(char **data, int *sz)
{
	xmlDocPtr	doc = NULL;
	xmlNodePtr	root_node = NULL;
	packet_header_t	pktheader;
	vis_wl_interface_t *curnode = g_wlinterface;

	memset(&pktheader, 0x00, sizeof(pktheader));

	doc = xmlNewDoc((const xmlChar*)"1.0");
	add_packet_version(&root_node, PACKET_VERSION);
	xmlDocSetRootElement(doc, root_node);

	pktheader.type = PACKET_TYPE_REQ;
	pktheader.from = PACKET_FROM_DC;
	snprintf(pktheader.reqresptype, MAX_REQUEST_NAME, "%s", PACKET_REQLOGIN);
	pktheader.result = PACKET_SENT_SUCCESS;

	add_packet_header_node(&root_node, &pktheader);

	add_config_node(&root_node, &g_configs);

	/* Add DUT Details */
	while (curnode != NULL) {
		if (curnode->enabled) { /* add only one dut info just to get the config */
			add_dut_info_node(&doc, &root_node, &curnode->dut_info);
			break;
		}
		curnode = curnode->next;
	}

	xmlDocDumpMemory(doc, (xmlChar**)data, sz);
	if (*data)
		VIS_XML("XML DATA: %s\n\n", *data);

	xmlFreeDoc(doc);

	return 1;
}

/* Creates the counters request which will include site survery, channel stats and metrics
 * Details
 */
int
create_counters_request(vis_wl_interface_t *curnode, char **data, int *sz)
{
	xmlDocPtr	doc = NULL;
	xmlNodePtr	root_node = NULL;
	packet_header_t	pktheader;

	memset(&pktheader, 0x00, sizeof(pktheader));

	doc = xmlNewDoc((const xmlChar*)"1.0");
	add_packet_version(&root_node, PACKET_VERSION);
	xmlDocSetRootElement(doc, root_node);

	pktheader.type = PACKET_TYPE_REQ;
	pktheader.from = PACKET_FROM_DC;
	snprintf(pktheader.reqresptype, MAX_REQUEST_NAME, "%s", PACKET_REQCOUNTERS);
	pktheader.result = PACKET_SENT_SUCCESS;

	add_packet_header_node(&root_node, &pktheader);

	/* Add DUT Details */
	add_dut_info_node(&doc, &root_node, &curnode->dut_info);

	/* Add scan result */
	if (curnode->networks_list != NULL) {
		add_networks_node(&doc, &root_node, curnode->networks_list);
		free(curnode->networks_list);
		curnode->networks_list = NULL;
	}

	/* Add chanim stats node */
	if (curnode->congestion != NULL) {
		add_congestion_node(&root_node, curnode->congestion);
		free(curnode->congestion);
		curnode->congestion = NULL;
	}

	/* Add the Associated STA's element */
	if (curnode->stas_list != NULL) {
		add_associated_sta_node(&root_node, curnode->stas_list);
		free(curnode->stas_list);
		curnode->stas_list = NULL;
	}

	/* ADD tag Counters */
	if (curnode->counters.timestamp != 0)
		add_counters_node(&root_node, &curnode->counters);

	/* ADD tag AMPDU Stats */
	if (curnode->ampdulist != NULL) {
		add_ampdu_stats_node(&root_node, curnode->ampdulist);
		free(curnode->ampdulist);
		curnode->ampdulist = NULL;
	}

	/* Add the RRM STA side statistics element */
	if (curnode->rrmstatslist != NULL) {
		add_rrm_sta_stats_node(&root_node, curnode->rrmstatslist);
		free(curnode->rrmstatslist);
		curnode->rrmstatslist = NULL;
	}

	/* Add the STA side statistics element */
	if (curnode->stastatslist != NULL) {
		add_sta_stats_node(&root_node, curnode->stastatslist);
		free(curnode->stastatslist);
		curnode->stastatslist = NULL;
	}

	xmlDocDumpMemory(doc, (xmlChar**)data, sz);

	if (vis_debug_level & VIS_DEBUG_SAVEXMLFILE) {
		char filename[MAX_PATH];
		snprintf(filename, sizeof(filename), "/tmp/vis-xml-%s", curnode->ifr.ifr_name);
		xmlSaveFormatFile(filename, doc,  1);
	}

	if (*data)
		VIS_XML("XML DATA: %s\n\n", *data);
	xmlFreeDoc(doc);

	return 1;
}

/* If adding a new IOVAR call this function to creat the XML request to send it to server */
int
create_new_IOVAR_request(graph_list_t *graphdata, char *IOVARName, dut_info_t *dutinfo,
xmlChar **data, int *sz)
{
	xmlDocPtr	doc = NULL;
	xmlNodePtr	root_node = NULL;
	packet_header_t	pktheader;

	memset(&pktheader, 0x00, sizeof(packet_header_t));

	doc = xmlNewDoc((const xmlChar*)"1.0");
	add_packet_version(&root_node, PACKET_VERSION);
	xmlDocSetRootElement(doc, root_node);

	pktheader.type = PACKET_TYPE_REQ;
	pktheader.from = PACKET_FROM_DC;
	snprintf(pktheader.reqresptype, MAX_REQUEST_NAME, "%s", PACKET_REQCOUNTERS);
	pktheader.result = PACKET_SENT_SUCCESS;

	add_packet_header_node(&root_node, &pktheader);

	/* Add DUT Details */
	add_dut_info_node(&doc, &root_node, dutinfo);

	add_bargraph_details(&root_node, graphdata, IOVARName);

	xmlDocDumpMemory(doc, data, sz);

	if (vis_debug_level & VIS_DEBUG_SAVEXMLFILE)
		xmlSaveFormatFile("/tmp/vis-xml.xml", doc,  1);

	xmlFreeDoc(doc);

	return 1;
}

/* Create XML request of all the dut info */
int
create_all_dut_info_request(char **data, int *sz)
{
	xmlDocPtr	doc = NULL;
	xmlNodePtr	root_node = NULL;
	packet_header_t	pktheader;
	vis_wl_interface_t *curnode = g_wlinterface;

	memset(&pktheader, 0x00, sizeof(pktheader));

	doc = xmlNewDoc((const xmlChar*)"1.0");
	add_packet_version(&root_node, PACKET_VERSION);
	xmlDocSetRootElement(doc, root_node);

	pktheader.type = PACKET_TYPE_REQ;
	pktheader.from = PACKET_FROM_DC;
	snprintf(pktheader.reqresptype, MAX_REQUEST_NAME, "%s", PACKET_REQALLDUT);
	pktheader.result = PACKET_SENT_SUCCESS;

	add_packet_header_node(&root_node, &pktheader);

	add_config_node(&root_node, &g_configs);

	/* Add DUT Details */
	while (curnode != NULL) {
		add_dut_info_node(&doc, &root_node, &curnode->dut_info);
		curnode = curnode->next;
	}

	xmlDocDumpMemory(doc, (xmlChar**)data, sz);
	if (*data)
		VIS_XML("XML DATA: %s\n\n", *data);

	xmlFreeDoc(doc);

	return 1;
}
