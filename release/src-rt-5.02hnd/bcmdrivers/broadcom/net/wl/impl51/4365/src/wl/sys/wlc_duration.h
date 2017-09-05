/*
 * wlc_duration.h
 *
 * This module provides definitions for a basic profiler.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */


#if !defined(__wlc_duration_h__)
#define __wlc_duration_h__

/* Makes it easier to keep duration_enum and duration_names in sync */
#define FOREACH_DURATION(ENUMDEF)	\
	ENUMDEF(DUR_WATCHDOG)		\
	ENUMDEF(DUR_DPC)		\
	ENUMDEF(DUR_DPC_TXSTATUS)	\
	ENUMDEF(DUR_DPC_TXSTATUS_SENDQ)	\
	ENUMDEF(DUR_DPC_RXFIFO)		\
	ENUMDEF(DUR_SENDQ)		\
	ENUMDEF(DUR_LAST)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum {
	FOREACH_DURATION(GENERATE_ENUM)
} duration_enum;

extern duration_info_t *BCMATTACHFN(wlc_duration_attach)(wlc_info_t *);
extern int BCMATTACHFN(wlc_duration_detach)(duration_info_t *);

extern void wlc_duration_enter(wlc_info_t *wlc, duration_enum idx);
extern void wlc_duration_exit(wlc_info_t *wlc, duration_enum idx);

#endif /* __wlc_duration_h__ */
