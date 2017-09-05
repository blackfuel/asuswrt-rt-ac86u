/*
 * RadarDetect module internal interface (functions sharde by PHY type specific implementation).
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

#ifndef _phy_radar_utils_h_
#define _phy_radar_utils_h_

#include <typedefs.h>

/* utilities */
void wlc_phy_radar_generate_tlist(uint32 *inlist, int *outlist, int length, int n);
void wlc_phy_radar_filter_list(int *inlist, int *length, int min_val, int max_val);
int wlc_phy_radar_select_nfrequent(int *inlist, int length, int n, int *value,
	int *position, int *frequency, int *vlist, int *flist);

#endif /* _phy_radar_utils_h_ */
