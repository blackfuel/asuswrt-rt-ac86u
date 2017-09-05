/**
 * AKM (Authentication and Key Management) IE management module interface, 802.1x related.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_akm_ie.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_akm_ie_h_
#define _wlc_akm_ie_h_

extern wlc_akm_info_t *wlc_akm_attach(wlc_info_t *wlc);
extern void wlc_akm_detach(wlc_akm_info_t *akm);

#endif /* _wlc_akm_ie_h_ */
