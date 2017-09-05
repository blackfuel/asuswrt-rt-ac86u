/*
 * HND heap management.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_heap.h 488783 2014-07-02 12:45:04Z $
 */

#ifndef	_RTE_HEAP_H
#define	_RTE_HEAP_H

#include <typedefs.h>
#include <rte_ioctl.h>

/* malloc, free */
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
#define hnd_malloc(size)	hnd_malloc_align(size, 0, __FILE__, __LINE__)
extern void *hnd_malloc_align(uint size, uint alignbits, const char *file, int line);
#else
#define hnd_malloc(size)	hnd_malloc_align(size, 0)
extern void *hnd_malloc_align(uint size, uint alignbits);
#endif /* BCMDBG_MEM */
extern void *hnd_realloc(void *ptr, uint size);
extern void *hnd_calloc(uint num, uint size);
extern int hnd_free(void *ptr);
extern uint hnd_memavail(void);

extern uint hnd_arena_add(uint32 base, uint size);

#ifndef BCM_BOOTLOADER
#if defined(RTE_CONS) || defined(BCM_OL_DEV)
extern int hnd_get_heapuse(memuse_info_t *hu);

#endif /* RTE_CONS || BCM_OL_DEV */
#endif /* BCM_BOOTLOADER */

#endif	/* _RTE_HEAP_H */
