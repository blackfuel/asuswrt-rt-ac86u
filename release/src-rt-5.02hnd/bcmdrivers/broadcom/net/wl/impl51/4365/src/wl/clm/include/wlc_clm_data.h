/*
 * CLM Data structure definitions
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wlc_clm_data.h 540418 2015-03-12 01:36:03Z $
 */

#ifndef _WLC_CLM_DATA_H_
#define _WLC_CLM_DATA_H_

#include <bcmwifi_rates.h>


#define CLMATTACHDATA(_data) __attribute__ ((__section__ (".clmdataini2." #_data))) _data

extern const struct clm_data_header clm_header;
extern const struct clm_data_header clm_inc_header;

/*
***************************
* CLM DATA BLOB CONSTANTS *
***************************
*/

/* CLM header tag that designates the presence of CLM data (useful for
 * incremental CLM data)
 */
#define CLM_HEADER_TAG "CLM DATA"

/* CLM apps version that indicates the data is vanilla Broadcom, direct from
 * Regulatory
 */
#define CLM_APPS_VERSION_NONE_TAG "Broadcom-0.0"

/** Constants, related to CLM data BLOB */
enum clm_data_const {
	/* SPECIAL INDEX VALUES USED IN BLOB */

	/** Channel range ID that designates all channels */
	CLM_RANGE_ALL_CHANNELS = 0xFF,

	/** Restricted set that covers all channels */
	CLM_RESTRICTED_SET_NONE = 0xFF,


	/** Locale index that designates absence of locale. If all locales of
	 * region are absent region considered to be deleted from incremental
	 * BLOB
	 */
	CLM_LOC_NONE = 0xFFF,

	/** Locale index that designates that region in incremental data uses
	 * the same locale as correspondent base locale
	 */
	CLM_LOC_SAME = 0xFFE,

	/** Locale index marks region as deleted (used in incremental data) */
	CLM_LOC_DELETED = 0xFFD,

	/** Region revision that marks mapping, deleted in incremental
	 * aggregation
	 */
	CLM_DELETED_MAPPING = 0xFFFFu,

	/** Value for 'num' field that marks item as deleted (used in
	 * incremental data for clm_advertised_cc_set and clm_aggregate_cc_set
	 * structures)
	 */
	CLM_DELETED_NUM = -1,

	/** Base for subchannel rules indices */
	CLM_SUB_CHAN_IDX_BASE = 1,


	/* INDICES OF LOCALE TYPES IN VECTORS OF PER-TYPE LOCALE DEFINITIONS */

	/** Index of 2.4GHz base locales in vectors of locale definitions */
	CLM_LOC_IDX_BASE_2G = 0,

	/** Index of 5GHz base locales in vectors of locale definitions */
	CLM_LOC_IDX_BASE_5G = 1,

	/** Index of 2.4GHz HT locales in vectors of locale definitions */
	CLM_LOC_IDX_HT_2G = 2,

	/** Index of 5GHz HT locales in vectors of locale definitions */
	CLM_LOC_IDX_HT_5G = 3,

	/** Number of locale type indices (length of locale definition
	 * vectors)
	 */
	CLM_LOC_IDX_NUM = 4,


	/* INDICES OF VARIOUS DATA BYTES IN PARTS OF LOCALE DEFINITION */

	/** Index of locale valid channel index in locale definition header */
	CLM_LOC_DSC_CHANNELS_IDX = 0,

	/** Index of locale flags in base locale definition header */
	CLM_LOC_DSC_FLAGS_IDX  = 1,

	/** Index of restricted set index byte in base locale definition
	 * header
	 */
	CLM_LOC_DSC_RESTRICTED_IDX = 2,

	/** Index of transmission (in quarter of dBm) or public power (in dBm)
	 * in public or transmission record respectively
	 */
	CLM_LOC_DSC_POWER_IDX = 0,

	/** Index of channel range index in public or transmission record */
	CLM_LOC_DSC_RANGE_IDX = 1,

	/** Index of rates set index in transmission record */
	CLM_LOC_DSC_RATE_IDX = 2,

	/** Index of power for antenna index 1 */
	CLM_LOC_DSC_POWER1_IDX = 3,

	/** Index of power for antenna index 2 */
	CLM_LOC_DSC_POWER2_IDX = 4,

	/** Index of power for antenna index 3 */
	CLM_LOC_DSC_POWER3_IDX = 5,

	/** Length of base locale definition header */
	CLM_LOC_DSC_BASE_HDR_LEN = 3,

	/** Length of public power record */
	CLM_LOC_DSC_PUB_REC_LEN = 2,

	/** Length of transmission power records' header */
	CLM_LOC_DSC_TX_REC_HDR_LEN = 2,

	/** Length of transmission power record without additional powers */
	CLM_LOC_DSC_TX_REC_LEN = 3,


	/** INDICES INSIDE SUBCHANNEL RULE (CLM_SUB_CHAN_CHANNEL_RULES_XX_T
	 * STRUCTURE)
	 */

	/** Index of chan_range field */
	CLM_SUB_CHAN_RANGE_IDX = 0,

	/** Index of sub_chan_rules field */
	CLM_SUB_CHAN_RULES_IDX = 1,

