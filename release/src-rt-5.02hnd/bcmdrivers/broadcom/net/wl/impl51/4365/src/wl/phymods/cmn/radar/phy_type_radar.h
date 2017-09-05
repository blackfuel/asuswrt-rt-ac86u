/*
 * RadarDetect module internal interface (to PHY specific implementation).
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

#ifndef _phy_type_radar_h_
#define _phy_type_radar_h_

#include <typedefs.h>
#include <phy_radar_api.h>
#include <phy_radar.h>
#include "phy_radar_st.h"

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_radar_ctx_t;

typedef int (*phy_type_radar_init_fn_t)(phy_type_radar_ctx_t *ctx, bool on);
typedef void (*phy_type_radar_update_fn_t)(phy_type_radar_ctx_t *ctx);
typedef void (*phy_type_radar_mode_fn_t)(phy_type_radar_ctx_t *ctx, phy_radar_detect_mode_t mode);
typedef int (*phy_type_radar_run_fn_t)(phy_type_radar_ctx_t *ctx, int PLL_idx, int BW80_80_mode);
typedef int (*phy_type_radar_dump_fn_t)(phy_type_radar_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	phy_type_radar_init_fn_t init;
	phy_type_radar_update_fn_t update;
	phy_type_radar_mode_fn_t mode;
	phy_type_radar_run_fn_t run;
	phy_type_radar_ctx_t *ctx;
} phy_type_radar_fns_t;

/*
 * Register/unregister PHY type implementation to the RadarDetect module.
 * It returns BCME_XXXX.
 */
int phy_radar_register_impl(phy_radar_info_t *ri, phy_type_radar_fns_t *fns);
void phy_radar_unregister_impl(phy_radar_info_t *ri);

#endif /* _phy_type_radar_h_ */
