/*
 * Antenna Selection related header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_antsel.h 467328 2014-04-03 01:23:40Z $
*/

#ifndef _wlc_antsel_h_
#define _wlc_antsel_h_

#ifdef WLANTSEL
extern antsel_info_t *wlc_antsel_attach(wlc_info_t *wlc);
extern void wlc_antsel_detach(antsel_info_t *asi);
extern void wlc_antsel_init(antsel_info_t *asi);
extern uint16 wlc_antsel_buildtxh(antsel_info_t *asi, uint8 antcfg, uint8 fbantcfg);
extern void wlc_antsel_antcfg_get(antsel_info_t *asi, bool usedef, bool sel,
	uint8 id, uint8 fbid, uint8 *antcfg, uint8 *fbantcfg);
extern void wlc_antsel_ratesel(antsel_info_t *asi, uint8 *active_antcfg_num,
	uint8 *antselid_init);
extern void wlc_antsel_upd_dflt(antsel_info_t *asi, uint8 antselid);
extern uint8 wlc_antsel_antsel2id(antsel_info_t *asi, uint16 antsel);
extern void wlc_antsel_set_unicast(antsel_info_t *asi, uint8 antcfg);
extern uint8 wlc_antsel_antseltype_get(antsel_info_t *asi);
#else /* WLANTSEL */
static INLINE int wlc_antsel_detach(antsel_info_t *asi) { return 0; }
static INLINE uint16 wlc_antsel_buildtxh(antsel_info_t *asi, uint8 antcfg,
                                         uint8 fbantcfg) { return 0; }
static INLINE uint8 wlc_antsel_antsel2id(antsel_info_t *asi,
                                         uint16 antsel) {return 0;}
static INLINE uint8 wlc_antsel_antseltype_get(antsel_info_t *asi) {return 0;}
#define wlc_antsel_attach(a)	(antsel_info_t *)0xdeadbeef
#define wlc_antsel_init(a) 	do {} while (0)
#define wlc_antsel_antcfg_get(a, b, c, d, e, f, g) do {} while (0)
#define wlc_antsel_ratesel(a, b, c)	do {} while (0)
#define wlc_antsel_upd_dflt(a, b)	do {} while (0)
#define wlc_antsel_set_unicast(a, b)	do {} while (0)

#endif /* WLANTSEL */

#endif /* _wlc_antsel_h_ */
