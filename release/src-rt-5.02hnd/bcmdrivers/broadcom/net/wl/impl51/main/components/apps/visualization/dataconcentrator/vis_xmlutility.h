/*
 * Linux Visualization Data Concentrator XML utility header
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
 * $Id: vis_xmlutility.h 555336 2015-05-08 09:52:04Z $
 */

#ifndef _VIS_XML_UTILITY_H_
#define _VIS_XML_UTILITY_H_

#include <stdio.h>
#include <libxml/tree.h>
#include "vis_struct.h"

extern int parse_response(int sockfd, unsigned char **data, unsigned int size);

extern int send_config(int sockfd, configs_t *configs);

extern char* ReadFile(char *filename, int *size);

extern int create_login_request(xmlChar **data, int *sz);

extern int create_counters_request(xmlChar **data, int *sz);

extern void free_xmldata(char **data);

extern int send_to_server(int sockfd, char *data, unsigned int len);

extern int send_response(int sockfd, packet_header_t *pktheader, dut_settings_t *dutset);

extern onlygraphnamelist_t *get_enable_grphnames_from_packet(xmlDocPtr doc, xmlNodePtr node);

extern void copy_config_settings(configs_t *configs, int fromgconfig);

extern void copy_gateway_ip_to_config(char *gatewayip);

#endif /* _VIS_XML_UTILITY_H_ */
