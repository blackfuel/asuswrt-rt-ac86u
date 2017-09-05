/*
 * IOCV module interface.
 * For WLC.
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

#ifndef _wlc_iocv_high_h_
#define _wlc_iocv_high_h_

#ifndef WLC_HIGH
#error "WLC_HIGH is not defined"
#endif

#include <typedefs.h>
#include <wlc_types.h>
#include <wlc_iocv_types.h>

/* attach/detach */
wlc_iocv_info_t *wlc_iocv_high_attach(wlc_info_t *wlc);
void wlc_iocv_high_detach(wlc_iocv_info_t *ii);

/* register wlc iovar table & dispatcher */
int wlc_iocv_high_register_iovt(wlc_iocv_info_t *ii, const bcm_iovar_t *iovt,
	wlc_iov_disp_fn_t disp_fn, void *ctx);
/* register wlc ioctl table & dispatcher */
int wlc_iocv_high_register_ioct(wlc_iocv_info_t *ii, const wlc_ioctl_cmd_t *ioct, uint num_cmds,
	wlc_ioc_disp_fn_t disp_fn, void *ctx);

/*
 * TODO: Remove unnecessary interface
 * once integrated with existing iovar table handling.
 */

/* lookup iovar and return iovar entry and table id if found */
const bcm_iovar_t *wlc_iocv_high_find_iov(wlc_iocv_info_t *ii, const char *name, uint16 *tid);

/* forward iovar to registered table dispatcher */
int wlc_iocv_high_fwd_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid, const bcm_iovar_t *vi,
	uint8 *p, uint p_len, uint8 *a, uint a_len, uint var_sz);

/* lookup ioctl and return ioctl entry and table id if found */
const wlc_ioctl_cmd_t *wlc_iocv_high_find_ioc(wlc_iocv_info_t *ii, uint16 cid, uint16 *tid);

/* forward ioctl to registered table dispatcher */
int wlc_iocv_high_fwd_ioc(wlc_iocv_info_t *ii, uint16 tid, const wlc_ioctl_cmd_t *ci,
	uint8 *a, uint a_len);

/* validate ioctl */
int wlc_iocv_high_vld_ioc(wlc_iocv_info_t *ii, uint16 tid, const wlc_ioctl_cmd_t *ci,
	void *a, uint a_len);

#ifdef WLC_HIGH_ONLY
#include <bcm_xdr.h>

/* pack iovar parameters into buffer */
bool wlc_iocv_high_pack_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid,
	void *p, uint p_len, bcm_xdr_buf_t *b);
/* unpack iovar result from buffer */
bool wlc_iocv_high_unpack_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid,
	bcm_xdr_buf_t *b, void *a, uint a_len);

/* pack ioctl parameters into buffer */
bool wlc_iocv_high_pack_ioc(wlc_iocv_info_t *ii, uint16 tid, uint16 cid,
	void *a, uint a_len, bcm_xdr_buf_t *b);
/* unpack ioctl result from buffer */
bool wlc_iocv_high_unpack_ioc(wlc_iocv_info_t *ii, uint16 tid, uint16 cid,
	bcm_xdr_buf_t *b, void *a, uint a_len);
#endif /* WLC_HIGH_ONLY */

#endif /* _wlc_iocv_high_h_ */
