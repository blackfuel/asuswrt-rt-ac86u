/*
 * Linux Visualization Data Concentrator database synchronization header
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
 * $Id: vis_synchdb.h 555336 2015-05-08 09:52:04Z $
 */

#ifndef _VIS_SYNCH_DB_H_
#define _VIS_SYNCH_DB_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "vis_struct.h"

extern int initialize_db_mutexes();

extern void uninitialize_db_mutexes();

extern int is_mutex_present(char *tablename);

extern int add_mutex(char *tablename);

extern int lock_db_mutex(char *tablename, int *index);

extern int unlock_db_mutex(char *tablename, int index);

extern int initialize_config_mutex();

extern int uninitialize_config_mutex();

extern int lock_config_mutex();

extern int unlock_config_mutex();

extern int initialize_alldutsettings_mutex();

extern int uninitialize_alldutsettings_mutex();

extern int lock_alldutsettings_mutex();

extern int unlock_alldutsettings_mutex();

#endif /* _VIS_SYNCH_DB_H_ */
