/*
 * TxPowerCtrl module internal interface (to PHY specific implementations).
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

#ifndef _phy_type_tpc_h_
#define _phy_type_tpc_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_tpc.h>
#include "phy_tpc_st.h"

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_tpc_ctx_t;

typedef int (*phy_type_tpc_init_fn_t)(phy_type_tpc_ctx_t *ctx);
typedef void (*phy_type_tpc_recalc_fn_t)(phy_type_tpc_ctx_t *ctx);
typedef int (*phy_type_tpc_read_srom_fn_t)(phy_type_tpc_ctx_t *ctx, int bandtype);
typedef void (*phy_type_tpc_check_fn_t)(phy_type_tpc_ctx_t *ctx);
typedef int (*phy_type_tpc_dump_fn_t)(phy_type_tpc_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	/* init module including h/w */
	phy_type_tpc_init_fn_t init;
	/* recalc target txpwr & apply to h/w */
	phy_type_tpc_recalc_fn_t recalc;
	/* read parameters from SROM */
	phy_type_tpc_read_srom_fn_t read_srom;
	/* check txpwr limit */
	phy_type_tpc_check_fn_t check;
	/* context */
	phy_type_tpc_ctx_t *ctx;
} phy_type_tpc_fns_t;

/*
 * Register/unregister PHY type implementation to the TxPowerCtrl module.
 * It returns BCME_XXXX.
 */
int phy_tpc_register_impl(phy_tpc_info_t *mi, phy_type_tpc_fns_t *fns);
void phy_tpc_unregister_impl(phy_tpc_info_t *mi);

#endif /* _phy_type_tpc_h_ */
