/*
 * DONGLE debug macros
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: dngl_dbg.h 464743 2014-03-25 21:04:32Z $
 */

#ifndef _dngl_dbg_h_
#define _dngl_dbg_h_

#include <typedefs.h>
#include <osl.h>

#define DNGL_ERROR	0x00000001	/* enables err() printf statements */
#define DNGL_TRACE	0x00000002	/* enables trace() printf statements */
#define DNGL_PRPKT	0x00000008	/* enables hex() printf statements */
#define DNGL_INFORM	0x00000010	/* enables dbg() printf statements */

#if defined(BCMDBG) || defined(BCMDBG_ERR)
extern int dngl_msglevel;

#define dngl_dbg(bit, fmt, args...) do { \
	if (dngl_msglevel & (bit)) \
		printf("%s: " fmt "\n", __FUNCTION__ , ## args); \
} while (0)

#define dngl_hex(msg, buf, len) do { \
	if (dngl_msglevel & DNGL_PRPKT) \
		prhex(msg, buf, len); \
} while (0)

/* Debug functions */
#define err(fmt, args...) dngl_dbg(DNGL_ERROR, fmt , ## args)
#else
#define err(fmt, args...)
#endif	/* BCMDBG || BCMDBG_ERR */


#ifdef BCMDBG
#define trace(fmt, args...) dngl_dbg(DNGL_TRACE, fmt , ## args)
#define dbg(fmt, args...) dngl_dbg(DNGL_INFORM, fmt , ## args)
#define hex(msg, buf, len) dngl_hex(msg, buf, len)
#else
#define trace(fmt, args...)
#define dbg(fmt, args...)
#define hex(msg, buf, len)
#endif

#endif /* _dngl_dbg_h_ */
