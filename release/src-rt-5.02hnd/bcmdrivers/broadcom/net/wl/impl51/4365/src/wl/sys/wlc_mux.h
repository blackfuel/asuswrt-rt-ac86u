/*
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_mux.h 497839 2014-08-20 22:58:53Z $
 */

/**
 * @file
 * @brief Transmit Path MUX layer for Broadcom 802.11 Networking Driver
 */

#ifndef _wlc_mux_h_
#define _wlc_mux_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <wlc_types.h>

/**
 * @brief State structure for the MUX module created by wlc_mux_module_attach()
 */
typedef struct wlc_mux_info wlc_mux_info_t;

/**
 * @brief State structure for an individual MUX created by wlc_mux_alloc()
 */
typedef struct wlc_mux wlc_mux_t;

/**
 * @brief The callback function a MUX source registers in wlc_mux_add_source()
 *	to provide data to the MUX
 */
typedef uint (*mux_output_fn_t)(void *ctx, uint ac, uint request_time, struct spktq *output_q);

/**
 * @brief Handle for a MUX source created by wlc_mux_add_source()
 */
typedef struct mux_source* mux_source_handle_t;

/*
 * Module Attach/Detach
 */

/**
 * @brief Allocate and initialize the MUX module.
 */
wlc_mux_info_t *wlc_mux_module_attach(wlc_info_t *wlc);

/**
 * @brief Free all resources of the MUX module
 */
void wlc_mux_module_detach(wlc_mux_info_t *muxi);

/*
 * MUX alloc/free
 */

/**
 * @brief Allocate and initialize a MUX
 */
wlc_mux_t *wlc_mux_alloc(wlc_mux_info_t *muxi, uint ac, uint quanta);

/**
 * @brief Free all resources of a MUX
 */
void wlc_mux_free(wlc_mux_t *mux);

/*
 * Configuration
 */

/**
 * @brief Set the default source output quanta in microseconds
 */
int wlc_mux_set_quanta(wlc_mux_t *mux, uint quanta);

/**
 * @brief Get the default source output quanta in microseconds
 */
uint wlc_mux_get_quanta(wlc_mux_t *mux);


/*
 * Output
 */

/**
 * @brief Request output packets from a MUX
 */
uint wlc_mux_output(wlc_mux_t *mux, uint request_time, struct spktq *output_q);


/*
 * Registration
 */

/**
 * @brief Add a new source to a MUX
 */
int wlc_mux_add_source(wlc_mux_t *mux, void *ctx, mux_output_fn_t output_fn,
                       mux_source_handle_t *phdl);

/**
 * @brief Delete a source from a MUX
 */
void wlc_mux_del_source(wlc_mux_t *mux, mux_source_handle_t hndl);

/**
 * @brief Update the output fn for a MUX source
 */
void wlc_mux_source_set_output_fn(wlc_mux_t *mux, mux_source_handle_t hndl,
                                  void *ctx, mux_output_fn_t output_fn);

/**
 * @brief Mark the source as no longer stalled
 */
void wlc_mux_source_wake(wlc_mux_t *mux, mux_source_handle_t hndl);

/**
 * @brief Mark the source as enabled so that its MUX will call the source's output function
 */
void wlc_mux_source_start(wlc_mux_t *mux, mux_source_handle_t hndl);

/**
 * @brief Mark the source as disabled so that its MUX will not call the source's output funciton
 */
void wlc_mux_source_stop(wlc_mux_t *mux, mux_source_handle_t hndl);

#endif /* _wlc_mux_h_ */
