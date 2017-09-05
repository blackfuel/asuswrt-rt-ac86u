/*
 * HostFetch module interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_fetch.h 474540 2014-05-01 18:40:15Z $
 */


#ifndef _rte_fetch_h_
#define _rte_fetch_h_

#include <typedefs.h>
#include <osl_decl.h>
#include <bcmutils.h>
#include <sbhnddma.h>

/* Private fetch_rqst flags
 * Make sure these flags are cleared before dispatching
 */
#define FETCH_RQST_IN_BUS_LAYER 0x01
#define FETCH_RQST_CANCELLED 0x02

/* External utility API */
#define FETCH_RQST_FLAG_SET(fr, bit) ((fr)->flags |= bit)
#define FETCH_RQST_FLAG_GET(fr, bit) ((fr)->flags & bit)
#define FETCH_RQST_FLAG_CLEAR(fr, bit) ((fr)->flags &= (~bit))

typedef struct fetch_rqst fetch_rqst_t;

typedef void (*fetch_cmplt_cb_t)(struct fetch_rqst *rqst, bool cancelled);

struct fetch_rqst {
	osl_t *osh;
	dma64addr_t haddr;
	uint16 size;
	uint8 flags;
	uint8 rsvd;
	uint8 *dest;
	fetch_cmplt_cb_t cb;
	void *ctx;
	struct fetch_rqst *next;
};

typedef int (*bus_dispatch_cb_t)(struct fetch_rqst *fr, void *arg);

struct fetch_module_info {
	struct pktpool *pool;
	bus_dispatch_cb_t cb;
	void *arg;
};

extern struct fetch_module_info *fetch_info;

void hnd_fetch_bus_dispatch_cb_register(bus_dispatch_cb_t cb, void *arg);
void hnd_fetch_rqst(struct fetch_rqst *fr);
void hnd_wake_fetch_rqstq(void);
bool hnd_fetch_rqstq_empty(void);
void hnd_flush_fetch_rqstq(void);
void hnd_dmadesc_avail_cb(void);
int hnd_cancel_fetch_rqst(struct fetch_rqst *fr);

void hnd_fetch_module_init(osl_t *osh);

#endif /* _rte_fetch_h_ */
