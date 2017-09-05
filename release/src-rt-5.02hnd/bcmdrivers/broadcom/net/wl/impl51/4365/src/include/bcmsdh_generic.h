/*
 * SDIO host client driver interface of Broadcom HNBU
 *     export functions to client drivers
 *     abstract OS and BUS specific details of SDIO
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: bcmsdh_generic.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef	_bcmsdh_generic_h_
#define	_bcmsdh_generic_h_

/* ---- Include Files ---------------------------------------------------- */

#include "bcmsdh.h"


/* ---- Constants and Types ---------------------------------------------- */
/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

extern void* bcmsdh_probe(osl_t *osh);
extern void bcmsdh_remove(osl_t *osh, void *instance);

#endif	/* _bcmsdh_generic_h_ */
