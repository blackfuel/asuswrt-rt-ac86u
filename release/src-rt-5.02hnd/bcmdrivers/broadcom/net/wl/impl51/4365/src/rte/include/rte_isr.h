/*
 * OS independent ISR functions for ISRs or DPCs.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_isr.h 480442 2014-05-23 20:23:30Z $
 */

#ifndef	_RTE_ISR_H
#define	_RTE_ISR_H

#include <typedefs.h>

typedef void (*hnd_isr_t)(void *cbdata);
typedef void (*hnd_dpc_t)(void *cbdata);

/* register isr */
int hnd_isr_register(uint irq, uint coreid, uint unit, hnd_isr_t isr, void *cbdata, uint bus);

/* register isr that doesn't belong to any core */
int hnd_isr_register_n(uint irq, uint isr_num, hnd_isr_t isr, void *cbdata);

/* register dpc */
int hnd_dpc_register(uint irq, uint coreid, uint unit, hnd_dpc_t dpc, void *cbdata, uint bus);

#endif	/* _RTE_ISR_H */
