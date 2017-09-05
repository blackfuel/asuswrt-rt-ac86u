/*
 * ANTennaDIVersity module internal interface (to PHY specific implementations).
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

#ifndef _phy_type_antdiv_h_
#define _phy_type_antdiv_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_antdiv.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_antdiv_ctx_t;

#ifdef WLC_SW_DIVERSITY
typedef void (*phy_type_antdiv_init_fn_t)(phy_type_antdiv_ctx_t *ctx);
typedef void (*phy_type_antdiv_set_sw_fn_t)(phy_type_antdiv_ctx_t *ctx, uint8 ant);
typedef uint8 (*phy_type_antdiv_get_sw_fn_t)(phy_type_antdiv_ctx_t *ctx);
#endif
typedef void (*phy_type_antdiv_set_rx_fn_t)(phy_type_antdiv_ctx_t *ctx, uint8 ant);
typedef int (*phy_type_antdiv_dump_fn_t)(phy_type_antdiv_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
#ifdef WLC_SW_DIVERSITY
	phy_type_antdiv_init_fn_t init;
	phy_type_antdiv_set_sw_fn_t setsw;
	phy_type_antdiv_get_sw_fn_t getsw;
#endif
	phy_type_antdiv_set_rx_fn_t setrx;
	phy_type_antdiv_ctx_t *ctx;
} phy_type_antdiv_fns_t;

/*
 * Register/unregister PHY type implementation to the ANTennaDIVersity module.
 *
 * It returns BCME_XXXX.
 */
int phy_antdiv_register_impl(phy_antdiv_info_t *mi, phy_type_antdiv_fns_t *fns);
void phy_antdiv_unregister_impl(phy_antdiv_info_t *di);

#endif /* _phy_type_antdiv_h_ */
