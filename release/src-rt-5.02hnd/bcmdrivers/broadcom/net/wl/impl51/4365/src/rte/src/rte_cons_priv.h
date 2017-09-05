/*
 * Console support for RTE.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_cons_priv.h 480442 2014-05-23 20:23:30Z $
 */
#ifndef	_rte_cons_priv_h_
#define	_rte_cons_priv_h_

#include <typedefs.h>
#include <osl_decl.h>
#include <siutils.h>
#include <hnd_cons.h>

/* init/attach */
extern hnd_cons_t *hnd_cons_init(si_t *sih, osl_t *osh);
extern int hnd_cons_log_init(osl_t *osh);

#endif /* _rte_cons_priv_h_ */
