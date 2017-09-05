/*
 * CACHE module internal interface (to other PHY modules).
 *
 * This cache is dedicated to operating chanspec contexts (see phy_chanmgr_notif.h).
 *
 * Each cache entry once 'used' has a corresponding operating chanspec context.
 * The current operating chanspec context is the operating chanspec context whose
 * chanspec is programmed as the current radio chanspec. The cache entry that is
 * associated with the current operating chanspec context is called the current
 * cache entry.
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

#ifndef _phy_cache_h_
#define _phy_cache_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_cache_info phy_cache_info_t;

/* ================== interface for attach/detach =================== */

/* attach/detach */
phy_cache_info_t *phy_cache_attach(phy_info_t *pi);
void phy_cache_detach(phy_cache_info_t *ci);

/* ================== interface for calibration modules =================== */

/* forward declaration */
typedef uint32 phy_cache_cubby_id_t;

/*
 * Reserve a cubby in each cache entry and register the client callbacks and context.
 * - 'init' callback is optional and is invoked whenver a cache entry is made current
 *   but before any 'save' operation is performed to the cubby. It gives the cubby
 *   client a chance to initialize its relevant states to known ones.
 * - 'save' callback is mandatory and is invoked to save client states to the cubby.
 *   The invocation can happen automatically when a cache entry is make non-current
 *   and the cubby is reserved as AUTO SAVE cubby; or manually when phy_cache_save
 *   function is called.
 * - 'restore' callback is mandatory and is invoked to restore client states from
 *   from the cubby. It happens when a cache entry is made current.
 * - 'dump' callback is optional and is for debugging and dumpping the cubby.
 *
 * - 'ccid' is the cubby ID when the function is successfully called. It is used
 *   to call other cache module functions. It is also used to register a calibration
 *   callback to the calibration management (calmgr) module.
 */
typedef void phy_cache_ctx_t;

typedef void (*phy_cache_init_fn_t)(phy_cache_ctx_t *ctx);
typedef int (*phy_cache_save_fn_t)(phy_cache_ctx_t *ctx, uint8 *buf);
typedef int (*phy_cache_restore_fn_t)(phy_cache_ctx_t *ctx, uint8 *buf);
typedef int (*phy_cache_dump_fn_t)(phy_cache_ctx_t *ctx, uint8 *buf, struct bcmstrbuf *b);

int phy_cache_reserve_cubby(phy_cache_info_t *ci, phy_cache_init_fn_t init,
	phy_cache_save_fn_t save, phy_cache_restore_fn_t restore, phy_cache_dump_fn_t dump,
	phy_cache_ctx_t *ctx, uint32 size, uint32 flags, phy_cache_cubby_id_t *ccid);

/* cache cubby reservation flag */
#define PHY_CACHE_FLAG_AUTO_SAVE (1<<0)	/* save callback is invoked automatically
					 * by cache module when one makes the
					 * current cache entry non-current.
					 */

/* cache cubby reservation id */
#define PHY_CACHE_CUBBY_INV_ID	-1	/* invalid id - indicate to calmgr
					 * to bypass cache save operation.
					 */


/* ================== interface for calibration mgmt. module =================== */

/*
 * Save results after a calibration phase or module is finished.
 */
int phy_cache_save(phy_cache_info_t *ci, phy_cache_cubby_id_t ccid);


/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
#if defined(PHYCAL_CACHING) || defined(WLMCHAN)
extern int	wlc_phy_cal_cache_init(wlc_phy_t *ppi);
extern void wlc_phy_cal_cache_deinit(wlc_phy_t *ppi);
#endif
#if defined(PHYCAL_CACHING) || defined(PHYCAL_CACHE_SMALL)
extern void wlc_phy_cal_cache_set(wlc_phy_t *ppi, bool state);
extern bool wlc_phy_cal_cache_get(wlc_phy_t *ppi);
#endif
/* Calibration caching Module (regular caching, per channel, smart cal) */
#if defined(PHYCAL_CACHING) || defined(WLMCHAN)
extern int wlc_phy_create_chanctx(wlc_phy_t *ppi, chanspec_t chanspec);
extern void wlc_phy_destroy_chanctx(wlc_phy_t *ppi, chanspec_t chanspec);
extern int wlc_phy_invalidate_chanctx(wlc_phy_t *ppi, chanspec_t chanspec);
extern int wlc_phy_reuse_chanctx(wlc_phy_t *ppi, chanspec_t chanspec);
extern void wlc_phy_get_cachedchans(wlc_phy_t *ppi, chanspec_t *chanlist);
extern int32 wlc_phy_get_est_chanset_time(wlc_phy_t *ppi, chanspec_t chanspec);
extern bool wlc_phy_chan_iscached(wlc_phy_t *ppi, chanspec_t chanspec);
#endif


#endif /* _phy_cache_h_ */
