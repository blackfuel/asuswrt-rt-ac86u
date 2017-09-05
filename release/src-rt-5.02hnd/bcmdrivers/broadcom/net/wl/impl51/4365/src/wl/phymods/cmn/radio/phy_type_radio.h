/*
 * RADIO control module internal interface (to PHY specific implementations).
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

#ifndef _phy_type_radio_h_
#define _phy_type_radio_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_radio.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_radio_ctx_t;

typedef void (*phy_type_radio_switch_fn_t)(phy_type_radio_ctx_t *ctx, bool on);
typedef void (*phy_type_radio_on_fn_t)(phy_type_radio_ctx_t *ctx);
typedef void (*phy_type_radio_bandx_fn_t)(phy_type_radio_ctx_t *ctx);
typedef void (*phy_type_radio_init_fn_t)(phy_type_radio_ctx_t *ctx);
typedef uint32 (*phy_type_radio_id_fn_t)(phy_type_radio_ctx_t *ctx);
typedef int (*phy_type_radio_dump_fn_t)(phy_type_radio_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	/* switch radio on/off */
	phy_type_radio_switch_fn_t ctrl;
	/* turn radio on */
	phy_type_radio_on_fn_t on;
	/* turn radio off when switching band */
	phy_type_radio_bandx_fn_t bandx;
	/* turn radio off when initializing band */
	phy_type_radio_init_fn_t init;
	/* query radioid */
	phy_type_radio_id_fn_t id;
	/* dump */
	phy_type_radio_dump_fn_t dump;
	/* context */
	phy_type_radio_ctx_t *ctx;
} phy_type_radio_fns_t;

/*
 * Register/unregister PHY type implementation to the RADIO control module.
 *
 * It returns BCME_XXXX.
 */
int phy_radio_register_impl(phy_radio_info_t *ri, phy_type_radio_fns_t *fns);
void phy_radio_unregister_impl(phy_radio_info_t *ri);

#endif /* _phy_type_radio_h_ */
