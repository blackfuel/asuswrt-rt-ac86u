/*
 * IE management module debugging facilities
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_ie_mgmt_dbg.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_ie_mgmt_dbg_h_
#define _wlc_ie_mgmt_dbg_h_

#ifdef BCMDBG
/* message bitvec */
#define IEM_DBG_ATTACH	(1U<<0)
#define IEM_DBG_TRACE	(1U<<1)
#define IEM_DBG_INFO	(1U<<2)
#define IEM_DBG_TIME	(1U<<3)
#define IEM_DBG_DUMP	(1U<<31)
/* debug/trace macros */
extern uint iem_msg_level;
#define IEM_ATTACH(x) do {if (iem_msg_level & IEM_DBG_ATTACH) printf x;} while (FALSE)
#define IEM_TRACE(x) do {if (iem_msg_level & IEM_DBG_TRACE) printf x;} while (FALSE)
#define IEM_INFO(x) do {if (iem_msg_level & IEM_DBG_INFO) printf x;} while (FALSE)
#define IEM_T32D(wlc, prev) ((iem_msg_level & IEM_DBG_TIME) && \
			     (wlc) != NULL && (wlc)->clk ?		\
			     R_REG((wlc)->osh, &(wlc)->regs->tsf_timerlow) - (prev) : \
			     0)
#define IEM_DUMP_ON() (iem_msg_level & IEM_DBG_DUMP) ? TRUE : FALSE
#else
#define IEM_ATTACH(x)
#define IEM_TRACE(x)
#define IEM_INFO(x)
#define IEM_T32D(wlc, prev) ((void)(prev), 0)
#define IEM_DUMP_ON() FALSE
#endif /* BCMDBG */

#endif /* _wlc_ie_mgmt_dbg_h_ */
