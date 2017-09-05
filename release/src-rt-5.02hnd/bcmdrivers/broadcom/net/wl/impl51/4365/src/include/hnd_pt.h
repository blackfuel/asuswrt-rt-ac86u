/*
 * HND memory partition
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: hnd_pt.h $
 */

#ifndef	_HND_PT_H
#define	_HND_PT_H

#include <typedefs.h>
#include <bcmstdlib.h>
#include <osl_decl.h>

void mem_pt_init(osl_t *osh);

/* malloc, free */
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
#define hnd_malloc_pt(_size)	hnd_malloc_ptblk((_size), __FILE__, __LINE__)
extern void *hnd_malloc_ptblk(uint size, const char *file, int line);
#else
#define hnd_malloc_pt(_size)	hnd_malloc_ptblk((_size))
extern void *hnd_malloc_ptblk(uint size);
#endif /* BCMDBG_MEM */
extern void hnd_append_ptblk(void);
extern int hnd_free_pt(void *ptr);

/* Low Memory rescue functions
 * Implement a list of Low Memory free functions that rte can
 * call on allocation failure. List is populated through calls to
 * hnd_pt_lowmem_register() API
 */
typedef void (*hnd_lowmem_free_fn_t)(void *free_arg);
typedef struct hnd_lowmem_free hnd_lowmem_free_t;
struct hnd_lowmem_free {
	hnd_lowmem_free_t *next;
	hnd_lowmem_free_fn_t free_fn;
	void *free_arg;
};

extern void hnd_pt_lowmem_register(hnd_lowmem_free_t *lowmem_free_elt);
extern void hnd_pt_lowmem_unregister(hnd_lowmem_free_t *lowmem_free_elt);

#endif	/* _HND_PT_H */
