/*
 * HTPHY Core module internal interface (to other PHY modules).
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

#ifndef _phy_ht_h_
#define _phy_ht_h_

#ifdef ALL_NEW_PHY_MOD
typedef struct phy_ht_info phy_ht_info_t;
#else
/* < TODO: all these are going away... */
typedef struct phy_info_htphy phy_ht_info_t;
/* TODO: all these are going away... > */
#endif /* ALL_NEW_PHY_MOD */

#endif /* _phy_ht_h_ */
