/*
 * stdio.h - Broadcom RTE-specific POSIX replacement STD IO definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: stdio.h 473326 2014-04-29 00:37:35Z $
 */

#if !defined(_STDIO_H_)
#define _STDIO_H_

#define EOF (-1)

#define fprintf(stream, fmt, args...) printf(fmt, ## args)

#define perror(string) printf("%s error - %s", __FILE__, string )

/* Return NULL on fopen */
#define fopen(a, b) NULL
#define fclose(fp) do { } while(0)

#include <bcmstdlib.h>

#endif /* !defined(_STDIO_H_) */
