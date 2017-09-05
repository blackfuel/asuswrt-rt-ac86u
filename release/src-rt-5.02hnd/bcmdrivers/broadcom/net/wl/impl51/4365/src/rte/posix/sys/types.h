/*
 * types.h - Broadcom RTE-specific POSIX replacement data type definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: types.h 473326 2014-04-29 00:37:35Z $
 */

#if !defined(_TYPES_H_)
#define _TYPES_H_

typedef char * caddr_t;

/* We don't really support file descriptor sets in RTE, this definition
   is simply to keep us from using ifdefs to make our code compile. */
typedef int fd_set;

#include <osl.h>
typedef hnd_timer_t timer_t;

#endif /* !defined(_TYPES_H_) */
