/*
 * PHY Core module internal interface - connect PHY type specific layer to common layer.
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

#ifndef _phy_type_disp_h_
#define _phy_type_disp_h_

#include <typedefs.h>
#include <phy_api.h>
#include "phy_type.h"

#include <wlc_iocv_types.h>

/* forward declaration */
typedef struct phy_type_disp phy_type_disp_t;

/* Attach/detach PHY type specific implementation dispatch info */
phy_type_disp_t *phy_type_disp_attach(phy_info_t *pi);
void phy_type_disp_detach(phy_type_disp_t *disp);

/*
 * Attach/detach PHY type specific implementation.
 *
 * Call phy_type_attach() after all PHY modules are attached to the system.
 * Call phy_type_detach() before any PHY module detaches from the system.
 */
phy_type_info_t *phy_type_attach(phy_type_disp_t *disp, int bandtype);
void phy_type_detach(phy_type_disp_t *disp, phy_type_info_t *ti);

/*
 * Register/unregister PHY type specific implementations to/from PHY modules.
 *
 * Return BCME_OK when all registrations are successfully done; BCME_XXXX otherwise.
 *
 * Call phy_type_register_impl() after all PHY modules are attached to the system.
 * Call phy_type_unregister_impl() before any PHY module detaches from the system.
 */
int phy_type_register_impl(phy_type_disp_t *disp, int bandtype);
void phy_type_unregister_impl(phy_type_disp_t *disp);

/*
 * Reset h/w and/or s/w states upon attach
 */
void phy_type_reset_impl(phy_type_disp_t *disp);

/*
 * Init h/w and/or s/w states upon init (band switch)
 */
int phy_type_init_impl(phy_type_disp_t *disp);

/*
 * Register PHY type specific implementation iovar tables/handlers.
 *
 * Call phy_type_register_iovt() after all PHY modules are attached to the system.
 */
int phy_type_register_iovt(phy_type_disp_t *disp, wlc_iocv_info_t *ii);

/*
 * Register PHY type specific implementation ioctl tables/handlers.
 *
 * Call phy_type_register_ioct() after all PHY modules are attached to the system.
 */
int phy_type_register_ioct(phy_type_disp_t *disp, wlc_iocv_info_t *ii);

#if ((defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)) || \
	defined(BCMDBG_PHYDUMP)
/* dump phy type specific phy registers */
uint16 phy_type_read_phyreg(phy_type_disp_t *disp, uint addr);
int phy_type_dump_phyregs(phy_type_disp_t *disp, struct bcmstrbuf *b);
#endif 

#endif /* _phy_type_disp_h_ */
