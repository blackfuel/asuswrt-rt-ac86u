/*
 * MU-MIMO phy module internal interface (to PHY specific implementation).
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: phy_type_mu.h 525162 2015-01-09 02:07:08Z $
 */

#ifndef _phy_type_mu_h_
#define _phy_type_mu_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <d11.h>
#include <phy_mu.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_mu_ctx_t;

typedef int (*phy_type_mu_group_set_fn_t)(phy_type_mu_ctx_t *ctx,
	uint16 mu_group, uint8 user_pos, uint8 is_member);
typedef int (*phy_type_mu_dump_fn_t)(phy_type_mu_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	phy_type_mu_group_set_fn_t mu_group_set;
	phy_type_mu_dump_fn_t dump;
	phy_type_mu_ctx_t *ctx;
} phy_type_mu_fns_t;

/*
 * Register/unregister PHY type implementation.
 * Register returns BCME_XXXX.
 */
int phy_mu_register_impl(phy_mu_info_t *ri, phy_type_mu_fns_t *fns);
void phy_mu_unregister_impl(phy_mu_info_t *ri);

#endif /* _phy_type_mu_h_ */
