/*
 * Copyright 1998 Epigram, Inc.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: smean.h 241182 2011-02-17 21:50:03Z $
 *
 */
#ifndef _SMEAN_H_
#define _SMEAN_H_ 1

/*
 * A small package for keeping smoothed mean values and
 * smoothed mean deviations, based loosely on the
 * floating point examples of TCP round trip time functions.
 *
 * All integer arithmetic.
 * Sample values are integers.
 * Frac_Bits is the number of bits to the right of the decimal point.
 * Div_Bits is the log-base-2 of the divisor used for smoothing. It corresponds
 *    loosely to the number of samples included in the smoothed estimate.
 * The get_smean() and get_sdev() functions return integers.
 *
 * Note: There are no overflow checks at present.
 */

/*
 * A structure for use with general purpose subroutines.
 */
typedef struct smeandev2_struct
{
	unsigned char	div_bits;	/* power of 2 */
	unsigned char	frac_bits;	/* extra bits instead of real rounding */
	int	smean;		/* smoothed mean * 2^frac_bits */
	int	smdev;		/* smoothed mean deviation * 2^frac_bits */
} smeandev2_t;

/*
 * A structure for use with parameterized macros.
 */
typedef struct smeandev_struct
{
	int	smean;		/* smoothed mean * 2^frac_bits */
	int	smdev;		/* smoothed mean deviation * 2^frac_bits */
} smeandev_t;

void
init_smean_sdev(unsigned int frac_bits, unsigned int div_bits, smeandev2_t *p, int startval);

void
update_smean_sdev(smeandev2_t *p, int nextval);

int
get_smean(smeandev2_t *p);

int
get_smdev(smeandev2_t *p);


#define INIT_SMEAN_SDEV(frac_bits, div_bits, p, startval)		\
		do {							\
			(p)->smean = (startval) << (frac_bits);		\
			(p)->smdev = 0;					\
		} while (0)

#define UPDATE_SMEAN_SDEV(frac_bits, div_bits, p, nextval)		\
		do {							\
			int	err;					\
									\
			err = ((nextval) << (frac_bits)) - (p)->smean;	\
			(p)->smean += err >> (div_bits);		\
			if (err < 0) {					\
				err = -err;				\
			}						\
			err -= (p)->smdev;				\
			(p)->smdev += err >> (div_bits);		\
		} while (0)

#define GET_SMEAN(frac_bits, div_bits, p) \
		(((p)->smean + ((1 << (frac_bits)) >> 1)) >> (frac_bits))

#define GET_SMDEV(frac_bits, div_bits, p) \
		(((p)->smdev + ((1 << (frac_bits)) >> 1)) >> (frac_bits))

#endif	/* _SMEAN_H_ */
