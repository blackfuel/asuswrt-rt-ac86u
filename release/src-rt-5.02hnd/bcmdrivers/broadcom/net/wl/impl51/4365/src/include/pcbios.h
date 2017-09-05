/*
 *  PCBIOS OS Environment Layer
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: pcbios.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _pcbios_h_
#define _pcbios_h_


/* ACPI system timer related  */

#define ACPITIMER_TICKS_PER_MS	3580	/* 3579.7 ticks per ms */
#define ACPITIMER_TICKS_PER_US	4	/* 3.579 ticks pwr us */
#define MAX_ACPITIMER_TICKCOUNT 0x00FFFFFF /* 24 bit */


#endif	/* _pcbios_h_ */
