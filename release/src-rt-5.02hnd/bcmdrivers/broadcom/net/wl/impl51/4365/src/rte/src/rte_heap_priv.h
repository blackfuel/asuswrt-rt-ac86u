/*
 * Heap mgmt. - Private to RTE.
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

#ifndef	_hnd_heap_priv_
#define	_hnd_heap_priv_

#include <typedefs.h>

extern void hnd_heap_cli_init(void);

extern bool hnd_arena_init(uintptr base, uintptr lim);

extern void hnd_lbuf_fixup_2M_tcm(void);

extern uint hnd_hwm(void);

extern void hnd_meminuse(uint *inuse, uint *inuse_oh);
#ifdef BCMDBG_MEM
extern int hnd_memcheck(char *file, int line);
#endif

#endif /* _hnd_heap_priv_ */
