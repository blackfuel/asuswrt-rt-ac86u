/*
 * RADIO control module internal interface - shared by PHY type specific implementations.
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

#ifndef _phy_utils_radio_h_
#define _phy_utils_radio_h_

#include <typedefs.h>
#include <phy_api.h>

/* parse radio idcode and save result in pi */
void phy_utils_parse_idcode(phy_info_t *pi, uint32 idcode);

/* validate saved result in pi */
int phy_utils_valid_radio(phy_info_t *pi);

#endif /* _phy_utils_radio_h_ */
