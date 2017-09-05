/*
 * BTA (BlueTooth Alternate Mac and Phy module aka BT-AMP)
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_bta.h 467328 2014-04-03 01:23:40Z $
*/

#ifndef _wlc_bta_h_
#define _wlc_bta_h_

#ifdef WLBTAMP

#define WLC_BTA_RADIO_DISABLE	0
#define WLC_BTA_AP_ASSOC	3
#define WLC_BTA_RADIO_ENABLE	6

extern bta_info_t *wlc_bta_attach(wlc_info_t *wlc);
extern void wlc_bta_detach(bta_info_t *bta);

extern bool wlc_bta_active(bta_info_t *bta);
extern bool wlc_bta_inprog(bta_info_t *bta);

extern void wlc_bta_join_complete(bta_info_t *bta, struct scb *scb, uint8 status);
extern void wlc_bta_AKM_complete(bta_info_t *bta, struct scb *scb);

extern void wlc_bta_assoc_complete(bta_info_t *bta, wlc_bsscfg_t *cfg);

extern bool wlc_bta_recv_proc(bta_info_t *bta, struct wlc_frminfo *f, struct scb *scb);
extern bool wlc_bta_send_proc(bta_info_t *bta, void *p, wlc_if_t **wlcif);

extern void wlc_bta_tx_hcidata(void *handle, uint8 *data, uint len);
extern void wlc_bta_docmd(void *handle, uint8 *cmd, uint len);
extern void wlc_bta_scb_cleanup(bta_info_t *bta, struct scb *scb);
extern void wlc_bta_radio_status_upd(bta_info_t *bta);
extern void wlc_bta_assoc_status_upd(bta_info_t *bta, wlc_bsscfg_t *cfg, uint8 state);
extern bool wlc_bta_frameburst_active(bta_info_t *bta, wlc_pkttag_t *pkttag, uint rate);
#if defined(BCMDBG) || defined(WLMSG_BTA)
extern void wlc_bta_dump_stats(bta_info_t *bta);
#else
#define wlc_bta_dump_stats(a) do {} while (0)
#endif

#else	/* stubs */

#define wlc_bta_attach(a) (bta_info_t *)0x0dadbeef
#define	wlc_bta_detach(a) do {} while (0)

#endif /* WLBTAMP */

#endif /* _wlc_bta_h_ */
