/*
 * PHYComMoN module internal interface (to other PHY modules).
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

#ifndef _phy_cmn_h_
#define _phy_cmn_h_

#include <phy_api.h>


struct phy_cmn_info {
	shared_phy_t	*sh;
	phy_info_t	*pi[MAX_RSDB_MAC_NUM];
	uint8		num_d11_cores;
	uint16		phymode;
};

typedef struct phy_cmn_info phy_cmn_info_t;

/* attach/detach */
phy_cmn_info_t *phy_cmn_attach(phy_info_t *pi);
void phy_cmn_detach(phy_cmn_info_t *ci);

/* query object */
typedef enum phy_obj_type {
	PHY_OBJ_RADAR_DETECT = 0
} phy_obj_type_t;

typedef struct phy_obj_ptr phy_obj_ptr_t;

int phy_cmn_register_obj(phy_cmn_info_t *ci, phy_obj_ptr_t *obj, phy_obj_type_t type);
phy_obj_ptr_t *phy_cmn_find_obj(phy_cmn_info_t *ci, phy_obj_type_t type);
phy_info_t *phy_get_other_pi(phy_info_t *pi);
uint8 phy_get_current_core(phy_info_t *pi);

#endif /* _phy_cmn_h_ */
