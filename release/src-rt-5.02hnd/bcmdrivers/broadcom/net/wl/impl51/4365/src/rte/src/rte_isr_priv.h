/*
 * OS independent ISR functions for ISRs or DPCs - Private to RTE.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_isr_priv.h 483472 2014-06-09 22:49:49Z $
 */

#ifndef	_hnd_isr_priv_
#define	_hnd_isr_priv_

#include <typedefs.h>
#include <osl_ext.h>
#include <rte_isr.h>

typedef struct hnd_isr_action hnd_isr_action_t;

struct hnd_isr_action {
	hnd_isr_action_t *next;
	uint coreid;
	uint32 event;
	hnd_isr_t isr;
	void *cbdata;
	uint32 sbtpsflag;
	osl_ext_task_t *thread;	/* thread context */
};

typedef struct hnd_isr_instance hnd_isr_instance_t;

struct hnd_isr_instance {
	hnd_isr_action_t *hnd_isr_action_list;
	uint32 hnd_action_flags;
};

/* get ISR instance */
extern hnd_isr_instance_t *hnd_isr;
#define hnd_isr_get_inst() hnd_isr

/* get DPC instance */
extern hnd_isr_instance_t *hnd_dpc;
#define hnd_dpc_get_inst() hnd_dpc

/* run isr based on sbflagst, optional callbacks invoked pre and post isr run */
void hnd_isr_proc_sbflagst(hnd_isr_instance_t *instance, uint32 sbflagst,
	void (*pre_cb)(hnd_isr_action_t *action),
	void (*post_cb)(hnd_isr_action_t *action));

/* run isr for specified event */
bool hnd_dpc_proc_event(hnd_isr_instance_t *instance, uint32 event);


/* Note: Each OS must implement this interface - query the event # an isr will post
 * in order to trigger its corresponding dpc.
 */
/* used by thread running dpc triggerred off an event */
uint hnd_isr_get_dpc_event(osl_ext_task_t *thread, uint32 coreid);


/* initialize registries */
void hnd_isr_module_init(osl_t *osh);

#endif /* _hnd_isr_priv_ */
