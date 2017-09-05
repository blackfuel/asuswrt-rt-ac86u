/*
 * RSSI Compute module internal interface (to other PHY modules).
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

#ifndef _phy_rssi_h_
#define _phy_rssi_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_rssi_info phy_rssi_info_t;

/* attach/detach */
phy_rssi_info_t *phy_rssi_attach(phy_info_t *pi);
void phy_rssi_detach(phy_rssi_info_t *ri);

/*
 * Compare rssi at different antennas and return the antenna index that
 * has the largest rssi value.
 *
 * Return value is a bitvec, the bit index of '1' is the antenna index.
 */
uint8 phy_rssi_compare_ant(phy_rssi_info_t *ri);

/*
 * rssi merge mode?
 */
#define RSSI_ANT_MERGE_MAX	0	/* pick max rssi of all antennas */
#define RSSI_ANT_MERGE_MIN	1	/* pick min rssi of all antennas */
#define RSSI_ANT_MERGE_AVG	2	/* pick average rssi of all antennas */

/*
 * Init gain error table.
 */
void phy_rssi_init_gain_err(phy_rssi_info_t *ri);

#endif /* _phy_rssi_h_ */
