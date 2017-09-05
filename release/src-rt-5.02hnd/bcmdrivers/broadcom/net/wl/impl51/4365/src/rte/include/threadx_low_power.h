/*
 * Threadx application support.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: threadx_low_power.h 480442 2014-05-23 20:23:30Z $
 */

#ifndef	_THREADX_LOW_POWER_H
#define	_THREADX_LOW_POWER_H

/* msec since last time update */
uint32 threadx_low_power_time_since_update(void);

/* force a h/w timer update */
void threadx_low_power_timer_update(void);

#endif	/* _THREADX_LOW_POWER_H */
