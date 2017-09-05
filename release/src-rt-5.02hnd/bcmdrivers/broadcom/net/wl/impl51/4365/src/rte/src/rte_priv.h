/*
 * RTE private interfaces between different modules in RTE.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_priv.h 597803 2015-11-06 08:46:00Z $
 */

#ifndef _hnd_rte_priv_h_
#define _hnd_rte_priv_h_

#include <typedefs.h>
#include <siutils.h>
#include <osl_decl.h>

#include <rte_trap.h>

/* Forward declaration */
extern si_t *hnd_sih;		/* Chip backplane handle */
extern osl_t *hnd_osh;		/* Chip backplane osh */

extern uint32 c0counts_per_us;
extern uint32 c0counts_per_ms;

/* ========================== system ========================== */
void hnd_idle_init(si_t *sih);
/* Each CPU/Arch must implement this interface - idle loop */
void hnd_idle_loop(si_t *sih);
/* Each CPU/Arch must implement this interface - wait for interrupt */
void hnd_wait_irq(si_t *sih);

/* ======================= trap ======================= */
void hnd_trap_init(void);
void hnd_trap_common(trap_t *tr);
void hnd_print_stack(uint32 sp);

/* ================ CPU ================= */
void hnd_cpu_init(si_t *sih);

int hnd_cpu_stats_init(si_t *sih);
void hnd_cpu_stats_upd(uint32 start_time);
int hnd_cpu_deadman_init(si_t *sih);
uint32 hnd_cpu_gtimer_trap_validation(void);
void hnd_cpu_gtimer_fiq_hdl(void);

#define LOADAVG_HZ	100		/* Sample Hz */

extern uint32 loadavg_time;

void hnd_cpu_load_avg(uint epc, uint lr, uint sp);

/* ========================== time ========================== */
uint32 hnd_update_now(void);
uint32 hnd_update_now_us(void);

/* ========================== timer ========================== */
/* Each CPU/Arch must implement this interface - init possible global states */
int hnd_timer_init(si_t *sih);
/* Each CPU/Arch must implement this interface - program the h/w */
void hnd_set_irq_timer(uint ms);
/* Each CPU/Arch must implement this interface - ack h/w interrupt */
void hnd_ack_irq_timer(void);
/* Each CPU/Arch must implement this interface - run timeouts */
void hnd_run_timeouts(void);
/* Each CPU/Arch must implement this interface - register possible command line proc */
void hnd_timer_cli_init(void);

/* ======================= debug ===================== */
void hnd_stack_prot(void *stack_top);

/* ===================== debug ====================== */
void hnd_debug_info_init(void);

/* ======================= cache ===================== */
#ifdef RTE_CACHED
void hnd_caches_init(si_t *sih);
#endif

/* accessor functions */
extern si_t* get_hnd_sih(void);
extern void set_hnd_sih(si_t *val);

#endif /* _hnd_rte_priv_h_ */
