/*
 * TXIQLO CAL module interface (to other PHY modules).
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

#ifndef _phy_txiqlocal_h_
#define _phy_txiqlocal_h_

#include <phy_api.h>

/* forward declaration */
typedef struct phy_txiqlocal_info phy_txiqlocal_info_t;

/* attach/detach */
phy_txiqlocal_info_t *phy_txiqlocal_attach(phy_info_t *pi);
void phy_txiqlocal_detach(phy_txiqlocal_info_t *cmn_info);

/* init */

/* phy txcal coeffs structure used for HTPHY */
typedef struct txcal_coeffs {
	uint16 txa;
	uint16 txb;
	uint16 txd;	/* contain di & dq */
	uint8 txei;
	uint8 txeq;
	uint8 txfi;
	uint8 txfq;
	uint16 rxa;
	uint16 rxb;
} txcal_coeffs_t;

#endif /* _phy_txiqlocal_h_ */
