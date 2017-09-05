/*
 * CLM API functions.
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
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
 * $Id: wlc_clm.c 540418 2015-03-12 01:36:03Z $
 */

#include <wlc_cfg.h>
#include "wlc_clm.h"
#include "wlc_clm_data.h"
#include <bcmwifi_rates.h>
#include <typedefs.h>


#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>

/******************************
* MODULE MACROS AND CONSTANTS *
*******************************
*/

/* BLOB format version major number */
#define FORMAT_VERSION_MAJOR 18

/* BLOB format version minor number */
#define FORMAT_VERSION_MINOR 0

/* Minimum supported binary BLOB format's major version */
#define FORMAT_MIN_COMPAT_MAJOR 7

#if (FORMAT_VERSION_MAJOR != CLM_FORMAT_VERSION_MAJOR) || (FORMAT_VERSION_MINOR != \
	CLM_FORMAT_VERSION_MINOR)
#error CLM data format version mismatch between wlc_clm.c and wlc_clm_data.h
#endif

#ifndef NULL
	/** Null pointer */
	#define NULL 0
#endif

/** Boolean type definition for use inside this module */
typedef int MY_BOOL;

#ifndef TRUE
	/** TRUE for MY_BOOL */
	#define TRUE 1
#endif

#ifndef FALSE
	/** FALSE for MY_BOOL */
	#define FALSE 0
#endif

#ifndef OFFSETOF
	/** Offset of given field in given structure */
#ifdef _WIN64
	#define OFFSETOF(s, m) (unsigned long long)&(((s *)0)->m)
#else
	#define OFFSETOF(s, m) (unsigned long)&(((s *)0)->m)
#endif /* _WIN64 */
#endif /* OFFSETOF */

#ifndef ARRAYSIZE
	/** Number of elements in given array */
	#define ARRAYSIZE(x) (unsigned)(sizeof(x)/sizeof(x[0]))
#endif

#if WL_NUMRATES >= 178
	/** Defined if bcmwifi_rates.h contains TXBF rates */
	#define CLM_TXBF_RATES_SUPPORTED
#endif

/** CLM data source IDs */
typedef enum data_source_id {
	/** Incremental CLM data. Placed first so we look there before base
	 * data
	 */
	DS_INC,

	/** Main CLM data */
	DS_MAIN,

	/** Number of CLM data source IDs */
	DS_NUM
} data_source_id_t;

/** Indices in base_ht_loc_data[] vector used by some function and containing
 * data pertinent to base and HT locales
 */
typedef enum base_ht_id {
	/** Index for base locale */
	BH_BASE,

	/** Index for HT locale */
	BH_HT,

	/** Number of indices (length of base_ht_loc_data vector) */
	BH_NUM
} base_ht_id_t;

/** Module constants */
enum clm_internal_const {
	/** MASKS THAT DEFINE ITERATOR CONTENTS */

	/** Pointed data is in main CLM data source */
	ITER_DS_MAIN = 0x40000000,

	/** Pointed data is in incremental CLM data source */
	ITER_DS_INC = 0x00000000,

	/** Mask of data source field of iterator */
	ITER_DS_MASK = 0x40000000,

	/** Mask of index field of iterator */
	ITER_IDX_MASK = 0x3FFFFFFF,

	/** Number of MCS/OFDM rates, differing only by modulation */
	NUM_MCS_MODS = 8,

	/** Number of DSSS rates, differing only by modulation */
	NUM_DSSS_MODS = 4,

	/** Channel number stride at 20MHz */
	CHAN_STRIDE = 4,

	/** Mask of count field in subchannel path descriptor */
	SUB_CHAN_PATH_COUNT_MASK = 0xF0,

	/** Offset of count field in subchannel path descriptor */
	SUB_CHAN_PATH_COUNT_OFFSET = 4,

	/** Prefill constant for power limits used in clm_limits() */
	UNSPECIFIED_POWER = CLM_DISABLED_POWER + 1,

	/** clm_country_locales::computed_flags: country flags taken from main
	 * data
	 */
	COUNTRY_FLAGS_DS_MAIN = (unsigned char)DS_MAIN,

	/** clm_country_locales::computed_flags: country flags taken from
	 * incremental data
	 */
	COUNTRY_FLAGS_DS_INC = (unsigned char)DS_INC,

	/** clm_country_locales::computed_flags: mask for country flags source
	 * field
	 */
	COUNTRY_FLAGS_DS_MASK = (unsigned char)(DS_NUM - 1),

	/** Base value for rates in extended rate set */
	BASE_EXT_RATE = WL_RATE_1X3_DSSS_1
};

/** Rate types */
enum clm_rate_type {
	/** DSSS (802.11b) rate */
	RT_DSSS,

	/** OFDM (802.11a/g) rate */
	RT_OFDM,

	/** MCS (802.11n) rate */
	RT_MCS
};

/** Format of CC/rev representation in aggregations and advertisings */
typedef enum clm_ccrev_format {
	/** As clm_cc_rev_t */
	CCREV_FORMAT_CC_REV,

	/** As 8-bit index to region table */
	CCREV_FORMAT_CC_IDX8,

	/** As 16-bit index to region table */
	CCREV_FORMAT_CC_IDX16
} clm_ccrev_format_t;

/** Internal type for regrevs */
typedef unsigned short regrev_t;

/** CLM data set descriptor */
typedef struct data_dsc {
	/** Pointer to registry (TOC) structure of CLM data */
	const clm_registry_t *data;

	/** Relocation factor of CLM DATA set: value that shall be added to
	 * pointer contained in CLM data set to get a true pointer (e.g. 0 if
	 * data is not relocated)
	 */
	unsigned relocation;

	/** Valid channel comb sets (per band, per bandwidth). Empty for
	 * 80+80
	 */
	clm_channel_comb_set_t valid_channel_combs[CLM_BAND_NUM][CLM_BW_NUM];

	/** 40+MHz combs stored in BLOB - deprecated */
	MY_BOOL has_high_bw_combs;

	/** Index within clm_country_rev_definition_cd10_t::extra of byte with
	 * bits 9-16 of rev. -1 for 8-bit revs
	 */
	int reg_rev16_idx;

	/** Length of region record in bytes */
	int country_rev_rec_len;

	/** Address of header for version strings */
	const clm_data_header_t *header;

	/** True if BLOB capable of containing 160MHz data - deprecated */
	MY_BOOL has_160mhz;

	/** Per-bandwidth base addresses of channel range descriptors */
	const clm_channel_range_t *chan_ranges_bw[CLM_BW_NUM];

	/** Per-bandwidth base addresses of rate set definitions */
	const unsigned char *rate_sets_bw[CLM_BAND_NUM][CLM_BW_NUM];

	/** Index within clm_country_rev_definition_cdXX_t::extra of byte with
	 * bits 9-10 of locale index. -1 for 8-bit locale indices
	 */
	int reg_loc10_idx;

	/** Index within clm_country_rev_definition_cdXX_t::extra of byte with
	 * bits 11-12 of locale index. -1 for 8 and 10 bit locale indices
	 */
	int reg_loc12_idx;

	/** Index within clm_country_rev_definition_cdXX_t::extra of byte with
	 * region flags. -1 if region has no flags
	 */
	int reg_flags_idx;

	/** CC/revs representation in aggregations */
	clm_ccrev_format_t ccrev_format;

	/** Per-bandwidth base addresses of extended rate set definitions */
	const unsigned char *ext_rate_sets_bw[CLM_BAND_NUM][CLM_BW_NUM];

	/** True if BLOB contains ULB channels - deprecated */
	MY_BOOL has_ulb;

	/** If BLOB has regrev remaps - pointer to remap set structure */
	const clm_regrev_cc_remap_set_t *regrev_remap;

	/** 'flags' from clm_data_registry or 0 if there is no flags there */
	int registry_flags;

	/** Index within clm_country_rev_definition_cdXX_t::extra second of
	 * byte with region flags. -1 if region has no second byte of flags
	 */
	int reg_flags_2_idx;

	/** 4-bit subchannel index (for backward compatibility) */
	MY_BOOL scr_idx_4;
} data_dsc_t;

/** Addresses of locale-related data */
typedef struct locale_data {
	/** Locale definition */
	const unsigned char *def_ptr;

	/** Per-bandwidth base addresses of channel range descriptors */
	const clm_channel_range_t * const *chan_ranges_bw;

	/** Per-bandwidth base addresses of rate set definitions */
	const unsigned char * const *rate_sets_bw;

	/** Base address of valid channel sets definitions */
	const unsigned char *valid_channels;

	/** Base address of restricted sets definitions */
	const unsigned char *restricted_channels;

	/** Per-bandwidth channel combs */
	const clm_channel_comb_set_t *combs[CLM_BW_NUM];

	/** 80MHz subchannel rules - deprecated! */
	clm_sub_chan_region_rules_80_t sub_chan_channel_rules_80;

	/** 160MHz subchannel rules - deprecated! */
	clm_sub_chan_region_rules_160_t sub_chan_channel_rules_160;

	/** Per-bandwidth base addresses of extended rate set definitions */
	const unsigned char * const *ext_rate_sets_bw;
} locale_data_t;

/** Addresses of region (country)-related data */
typedef struct country_data {
	/** Per-bandwidth base addresses of channel range descriptors in data
	 * set this region defined in
	 */
	const clm_channel_range_t * const *chan_ranges_bw;

	/** 80MHz subchannel rules */
	clm_sub_chan_region_rules_t sub_chan_channel_rules_80;

	/** 160MHz subchannel rules */
	clm_sub_chan_region_rules_t sub_chan_channel_rules_160;

	/** 40MHz subchannel rules */
	clm_sub_chan_region_rules_t sub_chan_channel_rules_40[CLM_BAND_NUM];

	/** Power increments (offsets) for 80MHz subchannel rules (or NULL) */
	const unsigned char *sub_chan_increments_80;

	/** Power increments (offsets) for 80MHz subchannel rules (or NULL) */
	const unsigned char *sub_chan_increments_160;

	/** Power increments (offsets) for 80MHz subchannel rules (or NULL) */
	const unsigned char *sub_chan_increments_40[CLM_BAND_NUM];
} country_data_t;

/** Information about aggregation */
typedef struct aggregate_data {
	/** Default region */
	clm_cc_rev4_t def_reg;

	/** Number of regions */
	int num_regions;

	/** Pointer to vector of regions in BLOB-specific format */
	const void *regions;
} aggregate_data_t;

/** Descriptors of main and incremental data sources */
static data_dsc_t data_sources[] = {
	{ NULL, 0, {{{0, 0}}}, 0, 0, 0, NULL, 0, {NULL}, {{NULL}}, 0, 0, 0,
	(clm_ccrev_format_t)0, {{NULL}}, 0, NULL, 0, 0, 0 },
	{ NULL, 0, {{{0, 0}}}, 0, 0, 0, NULL, 0, {NULL}, {{NULL}}, 0, 0, 0,
	(clm_ccrev_format_t)0, {{NULL}}, 0, NULL, 0, 0, 0 }
};


/* Rate type table and related get/set macros */

#ifdef WL_CLM_RATE_TYPE_SPARSE
/* Legacy version */
static char rate_type[WL_NUMRATES];
#else
/** Rate type by rate index. Values are from enum clm_rate_type, compressed to
 * 2-bits
 */
static char rate_type[(WL_NUMRATES + 3)/4];
#endif

/* Gives rate type for given rate index. Use of named constants make this
 * expression too ugly
 */
#define RATE_TYPE(rate_idx)	\
	((get_rate_type()[(unsigned)(rate_idx) >> 2] >> (((rate_idx) & 3) << 1)) & 3)

/* Sets rate type for given rate index */
#define SET_RATE_TYPE(rate_idx, rt) \
	get_rate_type()[(unsigned)(rate_idx) >> 2] &= ~(3 << (((rate_idx) & 3) << 1)); \
	get_rate_type()[(unsigned)(rate_idx) >> 2] |= rt << (((rate_idx) & 3) << 1)


/** Valid 40M channels of 2.4G band */
static const struct clm_channel_comb valid_channel_combs_2g_40m[] = {
	{  3,  11, 1}, /* 3 - 11 with step of 1 */
};

/** Set of 40M 2.4G valid channel combs */
static const struct clm_channel_comb_set valid_channel_2g_40m_set = {
	1, valid_channel_combs_2g_40m
};

/** Valid 40M channels of 5G band */
static const struct clm_channel_comb valid_channel_combs_5g_40m[] = {
	{ 38,  62, 8}, /* 38 - 62 with step of 8 */
	{102, 142, 8}, /* 102 - 142 with step of 8 */
	{151, 159, 8}, /* 151 - 159 with step of 8 */
};

/** Set of 40M 5G valid channel combs */
static const struct clm_channel_comb_set valid_channel_5g_40m_set = {
	3, valid_channel_combs_5g_40m
};

/** Valid 80M channels of 5G band */
static const struct clm_channel_comb valid_channel_combs_5g_80m[] = {
	{ 42,  58, 16}, /* 42 - 58 with step of 16 */
	{106, 138, 16}, /* 106 - 138 with step of 16 */
	{155, 155, 16}, /* 155 - 155 with step of 16 */
};

/** Set of 80M 5G valid channel combs */
static const struct clm_channel_comb_set valid_channel_5g_80m_set = {
	3, valid_channel_combs_5g_80m
};

/** Valid 160M channels of 5G band */
static const struct clm_channel_comb valid_channel_combs_5g_160m[] = {
	{ 50,  50, 32}, /* 50 - 50 with step of 32 */
	{114, 114, 32}, /* 114 - 114 with step of 32 */
};

/** Set of 160M 5G valid channel combs */
static const struct clm_channel_comb_set valid_channel_5g_160m_set = {
	2, valid_channel_combs_5g_160m
};


/* This is a hack to accommodate PHOENIX2 builds which don't know about the VHT
 * rates
 */
#ifndef WL_RATESET_SZ_VHT_MCS
#define CLM_NO_VHT_RATES
#endif


/** Maps CLM_DATA_FLAG_WIDTH_...  to clm_bandwidth_t */
#ifdef WL11AC
static const unsigned char bw_width_to_idx_ac[CLM_DATA_FLAG_WIDTH_MASK + 1] = {
	CLM_BW_20, CLM_BW_40, 0, 0, 0, 0, 0, 0, CLM_BW_80, CLM_BW_160,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, CLM_BW_80_80, 0};
#else /* WL11AC */
static const unsigned char bw_width_to_idx_non_ac[CLM_DATA_FLAG_WIDTH_MASK + 1] = {
	CLM_BW_20, CLM_BW_40, 0, 0, 0, 0, 0, 0, CLM_BW_40 + 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0};
#endif /* WL11AC */

static const unsigned char* bw_width_to_idx;

/** Maps limit types to descriptors of paths from main channel to correspondent
 * subchannel. Each descriptor is a byte. High nibble contains number of steps
 * in path, bits in low nibble describe steps. 0 - select lower subchannel,
 * 1 - select upper subchannel. Least significant bit corresponds to last step
 */
static const unsigned char subchan_paths[] = {
	0x00, /* CHANNEL */
	0x10, /* SUBCHAN_L */
	0x11, /* SUBCHAN_U */
#ifdef WL11AC
	0x20, /* SUBCHAN_LL */
	0x21, /* SUBCHAN_LU */
	0x22, /* SUBCHAN_UL */
	0x23, /* SUBCHAN_UU */
	0x30, /* SUBCHAN_LLL */
	0x31, /* SUBCHAN_LLU */
	0x32, /* SUBCHAN_LUL */
	0x33, /* SUBCHAN_LUU */
	0x34, /* SUBCHAN_ULL */
	0x35, /* SUBCHAN_ULU */
	0x36, /* SUBCHAN_UUL */
	0x37, /* SUBCHAN_UUU */
#endif /* WL11AC */
};

/** Offsets of per-antenna power limits inside TX power record */
static const int antenna_power_offsets[] = {
	CLM_LOC_DSC_POWER_IDX, CLM_LOC_DSC_POWER1_IDX, CLM_LOC_DSC_POWER2_IDX,
	CLM_LOC_DSC_POWER3_IDX
};

#ifdef WL11ULB
/** Translates bandwidth to ULB bandwidth (both from clm_bandwidth_t) */
static unsigned char bw_to_ulb[CLM_BW_NUM];

