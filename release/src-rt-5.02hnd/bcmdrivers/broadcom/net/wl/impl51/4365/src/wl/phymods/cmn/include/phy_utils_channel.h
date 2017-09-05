/*
 * PHY utils - chanspec functions.
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

#ifndef _phy_utils_channel_h_
#define _phy_utils_channel_h_

#include <typedefs.h>
#include <bcmdefs.h>

#include <wlc_phy_hal.h>

int phy_utils_channel2freq(uint channel);
uint phy_utils_channel2idx(uint channel);
chanspec_t phy_utils_get_chanspec(phy_info_t *pi);

#endif /* _phy_utils_channel_h_ */
