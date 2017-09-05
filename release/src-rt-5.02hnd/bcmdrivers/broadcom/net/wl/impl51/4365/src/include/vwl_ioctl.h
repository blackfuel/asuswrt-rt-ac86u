/*
 * NETVWL interface definitions
 * Broadcom 802.11abg Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: vwl_ioctl.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef __vwl_ioctl_h__
#define __vwl_ioctl_h__

#ifndef DD_TYPE_NETVWL
#define DD_TYPE_NETVWL  FILE_DEVICE_BUS_EXTENDER
#endif

#define IOCTL_NETVWL_BASE          0x100

#define	NETVWL_IOCTL_COOKIE	0x12349678

typedef struct {
	ULONG cookie;
	ULONG oid;
} netvwl_ioctl_hdr_t;

#define NETVWL_IOCTL_BCM_OID		CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x30, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NETVWL_IOCTL_DEVICE_CONNECT	CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x31, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NETVWL_IOCTL_DEVICE_DISCONNECT	CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x32, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NETVWL_IOCTL_DEVICE_SEND_UP	CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x33, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NETVWL_IOCTL_DEVICE_LINK	CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x34, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NETVWL_IOCTL_DEVICE_TX_COMPLETE	CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x35, METHOD_BUFFERED, FILE_ANY_ACCESS)

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN	6
#endif

typedef void (*vwl_ioctl_complete_cb_t)(void *handle, int status);

typedef int (*wl_tx_cb_t)(void *handle, void *txc_handle, void *buf, unsigned int size);
typedef int (*wl_ioctl_cb_t)(void *handle, int oid, void *buf, unsigned int size,
	vwl_ioctl_complete_cb_t fn);
typedef void (*wl_halt_cb_t)(void *handle);

typedef struct _wl_device_pars
{
	int version;  /* version of this type (currently 1) */
	void *handle; /* handle for the wl device (wl_info_t *) */
	int instance;
	wl_tx_cb_t tx_cb;
	wl_ioctl_cb_t  ioctl_cb;
	wl_halt_cb_t  halt_cb;
	UCHAR macaddr[ETHER_ADDR_LEN];
} wl_device_pars;

#endif /* __vwl_ioctl_h__ */