static void
BCMRAMFN(set_bw_to_ulb)(unsigned char bw_idx, unsigned char bw_val)
{
	bw_to_ulb[bw_idx] = bw_val;
}
static unsigned char
BCMRAMFN(get_bw_to_ulb)(unsigned char bw_idx)
{
	return (unsigned char)bw_to_ulb[bw_idx];
}
#endif /* WL11ULB */

/****************************
* MODULE INTERNAL FUNCTIONS *
*****************************
*/

/** Local implementation of strcmp()
 * Implemented here because standard library might be unavailable
 * \param[in] str1 First zero-terminated string. Must be nonzero
 * \param[in] str2 Second zero-terminated string. Must be nonzero
 * \return <0 if first string is less than second, >0 if second string more
 * than first, ==0 if strings are equal
 */
static int
my_strcmp(const char *str1, const char *str2)
{
	while (*str1 && *str2 && (*str1 == *str2)) {
		++str1;
		++str2;
	}
	return *str1 - *str2;
}

/** Local implementation of memset()
 * Implemented here because standard library might be unavailable
 * \param[in] to Buffer to fill
 * \param[in] c Character to fill with
 * \param[in] len Length of buffer to fill
 */
static void
my_memset(void *to, char c, int len)
{
	char *t;
	for (t = (char *)to; len--; *t++ = c);
}


/** Removes bits set in mask from value, shifting right
 * \param[in] value Value to remove bits from
 * \param[in] mask Mask with bits to remove
 * Returns value with bits in mask removed
 */
static unsigned int
remove_extra_bits(unsigned int value, unsigned int mask)
{
	while (mask) {
		/* m has mask's lowest set bit and all lower bits set */
		unsigned int m = mask ^ (mask - 1);
		/* Clearing mask's lowest 1 bit */
		mask &= ~m;
		/* m has bits to left of former mask's lowest set bit set */
		m >>= 1;
		/* Removing mask's (former) lowest set bit from value */
		value = (value & m) | ((value >> 1) & ~m);
		/* Removing mask's (former) lowest set bit from mask */
		mask >>= 1;
	}
	return value;
}

/** Returns value of field with given name in given (main of incremental) CLM
 * data set. Interface to get_data that converts field name to offset of field
 * inside CLM data registry
 * \param[in] ds_id CLM data set identifier
 * \param[in] field Name of field in struct clm_registry
 * \return Field value as (const void *) pointer. NULL if given data source was
 * not set
 */
#define GET_DATA(ds_id, field) get_data(ds_id, OFFSETOF(clm_registry_t, field))


/* Accessor function to avoid data_sources structure from getting into ROM.
 * Don't have this function in ROM.
 */
static data_dsc_t *
BCMRAMFN(get_data_sources)(int ds_idx)
{
	return &data_sources[ds_idx];
}

/* Accessor function to avoid rate_type structure from getting into ROM.
 * Don't have this function in ROM.
 */
static char *
BCMRAMFN(get_rate_type)(void)
{
	return rate_type;
}

/** Returns value of field with given offset in given (main or incremental) CLM
 * data set
 * \param[in] ds_idx CLM data set identifier
 * \param[in] field_offset Offset of field in struct clm_registry
 * \return Field value as (const void *) pointer. NULL if given data source was
 * not set
 */
static const void *
get_data(int ds_idx, unsigned long field_offset)
{
	data_dsc_t *ds = get_data_sources(ds_idx);
	const uintptr paddr = (uintptr)ds->data;
	const char **pp = (const char **)(paddr + field_offset);

	return (ds->data && *pp) ? (*pp + ds->relocation) : NULL;
}

/** Converts given pointer value, fetched from some (possibly relocated) CLM
 * data structure to its 'true' value
 * Note that GET_DATA() return values are already converted to 'true' values
 * \param[in] ds_idx Identifier of CLM data set that contained given pointer
 * \param[in] ptr Pointer to convert
 * \return 'True' (unrelocated) pointer value
 */
static const void *
relocate_ptr(int ds_idx, const void *ptr)
{
	return ptr ? ((const char *)ptr + get_data_sources(ds_idx)->relocation) : NULL;
}

/** Returns address of data item, stored in one of ..._set_t structures
 * \param[in] ds_idx Identifier of CLM data set
 * \param[in] field_offset Offset of address of set field in clm_registry_t
 * \param[in] num_offset Offset of 'num' field in ..._set_t structure
 * \param[in] set_offset Offset of 'set' field in ..._set_t structure
 * \param[in] rec_len Length of record referenced by 'set' field
 * \param[in] idx Index of requested field
 * \param[out] num Optional output parameter - value of 'num' field of ..._set
    structure. -1 if structure not found
 * \return Address of idx's field of vector, referenced by set field or NULL
 */
static const void *
get_item(int ds_idx, unsigned long field_offset, unsigned long num_offset,
	unsigned long set_offset, int rec_len, int idx, int *num)
{
	const void *data = get_data(ds_idx, field_offset);
	if (data) {
		int n = *(const int *)((const char *)data + num_offset);
		if (num) {
			*num = n;
		}
		return (idx < n)
				? ((const char *)relocate_ptr(ds_idx,
				*(const void * const *)((const char *)data + set_offset)) +
				idx * rec_len)
				: NULL;
	}
	if (num) {
		*num = -1;
	}
	return NULL;
}

/** Removes syntax sugar around get_item() call
 * \param[in] ds_id Identifier of CLM data set
 * \param[in] registry_field name of field in clm_registry_t
 * \param[in] set_struct ..._set_t structure
 * \param[in] record_len Length of record, referenced by 'set' field
 * \param[in] idx Index of record
 * \param[out] num Optional output parameter - value of 'num' field of -1
 * \return Address of record or 0
 */
#define GET_ITEM(ds_id, registry_field, set_struct, record_len, idx, num_value) \
	get_item(ds_id, OFFSETOF(clm_registry_t, registry_field), \
	OFFSETOF(set_struct, num), OFFSETOF(set_struct, set), record_len, idx, num_value)

/** Retrieves CLM data set identifier and index, contained in given iterator
 * Applicable to previously 'packed' iterator. Not applicable to 'just
 * initialized' iterators
 * \param[in] iter Iterator to unpack
 * \param[out] ds_id CLM data set identifier retrieved from iterator
 * \param[out] idx Index retrieved from iterator
 */
static void
iter_unpack(int iter, data_source_id_t *ds_id, int *idx)
{
	--iter;
	if (ds_id) {
		*ds_id = ((iter & ITER_DS_MASK) == ITER_DS_MAIN) ? DS_MAIN : DS_INC;
	}
	if (idx) {
		*idx = iter & ITER_IDX_MASK;
	}
}

/** Creates (packs) iterator out of CLM data set identifier and index
 * \param[out] iter Resulted iterator
 * \param[in] ds_id CLM data set identifier to put to iterator
 * \param[in] idx Index Index to put to iterator
 */
static void
iter_pack(int *iter, data_source_id_t ds_id, int idx)
{
	if (iter) {
		*iter = (((ds_id == DS_MAIN) ? ITER_DS_MAIN : ITER_DS_INC) |
				(idx & ITER_IDX_MASK)) + 1;
	}
}

/** Traversal of byte string sequence
 * \param[in] byte_string_seq Byte string sequence to traverse
 * \param[in] idx Index of string to find
 * \return Address of idx's string in a sequence
 */
static const unsigned char *
get_byte_string(const unsigned char *byte_string_seq, int idx)
{
	while (idx--) {
		byte_string_seq += *byte_string_seq + 1;
	}
	return byte_string_seq;
}

/** Looks up byte string that contains locale definition, precomputes
 * locale-related data
 * \param[in] locales Region locales
 * \param[in] loc_type Type of locale to retrieve (CLM_LOC_IDX_...)
 * \param[out] loc_data Locale-related data. If locale not found all fields are
 * zeroed
 * \return TRUE in case of success, FALSE if region locale definitions
 * structure contents is invalid
 */
static MY_BOOL
get_loc_def(const clm_country_locales_t *locales, int loc_type,
	locale_data_t *loc_data)
{
	data_source_id_t ds_id;
	int idx, bw_idx;
	MY_BOOL is_base;
	clm_band_t band;
	const unsigned char *loc_def;
	const data_dsc_t *ds;

	my_memset(loc_data, 0, sizeof(*loc_data));

	switch (loc_type) {
	case CLM_LOC_IDX_BASE_2G:
		iter_unpack(locales->locale_2G, &ds_id, &idx);
		is_base = TRUE;
		band = CLM_BAND_2G;
		break;
	case CLM_LOC_IDX_BASE_5G:
		iter_unpack(locales->locale_5G, &ds_id, &idx);
		is_base = TRUE;
		band = CLM_BAND_5G;
		break;
	case CLM_LOC_IDX_HT_2G:
		iter_unpack(locales->locale_2G_HT, &ds_id, &idx);
		is_base = FALSE;
		band = CLM_BAND_2G;
		break;
	case CLM_LOC_IDX_HT_5G:
		iter_unpack(locales->locale_5G_HT, &ds_id, &idx);
		is_base = FALSE;
		band = CLM_BAND_5G;
		break;
	default:
		return FALSE;
	}
	if (idx == CLM_LOC_NONE) {
		return TRUE;
	}
	if (idx == CLM_LOC_SAME) {
		return FALSE;
	}
	ds = get_data_sources(ds_id);
	loc_def =  (const unsigned char *)relocate_ptr(ds_id, ds->data->locales[loc_type]);
	while (idx--) {
		int tx_rec_len;
		if (is_base) {
			loc_def += CLM_LOC_DSC_BASE_HDR_LEN;
			loc_def += 1 + CLM_LOC_DSC_PUB_REC_LEN * (int)(*loc_def);
		}
		for (;;) {
			unsigned char flags = *loc_def++;
			if (flags & CLM_DATA_FLAG_FLAG2) {
				loc_def++;
			}
			tx_rec_len = CLM_LOC_DSC_TX_REC_LEN +
					((flags &
					CLM_DATA_FLAG_PER_ANT_MASK) >>
					CLM_DATA_FLAG_PER_ANT_SHIFT);
			if (!(flags & CLM_DATA_FLAG_MORE)) {
				break;
			}
			loc_def += 1 + tx_rec_len * (int)(*loc_def);
		}
		loc_def += 1 + tx_rec_len * (int)(*loc_def);
	}
	loc_data->def_ptr = loc_def;
	loc_data->chan_ranges_bw = ds->chan_ranges_bw;
	loc_data->rate_sets_bw = ds->rate_sets_bw[band];
	loc_data->ext_rate_sets_bw = ds->ext_rate_sets_bw[band];
	loc_data->valid_channels = (const unsigned char *)GET_DATA(ds_id, locale_valid_channels);
	loc_data->restricted_channels =
			(const unsigned char *)GET_DATA(ds_id, restricted_channels);
	for (bw_idx = 0; bw_idx < CLM_BW_NUM; ++bw_idx) {
		loc_data->combs[bw_idx] =
				&get_data_sources(ds_id)->valid_channel_combs[band][bw_idx];
	}
	return TRUE;
}

/** Retrieving helper data on base and HT locale on given band
 * Sets def_ptr to point to TX power data
 * \param[in] locales Region locales
 * \param[in] band Band to retrieve information for
 * \param[out, optional] base_flags If nonnull - container for BLOB base locale
 * flags
 * \return TRUE if retrieval was successful
 */
static MY_BOOL
fill_base_ht_loc_data(const clm_country_locales_t *locales, clm_band_t band,
	locale_data_t base_ht_loc_data[BH_NUM], unsigned char *base_flags)
{
	if (base_flags) {
		*base_flags = 0;
	}
	/* Computing helper information on base locale for given bandwidth */
	if (!get_loc_def(locales, (band == CLM_BAND_2G) ?
			CLM_LOC_IDX_BASE_2G : CLM_LOC_IDX_BASE_5G,
			&(base_ht_loc_data[BH_BASE])))
	{
		return FALSE;
	}
	if (base_ht_loc_data[BH_BASE].def_ptr) {
		if (base_flags) {
			/* If base locale flags needed - retrieving them */
			*base_flags = base_ht_loc_data[BH_BASE].def_ptr[CLM_LOC_DSC_FLAGS_IDX];
		}
		/* Advance local definition pointer from regulatory info to
		 * power targets info
		 */
		base_ht_loc_data[BH_BASE].def_ptr += CLM_LOC_DSC_BASE_HDR_LEN;
		base_ht_loc_data[BH_BASE].def_ptr +=
				1 + CLM_LOC_DSC_PUB_REC_LEN *
				(int)*(base_ht_loc_data[BH_BASE].def_ptr);
	}
	return get_loc_def(locales,
			(band == CLM_BAND_2G) ? CLM_LOC_IDX_HT_2G : CLM_LOC_IDX_HT_5G,
			&(base_ht_loc_data[BH_HT]));
}

/** True if given bandwidth is ULB */
static MY_BOOL
is_ulb_bw(int bw)
{
#ifdef WL11ULB
	return (bw >= CLM_BW_2_5) && (bw <= CLM_BW_10);
#else /* WL11ULB */
	return FALSE;
#endif /* WL11ULB */
}
/** Tries to fill valid_channel_combs using given CLM data source
 * This function takes information about 20MHz channels from BLOB, 40MHz
 * channels from valid_channel_...g_40m_set structures hardcoded in this module
 * to save BLOB space
 * \param[in] ds_id Identifier of CLM data to get information from
 */
static void
try_fill_valid_channel_combs(data_source_id_t ds_id)
{
	const clm_channel_comb_set_t *combs[CLM_BAND_NUM][CLM_BW_NUM];
	int band, bw;
	data_dsc_t *ds = get_data_sources(ds_id);
	if (!ds->data) {
		/* Can't obtain data - make all combs empty */
		for (band = 0; band < CLM_BAND_NUM; ++band) {
			for (bw = 0; bw < CLM_BW_NUM; ++bw) {
				ds->valid_channel_combs[band][bw].num = 0;
			}
		}
		return;
	}
	/* Fill combs[][] with references to combs that will be retrieved from
	 * BLOB
	 */
	my_memset((void *)combs, 0, sizeof(combs));
	/* 20MHz for sure */
	combs[CLM_BAND_2G][CLM_BW_20] =
			(const clm_channel_comb_set_t*)GET_DATA(ds_id, valid_channels_2g_20m);
	combs[CLM_BAND_5G][CLM_BW_20] =
			(const clm_channel_comb_set_t*)GET_DATA(ds_id, valid_channels_5g_20m);
	if (ds->registry_flags & CLM_REGISTRY_FLAG_HIGH_BW_COMBS) {
		/* Higher bandwidths - only if they are there */
		static const struct {
			clm_band_t band;
			clm_bandwidth_t bw;
			unsigned int field_offset;
		} fields [] = {
			{CLM_BAND_2G, CLM_BW_40,
			OFFSETOF(clm_registry_t, valid_channels_2g_40m)},
			{CLM_BAND_5G, CLM_BW_40,
			OFFSETOF(clm_registry_t, valid_channels_5g_40m)},
#ifdef WL11AC
			{CLM_BAND_5G, CLM_BW_80,
			OFFSETOF(clm_registry_t, valid_channels_5g_80m)},
			{CLM_BAND_5G, CLM_BW_160,
			OFFSETOF(clm_registry_t, valid_channels_5g_160m)},
#endif /* WL11AC */
#ifdef WL11ULB
			{CLM_BAND_2G, CLM_BW_2_5,
			OFFSETOF(clm_registry_t, valid_channels_2g_2_5m)},
			{CLM_BAND_2G, CLM_BW_5,
			OFFSETOF(clm_registry_t, valid_channels_2g_5m)},
			{CLM_BAND_2G, CLM_BW_10,
			OFFSETOF(clm_registry_t, valid_channels_2g_10m)},
			{CLM_BAND_5G, CLM_BW_2_5,
			OFFSETOF(clm_registry_t, valid_channels_5g_2_5m)},
			{CLM_BAND_5G, CLM_BW_5,
			OFFSETOF(clm_registry_t, valid_channels_5g_5m)},
			{CLM_BAND_5G, CLM_BW_10,
			OFFSETOF(clm_registry_t, valid_channels_5g_10m)},
#endif /* WL11ULB */
		};
		int idx;
		for (idx = 0; idx < (int)ARRAYSIZE(fields); ++idx) {
			if (!is_ulb_bw(fields[idx].bw) ||
				(ds->registry_flags & CLM_REGISTRY_FLAG_ULB))
			{
				combs[fields[idx].band][fields[idx].bw] =
						(const clm_channel_comb_set_t*)get_data(ds_id,
						fields[idx].field_offset);
			}
		}
	}
	/* Transferring combs from BLOB to valid_channel_combs[][] */
	for (band = 0; band < CLM_BAND_NUM; ++band) {
		for (bw = 0; bw < CLM_BW_NUM; ++bw) {
			const clm_channel_comb_set_t *comb_set = combs[band][bw];
			clm_channel_comb_set_t *dest_comb_set =
				&ds->valid_channel_combs[band][bw];
			if (!comb_set || !comb_set->num) {
				/* Incremental comb set empty or absent?
				 * Borrowing from base
				 */
				if (ds_id == DS_INC) {
					*dest_comb_set =
							get_data_sources(DS_MAIN)->
							valid_channel_combs[band][bw];
				}
				continue;
			}
			/* Nonempty comb set - first copy all fields... */
			*dest_comb_set = *comb_set;
			/* ... then relocate pointer (to comb vector) */
			dest_comb_set->set =
				(const clm_channel_comb_t *)relocate_ptr(ds_id, comb_set->set);
			/* Base comb set empty? Share from incremental */
			if ((ds_id == DS_INC) &&
				!get_data_sources(DS_MAIN)->valid_channel_combs[band][bw].num)
			{
				get_data_sources(DS_MAIN)->valid_channel_combs[band][bw] =
						*dest_comb_set;
			}
		}
	}
	if (!(ds->registry_flags & CLM_REGISTRY_FLAG_HIGH_BW_COMBS)) {
		/* No 40+MHz combs in BLOB => using hardcoded ones */
		ds->valid_channel_combs[CLM_BAND_2G][CLM_BW_40] = valid_channel_2g_40m_set;
		ds->valid_channel_combs[CLM_BAND_5G][CLM_BW_40] = valid_channel_5g_40m_set;
#ifdef WL11AC
		ds->valid_channel_combs[CLM_BAND_5G][CLM_BW_80] = valid_channel_5g_80m_set;
		ds->valid_channel_combs[CLM_BAND_5G][CLM_BW_160] = valid_channel_5g_160m_set;
#endif /* WL11AC */
	}
}

