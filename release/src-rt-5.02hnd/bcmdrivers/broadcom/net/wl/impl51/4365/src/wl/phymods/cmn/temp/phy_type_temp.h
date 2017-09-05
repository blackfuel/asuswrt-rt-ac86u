/*
 * TEMPerature sense module internal interface (to PHY specific implementations).
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

#ifndef _phy_type_temp_h_
#define _phy_type_temp_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_temp.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_temp_ctx_t;

typedef uint8 (*phy_type_temp_throt_fn_t)(phy_type_temp_ctx_t *ctx);
typedef int (*phy_type_temp_get_fn_t)(phy_type_temp_ctx_t *ctx);
typedef int (*phy_type_temp_dump_fn_t)(phy_type_temp_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	/* temp. throttle */
	phy_type_temp_throt_fn_t throt;
	/* get current temp. */
	phy_type_temp_get_fn_t get;
	/* context */
	phy_type_temp_ctx_t *ctx;
} phy_type_temp_fns_t;

/*
 * Register/unregister PHY type implementation to the TEMPerature sense module.
 * It returns BCME_XXXX.
 */
int phy_temp_register_impl(phy_temp_info_t *mi, phy_type_temp_fns_t *fns);
void phy_temp_unregister_impl(phy_temp_info_t *mi);

#endif /* _phy_type_temp_h_ */