	/** Disabled power */
	CLM_DISABLED_POWER = 0x80u,

	/* FLAGS USED IN REGISTRY */

	/** Country (region) record with 10 bit locale indices and flags are
	 * used
	 */
	CLM_REGISTRY_FLAG_COUNTRY_10_FL = 0x00000001,

	/** BLOB header contains SW APPS version field */
	CLM_REGISTRY_FLAG_APPS_VERSION = 0x00000002,

	/** BLOB contains subchannel rules */
	CLM_REGISTRY_FLAG_SUB_CHAN_RULES = 0x00000004,

	/** BLOB contains 160MHz data */
	CLM_REGISTRY_FLAG_160MHZ = 0x00000008,

	/** BLOB contains per-bandwidth ranges and rate sets */
	CLM_REGISTRY_FLAG_PER_BW_RS = 0x00000010,

	/** BLOB contains per-bandwidth-band rate sets */
	CLM_REGISTRY_FLAG_PER_BAND_RATES = 0x00000020,

	/** BLOB contains user string */
	CLM_REGISTRY_FLAG_USER_STRING = 0x00000040,

	/** BLOB combs for 40, 80, 160 MHz channels */
	CLM_REGISTRY_FLAG_HIGH_BW_COMBS = 0x00000080,

	/** Field that contains number of rates in clm_rates enum (mask and
	 * shift)
	 */
	CLM_REGISTRY_FLAG_NUM_RATES_MASK = 0x0000FF00,
	CLM_REGISTRY_FLAG_NUM_RATES_SHIFT = 8,

	/** Context-dependent region definition record used (record length
	 * depends upon number of bytes needed for locale indices and rev,
	 * which depends upon BLOB contents. Also this flag forces use of
	 * cc/rev indices (instead of cc/revs themselves) in aggregation
	 * definitions
	 */
	CLM_REGISTRY_FLAG_CD_REGIONS = 0x00010000,

	/** Field that contains number of extra bytes (0, 1, 2) in region
	 * record that contain locale indices
	 */
	CLM_REGISTRY_FLAG_CD_LOC_IDX_BYTES_MASK  = 0x000C0000,
	CLM_REGISTRY_FLAG_CD_LOC_IDX_BYTES_SHIFT = 18,

	/** 16-bit region index. Used only if CLM_REGISTRY_FLAG_CD_REGIONS
	 * is set
	 */
	CLM_REGISTRY_FLAG_CD_16_BIT_REGION_INDEX = 0x00200000,

	/** 16-bit revs are used in cc/rev */
	CLM_REGISTRY_FLAG_CD_16_BIT_REV = 0x00400000,

	/** BLOB may contain extended rates sets */
	CLM_REGISTRY_FLAG_EXT_RATE_SETS = 0x00800000,

	/** BLOB may contain ULB channels */
	CLM_REGISTRY_FLAG_ULB = 0x01000000,

	/** BLOB contains regrev remap table */
	CLM_REGISTRY_FLAG_REGREV_REMAP = 0x02000000,

	/** BLOB contains 40MHz subchannel rules */
	CLM_REGISTRY_FLAG_SUBCHAN_RULES_40 = 0x04000000,

	/** Subchannel rules have power increments, stored with bandwidths */
	CLM_REGISTRY_FLAG_SUBCHAN_RULES_INC = 0x08000000,

	/** Subchannel rules have power increments, stored separately from
	 * bandwidths
	 */
	CLM_REGISTRY_FLAG_SUBCHAN_RULES_INC_SEPARATE = 0x10000000,

	/** Region records have two flag bytes */
	CLM_REGISTRY_FLAG_REGION_FLAG_2 = 0x20000000,

	/** All known registry flags */
	CLM_REGISTRY_FLAG_ALL = CLM_REGISTRY_FLAG_COUNTRY_10_FL
		| CLM_REGISTRY_FLAG_APPS_VERSION
		| CLM_REGISTRY_FLAG_SUB_CHAN_RULES
		| CLM_REGISTRY_FLAG_160MHZ
		| CLM_REGISTRY_FLAG_PER_BW_RS
		| CLM_REGISTRY_FLAG_PER_BAND_RATES
		| CLM_REGISTRY_FLAG_USER_STRING
		| CLM_REGISTRY_FLAG_HIGH_BW_COMBS
		| CLM_REGISTRY_FLAG_NUM_RATES_MASK
		| CLM_REGISTRY_FLAG_CD_REGIONS
		| CLM_REGISTRY_FLAG_CD_LOC_IDX_BYTES_MASK
		| CLM_REGISTRY_FLAG_CD_16_BIT_REGION_INDEX
		| CLM_REGISTRY_FLAG_CD_16_BIT_REV
		| CLM_REGISTRY_FLAG_EXT_RATE_SETS
		| CLM_REGISTRY_FLAG_ULB
		| CLM_REGISTRY_FLAG_REGREV_REMAP
		| CLM_REGISTRY_FLAG_SUBCHAN_RULES_40
		| CLM_REGISTRY_FLAG_SUBCHAN_RULES_INC
		| CLM_REGISTRY_FLAG_SUBCHAN_RULES_INC_SEPARATE
		| CLM_REGISTRY_FLAG_REGION_FLAG_2
};

