/*
 * DLS header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_dls.h 467328 2014-04-03 01:23:40Z $
*/


/**
 * 802.11e Spec introduced DLS (Direct Link Setup)
 */

#ifndef _WLC_DLS_H_
#define _WLC_DLS_H_

#ifdef WLDLS

extern dls_info_t *wlc_dls_attach(wlc_info_t *wlc);
extern void wlc_dls_detach(dls_info_t *dls);
extern void wlc_dls_recv_process_dls(dls_info_t *dls,
	uint action_id, struct dot11_management_header *hdr,
	uint8 *body, int body_len);
#else	/* stubs */
#define wlc_dls_attach(wlc) NULL
#define wlc_dls_detach(dls) do {} while (0)
#define wlc_dls_recv_process_dls(dls, action_id, hdr, body, body_len) do {} while (0)
#endif /* WLDLS */

#endif /* _WLC_DLS_H_ */
