/*
 * API for accessing CLM data
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
 * $Id: wlc_clm.h 520060 2014-12-10 03:23:44Z $
 */

#ifndef _WLC_CLM_H_
#define _WLC_CLM_H_

#include <bcmwifi_rates.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************
* API CONSTANTS	*
*****************
*/

/** Module constants */
enum clm_const {
	/** Initial ('before begin') value for iterator. It is guaranteed that
	 * iterator 'pointing' at some valid object it is not equal to this
	 * value
	 */
	CLM_ITER_NULL = 0
};

/** Frequency band identifiers */
typedef enum clm_band {
	/** 2.4HGz band */
	CLM_BAND_2G,

	/** 5GHz band */
	CLM_BAND_5G,

	/** Number of band identifiers */
	CLM_BAND_NUM
} clm_band_t;

/** Channel bandwidth identifiers */
typedef enum clm_bandwidth {
	/** 20MHz channel */
	CLM_BW_20,

	/** 40MHz channel */
	CLM_BW_40,
#ifdef WL11AC
	/** 80MHz channel */
	CLM_BW_80,

	/** 160MHz channel */
	CLM_BW_160,

	/** 80+80MHz channel */
	CLM_BW_80_80,
#endif
#if defined(WL11ULB) || defined(WL11ULB_IN_ROM)
	/** 2.5MHz channel */
	CLM_BW_2_5,

	/** 5MHz channel */
	CLM_BW_5,

	/** 10MHz channel */
	CLM_BW_10,
#endif
	/** Number of channel bandwidth identifiers */
	CLM_BW_NUM
} clm_bandwidth_t;

/** Return codes for API functions */
typedef enum clm_result {
	/** No error */
	CLM_RESULT_OK		 = 0,

	/** Invalid parameters */
	CLM_RESULT_ERR		 = 1,

	/** Lookup failed (something was not found) */
	CLM_RESULT_NOT_FOUND = -1
} clm_result_t;

#if defined(WLC_CLMAPI_PRE7) && !defined(BCM4334A0SIM_4334B0) && !defined(BCMROMBUILD)
/** Which 20MHz channel to use as extension in 40MHz operation */
typedef enum clm_ext_chan {
	/** Lower channel is extension, upper is control */
	CLM_EXT_CHAN_LOWER = -1,

	/** Upper channel is extension, lower is control */
	CLM_EXT_CHAN_UPPER =  1,

	/** Neither of the above (use for 20MHz operation) */
	CLM_EXT_CHAN_NONE  =  0
} clm_ext_chan_t;
#endif

/** Flags */
typedef enum clm_flags {
	/* DFS-RELATED COUNTRY (REGION) FLAGS */
	/** Common DFS rules */
	CLM_FLAG_DFS_NONE       = 0x00000000,

	/** EU DFS rules */
	CLM_FLAG_DFS_EU         = 0x00000001,

	/** US (FCC) DFS rules */
	CLM_FLAG_DFS_US         = 0x00000002,

	/** TW DFS rules */
	CLM_FLAG_DFS_TW         = 0x00000003,

	/** Mask of DFS-related flags */
	CLM_FLAG_DFS_MASK       = 0x00000003,

	/** FiltWAR1 flag from CLM XML */
	CLM_FLAG_FILTWAR1       = 0x00000004,

	/** Beamforming allowed */
	CLM_FLAG_TXBF           = 0x00000008,

	/** Region has per-antenna power targets */
	CLM_FLAG_PER_ANTENNA    = 0x00000010,

	/** Region is default for its CC */
	CLM_FLAG_DEFAULT_FOR_CC = 0x00000020,

	/** Region is EDCRS-EU-compliant */
	CLM_FLAG_EDCRS_EU       = 0x00000040,

	/* DEBUGGING FLAGS (ALLOCATED AS TOP BITS) */

	/** No 80MHz channels */
	CLM_FLAG_NO_80MHZ       = 0x80000000,

	/** No 40MHz channels */
	CLM_FLAG_NO_40MHZ       = 0x40000000,

	/** No 160MHz channels */
	CLM_FLAG_NO_160MHZ      = 0x04000000,

	/** No 80+80MHz channels */
	CLM_FLAG_NO_80_80MHZ    = 0x02000000,

	/** No MCS rates */
	CLM_FLAG_NO_MIMO        = 0x20000000,

	/** Has DSSS rates that use EIRP limits */
	CLM_FLAG_HAS_DSSS_EIRP  = 0x10000000,

	/* HAS OFDM RATES THAT USE EIRP LIMITS */
	CLM_FLAG_HAS_OFDM_EIRP  = 0x08000000
} clm_flags_t;

