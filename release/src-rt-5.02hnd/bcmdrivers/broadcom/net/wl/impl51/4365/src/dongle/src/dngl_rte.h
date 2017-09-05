/*
 * RTE dongle private definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: dngl_rte.h 497491 2014-08-19 18:24:00Z $
 */

#ifndef	_dngl_rte_h_
#define	_dngl_rte_h_

#include <rte_dev.h>
#include <bcmcdc.h>

#ifndef MAXVSLAVEDEVS
#define MAXVSLAVEDEVS	0
#endif

#if MAXVSLAVEDEVS > BDC_FLAG2_IF_MASK
#error "MAXVSLAVEDEVS is too big"
#endif

#if defined(WLRSDB)
/* Slave adjust value to accomodate new cfg during bsscfg MOVE */
#define WLRSDB_CLONE_ADJUST_SLAVES 1
#endif

typedef struct dngl {
	void *bus;			/* generic bus handle */
	osl_t *osh;			/* OS abstraction layer handler */
	si_t *sih;			/* sb/si backplane handler */
	void *proto;			/* BDC/RNDIS proto handler */
	dngl_stats_t stats;
	int medium;
	hnd_dev_t *rtedev;
	hnd_dev_t *primary_slave;
	hnd_dev_t **slaves;		/* real  + virtual slave device handles for AP interfaces
					 * [Note: Total max is  BDC_FLAG2_IF_MASK ]
					 */
	int *iface2slave_map;
#ifdef BCMET
	et_cb_t cb;			/* Link event handler */
#endif
	uint8 unit;			/* Device index */
	bool up;
	bool devopen;
#ifdef FLASH_UPGRADE
	int upgrade_status;		/* Upgrade return status code */
#endif
	int tunable_max_slave_devs;
	int num_realslave_devs;
#ifdef BCM_FD_AGGR
	void *rpc_th;           /* handle for the bcm rpc tp module */
	dngl_timer_t *rpctimer;     /* Timer for toggling gpio output */
	uint16 rpctime;     /* max time  to push the aggregation */
	bool rpctimer_active;   /* TRUE = rpc timer is running */
	bool fdaggr;    /* 1 = aggregation enabled */
#endif
	uint16	data_seq_no;
	uint16  ioctl_seq_no;
	uint16	data_seq_no_prev;
	uint16  ioctl_seq_no_prev;

} dngl_t;

#endif	/* _dngl_rte_h_ */
