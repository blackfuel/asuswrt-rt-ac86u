/*
 * Linux Visualization Data Concentrator database synchronization implementation
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
 * $Id: vis_synchdb.c 555336 2015-05-08 09:52:04Z $
 */

#include "vis_synchdb.h"
#include "vis_db_defines.h"
#include "database.h"

static mutexes_t *mutexes = NULL;

static pthread_mutex_t lock_config;

static pthread_mutex_t lock_alldutsettings;

/* Initialize the mutexes with default table names */
int
initialize_db_mutexes()
{
#ifndef USE_MUTEXES
	return 0;
#endif /* USE_MUTEXES */

	mutexes = NULL;

	/* add_mutex(TABLE_CONFIG);
	 * add_mutex(TABLE_DUTDETAILS);
	 * add_mutex(TABLE_SCAN);
	 * add_mutex(TABLE_ASSOCSTA);
	 * add_mutex(TABLE_GRAPHS);
	 */

	get_all_tablenames();

	return 0;
}

/* Uninitalize all the mutex */
void
uninitialize_db_mutexes()
{
#ifndef USE_MUTEXES
	return;
#endif /* USE_MUTEXES */
	mutexes_t *cur;

	while (mutexes != NULL) {
		cur = mutexes;
		mutexes = mutexes->next;
		pthread_mutex_destroy(&(cur->dbmutex));
		if (cur->tablename != NULL)
			free(cur->tablename);
		free(cur);
		cur = NULL;
	}
}

/* Check if the mutex for tablename present or not
 * If present return the index else return -1
 */
int
is_mutex_present(char *tablename)
{
#ifndef USE_MUTEXES
	return 0;
#endif /* USE_MUTEXES */
	int ret = -1, i = 0;
	mutexes_t *cur = mutexes;

	while (cur != NULL) {
		if (strcmp(tablename, cur->tablename) == 0) {
			VIS_SYNCH("MUTEX %s present\n", tablename);
			return i;
		}
		cur = cur->next;
		i++;
	}

	return ret;
}

/* This will add the mutex to sync_db structure and return the index */
int
add_mutex(char *tablename)
{
#ifndef USE_MUTEXES
	return 0;
#endif /* USE_MUTEXES */

	mutexes_t *tmp	= NULL;
	int idx		= 0;

	tmp = (mutexes_t*)malloc(sizeof(mutexes_t));
	if (tmp == NULL) {
		VIS_SYNCH("Failed to allocated memory for mutexes of : %d\n", 1);
		return -1;
	}
	tmp->tablename = (char*)malloc(sizeof(char) * (strlen(tablename) + 1));
	if (tmp->tablename == NULL) {
		VIS_SYNCH("Failed to allocated memory for tmp->tablename of %d\n", 1);
		free(tmp);
		return -1;
	}
	snprintf(tmp->tablename, strlen(tablename)+1, "%s", tablename);
	pthread_mutex_init(&(tmp->dbmutex), NULL);
	tmp->next = NULL;

	if (mutexes == NULL) {
		mutexes = tmp;
	} else {
		mutexes_t *cur = mutexes;

		while (cur->next != NULL) {
			cur = cur->next;
			idx++;
		}

		cur->next = tmp;
		idx++;
	}
	VIS_SYNCH("Added MUTEX %s\n", tablename);
	return idx;
}

/* Lock teh mutex */
int
lock_db_mutex(char *tablename, int *idx)
{
#ifndef USE_MUTEXES
	return 0;
#endif /* USE_MUTEXES */

	int ret = -1, i = 0;

	VIS_SYNCH("******* LOCKING %s.... *********\n", tablename);
	if (*idx >= 0) {
		mutexes_t *cur = mutexes;
		while (cur != NULL) {
			if (i == *idx) {
				ret = pthread_mutex_lock(&(cur->dbmutex));
				VIS_SYNCH("\tLOCKED %s.... ret %d\n", tablename, ret);
				break;
			}
			cur = cur->next;
			i++;
		}
	} else {
		mutexes_t *cur = mutexes;
		while (cur != NULL) {
			if (strcmp(tablename, cur->tablename) == 0) {
				ret = pthread_mutex_lock(&(cur->dbmutex));
				*idx = i;
				VIS_SYNCH("\tLOCKED %s.... ret %d\n", tablename, ret);
				break;
			}
			cur = cur->next;
			i++;
		}
	}

	return ret;
}

/* UnLock teh mutex */
int
unlock_db_mutex(char *tablename, int idx)
{
#ifndef USE_MUTEXES
	return 0;
#endif /* USE_MUTEXES */

	int ret = -1, i = 0;

	VIS_SYNCH("******* UNLOCKING %s.... ********\n", tablename);
	if (idx >= 0) {
		mutexes_t *cur = mutexes;
		while (cur != NULL) {
			if (i == idx) {
				ret = pthread_mutex_unlock(&(cur->dbmutex));
				VIS_SYNCH("UNLOCKED %s.... ret %d\n", tablename, ret);
				break;
			}
			cur = cur->next;
			i++;
		}
	} else {
		mutexes_t *cur = mutexes;
		while (cur != NULL) {
			if (strcmp(tablename, cur->tablename) == 0) {
				ret = pthread_mutex_unlock(&(cur->dbmutex));
				VIS_SYNCH("UNLOCKED %s.... ret %d\n", tablename, ret);
				break;
			}
			cur = cur->next;
		}
	}

	return ret;
}

/* initializes mutex for global config structure */
int
initialize_config_mutex()
{
	pthread_mutex_init(&lock_config, NULL);

	return 0;
}

/* uninitializes mutex for global config structure */
int
uninitialize_config_mutex()
{
	pthread_mutex_destroy(&lock_config);

	return 0;
}

/* lock mutex for global config structure */
int
lock_config_mutex()
{
	return pthread_mutex_lock(&lock_config);
}

/* unlock mutex for global config structure */
int
unlock_config_mutex()
{
	return pthread_mutex_unlock(&lock_config);
}

/* initializes mutex for global all dut setttings structure */
int
initialize_alldutsettings_mutex()
{
	pthread_mutex_init(&lock_alldutsettings, NULL);

	return 0;
}

/* uninitializes mutex for global all dut setttings structure */
int
uninitialize_alldutsettings_mutex()
{
	pthread_mutex_destroy(&lock_alldutsettings);

	return 0;
}

/* lock mutex for global all dut setttings structure */
int
lock_alldutsettings_mutex()
{
	return pthread_mutex_lock(&lock_alldutsettings);
}

/* unlock mutex for global all dut setttings structure */
int
unlock_alldutsettings_mutex()
{
	return pthread_mutex_unlock(&lock_alldutsettings);
}
