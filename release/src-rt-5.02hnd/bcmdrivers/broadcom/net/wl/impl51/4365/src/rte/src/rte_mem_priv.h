/*
 * Memory layout. - Private to RTE.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef	_hnd_mem_priv_
#define	_hnd_mem_priv_

#include <typedefs.h>

/* GLOBAL stack is used during intialization & at runtime in GLOBAL stack config,
 * and also used by WLAN thread in non GLOBAL stack config.
 */
#ifndef	 HND_STACK_SIZE
#define	 HND_STACK_SIZE	(8192)
#endif

/* SVC mode stack is only used during intialization in non GLOBAL stack config,
 * and maybe shared by IRQ mode stack in non GLOBAL stack config.
 */
#ifndef	 SVC_STACK_SIZE
#define	 SVC_STACK_SIZE	512
#endif

void hnd_mem_cli_init(void);

#ifdef GLOBAL_STACK
void hnd_stack_init(uint32 *stackbottom, uint32 *stacktop);
void hnd_stack_check(void);
#endif

#endif /* _hnd_mem_priv_ */
