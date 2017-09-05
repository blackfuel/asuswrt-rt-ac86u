/*
 * ACPHY Core module interface (to PHY Core module).
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

#ifndef _phy_type_ac_h_
#define _phy_type_ac_h_

#include <phy_api.h>
#include "phy_type.h"

/* attach/detach */
phy_type_info_t *phy_ac_attach(phy_info_t *pi, int bandtype);
void phy_ac_detach(phy_type_info_t *ti);

#endif /* _phy_type_ac_h_ */
