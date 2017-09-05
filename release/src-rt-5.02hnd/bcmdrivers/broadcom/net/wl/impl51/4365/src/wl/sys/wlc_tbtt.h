/*
 * TBTT module API
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_tbtt.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_tbtt_h_
#define _wlc_tbtt_h_

/* module entries */
extern wlc_tbtt_info_t *wlc_tbtt_attach(wlc_info_t *wlc);
extern void wlc_tbtt_detach(wlc_tbtt_info_t *ti);

/* configuration */
extern int wlc_tbtt_ent_tsf_adj_set(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg, int adj);

/* tbtt fn add/remove */
typedef struct {
	wlc_bsscfg_t *cfg;
	uint32 tsf_h;
	uint32 tsf_l;
} wlc_tbtt_ent_data_t;
typedef void (*wlc_tbtt_ent_fn_t)(void *ctx, wlc_tbtt_ent_data_t *notif_data);
extern int wlc_tbtt_ent_fn_add(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg,
	wlc_tbtt_ent_fn_t pre_fn, wlc_tbtt_ent_fn_t fn, void *arg);
extern int wlc_tbtt_ent_fn_del(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg,
	wlc_tbtt_ent_fn_t pre_fn, wlc_tbtt_ent_fn_t fn, void *arg);
extern void wlc_tbtt_ent_init(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg);

#endif /* _wlc_tbtt_h_ */