/** Major version number of CLM data format */
#define CLM_FORMAT_VERSION_MAJOR 18

/* Minor version number of CLM data format */
#define CLM_FORMAT_VERSION_MINOR 0

/** Flags and flag masks used in BLOB's byte fields */
enum clm_data_flags {

	/* BASE LOCALE FLAGS */

	/** General DFS rules */
	CLM_DATA_FLAG_DFS_NONE = 0x00,

	/** EU DFS rules */
	CLM_DATA_FLAG_DFS_EU = 0x01,

	/** US (FCC) DFS rules */
	CLM_DATA_FLAG_DFS_US = 0x02,

	/** TW DFS rules */
	CLM_DATA_FLAG_DFS_TW = 0x03,

	/** Mask of DFS field */
	CLM_DATA_FLAG_DFS_MASK = 0x03,

	/** FiltWAR1 flag from CLM XML */
	CLM_DATA_FLAG_FILTWAR1 = 0x04,


	/* TRANSMISSION POWER RECORD FLAGS */

	/** 20MHz channel width */
	CLM_DATA_FLAG_WIDTH_20 = 0x00,

	/** 40MHz channel width */
	CLM_DATA_FLAG_WIDTH_40 = 0x01,

	/** 80MHz channel width */
	CLM_DATA_FLAG_WIDTH_80 = 0x08,

	/** 160MHz channel width */
	CLM_DATA_FLAG_WIDTH_160 = 0x09,

	/** 80+80MHz channel width */
	CLM_DATA_FLAG_WIDTH_80_80 = 0x48,

	/** 2.5MHz channel width (if WIDTH_EXT flag set) */
	CLM_DATA_FLAG_WIDTH_2_5 = 0x00,

	/** 5MHz channel width (if WIDTH_EXT flag set) */
	CLM_DATA_FLAG_WIDTH_5 = 0x01,

	/** 10MHz channel width (if WIDTH_EXT flag set) */
	CLM_DATA_FLAG_WIDTH_10 = 0x08,

	/** Mask of (noncontiguous!) channel width field */
	CLM_DATA_FLAG_WIDTH_MASK = 0x49,

	/** TX power specified as conducted limit */
	CLM_DATA_FLAG_MEAS_COND = 0x00,

	/** TX power specified as EIRP limit */
	CLM_DATA_FLAG_MEAS_EIRP = 0x02,

	/** Mask of TX power limit type field */
	CLM_DATA_FLAG_MEAS_MASK = 0x02,

	/** No extra (per-antenna) power bytes in record */
	CLM_DATA_FLAG_PER_ANT_0 = 0x00,

	/** 1 extra (per-antenna) power bytes in record */
	CLM_DATA_FLAG_PER_ANT_1 = 0x10,

	/** 2 extra (per-antenna) power bytes in record */
	CLM_DATA_FLAG_PER_ANT_2 = 0x20,

	/** 3 extra (per-antenna) power bytes in record */
	CLM_DATA_FLAG_PER_ANT_3 = 0x30,

	/** Mask for number of extra (per-antenna) bytes */
	CLM_DATA_FLAG_PER_ANT_MASK = 0x30,

	/** Shift for number of extra (per-antenna) bytes */
	CLM_DATA_FLAG_PER_ANT_SHIFT = 4,

	/** Nonlast transmission power record in locale */
	CLM_DATA_FLAG_MORE = 0x04,

	/** Second power record flag byte follows */
	CLM_DATA_FLAG_FLAG2 = 0x80,


	/* SECOND TRANSMISSION POWER RECORD FLAG BYTE */

	/** Bandwidth extension bit (see comments to CLM_DATA_FLAG_WIDTH_) */
	CLM_DATA_FLAG2_WIDTH_EXT = 0x01,

	/** Rate set indices in extended rates' table */
	CLM_DATA_FLAG2_EXT_RATES = 0x02,


	/* REGION FLAGS */

	/** Subchannel rules 3-bit index */
	CLM_DATA_FLAG_REG_SC_RULES_MASK = 0x07,

	/** Width in bits of CLM_DATA_FLAG_REG_SC_RULES_MASK */
	CLM_DATA_FLAG_REG_SC_RULES_MASK_WIDTH = 3,

	/** Subchannel rules 4-bit index */
	CLM_DATA_FLAG_REG_SC_RULES_MASK_4 = 0x47,

	/** Bits to remove from index byte to get linear index */
	CLM_DATA_FLAG_REG_SC_RULES_EXTRA_BITS_4 = 0x38,

	/** Beamforming enabled */
	CLM_DATA_FLAG_REG_TXBF = 0x08,

	/** Region is default for its CC */
	CLM_DATA_FLAG_REG_DEF_FOR_CC = 0x10,

	/** Region is EDCRS-EU compliant */
	CLM_DATA_FLAG_REG_EDCRS_EU = 0x20,

	/** Subchannel rules index high 5 bits in second region flags byte */
	CLM_DATA_FLAG_2_REG_SC_RULES_MASK = 0x1F,


	/* SUBCHANNEL RULES BANDWIDTH FLAGS */

	/** Use 20MHz limits */
	CLM_DATA_FLAG_SC_RULE_BW_20 = 0x01,

