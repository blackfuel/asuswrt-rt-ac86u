/*
 * HND Run Time Environment Assert Facility.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_assert.h 580528 2015-08-19 11:48:07Z $
 */

#ifndef _rte_assert_h_
#define _rte_assert_h_

#include <typedefs.h>

extern uint32 g_assert_type;

/* accessor functions */
extern uint32 get_g_assert_type(void);
extern void set_g_assert_type(uint32 val);

void hnd_assert(const char *file, int line);

/* assertion */
#if defined(BCMDBG_ASSERT)
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)

#include <hnd_armtrap.h>

/* Use the system service call (SVC) instruction to generate a software
 * interrupt for a failed assert. This will use a 2-byte instruction to
 * generate the trap. It is more memory efficient than the alternate C
 * implementation which may use more than 2-bytes to generate the trap.
 * It also allows the trap handler to uniquely identify the trap as an
 * assert (since this is the only use of software interrupts in the system).
 */
#define ASSERT_WITH_TRAP(exp) do { \
		if (!(exp)) { \
		asm volatile(\
		"SVC #"STR(ASSERT_TRAP_SVC_NUMBER)\
		:\
		:\
		: "memory"); \
		} \
	} while (0)
#else /* !__ARM_ARCH_7M__ */
#define ASSERT_WITH_TRAP(exp) do { \
		if (!(exp)) { \
			int *null = NULL; \
			*null = 0; \
		} \
	} while (0)
#endif /* __ARM_ARCH_7M__ */
#if defined(BCMDBG_ASSERT_TRAP)
/* DBG_ASSERT_TRAP causes a trap/exception when an ASSERT fails, instead of calling
 * an assert handler to log the file and line number. This is a memory optimization
 * that eliminates the strings associated with the file/line and the function call
 * overhead associated with invoking the assert handler. The assert location can be
 * determined based upon the program counter displayed by the trap handler.
 */
#define ASSERT(exp) ASSERT_WITH_TRAP(exp)
#else /* !BCMDBG_ASSERT_TRAP */
#ifndef _FILENAME_
#define _FILENAME_ "_FILENAME_ is not defined"
#endif
#define ASSERT(exp) do { \
		if (!(exp)) { \
			hnd_assert(_FILENAME_, __LINE__); \
		} \
	} while (0)
#endif /* BCMDBG_ASSERT_TRAP */
#else
#define	ASSERT(exp)	do {} while (0)
#endif 

#ifdef DONGLEBUILD
#define ROMMABLE_ASSERT(exp) do { \
		if (!(exp)) { \
			int *null = NULL; \
			*null = 0; \
		} \
	} while (0)
#endif /* DONGLEBUILD */

#endif /* _rte_assert_h_ */
