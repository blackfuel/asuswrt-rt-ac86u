/*
 * UART h/w and s/w communication low level routine.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_uart.c 501882 2014-09-11 01:59:33Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <sbchipc.h>
#include <hndchipc.h>
#include <rte_chipc.h>
#include <rte_uart.h>

#ifdef RTE_UART

/* maximum uart devices to support */
#ifndef RTE_UART_MAX
#define RTE_UART_MAX 1
#endif

/* control blocks */
static struct {
	serial_dev_t *dev;	/* 'non-null when h/w is found */
	bool taken;	/* TRUE when h/w is bind to an user */
} serial_dev[RTE_UART_MAX];
static uint serial_devs = 0;

/* serial_add: callback to initialize the uart structure */
static void
BCMATTACHFN(serial_add)(si_t *sih, void *regs, uint irq, uint baud_base, uint reg_shift)
{
	osl_t *osh = si_osh(sih);
	serial_dev_t *dev;
	int quot;

	if (serial_devs >= RTE_UART_MAX)
		return;

	if ((dev = MALLOCZ(osh, sizeof(serial_dev_t))) == NULL)
		goto fail;

	dev->reg_base = regs;
	dev->irq = irq;
	dev->baud_base = baud_base / 16;
	dev->reg_shift = reg_shift;

	dev->osh = osh;

	serial_dev[serial_devs].dev = dev;
	serial_dev[serial_devs].taken = FALSE;
	serial_devs ++;

	/* Set baud and 8N1 */
	quot = (dev->baud_base + 57600) / 115200;
	serial_out(dev, UART_LCR, UART_LCR_DLAB);
	serial_out(dev, UART_DLL, quot & 0xff);
	serial_out(dev, UART_DLM, quot >> 8);
	serial_out(dev, UART_LCR, UART_LCR_WLEN8);

	/* enable interrupts for rx data */
	serial_out(dev, UART_MCR, UART_MCR_OUT2);

	/* Enable PTIME (Programmable THRE Interrupt mode)
	 * enabled via the Interrupt Enable Register (IER[7])
	 * FIFOs need to be enabled (FCR[0] == 1)
	 * threshold level is programmed into FCR[5:4].
	 * The available empty thresholds are: empty, 2, 1/4 and 1/2.
	 * currently we use empty.
	 */
	serial_out(dev, UART_IER, UART_IER_ERBFI|UART_IER_ETBEI|UART_IER_PTIME);

	/* enable FIFOs */
	serial_out(dev, UART_FCR, UART_FCR_FIFO_ENABLE);

	/* According to the Synopsys website: "the serial clock
	 * modules must have time to see new register values
	 * and reset their respective state machines. This
	 * total time is guaranteed to be no more than
	 * (2 * baud divisor * 16) clock cycles of the slower
	 * of the two system clocks. No data should be transmitted
	 * or received before this maximum time expires."
	 */
	OSL_DELAY(1000);

	return;

fail:
	return;
}

/* init/free interface */
int
BCMATTACHFN(serial_init_devs)(si_t *sih, osl_t *osh)
{
	/* probe chipc for uarts */
	if (!ISSIM_ENAB(sih))
		si_serial_init(sih, serial_add);
	return BCME_OK;
}

void
BCMATTACHFN(serial_free_devs)(si_t *sih, osl_t *osh)
{
	while (serial_devs > 0) {
		serial_devs --;
		MFREE(osh, serial_dev[serial_devs].dev, sizeof(serial_dev_t));
	}
}

/* bind the isr and the h/w */
serial_dev_t *
BCMATTACHFN(serial_bind_dev)(si_t *sih, uint id, uint32 ccintmask,
	cc_isr_fn isr, void *isr_ctx, cc_dpc_fn dpc, void *dpc_ctx)
{
	if (id < serial_devs && !serial_dev[id].taken) {
		si_cc_register_isr(sih, isr, ccintmask, isr_ctx);
#ifdef THREAD_SUPPORT
		si_cc_register_dpc(sih, dpc, ccintmask, dpc_ctx);
#endif	/* THREAD_SUPPORT */
		serial_dev[id].taken = TRUE;
		return serial_dev[id].dev;
	}
	return NULL;
}

#endif /* RTE_UART */
