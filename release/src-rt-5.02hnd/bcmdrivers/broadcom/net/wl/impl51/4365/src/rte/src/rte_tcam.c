/*
 * RTE TCAM support routines
 *
 * TCAM init/load can be done before si_attach()
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_tcam.c 522844 2014-12-24 02:03:22Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <sbsocram.h>
#include <hndtcam.h>
#include <rte_heap.h>
#include <rte_tcam.h>

void
BCMATTACHFN(hnd_tcam_reclaim)(void)
{
	uint8 *as = (uint8 *) _patch_align_start;
	uint8 *hs = (uint8 *) _patch_hdr_start;
	uint8 *he = (uint8 *) _patch_hdr_end;
	uint8 *pf = (uint8 *) _patch_table_start;
	uint8 *ps = (uint8 *) _patch_table_last;
	uint8 *pe = (uint8 *) _patch_table_end;
	int asize = pf - as;
	int hsize = he - hs;
	int psize = pe - ps;

	/* Dump stats before reclaim */
	hnd_tcam_stat();

	/* Reclaim padding from alignment */
	if (asize) {
		bzero(as, asize);
		hnd_arena_add((uint32)as, asize);
	}

	/* Reclaim patch header list */
	if (hsize) {
		bzero(hs, hsize);
		hnd_arena_add((uint32)hs, hsize);
	}

	/* Reclaim unused patch entries from pre-allocated patch table */
	if (psize) {
		bzero(ps, psize);
		hnd_arena_add((uint32)ps, psize);
	}
}
