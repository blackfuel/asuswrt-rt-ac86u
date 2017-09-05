/*
 * PHY Core module public interface (to MAC driver).
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

#ifndef _phy_api_h_
#define _phy_api_h_

#include <typedefs.h>
#include <bcmwifi_channels.h>

#ifdef ALL_NEW_PHY_MOD
typedef struct phy_shared_info phy_shared_info_t;
typedef struct phy_info phy_info_t;
#else
#include <wlc_phy_types.h>
#include <wlc_phy_hal.h>
#endif

#include <wlc_iocv_types.h>

/*
 * Attach/detach all PHY modules to/from the system.
 */
phy_info_t *phy_module_attach(shared_phy_t *sh, void *regs, int bandtype, char *vars);
void phy_module_detach(phy_info_t *pi);

/*
 * Register all iovar/ioctl tables/handlers to/from the system.
 */
int phy_register_iovt_all(phy_info_t *pi, wlc_iocv_info_t *ii);
int phy_register_ioct_all(phy_info_t *pi, wlc_iocv_info_t *ii);

/*
 * TODO: These functions should be registered to bmac in phy_module_attach(),
 * which requires bmac to have some registration infrastructure...
 */

/*
 * Init/deinit the PHY h/w.
 */
/* band specific init */
void phy_bsinit(phy_info_t *pi, chanspec_t chanspec, bool forced);
/* band width init */
void phy_bwinit(phy_info_t *pi, chanspec_t chanspec);
/* generic init */
void phy_init(phy_info_t *pi, chanspec_t chanspec);
/* generic deinit */
int phy_down(phy_info_t *pi);

/* Publish phyAPI's here.. */
#define PHY_RSBD_PI_IDX_CORE0 0
#define PHY_RSBD_PI_IDX_CORE1 1

void phy_set_phymode(phy_info_t *pi, uint16 new_phymode);
uint16 phy_get_phymode(const phy_info_t *pi);
phy_info_t *phy_get_pi(const phy_info_t *pi, int idx);
bool phy_init_pending(phy_info_t *pi);
mbool phy_get_measure_hold_status(phy_info_t *pi);
void phy_set_measure_hold_status(phy_info_t *pi, mbool set);
#endif /* _phy_api_h_ */
