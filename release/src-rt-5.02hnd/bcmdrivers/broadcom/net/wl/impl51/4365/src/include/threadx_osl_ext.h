/*
 * Threadx OS Support Extension Layer
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id:$
 */


#ifndef _threadx_osl_ext_h_
#define _threadx_osl_ext_h_

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Include Files ---------------------------------------------------- */

#include <tx_api.h>
#include <typedefs.h>


/* ---- Constants and Types ---------------------------------------------- */

/* This is really platform specific and not OS specific. */
#ifndef BWL_THREADX_TICKS_PER_SECOND
#define BWL_THREADX_TICKS_PER_SECOND	1000
#endif

#define OSL_MSEC_TO_TICKS(msec)  ((BWL_THREADX_TICKS_PER_SECOND * (msec)) / 1000)
#define OSL_TICKS_TO_MSEC(ticks) ((1000 * (ticks)) / BWL_THREADX_TICKS_PER_SECOND)

/* Semaphore. */
typedef TX_SEMAPHORE osl_ext_sem_t;
#define OSL_EXT_SEM_DECL(sem)		osl_ext_sem_t  sem;

/* Mutex. */
typedef TX_MUTEX osl_ext_mutex_t;
#define OSL_EXT_MUTEX_DECL(mutex)	osl_ext_mutex_t  mutex;

/* Timer. */
typedef TX_TIMER osl_ext_timer_t;
#define OSL_EXT_TIMER_DECL(timer)	osl_ext_timer_t  timer;

/* Task. */
typedef TX_THREAD osl_ext_task_t;
#define OSL_EXT_TASK_DECL(task)		osl_ext_task_t  task;

/* Queue. */
typedef TX_QUEUE osl_ext_queue_t;
#define OSL_EXT_QUEUE_DECL(queue)	osl_ext_queue_t  queue;

/* Event. */
typedef TX_EVENT_FLAGS_GROUP osl_ext_event_t;
#define OSL_EXT_EVENT_DECL(event)	osl_ext_event_t  event;


/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */


#ifdef __cplusplus
	}
#endif

#endif  /* _threadx_osl_ext_h_  */
