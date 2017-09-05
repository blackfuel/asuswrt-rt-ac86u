/*
 * 11g/n shared protection module private APIs
 * Broadcom 802.11abg Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_prot_priv.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_prot_priv_h_
#define _wlc_prot_priv_h_

#include <wlc_prot.h>

/* module private states */
typedef struct {
	wlc_info_t *wlc;
	uint16 cfg_offset;	/* bss_prot_cfg_t offset in bsscfg cubby client */
} wlc_prot_info_priv_t;

/* module states layout */
typedef struct {
	wlc_prot_info_t pub;
	wlc_prot_info_priv_t priv;
} wlc_prot_t;
/* module states size */
#define WLC_PROT_SIZE	(sizeof(wlc_prot_t))
/* moudle states location */
extern uint16 wlc_prot_info_priv_offset;
#define WLC_PROT_INFO_PRIV(prot) ((wlc_prot_info_priv_t *) \
				  ((uintptr)(prot) + wlc_prot_info_priv_offset))

/* bsscfg private states */
typedef	struct {
	int8	overlap;		/* Overlap BSS/IBSS protection for both 11g and 11n */
} bss_prot_cfg_t;

/* bsscfg states layout */
typedef struct {
	wlc_prot_cfg_t pub;
	bss_prot_cfg_t priv;
} bss_prot_t;
/* bsscfg states size */
#define BSS_PROT_SIZE	(sizeof(bss_prot_t))
/* bsscfg states location */
#define BSS_PROT_CUBBY(prot, cfg) ((bss_prot_t *) \
				   BSSCFG_CUBBY(cfg, WLC_PROT_INFO_CFGH(prot)))
#define BSS_PROT_CFG(prot, cfg) ((bss_prot_cfg_t *) \
				 ((uintptr)BSS_PROT_CUBBY(prot, cfg) +	\
				  WLC_PROT_INFO_PRIV(prot)->cfg_offset))

/* update configuration */
extern void wlc_prot_cfg_upd(wlc_prot_info_t *prot, wlc_bsscfg_t *cfg, uint idx, int val);
/* wlc_prot_cfg_upd() idx */
#define WLC_PROT_OVERLAP	1
#define WLC_PROT_SHORTPREAMBLE	2

/* configuration init */
extern void wlc_prot_cfg_init(wlc_prot_info_t *prot, wlc_bsscfg_t *cfg);

/* protection init */
extern void wlc_prot_init(wlc_prot_info_t *prot, wlc_bsscfg_t *cfg);

/* propagate condition */
extern void wlc_prot_cond_set(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint coff, bool set,
	uint8 *(*cb)(void *prot, wlc_bsscfg_t *cfg, uint coff), void *prot);

/* check if associated scbs have the flags set */
extern bool wlc_prot_scb_scan(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	uint32 flagmask, uint32 flagvalue);

#endif /* _wlc_prot_priv_h_ */
