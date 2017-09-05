/*
 * sys/select.h - Broadcom RTE-specific POSIX replacement data type 
 * definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: select.h 473326 2014-04-29 00:37:35Z $
 */

#if !defined(_SELECT_H_)
#define _SELECT_H_

/* None file descriptor set control macro's do anything other than 
   satisfy the compiler.  This keeps ifdef out of our code even though
   we don't support fd_sets on RTE(yet?)
*/
#define FD_ZERO(a)
#define FD_SET(a,b)
#define FD_ISSET(a,b) 0
#define FD_ZERO(a)

struct sockaddr_in {
        short   sin_family;            /* address family             */
        unsigned short sin_port;       /* port number                */
        struct  in_addr sin_addr;      /* internet address           */
        char    sin_zero[8];           /* 8-byte field of all zeroes */
};

#include <time.h>
int select(int n, void *readfds, void *writefds, void *exceptfds, struct timeval *timeout);

#endif /* !defined(_SELECT_H_) */
