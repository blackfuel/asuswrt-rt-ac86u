/*
 * ATE restruct.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wl_ate.h kshaha $
 */
#ifndef _wl_ate_h_
#define _wl_ate_h_

#include <siutils.h>
#include <rte_dev.h>

/* Buffer of size WLC_SAMPLECOLLECT_MAXLEN (=10240 for 4345a0 ACPHY)
 * gets copied to this, multiple times
 */
#define ATE_SAMPLE_COLLECT_BUFFER_SIZE	(128*1024)

#define ATE_OTP_REGVAL_SIZE     96

/* Externally used ATE  data structures */
typedef struct _ate_params {
	void*	wl;
	si_t*	sih;
	uint8	gpio_input;
	uint8	gpio_output;
	bool	cmd_proceed;
	uint16	cmd_idx;
	bool	ate_cmd_done;
} ate_params_t;

/* Buffer defn for storing various register values */
typedef struct {
	uint32 gmult_lpf;       /* RCCal lpf result */
	uint32 gmult_adc;       /* RCCal adc result */
	uint32 rccal_dacbuf;    /* RCCal dacbuf result */
	uint32 curr_radio_temp; /* The current radio temperature */
	uint32 rcal_value;	/* the rcal_cfg_north.rcal_value in radioreg offset 0x78 */
	uint32 minipmu_cal;     /* minipmu_cal */
} ate_buffer_regval_t;

extern ate_params_t ate_params;
extern char ate_buffer_sc[];
extern uint32 ate_buffer_sc_size;
extern ate_buffer_regval_t ate_buffer_regval;
extern void cpu_flush_cache_all(void);
extern uint16 otp_regval[ATE_OTP_REGVAL_SIZE];

/* Externally used ATE functions */
void wl_ate_cmd_proc(void);
void wl_ate_init(si_t *sih, hnd_dev_t *bcmwl);

#endif /* _wl_ate_h_ */
