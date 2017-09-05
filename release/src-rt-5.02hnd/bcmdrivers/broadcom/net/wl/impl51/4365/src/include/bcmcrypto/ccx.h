#ifdef BCMCCX

/*
 * ccx.h.c
 * Header file for CCX crypto functions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: ccx.h 451682 2014-01-27 20:30:17Z $
 */

#ifndef _CCX_H_
#define _CCX_H_

extern void CKIP_key_permute(uint8 *PK,		/* output permuted key */
				       const uint8 *CK,		/* input CKIP key */
				       uint8 toDsFromDs,	/* input toDs/FromDs bits */
				       const uint8 *piv);	/* input pointer to IV */

extern int wsec_ckip_mic_compute(const uint8 *CK, const uint8 *pDA, const uint8 *pSA,
                                           const uint8 *pSEQ, const uint8 *payload, int payloadlen,
                                           const uint8 *p2, int len2, uint8 pMIC[]);
extern int wsec_ckip_mic_check(const uint8 *CK, const uint8 *pDA, const uint8 *pSA,
                                         const uint8 *payload, int payloadlen, uint8 mic[]);

/* CCX v2 */

/* Key lengths in bytes */
#define CCKM_GK_LEN	48
#define CCKM_KRK_LEN	16
#define CCKM_BTK_LEN	32
#define CCKM_TKIP_PTK_LEN	64
#define CCKM_CKIP_PTK_LEN	CCKM_TKIP_PTK_LEN
#define CCKM_CCMP_PTK_LEN	48

#endif /* _CCX_H_ */

#endif /* BCMCCX */