/** Type of limits to output in clm_limits() */
typedef enum clm_limits_type {
	/** Limit for main channel */
	CLM_LIMITS_TYPE_CHANNEL,

	/** Limit for L-subchannel (20-in-40, 40-in-80) */
	CLM_LIMITS_TYPE_SUBCHAN_L,

	/** Limit for U-subchannel (20-in-40, 40-in-80) */
	CLM_LIMITS_TYPE_SUBCHAN_U,
#ifdef WL11AC
	/** Limit for LL-subchannel (20-in-80, 40-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_LL,

	/** Limit for LU-subchannel (20-in-80, 40-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_LU,

	/** Limit for UL-subchannel (20-in-80, 40-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_UL,

	/** Limit for UU-subchannel (20-in-80, 40-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_UU,

	/** Limit for LLL-subchannel (20-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_LLL,

	/** Limit for LLU-subchannel (20-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_LLU,

	/** Limit for LUL-subchannel (20-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_LUL,

	/** Limit for LUU-subchannel (20-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_LUU,

	/** Limit for ULL-subchannel (20-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_ULL,

	/** Limit for ULU-subchannel (20-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_ULU,

	/** Limit for UUL-subchannel (20-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_UUL,

	/** Limit for UUU-subchannel (20-in-160) */
	CLM_LIMITS_TYPE_SUBCHAN_UUU,
#endif /* WL11AC */
	CLM_LIMITS_TYPE_NUM
} clm_limits_type_t;

/** Strings stored in CLM data */
typedef enum clm_string_type {
	/** CLM data (spreadsheet) version */
	CLM_STRING_TYPE_DATA_VERSION,

	/** ClmCompiler version */
	CLM_STRING_TYPE_COMPILER_VERSION,

	/** Name and version of program that generated XML */
	CLM_STRING_TYPE_GENERATOR_VERSION,

	/** Engineering version of CLM data */
	CLM_STRING_TYPE_APPS_VERSION,

	/** User string */
	CLM_STRING_TYPE_USER_STRING,

	/** Number of enum members */
	CLM_STRING_TYPE_NUM
} clm_string_type_t;

/** Source (location) of string stored in CLM data */
typedef enum clm_string_source {
	/** String stored in base data */
	CLM_STRING_SOURCE_BASE,

	/** String stored in incremental data */
	CLM_STRING_SOURCE_INCREMENTAL,

	/** If incremental data present - string stored in it otherwise in base
	 * data
	 */
	CLM_STRING_SOURCE_EFFECTIVE,

	/** Number of enum members */
	CLM_STRING_SOURCE_NUM
} clm_string_source_t;


/*****************
* API DATA TYPES *
******************
*/

/** Country Code: a two byte code, usually ASCII
 * Note that 'worldwide' country code is now "ww", not "\0\0"
 */
typedef char ccode_t[2];

/** Channel set */
typedef struct clm_channels {
	unsigned char bitvec[25]; /* Bit vector, indexed by channel numbers */
} clm_channels_t;

/** Power in quarter of dBm units */
typedef signed char clm_power_t;

/** Per-TX-rate power limits */
typedef struct clm_power_limits {
	/** Per-rate limits (WL_RATE_DISABLED for disabled rates) */
	clm_power_t limit[WL_NUMRATES];
} clm_power_limits_t;


/* ITERATORS - TOKENS THAT REPRESENT VARIOUS ITEMS IN THE CLM */

/** Country (region) definition */
typedef int clm_country_t;

/** Locale definition */
typedef int clm_locale_t;

/** Aggregate definition */
typedef int clm_agg_country_t;

/** Definition of mapping inside aggregation */
typedef int clm_agg_map_t;


/** Locales (transmission rules) for a country (region) */
typedef struct clm_country_locales {
	/** 2.4GHz base locale (802.11b/g SISO) */
	clm_locale_t locale_2G;

	/** 5GHz base locale (802.11a SISO) */
	clm_locale_t locale_5G;

	/** 2.4GHz HT locale (802.11n and 802.11b/g MIMO) */
	clm_locale_t locale_2G_HT;

	/** 5GHz HT locale (802.11n and 802.11a MIMO) */
	clm_locale_t locale_5G_HT;

	/** Flags from country record */
	unsigned char country_flags;

	/** Computed country flags */
	unsigned char computed_flags;

	/** Second byte of flags from country record */
	unsigned char country_flags_2;
} clm_country_locales_t;

