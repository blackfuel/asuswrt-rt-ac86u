/*
 * JTAG access interface for drivers
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: bcmjtag.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _bcmjtag_h_
#define _bcmjtag_h_

/*
* Simple high level framework that provides easier driver integration.
*/
/* driver info struct */
typedef struct {
	/* attach to device */
	void *(*attach)(uint16 venid, uint16 devid, void *regsva, void *param);
	/* detach from device */
	void (*detach)(void *ch);
	/* poll device */
	void (*poll)(void *ch);
} bcmjtag_driver_t;

/* platform specific/high level functions */
extern int bcmjtag_register(bcmjtag_driver_t *driver);
extern void bcmjtag_unregister(void);

/*
* More sophisticated low level functions for flexible driver integration.
*/
/* forward declaration */
typedef struct bcmjtag_info bcmjtag_info_t;

/* common/low level functions */
extern bcmjtag_info_t *bcmjtag_attach(osl_t *osh,
                                      uint16 jtmvendorid, uint16 jtmdevid,
                                      void *jtmregs, uint jtmbustype,
                                      void *btparam, bool diffend);
extern int bcmjtag_detach(bcmjtag_info_t *jtih);

extern uint32 bcmjtag_read(bcmjtag_info_t *jtih, uint32 addr, uint size);
extern void bcmjtag_write(bcmjtag_info_t *jtih,
	uint32 addr, uint32 val, uint size);

extern bool bcmjtag_chipmatch(uint16 vendor, uint16 device);

extern int bcmjtag_devattach(bcmjtag_info_t *jtih, uint16 venid, uint16 devid,
	bool (*dcb)(void *arg, uint16 venid, uint16 devid, void *devregs),
	void *arg);

#endif /* _bcmjtag_h_ */
