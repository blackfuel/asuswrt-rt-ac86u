/*
 * Linux Visualization System common XML helper functions header
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
 * $Id: vis_xmlshared.h 606349 2015-12-15 07:15:11Z $
 */

#ifndef _VIS_XML_SHARED_H_
#define _VIS_XML_SHARED_H_

#include <stdio.h>
#include <libxml/tree.h>
#include "vis_struct.h"

extern int get_int_attribute(xmlNodePtr node, char *attribname, int *attribute);

extern int get_long_element(xmlDocPtr doc, xmlNodePtr node, long *element);

extern int get_string_element(xmlDocPtr doc, xmlNodePtr node, char *element, int nelemlen);

extern int get_int_element(xmlDocPtr doc, xmlNodePtr node, int *element);

extern int get_cdata_element(xmlNodePtr node, char *element, int nelemlen);

extern void add_config_node(xmlNodePtr *root_node, configs_t *configs);

extern int get_packet_header(xmlDocPtr doc, xmlNodePtr node, packet_header_t *pktheader);

extern int get_config_from_packet(xmlDocPtr doc, xmlNodePtr node, configs_t *conf);

extern void add_packet_version(xmlNodePtr *root_node, int version);

extern void add_packet_header_node(xmlNodePtr *root_node, packet_header_t *pktheader);

extern void add_dut_info_node(xmlDocPtr *doc, xmlNodePtr *root_node, dut_info_t *dut_info);

extern void add_networks_node(xmlDocPtr *doc, xmlNodePtr *root_node,
	networks_list_t *networks_list);

extern void add_congestion_node(xmlNodePtr *root_node, congestion_list_t *congestion);

extern void add_associated_sta_node(xmlNodePtr *root_node, assoc_sta_list_t *stas_list);

extern void add_counters_node(xmlNodePtr *root_node, counters_t *counters);

extern void add_ampdu_stats_node(xmlNodePtr *root_node, tx_ampdu_list_t *ampdulist);

extern int send_xml_data(int sockfd, xmlDocPtr *doc);

extern void free_xmldata(char **data);

extern int add_graph_header_details(xmlNodePtr *root_node, xmlNodePtr *node,
	iovar_handler_t *handle);

extern iovar_handler_t* find_iovar(const char *name);

extern int add_bargraph_details(xmlNodePtr *root_node, graph_list_t *graphdata, char *iovar_name);

extern int get_dut_settings_tag(xmlDocPtr doc, xmlNodePtr node, dut_settings_t *dutset);

extern void add_dut_settings_node(xmlNodePtr *root_node, dut_settings_t *dutset);

extern void add_rrm_sta_stats_node(xmlNodePtr *root_node, vis_rrm_statslist_t *rrmstatslist);

extern void add_sta_stats_node(xmlNodePtr *root_node, vis_sta_statslist_t *stastatslist);

#endif /* _VIS_XML_SHARED_H_ */
