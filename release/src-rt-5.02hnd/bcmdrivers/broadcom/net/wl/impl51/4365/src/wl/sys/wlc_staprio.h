/*
 * Common (OS-independent) portion of
 * Broadcom Station Prioritization Module
 *
 * This module is used to differentidate STA type (Video STA or Data Station) by setting
 * scb priority for each scb.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_staprio.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_sta_prio_h_
#define _wlc_sta_prio_h_

extern wlc_staprio_info_t *wlc_staprio_attach(wlc_info_t *wlc);
extern void wlc_staprio_detach(wlc_staprio_info_t *staprio);
extern bool wlc_get_scb_prio(wlc_staprio_info_t *staprio, struct scb *scb, uint8 *prio);

#endif  /* _wlc_sta_prio_h_ */
