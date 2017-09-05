/*
 * HND RTE chipcommon support.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_chipc.h 480442 2014-05-23 20:23:30Z $
 */

#ifndef _rte_chipc_h_
#define _rte_chipc_h_

#include <typedefs.h>
#include <siutils.h>

typedef	void (*cc_isr_fn)(void* cbdata, uint32 ccintst);
extern bool si_cc_register_isr(si_t *sih, cc_isr_fn isr, uint32 ccintmask, void *cbdata);

typedef	void (*cc_dpc_fn)(void* cbdata, uint32 ccintst);
extern bool si_cc_register_dpc(si_t *sih, cc_dpc_fn dpc, uint32 ccintmask, void *cbdata);

#endif /* _rte_chipc_h_ */
