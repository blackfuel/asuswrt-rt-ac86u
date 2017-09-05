/*
 * RSSI Compute module internal interface (to PHY specific implementation).
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

#ifndef _phy_type_rssi_h_
#define _phy_type_rssi_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <d11.h>
#include <phy_rssi.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_rssi_ctx_t;

typedef void (*phy_type_rssi_compute_fn_t)(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh);
typedef void (*phy_type_rssi_init_gain_err_fn_t)(phy_type_rssi_ctx_t *ctx);
typedef int (*phy_type_rssi_dump_fn_t)(phy_type_rssi_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	phy_type_rssi_compute_fn_t compute;
	phy_type_rssi_init_gain_err_fn_t init_gain_err;
	phy_type_rssi_dump_fn_t dump;
	phy_type_rssi_ctx_t *ctx;
} phy_type_rssi_fns_t;

/*
 * Register/unregister PHY type implementation to the common of the RSSI Compute module.
 * It returns BCME_XXXX.
 */
int phy_rssi_register_impl(phy_rssi_info_t *ri, phy_type_rssi_fns_t *fns);
void phy_rssi_unregister_impl(phy_rssi_info_t *ri);

/*
 * Enable RSSI moving average algorithm.
 * It returns BCME_XXXX.
 */
int phy_rssi_enable_ma(phy_rssi_info_t *ri, bool enab);

#endif /* _phy_type_rssi_h_ */
