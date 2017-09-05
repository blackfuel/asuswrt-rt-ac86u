/*
 * Linux Visualization System common shared file's header
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: vis_shared_util.h 555336 2015-05-08 09:52:04Z $
 */

#ifndef _VIS_SHARED_UTIL_H_
#define _VIS_SHARED_UTIL_H_

#include <stdio.h>
#include <string.h>
#include "vis_struct.h"

extern iovar_handler_t* find_iovar(const char *name);

extern int get_iovar_count();

extern iovar_handler_t* get_iovar_handler(int idx);

#endif /* _VIS_SHARED_UTIL_H_ */
