/*
 * Utility module public interface (to MAC driver).
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

#ifndef _phy_utils_api_h_
#define _phy_utils_api_h_

#include <typedefs.h>
#include <bcmdefs.h>

#include <wlc_phy_hal.h>

uint16 phy_utils_get_bwstate(phy_info_t *pi);
void phy_utils_set_bwstate(phy_info_t *pi, uint16 bw);
bool phy_utils_ismuted(phy_info_t *pi);
bool phy_utils_get_phyversion(phy_info_t *pi, uint16 *phytype, uint16 *phyrev,
	uint16 *radioid, uint16 *radiover, uint16 *phy_minor_rev);
bool phy_utils_get_encore(phy_info_t *pi);
uint32 phy_utils_get_coreflags(phy_info_t *pi);
uint8 phy_utils_get_corenum(phy_info_t *pi);

chanspec_t phy_utils_get_chanspec(phy_info_t *pi);
void phy_utils_chanspec_band_validch(phy_info_t *pi, uint band, chanvec_t *channels);
chanspec_t phy_utils_chanspec_band_firstch(phy_info_t *pi, uint band);

#endif /* _phy_utils_api_h_ */
