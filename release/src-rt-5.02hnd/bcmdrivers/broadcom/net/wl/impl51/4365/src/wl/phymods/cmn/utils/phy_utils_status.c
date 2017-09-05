/*
 * PHY utils - status access functions.
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

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>

#include <phy_utils_reg.h>
#include <phy_utils_status.h>
#include <phy_utils_api.h>

#include <wlc_phy_int.h>

uint16
phy_utils_get_bwstate(phy_info_t *pi)
{
	return pi->bw;
}

void
phy_utils_set_bwstate(phy_info_t *pi, uint16 bw)
{
	pi->bw = bw;
}

bool
phy_utils_ismuted(phy_info_t *pi)
{
	return PHY_MUTED(pi);
}

uint64
phy_utils_get_time_usec(phy_info_t *pi)
{
	uint64 time_lo, time_hi;

	time_lo = R_REG(GENERIC_PHY_INFO(pi)->osh, &pi->regs->tsf_timerlow);
	time_hi = R_REG(GENERIC_PHY_INFO(pi)->osh, &pi->regs->tsf_timerhigh);

	return ((time_hi << 32) | time_lo);
}

bool
BCMATTACHFN(phy_utils_get_phyversion)(phy_info_t *pi, uint16 *phytype, uint16 *phyrev,
	uint16 *radioid, uint16 *radiover, uint16 *phy_minor_rev)
{
	*phytype = (uint16)pi->pubpi.phy_type;
	*phyrev = (uint16)pi->pubpi.phy_rev;
	*radioid = RADIOID(pi->pubpi.radioid);
	*radiover = RADIOREV(pi->pubpi.radiorev);
	*phy_minor_rev = (uint16)pi->pubpi.phy_minor_rev;

	return TRUE;
}

uint32
BCMATTACHFN(phy_utils_get_coreflags)(phy_info_t *pi)
{
	return pi->pubpi.coreflags;
}

uint8
BCMATTACHFN(phy_utils_get_corenum)(phy_info_t *pi)
{
	return PHYCORENUM(pi->pubpi.phy_corenum);
}
