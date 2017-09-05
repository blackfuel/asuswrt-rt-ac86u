/*
 * SES debug header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: ses_dbg.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _ses_dbg_h_
#define _ses_dbg_h_

extern void debug_ses_trace(const char * fmt, ...);

#ifdef BCMDBG
#include <stdio.h>
#include <string.h>

#define SES_DEBUG_ERROR 	0x0001
#define SES_DEBUG_WARNING	0x0002
#define SES_DEBUG_INFO		0x0004
#define SES_DEBUG_PACKET	0x0008
#define SES_DEBUG_CB		0x0010
#define SES_DEBUG_GPIO		0x0020

extern int ses_debug_level;

#ifdef _MSC_VER

#define SES_ERROR \
        if (ses_debug_level & SES_DEBUG_ERROR)		debug_ses_trace

#define SES_WARNING \
        if (ses_debug_level & SES_DEBUG_WARNING)	debug_ses_trace

#define SES_INFO \
        if (ses_debug_level & SES_DEBUG_INFO)		debug_ses_trace

#define SES_CB \
        if (ses_debug_level & SES_DEBUG_CB)		debug_ses_trace

#define SES_GPIO \
        if (ses_debug_level & SES_DEBUG_CB)		debug_ses_trace

#define SES_PACKET(mem, size) \
		do { \
        if (ses_debug_level & SES_DEBUG_PACKET) { \
                char buf[80]; \
                int i, j, k; \
                for (i = 0; i < size; ) { \
                        j = sprintf(buf, "%04X: ", i); \
                        for (k = 0; k < 16 && i < size; k ++, i ++) \
                                j += sprintf(&buf[j], " %02X", mem[i]); \
                        debug_ses_trace("%s\n", buf); \
                } \
        } \
		} while(0)

#else

#define SES_ERROR(fmt, arg...) \
        do { if (ses_debug_level & SES_DEBUG_ERROR) \
                printf("%s: "fmt , __FUNCTION__ , ##arg); } while(0)

#define SES_WARNING(fmt, arg...) \
        do { if (ses_debug_level & SES_DEBUG_WARNING) \
                printf("%s: "fmt , __FUNCTION__ , ##arg); } while(0)

#define SES_INFO(fmt, arg...) \
        do { if (ses_debug_level & SES_DEBUG_INFO) \
                printf("%s: "fmt , __FUNCTION__ , ##arg); } while(0)

#define SES_CB(fmt, arg...) \
        do { if (ses_debug_level & SES_DEBUG_CB) \
                printf("%s: "fmt , __FUNCTION__ , ##arg); } while(0)

#define SES_GPIO(fmt, arg...) \
        do { if (ses_debug_level & SES_DEBUG_GPIO) \
                printf("%s: "fmt , __FUNCTION__ , ##arg); } while(0)

#define SES_PACKET(mem, size)       ( \
{ \
        if (ses_debug_level & SES_DEBUG_PACKET) { \
                char buf[80]; \
                int i, j, k; \
                for (i = 0; i < size; ) { \
                        j = sprintf(buf, "%04X: ", i); \
                        for (k = 0; k < 16 && i < size; k ++, i ++) \
                                j += sprintf(&buf[j], " %02X", mem[i]); \
                        printf("%s\n", buf); \
                } \
        } \
} \
)

#endif /* ifdef _MSC_VER */

#else   /* #if BCMDBG */

#ifdef _MSC_VER

#define SES_ERROR		1 ? (void)0 : debug_ses_trace
#define SES_WARNING		1 ? (void)0 : debug_ses_trace
#define SES_INFO		1 ? (void)0 : debug_ses_trace
#define SES_CB			1 ? (void)0 : debug_ses_trace
#define SES_GPIO		1 ? (void)0 : debug_ses_trace
#define SES_PACKET		1 ? (void)0 : debug_ses_trace

#else

#define SES_ERROR(fmt, arg...)
#define SES_WARNING(fmt, arg...)
#define SES_INFO(fmt, arg...)
#define SES_CB(fmt, arg...)
#define SES_GPIO(fmt, arg...)
#define SES_PACKET(mem, size)

#endif /* #ifdef _MSC_VER */

#endif  /* #if BCMDBG */

#endif /* _ses_dbg_h_ */
