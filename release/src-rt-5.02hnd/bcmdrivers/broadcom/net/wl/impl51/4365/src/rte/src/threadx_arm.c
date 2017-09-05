/*
 * ThreadX ARM port specific routines.
 * Contains functions that are specific to both ARM processor *and* RTE RTOS.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: threadx_arm.c 591227 2015-10-07 09:01:53Z $
 */

#include <typedefs.h>
#include <hnd_debug.h>
#include <hnd_trap.h>
#include <rte_assert.h>
#include "rte_priv.h"
#include <bcmstdlib.h>
#include <bcmstdlib_ext.h>

/**
 * this handler is called by low level assembly code. It should withstand reentrancy. CM3 nor CR4
 * make use of the FVIC controller in the ARM subsystem. An implication is that a 'normal' interrupt
 * can be preempted by a 'fast interrupt', but not by another 'normal' interrupt.
 */
void
hnd_trap_handler(trap_t *tr)
{
#ifdef HND_PRINTF_THREAD_SAFE
	in_trap_handler ++;
#endif	/* HND_PRINTF_THREAD_SAFE */

	/* Save the trap pointer */
	get_hnd_debug_info()->trap_ptr = tr;

	/* Common trap handling */
	hnd_trap_common(tr);

	/* Halt processing without forcing a trap since we are already in the trap handler. */
	if ((get_g_assert_type() == 0) || (get_g_assert_type() == 2)) {
		hnd_die_no_trap();
	}

#ifdef HND_PRINTF_THREAD_SAFE
	in_trap_handler --;
#endif	/* HND_PRINTF_THREAD_SAFE */
} /* hnd_trap_handler */
