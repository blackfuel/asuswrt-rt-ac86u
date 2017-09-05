/*
 * Sample Collect module interface (to other PHY modules).
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

#ifndef _phy_samp_h_
#define _phy_samp_h_

#include <phy_api.h>

/* forward declaration */
typedef struct phy_samp_info phy_samp_info_t;

/* attach/detach */
phy_samp_info_t *phy_samp_attach(phy_info_t *pi);
void phy_samp_detach(phy_samp_info_t *cmn_info);

/* up/down */
int phy_samp_init(phy_samp_info_t *cmn_info);
int phy_samp_down(phy_samp_info_t *cmn_info);

/* MAC based sample play regs */
#define PHYREF_SampleCollectCurPtr	u.d11acregs.SampleCollectCurPtr
#define PHYREF_SaveRestoreStartPtr	u.d11acregs.SaveRestoreStartPtr
#define PHYREF_SampleCollectStopPtr	u.d11acregs.SampleCollectStopPtr
#define PHYREF_SampleCollectStartPtr	u.d11acregs.SampleCollectStartPtr
#define PHYREF_SampleCollectPlayCtrl	u.d11acregs.SampleCollectPlayCtrl
#define PHYREF_SampleCollectCurPtrHigh	u.d11acregs.SampleCollectCurPtrHigh
#define PHYREF_SampleCollectPlayPtrHigh	u.d11acregs.SampleCollectPlayPtrHigh

/* bitfields in PhyCtrl (IHR Address 0x049) */
#define PHYCTRL_SAMPLEPLAYSTART_SHIFT 11
#define PHYCTRL_MACPHYFORCEGATEDCLKSON_SHIFT 1

/* bitfields in SampleCollectPlayCtrl | Applicable to (d11rev >= 53) and (d11rev == 50) */
#define SAMPLE_COLLECT_PLAY_CTRL_PLAY_START_SHIFT 9


/* ****************************************** */
/* CMN Layer sample collect Public API's      */
/* ****************************************** */
#ifdef SAMPLE_COLLECT
extern int phy_iovars_sample_collect(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);

/* ************************* */
/* phytype export prototypes */
/* ************************* */

/* HTPHY */
extern int phy_ht_sample_data(phy_info_t *pi, wl_sampledata_t *p, void *b);
extern int phy_ht_sample_collect(phy_info_t *pi, wl_samplecollect_args_t *p, uint32 *b);

/* NPHY */
extern int8 phy_n_sample_collect_gainadj(phy_info_t *pi, int8 gainadj, bool set);
extern int phy_n_sample_data(phy_info_t *pi, wl_sampledata_t *p, void *b);
extern int phy_n_sample_collect(phy_info_t *pi,	wl_samplecollect_args_t *p, uint32 *b);
extern int phy_n_mac_triggered_sample_data(phy_info_t *pi, wl_sampledata_t *p, void *b);
extern int phy_n_mac_triggered_sample_collect(phy_info_t *pi, wl_samplecollect_args_t *p,
	uint32 *b);

/* LCN40PHY */
extern int phy_lcn40_sample_collect(phy_info_t *pi, wl_samplecollect_args_t *collect,
	uint32 *buf);
extern int8 phy_lcn40_sample_collect_gainadj(phy_info_t *pi, int8 gainadj, bool set);
extern uint8 phy_lcn40_sample_collect_gainidx(phy_info_t *pi, uint8 gainidx, bool set);
extern int phy_lcn40_iqimb_check(phy_info_t *pi, uint32 nsamps, uint32 *buf, int32 *metric,
	int32 *result);
#endif /* SAMPLE_COLLECT */

#endif /* _phy_samp_h_ */