	/** Use 40MHz limits */
	CLM_DATA_FLAG_SC_RULE_BW_40 = 0x02,

	/** Use 80MHz limits */
	CLM_DATA_FLAG_SC_RULE_BW_80 = 0x04,

	/** Use 160MHz limits */
	CLM_DATA_FLAG_SC_RULE_BW_160 = 0x08
};

/** Subchannel identifiers == clm_limits_type-1 */
typedef enum clm_data_sub_chan_id {
	CLM_DATA_SUB_CHAN_L,
	CLM_DATA_SUB_CHAN_U,
	CLM_DATA_SUB_CHAN_LL,
	CLM_DATA_SUB_CHAN_LU,
	CLM_DATA_SUB_CHAN_UL,
	CLM_DATA_SUB_CHAN_UU,
	CLM_DATA_SUB_CHAN_LLL,
	CLM_DATA_SUB_CHAN_LLU,
	CLM_DATA_SUB_CHAN_LUL,
	CLM_DATA_SUB_CHAN_LUU,
	CLM_DATA_SUB_CHAN_ULL,
	CLM_DATA_SUB_CHAN_ULU,
	CLM_DATA_SUB_CHAN_UUL,
	CLM_DATA_SUB_CHAN_UUU,
	CLM_DATA_SUB_CHAN_NUM,
	CLM_DATA_SUB_CHAN_MAX_40 = CLM_DATA_SUB_CHAN_U + 1,
	CLM_DATA_SUB_CHAN_MAX_80 = CLM_DATA_SUB_CHAN_UU + 1,
	CLM_DATA_SUB_CHAN_MAX_160 = CLM_DATA_SUB_CHAN_UUU + 1
} clm_data_sub_chan_id_t;

/** 4-bit subchannel rules' indices, as encoded in country flags (deprecated
 * with transition to 8-bit indices)
 */
typedef enum clm_data_scr_indices_4 {
	CLM_SCR_IDX_NONE = 0x00,
	CLM_SCR_IDX_1 = 0x01,
	CLM_SCR_IDX_2 = 0x02,
	CLM_SCR_IDX_3 = 0x03,
	CLM_SCR_IDX_4 = 0x04,
	CLM_SCR_IDX_5 = 0x05,
	CLM_SCR_IDX_6 = 0x06,
	CLM_SCR_IDX_7 = 0x07,
	CLM_SCR_IDX_8 = 0x40,
	CLM_SCR_IDX_9 = 0x41,
	CLM_SCR_IDX_10 = 0x42,
	CLM_SCR_IDX_11 = 0x43,
	CLM_SCR_IDX_12 = 0x44,
	CLM_SCR_IDX_13 = 0x45,
	CLM_SCR_IDX_14 = 0x46,
	CLM_SCR_IDX_15 = 0x47
} clm_data_scr_indices_t;


/*
****************************
* CLM DATA BLOB STRUCTURES *
****************************
*/

/** Descriptor of channel comb
 * Channel comb is a sequence of evenly spaced channel numbers
 */
typedef struct clm_channel_comb {
	/** First channel number */
	unsigned char first_channel;

	/** Last channel number */
	unsigned char last_channel;

	/** Distance between channel numbers in sequence */
	unsigned char stride;
} clm_channel_comb_t;

/** Descriptor of set of channel combs */
typedef struct clm_channel_comb_set {
	/** Number of combs in set */
	int num;

	/** Address of combs' vector */
	const struct clm_channel_comb *set;
} clm_channel_comb_set_t;

/** Channel range descriptor */
typedef struct clm_channel_range {
	/** Number of first channel */
	unsigned char start;

	/** Number of last channel */
	unsigned char end;
} clm_channel_range_t;


/** Subchannel rule with increment. This representation of subchannel rules is
 * deprecated in favor of split bandwidth/increment vectors
 */
typedef struct clm_sub_chan_rule_inc {
	/** Bandwidths (combination of CLM_DATA_FLAG_SC_RULE_BW_ bits) */
	unsigned char bw;

	/** Power increment in qdB units */
	signed char inc;
} clm_sub_chan_rule_inc_t;

/** Subchannel rules descriptor for 40MHz channel range, with power increment
 */
typedef struct clm_sub_chan_channel_rules_40_inc {
	/** Channel range idx */
	unsigned char chan_range;

	/** Subchannel rules */
	clm_sub_chan_rule_inc_t sub_chan_rules[CLM_DATA_SUB_CHAN_MAX_40];
} clm_sub_chan_channel_rules_40_inc_t;

/** Subchannel rules descriptor for 80MHz channel range, with power increment
 */
typedef struct clm_sub_chan_channel_rules_80_inc {
	/** Channel range idx */
	unsigned char chan_range;

	/** Subchannel rules */
	clm_sub_chan_rule_inc_t sub_chan_rules[CLM_DATA_SUB_CHAN_MAX_80];
} clm_sub_chan_channel_rules_80_inc_t;

/** Subchannel rules descriptor for 160MHz channel range, with power increment
 */
typedef struct clm_sub_chan_channel_rules_160_inc {
	/** Channel range idx */
	unsigned char chan_range;

	/* Subchannel rules */
	clm_sub_chan_rule_inc_t sub_chan_rules[CLM_DATA_SUB_CHAN_MAX_160];
} clm_sub_chan_channel_rules_160_inc_t;
/* End of clm_sub_chan_rule_inc_t-based subchannel rule structures */

