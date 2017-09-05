/*
 * Link Power Control module interface (to other PHY modules).
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

#ifndef _phy_lpc_h_
#define _phy_lpc_h_

#include <phy_api.h>

/* forward declaration */
typedef struct phy_lpc_info phy_lpc_info_t;

/* attach/detach */
phy_lpc_info_t *phy_lpc_attach(phy_info_t *pi);
void phy_lpc_detach(phy_lpc_info_t *cmn_info);

/* up/down */
int phy_lpc_init(phy_lpc_info_t *cmn_info);
int phy_lpc_down(phy_lpc_info_t *cmn_info);

#endif /* _phy_lpc_h_ */
