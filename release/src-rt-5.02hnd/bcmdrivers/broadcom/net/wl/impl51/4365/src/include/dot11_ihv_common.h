/*
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * Common IHV declarations shared between Native WiFi
 * IHV service and miniport.
 *
 * $Id: $
 */

#if !defined(__DOT11_IHV_COMMON_H__)
#define __DOT11_IHV_COMMON_H__

/* base is offset by 16 due to Cisco SDK */
#define DOT11_AUTH_ALGO_IHV_COMMON_BASE	(DOT11_AUTH_ALGO_IHV_START + 0x00000010L)
#define DOT11_CIPH_ALGO_IHV_COMMON_BASE	(DOT11_CIPHER_ALGO_IHV_START + 0x00000010L)

/* custom auth algorithms */
#if defined(BCMWAPI_WAI)
#define DOT11_AUTH_ALGO_WAPI 		(DOT11_AUTH_ALGO_IHV_COMMON_BASE + 0)
#define DOT11_AUTH_ALGO_WAPI_PSK	(DOT11_AUTH_ALGO_IHV_COMMON_BASE + 1)
#endif /* BCMWAPI_WAI */

/* custom cipher algorithms */
#if defined(BCMWAPI_WAI)
#define DOT11_CIPHER_ALGO_SMS4		(DOT11_CIPH_ALGO_IHV_COMMON_BASE + 0)
#endif /* BCMWAPI_WAI */

#endif /* !__DOT11_IHV_COMMON_H__ */
