/*
 * HND mips trap handling.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: hnd_mipstrap.h 470663 2014-04-16 00:24:43Z $
 */

#ifndef	_HND_MIPSTRAP_H
#define	_HND_MIPSTRAP_H


/* MIPS trap handling */

/* Trap locations in lo memory */
#define FIRST_TRAP	0
#define LAST_TRAP	0x480
#define	TRAP_STRIDE	0x80

/* Trap "type" is the trap location */

#define	TRAP_TYPE_SH	7
#define	MAX_TRAP_TYPE	((LAST_TRAP >> TRAP_TYPE_SH) + 1)

/* The trap structure is defined here as offsets for assembly */
#define	TR_TYPE		0x00
#define	TR_STATUS	0x04
#define	TR_CAUSE	0x08
#define	TR_EPC		0x0c
#define	TR_HI		0x10
#define	TR_LO		0x14
#define	TR_BVA		0x18
#define	TR_ERRPC	0x1c
#define	TR_REGS		0x20

#define	TRAP_T_SIZE	160

#ifndef	_LANGUAGE_ASSEMBLY

#include <typedefs.h>

typedef struct _trap_struct {
	uint32		type;
	uint32		status;
	uint32		cause;
	uint32		epc;
	uint32		hi;
	uint32		lo;
	uint32		badvaddr;
	uint32		errorpc;
	uint32		r0;
	uint32		r1;
	uint32		r2;
	uint32		r3;
	uint32		r4;
	uint32		r5;
	uint32		r6;
	uint32		r7;
	uint32		r8;
	uint32		r9;
	uint32		r10;
	uint32		r11;
	uint32		r12;
	uint32		r13;
	uint32		r14;
	uint32		r15;
	uint32		r16;
	uint32		r17;
	uint32		r18;
	uint32		r19;
	uint32		r20;
	uint32		r21;
	uint32		r22;
	uint32		r23;
	uint32		r24;
	uint32		r25;
	uint32		r26;
	uint32		r27;
	uint32		r28;
	uint32		r29;
	uint32		r30;
	uint32		r31;
} trap_t;

#endif	/* !_LANGUAGE_ASSEMBLY */
#endif	/* _HND_TRAP_H */
