/*
 * bcmseclib_linux_timer.c -- linux platform dependent timer stuff
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: bcmseclib_linux_timer.c,v 1.2 2010-12-11 00:06:36 $
 */

#include <bcmseclib_timer.h>
#include <bcmseclib_timer_os.h>
#include <sys/time.h>


int
bcmseclib_os_get_time(bcmseclib_time_t *time)
{
	struct timeval	tv;
	int		ret;

	ret = gettimeofday(&tv, NULL);
	time->sec = tv.tv_sec;
	time->usec = tv.tv_usec;

	return (ret);
}
