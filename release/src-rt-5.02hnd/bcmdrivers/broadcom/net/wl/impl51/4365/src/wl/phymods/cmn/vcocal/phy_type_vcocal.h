/*
 * VCO CAL module internal interface (to PHY specific implementations).
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

#ifndef _phy_type_vcocal_h_
#define _phy_type_vcocal_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_vcocal.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_vcocal_ctx_t;

typedef int (*phy_type_vcocal_init_fn_t)(phy_type_vcocal_ctx_t *ctx);
typedef int (*phy_type_vcocal_dump_fn_t)(phy_type_vcocal_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	phy_type_vcocal_ctx_t *ctx;
} phy_type_vcocal_fns_t;

/*
 * Register/unregister PHY type implementation to the vcocal module.
 * It returns BCME_XXXX.
 */
int phy_vcocal_register_impl(phy_vcocal_info_t *cmn_info, phy_type_vcocal_fns_t *fns);
void phy_vcocal_unregister_impl(phy_vcocal_info_t *cmn_info);

#endif /* _phy_type_vcocal_h_ */
