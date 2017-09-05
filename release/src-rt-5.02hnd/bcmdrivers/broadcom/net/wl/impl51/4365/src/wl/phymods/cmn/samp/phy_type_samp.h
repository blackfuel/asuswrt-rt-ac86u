/*
 * Sample Collect module internal interface (to PHY specific implementations).
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _phy_type_samp_h_
#define _phy_type_samp_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_samp.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_samp_ctx_t;

typedef int (*phy_type_samp_init_fn_t)(phy_type_samp_ctx_t *ctx);
typedef int (*phy_type_samp_dump_fn_t)(phy_type_samp_ctx_t *ctx, struct bcmstrbuf *b);
typedef int (*phy_type_samp_collect_fn_t)(phy_type_samp_ctx_t *ctx,
	wl_samplecollect_args_t *collect, void *b);
typedef int (*phy_type_samp_data_fn_t)(phy_type_samp_ctx_t *ctx,
	wl_sampledata_t *sample_data, void *b);
typedef struct {
	phy_type_samp_collect_fn_t	samp_collect;
	phy_type_samp_data_fn_t		samp_data;
	phy_type_samp_ctx_t		*ctx;
} phy_type_samp_fns_t;

/*
 * Register/unregister PHY type implementation to the MultiPhaseCal module.
 * It returns BCME_XXXX.
 */
int phy_samp_register_impl(phy_samp_info_t *cmn_info, phy_type_samp_fns_t *fns);
void phy_samp_unregister_impl(phy_samp_info_t *cmn_info);

#endif /* _phy_type_samp_h_ */
