/*
 * Channel Manager Notification internal interface (to other PHY modules).
 *
 * This is chanspec event notification client management interface.
 *
 * It provides interface for clients to register notification callbacks.
 * It invokes registered callbacks synchronously when interested event
 * is raised.
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

#ifndef _phy_chanmgr_notif_h_
#define _phy_chanmgr_notif_h_

#include <typedefs.h>
#include <bcmwifi_channels.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_chanmgr_notif_info phy_chanmgr_notif_info_t;

/* attach/detach */
phy_chanmgr_notif_info_t *phy_chanmgr_notif_attach(phy_info_t *pi);
void phy_chanmgr_notif_detach(phy_chanmgr_notif_info_t *cni);

/* register client/interest */
typedef void phy_chanmgr_notif_ctx_t;
typedef struct {
	uint16 event;		/* event (single) */
	chanspec_t new;		/* the new chanspec */
	chanspec_t old;		/* the old chanspec (for channel change related events) */
} phy_chanmgr_notif_data_t;
typedef int (*phy_chanmgr_notif_fn_t)(phy_chanmgr_notif_ctx_t *ctx,
	phy_chanmgr_notif_data_t *data);

/* event - maximum 16 events */
#define PHY_CHANMGR_NOTIF_OPCHCTX_OPEN	(1<<0)	/* operating chanspec context creation */
#define PHY_CHANMGR_NOTIF_OPCHCTX_CLOSE	(1<<1)	/* operating chanspec context delete */
#define PHY_CHANMGR_NOTIF_OPCH_CHG	(1<<2)	/* operating chanspec change */
#define PHY_CHANMGR_NOTIF_CH_CHG		(1<<3)	/* non-operating chanspec change */

/* Callback order.
 *
 * All callbacks are iovoked when a given event is raised. Callback registered
 * with smaller numeric number is invoked first.
 *
 * It is used in attach process only so no need to assign values.
 * Maximum 256 orders.
 */
typedef enum phy_chanmgr_notif_order {
	PHY_CHANMGR_NOTIF_ORDER_START = 0,
	/* the next three clients must remain this order */
	PHY_CHANMGR_NOTIF_ORDER_CACHE,
	PHY_CHANMGR_NOTIF_ORDER_CALMGR,
	PHY_CHANMGR_NOTIF_ORDER_WD
	/* add new client here */
} phy_chanmgr_notif_order_t;

/* 'events' is a mask of the supported events */
int phy_chanmgr_notif_add_interest(phy_chanmgr_notif_info_t *cni,
	phy_chanmgr_notif_fn_t fn, phy_chanmgr_notif_ctx_t *ctx,
	phy_chanmgr_notif_order_t order, uint16 events);

#endif /* _phy_chanmgr_notif_h_ */
