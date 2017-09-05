#ifdef BCMCCX

/*
 *   bcmccx.h - Prototypes for BCM CCX utility functions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: bcmccx.h 451682 2014-01-27 20:30:17Z $
 */

#ifndef _BCM_CCX_H_
#define _BCM_CCX_H_

#include <proto/eap.h>

#define CCX_PW_HASH_LEN		16 /* CCX Password Hash length */
#define CCX_SESSION_KEY_LEN	16 /* CCX session Key lenght */

#define CKIP_LLC_SNAP_LEN	8 /* SKIP LLC SNAP header length */

#define	BCM_CKIP_RXSEQ_WIN	32	/* rx sequence number window buffer size */

/* Pick a target position in the RXSEQ window for the latest sequence number
 * when the window needs to be moved. Below is 3/4 of the way through the window.
 */
#define BCM_CKIP_RXSEQ_WIN_TARGET	(BCM_CKIP_RXSEQ_WIN - BCM_CKIP_RXSEQ_WIN/4 - 1)

/* bcm_ckip_rxseq_check() uses the 32 bit sequence number shifed down 1 bit */
#define BCM_CKIP_RXSEQ_MASK		0x7fffffff /* Mask for CKIP RxSEQ */

/* bcm_ckip_rxseq_check() uses a uint32 for the BCM_CKIP_RXSEQ_WIN bitmap,
 * so make sure the window fits in the bitmap
 */
#if (BCM_CKIP_RXSEQ_WIN > 32)
#error "BCM_CKIP_RXSEQ_WIN does not fit in 32 bits"
#endif /* (BCM_CKIP_RXSEQ_WIN > 32) */

/* Get the MD4 hash and hash-hash of a password. */
extern void bcm_ccx_hashpwd(uint8 *pwd, size_t pwdlen,
                                      uint8 hash[CCX_PW_HASH_LEN],
                                      uint8 hashhash[CCX_PW_HASH_LEN]);

/* Apply MD5 to get the CCX session key */
extern void bcm_ccx_session_key(uint8 *inbuf, size_t in_len,
                                          uint8 outbuf[CCX_SESSION_KEY_LEN]);

/* Derive LEAP response from LEAP challenge and password hash */
extern void bcm_ccx_leap_response(uint8 pwhash[CCX_PW_HASH_LEN],
                                            uint8 challenge[LEAP_CHALLENGE_LEN],
                                            uint8 response[LEAP_RESPONSE_LEN]);

/* allow out-of-order SEQ due to QoS, seq number increase/reset per key */
extern bool bcm_ckip_rxseq_check(uint32 seq_odd, uint32 *rxseq_base,
                                           uint32 *rxseq_bitmap);

#endif /* _BCM_CCX_H_ */

#endif /* BCMCCX */