/** Parameters that refine clm_limits() output data
 * To use this structure one shall use clm_limits_params_init() to reset
 * parameters from this structure to default state, then change only parameters
 * relevant to task, leaving all others in default states
 */
typedef struct clm_limits_params {
	/** Channel bandwidth. Default is CLM_BW_20 (20MHz) */
	clm_bandwidth_t bw;

	/** SAR limit in quarter dBm. Default is 0x7F (no SAR limit) */
	int sar;

	/** 0-based antenna index (0 .. WL_TX_CHAINS_MAX-1). This parameter
	 * only affects rates for which per-antenna power limits specified.
	 * If 2 limits specified - they'll be returned for antenna indices 0
	 * and 1, WL_RATE_DISABLED will be returned for antenna index 3. If 3
	 * limits specified - they'll be returned for corresponded antenna
	 * indices. Default is 0
	 */
	int antenna_idx;

	/** For 80+80 channel it is other channel in pair (not one for which
	 * power limit is requested
	 */
	unsigned int other_80_80_chan;
} clm_limits_params_t;

/** Input parameters for clm_valid_channels() */
typedef struct clm_channels_params {
	/** Bandwidth of channels to look for. Default is CLM_BW_20 */
	clm_bandwidth_t bw;

	/** If nonzero - one channel in 80+80 channel pair, in this case
	 * function returns other 80MHz channels that may be used in pair with
	 * it
	 */
	unsigned int this_80_80;
} clm_channels_params_t;

/* forward declaration for CLM header data structure used in clm_init()
 * struct clm_data_header is defined in clm_data.h
 */
struct clm_data_header;


/***************
* API ROUTINES *
****************
*/

/** Provides main CLM data to the CLM access API
 * Must be called before any access APIs are used
 * \param[in] data Header of main CLM data. Only one main CLM data source may
 * be set
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if given address is
 * nonzero and CLM data tag is absent at given address or major number of CLM
 * data format version is not supported by CLM API
 */
extern clm_result_t
clm_init(const struct clm_data_header *data);

/** Provides incremental CLM data to the CLM access API
 * This call is optional
 * \param[in] data Header of incremental CLM data. No more than one incremental
 * CLM data source may be set
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if given address is
 * non zero and CLM data tag is absent at given address or major number of CLM
 * data format version is not supported by CLM API
 */
extern clm_result_t
clm_set_inc_data(const struct clm_data_header *data);

/** Initializes iterator before iteration via of clm_..._iter()
 * May be done manually - by assigning CLM_ITER_NULL
 * \param[out] iter Iterator to be initialized (type is clm_country_t,
 * clm_locale_t, clm_agg_country_t, clm_agg_map_t)
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if null pointer
 * passed
 */
extern clm_result_t
clm_iter_init(int *iter);

/** Resets given clm_limits() parameters structure to defaults
 * \param[out] params Address of parameters' structure to reset
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if null pointer
 * passed
 */
extern clm_result_t
clm_limits_params_init(struct clm_limits_params *params);

/** Performs one iteration step over set of countries (regions)
 * Looks up first/next country (region)
 * \param[in,out] country Iterator token. Shall be initialized with
 * clm_iter_init() before iteration begin. After successful call iterator token
 * 'points' to same region as returned cc/rev
 * \param[out] cc Country (region) code
 * \param[out] rev Country (region) revision
 * \return CLM_RESULT_OK if first/next country was found, CLM_RESULT_ERR if any
 * of passed pointers was null, CLM_RESULT_NOT_FOUND if first/next country was
 * not found (iteration completed)
 */
extern clm_result_t
clm_country_iter(clm_country_t *country, ccode_t cc, unsigned int *rev);


/** Looks up for country (region) with given country code and revision
 * \param[in] cc Country code to look for
 * \param[in] rev Country (region) revision to look for
 * \param[out] country Iterator that 'points' to found country
 * \return CLM_RESULT_OK if country was found, CLM_RESULT_ERR if any of passed
 * pointers was null, CLM_RESULT_NOT_FOUND if required country was not found
 */
extern clm_result_t
clm_country_lookup(const ccode_t cc, unsigned int rev, clm_country_t *country);

/** Retrieves locale definitions of given country (regions)
 * \param[in] country Iterator that 'points' to country (region) information
 * \param[out] locales Locales' information for given country
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if `locales`
 * pointer is null, CLM_RESULT_NOT_FOUND if given country iterator does not
 * point to valid country (region) definition
 */
