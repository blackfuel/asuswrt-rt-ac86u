/*
 * Named dump callback registry functions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _wlc_dump_reg_h_
#define _wlc_dump_reg_h_

#include <typedefs.h>
#include <osl.h>

/* forward declarataion */
typedef struct wlc_dump_reg_info wlc_dump_reg_info_t;

/* callback function of registry */
typedef int (*wlc_dump_reg_fn_t)(const void *ctx, void *arg);

/* create a registry with 'count' entries */
wlc_dump_reg_info_t *wlc_dump_reg_create(osl_t *osh, uint16 count);

/* destroy a registry */
void wlc_dump_reg_destroy(osl_t *osh, wlc_dump_reg_info_t *reg);

/* add a name and its callback function to a registry */
int wlc_dump_reg_add_fn(wlc_dump_reg_info_t *reg, char *name, wlc_dump_reg_fn_t fn,
	const void *ctx);

/* invoke a callback function in a registry by name */
int wlc_dump_reg_invoke_fn(wlc_dump_reg_info_t *reg, char *name, void *arg);

#endif /* _wlc_dump_reg_h_ */
