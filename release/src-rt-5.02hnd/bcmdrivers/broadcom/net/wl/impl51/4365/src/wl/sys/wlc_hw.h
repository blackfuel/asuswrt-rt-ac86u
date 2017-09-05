/*
 * H/W info API of
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
 * $Id: wlc_hw.h 527924 2015-01-20 18:09:41Z $
 */

#ifndef _wlc_hw_h_
#define _wlc_hw_h_

#include <wlc_types.h>
#include <wlc_phy_types.h>
#include <hnddma.h>
#include <wlc_pio.h>

#define WLC_HW_DMACOMMON(wlc) ((wlc)->hw_pub->dmacommon)
#define WLC_HW_DI(wlc, fifo) ((wlc)->hw_pub->di[fifo])
#ifdef BCM_DMA_CT
#define WLC_HW_AQM_DI(wlc, fifo) ((wlc)->hw_pub->aqm_di[fifo])
#endif
#define WLC_HW_PIO(wlc, fifo) ((wlc)->hw_pub->pio[fifo])
#define WLC_HW_NFIFO_INUSE(wlc) ((wlc)->hw_pub->nfifo_inuse)

/* Data APIs */
/* Code outside the HW module (wlc_bmac, wlc_high_stubs, wlc_intr, wlc_diag,
 * wlc_bmac_stubs?, etc.) shall not modify nor read these data directly, and
 * they shall use APIs or MACROs in case these data are moved to inside the
 * module later when performance is determined to be a non-issue.
 *
 * These data shall be read via the following MACROs:
 * - WLC_HW_DMACOMMON()
 * - WLC_HW_DI()
 * - WLC_HW_AQM_DI()
 * - WLC_HW_PIO()
 * - WLC_HW_NFIFO_INUSE()
 * These data shall be modified via the following APIs:
 * - wlc_hw_set_dmacommon()
 * - wlc_hw_set_di()
 * - wlc_hw_set_aqm_di()
 * - wlc_hw_set_pio()
 */
struct wlc_hw {
	dma_common_t *dmacommon;         /* Global DMA state and info for all DMA channels to */
	hnddma_t	*di[NFIFO_EXT];		/* hnddma handles, per fifo */
	pio_t		*pio[NFIFO];		/* pio handlers, per fifo */
	uint		nfifo_inuse;		/* # of FIFOs that are in use.
						 * Based on certain features like WL_MU_TX this
						 * can change
						 */
	hnddma_t	*aqm_di[NFIFO_EXT];	/* hnddma handles, per AQM fifo */
};

/*
 * Detect Card removed.
 * Even checking an sbconfig register read will not false trigger when the core is in reset.
 * it breaks CF address mechanism. Accessing gphy phyversion will cause SB error if aphy
 * is in reset on 4306B0-DB. Need a simple accessible reg with fixed 0/1 pattern
 * (some platforms return all 0).
 * If clocks are present, call the sb routine which will figure out if the device is removed.
 */
#ifdef BCMNODOWN
#define DEVICEREMOVED(wlc)	0
#elif defined(BCMSDIO) && !defined(BCMDBUS)
#define DEVICEREMOVED(wlc)		\
	((wlc)->pub->sih->bustype == SDIO_BUS ?	\
	 !(wlc)->device_present : \
	 ((wlc)->clk ? \
	  (R_REG((wlc)->osh, &(wlc)->regs->maccontrol) & \
	    (MCTL_PSM_JMP_0 | MCTL_IHR_EN)) != MCTL_IHR_EN : \
	  si_deviceremoved((wlc)->pub->sih)))
#elif defined(WLC_HIGH_ONLY)
#define DEVICEREMOVED(wlc)	(!wlc->device_present)
#else
#define DEVICEREMOVED(wlc)      wlc_hw_deviceremoved((wlc)->hw)
#endif /* BCMNODOWN */

/* Function APIs */
extern wlc_hw_info_t *wlc_hw_attach(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err);
extern void wlc_hw_detach(wlc_hw_info_t *wlc_hw);

extern void wlc_hw_set_piomode(wlc_hw_info_t *wlc_hw, bool piomode);
extern bool wlc_hw_get_piomode(wlc_hw_info_t *wlc_hw);

extern void wlc_hw_set_dmacommon(wlc_hw_info_t *wlc_hw, dma_common_t *dmacommon);
extern void wlc_hw_set_di(wlc_hw_info_t *wlc_hw, uint fifo, hnddma_t *di);
#ifdef BCM_DMA_CT
extern void wlc_hw_set_aqm_di(wlc_hw_info_t *wlc_hw, uint fifo, hnddma_t *di);
#endif
extern void wlc_hw_set_pio(wlc_hw_info_t *wlc_hw, uint fifo, pio_t *pio);
extern void wlc_hw_set_nfifo_inuse(wlc_hw_info_t *wlc_hw, uint fifo_inuse);

#ifdef WLC_LOW
extern bool wlc_hw_deviceremoved(wlc_hw_info_t *wlc_hw);
extern uint32 wlc_hw_get_wake_override(wlc_hw_info_t *wlc_hw);
extern uint wlc_hw_get_bandunit(wlc_hw_info_t *wlc_hw);
extern uint32 wlc_hw_get_macintmask(wlc_hw_info_t *wlc_hw);
extern uint32 wlc_hw_get_macintstatus(wlc_hw_info_t *wlc_hw);
#endif

#if defined(WLC_HIGH) || defined(SCANOL)
/* MHF2_SKIP_ADJTSF ucode host flag manipulation - global user ID */
#define WLC_SKIP_ADJTSF_SCAN		0
#define WLC_SKIP_ADJTSF_RM		1
#define WLC_SKIP_ADJTSF_MCNX		2
#define WLC_SKIP_ADJTSF_USER_MAX	4
extern void wlc_skip_adjtsf(wlc_info_t *wlc, bool skip, wlc_bsscfg_t *cfg, uint32 user, int bands);
/* MCTL_AP maccontrol register bit manipulation - global user ID */
#define WLC_AP_MUTE_SCAN	0
#define WLC_AP_MUTE_RM		1
#define WLC_AP_MUTE_USER_MAX	4
/* mux s/w MCTL_AP on/off request */
#ifdef AP
extern void wlc_ap_mute(wlc_info_t *wlc, bool mute, wlc_bsscfg_t *cfg, uint32 user);
#else
#define wlc_ap_mute(wlc, mute, cfg, user) do {} while (0)
#endif
/* force MCTL_AP off; mux s/w MCTL_AP on request */
extern void wlc_ap_ctrl(wlc_info_t *wlc, bool on, wlc_bsscfg_t *cfg, uint32 user);
#endif /* WLC_HIGH || SCANOL */

extern void wlc_template_cfg_init(wlc_info_t *wlc, uint corerev);

#endif /* !_wlc_hw_h_ */
