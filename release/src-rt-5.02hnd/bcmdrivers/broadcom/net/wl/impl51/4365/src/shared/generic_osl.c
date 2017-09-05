/*
 * Generic OS Support Layer
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: generic_osl.c 467150 2014-04-02 17:30:43Z $
 */

/* ---- Include Files ---------------------------------------------------- */

#include "typedefs.h"
#include "bcmdefs.h"
#include "bcmendian.h"
#include "bcmutils.h"
#include "osl.h"
#include <stdlib.h>
#include "pkt_lbuf.h"

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define OSL_LOG(a)	printf a

#define OS_HANDLE_MAGIC		0x1234abcd	/* Magic # to recognise osh */
#define BCM_MEM_FILENAME_LEN 	24		/* Mem. filename length */


/* Linked list of dynamically allocated memory blocks. */
typedef struct bcm_mem_link {
	struct bcm_mem_link	*prev;
	struct bcm_mem_link 	*next;
	uint			size;
	int			line;
	char			file[BCM_MEM_FILENAME_LEN];
} bcm_mem_link_t;


/* Operating system state. */
struct osl_info {
	/* Publically accessible state variables. */
	osl_pubinfo_t	pub;

	/* Used for debug/validate purposes. */
	uint		magic;

	/* Network interface packet buffer handle. */
	pkt_handle_t 	*pkt_info;

	/* Number of bytes allocated. */
	uint 		malloced;

	/* Number of failed dynamic memory allocations. */
	uint 		failed;

	/* Linked list of allocated memory blocks. */
	bcm_mem_link_t	*dbgmem_list;
};


/* ---- Private Variables ------------------------------------------------ */

static osl_t *g_osh = 0;


/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

/* ----------------------------------------------------------------------- */
osl_t *
osl_attach(void)
{
	osl_t *osh;

	if (g_osh != 0)
		return g_osh;

	g_osh = osh = osl_malloc_std(sizeof(osl_t));
	ASSERT(osh);

	bzero(osh, sizeof(osl_t));
	osh->magic = OS_HANDLE_MAGIC;

	osh->pkt_info = pkt_init(osh);

	return (osh);
}

/* ----------------------------------------------------------------------- */
void
osl_detach(osl_t *osh)
{
	if (osh == NULL)
		return;

	ASSERT(osh->magic == OS_HANDLE_MAGIC);

	pkt_deinit(osh, osh->pkt_info);

	osl_free_std(osh);
	g_osh = 0;
}


#ifdef BCMDBG_MEM

/* ----------------------------------------------------------------------- */
void*
osl_debug_malloc(osl_t *osh, uint size, int line, char* file)
{
	bcm_mem_link_t *p;
	char* basename;

	if (osh == NULL)
		osh = g_osh;

	ASSERT(size);

	if ((p = (bcm_mem_link_t*)osl_malloc(osh, sizeof(bcm_mem_link_t) + size)) == NULL)
		return (NULL);

	p->size = size;
	p->line = line;

	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;

	strncpy(p->file, basename, BCM_MEM_FILENAME_LEN);
	p->file[BCM_MEM_FILENAME_LEN - 1] = '\0';

	/* link this block */
	p->prev = NULL;
	p->next = osh->dbgmem_list;
	if (p->next)
		p->next->prev = p;
	osh->dbgmem_list = p;

	return p + 1;
}

/* ----------------------------------------------------------------------- */
void*
osl_debug_mallocz(osl_t *osh, uint size, int line, char* file)
{
	void *ptr;

	ptr = osl_debug_malloc(osh, size, line, file);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

/* ----------------------------------------------------------------------- */
void
osl_debug_mfree(osl_t *osh, void *addr, uint size, int line, char* file)
{
	bcm_mem_link_t *p = (bcm_mem_link_t *)((int8*)addr - sizeof(bcm_mem_link_t));

	if (osh == NULL)
		osh = g_osh;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (p->size == 0) {
		OSL_LOG(("osl_debug_mfree: double free on addr %p size %d at line %d file %s\n",
			addr, size, line, file));
		ASSERT(p->size);
		return;
	}

	if (p->size != size) {
		OSL_LOG(("osl_debug_mfree: dealloc size %d does not match alloc size %d on addr %p"
		       " at line %d file %s\n",
		       size, p->size, addr, line, file));
		ASSERT(p->size == size);
		return;
	}

	/* unlink this block */
	if (p->prev)
		p->prev->next = p->next;
	if (p->next)
		p->next->prev = p->prev;
	if (osh->dbgmem_list == p)
		osh->dbgmem_list = p->next;
	p->next = p->prev = NULL;

	osl_mfree(osh, (char *)p, size + sizeof(bcm_mem_link_t));
}

/* ----------------------------------------------------------------------- */
int
osl_debug_memdump(osl_t *osh, struct bcmstrbuf *b)
{
	bcm_mem_link_t *p;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	bcm_bprintf(b, "   Address\tSize\tFile:line\n");
	for (p = osh->dbgmem_list; p; p = p->next)
		bcm_bprintf(b, "0x%08x\t%5d\t%s:%d\n",
			(uintptr)p + sizeof(bcm_mem_link_t), p->size, p->file, p->line);

	return 0;
}

#endif	/* BCMDBG_MEM */

/* ----------------------------------------------------------------------- */
void*
osl_malloc(osl_t *osh, uint size)
{
	void *addr;

	if (osh == NULL)
		osh = g_osh;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	addr = osl_malloc_std(size);

	if (addr == NULL)
		osh->failed++;
	else
		osh->malloced += size;

	return (addr);
}

/* ----------------------------------------------------------------------- */
void*
osl_mallocz(osl_t *osh, uint size)
{
	void *ptr;

	ptr = osl_malloc(osh, size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

/* ----------------------------------------------------------------------- */
void
osl_mfree(osl_t *osh, void *addr, uint size)
{
	if (osh == NULL)
		osh = g_osh;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	osh->malloced -= size;
	osl_free_std(addr);
}

/* ----------------------------------------------------------------------- */
uint
osl_malloced(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (osh->malloced);
}

/* ----------------------------------------------------------------------- */
uint
osl_malloc_failed(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (osh->failed);
}


/* ----------------------------------------------------------------------- */
#ifdef BCMDBG_ASSERT
void
osl_assert(char *exp, char *file, int line)
{
	int *null = NULL;

	printf("\n\nASSERT \"%s\" failed: file \"%s\", line %d\n\n", exp, file, line);

	/* Force a crash. */
	*null = 0;
}
#endif /* BCMDBG_ASSERT */
