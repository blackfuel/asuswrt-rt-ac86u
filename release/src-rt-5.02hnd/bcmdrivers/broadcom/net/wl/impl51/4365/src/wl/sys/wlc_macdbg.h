/*
 * MAC debug and print functions
 * Broadcom 802.11bang Networking Device Driver
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
#ifndef WLC_MACDBG_H_
#define WLC_MACDBG_H_

#include <typedefs.h>
#include <wlc_types.h>
#include <wlioctl.h>

/* fatal reason code */
#define PSM_FATAL_ANY		0
#define PSM_FATAL_PSMWD		1
#define PSM_FATAL_SUSP		2
#define PSM_FATAL_WAKE		3
#define PSM_FATAL_TXSUFL	4
#define PSM_FATAL_PSMXWD	5
#define PSM_FATAL_TXSTUCK	6
#define PSM_FATAL_LAST		7

#define PSMX_FATAL_ANY		0
#define PSMX_FATAL_PSMWD	1
#define PSMX_FATAL_SUSP		2
#define PSMX_FATAL_TXSTUCK	3
#define PSMX_FATAL_LAST		4

#define	PRVAL(name)	bcm_bprintf(b, "%s %d ", #name, WLCNTVAL(cnt->name))
#define	PRNL()		bcm_bprintf(b, "\n")
#define PRVAL_RENAME(varname, prname)	\
	bcm_bprintf(b, "%s %d ", #prname, WLCNTVAL(cnt->varname))

/* attach/detach */
extern wlc_macdbg_info_t *wlc_macdbg_attach(wlc_info_t *wlc);
extern void wlc_macdbg_detach(wlc_macdbg_info_t *macdbg);

#if defined(DONGLEBUILD) && defined(WLC_HOSTPMAC)
extern void wlc_macdbg_sendup_d11regs(wlc_macdbg_info_t *macdbg);
#else
#define wlc_macdbg_sendup_d11regs(a) do {} while (0)
#endif


extern void wlc_dump_ucode_fatal(wlc_info_t *wlc, uint reason);

/* catch any interrupts from psmx */
#ifdef WL_PSMX
void wlc_bmac_psmx_errors(wlc_info_t *wlc);
void wlc_dump_psmx_fatal(wlc_info_t *wlc, uint reason);
#ifdef VASIP_HW_SUPPORT
void wlc_dump_vasip_fatal(wlc_info_t *wlc);
#endif
#else
#define wlc_bmac_psmx_errors(wlc) do {} while (0)
#endif /* WL_PSMX */

extern void wlc_dump_mac_fatal(wlc_info_t *wlc, uint reason);

#endif /* WLC_MACDBG_H_ */
