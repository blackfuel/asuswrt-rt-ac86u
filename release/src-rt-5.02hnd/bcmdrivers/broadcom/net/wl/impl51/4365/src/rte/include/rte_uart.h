/*
 * UART h/w and s/w communication low level interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_uart.h 474540 2014-05-01 18:40:15Z $
 */

#ifndef _rte_uart_h_
#define _rte_uart_h_

#include <typedefs.h>
#include <osl.h>
#include <siutils.h>
#include <sbchipc.h>
#include <rte_chipc.h>

/* uart control block */
typedef struct serial_dev {
	osl_t	*osh;
	int	baud_base;
	int	irq;
	uint8	*reg_base;
	uint16	reg_shift;
} serial_dev_t;

/* uart id/idx */
#define RTE_UART_0	0

/* UART interrupts */
#define UART_IER_INTERRUPTS	(UART_IER_ERBFI|UART_IER_ETBEI|UART_IER_PTIME)

#ifdef RTE_UART

/* init/free */
int serial_init_devs(si_t *sih, osl_t *osh);
void serial_free_devs(si_t *sih, osl_t *osh);


/* bind the isr to the uart h/w */
serial_dev_t *serial_bind_dev(si_t *sih, uint id, uint32 ccintmask,
	cc_isr_fn isr, void *isr_ctx, cc_dpc_fn dpc, void *dpc_ctx);


/* ============= in/out ops ============== */

/* serial_in: read a uart register */
static INLINE int
serial_in(serial_dev_t *dev, int offset)
{
	return (int)R_REG(dev->osh, (uint8 *)(dev->reg_base + (offset << dev->reg_shift)));
}

/* serial_out: write a uart register */
static INLINE void
serial_out(serial_dev_t *dev, int offset, int value)
{
	W_REG(dev->osh, (uint8 *)(dev->reg_base + (offset << dev->reg_shift)), value);
}

/* serial_getc: non-blocking, return -1 when there is no input */
static INLINE int
serial_getc(serial_dev_t *dev)
{
	/* Input available? */
	if ((serial_in(dev, UART_LSR) & UART_LSR_RXRDY) == 0)
		return -1;

	/* Get the char */
	return serial_in(dev, UART_RX);
}

/* serial_putc: spinwait for room in UART output FIFO, then write character */
static INLINE void
serial_putc(serial_dev_t *dev, int c)
{
	while ((serial_in(dev, UART_LSR) & UART_LSR_THRE))
		;
	serial_out(dev, UART_TX, c);
}

#else

/* init/free */
#define serial_init_devs(sih, osh) (BCME_OK)
#define serial_free_devs(sih, osh) do {} while (0)

#define serial_in(dev, offset) (int)(-1)
#define serial_out(dev, offset, value) do {} while (0)
#define serial_getc(dev) (int)(-1)
#define serial_putc(dev, c) do {} while (0)

#endif /* RTE_UART */

#endif /* _rte_uart_h_ */
