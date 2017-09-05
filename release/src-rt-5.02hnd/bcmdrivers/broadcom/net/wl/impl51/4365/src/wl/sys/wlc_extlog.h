/*
 * EXTLOG Module Public Interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_extlog.h 467328 2014-04-03 01:23:40Z $
 */
#ifndef _WLC_EXTLOG_H_
#define _WLC_EXTLOG_H_

#ifdef WLEXTLOG
#include "wlioctl.h"

#define WLC_EXTLOG	wlc_extlog_msg

struct wlc_extlog_info {
	wlc_info_t *wlc;
	osl_t *osh;
	wlc_extlog_cfg_t cfg;
	uint8 last_idx;
	uint8 cur_idx;
	uint32 seq_num;
	log_record_t *log_table;
};

extern void *wlc_extlog_attach(osl_t *osh, wlc_info_t *wlc);
extern int wlc_extlog_detach(wlc_extlog_info_t *wlc_extlog);
extern void wlc_extlog_msg(wlc_info_t *wlc, uint16 module, uint8 id,
	uint8 level, uint8 sub_unit, int arg, char *str);
#else
#define WLC_EXTLOG(wlc, module, id, level, sub_unit, arg, str) do {} while (0)
#endif /* WLEXTLOG */

#endif /* _WLC_EXTLOG_H_ */
