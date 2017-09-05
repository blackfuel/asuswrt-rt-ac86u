/*
 * HND RTE OS Abstraction Layer
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_osl.c 557562 2015-05-19 10:53:33Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#ifdef SBPCI
#include <hndpci.h>
#endif /* SBPCI */
#include <rte_heap.h>
#include <rte_trap.h>

osl_t *
#ifdef SHARED_OSL_CMN
BCMATTACHFN(osl_attach)(void *dev, void **osl_cmn)
#else
BCMATTACHFN(osl_attach)(void *dev)
#endif /* SHARED_OSL_CMN */
{
#ifndef SHARED_OSL_CMN
	void **osl_cmn = NULL;
#endif
	osl_t *osh;

	osh = (osl_t *)hnd_malloc(sizeof(osl_t));
	ASSERT(osh);
	bzero(osh, sizeof(osl_t));

	if (osl_cmn == NULL || *osl_cmn == NULL) {
		osh->cmn = (osl_cmn_t *)hnd_malloc(sizeof(osl_cmn_t));
		bzero(osh->cmn, sizeof(osl_cmn_t));
		if (osl_cmn)
			*osl_cmn = osh->cmn;
	} else
		if (osl_cmn)
			osh->cmn = *osl_cmn;

	osh->cmn->refcount++;

	ASSERT(osh->cmn);

	osh->dev = dev;
	return osh;
}

void
BCMATTACHFN(osl_detach)(osl_t *osh)
{
	if (osh == NULL)
		return;
	osh->cmn->refcount--;
	hnd_free(osh);
}

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
void *
osl_malloc(uint size, const char *file, int line)
{
	return hnd_malloc_align(size, 0, file, line);
}

void *
osl_mallocz(uint size, const char *file, int line)
{
	void * ptr;

	ptr = osl_malloc(size, file, line);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void *
osl_malloc_align(uint size, uint align_bits, const char *file, int line)
{
	return hnd_malloc_align(size, align_bits, file, line);
}
#else
void *
osl_malloc(uint size)
{
	void *p = hnd_malloc_align(size, 0);
#ifdef BCMDBG_MEMNULL
	if (p == NULL)
		printf("MALLOC failed: size=%d ra=0x%x\n", size, __builtin_return_address(0));
#endif
	return p;
}

void *
osl_mallocz(uint size)
{
	void * ptr;

	ptr = osl_malloc(size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void *
osl_malloc_align(uint size, uint align_bits)
{
	void *p = hnd_malloc_align(size, align_bits);
#ifdef BCMDBG_MEMNULL
	if (p == NULL)
		printf("MALLOC failed: size=%d ra=0x%x\n", size, __builtin_return_address(0));
#endif
	return p;
}
#endif /* BCMDBG_MEM */

int
osl_mfree(void *addr)
{
	return hnd_free(addr);
}

uint
osl_malloced(osl_t *osh)
{
	return 0;
}

uint
osl_malloc_failed(osl_t *osh)
{
	return 0;
}

int
osl_busprobe(uint32 *val, uint32 addr)
{
	*val = *(uint32 *)addr;
	return 0;
}

/* translate these erros into hnd specific errors */
int
osl_error(int bcmerror)
{
	return bcmerror;
}

#ifdef SBPCI
#include <typedefs.h>
#include <rte_dev.h>

uint
osl_pci_bus(osl_t *osh)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;

	return pdev->bus;
}

uint
osl_pci_slot(osl_t *osh)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;

	return pdev->slot;
}

uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;
	uint32 data;

	if (extpci_read_config(pdev->sih, pdev->bus, pdev->slot, pdev->func, offset,
	                       &data, size) != 0)
		data = 0xffffffff;

	return data;
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = dev->pdev;

	extpci_write_config(pdev->sih, pdev->bus, pdev->slot, pdev->func, offset, &val, size);
}
#endif /* SBPCI */

uint32
osl_rand(void)
{
	uint32 x, hi, lo, t;

	x = OSL_SYSUPTIME();
	hi = x / 127773;
	lo = x % 127773;
	t = 16807 * lo - 2836 * hi;
	if (t <= 0) t += 0x7fffffff;
	return t;
}

void
osl_sys_halt(void)
{
	hnd_die();
}
