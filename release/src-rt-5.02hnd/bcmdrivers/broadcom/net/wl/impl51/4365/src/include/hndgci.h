/*
 * HND GCI core software interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * APIs to access Plain UART mode functionality of GCI
 *
 * $Id: $
 */

#ifndef _hndgci_h_
#define _hndgci_h_

/* SECI Block operating modes */
#define HND_GCI_PLAIN_UART_MODE		0
#define HND_GCI_SECI_MODE		1
#define HND_GCI_BTSIG_MODE		2
#define HND_GCI_PURE_GPIO_MODE		3

/* Status/Error messages */
#define	HND_GCI_SUCCESS		0
#define	HND_GCI_UNINITIALIZED	-1
#define	HND_GCI_INVALID_PARAM	-2
#define	HND_GCI_INVALID_BUFFER	-3
#define	HND_GCI_TX_INPROGRESS	-4
#define	HND_GCI_NO_SUPPORT	-5
#define	HND_GCI_NO_MEMORY	-6

/* baudrate index */
enum _baudrate_idx {
	GCI_UART_BR_115200,
	GCI_UART_MAX_BR_IDX
};

typedef void (*hndgci_cb_t)(char *buf, uint32 len);
typedef	void (*rx_cb_t)(void *ctx, char *buf, int len);

typedef struct _hndgci_cbs_t {
	void *context;
	void (*rx_cb)(void *ctx, char *buf, int len);
	void (*tx_status)(void *ctx, int status);
} hndgci_cbs_t;

extern int hndgci_init(si_t *sih, osl_t *osh, uint8 seci_mode, uint8 baudrate_idx);
extern void hndgci_deinit(si_t *sih);
extern int hndgci_uart_tx(si_t *sih, void *buf, int len);
extern int hndgci_uart_config_rx_complete(char delim, int len, int timeout,
	rx_cb_t rx_cb, void *ctx);
extern void hndgci_handler_process(uint32 stat, si_t *sih);

/* debug function */
extern int hndgci_uart_rx(si_t *sih, char *buf, int count, int timeout);

#endif /* _hndgci_h_ */
