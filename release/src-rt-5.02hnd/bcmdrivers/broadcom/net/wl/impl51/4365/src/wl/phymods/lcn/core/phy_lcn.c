/*
 * LCNPHY Core module implementation
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_lcn_ana.h>
#include <phy_lcn_radio.h>
#include <phy_lcn_tbl.h>
#include <phy_lcn_tpc.h>
#include <phy_lcn_noise.h>
#include <phy_lcn_antdiv.h>
#include <phy_lcn_rssi.h>
#include "phy_type.h"
#include "phy_type_lcn.h"
#include "phy_type_lcn_iovt.h"
#include "phy_type_lcn_ioct.h"
#include <phy_lcn.h>
#include <phy_utils_radio.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn.h>
/* TODO: all these are going away... > */
#endif

/* local functions */
static int phy_lcn_attach_ext(phy_info_t *pi, int bandtype);
static int phy_lcn_register_impl(phy_info_t *pi, phy_type_info_t *ti, int bandtype);
static void phy_lcn_unregister_impl(phy_info_t *pi, phy_type_info_t *ti);
#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)
static int phy_lcn_dump_phyregs(phy_info_t *pi, phy_type_info_t *ti, struct bcmstrbuf *b);
#else
#define	phy_lcn_dump_phyregs	NULL
#endif 

/* attach/detach */
phy_type_info_t *
BCMATTACHFN(phy_lcn_attach)(phy_info_t *pi, int bandtype)
{
	phy_lcn_info_t *lcni;
	phy_type_fns_t fns;
	uint32 idcode;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	/* Extend phy_attach() here to initialize LCNPHY specific stuff */
	if (phy_lcn_attach_ext(pi, bandtype) != BCME_OK) {
		PHY_ERROR(("%s: phy_lcn_attach_ext failed\n", __FUNCTION__));
		return NULL;
	}

	/* read idcode */
	idcode = phy_lcn_radio_query_idcode(pi);
	PHY_TRACE(("%s: idcode 0x%08x\n", __FUNCTION__, idcode));
	/* parse idcode */
	phy_utils_parse_idcode(pi, idcode);
	/* validate radio id */
	if (phy_utils_valid_radio(pi) != BCME_OK) {
		PHY_ERROR(("%s: phy_utils_valid_radio failed\n", __FUNCTION__));
		return NULL;
	}

	/* TODO: move the acphy attach code to here... */
	if (wlc_phy_attach_lcnphy(pi, bandtype) == FALSE) {
		PHY_ERROR(("%s: wlc_phy_attach_lcnphy failed\n", __FUNCTION__));
		return NULL;
	}
	lcni = pi->u.pi_lcnphy;
	lcni->pi = pi;

	/* register PHY type implementation entry points */
	bzero(&fns, sizeof(fns));
	fns.reg_impl = phy_lcn_register_impl;
	fns.unreg_impl = phy_lcn_unregister_impl;
	fns.reg_iovt = phy_lcn_register_iovt;
	fns.reg_ioct = phy_lcn_register_ioct;
	fns.dump_phyregs = phy_lcn_dump_phyregs;
	fns.ti = (phy_type_info_t *)lcni;

	phy_register_impl(pi, &fns);

	return (phy_type_info_t *)lcni;
}

void
BCMATTACHFN(phy_lcn_detach)(phy_type_info_t *ti)
{
	phy_lcn_info_t *lcni = (phy_lcn_info_t *)ti;
	phy_info_t *pi = lcni->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* TODO: move the detach code to here... */
	wlc_phy_detach_lcnphy(pi);
}

static int
BCMATTACHFN(phy_lcn_attach_ext)(phy_info_t *pi, int bandtype)
{
	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	pi->phynoise_polling = FALSE;

	return BCME_OK;
}

/* Register/unregister LCNPHY specific implementations to their commons.
 * Used to configure features/modules implemented for LCNPHY.
 */
