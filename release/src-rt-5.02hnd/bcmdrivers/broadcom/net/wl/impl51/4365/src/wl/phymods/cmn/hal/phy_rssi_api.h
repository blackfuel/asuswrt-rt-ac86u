/*
 * RSSICompute module public interface (to MAC driver).
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _phy_rssi_api_h_
#define _phy_rssi_api_h_

#include <typedefs.h>
#include <d11.h>
#include <phy_api.h>

/*
 * Compute rssi based on rxh info and save the result in wrxh.
 * Return the computed rssi value as well.
 */
int8 phy_rssi_compute_rssi(phy_info_t *pi, wlc_d11rxhdr_t *wrxh);

#endif /* _phy_rssi_api_h_ */
