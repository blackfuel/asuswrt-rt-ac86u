/*
 * INIT/DOWN control module internal interface (to other modules)
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _phy_init_h_
#define _phy_init_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_init_info phy_init_info_t;

/* ******** interface for Core module ******** */

/* attach/detach */
phy_init_info_t *phy_init_attach(phy_info_t *pi);
void phy_init_detach(phy_init_info_t *);

/* invoke init callbacks */
int phy_init_invoke_init_fns(phy_init_info_t *ii);
#ifndef BCMNODOWN
/* invoke down callbacks */
void phy_init_invoke_down_fns(phy_init_info_t *ii);
#endif

/* ******** interface for other modules ******** */

/* Add a callback fn in the INIT/DOWN sequence.
 *
 * The callback fn returns BCME_XXXX.
 * The INIT sequence will be aborted when a callback returns an error.
 * The DOWN sequence will ignore callback returns.
 */
typedef void phy_init_ctx_t;
typedef int (*phy_init_fn_t)(phy_init_ctx_t *ctx);
/*
 * INIT/DOWN callbacks execution order.
 * Note: Keep the enums between 0 and 255!
 */
typedef enum phy_init_order {
	/* Insert new INIT callbacks at appropriate place. */
	PHY_INIT_START = 0,
	PHY_INIT_CACHE,		/* CACHE (s/w) */
	PHY_INIT_RSSI,		/* RSSICompute */
	PHY_INIT_ANA,		/* ANAcore */
	PHY_INIT_CHBW,		/* CHannelBandWidth */
	PHY_INIT_PHYIMPL,	/* PHYIMPLementation */
	PHY_INIT_NOISERST,	/* NOISEReSeT */
	PHY_INIT_RADIO,		/* RADIO */
	PHY_INIT_PHYTBL,	/* phyTaBLes */
	PHY_INIT_TPC,		/* TxPowerControl */
	PHY_INIT_RADAR,		/* RADARdetection */
	PHY_INIT_ANTDIV,	/* ANTennaDIVersity */
	PHY_INIT_NOISE,		/* NOISE */
	PHY_INIT_CHSPEC,	/* CHannelSPEC */
	PHY_INIT_TXIQLOCAL, /* Tx IQLO Cal */
	PHY_INIT_RXIQCAL,	/* Rx IQ Cal */
	PHY_INIT_PAPDCAL,	/* PAPD IQ Cal */
	PHY_INIT_VCOCAL,	/* VCO IQ Cal */

	/* Insert new DOWN callbacks at appropriate place. */
	PHY_DOWN_START = 0,
	PHY_DOWN_PHYTBL
} phy_init_order_t;

/* Add an init callback entry. Returns BCME_XXXX. */
int phy_init_add_init_fn(phy_init_info_t *ii,
	phy_init_fn_t fn, phy_init_ctx_t *ctx,
	phy_init_order_t order);
#ifndef BCMNODOWN
/* Add a down callback entry. Returns BCME_XXXX. */
int phy_init_add_down_fn(phy_init_info_t *ii,
	phy_init_fn_t fn, phy_init_ctx_t *ctx,
	phy_init_order_t order);
#endif

#endif /* _phy_init_h_ */