/** Subchannel 'rules with optional increment' descriptor for region for a
 * particular band/bandwidth. Extends and supersedes
 * clm_sub_chan_region_rules_XX(_t) structures
 */
typedef struct clm_sub_chan_region_rules_inc {
	/** Number of channel-range-level rules */
	int num;

	/** Array of channel-range-level rules */
	const void *channel_rules;

	/** Array of channel-range-level increments, parallel to
	 * 'channel_rules' or NULL
	 */
	const unsigned char *increments;
} clm_sub_chan_region_rules_inc_t;

/** Subchannel rules descriptor for region for certain bandwidth. Binary
 * identical to all clm_sub_chan_region_rules_XX(_t) structures
 */
typedef struct clm_sub_chan_region_rules {
	/** Number of channel-range-level rules */
	int num;

	/** Array of channel-range-level rules */
	const void *channel_rules;
} clm_sub_chan_region_rules_t;

/** Set of region-level subchannel rules for certain bandwidth. Binary
 * identical to all clm_sub_chan_rules_set_XX(_t) structures
 */
typedef struct clm_sub_chan_rules_set {
	/** Number of region-level subchannel rules */
	int num;

	/** Array of region-level subchannel rules
	 * (clm_sub_chan_region_rules_40_t or clm_sub_chan_region_rules_inc_t
	 * structures)
	 */
	const void *region_rules;
} clm_sub_chan_rules_set_t;

/** Subchannel rules descriptor for 40MHz channel range */
typedef struct clm_sub_chan_channel_rules_40 {
	/** Channel range idx */
	unsigned char chan_range;

	/** Subchannel rules (combinations of CLM_DATA_FLAG_SC_RULE_BW_ bits)
	 */
	unsigned char sub_chan_rules[CLM_DATA_SUB_CHAN_MAX_40];
} clm_sub_chan_channel_rules_40_t;

/** Subchannel rules descriptor for region for 40MHz channels */
typedef struct clm_sub_chan_region_rules_40 {
	/** Number of channel-range-level rules */
	int num;

	/** Array of channel-range-level rules */
	const void *channel_rules;
} clm_sub_chan_region_rules_40_t;

/** Set of region-level 40MHz subchannel rules */
typedef struct clm_sub_chan_rules_set_40 {
	/** Number of region-level subchannel rules */
	int num;

	/** Array of region-level subchannel rules
	 * (clm_sub_chan_region_rules_40_t or clm_sub_chan_region_rules_inc_t
	 * structures)
	 */
	const void *region_rules;
} clm_sub_chan_rules_set_40_t;

/** Subchannel rules descriptor for 80MHz channel range */
typedef struct clm_sub_chan_channel_rules_80 {
	/** Channel range idx */
	unsigned char chan_range;

	/** Subchannel rules (combinations of CLM_DATA_FLAG_SC_RULE_BW_ bits)
	 */
	unsigned char sub_chan_rules[CLM_DATA_SUB_CHAN_MAX_80];
} clm_sub_chan_channel_rules_80_t;

/** Subchannel rules descriptor for region for 80MHz channels */
typedef struct clm_sub_chan_region_rules_80 {
	/** Number of channel-range-level rules */
	int num;

	/** Array of channel-range-level rules */
	const void *channel_rules;
} clm_sub_chan_region_rules_80_t;

/** Set of region-level 80MHz subchannel rules */
typedef struct clm_sub_chan_rules_set_80 {
	/** Number of region-level subchannel rules */
	int num;

	/** Array of region-level subchannel rules
	 * (clm_sub_chan_region_rules_80_t or clm_sub_chan_region_rules_inc_t
	 * structures)
	 */
	const void *region_rules;
} clm_sub_chan_rules_set_80_t;

/** Subchannel rules descriptor for 160MHz channel range */
typedef struct clm_sub_chan_channel_rules_160 {
	/** Channel range idx */
	unsigned char chan_range;

	/** Subchannel rules (combinations of CLM_DATA_FLAG_SC_RULE_BW_ bits)
	 */
	unsigned char sub_chan_rules[CLM_DATA_SUB_CHAN_MAX_160];
} clm_sub_chan_channel_rules_160_t;

/** Subchannel rules descriptor for region for 160MHz channels */
typedef struct clm_sub_chan_region_rules_160 {
	/** Number of channel-range-level rules */
	int num;

	/** Array of channel-range-level rules */
	const void *channel_rules;
} clm_sub_chan_region_rules_160_t;

/** Set of region-level 160MHz subchannel rules */
typedef struct clm_sub_chan_rules_set_160 {
	/** Number of region-level subchannel rules */
	int num;

	/** Array of region-level subchannel rules
	 * (clm_sub_chan_region_rules_160_t or clm_sub_chan_region_rules_inc_t
	 * structures)
	 */
	const void *region_rules;
} clm_sub_chan_rules_set_160_t;

/** Region identifier, 1-byte rev */
typedef struct clm_cc_rev {
	/** Region country code */
	char cc[2];

	/** Region revision */
	unsigned char rev;
} clm_cc_rev_t;

