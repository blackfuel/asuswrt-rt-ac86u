/*
 * Generic "Device" for RTEes/RTOSes (hndrte, threadx, etc.).
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_dev.h 573100 2015-07-21 23:55:14Z $
 */


#ifndef _rte_dev_h_
#define _rte_dev_h_

#include <siutils.h>

typedef struct hnd_dev_stats hnd_dev_stats_t;
typedef struct hnd_dev_ops hnd_dev_ops_t;
typedef struct hnd_dev hnd_dev_t;

/* Device stats */
struct hnd_dev_stats {
	uint32	rx_packets;		/* total packets received */
	uint32	tx_packets;		/* total packets transmitted */
	uint32	rx_bytes;		/* total bytes received */
	uint32	tx_bytes;		/* total bytes transmitted */
	uint32	rx_errors;		/* bad packets received */
	uint32	tx_errors;		/* packet transmit problems */
	uint32	rx_dropped;		/* packets dropped by dongle */
	uint32	tx_dropped;		/* packets dropped by dongle */
	uint32  multicast;		/* multicast packets received */
};

/* Device entry points */
struct hnd_dev_ops {
	void *(*probe)(hnd_dev_t *dev, void *regs, uint bus,
	               uint16 device, uint coreid, uint unit);

	/* standard */
	int (*open)(hnd_dev_t *dev);
	int (*close)(hnd_dev_t *dev);
	int (*xmit)(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lbuf);
	int (*xmit_ctl)(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lbuf);
	int (*recv)(hnd_dev_t *src, hnd_dev_t *dev, void *pkt);
	int (*ioctl)(hnd_dev_t *dev, uint32 cmd, void *buffer, int len,
	             int *used, int *needed, int set);
	void (*txflowcontrol) (hnd_dev_t *dev, bool state, int prio);

	/* optional */
	void (*poll)(hnd_dev_t *dev);

	/* extension */
	int (*xmit2)(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lbuf, uint32 ep_idx);
	void (*wowldown) (hnd_dev_t *src);
	uint32  (*flowring_link_update)(hnd_dev_t *dev, uint16 flowid, uint8 op,
	                                uint8 *sa, uint8 *da, uint8 tid);
};

#define HND_DEV_NAME_MAX	16

/* Device instance */
struct hnd_dev {
	char		name[HND_DEV_NAME_MAX];
	uint32		devid;
	uint32		flags;		/* RTEDEVFLAG_XXXX */
	hnd_dev_ops_t	*ops;
	void		*softc;		/* Software context */
	hnd_dev_t	*next;
	hnd_dev_t	*chained;
	hnd_dev_stats_t	*stats;		/* May be NULL */
	void		*commondata;
	void		*pdev;
};

/* flags */
#define RTEDEVFLAG_HOSTASLEEP	0x000000001	/* host is asleep */
#define RTEDEVFLAG_USB30	0x000000002	/* has usb 3.0 device core  */


#ifdef SBPCI
typedef struct _pdev {
	struct _pdev	*next;
	si_t		*sih;
	uint16		vendor;
	uint16		device;
	uint		bus;
	uint		slot;
	uint		func;
	void		*address;
	bool		inuse;
} pdev_t;
#endif /* SBPCI */


/* device operations */
int hnd_add_device(si_t *sih, hnd_dev_t *dev, uint16 coreid, uint16 device);
int hnd_add_d11device(si_t *sih, hnd_dev_t *dev, uint16 device);

/* query s/w object */
hnd_dev_t *hnd_get_dev(char *name);

/* poll h/w interrupt status and route them to registered isr(s) */
void hnd_dev_poll(void);
/* route h/w interrupt status to registered isr(s) */
void hnd_dev_isr(void);

#ifdef SBPCI
/* initialize PCI core */
void hnd_dev_init_pci(si_t *sih, osl_t *osh);
#endif

#endif /* _rte_dev_h_ */
