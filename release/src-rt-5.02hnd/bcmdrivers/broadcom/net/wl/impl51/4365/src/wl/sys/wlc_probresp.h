/*
 * SW probe response module header file
 * disable ucode sending probe response,
 * driver will decide whether send probe response,
 * after check the received probe request.
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_probresp.h 467328 2014-04-03 01:23:40Z $
*/

#ifndef _wlc_probresp_h_
#define _wlc_probresp_h_

typedef bool (*probreq_filter_fn_t)(void *handle, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr,
	uint8 *body, int body_len, bool *psendProbeResp);

/* APIs */
#ifdef WLPROBRESP_SW
/* module */
extern wlc_probresp_info_t *wlc_probresp_attach(wlc_info_t *wlc);
extern void wlc_probresp_detach(wlc_probresp_info_t *mprobresp);
extern int wlc_probresp_register(wlc_probresp_info_t *mprobresp, void *hdl,
	probreq_filter_fn_t filter_fn, bool p2p);
extern int wlc_probresp_unregister(wlc_probresp_info_t *mprobresp, void *hdl);
extern void wlc_probresp_recv_process_prbreq(wlc_probresp_info_t *mprobresp,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr,
	uint8 *body, int body_len);
#else /* !WLPROBRESP_SW */

#define wlc_probresp_attach(wlc) (NULL)
#define wlc_probresp_detach(mprobresp) ((void)(0))
#define wlc_probresp_register(mprobresp, hdl, filter_fn, p2p) ((void)(BCME_OK))
#define wlc_probresp_unregister(mprobresp, hdl) ((void)(BCME_OK))
#define wlc_probresp_recv_process_prbreq(mprobresp, wrxh, plcp, hdr, body, body_len) ((void)(0))

#endif /* !WLPROBRESP_SW */

#endif /* _wlc_probresp_h_ */
