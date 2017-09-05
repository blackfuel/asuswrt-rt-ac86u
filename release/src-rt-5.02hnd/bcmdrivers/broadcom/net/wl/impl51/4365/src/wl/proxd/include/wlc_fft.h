/*
 * TOF based proximity detection implementation for Broadcom 802.11 Networking Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_fft.h 580000 2015-08-17 22:08:34Z $
 */
#ifndef _wlc_fft_h
#define _wlc_fft_h

#include <typedefs.h>
#include <wlc_types.h>
#include <bcmutils.h>
#include <osl.h>
#include <wlc_phy_int.h>

#define LIMIT(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define ROUND(x, s) (((x) >> (s)) + (((x) >> ((s) - 1)) & (s != 0)))

extern int FFT64(osl_t *osh, cint32 *inBuf, cint32 *outBuf);
extern int FFT128(osl_t *osh, cint32 *inBuf, cint32 *outBuf);
extern int FFT256(osl_t *osh, cint32 *inBuf, cint32 *outBuf);
#ifdef TOF_SEQ_20_IN_80MHz
extern int FFT512(osl_t *osh, cint32 *inBuf, cint32 *outBuf);
#endif
extern int tof_pdp_ts(int log2n, void* pBuf, int FsMHz, int rx, void* pparams,
	int32* p_ts_thresh, int32* p_thresh_adj);
extern void wlapi_pdtof_fft(osl_t *osh, int n, void *inBuf, void *outBuf, int oversamp);
#endif /* _wlc_fft_h */
