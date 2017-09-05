/*
 * Packet fetch (from host) interface.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_pktfetch.h 500220 2014-09-03 07:55:07Z $
 */


#ifndef _rte_pktfetch_h_
#define _rte_pktfetch_h_

#include <typedefs.h>
#include <osl_decl.h>
#include <siutils.h>

/* PktFetch module related stuff */
/* PktFetch info small enough to fit into PKTTAG of lbuf/lfrag */

typedef void (*pktfetch_cmplt_cb_t)(void *lbuf, void *orig_lfrag, void *ctx, bool cancelled);

struct pktfetch_info {
	osl_t *osh;
	void *lfrag;
	uint16 headroom;
	int16 host_offset;
	pktfetch_cmplt_cb_t cb;
	void *ctx;
	struct pktfetch_info *next;
};

struct pktfetch_generic_ctx {
	uint32 ctx_count;
	void* ctx[0];
};

/* Set default pktfetch headroom to 256:
 * greater than TXOFF + amsdu headroom requirements
 */
#define PKTFETCH_DEFAULT_HEADROOM	0

int hnd_pktfetch(struct pktfetch_info *pinfo);
void hnd_pktfetch_module_init(si_t *sih, osl_t *osh);
void hnd_pktfetchpool_init(osl_t *osh);

#endif /* _rte_pktfetch_h_ */
