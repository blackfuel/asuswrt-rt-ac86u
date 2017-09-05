/*****************************************************************************
 * wpa service (private)
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *****************************************************************************
*/

#if !defined(__WPA_SVCP_H__)
#define __WPA_SVCP_H__


struct cb_stack;

struct wpa_svc_dat {
	struct bind_sk *eapol_sk, *wlss_sk;
	void *eapol_binding, *wlss_binding;
#if defined(WPA_SVC_PRIVATE)
	struct wpa_dat wpa_dat;
#endif /* defined(WPA_SVC_PRIVATE) */
};

#endif /* !defined(__WPA_SVCP_H__) */
