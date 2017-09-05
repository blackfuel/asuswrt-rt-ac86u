/*****************************************************************************
 * wps service (private)
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

#if !defined(__wps_svcp_h__)
#define __wps_svcp_h__


struct wps_svc_dat {
	struct bind_sk *eapol_sk, *wlss_sk;
	void *eapol_binding, *wlss_binding;
#if defined(WPS_SVC_PRIVATE)
	struct wps_dat wps_dat;
#endif /* defined(WPS_SVC_PRIVATE) */
};

#endif /* !defined(__wps_svcp_h__) */
