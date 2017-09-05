/*
 * stdlib.h - Broadcom RTE-specific POSIX replacement STD LIB definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: stdlib.h 473326 2014-04-29 00:37:35Z $
 */

#if !defined(_STDLIB_H_)
#define _STDLIB_H_

#include <osl.h>

#define malloc hnd_malloc
#define realloc hnd_realloc
#define free hnd_free

#include <bcmstdlib.h>

#endif /* !defined(_STDLIB_H_) */
