/*
 * Broadcom 802.11 Networking Device Driver
 *
 * Functionality relating to calculating the txtime
 * of frames, frame components, and frame exchanges.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_txtime.h 492910 2014-07-23 23:29:55Z $
 */

/**
 * @file
 * @brief TX airtime calculation
 * Functionality relating to calculating the airtime
 * of frames, frame components, and frame exchanges.
 */

#ifndef _wlc_txtime_h_
#define _wlc_txtime_h_

#include "typedefs.h"
#include "wlc_types.h"
#include "wlc_rate.h"

/**
 * @brief a structure to hold parameters for packet tx time calculations
 *
 * This structure holds parameters for the time calculation and packet independent
 * partial results. This allows less work per-packet to calculation tx time.
 */
typedef struct timecalc {
	ratespec_t rspec;       /**< @brief ratespec for time calulations */
	uint fixed_overhead_us; /**< @brief length independent fixed time overhead */
	int is2g;               /**< @brief true for 2GHz band calculation */
	int short_preamble;     /**< @brief true if using short preamble */
} timecalc_t;

/**
 * @brief Calculate the airtime for a frame including preamble
 *
 * Calculate the airtime for a frame including preamble
 *
 * Calculation follows the TXTIME primitive for the PHYs we support where the TXVECTOR parmeters
 * are provided in ratespec, preamble_type, and psdu_len.
 *
 * @param	ratespec	the phy rate and other modulation parameters for frame
 * @param	band2g		true if frame is on 2.4G band
 * @param	short_preamble	For DSSS rates, 1 indicates Short Preamble, 0 indicates
 *                              Long Preamble.
 *				For HT, 1 indicates GreenField preamble, 0 indicates Mixed Mode.
 *                              For VHT, this parameter is 0 since VHT always uses Mixed Mode.
 *                              For OFDM, this parameter is 0 since there is only one preamble type.
 * @param	psdu_len	count of bytes in Phy Service Data Unit. For a simple MSDU frame
 *				this is the length from 802.11 FC field up to and including the FCS.
 *				For A-MPDUs, this is the total length of the A-MPDU adding MPDU
 *				delimiter fields, MPDU sub frame padding, and EOF padding
 *
 * @return	return value is the time in microseconds to transmit the entire frame
 *
 */
uint wlc_txtime(ratespec_t ratespec, bool band2g, int short_preamble, uint psdu_len);


/**
 * @brief Calculate the airtime for just the given byte length of a DATA portion of a PPDU
 *
 * Calculate the airtime for just the given byte length of a DATA portion of a PPDU
 *
 * Calculation follows the TXTIME primitive for the PHYs we support where the relavent TXVECTOR
 * parmeters are provided in ratespec. The time is the time for the number of symbols in the
 * DATA portion of the PPDU where the MSDU frame bytes are encoded.
 *
 * @param	ratespec	the phy rate and other modulation parameters for frame
 * @param	data_len	count of bytes for which to calculate the airtime
 *
 * @return	return value is the time in microseconds to transmit the given data portion
 *              of a frame
 *
 */
uint wlc_txtime_data(ratespec_t ratespec, uint data_len);


/**
 * @brief Calculate Ndbps (Number of Data Bits Per Symbol) value for the given ratespec.
 *
 * Calculate Ndbps (Number of Data Bits Per Symbol) value for the given ratespec.
 *
 * Return Ndbps (Number of Data Bits Per Symbol) value for the given ratespec
 * Note that the symbol time for OFDM, HT, and VHT is 4us, or 3.6us when using SGI,
 * and DSSS symbol time is 1us. Ndbps for DSSS rate 5.5Mbps will evaluate to 5.
 *
 * @param	ratespec	The ratespec specifying the modulation for which Ndbps is desired.
 *
 * @return	return value is the number of data bits per symbol
 *
 */
uint wlc_txtime_Ndbps(ratespec_t ratespec);

/**
 * @brief Calculate Nes (Number of Encoding Streams) value for the given ratespec.
 *
 * Calculate Nes (Number of Encoding Streams) value for the given ratespec.
 *
 * @param	ratespec	The ratespec specifying the modulation for which Nes is desired.
 *
 * @return	return value is the number of encoding streams
 */
uint wlc_txtime_Nes(ratespec_t ratespec);

#endif /* _wlc_txtime_h_ */
