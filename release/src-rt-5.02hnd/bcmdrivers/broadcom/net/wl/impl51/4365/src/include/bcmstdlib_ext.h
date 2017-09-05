/*
 * bcmstdlib extension
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: bcmstdlib_ext.h 480142 2014-05-23 00:13:54Z $
 */


#ifndef _bcmstdlib_ext_h_
#define _bcmstdlib_ext_h_

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Include Files ---------------------------------------------------- */

#include <typedefs.h>

/* ---- Constants and Types ---------------------------------------------- */

/* ---- Variable Externs ------------------------------------------------- */
extern int in_isr_handler, in_trap_handler, in_fiq_handler;

/* ---- Function Prototypes ---------------------------------------------- */
#ifdef HND_PRINTF_THREAD_SAFE
bool printf_lock_init(void);
bool printf_lock_cleanup(void);
#endif	/* HND_PRINTF_THREAD_SAFE */

#ifdef __cplusplus
	}
#endif

#endif  /* _bcmstdlib_ext_h_  */
