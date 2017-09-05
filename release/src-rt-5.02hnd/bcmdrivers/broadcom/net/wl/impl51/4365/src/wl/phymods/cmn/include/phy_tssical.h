/*
 * TSSI Cal module interface (to other PHY modules).
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

#ifndef _phy_tssical_h_
#define _phy_tssical_h_

#include <phy_api.h>

typedef int phy_tssical_entry_id_t;

/* forward declaration */
typedef struct phy_tssical_info phy_tssical_info_t;

/* attach/detach */
phy_tssical_info_t *phy_tssical_attach(phy_info_t *pi);
void phy_tssical_detach(phy_tssical_info_t *cmn_info);

/* down */
int phy_tssical_down(phy_tssical_info_t *cmn_info);
#endif /* _phy_tssical_h_ */