static int
BCMATTACHFN(phy_lcn_register_impl)(phy_info_t *pi, phy_type_info_t *ti, int bandtype)
{
	phy_lcn_info_t *lcni = (phy_lcn_info_t *)ti;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	/* Register with ANAcore control module */
	if (pi->anai != NULL &&
	    (lcni->anai = phy_lcn_ana_register_impl(pi, lcni, pi->anai)) == NULL) {
		PHY_ERROR(("%s: phy_lcn_ana_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with RADIO control module */
	if (pi->radioi != NULL &&
	    (lcni->radioi = phy_lcn_radio_register_impl(pi, lcni, pi->radioi)) == NULL) {
		PHY_ERROR(("%s: phy_lcn_radio_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with PHYTableInit module */
	if (pi->tbli != NULL &&
	    (lcni->tbli = phy_lcn_tbl_register_impl(pi, lcni, pi->tbli)) == NULL) {
		PHY_ERROR(("%s: phy_lcn_tbl_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with TxPowerCtrl module */
	if (pi->tpci != NULL &&
	    (lcni->tpci = phy_lcn_tpc_register_impl(pi, lcni, pi->tpci)) == NULL) {
		PHY_ERROR(("%s: phy_lcn_tpc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with ANTennaDIVersity module */
	if (pi->antdivi != NULL &&
	    (lcni->antdivi = phy_lcn_antdiv_register_impl(pi, lcni, pi->antdivi)) == NULL) {
		PHY_ERROR(("%s: phy_lcn_antdiv_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

#ifndef WLC_DISABLE_ACI
	/* Register with INTerFerence module */
	if (pi->noisei != NULL &&
	    (lcni->noisei = phy_lcn_noise_register_impl(pi, lcni, pi->noisei)) == NULL) {
		PHY_ERROR(("%s: phy_lcn_noise_register_impl failed\n", __FUNCTION__));
		goto fail;
	}
#endif

	/* Register with RSSICompute module */
	if (pi->rssii != NULL &&
	    (lcni->rssii = phy_lcn_rssi_register_impl(pi, lcni, pi->rssii)) == NULL) {
		PHY_ERROR(("%s: phy_lcn_rssi_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* ...Add your module registration here... */

	return BCME_OK;
fail:
	return BCME_ERROR;
}

static void
BCMATTACHFN(phy_lcn_unregister_impl)(phy_info_t *pi, phy_type_info_t *ti)
{
	phy_lcn_info_t *lcni = (phy_lcn_info_t *)ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Unregister from ANAcore control module */
	if (lcni->anai != NULL)
		phy_lcn_ana_unregister_impl(lcni->anai);

	/* Unregister from RADIO control module */
	if (lcni->radioi != NULL)
		phy_lcn_radio_unregister_impl(lcni->radioi);

	/* Unregister from PHYTableInit module */
	if (lcni->tbli != NULL)
		phy_lcn_tbl_unregister_impl(lcni->tbli);

	/* Unregister from TxPowerCtrl module */
	if (lcni->tpci != NULL)
		phy_lcn_tpc_unregister_impl(lcni->tpci);

	/* Unregister from ANTennaDIVersity module */
	if (lcni->antdivi != NULL)
		phy_lcn_antdiv_unregister_impl(lcni->antdivi);

#ifndef WLC_DISABLE_ACI
	/* Unregister from INTerFerence module */
	if (lcni->noisei != NULL)
		phy_lcn_noise_unregister_impl(lcni->noisei);
#endif

	/* Unregister from RSSICompute module */
	if (lcni->rssii != NULL)
		phy_lcn_rssi_unregister_impl(lcni->rssii);

	/* ...Add your module registration here... */
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#if defined(DBG_PHY_IOV)
static int
phy_lcn_dump_phyregs(phy_info_t *pi, phy_type_info_t *ti, struct bcmstrbuf *b)
{
	PHY_TRACE(("%s:***CHECK***\n", __FUNCTION__));

	return BCME_OK;
}
#endif 
#endif /* BCMDBG || BCMDBG_DUMP */
