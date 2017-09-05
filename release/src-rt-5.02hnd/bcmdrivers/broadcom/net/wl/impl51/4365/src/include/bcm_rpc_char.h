/*
 * A simple linux style char driver for device driver testing
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: bcm_rpc_char.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _bcm_rpc_char_h_
#define _bcm_rpc_char_h_

typedef struct chardev_bus chardev_bus_t;
typedef void (*chardev_rx_fn_t)(void* context, char* data, uint len);

extern chardev_bus_t* chardev_attach(osl_t *osh);
extern void chardev_detach(chardev_bus_t* cbus);
extern void chardev_register_callback(chardev_bus_t* cbus, chardev_rx_fn_t rx_data, void *rx_ctx);
extern int chardev_send(chardev_bus_t* cbus, void* data, uint len);

#endif /* _bcm_rpc_char_h_ */
