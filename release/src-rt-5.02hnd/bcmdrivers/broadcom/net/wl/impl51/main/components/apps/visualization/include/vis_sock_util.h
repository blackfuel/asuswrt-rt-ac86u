/*
 * Linux Visualization System common socket utilities header
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
 * $Id: vis_sock_util.h 555336 2015-05-08 09:52:04Z $
 */

#ifndef _VIS_SOCK_UTIL_H_
#define _VIS_SOCK_UTIL_H_

#ifdef WIN32
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif /* _MSC_VER */
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h> /* _beginthread() */
#else
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <errno.h>

#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
#endif /* WIN32 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <typedefs.h>
#include "defines.h"

#define VIS_SOCKET_WAIT_TIMEOUT	15
#define MAX_READ_BUFFER		1449

extern int init_socket();

extern void *get_in_addr(struct sockaddr *sa);

extern void close_socket(int *sockfd);

extern int add_length_and_send_data(int sockfd, const char *data, unsigned int len);

extern int on_receive(int sockfd, unsigned char **data);

extern int get_socket_error_code();

extern void print_error(char *str);

#endif /* _VIS_SOCK_UTIL_H_ */
