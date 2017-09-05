/*
 * HT PHY RXIQ CAL module implementation
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
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_rxiqcal.h>
#include <phy_ht.h>
#include <phy_ht_rxiqcal.h>

/* module private states */
struct phy_ht_rxiqcal_info {
	phy_info_t *pi;
	phy_ht_info_t *aci;
	phy_rxiqcal_info_t *cmn_info;
/* add other variable size variables here at the end */
};

/* register phy type specific implementation */
phy_ht_rxiqcal_info_t *
BCMATTACHFN(phy_ht_rxiqcal_register_impl)(phy_info_t *pi, phy_ht_info_t *aci,
	phy_rxiqcal_info_t *cmn_info)
{
	phy_ht_rxiqcal_info_t *ac_info;
	phy_type_rxiqcal_fns_t fns;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ht_rxiqcal_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	bzero(ac_info, sizeof(phy_ht_rxiqcal_info_t));
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	fns.ctx = ac_info;

	if (phy_rxiqcal_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_rxiqcal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_ht_rxiqcal_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ht_rxiqcal_unregister_impl)(phy_ht_rxiqcal_info_t *ac_info)
{
	phy_rxiqcal_info_t *cmn_info = ac_info->cmn_info;
	phy_info_t *pi = ac_info->pi;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_rxiqcal_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info, sizeof(phy_ht_rxiqcal_info_t));
}
