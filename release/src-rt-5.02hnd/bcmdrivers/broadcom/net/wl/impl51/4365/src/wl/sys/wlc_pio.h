/*
 * HND d11 PIO module
 * Broadcom 802.11abg Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_pio.h 594707 2015-10-23 08:16:44Z $
 */
#ifndef _wlc_pio_h_
#define _wlc_pio_h_

typedef struct pios	pio_t;

/* pio callback functions */
typedef int (*pio_detach_t)(pio_t *pio);
typedef int (*pio_reset_t)(pio_t *pio);
typedef int (*pio_init_t)(pio_t *pio);
typedef int (*pio_txsuspend_t)(pio_t *pio);
typedef bool (*pio_txsuspended_t)(pio_t *pio);
typedef int (*pio_txresume_t)(pio_t *pio);
typedef bool (*pio_txavailable_t)(pio_t *pio, uint len, uint nfrags);
typedef bool (*pio_rxfrmrdy_t)(pio_t *pio);
typedef int (*pio_cntupd_t)(pio_t *pio, uint len);
typedef void (*pio_dump_t)(pio_t *pio, struct bcmstrbuf *b);
typedef int (*pio_txfifodepthset_t)(pio_t *pio, uint len);
typedef uint (*pio_txfifodepthget_t)(pio_t *pio);
typedef void* (*pio_getnexttxp_t)(pio_t *pio);
typedef void* (*pio_txreclaim_t)(pio_t *pio);
typedef int (*pio_tx_t)(pio_t *pio, void *p0);
typedef void* (*pio_rx_t)(pio_t *pio);


/* pio opsvec */
typedef struct piof_s {
	pio_reset_t		reset;
	pio_init_t		init;
	pio_rx_t		rx;
	pio_tx_t		tx;
	pio_txsuspend_t		txsuspend;
	pio_txsuspended_t	txsuspended;
	pio_txresume_t		txresume;
	pio_rxfrmrdy_t		rxfrmrdy;
	pio_txavailable_t	txavailable;
	pio_cntupd_t		cntupd;
	pio_dump_t		dump;
	pio_txfifodepthset_t	txdepthset;
	pio_txfifodepthget_t	txdepthget;
	pio_getnexttxp_t	nexttxp;
	pio_txreclaim_t		txreclaim;
	pio_detach_t		detach;
} piof_t;

struct pios {
	piof_t	pio_fn;		/* pio mode function pointers */
};

/* forward declaration */
struct wlc_info;

#ifdef WLPIO
extern pio_t* wlc_pio_attach(wlc_pub_t *pub, struct wlc_info *wlc, uint fifo);
/* below function needs to be called after pio_attach to avoid overriden by default */
extern void wlc_pio_register_fn(pio_t *pio, piof_t *fn);
#define	wlc_pio_detach(pio)		((pio)->pio_fn.detach(pio))
#define	wlc_pio_init(pio)		((pio)->pio_fn.init(pio))
#define	wlc_pio_reset(pio)		((pio)->pio_fn.reset(pio))
#define	wlc_pio_txsuspend(pio)		((pio)->pio_fn.txsuspend(pio))
#define	wlc_pio_txsuspended(pio)	((pio)->pio_fn.txsuspended(pio))
#define	wlc_pio_txresume(pio)		((pio)->pio_fn.txresume(pio))
#define	wlc_pio_rxfrmrdy(pio)		((pio)->pio_fn.rxfrmrdy(pio))
#define	wlc_pio_cntupd(pio, len)	((pio)->pio_fn.cntupd(pio, len))
#define	wlc_pio_dump(pio, b)		((pio)->pio_fn.dump(pio, b))
#define	wlc_pio_txdepthset(pio, len)	((pio)->pio_fn.txdepthset(pio, len))
#define	wlc_pio_txdepthget(pio)		((pio)->pio_fn.txdepthget(pio))
#define	wlc_pio_getnexttxp(pio)		((pio)->pio_fn.nexttxp(pio))
#define	wlc_pio_txreclaim(pio)		((pio)->pio_fn.txreclaim(pio))
#define	wlc_pio_txavailable(pio, len, frags)	((pio)->pio_fn.txavailable(pio, len, frags))
#define	wlc_pio_tx(pio, p0)		((pio)->pio_fn.tx(pio, p0))
#define	wlc_pio_rx(pio)			((pio)->pio_fn.rx(pio))

#else

#define wlc_pio_attach(pub, wlc, fifo)	(NULL)
#define wlc_pioregs_offset(wlc, txrx, fifonum) (NULL)
#define wlc_pio_register_fn(pio, fn)	ASSERT(0)

#define	wlc_pio_detach(pio)		ASSERT(0)
#define	wlc_pio_init(pio)		ASSERT(0)
#define	wlc_pio_reset(pio)		ASSERT(0)
#define	wlc_pio_txsuspend(pio)		ASSERT(0)
#define	wlc_pio_txsuspended(pio)	(FALSE)
#define	wlc_pio_txresume(pio)		ASSERT(0)
#define	wlc_pio_txavailable(pio, l, f)	(FALSE)
#define	wlc_pio_rxfrmrdy(pio)		(FALSE)
#define	wlc_pio_cntupd(pio, len)	ASSERT(0)
#define	wlc_pio_dump(pio, b)		ASSERT(0)
#define	wlc_pio_txdepthset(pio, len)	ASSERT(0)
#define	wlc_pio_txdepthget(pio)		(999)
#define	wlc_pio_getnexttxp(pio)		(NULL)
#define	wlc_pio_txreclaim(pio)		ASSERT(0)
#define	wlc_pio_tx(pio, p0)		ASSERT(0)
#define	wlc_pio_rx(pio)			(NULL)
#endif	/* WLPIO */

#endif /* _wlc_pio_h_ */