/** In given comb set looks up for comb that contains given channel
 * \param[in] channel Channel to find comb for
 * \param[in] combs Comb set to find comb in
 * \return NULL or address of given comb
 */
static const clm_channel_comb_t *
get_comb(int channel, const clm_channel_comb_set_t *combs)
{
	const clm_channel_comb_t *ret, *comb_end;
	for (ret = combs->set, comb_end = ret + combs->num; ret != comb_end; ++ret) {
		if ((channel >= ret->first_channel) && (channel <= ret->last_channel) &&
		   (((channel - ret->first_channel) % ret->stride) == 0))
		{
			return ret;
		}
	}
	return NULL;
}

/** Among combs whose first channel is greater than given looks up one with
 * minimum first channel
 * \param[in] channel Channel to find comb for
 * \param[in] combs Comb set to find comb in
 * \return Address of found comb, NULL if all combs have first channel smaller
 * than given
 */
static const clm_channel_comb_t *
get_next_comb(int channel, const clm_channel_comb_set_t *combs)
{
	const clm_channel_comb_t *ret, *comb, *comb_end;
	for (ret = NULL, comb = combs->set, comb_end = comb+combs->num; comb != comb_end;
		++comb)
	{
		if (comb->first_channel <= channel) {
			continue;
		}
		if (!ret || (comb->first_channel < ret->first_channel)) {
			ret = comb;
		}
	}
	return ret;
}

/** Fills channel set structure from given source
 * \param[out] channels Channel set to fill
 * \param[in] channel_defs Byte string sequence that contains channel set
 * definitions
 * \param[in] def_idx Index of byte string that contains required channel set
 * definition
 * \param[in] ranges Vector of channel ranges
 * \param[in] combs Set of combs of valid channel numbers
 */
static void
get_channels(clm_channels_t *channels, const unsigned char *channel_defs,
	unsigned char def_idx, const clm_channel_range_t *ranges,
	const clm_channel_comb_set_t *combs)
{
	int num_ranges;

	if (!channels) {
		return;
	}
	my_memset(channels->bitvec, 0, sizeof(channels->bitvec));
	if (!channel_defs) {
		return;
	}
	if (def_idx == CLM_RESTRICTED_SET_NONE) {
		return;
	}
	channel_defs = get_byte_string(channel_defs, def_idx);
	num_ranges = *channel_defs++;
	while (num_ranges--) {
		unsigned char r = *channel_defs++;
		if (r == CLM_RANGE_ALL_CHANNELS) {
			const clm_channel_comb_t *comb = combs->set;
			int num_combs;
			for (num_combs = combs->num; num_combs--; ++comb) {
				int chan;
				for (chan = comb->first_channel; chan <= comb->last_channel;
					chan += comb->stride) {
					channels->bitvec[chan / 8] |=
						(unsigned char)(1 << (chan % 8));
				}
			}
		} else {
			int chan = ranges[r].start, end = ranges[r].end;
			const clm_channel_comb_t *comb = get_comb(chan, combs);
			if (!comb) {
				continue;
			}
			for (;;) {
				channels->bitvec[chan / 8] |= (unsigned char)(1 << (chan % 8));
				if (chan >= end) {
					break;
				}
				if (chan < comb->last_channel) {
					chan += comb->stride;
					continue;
				}
				comb = get_next_comb(chan, combs);
				if (!comb || (comb->first_channel > end)) {
					break;
				}
				chan = comb->first_channel;
			}
		}
	}
}

/** True if given channel belongs to given range and belong to comb that
 * represent this range
 * \param[in] channel Channel in question
 * \param[in] range Range in question
 * \param[in] combs Comb set
 * \param[in] other_in_pair Other channel in 80+80 channel pair. Used when
 * combs parameter is zero-length
 * \return True if given channel belongs to given range and belong to comb that
 * represent this range
 */
static MY_BOOL
channel_in_range(int channel, const clm_channel_range_t *range,
	const clm_channel_comb_set_t *combs, int other_in_pair)
{
	const clm_channel_comb_t *comb;
	if (!combs->num) {
		return (channel == range->start) && (other_in_pair == range->end);
	}
	if ((channel < range->start) || (channel > range->end)) {
		return FALSE;
	}
	comb = get_comb(range->start, combs);
	while (comb && (comb->last_channel < channel)) {
		comb = get_next_comb(comb->last_channel, combs);
	}
	return comb && (comb->first_channel <= channel) &&
		!((channel - comb->first_channel) % comb->stride);
}

/** Fills rate_type[] */
static void
fill_rate_types(void)
{
	/** Rate range descriptor */
	static const struct {
		/** First rate in range */
		int start;

		/* Number of rates in range */
		int length;

		/* Rate type for range */
		enum clm_rate_type rt;
	} rate_ranges[] = {
		{0,                      WL_NUMRATES,        RT_MCS},
		{WL_RATE_1X1_DSSS_1,     WL_RATESET_SZ_DSSS, RT_DSSS},
		{WL_RATE_1X2_DSSS_1,     WL_RATESET_SZ_DSSS, RT_DSSS},
		{WL_RATE_1X3_DSSS_1,     WL_RATESET_SZ_DSSS, RT_DSSS},
		{WL_RATE_1X1_OFDM_6,     WL_RATESET_SZ_OFDM, RT_OFDM},
		{WL_RATE_1X2_CDD_OFDM_6, WL_RATESET_SZ_OFDM, RT_OFDM},
		{WL_RATE_1X3_CDD_OFDM_6, WL_RATESET_SZ_OFDM, RT_OFDM},
#ifdef CLM_TXBF_RATES_SUPPORTED
		{WL_RATE_1X2_TXBF_OFDM_6, WL_RATESET_SZ_OFDM, RT_OFDM},
		{WL_RATE_1X3_TXBF_OFDM_6, WL_RATESET_SZ_OFDM, RT_OFDM},
#endif /* LM_TXBF_RATES_SUPPORTED */
#if WL_NUMRATES >= 336
		{WL_RATE_1X4_DSSS_1,     WL_RATESET_SZ_DSSS, RT_DSSS},
		{WL_RATE_1X4_CDD_OFDM_6, WL_RATESET_SZ_OFDM, RT_OFDM},
#ifdef CLM_TXBF_RATES_SUPPORTED
		{WL_RATE_1X4_TXBF_OFDM_6, WL_RATESET_SZ_OFDM, RT_OFDM},
#endif /* CLM_TXBF_RATES_SUPPORTED */
#endif /* WL_NUMRATES >= 336 */
	};
	int range_idx;
	for (range_idx = 0; range_idx < (int)ARRAYSIZE(rate_ranges); ++range_idx) {
		int rate_idx = rate_ranges[range_idx].start;
		int end = rate_idx + rate_ranges[range_idx].length;
		enum clm_rate_type rt = rate_ranges[range_idx].rt;
		do {
			SET_RATE_TYPE(rate_idx, rt);
		} while (++rate_idx < end);
	}
}

/** True if BLOB format with given major version supported
 * Made separate function to avoid ROM invalidation (albeit it is always wrong idea to put
 * CLM to ROM)
 * \param[in] BLOB major version
 * Returns TRUE if given major BLOB format version supported
 */
static MY_BOOL is_blob_version_supported(int format_major)
{
	return (format_major <= FORMAT_VERSION_MAJOR) &&
			(format_major >= FORMAT_MIN_COMPAT_MAJOR);
}

/** Initializes given CLM data source
 * \param[in] header Header of CLM data
 * \param[in] ds_id Identifier of CLM data set
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if data address is
 * zero or if CLM data tag is absent at given address or if major number of CLM
 * data format version is not supported by CLM API
 */
static clm_result_t
data_init(const clm_data_header_t *header, data_source_id_t ds_id)
{
	data_dsc_t *ds = get_data_sources(ds_id);
	if (header) {
		MY_BOOL has_registry_flags = TRUE;
		if (my_strcmp(header->header_tag, CLM_HEADER_TAG)) {
			return CLM_RESULT_ERR;
		}
		if ((header->format_major == 5) && (header->format_minor == 1)) {
			has_registry_flags = FALSE;
		} else if (!is_blob_version_supported(header->format_major)) {
			return CLM_RESULT_ERR;
		}
		ds->scr_idx_4 = (header->format_major == 16) || (header->format_major == 17);
		ds->relocation = (unsigned)((const char*)header -(const char*)header->self_pointer);
		ds->data = (const clm_registry_t*)relocate_ptr(ds_id, header->data);
		ds->registry_flags = has_registry_flags ? ds->data->flags : 0;
		ds->reg_flags_2_idx = -1;
		if (has_registry_flags) {
			int blob_num_rates = (ds->data->flags & CLM_REGISTRY_FLAG_NUM_RATES_MASK) >>
				CLM_REGISTRY_FLAG_NUM_RATES_SHIFT;
			/* To avoid extension of this field - just modulo
			 * comparison)
			 */
			if (blob_num_rates &&
					(blob_num_rates !=
					(WL_NUMRATES & (CLM_REGISTRY_FLAG_NUM_RATES_MASK >>
					CLM_REGISTRY_FLAG_NUM_RATES_SHIFT))))
			{
				return CLM_RESULT_ERR;
			}
			if (ds->data->flags & CLM_REGISTRY_FLAG_REGREV_REMAP) {
				ds->regrev_remap = (const clm_regrev_cc_remap_set_t *)
						GET_DATA(ds_id, regrev_remap);
			}
			/* Unknown flags present. May never happen for
			 * regularly released ClmAPI sources, but can for
			 * ClmAPI with patched-in features from more recent
			 * BLOB formats
			 */
			if (ds->registry_flags & ~CLM_REGISTRY_FLAG_ALL) {
				return CLM_RESULT_ERR;
			}
		}
		ds->header = header;
		if (has_registry_flags && (ds->data->flags & CLM_REGISTRY_FLAG_CD_REGIONS)) {
			int idx = 0;
			int extra_loc_idx_bytes =
					(ds->data->flags &
					CLM_REGISTRY_FLAG_CD_LOC_IDX_BYTES_MASK) >>
					CLM_REGISTRY_FLAG_CD_LOC_IDX_BYTES_SHIFT;
			ds->reg_rev16_idx =
					(ds->data->flags & CLM_REGISTRY_FLAG_CD_16_BIT_REV)
					? idx++ : -1;
			ds->reg_loc10_idx = (extra_loc_idx_bytes-- > 0) ? idx++ : -1;
			ds->reg_loc12_idx = (extra_loc_idx_bytes-- > 0) ? idx++ : -1;
			ds->reg_flags_idx = idx++;
			if (ds->data->flags & CLM_REGISTRY_FLAG_REGION_FLAG_2) {
				ds->reg_flags_2_idx = idx++;
			}
			ds->country_rev_rec_len =
					OFFSETOF(clm_country_rev_definition_cd10_t, extra) + idx;
			ds->ccrev_format =
					(ds->data->flags &
					CLM_REGISTRY_FLAG_CD_16_BIT_REGION_INDEX)
					? CCREV_FORMAT_CC_IDX16 : CCREV_FORMAT_CC_IDX8;
		} else if (has_registry_flags &&
				(ds->data->flags & CLM_REGISTRY_FLAG_COUNTRY_10_FL))
		{
			ds->reg_rev16_idx = -1;
			ds->reg_loc10_idx = 0;
			ds->reg_loc12_idx = -1;
			ds->reg_flags_idx = 1;
			ds->country_rev_rec_len = sizeof(clm_country_rev_definition10_fl_t);
			ds->ccrev_format = CCREV_FORMAT_CC_REV;
		} else {
			ds->reg_rev16_idx = -1;
			ds->reg_loc10_idx = -1;
			ds->reg_loc12_idx = -1;
			ds->reg_flags_idx = -1;
			ds->country_rev_rec_len = sizeof(clm_country_rev_definition_t);
			ds->ccrev_format = CCREV_FORMAT_CC_REV;
		}
		if (has_registry_flags && (ds->data->flags & CLM_REGISTRY_FLAG_PER_BW_RS)) {
			const unsigned char **rate_sets_5g = ds->rate_sets_bw[CLM_BAND_5G];
			const unsigned char **rate_sets_2g = ds->rate_sets_bw[CLM_BAND_2G];
			const clm_channel_range_t **chan_ranges_bw = ds->chan_ranges_bw;
			chan_ranges_bw[CLM_BW_20] =
					(const clm_channel_range_t *)GET_DATA(ds_id,
					channel_ranges_20m);
			chan_ranges_bw[CLM_BW_40] =
					(const clm_channel_range_t *)GET_DATA(ds_id,
					channel_ranges_40m);
#ifdef WL11AC
			chan_ranges_bw[CLM_BW_80_80] = ds->chan_ranges_bw[CLM_BW_80] =
					(const clm_channel_range_t *)GET_DATA(ds_id,
					channel_ranges_80m);
			chan_ranges_bw[CLM_BW_160] =
					(const clm_channel_range_t *)GET_DATA(ds_id,
					channel_ranges_160m);
#endif /* WL11AC */
#ifdef WL11ULB
			chan_ranges_bw[CLM_BW_2_5] =
					(const clm_channel_range_t *)GET_DATA(ds_id,
					channel_ranges_2_5m);
			chan_ranges_bw[CLM_BW_5] =
					(const clm_channel_range_t *)GET_DATA(ds_id,
					channel_ranges_5m);
			chan_ranges_bw[CLM_BW_10] =
					(const clm_channel_range_t *)GET_DATA(ds_id,
					channel_ranges_10m);
#endif /* WL11ULB */
			rate_sets_5g[CLM_BW_20] =
					(const unsigned char *)GET_DATA(ds_id,
					locale_rate_sets_5g_20m);
			rate_sets_5g[CLM_BW_40] =
					(const unsigned char *)GET_DATA(ds_id,
					locale_rate_sets_5g_40m);
#ifdef WL11AC
			rate_sets_5g[CLM_BW_80_80] =
					rate_sets_5g[CLM_BW_80] =
					(const unsigned char *)GET_DATA(ds_id,
					locale_rate_sets_5g_80m);
			rate_sets_5g[CLM_BW_160] =
					(const unsigned char *)GET_DATA(ds_id,
					locale_rate_sets_5g_160m);
#endif /* WL11AC */
			if (ds->data->flags & CLM_REGISTRY_FLAG_PER_BAND_RATES) {
				rate_sets_2g[CLM_BW_20] =
						(const unsigned char *)GET_DATA(ds_id,
						locale_rate_sets_2g_20m);
				rate_sets_2g[CLM_BW_40] =
					(const unsigned char *)GET_DATA(ds_id,
					locale_rate_sets_2g_40m);
			} else {
				rate_sets_2g[CLM_BW_20] = rate_sets_5g[CLM_BW_20];
				rate_sets_2g[CLM_BW_40] = rate_sets_5g[CLM_BW_40];
			}
			if (ds->data->flags & CLM_REGISTRY_FLAG_EXT_RATE_SETS) {
				static const struct {
					clm_band_t band;
					clm_bandwidth_t bw;
					unsigned int field_offset;
				} ext_rate_sets[] = {
					{CLM_BAND_2G, CLM_BW_20,
					OFFSETOF(clm_registry_t, locale_ext_rate_sets_2g_20m)},
					{CLM_BAND_2G, CLM_BW_40,
					OFFSETOF(clm_registry_t, locale_ext_rate_sets_2g_40m)},
					{CLM_BAND_5G, CLM_BW_20,
					OFFSETOF(clm_registry_t, locale_ext_rate_sets_5g_20m)},
					{CLM_BAND_5G, CLM_BW_40,
					OFFSETOF(clm_registry_t, locale_ext_rate_sets_5g_40m)},
#ifdef WL11AC
					{CLM_BAND_5G, CLM_BW_80,
					OFFSETOF(clm_registry_t, locale_ext_rate_sets_5g_80m)},
					{CLM_BAND_5G, CLM_BW_80_80,
					OFFSETOF(clm_registry_t, locale_ext_rate_sets_5g_80m)},
					{CLM_BAND_5G, CLM_BW_160,
					OFFSETOF(clm_registry_t, locale_ext_rate_sets_5g_160m)}
#endif /* WL11AC */
				};
				int idx;
				for (idx = 0; idx < (int)ARRAYSIZE(ext_rate_sets); ++idx) {
					ds->ext_rate_sets_bw[ext_rate_sets[idx].band]
						[ext_rate_sets[idx].bw] =
						(const unsigned char *)get_data(ds_id,
						ext_rate_sets[idx].field_offset);
				}
			}
#ifdef WL11ULB
			if (ds->data->flags & CLM_REGISTRY_FLAG_ULB) {
				int band, bw;
				for (band = 0; band < CLM_BAND_NUM; ++band) {
					for (bw = CLM_BW_2_5; bw <= CLM_BW_10; ++bw) {
						ds->rate_sets_bw[band][bw] =
								ds->rate_sets_bw
								[band][CLM_BW_20];
						ds->ext_rate_sets_bw[band][bw] =
								ds->ext_rate_sets_bw
								[band][CLM_BW_20];
					}
				}
			}
#endif /* WL11ULB */
		} else {
			int band, bw;
			for (bw = 0; bw < CLM_BW_NUM; ++bw) {
				const clm_channel_range_t *channel_ranges =
						(const clm_channel_range_t *)GET_DATA(ds_id,
						channel_ranges_20m);
				const unsigned char *rate_sets =
						(const unsigned char *)GET_DATA(ds_id,
						locale_rate_sets_5g_20m);
				ds->chan_ranges_bw[bw] = channel_ranges;
				for (band = 0; band < CLM_BAND_NUM; ++band) {
					ds->rate_sets_bw[band][bw] = rate_sets;
				}
			}
		}
	} else {
		ds->relocation = 0;
		ds->data = NULL;
	}
	try_fill_valid_channel_combs(ds_id);
	if (ds_id == DS_MAIN) {
		try_fill_valid_channel_combs(DS_INC);
	}
	fill_rate_types();
#ifdef WL11ULB
	set_bw_to_ulb(CLM_BW_20, CLM_BW_2_5);
	set_bw_to_ulb(CLM_BW_40, CLM_BW_5);
#ifdef WL11AC
	set_bw_to_ulb(CLM_BW_80, CLM_BW_10);
#else
	set_bw_to_ulb(CLM_BW_40 + 1, CLM_BW_10);
#endif
#endif /* WL11ULB */
	return CLM_RESULT_OK;
}

