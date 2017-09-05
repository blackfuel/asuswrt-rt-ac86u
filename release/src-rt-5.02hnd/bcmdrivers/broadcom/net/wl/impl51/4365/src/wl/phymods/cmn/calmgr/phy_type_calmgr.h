/*
 * Calibration Management module internal interface (to PHY specific implementation).
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

#ifndef _phy_type_calmgr_h_
#define _phy_type_calmgr_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_calmgr.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_calmgr_ctx_t;

typedef int (*phy_type_calmgr_prepare_fn_t)(phy_type_calmgr_ctx_t *ctx);
typedef void (*phy_type_calmgr_cleanup_fn_t)(phy_type_calmgr_ctx_t *ctx);
typedef struct {
	phy_type_calmgr_prepare_fn_t prepare;
	phy_type_calmgr_cleanup_fn_t cleanup;
	phy_type_calmgr_ctx_t *ctx;
} phy_type_calmgr_fns_t;

/*
 * Register/unregister PHY type implementation to the common layer.
 * It returns BCME_XXXX.
 */
int phy_calmgr_register_impl(phy_calmgr_info_t *ci, phy_type_calmgr_fns_t *fns);
void phy_calmgr_unregister_impl(phy_calmgr_info_t *ci);

#endif /* _phy_type_calmgr_h_ */
