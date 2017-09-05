/*
 * Channel Manager module public interface (to MAC driver).
 *
 * Controls radio chanspec and manages related operating chanpsec context.
 *
 * Operating chanspec context:
 *
 * A Operating chanspec context is a collection of s/w properties
 * associated with a radio chanspec.
 *
 * Current operating chanspec context:
 *
 * The current operating chanspec context is the operating chanspec context
 * whose associated chanspec is used as the current radio chanspec, and
 * whose s/w properties are applied to the corresponding h/w if any.
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

#ifndef _phy_chanmgr_api_h_
#define _phy_chanmgr_api_h_

#include <typedefs.h>
#include <bcmwifi_channels.h>
#include <phy_api.h>

/*
 * Create/Destroy an operating chanspec context for radio 'chanspec'.
 */
int phy_chanmgr_create_ctx(phy_info_t *pi, chanspec_t chanspec);
void phy_chanmgr_destroy_ctx(phy_info_t *pi, chanspec_t chanspec);

/*
 * Set the operating chanspec context associated with the 'chanspec'
 * as the current operating chanspec context, and set the 'chanspec'
 * as the current radio chanspec.
 */
int phy_chanmgr_set_oper(phy_info_t *pi, chanspec_t chanspec);

/*
 * Set the radio chanspec to 'chanspec', and unset the current
 * operating chanspec context if any.
 */
int phy_chanmgr_set(phy_info_t *pi, chanspec_t chanspec);

#endif /* _phy_chanmgr_api_h_ */
