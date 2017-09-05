/*
 * MU-MIMO module public interface (to MAC driver).
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: phy_mu_api.h 525162 2015-01-09 02:07:08Z $
 */

#ifndef _phy_mu_api_h_
#define _phy_mu_api_h_

#include <typedefs.h>
#include <d11.h>
#include <phy_api.h>

/* Set or clear MU-MIMO group membership for a given group. When this station
 * is a member of the given group, set the user position as well.
 * Inputs:
 *   pi       - phy info
 *   mu_group - MU-MIMO group ID [1, 62]
 *   user_pos - user position within this group (i.e., which of the NSTS fields
 *              within VHT-SIG-A specifies the number of spatial streams to demodulate.)
 *              Unused if is_member is 0.
 *   is_member - 1 if this station is a member of this group
 *               0 otherwise
 *
 * Returns:
 *   BCME_OK if hardware is successfully updated
*/
int phy_mu_group_set(phy_info_t *pi, uint16 mu_group, uint8 user_pos, uint8 is_member);

#endif /* _phy_mu_api_h_ */