/** Region identifier, 2-byte rev (4 bytes overall) */
typedef struct clm_cc_rev4 {
	/** Region country code */
	char cc[2];

	/** Region revision */
	unsigned short rev;
} clm_cc_rev4_t;

/** First-generation region descriptor: 8-bit locale indices, no flags */
typedef struct clm_country_rev_definition {
	/** Region identifier */
	struct clm_cc_rev cc_rev;

	/** Indices of region locales' descriptors */
	unsigned char locales[CLM_LOC_IDX_NUM];
} clm_country_rev_definition_t;

/** Second-generation region descriptor, uses 10-bit locale indices, has
 * flags
 */
typedef struct clm_country_rev_definition10_fl {
	/** Region identifier */
	struct clm_cc_rev cc_rev;

	/** Indices of region locales' descriptors */
	unsigned char locales[CLM_LOC_IDX_NUM];

	/** Higher bits of locale indices */
	unsigned char hi_bits;

	/** Region flags */
	unsigned char flags;
} clm_country_rev_definition10_fl_t;

/** Third-generation region descriptor: content-dependent layout, 1 extra byte
 * (8 bytes overall)
 */
typedef struct clm_country_rev_definition_cd8 {
	/** Region identifier */
	struct clm_cc_rev cc_rev;

	/** Indices of region locales' descriptors */
	unsigned char locales[CLM_LOC_IDX_NUM];

	/** Extra byte */
	unsigned char extra[1];
} clm_country_rev_definition_cd8_t;

/** Third-generation region descriptor: content-dependent layout, 2 extra bytes
 * (9 bytes overall)
 */
typedef struct clm_country_rev_definition_cd9 {
	/** Region identifier */
	struct clm_cc_rev cc_rev;

	/** Indices of region locales' descriptors */
	unsigned char locales[CLM_LOC_IDX_NUM];

	/** Extra bytes */
	unsigned char extra[2];
} clm_country_rev_definition_cd9_t;

/** Third-generation region descriptor: content-dependent layout, 3 extra bytes
 * (10 bytes overall)
 */
typedef struct clm_country_rev_definition_cd10 {
	/** Region identifier */
	struct clm_cc_rev cc_rev;

	/** Indices of region locales' descriptors */
	unsigned char locales[CLM_LOC_IDX_NUM];

	/** Extra bytes */
	unsigned char extra[3];
} clm_country_rev_definition_cd10_t;

/** Third-generation region descriptor: content-dependent layout, 4 extra bytes
 * (11 bytes overall)
 */
typedef struct clm_country_rev_definition_cd11 {
	/** Region identifier */
	struct clm_cc_rev cc_rev;

	/** Indices of region locales' descriptors */
	unsigned char locales[CLM_LOC_IDX_NUM];

	/** Extra bytes */
	unsigned char extra[4];
} clm_country_rev_definition_cd11_t;

/** Third-generation region descriptor: content-dependent layout, 5 extra bytes
 * (12 bytes overall)
 */
typedef struct clm_country_rev_definition_cd12 {
	/** Region identifier */
	struct clm_cc_rev cc_rev;

	/** Indices of region locales' descriptors */
	unsigned char locales[CLM_LOC_IDX_NUM];

	/** Extra bytes */
	unsigned char extra[5];
} clm_country_rev_definition_cd12_t;

/** Set of region descriptors */
typedef struct clm_country_rev_definition_set {
	/** Number of region descriptors in set */
	int num;

	/** Vector of region descriptors */
	const void *set;
} clm_country_rev_definition_set_t;

/** Region alias descriptor */
typedef struct clm_advertised_cc {
	/** Aliased (effective) country codes */
	char cc[2];

	/** Number of region identifiers */
	int num_aliases;

	/** Vector of region identifiers (clm_cc_rev structures for
	 * content-independent BLOB, cc/rev indices in content-dependent BLOB
	 * clm_cc_rev4 structures)
	 */
	const void *aliases;
} clm_advertised_cc_t;

/** Set of alias descriptors */
typedef struct clm_advertised_cc_set {
	/** Number of descriptors in set */
	int num;

	/** Vector of alias descriptors */
	const struct clm_advertised_cc *set;
} clm_advertised_cc_set_t;

/** Aggregation descriptor */
typedef struct clm_aggregate_cc {
	/** Default region identifier */
	struct clm_cc_rev def_reg;

	/** Number of region mappings in aggregation */
	int num_regions;

	/** Vector of aggregation's region mappings */
	const struct clm_cc_rev *regions;
} clm_aggregate_cc_t;

/** Aggregation descriptor with 16-bit rev in def_reg */
typedef struct clm_aggregate_cc_cd {
	/** Default region identifier */
	struct clm_cc_rev4 def_reg;

	/** Number of region mappings in aggregation */
	int num_regions;

	/** Vector of indices of regions' cc/revs */
	const void *regions;
} clm_aggregate_cc16_t;

/** Set of aggregation descriptors */
typedef struct clm_aggregate_cc_set {
	/** Number of aggregation descriptors */
	int num;

	/** Vector of aggregation descriptors */
	const void *set;
} clm_aggregate_cc_set_t;

