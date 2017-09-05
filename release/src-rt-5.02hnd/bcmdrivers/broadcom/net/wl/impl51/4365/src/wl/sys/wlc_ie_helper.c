/*
 * IE management callback data structure decode utilities
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_ie_helper.c 467328 2014-04-03 01:23:40Z $
 */

/**
 * @file
 * @brief
 * Hard-coding IEs in management frames by calling out each module's IE build function is less
 * desirable for the following reasons:
 *     - it is costly to patch a ROM'ed management frame generation function when adding new
 *       features that required new IEs
 *     - it is hard to maintain the right order of the IEs presented without knowing all different
 *       features requirements
 *     - it forces other modules to export IE build/parse functions and/or data structures
 *     - it adds IE lookup overhead for each module to find their own IEs
 * The IE management module is designed to address above issues by providing:
 *     - callback registration to decouple management frames' build/parse function and participating
 *       modules and to provide future extension
 *     - IE tags table based on the specification to enforce the IE order in a central place
 *     - IE parser to reduce lookup overhead
 */


#include <wlc_cfg.h>
#include <typedefs.h>
#include <wlc_types.h>
#include <wlc_ie_mgmt_types.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_helper.h>

/* Accessors */

/*
 * 'calc_len' callback data structure decode accessors
 */
wlc_bsscfg_t *
wlc_iem_calc_get_cfg(wlc_iem_calc_data_t *calc)
{
	if (calc != NULL)
		return calc->cfg;
	return NULL;
}

uint16
wlc_iem_calc_get_ft(wlc_iem_calc_data_t *calc)
{
	if (calc != NULL)
		return calc->ft;
	return WLC_IEM_FC_UNK;
}

wlc_iem_cbparm_t *
wlc_iem_calc_get_parm(wlc_iem_calc_data_t *calc)
{
	if (calc != NULL)
		return calc->cbparm;
	return NULL;
}

/*
 * 'build' callback data structure decode accessors
 */
wlc_bsscfg_t *
wlc_iem_build_get_cfg(wlc_iem_build_data_t *build)
{
	if (build != NULL)
		return build->cfg;
	return NULL;
}

uint16
wlc_iem_build_get_ft(wlc_iem_build_data_t *build)
{
	if (build != NULL)
		return build->ft;
	return WLC_IEM_FC_UNK;
}

wlc_iem_cbparm_t *
wlc_iem_build_get_parm(wlc_iem_build_data_t *build)
{
	if (build != NULL)
		return build->cbparm;
	return NULL;
}

/*
 * 'parse' callback data structure decode accessors
 */
wlc_bsscfg_t *
wlc_iem_parse_get_cfg(wlc_iem_parse_data_t *parse)
{
	if (parse != NULL)
		return parse->cfg;
	return NULL;
}

uint16
wlc_iem_parse_get_ft(wlc_iem_parse_data_t *parse)
{
	if (parse != NULL)
		return parse->ft;
	return WLC_IEM_FC_UNK;
}

wlc_iem_pparm_t *
wlc_iem_parse_get_parm(wlc_iem_parse_data_t *parse)
{
	if (parse != NULL)
		return parse->pparm;
	return NULL;
}

/*
 * 'calc_len' Frame Type specific parameter structure decode accessors.
 */
wlc_bss_info_t *
wlc_iem_calc_get_assocreq_target(wlc_iem_calc_data_t *calc)
{
	switch (calc->ft) {
	case FC_ASSOC_REQ:
	case FC_REASSOC_REQ: {
		wlc_iem_cbparm_t *cbparm = calc->cbparm;
		wlc_iem_ft_cbparm_t *ftcbparm;

		ASSERT(cbparm != NULL);
		ASSERT(cbparm->ft != NULL);

		if (cbparm != NULL && (ftcbparm = cbparm->ft) != NULL)
			return ftcbparm->assocreq.target;
		break;
	}
	default:
		ASSERT(0);
		break;
	}

	return NULL;
}

/*
 * 'build' Frame Type specific parameter structure decode accessors.
 */
wlc_bss_info_t *
wlc_iem_build_get_assocreq_target(wlc_iem_build_data_t *build)
{
	switch (build->ft) {
	case FC_ASSOC_REQ:
	case FC_REASSOC_REQ: {
		wlc_iem_cbparm_t *cbparm = build->cbparm;
		wlc_iem_ft_cbparm_t *ftcbparm;

		ASSERT(cbparm != NULL);
		ASSERT(cbparm->ft != NULL);

		if (cbparm != NULL && (ftcbparm = cbparm->ft) != NULL)
			return ftcbparm->assocreq.target;
		break;
	}
	default:
		ASSERT(0);
		break;
	}

	return NULL;
}
