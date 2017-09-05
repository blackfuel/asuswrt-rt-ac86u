/*
 * PHY Core module internal interface (to PHY specific implementations).
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

#ifndef _phy_type_h_
#define _phy_type_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_api.h>
#include "phy_shared.h"
#include <wlc_iocv_types.h>

/* PHY type specific implementation entry points */
typedef struct phy_type_info phy_type_info_t;
typedef struct {
	/* register PHY type specific implementation layer to common layer */
	int (*reg_impl)(phy_info_t *pi, phy_type_info_t *ti, int bandtype);
	/* unregister PHY type specific implementation layer from common layer */
	void (*unreg_impl)(phy_info_t *pi, phy_type_info_t *ti);
	/* reset PHY type specific implementation layer */
	void (*reset_impl)(phy_info_t *pi, phy_type_info_t *ti);
	/* register iovar tables to iocv module */
	int (*reg_iovt)(phy_info_t *pi, phy_type_info_t *ti, wlc_iocv_info_t *ii);
	/* register ioctl tables to iocv module */
	int (*reg_ioct)(phy_info_t *pi, phy_type_info_t *ti, wlc_iocv_info_t *ii);
	/* init PHY specfic s/w and/or h/w */
	int (*init_impl)(phy_info_t *pi, phy_type_info_t *ti);
	/* read phyreg for dump fn */
	uint16 (*read_phyreg)(phy_info_t *pi, phy_type_info_t *ti, uint addr);
	/* dump function for phyreg */
	int (*dump_phyregs)(phy_info_t *pi, phy_type_info_t *ti, struct bcmstrbuf *b);
	phy_type_info_t *ti;
} phy_type_fns_t;

/*
 * Register/unregister PHY Core PHY type specific implementation to PHY Core common layer.
 * It returns BCME_XXXX.
 */
int phy_register_impl(phy_info_t *pi, phy_type_fns_t *fns);
void phy_unregister_impl(phy_info_t *pi);

#endif /* _phy_type_h_ */
