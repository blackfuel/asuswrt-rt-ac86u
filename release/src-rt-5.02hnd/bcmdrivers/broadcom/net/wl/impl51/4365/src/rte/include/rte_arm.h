/*
 * HND Run Time Environment ARM7TDMIs specific.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_arm.h 514393 2014-11-11 02:02:54Z $
 */

#ifndef _rte_arm_h_
#define _rte_arm_h_

#include <typedefs.h>
#include <sbhndarm.h>
#include <hndarm.h>

/* register access macros */
#define wreg32(r, v)		(*(volatile uint32 *)(r) = (uint32)(v))
#define rreg32(r)		(*(volatile uint32 *)(r))
#ifdef IL_BIGENDIAN
#define wreg16(r, v)		(*(volatile uint16 *)((uintptr)(r) ^ 2) = (uint16)(v))
#define rreg16(r)		(*(volatile uint16 *)((uintptr)(r) ^ 2))
#define wreg8(r, v)		(*(volatile uint8 *)((uintptr)(r) ^ 3) = (uint8)(v))
#define rreg8(r)		(*(volatile uint8 *)((uintptr)(r) ^ 3))
#else
#define wreg16(r, v)		(*(volatile uint16 *)(r) = (uint16)(v))
#define rreg16(r)		(*(volatile uint16 *)(r))
#define wreg8(r, v)		(*(volatile uint8 *)(r) = (uint8)(v))
#define rreg8(r)		(*(volatile uint8 *)(r))
#endif

/* uncached/cached virtual address */
#define	hnd_uncached(va)	((void *)(va))
#define	hnd_cached(va)		((void *)(va))

/* host/bus architecture-specific address byte swap */
#define BUS_SWAP32(v)		(v)

/* get cycle counter */
#define	osl_getcycles		get_arm_cyclecount

/* map/unmap physical to virtual I/O */
#define	hnd_reg_map(pa, size)	({BCM_REFERENCE(size); (void *)(pa);})
#define	hnd_reg_unmap(va)	BCM_REFERENCE(va)

/* map/unmap shared (dma-able) memory */
#ifdef CONFIG_XIP
#define MEMORY_REMAP	(SI_ARM_SRAM2)
/*
 * arm bootloader memory is remapped but backplane addressing is 0-based
 *
 * Background: since the mask rom bootloader code executes in a
 * read-only memory space apart from SoC RAM, data addresses
 * specified by bootloader code must be decoded differently from
 * text addresses. Consequently, the processor must run in a
 * "memory remap" mode whereby data addresses containing the
 * MEMORY_REMAP bit are decoded as residing in SoC RAM. However,
 * backplane agents, e.g., the dma engines, always use 0-based
 * addresses for SoC RAM, regardless of processor mode.
 * Consequently it is necessary to strip the MEMORY_REMAP bit
 * from addresses programmed into backplane agents.
 */
#define	hnd_dma_map(va, size)	({BCM_REFERENCE(size); ((uint32)va & ~MEMORY_REMAP);})
#else
#define	hnd_dma_map(va, size)	({BCM_REFERENCE(size); va;})
#endif /* CONFIG_XIP */
#define	hnd_dma_unmap(pa, size)	({BCM_REFERENCE(pa); BCM_REFERENCE(size);})

/* Cache support (or lack thereof) */
#if defined(__ARM_ARCH_7R__) && defined(RTE_CACHED)
extern void caches_on(void);
#else
static inline void caches_on(void) { return; }
#endif /* defined(__ARM_ARCH_7R__) && defined(RTE_CACHED) */

static inline void blast_dcache(void) { return; }
static inline void blast_icache(void) { return; }
static inline void flush_dcache(uint32 base, uint size) { return; }
static inline void flush_icache(uint32 base, uint size) { return; }

#endif	/* _rte_arm_h_ */
