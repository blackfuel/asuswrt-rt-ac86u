/*
 * WLC NDIS IOVAR module (iovars/ioctls that are used in between wlc and per port) of
 * Broadcom 802.11bang Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_ndis_iovar.h 499900 2014-09-01 14:56:02Z $
 *
 */

#ifndef _wlc_ndis_iovar_h_
#define _wlc_ndis_iovar_h_

extern int wlc_ndis_iovar_attach(wlc_info_t *wlc);
extern int wlc_ndis_iovar_detach(wlc_info_t *wlc);

extern si_t *wlc_get_sih(wlc_info_t *wlc);
extern bool wlc_cfg_associated(wlc_info_t *wlc);

#endif	/* _wlc_ndis_iovar_h_ */
