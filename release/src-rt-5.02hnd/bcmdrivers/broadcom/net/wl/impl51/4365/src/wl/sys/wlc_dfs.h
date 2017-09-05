/*
 * 802.11h DFS module header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_dfs.h 644679 2016-06-21 10:03:32Z $
 */


#ifndef _wlc_dfs_h_
#define _wlc_dfs_h_

#if defined(WLRSDB) || defined(BGDFS)
#include <wlc_modesw.h>
#endif /* WLRSDB || BGDFS */

/* check if core revision supports background DFS scan core */
#define DFS_HAS_BACKGROUND_SCAN_CORE(wlc) D11REV_GE((wlc)->pub->corerev, 0x40)


/* radar subband information is available only for 4365/4366 corerev 0x40 (<=b1) and 0x41 (c0) */
#define DFS_HAS_SUBBAND_INFO(wlc) (D11REV_IS(wlc->pub->corerev, 0x40) || \
		D11REV_IS(wlc->pub->corerev, 0x41))

/* 160MHz mode is available only for 4365/4366 corerev 0x41 (c0) */
#define DFS_HAS_DYN160(wlc)	D11REV_IS(wlc->pub->corerev, 0x41)

#ifdef WLDFS

/* module */
extern wlc_dfs_info_t *wlc_dfs_attach(wlc_info_t *wlc);
extern void wlc_dfs_detach(wlc_dfs_info_t *dfs);

/* others */
extern void wlc_set_dfs_cacstate(wlc_dfs_info_t *dfs, int state);
extern chanspec_t wlc_dfs_sel_chspec(wlc_dfs_info_t *dfs, bool force);
extern void wlc_dfs_reset_all(wlc_dfs_info_t *dfs);
extern int wlc_dfs_set_radar(wlc_dfs_info_t *dfs, int radar, uint subband);
extern bool wlc_valid_dfs_chanspec(wlc_info_t *wlc, chanspec_t chspec);
extern uint wlc_dfs_get_cactime_ms(wlc_dfs_info_t *dfs);
extern bool wlc_cac_is_clr_chanspec(wlc_dfs_info_t *dfs, chanspec_t chspec);
extern void wlc_dfs_send_action_frame_complete(wlc_info_t *wlc, uint txstauts, void *arg);

/* accessors */
extern uint32 wlc_dfs_get_radar(wlc_dfs_info_t *dfs);

extern uint32 wlc_dfs_get_chan_info(wlc_dfs_info_t *dfs, uint channel);

#if defined(RSDB_DFS_SCAN) || defined(BGDFS)
extern int wlc_dfs_scan_in_progress(wlc_dfs_info_t *dfs);
extern void wlc_dfs_scan_abort(wlc_dfs_info_t *dfs);
extern void wlc_dfs_handle_modeswitch(wlc_dfs_info_t *dfs, uint new_state);
extern int wlc_dfs_scan(wlc_dfs_info_t *dfs, wl_chan_switch_t *csa);
#else
#define wlc_dfs_scan_in_progress(dfs) 0
#define wlc_dfs_scan_abort(dfs) do {} while (0)
#define wlc_dfs_handle_modeswitch(dfs, new_state) do {} while (0)
#define wlc_dfs_scan(dfs, csa) 0
#endif /* RSDB_DFS_SCAN || BGDFS */

#if defined(BGDFS) && defined(WL_MODESW)
extern void wlc_dfs_opmode_change_cb(void *ctx, wlc_modesw_notif_cb_data_t *notif_data);
#endif /* BGDFS && WL_MODESW */

#else /* !WLDFS */

#define wlc_dfs_attach(wlc) NULL
#define wlc_dfs_detach(dfs) do {} while (0)

#define wlc_set_dfs_cacstate(dfs, state) do {} while (0)
#define wlc_dfs_sel_chspec(dfs, force) 0
#define wlc_dfs_reset_all(dfs) do {} while (0)
#define wlc_dfs_set_radar(dfs, radar, subband)  BCME_UNSUPPORTED
#define wlc_valid_dfs_chanspec(wlc, chspec) (TRUE)
#define wlc_dfs_send_action_frame_complete(wlc, txstauts, arg) do {} while (0)

#define wlc_dfs_get_radar(dfs) 0
#define wlc_dfs_get_chan_info(dfs, channel) 0

#define wlc_dfs_scan_in_progress(dfs) 0
#define wlc_dfs_scan_abort(dfs) do {} while (0)
#define wlc_dfs_handle_modeswitch(dfs, new_state) do {} while (0)
#define wlc_dfs_scan(dfs, csa) 0

#define wlc_dfs_opmode_change_cb(ctx, notif_data) do {} while (0)

#endif /* !WLDFS */

#endif /* _wlc_dfs_h_ */