/** True if two given CC/Revs are equal
 * \param[in] cc_rev1 First CC/Rev
 * \param[in] cc_rev2 Second CC/Rev
 * \return True if two given CC/Revs are equal
 */
static MY_BOOL
cc_rev_equal(const clm_cc_rev4_t *cc_rev1, const clm_cc_rev4_t *cc_rev2)
{
	return (cc_rev1->cc[0] == cc_rev2->cc[0]) && (cc_rev1->cc[1] == cc_rev2->cc[1]) &&
		(cc_rev1->rev == cc_rev2->rev);
}

/** True if two given CCs are equal
 * \param[in] cc1 First CC
 * \param[in] cc2 Second CC
 * \return
 */
static MY_BOOL
cc_equal(const char *cc1, const char *cc2)
{
	return (cc1[0] == cc2[0]) && (cc1[1] == cc2[1]);
}

/** Copies CC
 * \param[out] to Destination CC
 * \param[in] from Source CC
 */
static void
copy_cc(char *to, const char *from)
{
	to[0] = from[0];
	to[1] = from[1];
}

/** Translates old-style CC to new style
 * For now only "\0\0" -> "ww" translation is performed
 * \param[in,out] cc On input points to CC to be translated, on output points
 * to translated CC
 */
static const ccode_t ww = {'w', 'w'};
static void
translate_cc(const char **cc)
{
	if (((*cc)[0] == 0) && ((*cc)[1] == 0)) {
		*cc = ww;
	}
}

/** Returns country definition by index
 * \param[in] ds_id Data set to look in
 * \param[in] idx Country definition index
 * \param[out] num_countries Optional output parameter - number of regions
 * \return Country definition address, NULL if data set contains no countries
 * or index is out of range
 */
static const clm_country_rev_definition_cd10_t *
get_country_def_by_idx(data_source_id_t ds_id, int idx, int *num_countries)
{
	return (const clm_country_rev_definition_cd10_t *)
		GET_ITEM(ds_id, countries, clm_country_rev_definition_set_t,
		get_data_sources(ds_id)->country_rev_rec_len, idx, num_countries);
}

/** Performs 8->18 regrev remap
 * This function assumes that remap is needed - i.e. CLM data set uses 8 bit
 * regrevs and has CLM_REGISTRY_FLAG_REGREV_REMAP flag set
 * \param[in] ds_id Data set from which CC/rev was retrieved
 * \param[in,out] ccrev CC/rev to remap
 */
static void
remap_regrev(data_source_id_t ds_id, clm_cc_rev4_t *ccrev)
{
	const clm_regrev_cc_remap_set_t *regrev_remap_set =
			get_data_sources(ds_id)->regrev_remap;
	const clm_regrev_cc_remap_t *cc_remap =
			(const clm_regrev_cc_remap_t *)relocate_ptr(ds_id,
			regrev_remap_set->cc_remaps);
	const clm_regrev_cc_remap_t *cc_remap_end = cc_remap + regrev_remap_set->num;

	unsigned int num_regrevs;
	const clm_regrev_regrev_remap_t *regrev_regrev_remap;
	for (; cc_remap < cc_remap_end;	++cc_remap)
	{
		if (cc_equal(ccrev->cc, cc_remap->cc)) {
			break;
		}
	}
	if (cc_remap >= cc_remap_end) {
		return;
	}
	regrev_regrev_remap = (const clm_regrev_regrev_remap_t *)relocate_ptr(ds_id,
			regrev_remap_set->regrev_remaps) + cc_remap->index;
	for (num_regrevs = cc_remap[1].index - cc_remap->index; num_regrevs--;
			++regrev_regrev_remap)
	{
		if (regrev_regrev_remap->r8 == ccrev->rev) {
			ccrev->rev = (unsigned int)regrev_regrev_remap->r16l +
					((unsigned int)regrev_regrev_remap->r16h << 8);
			break;
		}
	}
}

/** Reads CC/rev from region (country) record
 * \param[in] ds_id Identifier of CLM data set
 * \param[in] country Region record to read from
 * \param[out] result Buffer for result
 */
static void
get_country_ccrev(data_source_id_t ds_id,
	const clm_country_rev_definition_cd10_t *country,
	clm_cc_rev4_t *result)
{
	data_dsc_t *ds = get_data_sources(ds_id);
	copy_cc(result->cc, country->cc_rev.cc);
	result->rev = country->cc_rev.rev;
	if (ds->reg_rev16_idx >= 0) {
		result->rev += ((unsigned int)country->extra[ds->reg_rev16_idx]) << 8;
	} else if (result->rev == (CLM_DELETED_MAPPING & 0xFF)) {
		result->rev = CLM_DELETED_MAPPING;
	} else if (ds->regrev_remap) {
		remap_regrev(ds_id, result);
	}
}

/** Reads CC/rev from given memory address
 * CC/rev in memory may be in form of clm_cc_rev4_t, 8-bit index, 16-bit index
 * \param[in] ds_id Identifier of CLM data set
 * \param[out] result Buffer for result
 * \param[in] raw_ccrev Address of raw CC/rev or vector of CC/revs
 * \param[in] raw_ccrev_idx Index in vector of CC/revs
 */
static void
get_ccrev(data_source_id_t ds_id, clm_cc_rev4_t *result, const void *raw_ccrev,
	int raw_ccrev_idx)
{
	data_dsc_t *ds = get_data_sources(ds_id);
	const clm_cc_rev_t *plain_ccrev = NULL;
	/* Determining storage format */
	switch (ds->ccrev_format) {
	case CCREV_FORMAT_CC_REV:
		/* Stored as plain clm_cc_rev_t */
		plain_ccrev = (const clm_cc_rev_t *)raw_ccrev + raw_ccrev_idx;
		break;
	case CCREV_FORMAT_CC_IDX8:
	case CCREV_FORMAT_CC_IDX16:
		{
			/* Stored as 8-bit or 16-bit index */
			int idx = (ds->ccrev_format == CCREV_FORMAT_CC_IDX8)
					? *((const unsigned char *)raw_ccrev + raw_ccrev_idx)
					: *((const unsigned short *)raw_ccrev + raw_ccrev_idx);
			int num_countries;
			const clm_country_rev_definition_cd10_t *country =
					get_country_def_by_idx(ds_id, idx, &num_countries);
			/* Index to region table or to extra_ccrevs? */
			if (country) {
				/* Index to region table */
				get_country_ccrev(ds_id, country, result);
			} else {
				/* Index to extra_ccrev */
				const void *ccrev =
						GET_ITEM(ds_id, extra_ccrevs, clm_cc_rev_set_t,
						(ds->reg_rev16_idx >= 0) ? sizeof(clm_cc_rev4_t)
						: sizeof(clm_cc_rev_t),
						idx - num_countries, NULL);
				/* What format extra_ccrev has? */
				if (ds->reg_rev16_idx >= 0) {
					*result = *(const clm_cc_rev4_t *)ccrev;
				} else {
					/* clm_cc_rev_t structures (8-bit
					 * rev)
					 */
					plain_ccrev = (const clm_cc_rev_t *)ccrev;
				}
			}
		}
		break;
	}
	if (plain_ccrev) {
		copy_cc(result->cc, plain_ccrev->cc);
		result->rev = plain_ccrev->rev;
		if (result->rev == (CLM_DELETED_MAPPING & 0xFF)) {
			result->rev = CLM_DELETED_MAPPING;
		} else if (ds->regrev_remap) {
			remap_regrev(ds_id, result);
		}
	}
}

/** Retrieves aggregate data by aggregate index
 * \param[in] ds_id Identifier of CLM data set
 * \param[in] idx Aggregate index
 * \param[out] result Buffer for result
 * \param[out] num_aggregates Optional output parameter - number of aggregates
    in data set
 * \return TRUE if index is in valid range
 */
static MY_BOOL
get_aggregate_by_idx(data_source_id_t ds_id, int idx, aggregate_data_t *result,
	int *num_aggregates)
{
	MY_BOOL maps_indices =
			get_data_sources(ds_id)->ccrev_format != CCREV_FORMAT_CC_REV;
	const void *p =
			GET_ITEM(ds_id, aggregates, clm_aggregate_cc_set_t,
			maps_indices ? sizeof(clm_aggregate_cc16_t) : sizeof(clm_aggregate_cc_t),
			idx, num_aggregates);
	if (!p) {
		return FALSE;
	}
	if (maps_indices) {
		result->def_reg = ((const clm_aggregate_cc16_t *)p)->def_reg;
		result->num_regions = ((const clm_aggregate_cc16_t *)p)->num_regions;
		result->regions = relocate_ptr(ds_id, ((const clm_aggregate_cc16_t *)p)->regions);
	} else {
		get_ccrev(ds_id, &result->def_reg,
				&((const clm_aggregate_cc_t *)p)->def_reg, 0);
		result->num_regions = ((const clm_aggregate_cc_t *)p)->num_regions;
		result->regions = relocate_ptr(ds_id, ((const clm_aggregate_cc_t *)p)->regions);
	}
	return TRUE;
}

/** Looks for given aggregation in given data set
 * \param[in] ds_id Identifier of CLM data set
 * \param[in] cc_rev Aggregation's default region CC/rev
 * \param[out] idx Optional output parameter - index of aggregation in set
 * \param[out] result Output parameter - buffer for aggregate data
 * \return TRUE if found
 */
