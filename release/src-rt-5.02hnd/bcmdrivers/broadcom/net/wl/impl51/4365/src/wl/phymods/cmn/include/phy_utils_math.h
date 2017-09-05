/*
 * PHY utils - math library functions.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _phy_utils_math_h_
#define _phy_utils_math_h_

#include <typedefs.h>

#define	CORDIC_AG	39797
#define	CORDIC_NI	18
#define	FIXED(X)	((int32)((X) << 16))
#define	FLOAT(X)	(((X) >= 0) ? ((((X) >> 15) + 1) >> 1) : -((((-(X)) >> 15) + 1) >> 1))

typedef int32    math_fixed;	/* s15.16 fixed-point */

typedef struct _cint32 {
	math_fixed	q;
	math_fixed	i;
} math_cint32;

void phy_utils_computedB(uint32 *cmplx_pwr, int8 *p_cmplx_pwr_dB, uint8 core);
void phy_utils_cordic(math_fixed theta, math_cint32 *val);
void phy_utils_invcordic(math_cint32 val, int32 *angle);
uint8 phy_utils_nbits(int32 value);
uint32 phy_utils_sqrt_int(uint32 value);
uint32 phy_utils_qdiv(uint32 dividend, uint32 divisor, uint8 precision, bool round);
uint32 phy_utils_qdiv_roundup(uint32 dividend, uint32 divisor, uint8 precision);
void phy_utils_mat_rho(int64 *n, int64 *p, int64 *rho, int m);
void phy_utils_mat_transpose(int64 *a, int64 *b, int m, int n);
void phy_utils_mat_mult(int64 *a, int64 *b, int64 *c, int m, int n, int r);
void phy_utils_mat_inv_prod_det(int64 *a, int64 *b);
void phy_utils_mat_det(int64 *a, int64 *det);

#endif /* _phy_utils_math_h_ */
