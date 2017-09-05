/*
 * PATCH routines common hdr file
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_patch.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_patch_h_
#define _wlc_patch_h_

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <wlc_types.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc.h>

#ifndef BCM_OL_DEV
#include <wlc_bsscfg.h>
#endif


/* "Patch preambles" are assembly instructions corresponding to the first couple instructions
 * for each ROM function. These instructions are executed (in RAM) by manual patch functions prior
 * to branching to an offset within the patched ROM function. This avoids recursively hitting the
 * TCAM entry located at the beginning of the ROM function (in the absense of ROM function nop
 * preambles).
 */
#if defined(BCMROM_PATCH_PREAMBLE)
	#define CALLROM_ENTRY(a) a##__bcmromfn_preamble
#else
	#define CALLROM_ENTRY(a) a##__bcmromfn
#endif


#ifdef WLC_PATCH_IOCTL

extern int wlc_ioctl_patchmod(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif);


#endif /* WLC_PATCH_IOCTL */


/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file. It must be
 * included after the prototypes above. The name of the included source file (WLC_PATCH_IOCTL_FILE)
 * is defined by the build environment.
 */
#if (defined(WLC_PATCH_IOCTL) || defined(WLC_PATCH_IOCTL_CHECKSUM))
	#if defined(WLC_PATCH_IOCTL_FILE)
		#include WLC_PATCH_IOCTL_FILE
	#endif
#endif /* WLC_PATCH_IOCTL || WLC_PATCH_IOCTL_CHECKSUM */


#endif /* _wlc_patch_h_ */
