/*
 * ANTennaDIVersity module public interface (to MAC driver).
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

#ifndef _phy_antdiv_api_h_
#define _phy_antdiv_api_h_

#include <typedefs.h>
#include <phy_api.h>

/* set/get */
int phy_antdiv_set_rx(phy_info_t *pi, uint8 ant);
void phy_antdiv_get_rx(phy_info_t *pi, uint8 *ant);

#ifdef WLC_SW_DIVERSITY
void phy_antdiv_set_sw(phy_info_t *pi, uint8 ant);
uint8 phy_antdiv_get_sw(phy_info_t *pi);
#endif

#endif /* _phy_antdiv_api_h_ */