static MY_BOOL
get_aggregate(data_source_id_t ds_id, const clm_cc_rev4_t *cc_rev, int *idx,
	aggregate_data_t *result)
{
	int i;
	/* Making copy because *cc_rev may be part of *result */
	clm_cc_rev4_t target = *cc_rev;
	for (i = 0; get_aggregate_by_idx(ds_id, i, result, NULL); ++i) {
		if (cc_rev_equal(&result->def_reg, &target)) {
			if (idx) {
				*idx = i;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/** Looks for mapping with given CC in given aggregation
 * \param[in] ds_id Identifier of CLM data set aggregation belongs to
 * \param[in] agg Aggregation to look in
 * \param[in] cc CC to look for
 * \param[out] result Optional buffer for resulted mapping
 * \return TRUE if found
 */
static MY_BOOL
get_mapping(data_source_id_t ds_id, const aggregate_data_t *agg,
	const ccode_t cc, clm_cc_rev4_t *result)
{
	const unsigned char *mappings =	agg ? (const unsigned char *)agg->regions : NULL;
	clm_cc_rev4_t ccrev_buf;
	int i;
	if (!mappings) {
		return FALSE;
	}
	if (!result) {
		result = &ccrev_buf;
	}
	for (i = 0; i < agg->num_regions; ++i) {
		get_ccrev(ds_id, result, mappings, i);
		if (cc_equal(cc, result->cc)) {
			return TRUE;
		}
	}
	return FALSE;
}

/** Reads locale index from region record
 * \param[in] ds_id Identifier of CLM data set region record belongs to
 * \param[in] country_definition Region definition record
 * \param[in] loc_type Locale type
 * \return Locale index or one of CLM_LOC_... special indices
 */
static int
loc_idx(data_source_id_t ds_id,
	const clm_country_rev_definition_cd10_t *country_definition,
	int loc_type)
{
	int mask = 0xFF;
	int idx10 = get_data_sources(ds_id)->reg_loc10_idx;
	int idx12 = get_data_sources(ds_id)->reg_loc12_idx;
	int ret = country_definition->locales[loc_type];
	if (idx10 >= 0) {
		ret |= ((int)country_definition->extra[idx10] <<
				((CLM_LOC_IDX_NUM-loc_type)*2)) & 0x300;
		mask = 0x3FF;
	}
	if (idx12 >= 0) {
		ret |= ((int)country_definition->extra[idx12] <<
				((CLM_LOC_IDX_NUM-loc_type)*2 + 2)) & 0xC00;
		mask = 0xFFF;
	}
	if (ret == (CLM_LOC_NONE & mask)) {
		ret = CLM_LOC_NONE;
	} else if (ret == (CLM_LOC_SAME & mask)) {
		ret = CLM_LOC_SAME;
	} else if (ret == (CLM_LOC_DELETED & mask)) {
		ret = CLM_LOC_DELETED;
	}
	return ret;
}

/** True if given country definition marked as deleted
 * \param[in] ds_id Identifier of CLM data set country definition belongs to
 * \param[in] country_definition Country definition structure
 * \return True if given country definition marked as deleted
 */
static MY_BOOL
country_deleted(data_source_id_t ds_id,
	const clm_country_rev_definition_cd10_t *country_definition)
{
	return loc_idx(ds_id, country_definition, 0) == CLM_LOC_DELETED;
}

/** Looks up for definition of given country (region) in given CLM data set
 * \param[in] ds_id Data set id to look in
 * \param[in] cc_rev Region CC/rev to look for
 * \param[out] idx Optional output parameter: index of found country definition
 * \return Address of country definition or NULL
 */
static const clm_country_rev_definition_cd10_t *
get_country_def(data_source_id_t ds_id, const clm_cc_rev4_t *cc_rev, int *idx)
{
	int i;
	const clm_country_rev_definition_cd10_t *ret;
	for (i = 0; (ret = get_country_def_by_idx(ds_id, i, NULL)) != NULL; ++i) {
		clm_cc_rev4_t region_ccrev;
		get_country_ccrev(ds_id, ret, &region_ccrev);
		if (cc_rev_equal(&region_ccrev, cc_rev)) {
			if (idx) {
				*idx = i;
			}
			return ret;
		}
	}
	return NULL;
}

/** Finds subchannel rule for given main channel and fills channel table for it
 * \param[out] actual_table Table to fill. Includes channel numbers only for
 * bandwidths included in subchannel rule
 * \param[out] power_inc Power increment to apply
 * \param[in] full_table Full subchannel table to take channel numbers from
 * \param[in] limits_type Limits type (subchannel ID)
 * \param[in] channel Main channel
 * \param[in] ranges Channel ranges' table
 * \param[in] comb_set Comb set for main channel's bandwidth
 * \param[in] main_rules Array of main channel subchannel rules (each rule
 * pertinent to range of main channels)
 * \param[in] num_main_rules Number of main channel subchannel rules
 * \param[in] num_subchannels Number of subchannels in rule
 * (CLM_DATA_SUB_CHAN_MAX_... constant)
 * \param[in] registry_flags Registry flags for data set that contains
 * subchannel rules
 */
static void
fill_actual_subchan_table(unsigned char actual_table[CLM_BW_NUM],
	clm_power_t *power_inc, unsigned char full_table[CLM_BW_NUM],
	int limits_type, int channel, const clm_channel_range_t *ranges,
	const clm_channel_comb_set_t *comb_set,
	const clm_sub_chan_region_rules_t *region_rules,
	const unsigned char *increments,
	int num_subchannels, int registry_flags)
{
	/* Rule pointer as character pointer (to simplify address
	 * arithmetic)
	 */
	const unsigned char *r;
	unsigned bw_data_len = (registry_flags & CLM_REGISTRY_FLAG_SUBCHAN_RULES_INC)
			? sizeof(clm_sub_chan_rule_inc_t) : sizeof(unsigned char);
	unsigned chan_rule_len = CLM_SUB_CHAN_RULES_IDX + (bw_data_len * num_subchannels);
	int rule_index = 0;

	/* Loop over subchannel rules rules */
	for (r = (const unsigned char *)region_rules->channel_rules;
			rule_index < region_rules->num; r += chan_rule_len, ++rule_index)
	{
		/* Did we find rule for range that contains given main
		 * channel?
		 */
		if (channel_in_range(channel, ranges + r[CLM_SUB_CHAN_RANGE_IDX], comb_set, 0)) {
			/* Rule found - now let's fill the table */

			/* Loop index, actual type is clm_bandwidth_t */
			int bw_idx;
			/* Subchannel rule (cell in 'Rules' page) */
			const clm_sub_chan_rule_inc_t *sub_chan_rule =
					(const clm_sub_chan_rule_inc_t *)
					(r + CLM_SUB_CHAN_RULES_IDX +
					(limits_type - 1) * bw_data_len);
			/* Probing all possible bandwidths */
			for (bw_idx = 0; bw_idx < CLM_BW_NUM; ++bw_idx) {
				/* If bandwidth included to rule */
				if ((1 << (bw_idx - CLM_BW_20)) & sub_chan_rule->bw) {
					/* Copy channel number for this
					 * bandwidth from full table
					 */
					actual_table[bw_idx] = full_table[bw_idx];
				}
			}
			*power_inc = (registry_flags & CLM_REGISTRY_FLAG_SUBCHAN_RULES_INC) ?
					sub_chan_rule->inc :
					(increments ? increments[rule_index * num_subchannels
					+ (limits_type - 1)] : 0);
			return; /* All done, no need to look for more rules */
		}
	}
}

clm_result_t
clm_init(const struct clm_data_header *header)
{
#ifdef WL11AC
	bw_width_to_idx = bw_width_to_idx_ac;
#else /* WL11AC */
	bw_width_to_idx = bw_width_to_idx_non_ac;
#endif /* WL11AC */
	return data_init(header, DS_MAIN);
}

clm_result_t
clm_set_inc_data(const struct clm_data_header *header)
{
	return data_init(header, DS_INC);
}

clm_result_t
clm_iter_init(int *iter)
{
	if (iter) {
		*iter = CLM_ITER_NULL;
		return CLM_RESULT_OK;
	}
	return CLM_RESULT_ERR;
}

clm_result_t
clm_limits_params_init(struct clm_limits_params *params)
{
	if (!params) {
		return CLM_RESULT_ERR;
	}
	params->bw = CLM_BW_20;
	params->antenna_idx = 0;
	params->sar = 0x7F;
	params->other_80_80_chan = 0;
	return CLM_RESULT_OK;
}

clm_result_t
clm_country_iter(clm_country_t *country, ccode_t cc, unsigned int *rev)
{
	data_source_id_t ds_id;
	int idx;
	clm_result_t ret = CLM_RESULT_OK;
	if (!country || !cc || !rev) {
		return CLM_RESULT_ERR;
	}
	if (*country == CLM_ITER_NULL) {
		ds_id = DS_INC;
		idx = 0;
	} else {
		iter_unpack(*country, &ds_id, &idx);
		++idx;
	}
	for (;;) {
		int num_countries;
		const clm_country_rev_definition_cd10_t *country_definition =
				get_country_def_by_idx(ds_id, idx, &num_countries);
		clm_cc_rev4_t country_ccrev;
		if (!country_definition) {
			if (ds_id == DS_INC) {
				ds_id = DS_MAIN;
				idx = 0;
				continue;
			} else {
				ret = CLM_RESULT_NOT_FOUND;
				idx = (num_countries >= 0) ? num_countries : 0;
				break;
			}
		}
		get_country_ccrev(ds_id, country_definition, &country_ccrev);
		if (country_deleted(ds_id, country_definition)) {
			++idx;
			continue;
		}
		if ((ds_id == DS_MAIN) && get_data_sources(DS_INC)->data) {
			int i, num_inc_countries;
			const clm_country_rev_definition_cd10_t *inc_country_definition;
			for (i = 0;
					(inc_country_definition =
					get_country_def_by_idx(DS_INC, i, &num_inc_countries))
					!= NULL;
					++i)
			{
				clm_cc_rev4_t inc_country_ccrev;
				get_country_ccrev(DS_INC, inc_country_definition,
						&inc_country_ccrev);
				if (cc_rev_equal(&country_ccrev, &inc_country_ccrev)) {
					break;
				}
			}
			if (i < num_inc_countries) {
				++idx;
				continue;
			}
		}
		copy_cc(cc, country_ccrev.cc);
		*rev = country_ccrev.rev;
		break;
	}
	iter_pack(country, ds_id, idx);
	return ret;
}

clm_result_t
clm_country_lookup(const ccode_t cc, unsigned int rev, clm_country_t *country)
{
	int ds_idx;
	clm_cc_rev4_t cc_rev;
	if (!cc || !country) {
		return CLM_RESULT_ERR;
	}
	translate_cc(&cc);
	copy_cc(cc_rev.cc, cc);
	cc_rev.rev = (regrev_t)rev;
	for (ds_idx = 0; ds_idx < DS_NUM; ++ds_idx) {
		int idx;
		const clm_country_rev_definition_cd10_t *country_definition =
				get_country_def((data_source_id_t)ds_idx, &cc_rev, &idx);
		if (!country_definition) {
			continue;
		}
		if (country_deleted((data_source_id_t)ds_idx, country_definition)) {
			return CLM_RESULT_NOT_FOUND;
		}
		iter_pack(country, (data_source_id_t)ds_idx, idx);
		return CLM_RESULT_OK;
	}
	return CLM_RESULT_NOT_FOUND;
}

clm_result_t
clm_country_def(const clm_country_t country, clm_country_locales_t *locales)
{
	struct locale_field_dsc {
		unsigned clm_offset;
		int data_idx;
	};
	static const struct locale_field_dsc fields[] = {
		{OFFSETOF(clm_country_locales_t, locale_2G),	CLM_LOC_IDX_BASE_2G},
		{OFFSETOF(clm_country_locales_t, locale_5G),	CLM_LOC_IDX_BASE_5G},
		{OFFSETOF(clm_country_locales_t, locale_2G_HT), CLM_LOC_IDX_HT_2G},
		{OFFSETOF(clm_country_locales_t, locale_5G_HT), CLM_LOC_IDX_HT_5G},
	};
	data_source_id_t ds_id;
	int idx, i;
	const clm_country_rev_definition_cd10_t *country_definition;
	const clm_country_rev_definition_cd10_t *main_country_definition = NULL;
	int flags_idx;
	if (!locales) {
		return CLM_RESULT_ERR;
	}
	iter_unpack(country, &ds_id, &idx);
	country_definition = get_country_def_by_idx(ds_id, idx, NULL);
	if (!country_definition) {
		return CLM_RESULT_NOT_FOUND;
	}
	for (i = 0; i < (int)ARRAYSIZE(fields); ++i) {
		data_source_id_t locale_ds_id = ds_id;
		int locale_idx = loc_idx(locale_ds_id, country_definition, fields[i].data_idx);
		if (locale_idx == CLM_LOC_SAME) {
			if (!main_country_definition) {
				clm_cc_rev4_t country_ccrev;
				get_country_ccrev(ds_id, country_definition, &country_ccrev);
				main_country_definition =
						get_country_def(DS_MAIN, &country_ccrev, NULL);
			}
			locale_ds_id = DS_MAIN;
			locale_idx = main_country_definition
					? loc_idx(locale_ds_id, main_country_definition,
					fields[i].data_idx)
					: CLM_LOC_NONE;
		}
		iter_pack((int*)((char *)locales + fields[i].clm_offset), locale_ds_id,
				locale_idx);
	}
	flags_idx = get_data_sources(ds_id)->reg_flags_idx;
	locales->country_flags = (flags_idx >= 0) ? country_definition->extra[flags_idx] : 0;
	flags_idx = get_data_sources(ds_id)->reg_flags_2_idx;
	locales->country_flags_2 = (flags_idx >= 0) ? country_definition->extra[flags_idx] : 0;
	locales->computed_flags = (unsigned char)ds_id;
	return CLM_RESULT_OK;
}

clm_result_t
clm_country_channels(const clm_country_locales_t *locales, clm_band_t band,
	clm_channels_t *valid_channels, clm_channels_t *restricted_channels)
{
	clm_channels_t dummy_valid_channels;
	locale_data_t loc_data;

	if (!locales || ((unsigned)band >= (unsigned)CLM_BAND_NUM)) {
		return CLM_RESULT_ERR;
	}
	if (!restricted_channels && !valid_channels) {
		return CLM_RESULT_OK;
	}
	if (!valid_channels) {
		valid_channels = &dummy_valid_channels;
	}
	if (!get_loc_def(locales, (band == CLM_BAND_2G) ?
			CLM_LOC_IDX_BASE_2G : CLM_LOC_IDX_BASE_5G, &loc_data))
	{
		return CLM_RESULT_ERR;
	}
	if (loc_data.def_ptr) {
		get_channels(valid_channels, loc_data.valid_channels,
				loc_data.def_ptr[CLM_LOC_DSC_CHANNELS_IDX],
				loc_data.chan_ranges_bw[CLM_BW_20],
				loc_data.combs[CLM_BW_20]);
		get_channels(restricted_channels, loc_data.restricted_channels,
				loc_data.def_ptr[CLM_LOC_DSC_RESTRICTED_IDX],
				loc_data.chan_ranges_bw[CLM_BW_20],
				loc_data.combs[CLM_BW_20]);
		if (restricted_channels) {
			int i;
			for (i = 0; i < (int)ARRAYSIZE(restricted_channels->bitvec); ++i) {
				restricted_channels->bitvec[i] &= valid_channels->bitvec[i];
			}
		}
	} else {
		get_channels(valid_channels, NULL, 0, NULL, NULL);
		get_channels(restricted_channels, NULL, 0, NULL, NULL);
	}
	return CLM_RESULT_OK;
}

clm_result_t
clm_country_flags(const clm_country_locales_t *locales, clm_band_t band,
	unsigned long *ret_flags)
{
	int base_ht_idx;
	locale_data_t base_ht_loc_data[BH_NUM];
	unsigned char base_flags;
	if (!locales || !ret_flags || ((unsigned)band >= (unsigned)CLM_BAND_NUM)) {
		return CLM_RESULT_ERR;
	}
	*ret_flags = (unsigned long)(CLM_FLAG_DFS_NONE | CLM_FLAG_NO_40MHZ | CLM_FLAG_NO_80MHZ |
			CLM_FLAG_NO_80_80MHZ | CLM_FLAG_NO_160MHZ | CLM_FLAG_NO_MIMO);
	if (!fill_base_ht_loc_data(locales, band, base_ht_loc_data, &base_flags)) {
		return CLM_RESULT_ERR;
	}
	switch (base_flags & CLM_DATA_FLAG_DFS_MASK) {
	case CLM_DATA_FLAG_DFS_NONE:
		*ret_flags |= CLM_FLAG_DFS_NONE;
		break;
	case CLM_DATA_FLAG_DFS_EU:
		*ret_flags |= CLM_FLAG_DFS_EU;
		break;
	case CLM_DATA_FLAG_DFS_US:
		*ret_flags |= CLM_FLAG_DFS_US;
		break;
	case CLM_DATA_FLAG_DFS_TW:
		*ret_flags |= CLM_FLAG_DFS_TW;
		break;
	}
	if (base_flags & CLM_DATA_FLAG_FILTWAR1) {
		*ret_flags |= CLM_FLAG_FILTWAR1;
	}
	for (base_ht_idx = 0; base_ht_idx < (int)ARRAYSIZE(base_ht_loc_data); ++base_ht_idx) {
		unsigned char flags, flags2;
		const unsigned char *tx_rec = base_ht_loc_data[base_ht_idx].def_ptr;

		if (!tx_rec) {
			continue;
		}
		do {
			int num_rec, tx_rec_len;
			MY_BOOL eirp;
			unsigned char bw_idx;
			unsigned long bw_flag_mask = 0;
			const unsigned char * const *rate_sets_bw;
			unsigned int base_rate;

			flags = *tx_rec++;
			flags2 = (flags & CLM_DATA_FLAG_FLAG2) ? *tx_rec++ : 0;
			bw_idx = bw_width_to_idx[flags & CLM_DATA_FLAG_WIDTH_MASK];
#ifdef WL11ULB
			if (flags2 & CLM_DATA_FLAG2_WIDTH_EXT) {
				bw_idx =  get_bw_to_ulb(bw_idx);
			}
#endif /* WL11ULB */
			if (bw_idx == CLM_BW_40) {
				bw_flag_mask = CLM_FLAG_NO_40MHZ;
#ifdef WL11AC
			} else if (bw_idx == CLM_BW_80) {
				bw_flag_mask = CLM_FLAG_NO_80MHZ;
			} else if (bw_idx == CLM_BW_80_80) {
				bw_flag_mask = CLM_FLAG_NO_80_80MHZ;
			} else if (bw_idx == CLM_BW_160) {
				bw_flag_mask = CLM_FLAG_NO_160MHZ;
#endif /* WL11AC */
			}
			tx_rec_len = CLM_LOC_DSC_TX_REC_LEN + ((flags &
				CLM_DATA_FLAG_PER_ANT_MASK) >> CLM_DATA_FLAG_PER_ANT_SHIFT);
			if (tx_rec_len != CLM_LOC_DSC_TX_REC_LEN) {
				*ret_flags |= CLM_FLAG_PER_ANTENNA;
			}
			eirp = (flags & CLM_DATA_FLAG_MEAS_MASK) == CLM_DATA_FLAG_MEAS_EIRP;
			rate_sets_bw = (flags2 & CLM_DATA_FLAG2_EXT_RATES)
					? base_ht_loc_data[base_ht_idx].ext_rate_sets_bw
					: base_ht_loc_data[base_ht_idx].rate_sets_bw;
			base_rate = (flags2 & CLM_DATA_FLAG2_EXT_RATES) ? BASE_EXT_RATE : 0;
			for (num_rec = (int)*tx_rec++; num_rec--; tx_rec += tx_rec_len) {
				const unsigned char *rates =
						get_byte_string(
						rate_sets_bw[bw_idx],
						tx_rec[CLM_LOC_DSC_RATE_IDX]);
				int num_rates = *rates++;
				/* Check for a non-disabled power before
				 * clearing NO_bw flag
				 */
				if ((unsigned char)CLM_DISABLED_POWER ==
						(unsigned char)tx_rec[CLM_LOC_DSC_POWER_IDX]) {
					continue;
				}
				if (bw_flag_mask) {
					*ret_flags &= ~bw_flag_mask;
					/* clearing once should be enough */
					bw_flag_mask = 0;
				}
				while (num_rates--) {
					unsigned int rate_idx = base_rate + (*rates++);
					switch (RATE_TYPE(rate_idx)) {
					case RT_DSSS:
						if (eirp) {
							*ret_flags |= CLM_FLAG_HAS_DSSS_EIRP;
						}
						break;
					case RT_OFDM:
						if (eirp) {
							*ret_flags |= CLM_FLAG_HAS_OFDM_EIRP;
						}
						break;
					case RT_MCS:
						*ret_flags &= ~CLM_FLAG_NO_MIMO;
						break;
					}
				}
			}
		} while (flags & CLM_DATA_FLAG_MORE);
	}
	if (locales->country_flags & CLM_DATA_FLAG_REG_TXBF) {
		*ret_flags |= CLM_FLAG_TXBF;
	}
	if (locales->country_flags & CLM_DATA_FLAG_REG_DEF_FOR_CC) {
		*ret_flags |= CLM_FLAG_DEFAULT_FOR_CC;
	}
	if (locales->country_flags & CLM_DATA_FLAG_REG_EDCRS_EU) {
		*ret_flags |= CLM_FLAG_EDCRS_EU;
	}
	return CLM_RESULT_OK;
}

clm_result_t
clm_country_advertised_cc(const clm_country_t country, ccode_t advertised_cc)
{
	data_source_id_t ds_id;
	int idx, ds_idx;
	const clm_country_rev_definition_cd10_t *country_def;
	clm_cc_rev4_t cc_rev;

	if (!advertised_cc) {
		return CLM_RESULT_ERR;
	}
	iter_unpack(country, &ds_id, &idx);
	country_def = get_country_def_by_idx(ds_id, idx, NULL);
	if (!country_def) {
		return CLM_RESULT_ERR;
	}
	get_country_ccrev(ds_id, country_def, &cc_rev);
	for (ds_idx = 0; ds_idx < DS_NUM; ++ds_idx) {
		int adv_cc_idx;
		const clm_advertised_cc_t *adv_cc;
		for (adv_cc_idx = 0;
				(adv_cc = (const clm_advertised_cc_t *)GET_ITEM(ds_idx,
				advertised_ccs, clm_advertised_cc_set_t,
				sizeof(clm_advertised_cc_t), adv_cc_idx, NULL)) != NULL;
				++adv_cc_idx)
		{
			int num_aliases = adv_cc->num_aliases;
			int alias_idx;
			const void *alias = (const void *)relocate_ptr(ds_idx, adv_cc->aliases);
			if (num_aliases == CLM_DELETED_NUM) {
				continue;
			}
			if ((ds_idx == DS_MAIN) && get_data_sources(DS_INC)->data) {
				int inc_adv_cc_idx;
				const clm_advertised_cc_t *inc_adv_cc;
				for (inc_adv_cc_idx = 0;
						(inc_adv_cc =
						(const clm_advertised_cc_t *)GET_ITEM(DS_INC,
						advertised_ccs, clm_advertised_cc_set_t,
						sizeof(clm_advertised_cc_t), inc_adv_cc_idx,
						NULL)) != NULL;
						++inc_adv_cc_idx)
				{
					if (cc_equal(adv_cc->cc, inc_adv_cc->cc)) {
						break;
					}
				}
				if (inc_adv_cc) {
					continue;
				}
			}
			for (alias_idx = 0; alias_idx < num_aliases; ++alias_idx) {
				clm_cc_rev4_t alias_ccrev;
				get_ccrev((data_source_id_t)ds_idx, &alias_ccrev, alias, alias_idx);
				if (cc_rev_equal(&alias_ccrev, &cc_rev)) {
					copy_cc(advertised_cc, adv_cc->cc);
					return CLM_RESULT_OK;
				}
			}
		}
	}
	copy_cc(advertised_cc, cc_rev.cc);
	return CLM_RESULT_OK;
}

#if !defined(WLC_CLMAPI_PRE7) || defined(BCMROMBUILD)

/** Precomputes country (region) related data
 * \param[in] locales Region locales
 * \param[out] loc_data Country-related data
 */
static void
get_country_data(const clm_country_locales_t *locales,
	country_data_t *country_data)
{
	data_source_id_t ds_id =
			(data_source_id_t)(locales->computed_flags & COUNTRY_FLAGS_DS_MASK);
	const data_dsc_t *ds = get_data_sources(ds_id);
	/* Index of region subchannel rules */
	int rules_idx;
	my_memset(country_data, 0, sizeof(*country_data));
	country_data->chan_ranges_bw = ds->chan_ranges_bw;
	/* Computing subchannel rules index */
	if (ds->scr_idx_4) {
		/* Deprecated 4-bit noncontiguous index field in first region
		 * flag byte
		 */
		rules_idx = remove_extra_bits(
				locales->country_flags & CLM_DATA_FLAG_REG_SC_RULES_MASK_4,
				CLM_DATA_FLAG_REG_SC_RULES_EXTRA_BITS_4);
	} else if (ds->registry_flags & CLM_REGISTRY_FLAG_REGION_FLAG_2) {
		/* New 3+5=8-bit index located in lower 3 and lower 5 bits of
		 * first and second region flag bytes
		 */
		rules_idx = (locales->country_flags & CLM_DATA_FLAG_REG_SC_RULES_MASK) |
				((locales->country_flags_2 & CLM_DATA_FLAG_2_REG_SC_RULES_MASK)
				<< CLM_DATA_FLAG_REG_SC_RULES_MASK_WIDTH);
	} else {
		/* Original 3-bit index */
		rules_idx = locales->country_flags & CLM_DATA_FLAG_REG_SC_RULES_MASK;
	}
	/* Making index 0-based */
	rules_idx -= CLM_SUB_CHAN_IDX_BASE;
	if (rules_idx >= 0) {
		/* Information for shoveling region rules from BLOB to
		 * country_data (band/bandwidth specific)
		 */
		static const struct rules_dsc {
			/* Registry flags that must be set for field to be in
			 * BLOB
			 */
			int registry_flags;
			/* Offset to pointer to clm_sub_chan_rules_set_t
			 * structure in BLOB
			 */
			unsigned registry_offset;

			/* Offset to destination rules field in country_data */
			unsigned country_data_rules_offset;

			/* Offset to destination rules field in country_data */
			unsigned country_data_increments_offset;
		} rules_dscs[] = {
			{CLM_REGISTRY_FLAG_SUB_CHAN_RULES,
			OFFSETOF(clm_registry_t, sub_chan_rules_80),
			OFFSETOF(country_data_t, sub_chan_channel_rules_80),
			OFFSETOF(country_data_t, sub_chan_increments_80)},
			{CLM_REGISTRY_FLAG_SUB_CHAN_RULES | CLM_REGISTRY_FLAG_160MHZ,
			OFFSETOF(clm_registry_t, sub_chan_rules_160),
			OFFSETOF(country_data_t, sub_chan_channel_rules_160),
			OFFSETOF(country_data_t, sub_chan_increments_160)},
			{CLM_REGISTRY_FLAG_SUB_CHAN_RULES | CLM_REGISTRY_FLAG_SUBCHAN_RULES_40,
			OFFSETOF(clm_registry_t, sub_chan_rules_2g_40m),
			OFFSETOF(country_data_t, sub_chan_channel_rules_40[CLM_BAND_2G]),
			OFFSETOF(country_data_t, sub_chan_increments_40[CLM_BAND_2G])},
			{CLM_REGISTRY_FLAG_SUB_CHAN_RULES | CLM_REGISTRY_FLAG_SUBCHAN_RULES_40,
			OFFSETOF(clm_registry_t, sub_chan_rules_5g_40m),
			OFFSETOF(country_data_t, sub_chan_channel_rules_40[CLM_BAND_5G]),
			OFFSETOF(country_data_t, sub_chan_increments_40[CLM_BAND_5G])}};
		/* Offset in BLOB to this region's rules from
		 * clm_sub_chan_rules_set_t::region_rules
		 */
		unsigned int region_rules_offset = rules_idx *
		((ds->registry_flags & CLM_REGISTRY_FLAG_SUBCHAN_RULES_INC_SEPARATE) ?
		sizeof(clm_sub_chan_region_rules_inc_t) : sizeof(clm_sub_chan_region_rules_t));
		/* Indx in rules_dscs. Corresponds to band/bandwidth */
		int ri;
		for (ri = 0; ri < ARRAYSIZE(rules_dscs); ++ri) {
			/* Address of current rules_dsc structure */
			const struct rules_dsc *rds = rules_dscs + ri;
			/* Address of sub_chan_channel_rules_XX field in
			 * destination country_data_t structure
			 */
			clm_sub_chan_region_rules_t *country_data_rules_field =
			(clm_sub_chan_region_rules_t *)((unsigned char *)country_data +
			rds->country_data_rules_offset);
			/* Address of sub_chan_increments_XX field in
			 * destination country_data_t structure
			 */
			const unsigned char **country_data_increments_field =
			(const unsigned char **)((unsigned char *)country_data +
			rds->country_data_increments_offset);
			/* Address of top-level clm_sub_chan_rules_set_t for
			 * current band/bandwidth in BLOB
			 */
			const clm_sub_chan_rules_set_t *blob_rule_set;
			/* Address of this region's
			 * clm_sub_chan_region_rules_inc_t structure in BLOB
			 */
			const clm_sub_chan_region_rules_inc_t *blob_region_rules;
			/* Can BLOB have subchannel rules for current
			 * band/bandwidth?
			 */
			if ((ds->registry_flags & rds->registry_flags) != rds->registry_flags) {
				continue;
			}
			blob_rule_set = (const clm_sub_chan_rules_set_t *)get_data(ds_id,
			rds->registry_offset);
			/* Does BLOB actually have subchannel rules for current
			 * band/bandwidth?
			 */
			if (!blob_rule_set || (rules_idx >= blob_rule_set->num)) {
				continue;
			}
			blob_region_rules = (const clm_sub_chan_region_rules_inc_t *)
			((const unsigned char *)relocate_ptr(ds_id, blob_rule_set->region_rules)
			+ region_rules_offset);
			country_data_rules_field->num = blob_region_rules->num;
			country_data_rules_field->channel_rules =
			relocate_ptr(ds_id, blob_region_rules->channel_rules);
			*country_data_increments_field = (ds->registry_flags &
			CLM_REGISTRY_FLAG_SUBCHAN_RULES_INC_SEPARATE) ?
			(const unsigned char *)relocate_ptr(ds_id, blob_region_rules->increments) :
			NULL;
		}
	}
}

/** Fills subchannel table that maps bandwidth to subchannel numbers
 * \param[out] subchannels Subchannel table being filled
 * \param[in] channel Main channel number
 * \param[in] bw Main channel bandwidth (actual type is clm_bandwidth_t)
 * \param[in] limits_type Limits type (subchannel ID)
 * \return TRUE if bandwidth/limits type combination is valid
 */
static MY_BOOL
fill_full_subchan_table(unsigned char subchannels[CLM_BW_NUM], int channel,
int bw, clm_limits_type_t limits_type)
{
	/* Descriptor of path to given subchannel */
	unsigned char path = subchan_paths[limits_type];

	/* Mask that corresponds to current step in path */
	unsigned char path_mask =
			1 << ((path & SUB_CHAN_PATH_COUNT_MASK) >> SUB_CHAN_PATH_COUNT_OFFSET);

	/* Channel number stride for current bandwidth */
	int stride = (CHAN_STRIDE << (bw - CLM_BW_20)) / 2;

	/* Emptying the map */
	my_memset(subchannels, 0, sizeof(unsigned char) * CLM_BW_NUM);
	for (;;) {
		/* Setting channel number for current bandwidth */
		subchannels[bw--] = (unsigned char)channel;

		/* Rest will related to previous (halved) bandwidth, i.e. to
		 * subchannel
		 */

		/* Is path over? */
		if ((path_mask >>= 1) == 0) {
			return TRUE; /* Yes - success */
		}
		/* Path not over but we passed through minimum bandwidth? */
		if (bw < 0) {
			return FALSE; /* Yes, failure */
		}

		/* Halving channel stride */
		stride >>= 1;

		/* Selecting subchannel number according to path */
		if (path & path_mask) {
			channel += stride;
		} else {
			channel -= stride;
		}
	}
}

/* Preliminary observations.
 * Every power in *limits is a minimum over values from several sources:
 * - For main (full) channel - over per-channel-range limits from limits that
 *   contain this channel. There can be legitimately more than one such limit:
 *   e.g. one EIRP and another Conducted (this was legal up to certain moment,
 *   not legal now but may become legal again in future)
 * - For subchannels it might be minimum over several enclosing channels (e.g.
 *   for 20-in-80 it may be minimum over corresponding 20MHz main (full)
 *   channel and 40MHz enclosing main (full) channel). Notice that all
 *   enclosing channels have different channel numbers (e.g. for 36LL it is
 *   40MHz enclosing channel 38 and 80MHz enclosing channel 42)
 * - 2.4GHz 20-in-40 channels also take power targets for DSSS rates from 20MHz
 *   channel data (even though other limits are taken from enclosing 40MHz
 *   channel)
 *
 * So in general resulting limit is a minimum of up to 3 channels (one per
 * bandwidth) and these channels have different numbers!
 * 'bw_to_chan' vector contains mapping from bandwidths to channel numbers.
 * Bandwidths not used in limit computation are mapped to 0.
 * 20-in-40 DSSS case is served by 'channel_dsss' variable, that when nonzero
 * contains number of channel where from DSSS limits shall be taken.
 *
 * 'chan_offsets' mapping is initially computed to to derive all these channel
 * numbers from main channel number, main channel bandwidth and power limit
 * type (i.e. subchannel ID)  is computed. Also computation of chan_offsets is
 * used to determine if bandwidth/limits_type pair is valid.
*/
extern clm_result_t
clm_limits(const clm_country_locales_t *locales, clm_band_t band,
	unsigned int channel, int ant_gain, clm_limits_type_t limits_type,
	const clm_limits_params_t *params, clm_power_limits_t *limits)
{
	/* Locale characteristics for base and HT locales of given band */
	locale_data_t base_ht_loc_data[BH_NUM];

	/* Loop variable. Points first to base than to HT locale */
	int base_ht_idx;

	/* Channel for DSSS rates of 20-in-40 power targets (0 if
	 * irrelevant)
	 */
	int channel_dsss = 0;

	/* Become true if at least one power target was found */
	MY_BOOL valid_channel = FALSE;

	/* Maps bandwidths to subchannel numbers.
	 * Leaves zeroes for subchannels of given limits_type
	 */
	unsigned char subchannels[CLM_BW_NUM];

	/* Country (region) precomputed data */
	country_data_t country_data;

	/* Per-bandwidth channel comb sets taken from same data source as
	 * country definition
	 */
	const clm_channel_comb_set_t *country_comb_sets;

	/* Data set descriptor */
	data_dsc_t *ds;

	/* Simple validity check */
	if (!locales || !limits || ((unsigned)band >= (unsigned)CLM_BAND_NUM) || !params ||
			((unsigned)params->bw >= (unsigned)CLM_BW_NUM) ||
			((unsigned)limits_type >= CLM_LIMITS_TYPE_NUM) ||
			((unsigned)params->antenna_idx >= WL_TX_CHAINS_MAX) ||
			((limits_type != CLM_LIMITS_TYPE_CHANNEL) && is_ulb_bw(params->bw)))
	{
		return CLM_RESULT_ERR;
	}

	/* Fills bandwidth->channel number map */
	if (!fill_full_subchan_table(subchannels, channel,
#ifdef WL11AC
			(params->bw == CLM_BW_80_80) ? CLM_BW_80 :
#endif /* WL11AC */
			params->bw, limits_type))
	{
		/** bandwidth/limits_type pair is invalid */
		return CLM_RESULT_ERR;
	}

	ds = get_data_sources((data_source_id_t)(locales->computed_flags & COUNTRY_FLAGS_DS_MASK));

	if ((band == CLM_BAND_2G) && (params->bw != CLM_BW_20) && subchannels[CLM_BW_20]) {
		/* 20-in-something, 2.4GHz. Channel to take DSSS limits from */
		channel_dsss = subchannels[CLM_BW_20];
	}
	my_memset(limits, (unsigned char)UNSPECIFIED_POWER, sizeof(limits->limit));

	/* Computing helper information on locales */
	if (!fill_base_ht_loc_data(locales, band, base_ht_loc_data, NULL)) {
		return CLM_RESULT_ERR;
	}

	/* Obtaining precomputed country data */
	get_country_data(locales, &country_data);

	/* Obtaining comb sets pertinent to data source that contains
	* country
	*/
	country_comb_sets = ds->valid_channel_combs[band];

	/* For base then HT locales do */
	for (base_ht_idx = 0; base_ht_idx < (int)ARRAYSIZE(base_ht_loc_data); ++base_ht_idx) {
		/* Precomputed helper data for current locale */
		const locale_data_t *loc_data = base_ht_loc_data + base_ht_idx;

		/* Channel combs for given band - vector indexed by channel
		 * bandwidth
		 */
		const clm_channel_comb_set_t *const* comb_sets = loc_data->combs;

		/* Transmission power records' sequence for current locale */
		const unsigned char *tx_rec = loc_data->def_ptr;

		/* CLM_DATA_FLAG_ flags for current subsequence of transmission power
		   records' sequence
		 */
		unsigned char flags, flags2;

		/* Same as subchannels, but only has nonzeroes for bandwidths,
		 * used by current subchannel rule
		 */
		unsigned char bw_to_chan[CLM_BW_NUM];

		/* Power increment from subchannel rule */
		clm_power_t power_inc = 0;

		/* Base value for rates in current rate set */
		unsigned int base_rate;

		/* No transmission power records - nothing to do for this
		 * locale
		 */
		if (!tx_rec) {
			continue;
		}

		/* Now computing 'bw_to_chan' - bandwidth to channel map that
		 * determines which channels will be used for limit computation
		 */
		/* Preset to 'no bandwidths' */
		my_memset(bw_to_chan, 0, sizeof(bw_to_chan));
		if (limits_type == CLM_LIMITS_TYPE_CHANNEL) {
			/* Main channel case - bandwidth specified as
			 * parameter, channel specified as parameter
			 */
			bw_to_chan[params->bw] = (unsigned char)channel;
		} else if (params->bw == CLM_BW_40) {
			clm_sub_chan_region_rules_t *rules_40 =
					&country_data.sub_chan_channel_rules_40[band];
			if (rules_40->channel_rules) {
				/* Explicit 20-in-40 rules are defined */
				fill_actual_subchan_table(bw_to_chan, &power_inc, subchannels,
						limits_type, channel,
						country_data.chan_ranges_bw[params->bw],
						&country_comb_sets[CLM_BW_40],
						rules_40,
						country_data.sub_chan_increments_40[band],
						CLM_DATA_SUB_CHAN_MAX_40, ds->registry_flags);
			} else {
				/* Default. Use 20-in-40 'limit from 40MHz' rule */
				bw_to_chan[CLM_BW_40] = (unsigned char)channel;
			}
#ifdef WL11AC
		} else if (params->bw == CLM_BW_80) {
			fill_actual_subchan_table(bw_to_chan, &power_inc, subchannels, limits_type,
					channel, country_data.chan_ranges_bw[params->bw],
					&country_comb_sets[CLM_BW_80],
					&country_data.sub_chan_channel_rules_80,
					country_data.sub_chan_increments_80,
					CLM_DATA_SUB_CHAN_MAX_80, ds->registry_flags);
		} else if (params->bw == CLM_BW_80_80) {
			fill_actual_subchan_table(bw_to_chan, &power_inc, subchannels, limits_type,
					channel, country_data.chan_ranges_bw[CLM_BW_80],
					&country_comb_sets[CLM_BW_80],
					&country_data.sub_chan_channel_rules_80,
					country_data.sub_chan_increments_80,
					CLM_DATA_SUB_CHAN_MAX_80, ds->registry_flags);
			bw_to_chan[CLM_BW_80_80] = bw_to_chan[CLM_BW_80];
			bw_to_chan[CLM_BW_80] = 0;
		} else if (params->bw == CLM_BW_160) {
			fill_actual_subchan_table(bw_to_chan, &power_inc, subchannels, limits_type,
					channel, country_data.chan_ranges_bw[params->bw],
					&country_comb_sets[CLM_BW_160],
					&country_data.sub_chan_channel_rules_160,
					country_data.sub_chan_increments_160,
					CLM_DATA_SUB_CHAN_MAX_160, ds->registry_flags);
#endif /* WL11AC */
		}
		/* bw_to_chan computed */

		/* Loop over all transmission power subsequences */
		do {
			/* Number of records in subsequence */
			int num_rec;
			/* Bandwidth of records in subsequence */
			clm_bandwidth_t pg_bw;
			/* Channel combs for bandwidth used in subsequence.
			 * NULL for 80+80 chan
			 */
			const clm_channel_comb_set_t *comb_set_for_bw;
			/* Channel number to look for bandwidth used in this
			 * subsequence
			 */
			int channel_for_bw;
			/* Length of TX power records in current subsequence */
			int tx_rec_len;
			/* Index of TX power inside TX power record */
			int tx_power_idx;
			/* Base address for channel ranges for bandwidth used
			 * in this subsequence
			 */
			const clm_channel_range_t *ranges;
			/* Sequence of rate sets' definition for bw used in
			 * this subsequence
			 */
			const unsigned char *rate_sets;

			flags = *tx_rec++;
			flags2 = (flags & CLM_DATA_FLAG_FLAG2) ? *tx_rec++ : 0;
			pg_bw = (clm_bandwidth_t)bw_width_to_idx[flags & CLM_DATA_FLAG_WIDTH_MASK];
#ifdef WL11ULB
			if (flags2 & CLM_DATA_FLAG2_WIDTH_EXT) {
				pg_bw = (clm_bandwidth_t)get_bw_to_ulb((unsigned char)pg_bw);
			}
#endif /* WL11ULB */
			comb_set_for_bw = comb_sets[pg_bw];
			channel_for_bw = bw_to_chan[pg_bw];
			tx_rec_len = CLM_LOC_DSC_TX_REC_LEN +
					((flags & CLM_DATA_FLAG_PER_ANT_MASK) >>
					CLM_DATA_FLAG_PER_ANT_SHIFT);
			tx_power_idx = (tx_rec_len == CLM_LOC_DSC_TX_REC_LEN)
					? CLM_LOC_DSC_POWER_IDX
					: antenna_power_offsets[params->antenna_idx];
			ranges = loc_data->chan_ranges_bw[pg_bw];
			rate_sets = (flags2 & CLM_DATA_FLAG2_EXT_RATES)
					? loc_data->ext_rate_sets_bw[pg_bw]
					: loc_data->rate_sets_bw[pg_bw];
			base_rate = (flags2 & CLM_DATA_FLAG2_EXT_RATES) ? BASE_EXT_RATE : 0;
			/* Loop over all records in subsequence */
			for (num_rec = (int)*tx_rec++; num_rec--; tx_rec += tx_rec_len)
			{
				/* Channel range for current transmission power
				 * record
				 */
				const clm_channel_range_t *range = ranges +
						tx_rec[CLM_LOC_DSC_RANGE_IDX];
				/* Power targets for current transmission power
				 * record - original and incremented per
				 * subchannel rule
				 */
				char qdbm, qdbm_inc;

				/* Per-antenna record without a limit for given
				 * antenna index?
				 */
				if (tx_power_idx >= tx_rec_len) {
					/* At least check if chan is valid -
					 * return OK if it is
					 */
					if (!valid_channel && channel_for_bw &&
							channel_in_range(channel_for_bw, range,
							comb_set_for_bw,
							params->other_80_80_chan) &&
							((unsigned char)tx_rec[0] !=
							(unsigned char)CLM_DISABLED_POWER))
					{
						valid_channel = TRUE;
					}
					/* Skip the rest - no limit for given
					 * antenna index
					 */
					continue;
				}
				qdbm_inc = qdbm = (char)tx_rec[tx_power_idx];
				if ((unsigned char)qdbm != (unsigned char)CLM_DISABLED_POWER) {
					if ((flags & CLM_DATA_FLAG_MEAS_MASK) ==
							CLM_DATA_FLAG_MEAS_EIRP) {
						qdbm -= (char)ant_gain;
						qdbm_inc = qdbm;
					}
					qdbm_inc += power_inc;
					/* Apply SAR limit */
					qdbm = (char)((qdbm > params->sar) ? params->sar : qdbm);
					qdbm_inc = (char)((qdbm_inc > params->sar)
							? params->sar : qdbm_inc);
				}

				/* If this record related to channel for this
				 * bandwidth?
				 */
				if (channel_for_bw &&
						channel_in_range(channel_for_bw, range,
						comb_set_for_bw, params->other_80_80_chan))
				{
					/* Rate indices  for current records'
					 * rate set
					 */
					const unsigned char *rates = get_byte_string(rate_sets,
							tx_rec[CLM_LOC_DSC_RATE_IDX]);
					int num_rates = *rates++;
					/* Loop over this tx power record's
					 * rate indices
					 */
					while (num_rates--) {
						unsigned int rate_idx = *rates++ + base_rate;
						clm_power_t *pp = &limits->limit[rate_idx];
						/* Looking for minimum power */
						if ((!channel_dsss ||
								(RATE_TYPE(rate_idx) != RT_DSSS)) &&
								((*pp == (clm_power_t)
								(unsigned char)UNSPECIFIED_POWER) ||
								((*pp > qdbm_inc) &&
								(*pp != (clm_power_t)
								(unsigned char)
								CLM_DISABLED_POWER))))
						{
							*pp = qdbm_inc;
						}
					}
					if ((unsigned char)qdbm_inc !=
						(unsigned char)CLM_DISABLED_POWER) {
						valid_channel = TRUE;
					}
				}
				/* If this rule related to 20-in-something DSSS
				 * channel?
				 */
				if (channel_dsss && (pg_bw == CLM_BW_20) &&
						channel_in_range(channel_dsss, range,
						comb_sets[CLM_BW_20], 0))
				{
					/* Same as above */
					const unsigned char *rates = get_byte_string(rate_sets,
							tx_rec[CLM_LOC_DSC_RATE_IDX]);
					int num_rates = *rates++;
					while (num_rates--) {
						unsigned int rate_idx = *rates++ + base_rate;
						clm_power_t *pp = &limits->limit[rate_idx];
						if (RATE_TYPE(rate_idx) == RT_DSSS) {
							*pp = qdbm;
						}
					}
				}
			}
		} while (flags & CLM_DATA_FLAG_MORE);
	}
	if (valid_channel) {
		/* Converting CLM_DISABLED_POWER and UNSPECIFIED_POWER to
		 * WL_RATE_DISABLED
		 */
		clm_power_t *pp = limits->limit, *end = pp + ARRAYSIZE(limits->limit);
		do {
			if ((*pp == (clm_power_t)(unsigned char)CLM_DISABLED_POWER) ||
					(*pp == (clm_power_t)(unsigned char)UNSPECIFIED_POWER))
			{
				*pp = WL_RATE_DISABLED;
			}
		} while (++pp < end);
	}
	return valid_channel ? CLM_RESULT_OK : CLM_RESULT_NOT_FOUND;
}


/** Retrieves information about channels with valid power limits for locales of
 * some region
 * \param[in] locales Country (region) locales' information
 * \param[out] valid_channels Valid 5GHz channels
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if `locales` is
 * null or contains invalid information, CLM_RESULT_NOT_FOUND if channel has
 * invalid value
 */
extern clm_result_t
clm_valid_channels_5g(const clm_country_locales_t *locales,
	clm_channels_t *channels20, clm_channels_t *channels4080)
{
	/* Locale characteristics for base and HT locales of given band */
	locale_data_t base_ht_loc_data[BH_NUM];
	/* Loop variable */
	int base_ht_idx;

	/* Check pointers' validity */
	if (!locales || !channels20 || !channels4080) {
		return CLM_RESULT_ERR;
	}
	/* Clearing output parameters */
	my_memset(channels20, 0, sizeof(*channels20));
	my_memset(channels4080, 0, sizeof(*channels4080));

	/* Computing helper information on locales */
	if (!fill_base_ht_loc_data(locales, CLM_BAND_5G, base_ht_loc_data, NULL)) {
		return CLM_RESULT_ERR;
	}

	/* For base then HT locales do */
	for (base_ht_idx = 0; base_ht_idx < (int)ARRAYSIZE(base_ht_loc_data); ++base_ht_idx) {
		/* Precomputed helper data for current locale */
		const locale_data_t *loc_data = base_ht_loc_data + base_ht_idx;

		/* Channel combs for given band - vector indexed by channel
		 * bandwidth
		 */
		const clm_channel_comb_set_t *const* comb_sets = loc_data->combs;

		/* Transmission power records' sequence for current locale */
		const unsigned char *tx_rec = loc_data->def_ptr;

		/* CLM_DATA_FLAG_ flags for current subsequence of transmission
		 * power records' sequence
		 */
		unsigned char flags;
#ifdef WL11ULB
		unsigned char flags2;
#endif /* WL11ULB */

		/* No transmission power records - nothing to do for this
		 * locale
		 */
		if (!tx_rec) {
			continue;
		}

		/* Loop over all transmission power subsequences */
		do {
			/* Number of records in subsequence */
			int num_rec;
			/* Bandwidth of records in subsequence */
			clm_bandwidth_t pg_bw;
			/* Channel combs for bandwidth used in subsequence */
			const clm_channel_comb_set_t *comb_set_for_bw;
			/* Length of TX power records in current subsequence */
			int tx_rec_len;
			/* Vector of channel ranges' definition */
			const clm_channel_range_t *ranges;
			clm_channels_t *channels;

			flags = *tx_rec++;
#ifdef WL11ULB
			flags2 = (flags & CLM_DATA_FLAG_FLAG2) ? *tx_rec++ : 0;
#else /* WL11ULB */
			if (flags & CLM_DATA_FLAG_FLAG2) {
				++tx_rec;
			}
#endif /* WL11ULB */
			pg_bw = (clm_bandwidth_t)bw_width_to_idx[flags & CLM_DATA_FLAG_WIDTH_MASK];
#ifdef WL11ULB
			if (flags2 & CLM_DATA_FLAG2_WIDTH_EXT) {
				pg_bw = (clm_bandwidth_t)get_bw_to_ulb((unsigned char)pg_bw);
			}
#endif /* WL11ULB */
			num_rec = (int)*tx_rec++;
			tx_rec_len = CLM_LOC_DSC_TX_REC_LEN +
					((flags & CLM_DATA_FLAG_PER_ANT_MASK) >>
					CLM_DATA_FLAG_PER_ANT_SHIFT);
#ifdef WL11AC
			if ((pg_bw == CLM_BW_80_80) || is_ulb_bw(pg_bw)) {
				tx_rec += num_rec * tx_rec_len;
				continue;
			}
#endif /* WL11AC */
			if (pg_bw == CLM_BW_20)
				channels = channels20;
			else
				channels = channels4080;

			ranges = loc_data->chan_ranges_bw[pg_bw];

			/* Loop over all records in subsequence */

			comb_set_for_bw = comb_sets[pg_bw];

			for (; num_rec--; tx_rec += tx_rec_len)
			{

				/* Channel range for current transmission power
				 * record
				 */
				const clm_channel_range_t *range = ranges +
						tx_rec[CLM_LOC_DSC_RANGE_IDX];

				int num_combs;
				const clm_channel_comb_set_t *combs = loc_data->combs[pg_bw];
				const clm_channel_comb_t *comb = combs->set;

				/* Check for a non-disabled power before
				 * clearing NO_bw flag
				 */
				if ((unsigned char)CLM_DISABLED_POWER ==
						(unsigned char)tx_rec[CLM_LOC_DSC_POWER_IDX]) {
					continue;
				}

				for (num_combs = comb_set_for_bw->num; num_combs--; ++comb) {
					int chan;
					for (chan = comb->first_channel; chan <= comb->last_channel;
						chan += comb->stride) {
						if (chan && channel_in_range(chan, range,
								comb_set_for_bw, 0)) {
							channels->bitvec[chan / 8] |=
									(unsigned char)
									(1 << (chan % 8));
						}
					}
				}
			}
		} while (flags & CLM_DATA_FLAG_MORE);
	}
	return CLM_RESULT_OK;
}

clm_result_t
clm_channels_params_init(clm_channels_params_t *params)
{
	if (!params) {
		return CLM_RESULT_ERR;
	}
	params->bw = CLM_BW_20;
	params->this_80_80 = 0;
	return CLM_RESULT_OK;
}

clm_result_t
clm_valid_channels(const clm_country_locales_t *locales, clm_band_t band,
	const clm_channels_params_t *params, clm_channels_t *channels)
{
	/* Locale characteristics for base and HT locales of given band */
	locale_data_t base_ht_loc_data[BH_NUM];
	/* Index for looping over base and HT locales */
	int base_ht_idx;
	/* Return value */
	clm_result_t ret = CLM_RESULT_NOT_FOUND;

	/* Check parameters' validity */
	if (!locales || !channels || !params ||
			((unsigned int)band >= (unsigned int)CLM_BAND_NUM) ||
#ifdef WL11AC
			((unsigned)params->bw >= (unsigned int)CLM_BW_NUM) ||
			((params->this_80_80 != 0) != (params->bw == CLM_BW_80_80)))
#else
			((unsigned)params->bw >= (unsigned int)CLM_BW_NUM))
#endif /* WL11AC */
	{
		return CLM_RESULT_ERR;
	}
	/* Clearing output parameters */
	my_memset(channels, 0, sizeof(*channels));

	/* Computing helper information on locales */
	if (!fill_base_ht_loc_data(locales, band, base_ht_loc_data, NULL)) {
		return CLM_RESULT_ERR;
	}

	/* For base then HT locales do */
	for (base_ht_idx = 0; base_ht_idx < (int)ARRAYSIZE(base_ht_loc_data); ++base_ht_idx) {
		/* Precomputed helper data for current locale */
		const locale_data_t *loc_data = base_ht_loc_data + base_ht_idx;

		/* Channel combs for given band - vector indexed by channel
		 * bandwidth
		 */
		const clm_channel_comb_set_t *const* comb_sets = loc_data->combs;

		/* Transmission power records' sequence for current locale */
		const unsigned char *tx_rec = loc_data->def_ptr;

		/* CLM_DATA_FLAG_ flags for current subsequence of transmission
		 * power records' sequence
		 */
		unsigned char flags;
#ifdef WL11ULB
		unsigned char flags2;
#endif /* WL11ULB */

		/* No transmission power records - nothing to do for this
		 * locale
		 */
		if (!tx_rec) {
			continue;
		}

		/* Loop over all transmission power subsequences */
		do {
			/* Number of records in subsequence */
			int num_rec;
			/* Bandwidth of records in subsequence */
			clm_bandwidth_t pg_bw;
			/* Channel combs for bandwidth used in subsequence */
			const clm_channel_comb_set_t *comb_set_for_bw;
			/* Length of TX power records in current subsequence */
			int tx_rec_len;
			/* Vector of channel ranges' definition */
			const clm_channel_range_t *ranges;

			flags = *tx_rec++;
#ifdef WL11ULB
			flags2 = (flags & CLM_DATA_FLAG_FLAG2) ? *tx_rec++ : 0;
#else /* WL11ULB */
			if (flags & CLM_DATA_FLAG_FLAG2) {
				++tx_rec;
			}
#endif /* WL11ULB */
			pg_bw = (clm_bandwidth_t)bw_width_to_idx[flags & CLM_DATA_FLAG_WIDTH_MASK];
#ifdef WL11ULB
			if (flags2 & CLM_DATA_FLAG2_WIDTH_EXT) {
				pg_bw = (clm_bandwidth_t)get_bw_to_ulb((unsigned char)pg_bw);
			}
#endif /* WL11ULB */
			num_rec = (int)*tx_rec++;
			tx_rec_len = CLM_LOC_DSC_TX_REC_LEN +
					((flags & CLM_DATA_FLAG_PER_ANT_MASK) >>
					CLM_DATA_FLAG_PER_ANT_SHIFT);
			/* If not bandwidth we are looking for - skip to next
			 * group of TX records
			 */
			if (pg_bw != params->bw) {
				tx_rec += num_rec * tx_rec_len;
				continue;
			}

			ranges = loc_data->chan_ranges_bw[pg_bw];
			comb_set_for_bw = comb_sets[pg_bw];
			/* Loop over all records in subsequence */
			for (; num_rec--; tx_rec += tx_rec_len)
			{
				/* Channel range for current transmission power
				 * record
				 */
				const clm_channel_range_t *range = ranges +
						tx_rec[CLM_LOC_DSC_RANGE_IDX];

				int num_combs;
				const clm_channel_comb_set_t *combs = loc_data->combs[pg_bw];
				const clm_channel_comb_t *comb = combs->set;

				/* Skip disabled power record */
				if ((unsigned char)CLM_DISABLED_POWER ==
						(unsigned char)tx_rec[CLM_LOC_DSC_POWER_IDX])
				{
					continue;
				}
				/* 80+80 ranges are special - they are channel
				 * pairs
				 */
				if (params->this_80_80) {
					if (range->start == params->this_80_80) {
						channels->bitvec[range->end / 8] |=
								(unsigned char)
								(1 << (range->end % 8));
						ret = CLM_RESULT_OK;
					}
					continue;
				}
				/* Normal enabled range - loop over all
				 * channels in it
				 */
				for (num_combs = comb_set_for_bw->num; num_combs--; ++comb) {
					unsigned char chan;
					if ((range->end < comb->first_channel) ||
							(range->start > comb->last_channel))
					{
						continue;
					}
					for (chan = comb->first_channel;
							chan <= comb->last_channel;
							chan += comb->stride)
					{
						if (channel_in_range(chan, range,
								comb_set_for_bw, 0)) {
							channels->bitvec[chan / 8] |=
									(unsigned char)
									(1 << (chan % 8));
							ret = CLM_RESULT_OK;
						}
					}
				}
			}
		} while (flags & CLM_DATA_FLAG_MORE);
	}
	return ret;
}
#endif /* !WLC_CLMAPI_PRE7 || BCMROMBUILD */

clm_result_t
clm_regulatory_limit(const clm_country_locales_t *locales, clm_band_t band,
	unsigned int channel, int *limit)
{
	int num_rec;
	locale_data_t loc_data;
	const unsigned char *pub_rec;
	const clm_channel_range_t *ranges;
	const clm_channel_comb_set_t *comb_set;
	if (!locales || ((unsigned)band >= (unsigned)CLM_BAND_NUM) || !limit) {
		return CLM_RESULT_ERR;
	}
	if (!get_loc_def(locales, (band == CLM_BAND_2G)
			? CLM_LOC_IDX_BASE_2G : CLM_LOC_IDX_BASE_5G, &loc_data))
	{
		return CLM_RESULT_ERR;
	}
	if ((pub_rec = loc_data.def_ptr) == NULL) {
		return CLM_RESULT_ERR;
	}
	pub_rec += CLM_LOC_DSC_BASE_HDR_LEN;
	ranges = loc_data.chan_ranges_bw[CLM_BW_20];
	comb_set = loc_data.combs[CLM_BW_20];
	for (num_rec = *pub_rec++; num_rec--; pub_rec += CLM_LOC_DSC_PUB_REC_LEN) {
		if (channel_in_range(channel, ranges + pub_rec[CLM_LOC_DSC_RANGE_IDX],
				comb_set, 0)) {
			*limit = (int)pub_rec[CLM_LOC_DSC_POWER_IDX];
			return CLM_RESULT_OK;
		}
	}
	return CLM_RESULT_NOT_FOUND;
}

clm_result_t
clm_agg_country_iter(clm_agg_country_t *agg, ccode_t cc, unsigned int *rev)
{
	data_source_id_t ds_id;
	int idx;
	clm_result_t ret = CLM_RESULT_OK;
	if (!agg || !cc || !rev) {
		return CLM_RESULT_ERR;
	}
	if (*agg == CLM_ITER_NULL) {
		ds_id = DS_INC;
		idx = 0;
	} else {
		iter_unpack(*agg, &ds_id, &idx);
		++idx;
	}
	for (;;) {
		int num_aggregates;
		aggregate_data_t aggregate_data, inc_aggregate_data;
		if (!get_aggregate_by_idx(ds_id, idx, &aggregate_data, &num_aggregates)) {
			if (ds_id == DS_INC) {
				ds_id = DS_MAIN;
				idx = 0;
				continue;
			} else {
				ret = CLM_RESULT_NOT_FOUND;
				idx = (num_aggregates >= 0) ? num_aggregates : 0;
				break;
			}
		}
		if (aggregate_data.num_regions == CLM_DELETED_NUM) {
			++idx;
			continue;
		}
		if ((ds_id == DS_MAIN) && get_aggregate(DS_INC,
			&aggregate_data.def_reg, NULL, &inc_aggregate_data)) {
			++idx;
			continue;
		}
		copy_cc(cc, aggregate_data.def_reg.cc);
		*rev = aggregate_data.def_reg.rev;
		break;
	}
	iter_pack(agg, ds_id, idx);
	return ret;
}

clm_result_t
clm_agg_map_iter(const clm_agg_country_t agg, clm_agg_map_t *map, ccode_t cc,
	unsigned int *rev)
{
	data_source_id_t ds_id, mapping_ds_id;
	int agg_idx, mapping_idx;
	aggregate_data_t aggregate_data;
	clm_cc_rev4_t mapping = {"", 0};

	if (!map || !cc || !rev) {
		return CLM_RESULT_ERR;
	}
	iter_unpack(agg, &ds_id, &agg_idx);
	get_aggregate_by_idx(ds_id, agg_idx, &aggregate_data, NULL);
	if (*map == CLM_ITER_NULL) {
		mapping_idx = 0;
		mapping_ds_id = ds_id;
	} else {
		iter_unpack(*map, &mapping_ds_id, &mapping_idx);
		++mapping_idx;
	}
	for (;;) {
		aggregate_data_t cur_agg_data;
		MY_BOOL found = TRUE;
		if (mapping_ds_id == ds_id) {
			cur_agg_data = aggregate_data;
		} else {
			found = get_aggregate(mapping_ds_id, &aggregate_data.def_reg, NULL,
					&cur_agg_data);
		}
		if (found) {
			if (mapping_idx < cur_agg_data.num_regions) {
				get_ccrev(mapping_ds_id, &mapping, cur_agg_data.regions,
						mapping_idx);
			} else {
				found = FALSE;
			}
		}
		if (!found) {
			if (mapping_ds_id == DS_MAIN) {
				iter_pack(map, mapping_ds_id, mapping_idx);
				return CLM_RESULT_NOT_FOUND;
			}
			mapping_ds_id = DS_MAIN;
			mapping_idx = 0;
			continue;
		}
		if (mapping.rev == CLM_DELETED_MAPPING) {
			++mapping_idx;
			continue;
		}
		if ((ds_id == DS_INC) && (mapping_ds_id == DS_MAIN) &&
				get_mapping(DS_INC, &aggregate_data, mapping.cc, NULL)) {
			++mapping_idx;
			continue;
		}
		copy_cc(cc, mapping.cc);
		*rev = mapping.rev;
		break;
	}
	iter_pack(map, mapping_ds_id, mapping_idx);
	return CLM_RESULT_OK;
}

clm_result_t
clm_agg_country_lookup(const ccode_t cc, unsigned int rev,
	clm_agg_country_t *agg)
{
	int ds_idx;
	clm_cc_rev4_t cc_rev;
	if (!cc || !agg) {
		return CLM_RESULT_ERR;
	}
	translate_cc(&cc);
	copy_cc(cc_rev.cc, cc);
	cc_rev.rev = (regrev_t)rev;
	for (ds_idx = 0; ds_idx < DS_NUM; ds_idx++) {
		int agg_idx;
		aggregate_data_t agg_data;
		if (get_aggregate((data_source_id_t)ds_idx, &cc_rev, &agg_idx, &agg_data)) {
			if (agg_data.num_regions == CLM_DELETED_NUM) {
				return CLM_RESULT_NOT_FOUND;
			}
			iter_pack(agg, (data_source_id_t)ds_idx, agg_idx);
			return CLM_RESULT_OK;
		}
	}
	return CLM_RESULT_NOT_FOUND;
}

clm_result_t
clm_agg_country_map_lookup(const clm_agg_country_t agg,
	const ccode_t target_cc, unsigned int *rev)
{
	data_source_id_t ds_id;
	int aggregate_idx;
	aggregate_data_t aggregate_data;
	clm_cc_rev4_t mapping;

	if (!target_cc || !rev) {
		return CLM_RESULT_ERR;
	}
	translate_cc(&target_cc);
	iter_unpack(agg, &ds_id, &aggregate_idx);
	get_aggregate_by_idx(ds_id, aggregate_idx, &aggregate_data, NULL);
	for (;;) {
		if (get_mapping(ds_id, &aggregate_data, target_cc, &mapping)) {
			if (mapping.rev == CLM_DELETED_MAPPING) {
				return CLM_RESULT_NOT_FOUND;
			}
			*rev = mapping.rev;
			return CLM_RESULT_OK;
		}
		if (ds_id == DS_MAIN) {
			return CLM_RESULT_NOT_FOUND;
		}
		ds_id = DS_MAIN;
		if (!get_aggregate(DS_MAIN, &aggregate_data.def_reg, NULL, &aggregate_data)) {
			return CLM_RESULT_NOT_FOUND;
		}
	}
}

static const char*
clm_get_app_version_string(data_source_id_t ds_id)
{
	const char* strptr = NULL;
	data_dsc_t *ds = get_data_sources(ds_id);
	const clm_registry_t *data = ds->data;
	const int flags = data ? data->flags : 0;

	if (flags & CLM_REGISTRY_FLAG_APPS_VERSION) {
		const clm_data_header_t *header = ds->header;

		if (my_strcmp(header->apps_version, CLM_APPS_VERSION_NONE_TAG)) {
			strptr = header->apps_version;
		}
	}
	return strptr;
}


const char*
clm_get_base_app_version_string(void)
{
	return clm_get_app_version_string(DS_MAIN);
}


const char*
clm_get_inc_app_version_string(void)
{
	return clm_get_app_version_string(DS_INC);
}

const char*
clm_get_string(clm_string_type_t string_type,
	clm_string_source_t string_source)
{
	data_source_id_t ds_id =
			((string_source == CLM_STRING_SOURCE_BASE) ||
			((string_source == CLM_STRING_SOURCE_EFFECTIVE) &&
			!get_data_sources(DS_INC)->data))
			? DS_MAIN : DS_INC;
	const data_dsc_t *ds = get_data_sources(ds_id);
	const clm_data_header_t *header = ds->header;
	const char* ret = NULL;
	const char* def_value = "";
	/* NULL if data source is invalid or absent */
	if (((unsigned)string_source >= (unsigned)CLM_STRING_SOURCE_NUM) || !ds->data) {
		return NULL;
	}
	switch (string_type) {
	case CLM_STRING_TYPE_DATA_VERSION:
		ret = header->clm_version;
		break;
	case CLM_STRING_TYPE_COMPILER_VERSION:
		ret = header->compiler_version;
		break;
	case CLM_STRING_TYPE_GENERATOR_VERSION:
		ret = header->generator_version;
		break;
	case CLM_STRING_TYPE_APPS_VERSION:
		if (ds->data->flags & CLM_REGISTRY_FLAG_APPS_VERSION) {
			ret = header->apps_version;
			def_value = CLM_APPS_VERSION_NONE_TAG;
		}
		break;
	case CLM_STRING_TYPE_USER_STRING:
		if (ds->data->flags & CLM_REGISTRY_FLAG_USER_STRING) {
			ret = (const char*)relocate_ptr(ds_id, ds->data->user_string);
		}
		break;
	default:
		return NULL;
	}
	/* If string equals default value - will return NULL */
	if (ret && !my_strcmp(ret, def_value)) {
		ret = NULL;
	}
	return ret;
}
