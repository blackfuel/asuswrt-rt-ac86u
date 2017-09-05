/*
 * Separate alloc/free module for wlc_xxx.c files. Decouples
 * the code that does alloc/free from other code so data
 * structure changes don't affect ROMMED code as much.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_alloc.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_alloc_h_
#define _wlc_alloc_h_

extern wlc_info_t *wlc_attach_malloc(osl_t *osh, uint unit, uint *err, uint devid, void *objr);
extern void wlc_detach_mfree(wlc_info_t *wlc, osl_t *osh);

#endif /* _wlc_alloc_h_ */