extern clm_result_t
clm_country_def(const clm_country_t country, clm_country_locales_t *locales);


/** Retrieves information about valid and restricted channels for locales of
 * some region
 * \param[in] locales Country (region) locales' information
 * \param[in] band Frequency band
 * \param[out] valid_channels Valid 20MHz channels (optional parameter)
 * \param[out] restricted_channels Restricted channels (optional parameter)
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if `locales`
 * pointer is null, or band ID is invalid or `locales` contents is invalid
 */
extern clm_result_t
clm_country_channels(const clm_country_locales_t *locales, clm_band_t band,
	clm_channels_t *valid_channels, clm_channels_t *restricted_channels);


/** Retrieves flags associated with given country (region) for given band
 * \param[in] locales Country (region) locales' information
 * \param[in] band Frequency band
 * \param[out] flags Flags associated with given country (region) for given
 * band
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if `locales` or
 * `flags` pointer is null, or band ID is invalid or `locales` contents is
 * invalid
 */
extern clm_result_t
clm_country_flags(const clm_country_locales_t *locales, clm_band_t band,
	unsigned long *flags);


/** Retrieves advertised country code for country (region) pointed by given
 * iterator
 * \param[in] country Iterator that points to country (region) in question
 * \param[out] advertised_cc Advertised CC for given region
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if `country` or
 * `advertised_cc` is null, or if `country` not `points` t a valid country
 * (region) definition
 */
extern clm_result_t
clm_country_advertised_cc(const clm_country_t country, ccode_t advertised_cc);

#if defined(WLC_CLMAPI_PRE7) && !defined(BCM4334A0SIM_4334B0) && !defined(BCMROMBUILD)
/* This version required for ROM compatibility */
extern clm_result_t
clm_limits(const clm_country_locales_t *locales, clm_band_t band,
	unsigned int channel, clm_bandwidth_t bw, int ant_gain, int sar,
	clm_ext_chan_t extension_channel, clm_power_limits_t *limits,
	clm_power_limits_t *bw20in40_limits);
#else
/** Retrieves the power limits on the given band/(sub)channel/bandwidth using
 * the given antenna gain
 * \param[in] locales Country (region) locales' information
 * \param[in] band Frequency band
 * \param[in] channel Channel number (main channel if subchannel limits output
 * is required)
 * \param[in] ant_gain Antenna gain in quarter dBm (used if limit is given in
 * EIRP terms)
 * \param[in] limits_type Subchannel to get limits for
 * \param[in] params Other parameters
 * \param[out] limits Limits for given above parameters
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if `locales` is
 * null or contains invalid information, or if any other input parameter
 * (except channel) has invalid value, CLM_RESULT_NOT_FOUND if channel has
 * invalid value
 */
extern clm_result_t
clm_limits(const clm_country_locales_t *locales, clm_band_t band,
	unsigned int channel, int ant_gain, clm_limits_type_t limits_type,
	const clm_limits_params_t *params, clm_power_limits_t *limits);

/** Retrieves information about channels with valid power limits for locales of
 * some region. This function is deprecated. It is being superseded by
 * clm_valid_channels()
 * \param[in] locales Country (region) locales' information
 * \param[out] valid_channels Valid 5GHz channels
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if `locales` is
 * null or contains invalid information, CLM_RESULT_NOT_FOUND if channel has
 * invalid value
 */
extern clm_result_t
clm_valid_channels_5g(const clm_country_locales_t *locales,
	clm_channels_t *channels20, clm_channels_t *channels4080);

/** Resets given clm_valid_channels() parameters structure to defaults
 * \param[out] params Address of parameters' structure to reset
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if null pointer
 * passed
 */
extern clm_result_t
clm_channels_params_init(clm_channels_params_t *params);

/** Retrieves information about certain channels with valid power limits for
 * locales of some region
 * \param[in] locales Country (region) locales' information
 * \param[in] band Band of channels being requested
 * \param[in] params Other parameters of channels being requested
 * \param[out] channels Country's (region's) channels that match given criteria
 * \return CLM_RESULT_OK if some channels were found, CLM_RESULT_NOT_FOUND if
 * no matching channels were found, CLM_RESULT_ERR if parameters has invalid
 * values
 */
extern clm_result_t
clm_valid_channels(const clm_country_locales_t *locales, clm_band_t band,
		const clm_channels_params_t *params, clm_channels_t *channels);
