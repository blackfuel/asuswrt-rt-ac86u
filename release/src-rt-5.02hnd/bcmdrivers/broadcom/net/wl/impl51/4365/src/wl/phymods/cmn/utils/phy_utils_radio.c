/*
 * RADIO control module implementation - shared by PHY type specific implementations.
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

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <phy_api.h>
#include <phy_utils_radio.h>

#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>

void
phy_utils_parse_idcode(phy_info_t *pi, uint32 idcode)
{
	pi->pubpi.radioid = (idcode & IDCODE_ID_MASK) >> IDCODE_ID_SHIFT;
	pi->pubpi.radiorev = (idcode & IDCODE_REV_MASK) >> IDCODE_REV_SHIFT;
	pi->pubpi.radiover = (idcode & IDCODE_VER_MASK) >> IDCODE_VER_SHIFT;

#if defined(DSLCPE) && defined(CONFIG_BCM963268)
	if (CHIPID(pi->sh->chip) == BCM6362_CHIP_ID) {
		/* overriding radiover to 8 */
		pi->pubpi.radiorev = 8;
	}
#endif /* defined(DSLCPE) && defined(CONFIG_BCM963268) */
}

int
phy_utils_valid_radio(phy_info_t *pi)
{
	if (VALID_RADIO(pi, RADIOID(pi->pubpi.radioid))) {
		/* ensure that the built image matches the target */
#ifdef BCMRADIOID
		if (pi->pubpi.radioid != BCMRADIOID)
			PHY_ERROR(("%s: Chip's radioid=0x%x, BCMRADIOID=0x%x\n",
			           __FUNCTION__, pi->pubpi.radioid, BCMRADIOID));
		ASSERT(pi->pubpi.radioid == BCMRADIOID);
#endif
#ifdef BCMRADIOREV
		if (pi->pubpi.radiorev != BCMRADIOREV)
			PHY_ERROR(("%s: Chip's radiorev=%d, BCMRADIOREV=%d\n",
			           __FUNCTION__, pi->pubpi.radiorev, BCMRADIOREV));
		ASSERT(pi->pubpi.radiorev == BCMRADIOREV);
#endif
		return BCME_OK;
	} else {
		PHY_ERROR(("wl%d: %s: Unknown radio ID: 0x%x rev 0x%x phy %d, phyrev %d\n",
		           pi->sh->unit, __FUNCTION__,
		           RADIOID(pi->pubpi.radioid), RADIOREV(pi->pubpi.radiorev),
		           pi->pubpi.phy_type, pi->pubpi.phy_rev));
		return BCME_UNSUPPORTED;
	}
}
