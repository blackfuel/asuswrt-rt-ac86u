/*
 * EDCRS module public interface (to MAC driver).
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

#ifndef _phy_rxgcrs_api_h_
#define _phy_rxgcrs_api_h_

#include <typedefs.h>
#include <phy_api.h>

/*
 * These region constants are set by the
 * driver on region change if and when region-specific
 * tuning is required. For example, this is used to
 * turn on EU-specific energy detection.
 */
#define REGION_OTHER 0
#define REGION_EU 1

/*
 * edcrs related function
 * set edcrs based on EU local.
 *   0 --> everybody else
 *   1 --> EU
 */
extern void wlc_phy_set_locale(phy_info_t *pi, uint8 region_group);

#endif /* _phy_rssi_api_h_ */
