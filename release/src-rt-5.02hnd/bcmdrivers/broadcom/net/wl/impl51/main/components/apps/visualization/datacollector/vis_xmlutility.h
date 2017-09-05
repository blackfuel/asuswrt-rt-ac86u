/*
 * Linux Visualization Data Collector xml utility header
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
 * $Id: vis_xmlutility.h 544222 2015-03-26 10:48:42Z $
 */

#ifndef _VIS_XML_UTILITY_H_
#define _VIS_XML_UTILITY_H_

#include <stdio.h>
#include <libxml/tree.h>
#include "vis_struct.h"

extern int parse_response(unsigned char *data, unsigned int size);

extern int create_login_request(char **data, int *sz);

extern int create_counters_request(vis_wl_interface_t *curnode, char **data, int *sz);

extern int create_all_dut_info_request(char **data, int *sz);

#endif /* _VIS_XML_UTILITY_H_ */
