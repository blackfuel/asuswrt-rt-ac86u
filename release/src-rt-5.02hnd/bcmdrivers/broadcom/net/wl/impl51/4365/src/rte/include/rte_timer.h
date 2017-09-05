/*
 * HND timer interfaces.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_timer.h 474540 2014-05-01 18:40:15Z $
 */

#ifndef	_rte_timer_h_
#define	_rte_timer_h_

#include <typedefs.h>

typedef struct hnd_timer hnd_timer_t;
typedef struct hnd_timer hnd_task_t;

typedef void (*hnd_timer_mainfn_t)(hnd_timer_t *data);
typedef void (*hnd_timer_auxfn_t)(void *ctx);

/* timer primitive interfaces */
hnd_timer_t *hnd_timer_create(void *ctx, void *data,
	hnd_timer_mainfn_t mainfn, hnd_timer_auxfn_t auxfn);
void hnd_timer_free(hnd_timer_t *t);
bool hnd_timer_start(hnd_timer_t *t, uint ms, bool periodic);
bool hnd_timer_stop(hnd_timer_t *t);

/* timer accessor interfaces */
void *hnd_timer_get_data(hnd_timer_t *t);
void *hnd_timer_get_ctx(hnd_timer_t *t);
hnd_timer_auxfn_t hnd_timer_get_auxfn(hnd_timer_t *t);

#endif /* _rte_timer_h_ */
