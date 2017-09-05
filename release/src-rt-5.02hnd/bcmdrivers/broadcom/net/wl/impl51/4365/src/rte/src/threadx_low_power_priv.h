/*
 * Threadx application support.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: threadx_low_power_priv.h 480442 2014-05-23 20:23:30Z $
 */

#ifndef	_threadx_low_power_h_
#define	_threadx_low_power_h_

/* initialize low power mode */
void threadx_low_power_init(void);

/* enter low power mode */
void threadx_low_power_enter(void);

/* exit low power mode */
void threadx_low_power_exit(void);

#endif	/* _threadx_low_power_h_ */
