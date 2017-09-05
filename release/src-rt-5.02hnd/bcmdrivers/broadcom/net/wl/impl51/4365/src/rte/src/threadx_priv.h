/*
 * Threadx application support.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: threadx_priv.h 597803 2015-11-06 08:46:00Z $
 */

#ifndef	_THREADX_PRIV_H
#define	_THREADX_PRIV_H

#include <typedefs.h>

/* These trap structure are private between the CPU/ThreadX and the ISR
 * routine threadx_isr i.e. they are determined by how CPU/ThreadX saves
 * thread context (either by CPU h/w or by ThreadX s/w).
 */
#ifdef __ARM_ARCH_7M__
typedef struct {
	uint32	IPSR;
	uint32	r10;
	uint32	lr_s;	/* same as lr but pushed by s/w */
	uint32	r0;	/* (Hardware stack starts here!!) */
	uint32	r1;
	uint32	r2;
	uint32	r3;
	uint32	r12;
	uint32	lr;
	uint32	pc;
	uint32	xPSR;
} threadx_trap_t;
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
typedef struct {
	uint32	SPSR;
	uint32	r10;
	uint32	ip;
	uint32	lr;
	uint32	r0;
	uint32	r1;
	uint32	r2;
	uint32	r3;
} threadx_trap_t;
#else /* Unsupported CPU architecture */
#error unsupported CPU architecture
#endif /* Unsupported CPU architecture */

/* top level interrupt handler */
void threadx_isr(threadx_trap_t *tr);
#ifndef BCMDBG_LOADAVG
void threadx_fiq_isr(void);
#endif

#endif	/* _THREADX_PRIV_H */
