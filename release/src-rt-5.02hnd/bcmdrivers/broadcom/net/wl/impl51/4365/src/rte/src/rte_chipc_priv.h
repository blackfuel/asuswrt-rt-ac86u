/*
 * RTE chipcommon support.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_chipc_priv.h 493125 2014-07-25 03:34:23Z $
 */

#ifndef _rte_chipc_priv_h_
#define _rte_chipc_priv_h_

#include <typedefs.h>
#include <siutils.h>
#include <sbchipc.h>

/* chipcommon top level init */
void hnd_chipc_init(si_t *sih);

/* init gpio module */
int rte_gpio_init(si_t *sih);
/* init gci module */
void hnd_gci_init(si_t *sih);
/* init eci module */
void hnd_eci_init(si_t *sih);

/* Forward declaration */
extern chipcregs_t *hnd_ccr;	/* Chipc core regs */

#endif /* _rte_chipc_priv_h_ */
