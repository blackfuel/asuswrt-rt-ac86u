/*
 * This file defines a collection of low-level IE management functions to:
 *
 * 1. manipulate IE management related callback tables
 * 2. register callbacks
 * 3. invoke callbacks
 *
 * The user provides callbacks table's storage.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_ie_mgmt_lib.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_ie_mgmt_lib_h_
#define _wlc_ie_mgmt_lib_h_

#include <typedefs.h>
#include <wlc_types.h>
#include <wlc_ie_mgmt_types.h>

/*
 * 'calc_len'/'build' callback pair callback table entry
 */
typedef struct {
	wlc_iem_calc_fn_t calc;
	wlc_iem_build_fn_t build;
	void *ctx;
} wlc_iem_cbe_t;

/*
 * Register 'calc_len'/'build' callback pair
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build callback table and size.
 * 'calc_fn', 'build_fn', and 'ctx' are the callback functions and context.
 * 'tag' is a value (IE tag or VS IE prio) that the callback pair is registered for.
 */
extern void wlc_ieml_add_build_fn(uint8 *build_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx,
	uint8 tag);

/*
 * Invoke to calculate all IEs' length.
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build pair callback table
 * and size.
 * 'is_tag' indicates if the tag table 'build_tag' contains IE tags or VS IE PRIOs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the 'calc_len' callbacks as is.
 */
extern uint wlc_ieml_calc_len(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm);

/*
 * Invoke to calculate a specific non Vendor Specific IE's length.
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build pair callback table
 * and size.
 * 'is_tag' indicates if the tag table 'build_tag' contains IE tags or VS IE PRIOs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the 'calc_len' callbacks as is.
 * 'tag' is the specific IE's tag.
 */
extern uint wlc_ieml_calc_ie_len(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	uint8 tag, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm);

/*
 * Invoke to write all IEs into buffer
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build pair callback table
 * and size.
 * 'is_tag' indicates if the tag table 'build_tag' contains IE tags or VS IE PRIOs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the 'build' callbacks as is.
 * 'buf' and 'buf_len' are the buffer and its length the IEs are written to.
 */
extern int wlc_ieml_build_frame(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm,
	uint8 *buf, uint buf_len);

/*
 * Invoke to write a specific IE into buffer
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build pair callback table
 * and size.
 * 'is_tag' indicates if the tag table 'build_tag' contains IE tags or VS IE PRIOs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the 'build' callbacks as is.
 * 'buf' and 'buf_len' are the buffer and its length the IEs are written to.
 * 'tag' is the specific IE's tag.
 */
extern int wlc_ieml_build_ie(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	uint8 tag, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm, uint8 *buf, uint buf_len);

/*
 * Sort calc_len/build callback table (build_tag + build_cb) entries based on
 * the tags order in the tag table 'tag'.
 */
extern void wlc_ieml_sort_cbtbl(uint8 *build_tag, wlc_iem_cbe_t *buidd_cb, uint16 build_cnt,
	const uint8 *tag, uint16 cnt);

/*
 * 'parse' callback table entry
 */
typedef struct {
	wlc_iem_parse_fn_t parse;
	void *ctx;
} wlc_iem_pe_t;

/*
 * Register 'parse' callback
 *
 * 'parse_tag', 'parse_cb', and 'parse_cnt' are the callback registration table and size.
 * 'parse_fn' and ctx are callback function and context.
 * 'tag' is a value (IE tag or VS IE id) that the callback is registered for.
 */
extern void wlc_ieml_add_parse_fn(uint8 *parse_tag, wlc_iem_pe_t *parse_cb, uint16 parse_cnt,
	wlc_iem_parse_fn_t parse_fn, void *ctx,
	uint8 tag);

/*
 * Invoke to parse all IEs in buffer.
 *
 * 'parse_tag', 'parse_cb', and 'parse_cnt' are callback registration table and size.
 * 'is_tag' indicates if the tag table 'parse_tag' contains IE tags or VS IE IDs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the parse callbacks as is.
 * 'buf' and 'buf_len' are the buffer containing the IEs and their length in bytes.
 */
extern int wlc_ieml_parse_frame(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *parse_tag, bool is_tag, wlc_iem_pe_t *parse_cb, uint16 parse_cnt,
	wlc_iem_upp_t *upp, wlc_iem_pparm_t *pparm,
	uint8 *buf, uint buf_len);

#endif /* _wlc_ie_mgmt_lib_h_ */
