/*
 * NOISEmeasure module internal interface (to PHY specific implementations).
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

#ifndef _phy_type_noise_h_
#define _phy_type_noise_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_noise.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_noise_ctx_t;

typedef void (*phy_type_noise_mode_fn_t)(phy_type_noise_ctx_t *ctx, int mode, bool init);
typedef void (*phy_type_noise_reset_fn_t)(phy_type_noise_ctx_t *ctx);
typedef int (*phy_type_noise_get_fixed_fn_t)(phy_type_noise_ctx_t *ctx);
typedef bool (*phy_type_noise_start_fn_t)(phy_type_noise_ctx_t *ctx, uint8 reason);
typedef void (*phy_type_noise_stop_fn_t)(phy_type_noise_ctx_t *ctx);
typedef int (*phy_type_noise_dump_fn_t)(phy_type_noise_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	phy_type_noise_mode_fn_t mode;	/* From Interference */
	phy_type_noise_reset_fn_t reset; /* From Interference */
	/* get fixed noise */
	phy_type_noise_get_fixed_fn_t get_fixed;
	/* start measure request */
	phy_type_noise_start_fn_t start;
	/* stop measure */
	phy_type_noise_stop_fn_t stop;
	/* context */
	phy_type_noise_ctx_t *ctx;
	/* dump */
	phy_type_noise_dump_fn_t dump;
} phy_type_noise_fns_t;

/*
 * Register/unregister PHY type implementation to the NOISEmeasure module.
 * It returns BCME_XXXX.
 */
int phy_noise_register_impl(phy_noise_info_t *mi, phy_type_noise_fns_t *fns);
void phy_noise_unregister_impl(phy_noise_info_t *mi);

#endif /* _phy_type_noise_h_ */
