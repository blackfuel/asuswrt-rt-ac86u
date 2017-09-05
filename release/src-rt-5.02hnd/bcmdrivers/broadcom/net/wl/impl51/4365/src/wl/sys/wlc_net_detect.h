/*
 * Common (OS-independent) portion of
 * Broadcom support for Intel NetDetect interface.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_net_detect.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_net_detect_h_
#define _wlc_net_detect_h_

#ifdef NET_DETECT

/*
 * Initialize net detect private context.
 * Returns a pointer to the net detect private context, NULL on failure.
 */
extern wlc_net_detect_ctxt_t *wlc_net_detect_attach(wlc_info_t *wlc);

/* Cleanup net detect private context */
extern void wlc_net_detect_detach(wlc_net_detect_ctxt_t *net_detect_ctxt);

#endif	/* NET_DETECT */

#endif  /* _wlc_net_detect_h_ */
