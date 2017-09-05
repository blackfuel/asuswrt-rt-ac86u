/*
 * RADIO control module public interface (to MAC driver).
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

#ifndef _phy_radio_api_h_
#define _phy_radio_api_h_

#include <typedefs.h>
#include <phy_api.h>

/* switch the radio on/off */
void phy_radio_switch(phy_info_t *pi, bool on);

/* switch the radio off when switching band */
void phy_radio_xband(phy_info_t *pi);

/* switch the radio off when initializing */
void phy_radio_init(phy_info_t *pi);

#endif /* _phy_radio_api_h_ */
