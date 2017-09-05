/*
 * IE management module interface - this is a top-level function
 * managing IE build and/or parse in all frames other than those
 * listed in wlc_ie_mgmt.c (e.g. it can be used in Action frame or
 * or Wrapper IE build/parse).
 *
 * It is used to decouple server modules who need to deal with
 * IEs owned by other features/modules and client modules who
 * 'own' these IEs.
 *
 * These steps briefly describe what modules should do:
 * - servers create IE registries (at attach time)
 * - clients register IEs' build and/or parse callbacks
 *   with the registries
 * - servers invoke clients' callbacks.
 * - clients build/parse their IEs
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_ie_reg.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_ie_reg_h_
#define _wlc_ie_reg_h_

#include <typedefs.h>
#include <wlc_types.h>
#include <wlc_ie_mgmt_types.h>

/*
 * module attach/detach
 */
extern wlc_ier_info_t *wlc_ier_attach(wlc_info_t *wlc);
extern void wlc_ier_detach(wlc_ier_info_t *iem);

/*
 * Create a registry to hold 'calc_len/build' and 'parse' callbacks.
 *
 * Inputs:
 * - build_cnt: # of 'calc_len/build' entries
 * - parse_cnt: # of 'parse' entries
 *
 * Return:
 * - A NULL return value indicates an error (e.g. out of memory).
 */
extern wlc_ier_reg_t *wlc_ier_create_registry(wlc_ier_info_t *iem,
	uint16 build_cnt, uint16 parse_cnt);

/*
 * Free the registry.
 */
extern void wlc_ier_destroy_registry(wlc_ier_reg_t *reg);

/*
 * Sort the 'calc_len/build' portion of the registry with the provided sorting order
 * table 'ie_tags/ie_cnt'.
 *
 * The sorting order is normally defined in the relevant IEEE spec.
 *
 * Return:
 * - BRCM_XXXX
 */
extern int wlc_ier_sort_cbtbl(wlc_ier_reg_t *reg, const uint8 *ie_tags, uint16 ie_cnt);

/*
 * Register a pair of 'calc_len'/'build' callbacks for an non Vendor Specific IE.
 *
 * Inputs:
 * - tag: IE tag as defined by DOT11_MNG_XXXX in 802.11.h
 *
 * Return:
 * - A negative return value indicates an error.
 */
extern int wlc_ier_add_build_fn(wlc_ier_reg_t *reg, uint8 tag,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx);

/*
 * Register a pair of 'calc_len'/build_frame' callbacks for an Vendor Specific IE.
 *
 * Inputs:
 * - prio: Relative position of Vendor Specific IE in a frame
 *
 * Some known priorities are defined by WLC_IEM_VS_IE_PRIO_XXXX (0 at the
 * first in the frame while 249 at the last).
 *
 * Position of Vendor Specific IEs are undefined with the same prio value.
 *
 * Return:
 * - A negative return value indicates an error.
 */
extern int wlc_ier_vs_add_build_fn(wlc_ier_reg_t *reg, uint8 prio,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx);

/*
 * Calculate all IEs' length in a frame.
 *
 * Inputs:
 * - cbparm: Callback parameters
 * - ft: Frame type, can be FC_ACTION or anything that the callbacks understand
 *
 * Return:
 * - IEs' total length in bytes
 */
extern uint wlc_ier_calc_len(wlc_ier_reg_t *reg, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_cbparm_t *cbparm);

/*
 * Write all IEs in a frame.
 *
 * Inputs:
 * - cbparm: Callback parameters
 * - ft: Frame type, can be FC_ACTION or anything that the callbacks understand
 * - buf: IEs' buffer pointer
 * - buf_len: IEs' buffer length
 *
 * Outputs:
 * - buf: IEs written in the buffer
 *
 * Return:
 * - A negative return value indicates an error (BCME_XXXX).
 */
extern int wlc_ier_build_frame(wlc_ier_reg_t *reg, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_cbparm_t *cbparm, uint8 *buf, uint buf_len);

/*
 * Register 'parse' callback for an non Vendor Specific IE 'tag'.
 *
 * The callback is invoked when parsing a frame with the IE.
 *
 * Inputs:
 * - tag: IE tag as defined by DOT11_MNG_XXXX in 802.11.h
 *
 * Return:
 * - A negative return value indicates an error.
 */
extern int wlc_ier_add_parse_fn(wlc_ier_reg_t *reg, uint8 tag,
	wlc_iem_parse_fn_t parse_fn, void *ctx);

/*
 * Register 'parse' callback for an Vendor Specific IE 'id'.
 *
 * The callback is invoked when parsing a frame with the IE.
 *
 * Inputs:
 * - id: Vendor Specific IE ID
 *
 * Known IDs are defined by WLC_IEM_VS_IE_PRIO_XXXX in wlc_iem_vs.h.
 *
 * Return:
 * - A negative return value indicates an error.
 */
extern int wlc_ier_vs_add_parse_fn(wlc_ier_reg_t *reg, uint8 id,
	wlc_iem_parse_fn_t parse_fn, void *ctx);

/*
 * Parse IEs in a frame.
 *
 * Inputs:
 * - ft: Frame type, can be FC_ACTION or anything that the callbacks understand
 * - upp: User parse parameters
 * - pparm: Parameters passed to callbacks
 * - buf: IEs' buffer pointer
 * - buf_len: IEs' buffer length
 *
 * An internal classification callback is used (wlc_iem_vs_get_id) if upp is NULL,
 * in which case an Vendor Specific IE may be ignored if the built-in classifier
 * doesn't know how to map the Vendor Specific IE to an 'id'.
 *
 * Return:
 * - A negative return value indicates an error (BCME_XXXX).
 */
extern int wlc_ier_parse_frame(wlc_ier_reg_t *reg, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_upp_t *upp, wlc_iem_pparm_t *pparm, uint8 *buf, uint buf_len);

#endif /* _wlc_ie_reg_h_ */
