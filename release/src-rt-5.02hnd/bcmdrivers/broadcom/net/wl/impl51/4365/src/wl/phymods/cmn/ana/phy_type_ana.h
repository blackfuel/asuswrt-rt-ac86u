/*
 * ANAcore module internal interface (to PHY specific implementations).
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

#ifndef _phy_type_ana_h_
#define _phy_type_ana_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_ana.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_ana_ctx_t;

typedef int (*phy_type_ana_ctrl_fn_t)(phy_type_ana_ctx_t *ctx, bool on);
typedef void (*phy_type_ana_reset_fn_t)(phy_type_ana_ctx_t *ctx);
typedef int (*phy_type_ana_dump_fn_t)(phy_type_ana_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	/* init h/w */
	phy_type_ana_ctrl_fn_t ctrl;
	/* reset h/w */
	phy_type_ana_reset_fn_t reset;
	/* context */
	phy_type_ana_ctx_t *ctx;
} phy_type_ana_fns_t;

/*
 * Register/unregister PHY type implementation to the ANAcore module.
 *
 * It returns BCME_XXXX.
 */
int phy_ana_register_impl(phy_ana_info_t *ani, phy_type_ana_fns_t *fns);
void phy_ana_unregister_impl(phy_ana_info_t *ani);

#endif /* _phy_type_ana_h_ */
