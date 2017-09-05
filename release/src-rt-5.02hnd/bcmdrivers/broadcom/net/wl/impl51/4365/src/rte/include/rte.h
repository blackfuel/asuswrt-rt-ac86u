/*
 * HND RTE misc interfaces.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte.h 480966 2014-05-27 22:13:13Z $
 */

#ifndef _rte_h_
#define _rte_h_

#include <typedefs.h>
#include <siutils.h>

#include <rte_trap.h>
#include <rte_timer.h>

#define RTE_WORD_SIZE 4 /* Word size in rte */

/* ========================== CPU ========================= */
/* Each CPU/Arch must implement this interface */
int32 hnd_cpu_clockratio(si_t *sih, uint8 div);

/* ========================== timer ========================== */
int hnd_schedule_work(void *ctx, void *data,
	hnd_timer_mainfn_t taskfn, int delay);
/* Each CPU/Arch must implement this interface - suppress any further timer requests */
void hnd_suspend_timer(void);
/* Each CPU/Arch must implement this interface - resume timers */
void hnd_resume_timer(void);

/* ========================== system ========================== */
void hnd_enable_interrupts(void);
void hnd_disable_interrupts(void);
si_t *hnd_init(void);
#ifdef ATE_BUILD
#define hnd_poll(sih) wl_ate_cmd_proc()
void wl_ate_cmd_proc(void);
#else /* !ATE_BUILD */
void hnd_poll(si_t *sih);
#endif
/* Each OS implement this interface */
void hnd_idle(si_t *sih);

/* ======================= debug ===================== */
void hnd_memtrace_enab(bool on);

#ifdef	BCMDBG
#if defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
#define	BCMDBG_TRACE(x)		__watermark = (x)
#else
#define	BCMDBG_TRACE(x)
#endif	/* !__arm__ && !__thumb__ && !__thumb2__ */
#else
#define	BCMDBG_TRACE(x)
#endif	/* BCMDBG */

extern volatile uint __watermark;

/* ============================ misc =========================== */
void hnd_unimpl(void);

#endif /* _rte_h_ */
