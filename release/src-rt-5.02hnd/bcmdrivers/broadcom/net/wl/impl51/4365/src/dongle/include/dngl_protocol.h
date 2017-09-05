/*
 * Function prototypes for dongle bus protocol interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: dngl_protocol.h 474819 2014-05-02 10:22:36Z $
 */

#ifndef _dngl_protocol_h_
#define _dngl_protocol_h_

#define MAXMULTILIST		32	/* max # multicast addresses */

#ifdef BCMUSBDEV
extern void proto_pr46794WAR(struct dngl *dngl);
#endif

struct dngl_proto_ops_t {
	void* (*proto_attach_fn)(osl_t*, struct dngl*, struct dngl_bus *, char*, bool);
	void (*proto_detach_fn)(void*);
	void (*proto_ctrldispatch_fn)(void*, void *, uchar *);
	void* (*proto_pkt_header_push_fn)(void *proto, void *p);
	int (*proto_pkt_header_pull_fn)(void *proto, void *p);
	void (*proto_dev_event_fn)(void *proto, void *data);
};

#endif /* _dngl_protocol_h_ */
