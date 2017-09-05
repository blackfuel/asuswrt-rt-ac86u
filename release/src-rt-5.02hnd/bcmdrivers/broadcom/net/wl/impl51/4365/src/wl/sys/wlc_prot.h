/*
 * 11g/n shared protection module APIs
 * Broadcom 802.11abg Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_prot.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wlc_prot_h_
#define _wlc_prot_h_

/* data APIs */
/* module public states - read only */
struct wlc_prot_info {
	int cfgh;
};
#define WLC_PROT_INFO_CFGH(prot)	((prot)->cfgh)

/* bsscfg specific states - read only */
typedef	struct {
	bool shortpreamble;		/* currently operating with CCK ShortPreambles */
} wlc_prot_cfg_t;
#define WLC_PROT_CFG(prot, cfg)	((wlc_prot_cfg_t *) \
				 BSSCFG_CUBBY(cfg, WLC_PROT_INFO_CFGH(prot)))
#define WLC_PROT_CFG_SHORTPREAMBLE(prot, cfg) (WLC_PROT_CFG(prot, cfg)->shortpreamble)

/* function APIs */
/* module entries */
extern wlc_prot_info_t *wlc_prot_attach(wlc_info_t *wlc);
extern void wlc_prot_detach(wlc_prot_info_t *prot);

#endif /* _wlc_prot_h_ */
