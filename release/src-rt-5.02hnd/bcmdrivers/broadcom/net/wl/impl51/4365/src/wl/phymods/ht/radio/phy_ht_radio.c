/*
 * HTPHY RADIO contorl module implementation
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_radio.h>
#include "phy_type_radio.h"
#include <phy_ht.h>
#include <phy_ht_radio.h>
#include "phy_utils_reg.h"

#include <wlc_phy_radio.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_ht.h>
/* TODO: all these are going away... > */
#endif

/* module private states */
struct phy_ht_radio_info {
	phy_info_t *pi;
	phy_ht_info_t *hti;
	phy_radio_info_t *ri;
};

/* local functions */
static void phy_ht_radio_switch(phy_type_radio_ctx_t *ctx, bool on);
static void phy_ht_radio_on(phy_type_radio_ctx_t *ctx);
static void phy_ht_radio_off(phy_type_radio_ctx_t *ctx);
#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)
static int phy_ht_radio_dump(phy_type_radio_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_ht_radio_dump NULL
#endif

/* Register/unregister HTPHY specific implementation to common layer. */
phy_ht_radio_info_t *
BCMATTACHFN(phy_ht_radio_register_impl)(phy_info_t *pi, phy_ht_info_t *hti,
	phy_radio_info_t *ri)
{
	phy_ht_radio_info_t *info;
	phy_type_radio_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ht_radio_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_ht_radio_info_t));
	info->pi = pi;
	info->hti = hti;
	info->ri = ri;

	pi->pubpi.radiooffset = RADIO_2059_READ_OFF;

#ifndef BCM_OL_DEV
	/* make sure the radio is off until we do an "up" */
	phy_ht_radio_switch(info, OFF);
#endif

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctrl = phy_ht_radio_switch;
	fns.on = phy_ht_radio_on;
	fns.bandx = phy_ht_radio_off;
	fns.init = phy_ht_radio_off;
	fns.dump = phy_ht_radio_dump;
	fns.ctx = info;

	phy_radio_register_impl(ri, &fns);

	return info;

fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ht_radio_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ht_radio_unregister_impl)(phy_ht_radio_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_radio_info_t *ri = info->ri;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_radio_unregister_impl(ri);

	phy_mfree(pi, info, sizeof(phy_ht_radio_info_t));
}

/* switch radio on/off */
static void
phy_ht_radio_switch(phy_type_radio_ctx_t *ctx, bool on)
{
	phy_ht_radio_info_t *info = (phy_ht_radio_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("wl%d: %s %d\n", pi->sh->unit, __FUNCTION__, on));

	wlc_phy_switch_radio_htphy(pi, on);
}

/* turn radio on */
static void
WLBANDINITFN(phy_ht_radio_on)(phy_type_radio_ctx_t *ctx)
{
	phy_ht_radio_switch(ctx, ON);
}

/* switch radio off */
static void
phy_ht_radio_off(phy_type_radio_ctx_t *ctx)
{
	phy_ht_radio_switch(ctx, OFF);
}

/* query radio idcode */
uint32
phy_ht_radio_query_idcode(phy_info_t *pi)
{
	uint32 b0, b1, b2;

	W_REG(pi->sh->osh, &pi->regs->radioregaddr, 0);
#ifdef __mips__
	(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif
	b0 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);
	W_REG(pi->sh->osh, &pi->regs->radioregaddr, 1);
#ifdef __mips__
	(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif
	b1 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);
	W_REG(pi->sh->osh, &pi->regs->radioregaddr, 2);
#ifdef __mips__
	(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif
	b2 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);

	return ((b0  & 0xf) << 28) | (((b2 << 8) | b1) << 12) | ((b0 >> 4) & 0xf);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#if defined(DBG_PHY_IOV)
static int
phy_ht_radio_dump(phy_type_radio_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_ht_radio_info_t *gi = (phy_ht_radio_info_t *)ctx;
	phy_info_t *pi = gi->pi;
	const char *name = NULL;
	int i, jtag_core;
	uint16 addr = 0;
	radio_20xx_regs_t *radio20xxregs = NULL;

	if (RADIOID(pi->pubpi.radioid) == BCM2059_ID) {
		radio20xxregs = regs_2059_rev0;
		name = "2059";
	}
	else
		return BCME_ERROR;
	jtag_core = 0;

	bcm_bprintf(b, "----- %08s -----\n", name);
	bcm_bprintf(b, "Add Value0 Value1 ...\n");

	i = 0;
	while (TRUE) {
		addr = radio20xxregs[i].address;

		if (addr == 0xffff)
			break;

		jtag_core = (addr & JTAG_2059_MASK);
		addr &= (~JTAG_2059_MASK);

		if ((RADIOID(pi->pubpi.radioid) == BCM2059_ID) &&
			(jtag_core == JTAG_2059_ALL)) {
			bcm_bprintf(b, "%03x %04x %04x %04x\n", addr,
			            phy_utils_read_radioreg(pi, addr | JTAG_2059_CR0),
			            phy_utils_read_radioreg(pi, addr | JTAG_2059_CR1),
			            phy_utils_read_radioreg(pi, addr | JTAG_2059_CR2));
		}
		i++;
	}

	return BCME_OK;
}
#endif 
#endif /* BCMDBG || BCMDBG_DUMP */
