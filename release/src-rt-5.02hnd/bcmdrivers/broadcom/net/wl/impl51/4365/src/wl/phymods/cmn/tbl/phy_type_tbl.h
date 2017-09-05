/*
 * PHYTableInit module internal interface (to PHY specific implementation).
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

#ifndef _phy_type_tbl_h_
#define _phy_type_tbl_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_tbl.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_tbl_ctx_t;

typedef int (*phy_type_tbl_init_fn_t)(phy_type_tbl_ctx_t *ctx);
typedef int (*phy_type_tbl_down_fn_t)(phy_type_tbl_ctx_t *ctx);
typedef bool (*phy_type_tbl_dump_tblfltr_fn_t)(phy_type_tbl_ctx_t *ctx, phy_table_info_t *ti);
typedef bool (*phy_type_tbl_dump_addrfltr_fn_t)(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr);
typedef void (*phy_type_tbl_read_tbl_fn_t)(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr, uint16 *val, uint16 *qval);
typedef int (*phy_type_tbl_dump_fn_t)(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	phy_type_tbl_init_fn_t init;
	phy_type_tbl_down_fn_t down;
	phy_type_tbl_dump_tblfltr_fn_t tblfltr;
	phy_type_tbl_dump_addrfltr_fn_t addrfltr;
	phy_type_tbl_read_tbl_fn_t readtbl;
	phy_type_tbl_dump_fn_t dump[4];
	phy_type_tbl_ctx_t *ctx;
} phy_type_tbl_fns_t;

/*
 * Register/unregister PHY type implementation to the PHY Init module.
 * It returns BCME_XXXX.
 */
int phy_tbl_register_impl(phy_tbl_info_t *ii, phy_type_tbl_fns_t *fns);
void phy_tbl_unregister_impl(phy_tbl_info_t *ii);

/* Dump specified table to buffer */
void phy_tbl_do_dumptbl(phy_tbl_info_t *info, phy_table_info_t *ti, struct bcmstrbuf *b);

#endif /* _phy_type_tbl_h_ */
