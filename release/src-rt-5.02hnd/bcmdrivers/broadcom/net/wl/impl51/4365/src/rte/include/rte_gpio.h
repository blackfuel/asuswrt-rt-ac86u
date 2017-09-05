/*
 * HND GPIO control software interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: $
 */

#ifndef _rte_gpio_h_
#define _rte_gpio_h_

typedef struct rte_gpioh rte_gpioh_t;

typedef void (*gpio_handler_t)(uint32 stat, void *arg);

#ifdef WLGPIOHLR
extern rte_gpioh_t * rte_gpio_handler_register(uint32 event,
	bool level, gpio_handler_t cb, void *arg);
extern void rte_gpio_handler_unregister(rte_gpioh_t *gi);
#else
#define rte_gpio_handler_register(event, level, cb, arg) ((rte_gpioh_t *)(((int)cb ^ (int)cb)))
#define rte_gpio_handler_unregister(gi) do {} while (0)
#endif /* WLGPIOHLR */
#endif /* _rte_gpio_h_ */
