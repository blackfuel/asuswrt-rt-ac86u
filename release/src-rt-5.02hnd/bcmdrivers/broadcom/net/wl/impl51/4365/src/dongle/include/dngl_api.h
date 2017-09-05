/*
 * RTE DONGLE API external definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: dngl_api.h 474819 2014-05-02 10:22:36Z $
 */

#ifndef _dngl_api_h_
#define _dngl_api_h_

#include <dngl_stats.h>
#include <osl.h>

#define DNGL_MEDIUM_UNKNOWN	0		/* medium is non-wireless (802.3) */
#define DNGL_MEDIUM_WIRELESS	1		/* medium is wireless (802.11) */

struct dngl_bus;
struct dngl;

#include <rte_timer.h>
typedef hnd_timer_t dngl_timer_t;
typedef hnd_task_t dngl_task_t;
#define dngl_init_timer(context, data, fn) hnd_timer_create(context, data, fn, NULL)
#define dngl_free_timer(t) hnd_timer_free(t)
#define dngl_add_timer(t, ms, periodic) hnd_timer_start(t, ms, periodic)
#define dngl_del_timer(t) hnd_timer_stop(t)
#include <rte.h>
#define dngl_schedule_work(context, data, fn, delay) hnd_schedule_work(context, data, fn, delay)
#ifdef BCMDBG_SD_LATENCY
#define dngl_time_now_us() hnd_time_us()
#endif /* BCMDBG_SD_LATENCY */
extern void _dngl_reboot(dngl_task_t *task);

extern void dngl_keepalive(struct dngl *dngl, uint32 sec);

/* DONGLE OS operations */
extern struct dngl *dngl_attach(struct dngl_bus *bus, void *drv, si_t *sih, osl_t *osh);
extern void dngl_detach(struct dngl *dngl);
#ifdef RSOCK
extern int dngl_sendslave(struct dngl *dngl, void *p);
#endif
extern void dngl_sendup(struct dngl *dngl, void *p);
extern void dngl_ctrldispatch(struct dngl *dngl, void *p, uchar *buf);
extern void dngl_txstop(struct dngl *dngl);
extern void dngl_txstart(struct dngl *dngl);
extern void dngl_suspend(struct dngl *dngl);
extern void dngl_resume(struct dngl *dngl);
extern void dngl_init(struct dngl *dngl);
extern void dngl_halt(struct dngl *dngl);
extern void dngl_reset(struct dngl *dngl);
extern int dngl_binddev(struct dngl *dngl, void *bus, void *dev, uint numslaves);
extern void dngl_rebinddev(struct dngl *dngl, void *bus, void *new_dev, int ifindex);
extern int dngl_unbinddev(struct dngl *dngl, void *bus, void *dev);
extern int dngl_opendev(struct dngl *dngl);
extern void dngl_rxflowcontrol(struct dngl *dngl, bool state, int prio);
#ifdef RSOCK
extern int dngl_sendbus(struct dngl *dngl, void *p);
#endif
#ifdef BCMUSBDEV_BMAC
extern int dngl_sendpkt(struct dngl *dngl, void *src, void *p, uint32 ep_idx);
#else
extern int dngl_sendpkt(struct dngl *dngl, void *src, void *p);
#endif /* BCMUSBDEV_BMAC */

extern int dngl_sendctl(struct dngl *dngl, void *src, void *p);

extern int _dngl_devioctl(struct dngl *dngl, int ifindex,
	uint32 cmd, void *buf, int len, int *used, int *needed, bool set);
extern bool dngl_get_netif_stats(struct dngl *dngl, dngl_stats_t *stats);
extern ulong dngl_get_netif_mtu(struct dngl *dngl);
extern void dngl_get_stats(struct dngl *dngl, dngl_stats_t *stats);
extern void *dngl_proto(struct dngl *dngl);
extern int dngl_flowring_update(struct dngl *dngl, uint8 ifindex, uint16 flowid,
	uint8 op, uint8 * sa, uint8 *da, uint8 tid);
#ifdef FLASH_UPGRADE
extern int dngl_upgrade(struct dngl *dngl, uchar *buf, uint len);
extern int dngl_upgrade_status(struct dngl *dngl);
#endif

#ifdef RSOCK
extern int dngl_sendslave(struct dngl *dngl, void *p);
extern int dngl_sendbus(struct dngl *dngl, void *p);
#endif

#ifdef BCM_FD_AGGR
extern void dngl_sendup_aggr(struct dngl *dngl, void *p);
extern int dngl_sendpkt_aggr(struct dngl *dngl, void *p);
#endif
/* Simple ioctl() call */
#define dngl_dev_ioctl(dngl, cmd, buf, len) \
	_dngl_devioctl((dngl), 0, (cmd), (buf), (len), NULL, NULL, TRUE)

/* Relay OID request through ioctl() */
#define dngl_dev_query_oid(dngl, ifindex, cmd, buf, len, written, needed) \
	_dngl_devioctl((dngl), (ifindex), (cmd), (buf), (len), (written), (needed), FALSE)
#define dngl_dev_set_oid(dngl, ifindex, cmd, buf, len, read, needed) \
	_dngl_devioctl((dngl), (ifindex), (cmd), (buf), (len), (read), (needed), TRUE)

#endif /* _dngl_api_h_ */
