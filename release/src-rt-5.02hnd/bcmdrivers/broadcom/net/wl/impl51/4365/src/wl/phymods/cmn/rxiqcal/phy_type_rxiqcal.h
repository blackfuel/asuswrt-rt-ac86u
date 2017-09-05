/*
 * RXIQ CAL module internal interface (to PHY specific implementations).
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

#ifndef _phy_type_rxiqcal_h_
#define _phy_type_rxiqcal_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_rxiqcal.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_rxiqcal_ctx_t;

typedef int (*phy_type_rxiqcal_init_fn_t)(phy_type_rxiqcal_ctx_t *ctx);
typedef int (*phy_type_rxiqcal_dump_fn_t)(phy_type_rxiqcal_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	phy_type_rxiqcal_ctx_t *ctx;
} phy_type_rxiqcal_fns_t;

/*
 * Register/unregister PHY type implementation to the RXIQCAL module.
 * It returns BCME_XXXX.
 */
int phy_rxiqcal_register_impl(phy_rxiqcal_info_t *cmn_info, phy_type_rxiqcal_fns_t *fns);
void phy_rxiqcal_unregister_impl(phy_rxiqcal_info_t *cmn_info);

#endif /* _phy_type_rxiqcal_h_ */
