/*
 * Linux Visualization Data Collector client socket implementation
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
 * $Id: vis_socketclient.c 555336 2015-05-08 09:52:04Z $
 */
#include "vis_socketclient.h"

/* Given port and server address it connects to the server */
int
connect_to_server(uint32 nport, char *straddrs)
{
	struct sockaddr_in server_addr;
	int res, valopt;
	long arg;
	fd_set readfds;
	struct timeval tv;
	socklen_t lon;
	int sockfd;
	struct hostent *host;

	init_socket();

	sockfd = INVALID_SOCKET;
	memset(&server_addr, 0, sizeof(server_addr));

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		print_error("socket");
		return INVALID_SOCKET;
	}

	VIS_SOCK("Server = %s\t port = %d\n", straddrs, nport);

	if ((host = gethostbyname(straddrs)) == NULL) {
		print_error("gethostbyname");
		return INVALID_SOCKET;
	}

	/* Set nonblock on the socket so we can timeout */
	if ((arg = fcntl(sockfd, F_GETFL, NULL)) < 0 ||
		fcntl(sockfd, F_SETFL, arg | O_NONBLOCK) < 0) {
			print_error("fcntl error");
			return INVALID_SOCKET;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nport);
	server_addr.sin_addr = *((struct in_addr*)host -> h_addr);
	res = connect(sockfd, (struct sockaddr*)&server_addr,
		sizeof(struct sockaddr));

	if (res < 0) {
		if (errno == EINPROGRESS) {
			tv.tv_sec = 30;
			tv.tv_usec = 0;
			FD_ZERO(&readfds);
			FD_SET(sockfd, &readfds);
			if (select(sockfd+1, NULL, &readfds, NULL, &tv) > 0) {
				lon = sizeof(int);
				getsockopt(sockfd, SOL_SOCKET, SO_ERROR,
					(void*)(&valopt), &lon);
				if (valopt) {
					VIS_SOCK("Error in connection() %d - %s\n",
						valopt, strerror(valopt));
					return INVALID_SOCKET;
				}
			} else {
				VIS_SOCK("Timeout or error() %d - %s\n",
					valopt, strerror(valopt));
				return INVALID_SOCKET;
			}
		} else {
			VIS_SOCK("Error connecting %d - %s\n",
				errno, strerror(errno));
			return INVALID_SOCKET;
		}
	}
	VIS_SOCK("Connection Successfull with server : %s\n", straddrs);

	return sockfd;
}
