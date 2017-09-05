/*
 * Console support for RTE.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_cons.h 480442 2014-05-23 20:23:30Z $
 */
#ifndef	_rte_cons_h_
#define	_rte_cons_h_

#include <typedefs.h>
#include <hnd_cons.h>

/* Console command support */
hnd_cons_t *hnd_cons_active_cons_state(void);
extern void hnd_cons_check(void);
extern void hnd_cons_flush(void);
#if defined(RTE_CONS) || defined(BCM_OL_DEV)
extern void process_ccmd(char *line, uint len);
#endif
typedef void (*cons_fun_t)(void *arg, int argc, char *argv[]);
#if defined(RTE_CONS) || defined(BCM_OL_DEV)
extern void hnd_cons_add_cmd(const char *name, cons_fun_t fun, void *arg);
#else
#define hnd_cons_add_cmd(name, fun, arg) { (void)(name); (void)(fun); (void)(arg); }
#endif

/* receive callback to pass UART rx data to higher layers */
typedef void (*uart_rx_cb_t)(void *ctx, uint8 c);
void hndrte_uart_tx(uint8 *buf, int len);
void* hndrte_register_uart_rx_cb(void *ctx, uart_rx_cb_t cb);

#endif /* _rte_cons_h_ */