/** Set of cc/revs */
typedef struct clm_cc_rev_set {
	/** Number of cc/revs */
	int num;

	/** Vector of clm_cc_rev_t or clm_cc_rev4_t structures */
	const void *set;
} clm_cc_rev_set_t;

/** Regrev remap descriptor for a single CC */
typedef struct clm_regrev_cc_remap {
	/** CC whose regrevs are being remapped */
	char cc[2];

	/** Index of first element in regrev remap table */
	unsigned short index;
} clm_regrev_cc_remap_t;

/** Describes remap of one regrev */
typedef struct clm_regrev_regrev_remap {
	/** High byte of 16-bit regrev */
	unsigned char r16h;

	/** Low byte of 16-bit regrev */
	unsigned char r16l;

	/** Correspondent 8-bit regrev */
	unsigned char r8;
} clm_regrev_regrev_remap_t;

/** Set of regrev remap descriptors */
typedef struct clm_regrev_cc_remap_set {
	/** Number of elements */
	int num;

	/** Pointer to table of per-CC regrev remap descriptors. If this table
	 * is nonempty it contains one after-last element that denotes the end
	 * of last remap descriptor's portion of remap table
	 */
	const struct clm_regrev_cc_remap *cc_remaps;

	/** Remap table. For each CC it has contiguous span of elements (first
	 * element identified by 'index' field of per-CC remap descriptor, last
	 * element precedes first element of next per-CC remap descriptor (i.e.
	 * determined by 'index' field of next per-CC remap descriptor). Each
	 * span is a sequence of clm_regrev_regrev_remap structures that
	 * describe remap for individual CC/rev
	 */
	const struct clm_regrev_regrev_remap *regrev_remaps;
} clm_regrev_cc_remap_set_t;


#ifdef CLM_NO_PER_BAND_RATESETS_IN_ROM
#define locale_rate_sets_5g_20m		locale_rate_sets_20m
#define locale_rate_sets_5g_40m		locale_rate_sets_40m
#define locale_rate_sets_5g_80m		locale_rate_sets_80m
#define locale_rate_sets_5g_160m	locale_rate_sets_160m
#endif


