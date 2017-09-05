/*
 * Dynamic WDS module header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_wds.h 600588 2015-11-19 07:34:57Z $
*/


#ifndef _wlc_wds_h_
#define _wlc_wds_h_

/* flags for wlc_wds_create() */
#define WDS_INFRA_BSS	0x1	/* WDS link is part of the infra mode BSS */
#define WDS_DYNAMIC	0x2	/* WDS link is dynamic */

/* APIs */
#ifdef WDS
/* module */
extern wlc_wds_info_t *wlc_wds_attach(wlc_info_t *wlc);
extern void wlc_wds_detach(wlc_wds_info_t *wds);
extern void wlc_ap_wds_probe_complete(wlc_info_t *wlc, uint txstatus, struct scb *scb);
extern int wlc_wds_create(wlc_info_t *wlc, struct scb *scb, uint flags);
extern void wlc_scb_wds_free(struct wlc_info *wlc);
extern bool wlc_wds_lazywds_is_enable(wlc_wds_info_t *mwds);
extern int wlc_wds_create_link_event(wlc_info_t *wlc, struct scb *scb, bool isup);
#ifdef DPSTA
#if defined(STA) && defined(DWDS)
extern struct scb *wlc_dwds_client_is_ds_sta(wlc_info_t *wlc, struct ether_addr *mac);
extern bool wlc_dwds_is_ds_sta(wlc_info_t *wlc, struct ether_addr *mac);
extern bool wlc_dwds_authorized(wlc_bsscfg_t *cfg);
#endif /* STA && DWDS */
#endif /* DPSTA */
#else /* !WDS */

#define wlc_wds_attach(wlc) NULL
#define wlc_wds_detach(mwds) do {} while (0)
#define wlc_ap_wds_probe_complete(a, b, c) 0
#define wlc_wds_create(a, b, c)	0
#define wlc_scb_wds_free(a) do {} while (0)
#define wlc_wds_lazywds_is_enable(a) 0
#define wlc_wds_create_link_event(a, b, c) do {} while (0)

#endif /* !WDS */

#endif /* _wlc_wds_h_ */
