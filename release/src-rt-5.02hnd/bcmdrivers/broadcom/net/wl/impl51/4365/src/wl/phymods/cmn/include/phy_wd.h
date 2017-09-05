/*
 * WatchDog module interface (to other PHY modules).
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

#ifndef _phy_wd_h_
#define _phy_wd_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_wd_info phy_wd_info_t;

/* attach/detach */
phy_wd_info_t *phy_wd_attach(phy_info_t *pi);
void phy_wd_detach(phy_wd_info_t *ri);

/*
 * Add a watchdog callback fn for a module.
 *
 * The callback is invoked at every period in general when now % period == 0.
 * The callback returns TRUE to indicate it has finished the task for the current
 * period; it returns FALSE otherwise. In the case the callback indicates it hasn't
 * finished the task for the current period when the callback is invoked the callback
 * will be invoked again at the next watchdog tick (1s later) repeately until the
 * callback indicates it's done the task.
 */
typedef void phy_wd_ctx_t;
typedef bool (*phy_wd_fn_t)(phy_wd_ctx_t *ctx);
/*
 * WATCHDOG callback periods.
 */
typedef enum phy_wd_prd {
	PHY_WD_PRD_1TICK = 1,		/* 1s */
	PHY_WD_PRD_FAST = 15,		/* 15s */
	PHY_WD_PRD_SLOW = 60,		/* 60s */
	PHY_WD_PRD_GLACIAL = 120,	/* 120s */
} phy_wd_prd_t;
/*
 * WATCHDOG callback execution orders.
 * Note: Keep the enums between 0 and 255!
 */
typedef enum phy_wd_order {
	PHY_WD_1TICK_START = 0,
	PHY_WD_1TICK_TPC,
	PHY_WD_1TICK_NOISE_STOP,
	PHY_WD_1TICK_INTF_NOISE,
	PHY_WD_1TICK_NOISE_ACI,
	PHY_WD_FAST_BTCX,
	PHY_WD_FAST_RADIO,
	PHY_WD_1TICK_NOISE_START,
	PHY_WD_1TICK_NOISE_RESET,
	PHY_WD_1TICK_CALMGR,
	PHY_WD_GLACIAL_CAL
} phy_wd_order_t;
/*
 * WATCHDOG callback flags.
 * Note: Keep the enums between 0 and 2^16 - 1!
 */
typedef enum phy_wd_flag {
	PHY_WD_FLAG_NONE = 0,

	/* defer until next watchdog tick (1s) */
	PHY_WD_FLAG_SCAN_DEFER = 0x01,
	PHY_WD_FLAG_PLT_DEFER = 0x02,
	PHY_WD_FLAG_AS_DEFER = 0x04,

	/* skip the period if in progress */
	PHY_WD_FLAG_SCAN_SKIP = 0x10,
	PHY_WD_FLAG_PLT_SKIP = 0x20,
	PHY_WD_FLAG_AS_SKIP = 0x40,

	/* combinations */
	PHY_WD_FLAG_DEF_DEFER = (PHY_WD_FLAG_SCAN_DEFER |
	                         PHY_WD_FLAG_PLT_DEFER |
	                         PHY_WD_FLAG_AS_DEFER),
	PHY_WD_FLAG_DEF_SKIP = (PHY_WD_FLAG_SCAN_SKIP |
	                        PHY_WD_FLAG_PLT_SKIP |
	                        PHY_WD_FLAG_AS_SKIP),

	/* multi-channel aware callback */
	PHY_WD_FLAG_MCHAN_AWARE = 0x100
} phy_wd_flag_t;

/* Add a watchdog callback fn. Return BCME_XXXX. */
#ifndef BCM_OL_DEV
int phy_wd_add_fn(phy_wd_info_t *wi, phy_wd_fn_t fn, phy_wd_ctx_t *ctx,
	phy_wd_prd_t prd, phy_wd_order_t order, phy_wd_flag_t flags);
#else
#define phy_wd_add_fn(wi, fn, ctx, prd, order, flags) ((void)(fn), BCME_OK)
#endif /* BCM_OL_DEV */

#endif /* _phy_wd_h_ */