/** Registry (TOC) of CLM data structures, obtained in BLOB */
typedef struct clm_data_registry {
	/** Valid 20MHz channels of 2.4GHz band */
	const struct clm_channel_comb_set *valid_channels_2g_20m;

	/** Valid 20MHz channels of 5GHz band  */
	const struct clm_channel_comb_set *valid_channels_5g_20m;

	/** Vector of channel range descriptors for 20MHz channel ranges (all
	 * channel ranges if CLM_REGISTRY_FLAG_PER_BW_RS registry flag not set)
	 */
	const struct clm_channel_range *channel_ranges_20m;

	/** Sequence of byte strings that encode restricted sets */
	const unsigned char *restricted_channels;

	/** Sequence of byte strings that encode locales' valid channels
	 * sets
	 */
	const unsigned char *locale_valid_channels;

	/** Sequence of byte strings that encode rate sets for 5GHz 20MHz and
	 * ULB channels (all 20MHz rate sets if
	 * CLM_REGISTRY_FLAG_PER_BAND_RATES registry flag not set, all rate
	 * sets if CLM_REGISTRY_FLAG_PER_BW_RS registry flag not set)
	 */
	const unsigned char *locale_rate_sets_5g_20m;

	/** Byte string sequences that encode locale definitions */
	const unsigned char *locales[CLM_LOC_IDX_NUM];

	/** Address of region definitions set descriptor */
	const struct clm_country_rev_definition_set	 *countries;

	/** Address of alias definitions set descriptor */
	const struct clm_advertised_cc_set *advertised_ccs;

	/** Address of aggregation definitions set descriptor */
	const struct clm_aggregate_cc_set *aggregates;

	/** Flags */
	int flags;

	/** Address of subchannel rules set descriptor for 80MHz channels or
	 * NULL
	 */
	const clm_sub_chan_rules_set_80_t *sub_chan_rules_80;

	/** Address of subchannel rules set descriptor for 160MHz channels or
	 * NULL
	 */
	const clm_sub_chan_rules_set_160_t *sub_chan_rules_160;

	/** Vector of channel range descriptors for 40MHz channel ranges */
	const struct clm_channel_range *channel_ranges_40m;

	/** Vector of channel range descriptors for 80MHz channel ranges */
	const struct clm_channel_range *channel_ranges_80m;

	/** Vector of channel range descriptors for 160MHz channel ranges */
	const struct clm_channel_range *channel_ranges_160m;

	/** Sequence of byte strings that encode rate sets for 5GHz 40MHz
	 * channels (all 40MHz rate sets if CLM_REGISTRY_FLAG_PER_BAND_RATES
	 * registry flag not set)
	 */
	const unsigned char *locale_rate_sets_5g_40m;

	/** Sequence of byte strings that encode rate sets for 5GHz 80MHz
	 * channels (all 80MHz rate sets if CLM_REGISTRY_FLAG_PER_BAND_RATES
	 * registry flag not set)
	 */
	const unsigned char *locale_rate_sets_5g_80m;

	/** Sequence of byte strings that encode rate sets for 5GHz 160MHz
	 * channels (all 160MHz rate sets if CLM_REGISTRY_FLAG_PER_BAND_RATES
	 * registry flag not set)
	 */
	const unsigned char *locale_rate_sets_5g_160m;

	/** Sequence of byte strings that encode rate sets for 2.4GHz 20MHz and
	 * ULB channels
	 */
	const unsigned char *locale_rate_sets_2g_20m;

	/** Sequence of byte strings that encode rate sets for 2.4GHz 40MHz
	 * channels
	 */
	const unsigned char *locale_rate_sets_2g_40m;

	/** User string or NULL */
	const char *user_string;

	/** Valid 40MHz channels of 2.4GHz band */
	const struct clm_channel_comb_set *valid_channels_2g_40m;

	/** Valid 40MHz channels of 5GHz band  */
	const struct clm_channel_comb_set *valid_channels_5g_40m;

	/** Valid 80MHz channels of 5GHz band  */
	const struct clm_channel_comb_set *valid_channels_5g_80m;

	/** Valid 160MHz channels of 5GHz band  */
	const struct clm_channel_comb_set *valid_channels_5g_160m;

	/** Extra cc/revs (cc/revs used in BLOB but not present in region
	 * table)
	 */
	const struct clm_cc_rev_set *extra_ccrevs;

	/** Sequence of byte strings that encode rate 3+TX sets for 2.4GHz
	 * 20MHz and ULB channels. Used in case of main rate set overflow or if
	 * 4TX rates are present
	 */
	const unsigned char *locale_ext_rate_sets_2g_20m;

	/** Sequence of byte strings that encode rate 3+TX sets for 2.4GHz
	 * 40MHz channels. Used in case of main rate set overflow or if 4TX
	 * rates are present
	 */
	const unsigned char *locale_ext_rate_sets_2g_40m;

	/** Sequence of byte strings that encode rate 3+TX sets for 5GHz 20MHz
	 * and ULB channels. Used in case of main rate set overflow or if 4TX
	 * rates are present
	 */
	const unsigned char *locale_ext_rate_sets_5g_20m;

	/** Sequence of byte strings that encode rate 3+TX sets for 5GHz 40MHz
	 * channels. Used in case of main rate set overflow or if 4TX rates are
	 * present
	 */
	const unsigned char *locale_ext_rate_sets_5g_40m;

	/** Sequence of byte strings that encode rate 3+TX sets for 5GHz 80MHz
	 * channels. Used in case of main rate set overflow or if 4TX rates are
	 * present
	 */
	const unsigned char *locale_ext_rate_sets_5g_80m;

	/** Sequence of byte strings that encode rate 3+TX sets for 5GHz 160MHz
	 * channels. Used in case of main rate sets overflow or if 4TX rates
	 * are present
	 */
	const unsigned char *locale_ext_rate_sets_5g_160m;

	/** Vector of channel range descriptors for 2.5MHz channel ranges */
	const struct clm_channel_range *channel_ranges_2_5m;

	/** Vector of channel range descriptors for 5MHz channel ranges */
	const struct clm_channel_range *channel_ranges_5m;

	/** Vector of channel range descriptors for 10MHz channel ranges */
	const struct clm_channel_range *channel_ranges_10m;

	/** Valid 2.5MHz channels of 2.4GHz band */
	const struct clm_channel_comb_set *valid_channels_2g_2_5m;

	/** Valid 5MHz channels of 2.4GHz band */
	const struct clm_channel_comb_set *valid_channels_2g_5m;

	/** Valid 10MHz channels of 2.4GHz band */
	const struct clm_channel_comb_set *valid_channels_2g_10m;

	/** Valid 2.5MHz channels of 5GHz band */
	const struct clm_channel_comb_set *valid_channels_5g_2_5m;

	/** Valid 5MHz channels of 5GHz band */
	const struct clm_channel_comb_set *valid_channels_5g_5m;

	/** Valid 10MHz channels of 5GHz band */
	const struct clm_channel_comb_set *valid_channels_5g_10m;

	/** Regrev remap table or NULL */
	const clm_regrev_cc_remap_set_t *regrev_remap;

	/** Address of subchannel rules set descriptor for 2.4GHz 40MHz
	 * channels or NULL
	 */
	const clm_sub_chan_rules_set_40_t *sub_chan_rules_2g_40m;

	/** Address of subchannel rules set descriptor for 5GHz 40MHz channels
	 * or NULL
	 */
	const clm_sub_chan_rules_set_40_t *sub_chan_rules_5g_40m;
} clm_registry_t;

/** CLM data BLOB header */
typedef struct clm_data_header {

	/** CLM data header tag */
	char header_tag[10];

	/** CLM BLOB format version major number */
	short format_major;

	/** CLM BLOB format version minor number */
	short format_minor;

	/** CLM data set version string */
	char clm_version[20];

	/** CLM compiler version string */
	char compiler_version[10];

	/** Pointer to self (for relocation compensation) */
	const struct clm_data_header *self_pointer;

	/** CLM BLOB data registry */
	const struct clm_data_registry *data;

	/** CLM compiler version string */
	char generator_version[30];

	/** SW apps version string */
	char apps_version[20];
} clm_data_header_t;

typedef struct clm_data_version {
        short major_version;
        short minor_version;
        char productid[20];
} clm_version_t;

#endif /* _WLC_CLM_DATA_H_ */