#endif /* WLC_CLMAPI_PRE7 && !BCM4334A0SIM_4334B0 && !BCMROMBUILD */


/** Retrieves maximum regulatory power for given channel
 * \param[in] locales Country (region) locales' information
 * \param[in] band Frequency band
 * \param[in] channel Channel number
 * \param[out] limit Regulatory power limit in dBm (!NOT qdBm!)
 * \return CLM_RESULT_OK in case of success, CLM_RESULT_ERR if `locales` is
 * null or contains invalid information, if `limit` is null or if any other
 * input parameter (except channel) has invalid value, CLM_RESULT_NOT_FOUND if
 * regulatory power limit not defined for given channel
 */
extern clm_result_t
clm_regulatory_limit(const clm_country_locales_t *locales, clm_band_t band,
	unsigned int channel, int *limit);

/** Performs one iteration step over set of aggregations. Looks up first/next
 * aggregation
 * \param[in,out] agg Iterator token. Shall be initialized with clm_iter_init()
 * before iteration begin. After successful call iterator token 'points' to
 * same aggregation as returned cc/rev
 * \param[out] cc Aggregation's default country (region) code
 * \param[out] rev Aggregation's default country (region) revision
 * \return CLM_RESULT_OK if first/next aggregation was found, CLM_RESULT_ERR if
 * any of passed pointers was null, CLM_RESULT_NOT_FOUND if first/next
 * aggregation was not found (iteration completed)
 */
extern clm_result_t
clm_agg_country_iter(clm_agg_country_t *agg, ccode_t cc, unsigned int *rev);

/** Performs one iteration step over sequence of aggregation's mappings
 * Looks up first/next mapping
 * \param[in] agg Aggregation whose mappings are being iterated
 * \param[in,out] map Iterator token. Shall be initialized with clm_iter_init()
 * before iteration begin. After successful call iterator token 'points' to
 * same mapping as returned cc/rev
 * \param[out] cc Mapping's country code
 * \param[out] rev Mapping's region revision
 * \return CLM_RESULT_OK if first/next mapping was found, CLM_RESULT_ERR if any
 * of passed pointers was null or if aggregation iterator does not 'point' to
 * valid aggregation, CLM_RESULT_NOT_FOUND if first/next mapping was not found
 * (iteration completed)
 */
extern clm_result_t
clm_agg_map_iter(const clm_agg_country_t agg, clm_agg_map_t *map, ccode_t cc,
	unsigned int *rev);

/** Looks up for aggregation with given default country code and revision
 * \param[in] cc Default country code of aggregation being looked for
 * \param[in] rev Default region revision of aggregation being looked for
 * \param[out] agg Iterator that 'points' to found aggregation
 * \return CLM_RESULT_OK if aggregation was found, CLM_RESULT_ERR if any of
 * passed pointers was null, CLM_RESULT_NOT_FOUND if required aggregation was
 * not found
 */
extern clm_result_t
clm_agg_country_lookup(const ccode_t cc, unsigned int rev,
	clm_agg_country_t *agg);

/** Looks up for mapping with given country code among mappings of given
* aggregation
 * \param[in] agg Aggregation whose mappings are being looked up
 * \param[in] target_cc Country code of mapping being looked up
 * \param[out] rev Country (region) revision of found mapping
 * \return CLM_RESULT_OK if mapping was found, CLM_RESULT_ERR if any of passed
 * pointers was null or aggregation iterator does not 'point' to valid
 * aggregation, CLM_RESULT_NOT_FOUND if required aggregation was not found
 */
extern clm_result_t
clm_agg_country_map_lookup(const clm_agg_country_t agg,
	const ccode_t target_cc, unsigned int *rev);


/** Returns base data app version string
 * \return Pointer to version if it's present and not the vanilla string.
 * NULL if version is not present or unchanged from default.
 */
extern const char*
clm_get_base_app_version_string(void);


/** Returns incremental data app version string
 * \return Pointer to version if it's present and not the vanilla string.
 * NULL if version is not present or unchanged from default.
 */
extern const char*
clm_get_inc_app_version_string(void);

/** Returns string stored in CLM data
 * \param[in] string_type Type of string
 * \param[in] string_source Location of string (base or incremental CLM data)
 * \param[out] rev Country (region) revision of found mapping
 * \return Pointer to requested string. NULL if string is absent in CLM data or
 * unchanged from default
 */
extern const char*
clm_get_string(clm_string_type_t string_type,
	clm_string_source_t string_source);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WLC_CLM_H_ */
