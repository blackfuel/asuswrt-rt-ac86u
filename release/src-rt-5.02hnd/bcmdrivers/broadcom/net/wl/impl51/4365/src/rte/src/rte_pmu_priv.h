/*
 * RTE PMU support.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_pmu_priv.h 563300 2015-06-12 09:05:12Z $
 */

#ifndef _rte_pmu_priv_h_
#define _rte_pmu_priv_h_

#include <typedefs.h>
#include <siutils.h>

/* accumulate tick count in memory */
uint32 hnd_pmu_accu_tick(uint32 diff);

/* get h/w tick count */
uint32 hnd_pmu_get_tick(void);

/* program the h/w timer */
void hnd_pmu_set_timer(uint32 ticks);
void hnd_pmu_ack_timer(void);

/* covnert msec to h/w tick */
uint32 hnd_pmu_ms2tick(uint32 ms);

/* init */
void hnd_pmu_init(si_t *sih);

void hnd_pmu_set_gtimer_period(void);
#endif /* _rte_pmu_priv_h_ */
