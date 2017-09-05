/*
 * Linux Visualization System common socket utility implementation
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
 * $Id: vis_sock_util.c 555336 2015-05-08 09:52:04Z $
 */

#include "vis_sock_util.h"

static int send_data(int sockfd, char *data, unsigned int len);
static int recv_data(int sockfd, unsigned char *read_buf, uint32 size);

/* Initializes the socket */
int
init_socket()
{
#ifdef WIN32
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		VIS_SOCK(stderr, "WSAStartup failed.\n");
		return -1;
	}
#endif /* WIN32 */

	return 1;
}

/* Closes the socket */
void
close_socket(int *sockfd)
{
	if (*sockfd == INVALID_SOCKET)
		return;

#ifdef WIN32
	closesocket(*sockfd);
#else
	close(*sockfd);
#endif /* WIN32 */
	*sockfd = INVALID_SOCKET;
}

/* Get the address */
void*
get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* Adds the length to the first 4 bytes and sends it to the server */
int
add_length_and_send_data(int sockfd, const char *data, unsigned int len)
{
	int	ret = 0;
	char	*sdata = NULL;
	int	totlen = 4 + len;

	sdata = (char*)malloc(sizeof(char) * (len + 4));
	if (sdata == NULL) {
		VIS_SOCK("Failed to allocate sdata buffer of size : %d\n", (len+4));
		return -1;
	}

	memcpy(sdata, &totlen, 4);

	memcpy(sdata+4, data, len);

	ret = send_data(sockfd, sdata, totlen);

	free(sdata);

	return ret;
}

/* Sends the data to socket */
static int
send_data(int sockfd, char *data, unsigned int len)
{
	int	nret = 0;
	int	totalsize = len, totalsent = 0;

	while (totalsent < totalsize) {
		fd_set WriteFDs, ExceptFDs;
		struct timeval tv;

		FD_ZERO(&WriteFDs);
		FD_ZERO(&ExceptFDs);

		if (sockfd == INVALID_SOCKET)
			return INVALID_SOCKET;

		FD_SET(sockfd, &WriteFDs);

		tv.tv_sec = VIS_SOCKET_WAIT_TIMEOUT;
		tv.tv_usec = 0;
		if (select(sockfd+1, NULL, &WriteFDs, &ExceptFDs, &tv) > 0) {
			if (FD_ISSET(sockfd, &WriteFDs))
				;
			else
				return INVALID_SOCKET;
		}

		nret = send(sockfd, &(data[totalsent]), len, 0);
		if (nret < 0) {
			print_error("send");
			return INVALID_SOCKET;
		}
		totalsent += nret;
		len -= nret;
		nret = 0;
	}

	return totalsent;
}

/* return value contains the number of bytes read or -1 for error
 * free the buffer after reading the data
 */
int
on_receive(int sockfd, unsigned char **data)
{
	int		sz, size = 0;
	unsigned char	szbuff[VIS_LEN_FIELD_SZ + 1] = {0};
	unsigned char	*read_buf = NULL;

	sz = recv_data(sockfd, szbuff, VIS_LEN_FIELD_SZ);
	if (sz <= 0) {
		VIS_SOCK("Read Failed\n");
		return INVALID_SOCKET;
	}

	if (sz >= VIS_LEN_FIELD_SZ) {
		memcpy(&size, szbuff, VIS_LEN_FIELD_SZ);
		VIS_SOCK("Total size : %d Total Read : %d\n", size, sz);
	} else {
		VIS_SOCK("Doesn't contain any size\n");
		return -1;
	}
	if (size <= VIS_LEN_FIELD_SZ) {
		VIS_SOCK("Size of less than %d indicates there is no data\n", VIS_LEN_FIELD_SZ);
		return -1;
	}

	read_buf = (unsigned char *)malloc(sizeof(unsigned char) * ((size-VIS_LEN_FIELD_SZ) + 1));
	if (read_buf == NULL) {
		VIS_SOCK("Failed to allocate read_buf buffer of size : %d\n",
			(size-VIS_LEN_FIELD_SZ));
		return -1;
	}

	sz = recv_data(sockfd, read_buf, (size-VIS_LEN_FIELD_SZ));
	if (sz <= 0) {
		if (read_buf != NULL) {
			free(read_buf);
			read_buf = NULL;
		}
		return INVALID_SOCKET;
	}

	*data = read_buf;

	return sz;
}

/* to recieve the 'size' number of data */
static int
recv_data(int sockfd, unsigned char *read_buf, uint32 size)
{
	uint32		nbytes, totalread = 0;
	struct timeval	tv;
	fd_set		ReadFDs, ExceptFDs;

	FD_ZERO(&ReadFDs);
	FD_ZERO(&ExceptFDs);
	FD_SET(sockfd, &ReadFDs);
	FD_SET(sockfd, &ExceptFDs);
	tv.tv_sec = VIS_SOCKET_WAIT_TIMEOUT;
	tv.tv_usec = 0;

	while (totalread < size) {
		if (select(sockfd+1, &ReadFDs, NULL, &ExceptFDs, &tv) > 0) {
			if (FD_ISSET(sockfd, &ReadFDs)) {
				/* fprintf(stdout, "SOCKET : Data is ready to read\n"); */;
			} else {
				return INVALID_SOCKET;
			}
		}

		nbytes = read(sockfd, &(read_buf[totalread]), size);
		VIS_SOCK("Read bytes  = %d\n\n", nbytes);

		if (nbytes <= 0) {
			print_error("Read Error");
			return INVALID_SOCKET;
		}

		totalread += nbytes;
	}

	read_buf[totalread] = '\0';

	return totalread;
}

/* Gets the socket error code */
int
get_socket_error_code()
{
#ifdef WIN32
	return WSAGetLastError();
#else
	return errno;
#endif /* WIN32 */
}

/* prints the error to the console */
void
print_error(char *str)
{
#ifdef WIN32
	VIS_SOCK("Error in %s is : %d\n", str, WSAGetLastError());
#else
	VIS_SOCK("Error in %s is : %s\n", str, strerror(errno));
#endif /* WIN32 */
}
