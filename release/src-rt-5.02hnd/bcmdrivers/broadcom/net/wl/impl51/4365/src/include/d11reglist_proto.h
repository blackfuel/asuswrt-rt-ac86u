/* D11reglist prototype for Broadcom 802.11abgn
 * Networking Adapter Device Drivers.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: d11reglist_proto.h 599445 2015-11-13 20:32:54Z $
 */
#ifndef _d11reglist_proto_h_
#define _d11reglist_proto_h_

/* this is for dump_mac */
enum {
	D11REG_TYPE_IHR16  = 0,
	D11REG_TYPE_IHR32  = 1,
	D11REG_TYPE_SCR    = 2,
	D11REG_TYPE_SHM    = 3,
	D11REG_TYPE_TPL    = 4,
	D11REG_TYPE_GE64   = 5,
	D11REG_TYPE_KEYTB  = D11REG_TYPE_GE64,
	D11REG_TYPE_IHRX16 = 6,
	D11REG_TYPE_SCRX   = 7,
	D11REG_TYPE_SHMX   = 8,
	D11REG_TYPE_MAX    = 9
};

#define D11REGTYPENAME {		\
	"ihr", "ihr", "scr", "shm",	\
	"tpl", "keytb", "ihrx", "scrx",	\
	"shmx"				\
}

typedef struct _d11regs_bmp_list {
	uint8 type;
	uint16 addr;
	uint32 bitmap;
	uint8 step;
	uint16 cnt; /* can be used together with bitmap or by itself */
} d11regs_list_t;

#define D11REG_BLK_SIZE		32
typedef struct _d11regs_addr_list {
	uint8 type;
	uint16 cnt;
	uint16 addr[D11REG_BLK_SIZE]; /* allow up to 32 per list */
} d11regs_addr_t;

#endif /* _d11reglist_proto_h_ */
