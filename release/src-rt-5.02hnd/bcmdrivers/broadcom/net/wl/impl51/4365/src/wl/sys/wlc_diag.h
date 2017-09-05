/*
 * Required functions exported by the wlc_diag.c
 * to common (os-independent) driver code.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_diag.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_diag_h_
#define _wlc_diag_h_


extern int wlc_diag(wlc_info_t *wlc, uint32 diagtype, uint32 *result);

#endif	/* _wlc_diag_h_ */
