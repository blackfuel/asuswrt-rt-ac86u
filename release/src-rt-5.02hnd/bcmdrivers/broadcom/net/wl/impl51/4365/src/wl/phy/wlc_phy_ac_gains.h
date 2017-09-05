/*
 * ACPHY gain table module header file
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

#ifndef _wlc_phy_ac_gains_h_
#define _wlc_phy_ac_gains_h_

#include "wlc_phy_types.h"
#include "wlc_phy_int.h"

extern void wlc_phy_ac_gains_load(phy_info_t *pi);
extern void wlc_phy_tx_gain_table_write_acphy(phy_info_t *pi, uint32 l,
	uint32 o, uint32 w, const void *d);

extern void BCMATTACHFN(wlc_phy_ac_delete_gain_tbl)(phy_info_t *pi);
extern void BCMATTACHFN(wlc_phy_set_txgain_tbls)(phy_info_t *pi);

#endif /* _wlc_phy_ac_gains_h_ */
