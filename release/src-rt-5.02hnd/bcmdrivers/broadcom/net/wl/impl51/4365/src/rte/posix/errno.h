/*
 * errno.h - Broadcom RTE-specific POSIX replacement data type definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: errno.h 473326 2014-04-29 00:37:35Z $
 */

#if !defined(_ERRNO_H_)
#define _ERRNO_H_

extern int errno;

#define EINVAL 1
#define EMSGSIZE 2

#endif /* !defined(_ERRNO_H_) */
