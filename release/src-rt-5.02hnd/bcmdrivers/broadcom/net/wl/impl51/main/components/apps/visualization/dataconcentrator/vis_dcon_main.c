/*
 * Linux Visualization Data Concentrator
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
 * $Id: vis_dcon_main.c 550658 2015-04-21 09:12:23Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#ifndef TARGETENV_android
#include <error.h>
#endif /* TARGETENV_android */

#include "database.h"
#include "../include/defines.h"
#include "vis_dcon_main.h"
#include "vis_struct.h"
#include "vis_synchdb.h"
#include <sys/wait.h>
#include "vis_sockserver.h"
#include "vis_utility.h"
#include <bcmnvram.h>
#include <libxml/tree.h>

static void uninitialize_all_dutsettings();

/* Read the integer type NVRAM value */
static int
get_nvram_int_var(char *name)
{
	char *tmpflag = nvram_get(name);
	if (tmpflag)
		return (atoi(tmpflag));

	return 0;
}

static void
cleanup_thread_specific_objects()
{
	uninitialize_db_mutexes();

	uninitialize_config_mutex();

	free_graphs_list();

	uninitialize_all_dutsettings();

	close_one_connection();
}

static void
exit_signal_handler(int signum)
{
	close_server();

	cleanup_thread_specific_objects();

	xmlCleanupThreads();
	xmlCleanupParser();

	exit(signum);
}

/* Initializes the all DUT settings structure which holds the settings of all DUTs */
static void
initialize_all_dutsettings()
{
	int szalloclen = 0;

	initialize_alldutsettings_mutex();

	szalloclen = (sizeof(alldut_settings_t) + (sizeof(dut_settings_t) * (MAX_DUTS)));
	g_alldutsettings = (alldut_settings_t*)malloc(szalloclen);
	if (g_alldutsettings == NULL) {
		VIS_DEBUG("Failed to allocate g_alldutsettings of %d\n", szalloclen);
		return;
	}
	memset(g_alldutsettings, 0x00, szalloclen);
	g_alldutsettings->length = MAX_DUTS;
	g_alldutsettings->ncount = 0;
}

/* UnInitializes the all DUT settings structure which holds the settings of all DUTs */
static void
uninitialize_all_dutsettings()
{
	uninitialize_alldutsettings_mutex();

	if (g_alldutsettings != NULL) {
		free(g_alldutsettings);
		g_alldutsettings = NULL;
	}
}

static void
start_dcon()
{
	int twocpuenabled = 0;

	VIS_DEBUG("Dcon Started\n");
	twocpuenabled = get_nvram_int_var("vis_m_cpu");
	VIS_DEBUG("Two CPU Enabled NVRAM value : %d\n", twocpuenabled);

	signal(SIGINT, exit_signal_handler);
	signal(SIGTSTP, exit_signal_handler);
	signal(SIGSEGV, exit_signal_handler);
	signal(SIGTERM, exit_signal_handler);
	signal(SIGQUIT, exit_signal_handler);
	signal(SIGFPE, exit_signal_handler);

	g_alldutsettings = NULL;

	initialize_db_mutexes();

	initialize_config_mutex();

	open_one_connection();

	create_graphs_table();

	memset(&g_configs, 0x00, sizeof(configs_t));

	/* Save the default config in DB */
	g_configs.interval	= DEFAULT_INTERVAL;
	g_configs.dbsize	= DEFAULT_DBSIZE;
	g_configs.isstart	= 0;
	g_configs.isoverwrtdb = 1;
	snprintf(g_configs.gatewayip, sizeof(g_configs.gatewayip), "%s", LOOPBACK_IP);
	save_configs_in_db(&g_configs, FALSE);

	get_graphs_list_from_db();

	initialize_all_dutsettings();

	xmlInitParser();

	if (create_server(VISUALIZATION_SERVER_PORT, twocpuenabled) == 0) {
		sleep(10);
	}

	/* DO the cleanup */
	cleanup_thread_specific_objects();
}

/* Get debug level flag from nvram */
static int
get_nvram_debug_settings()
{
	char *tmpdebugflag = nvram_get("vis_debug_level");
	if (tmpdebugflag)
		vis_debug_level = strtoul(tmpdebugflag, NULL, 0);

	return 0;
}

/* Visualization Data concentrator Linux Main function */
int
main(int argc, char **argv)
{
	int status = 0;
	pid_t pid;

	if (daemon(1, 1) == -1) {
		perror("daemon");
		exit(errno);
	}

	vis_debug_level = 1;
	get_nvram_debug_settings();

	VIS_DEBUG("Debug Message Level : %d\n", vis_debug_level);
	VIS_DEBUG("\n****************************************************************\n");
	VIS_DEBUG("\tVisualization Data Concentrator Main Started\n");
	VIS_DEBUG("****************************************************************\n");

	/* Create DB directory */
	create_directory(DB_FOLDER_NAME);

	create_database_and_tables();

start:
	pid = fork();
	if (pid == 0) {
		start_dcon();
	} else if (pid < 0) {
		start_dcon();
	} else {
		if (waitpid(pid, &status, 0) != pid)
			status = -1;
		goto start;
	}
	cleanup_thread_specific_objects();

	xmlCleanupThreads();
	xmlCleanupParser();

	return 0;
}
