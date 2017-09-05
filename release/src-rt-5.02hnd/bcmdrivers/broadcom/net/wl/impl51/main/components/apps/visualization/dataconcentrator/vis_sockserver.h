/*
 * Linux Visualization Data Concentrator socket server header
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
 * $Id: vis_sockserver.h 544877 2015-03-30 08:47:06Z $
 */

#ifndef _VIS_SOCK_SERVER_H_
#define _VIS_SOCK_SERVER_H_

#include "vis_sock_util.h"

extern int create_server(uint32 portno, int twocpuenabled);

extern void close_server();

#endif /* _VIS_SOCK_SERVER_H_ */
