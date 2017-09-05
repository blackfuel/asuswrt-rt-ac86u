/*
 * Describes PPI Capture format
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: PPIHeader.h 467328 2014-04-03 01:23:40Z $
 */
/* FILE-CSTYLED */

/* Refer to http://www.cacetech.com/documents/PPI Header format 1.0.7.pdf */

#include <sys/types.h>

#ifndef _PPIHEADER_H_
#define _PPIHEADER_H_

/* DLT_PPI defined in bpf.h */
#ifndef DLT_PPI
#define DLT_PPI 192
#endif

typedef struct ppi_packetheader {
	u_int8_t pph_version; /* Version. Currently 0 */
	u_int8_t pph_flags; /* Flags. */
	u_int16_t pph_len; /* Length of entire message,
			    * including this header and TLV
			    * payload. */
	u_int32_t pph_dlt; /* Data Link Type of the captured
			    * packet data. */
} ppi_packetheader_t;

/* pph_flags */
#define PPI_FLAG_ALIGN 0x01	/* Alignment. 32-bit aligned = 1, non-aligned = 0 */
#define IS_PPI_FLAG_ALIGN(x) ((x) & PPI_FLAG_ALIGN)

typedef struct ppi_fieldheader {
	u_int16_t pfh_type; /* Type */
	u_int16_t pfh_datalen; /* Length of data */
} ppi_fieldheader_t;

/* 802.11 Common field */
#define PPI_80211_COMMON_TYPE	2
#define PPI_80211_COMMON_LEN	20

typedef struct ppi_80211_common {
	ppi_fieldheader_t fld_hdr;
	u_int64_t       tsft;
	u_int16_t       flags;		/* See below */
	u_int16_t       rate;		/* In 500kbps */
	u_int16_t       channel_freq;	/* Frequency in MHz */
	u_int16_t       channel_flags;	/* same as radiotap */
	u_int8_t	fhss_hopset;
	u_int8_t	fhss_pattern;
	int8_t		dbm_antsignal;
	int8_t		dbm_antnoise;
} __attribute__((packed)) ppi_80211_common_t ;

/* flags in ppi_80211_common */
/* Packet flags LSB = bit 0. Bits:
   Bit 0 = If set, FCS present
   Bit 1 = If set to 1, the TSF-timer is in ms, if
   set to 0 the TSF-timer is in us
   Bit 2 = If set, the FCS is not valid
   Bit 3 = If set, there was a PHY error
   receiving the packet. If this bit is set, Bit 2
   is not relevant
*/
#define PPI_80211_COMMON_FCS		(1 << 0)
#define PPI_80211_COMMON_TSFT_MS 	(1 << 1)
#define PPI_80211_COMMON_INVALID_FCS	(1 << 2)
#define PPI_80211_COMMON_PHY_ERR	(1 << 3)

/* channel_flags in ppi_80211_common */
#define	PPI_80211_CHAN_TURBO	0x00000010 /* Turbo channel */
#define	PPI_80211_CHAN_CCK	0x00000020 /* CCK channel */
#define	PPI_80211_CHAN_OFDM	0x00000040 /* OFDM channel */
#define	PPI_80211_CHAN_2GHZ	0x00000080 /* 2 GHz spectrum channel. */
#define	PPI_80211_CHAN_5GHZ	0x00000100 /* 5 GHz spectrum channel */
#define	PPI_80211_CHAN_PASSIVE	0x00000200 /* Only passive scan allowed */
#define	PPI_80211_CHAN_DYN	0x00000400 /* Dynamic CCK-OFDM channel */
#define	PPI_80211_CHAN_GFSK	0x00000800 /* GFSK channel (FHSS PHY) */

/* 802.11n MAC+PHY Extension field */

#define PPI_HEADER_LEN (sizeof(ppi_packetheader_t))

#endif /* _PPIHEADER_H_ */
