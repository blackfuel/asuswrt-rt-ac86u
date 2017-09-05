/*
 * Visualization system data concentrator database utility implementation
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
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
 * $Id: database.c 672659 2016-11-29 10:24:01Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#ifndef TARGETENV_android
#include <error.h>
#endif /* TARGETENV_android */

#include "database.h"
#include <json.h>

#ifdef SHOW_TIME_ELAPSED
#include <time.h>
#endif

#include <sys/stat.h>

#include "vis_shared_util.h"
#include "vis_synchdb.h"
#include "vis_sock_util.h"
#include "vis_utility.h"
#include "vis_dcon_main.h"

#define SQLITE_QUERY_FAIL(szErrMsg) do { if (vis_debug_level & VIS_DEBUG_DB) \
	printf("VIS-DB >>%s(%d): SQL error: %s\n", \
	__FUNCTION__, __LINE__, szErrMsg); \
	sqlite3_free(szErrMsg); \
	szErrMsg = NULL; } while (0)

/* Sta Graph Flags	Codes		Bit	Initial Val  = 1 */

#define	PKT_REQUESTED	0
#define PKT_DROPPED	1
#define	PKT_STORED	2
#define PKT_RETRIED	3
#define	PKT_ACKED	4

static int g_sta_rrm_stats_graphs_state = 31;

static sqlite3 *g_db = NULL;

static uint32 g_maxdbsize = DEFAULT_DBSIZE; /* Default DB size */

#ifdef STORE_GRAPHS_IN_LIST
static tablelist_t *g_tablelist = NULL;
#endif

extern alldut_settings_t *g_alldutsettings;

static int get_pragma_value(char *pragma, int *val);
static int is_graph_present_in_db(graphdetails_t *graphdet, iovar_handler_t *handle);
static int delete_rows_from_tables();
static json_object* add_json_for_graph_data(char **qResults, char *tabname, int rowid,
	char *mac, int nRows, int nCols, int **mainrows);
static void add_sta_adv_airtimeoppoutunity(vis_rrm_statslist_t *rrmstatslist,
	json_object **sub_data_obj1);
static json_object* add_sta_adv_rrm_stats(vis_rrm_statslist_t *rrmstatslist);
static json_object* add_sta_adv_rrm_stats_acked(vis_rrm_statslist_t *rrmstatslist);
/* Difference in time */
double
difftime(time_t e, time_t s)
{
	return (e - s);
}

/* Sets the MAX DB size to global variable */
void
set_max_db_size(int dbsize)
{
	g_maxdbsize = dbsize;
}

/* Checks the DB size and removes the older rows from all teh tables if the
 * DB size exceeds the MAX limit
 */
int
remove_rows_on_db_size_exceedes()
{
	struct stat fst;
	int freecount = 0, pagesize = 0;
	int ret = 0;

	stat(DB_FOLDER_NAME"/"DB_NAME, &fst);

	ret = get_pragma_value("PRAGMA freelist_count;", &freecount);
	if (ret != 0)
		return 0;
	ret = get_pragma_value("PRAGMA page_size;", &pagesize);
	if (ret != 0)
		return 0;

	VIS_DB("DB size : %d\t Free Count : %d\tPage Size : %d\tActual Size : %d\t MAX size : %d\n",
		(int)fst.st_size, freecount, pagesize, (int)(fst.st_size - (freecount * pagesize)),
		(g_maxdbsize * 1024 *1024));

	if ((fst.st_size - (freecount * pagesize)) >= ((g_maxdbsize * 1024 * 1024) - (1024*5))) {
		/* Check to over write the DB or stop the data collector */
		if (g_configs.isoverwrtdb == 0) { /* Stop the data collector */
			lock_config_mutex();
			g_configs.isstart = 0;
			save_configs_in_db(&g_configs, TRUE);
			unlock_config_mutex();
		} else { /* Overwrite the DB */
			delete_rows_from_tables();
		}
		return 1;
	}

	return 0;
}

/* Opens the common DB connection */
int
open_one_connection()
{
	int ret = 0;

	ret = open_database(DB_NAME, &g_db);

	return ret;
}

/* Closes the common DB connection */
int
close_one_connection()
{
	if (g_db != NULL)
		close_database(&g_db);

	return 1;
}

/* Opens the database If the database doesnot exists, creates the DB */
int
open_database(const char *dbname, sqlite3 **db)
{
	int	ret = 0;
	char	tmpdbname[100];

	snprintf(tmpdbname, sizeof(tmpdbname), "%s/%s", DB_FOLDER_NAME, dbname);

	ret = sqlite3_open_v2(tmpdbname, db,
		SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX, 0);
	if (ret) {
		VIS_DB("Can't open database: %s\n", sqlite3_errmsg(*db));
		close_database(db);
	}

	return ret;
}

/* Closes the DB connection */
int
close_database(sqlite3 **db)
{
	sqlite3_close(*db);
	*db = NULL;

	return 1;
}

/* Gets the PRAGMA value from SQLITE DB */
static int
get_pragma_value(char *pragma, int *val)
{
	int		ret = 0;
	sqlite3_stmt	*createStmt;

	ret = sqlite3_prepare(g_db, pragma, -1, &createStmt, NULL);
	if (ret) {
		VIS_DB("Failed in sqlite3_prepare: %s in %s\n", sqlite3_errmsg(g_db),
			__FUNCTION__);
		return -1;
	}

	ret = sqlite3_step(createStmt);
	if (ret != SQLITE_ROW) {
		VIS_DB("Failed in sqlite3_step: %s in %s\n", sqlite3_errmsg(g_db),
			__FUNCTION__);
		return -1;
	}

	*val = sqlite3_column_int(createStmt, 0);

	sqlite3_finalize(createStmt);

	return 0;
}

/* Frees the graph list structure */
void
free_graphs_list()
{
#ifdef STORE_GRAPHS_IN_LIST
	if (g_tablelist != NULL) {
		free(g_tablelist);
		g_tablelist = NULL;
	}
#endif /* STORE_GRAPHS_IN_LIST */
}

/* Executes sqlite query */
static int
execute_sql_statement(sqlite3 *db, char *sql)
{
	int	ret = 0;
	char	*szErrMsg = 0;

	/* printf("\n\n QUERY : %s\n\n", sql); */
	/* Execute SQL statement */
	ret = sqlite3_exec(db, sql, NULL, 0, &szErrMsg);
	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		VIS_DB("In Query : %s\n", sql);
	}

	return ret;
}

/* Creates all the required tables */
int
create_database_and_tables()
{
	sqlite3		*db = NULL;
	char		querystr[1024];
	int		rval = 0;

	SHOW_TIME_ELAPSE_START();

	rval = open_database(DB_NAME, &db);
	if (rval != SQLITE_OK)
		return -1;

	/* Create Configs table */
	snprintf(querystr, sizeof(querystr), "CREATE TABLE IF NOT EXISTS %s(%s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s TEXT, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER); ",
			TABLE_CONFIG, COL_CONFIG_ROWID,
			COL_CONFIG_INTERVAL, COL_CONFIG_DBSIZE, COL_CONFIG_ISSTART,
			COL_CONFIG_GATEWAY_IP, COL_CONFIG_ISOVERWRITEDB, COL_CONFIG_ISAUTOSTART,
			COL_CONFIG_WEEKDAYS, COL_CONFIG_FROMTM, COL_CONFIG_TOTM);

	rval = execute_sql_statement(db, querystr);

	/* Create DUT Details table */
	snprintf(querystr, sizeof(querystr),
			"CREATE TABLE IF NOT EXISTS %s(%s INTEGER PRIMARY KEY AUTOINCREMENT, "
			"%s INTEGER, %s TEXT, %s TEXT, "
			"%s TEXT, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s TEXT, "
			"%s TEXT, %s TEXT, %s TEXT, %s INTEGER, %s INTEGER); ",
			TABLE_DUTDETAILS, COL_DUT_ROWID,
			COL_DUT_ISAP, COL_DUT_BSSID, COL_DUT_SSID,
			COL_DUT_MAC, COL_DUT_CHANNEL, COL_DUT_BANDWIDTH,
			COL_DUT_RSSI, COL_DUT_CONTROLCH, COL_DUT_NOISE,
			COL_DUT_BAND, COL_DUT_SPEED, COL_DUT_STANDARDS,
			COL_DUT_MCASTRSN, COL_DUT_UCASTRSN, COL_DUT_AKMRSN,
			COL_DUT_ISENABLED, COL_DUT_ERRINFO);

	rval = execute_sql_statement(db, querystr);

	/* Create SCAN Results table */
	snprintf(querystr, sizeof(querystr),
			"CREATE TABLE IF NOT EXISTS %s(%s INTEGER, %s INTEGER, "
			"%s TEXT, %s TEXT, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s TEXT, %s TEXT, "
			"%s TEXT, %s TEXT, %s INTEGER); ",
			TABLE_SCAN, COL_SCAN_ROWID, COL_SCAN_TIME,
			COL_SCAN_SSID, COL_SCAN_BSSID, COL_SCAN_RSSI,
			COL_SCAN_CHANNEL, COL_SCAN_NOISE, COL_SCAN_BANDWIDTH,
			COL_SCAN_SPEED, COL_SCAN_STANDARDS, COL_SCAN_MCASTRSN,
			COL_SCAN_UCASTRSN, COL_SCAN_AKMRSN, COL_SCAN_CONTROLCH);

	rval = execute_sql_statement(db, querystr);

	/* Create Associated STA table */
	snprintf(querystr, sizeof(querystr), "CREATE TABLE IF NOT EXISTS %s( "
			"%s INTEGER, %s INTEGER, %s TEXT, "
			"%s INTEGER, %s INTEGER); ",
			TABLE_ASSOCSTA,
			COL_ASSOC_ROWID, COL_ASSOC_TIME, COL_ASSOC_MAC,
			COL_ASSOC_RSSI, COL_ASSOC_PHYRATE);

	rval = execute_sql_statement(db, querystr);

	/* Create Channel statistics table */
	snprintf(querystr, sizeof(querystr), "CREATE TABLE IF NOT EXISTS %s( "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER); ",
			TABLE_CHANNELSTATS,
			COL_CHSTATS_ROWID, COL_CHSTATS_TIME, COL_CHSTATS_CHANNEL,
			COL_CHSTATS_TX, COL_CHSTATS_INBSS, COL_CHSTATS_OBSS,
			COL_CHSTATS_NOCAT, COL_CHSTATS_NOPKT, COL_CHSTATS_DOZE,
			COL_CHSTATS_TXOP, COL_CHSTATS_GOODTX, COL_CHSTATS_BADTX,
			COL_CHSTATS_GLITCH, COL_CHSTATS_BADPLCP, COL_CHSTATS_KNOISE,
			COL_CHSTATS_IDLE);

	rval = execute_sql_statement(db, querystr);

	/* Create AMPDU statistics table */
	snprintf(querystr, sizeof(querystr), "CREATE TABLE IF NOT EXISTS %s( "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER); ",
			TABLE_AMPDUSTATS,
			COL_AMPDU_ROWID, COL_AMPDU_TIME, COL_AMPDU_MCS,
			COL_AMPDU_TXMCS, COL_AMPDU_TXMCSPERCENT, COL_AMPDU_TXMCSSGI,
			COL_AMPDU_TXMCSSGIPERCENT, COL_AMPDU_RXMCS, COL_AMPDU_RXMCSPERCENT,
			COL_AMPDU_RXMCSSGI, COL_AMPDU_RXMCSSGIPERCENT, COL_AMPDU_TXVHT,
			COL_AMPDU_TXVHTPER, COL_AMPDU_TXVHTPERCENT, COL_AMPDU_TXVHTSGI,
			COL_AMPDU_TXVHTSGIPER, COL_AMPDU_TXVHTSGIPERCENT, COL_AMPDU_RXVHT,
			COL_AMPDU_RXVHTPER, COL_AMPDU_RXVHTPERCENT, COL_AMPDU_RXVHTSGI,
			COL_AMPDU_RXVHTSGIPER, COL_AMPDU_RXVHTSGIPERCENT, COL_AMPDU_MPDUDENS,
			COL_AMPDU_MPDUDENSPERCENT);

	rval = execute_sql_statement(db, querystr);

	/* Create Graphs table */
	snprintf(querystr, sizeof(querystr),
			"CREATE TABLE IF NOT EXISTS %s(%s INTEGER PRIMARY KEY AUTOINCREMENT, "
			"%s TEXT, %s TEXT, %s TEXT, "
			"%s TEXT, %s INTEGER, %s INTEGER, "
			"%s TEXT, %s TEXT, %s TEXT, "
			"%s TEXT, %s INTEGER); ",
			TABLE_GRAPHS, COL_GRAPH_ROWID,
			COL_GRAPH_TAB, COL_GRAPH_NAME, COL_GRAPH_HEADING,
			COL_GRAPH_PLOTWITH, COL_GRAPH_TYPE, COL_GRAPH_PERSTA,
			COL_GRAPH_BARHEADING, COL_GRAPH_XAXISNAME, COL_GRAPH_YAXISNAME,
			COL_GRAPH_TABLE, COL_GRAPH_ENABLE);

	rval = execute_sql_statement(db, querystr);

	/* Create DUT settings table */
	snprintf(querystr, sizeof(querystr), "CREATE TABLE IF NOT EXISTS %s( "
			"%s INTEGER, %s INTEGER); ",
			TABLE_DUTSETTINGS,
			COL_DUTSET_ROWID, COL_DUTSET_SCAN);

	rval = execute_sql_statement(db, querystr);

	/* Create RRM STA Side statistics table */
	snprintf(querystr, sizeof(querystr), "CREATE TABLE IF NOT EXISTS %s( "
			"%s INTEGER, %s TEXT, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER); ",
			TABLE_RRM_STA_STATS,
			COL_RRMSTATS_ROWID, COL_RRMSTATS_MAC, COL_RRMSTATS_TIME,
			COL_RRMSTATS_GENERATION, COL_RRMSTATS_TXSTREAM, COL_RRMSTATS_RXSTREAM,
			COL_RRMSTATS_RATE, COL_RRMSTATS_IDLE);

	rval = execute_sql_statement(db, querystr);

	/* Create RRM Advanced STA Side statistics table */
	snprintf(querystr, sizeof(querystr), "CREATE TABLE IF NOT EXISTS %s( "
			"%s INTEGER, %s TEXT, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER, "
			"%s INTEGER, %s INTEGER, %s INTEGER); ",
			TABLE_RRM_STA_ADV_STATS,
			COL_RRMADVSTATS_ROWID, COL_RRMADVSTATS_MAC, COL_RRMADVSTATS_TIME,
			COL_RRMADVSTATS_TXOP, COL_RRMADVSTATS_PKTREQ, COL_RRMADVSTATS_PKTDROP,
			COL_RRMADVSTATS_PKTSTORED, COL_RRMADVSTATS_PKTRETRIED,
			COL_RRMADVSTATS_PKTACKED);

	rval = execute_sql_statement(db, querystr);

	close_database(&db);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Creates all the tables required to store all the graphs details */
int
create_graphs_table()
{
	char	querystr[1024];
	int	rval = 0, i = 0, count = 0;

	SHOW_TIME_ELAPSE_START();

	count = get_iovar_count();
	for (i = 0; i < count; i++) {
		iovar_handler_t *handle = get_iovar_handler(i);
		graphdetails_t graphdet;

		/* In external build don't plot few graphs like AMPDU stats(not available) */
		if (handle->isplotinexternal == 0)
			continue;

		memset(&graphdet, 0x00, sizeof(graphdetails_t));

		fill_graph_details_struct(&graphdet, handle);

		/* Now get the rowID of the inserted element */
		is_graph_present_in_db(&graphdet, handle);

		if (graphdet.rowid <= 0) {
			snprintf(querystr, sizeof(querystr), "INSERT INTO %s (%s, %s, %s, "
				"%s, %s, %s, "
				"%s, %s, %s, "
				"%s, %s, %s) "
				"VALUES (NULL, '%s', '%s', "
				"'%s', '%s', '%d', "
				"'%d', '%s', '%s', "
				"'%s', '%s', '1'); ",
				TABLE_GRAPHS, COL_GRAPH_ROWID, COL_GRAPH_TAB, COL_GRAPH_NAME,
				COL_GRAPH_HEADING, COL_GRAPH_PLOTWITH, COL_GRAPH_TYPE,
				COL_GRAPH_PERSTA, COL_GRAPH_BARHEADING, COL_GRAPH_XAXISNAME,
				COL_GRAPH_YAXISNAME, COL_GRAPH_TABLE, COL_GRAPH_ENABLE,
				graphdet.tab, handle->name,
				handle->heading, handle->plotwith, handle->graphtype,
				handle->perSTA, handle->barheading, handle->xaxisname,
				handle->yaxisname, graphdet.table);

			rval = execute_sql_statement(g_db, querystr);
		}

		/* As the tables are already created for channel, ampdu stats
		 * and chanim statistics, Dont create again
		 */
		if (strcmp(handle->heading, "Channel Statistics") == 0 ||
			strcmp(handle->heading, "AMPDU Statistics") == 0 ||
			strcmp(handle->heading, IOVAR_NAME_CHANIM) == 0)
			continue;
		/* Now create the table */
		if (handle->perSTA == 1) {
			snprintf(querystr, sizeof(querystr),
					"CREATE TABLE IF NOT EXISTS %s(%s INTEGER, "
					"%s INTEGER, %s TEXT, %s INTEGER, "
					"%s TEXT, %s TEXT); ",
					graphdet.table, COL_CGRAPH_ROWID,
					COL_CGRAPH_GRAPHROWID, COL_CGRAPH_MAC, COL_CGRAPH_TIME,
					COL_CGRAPH_XAXIS, COL_CGRAPH_YAXIS);
		} else {
			snprintf(querystr, sizeof(querystr),
					"CREATE TABLE IF NOT EXISTS %s(%s INTEGER, "
					"%s INTEGER, %s INTEGER, "
					"%s TEXT, %s TEXT); ",
					graphdet.table, COL_CGRAPH_ROWID,
					COL_CGRAPH_GRAPHROWID, COL_CGRAPH_TIME,
					COL_CGRAPH_XAXIS, COL_CGRAPH_YAXIS);
		}
		rval = execute_sql_statement(g_db, querystr);
	}

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Get all the already created table names from DB */
int
get_all_tablenames()
{
	sqlite3		*db = NULL;
	char		*szErrMsg = NULL;
	char		**qResults = NULL;
	char		querystr[256];
	int		i = 0, ret = 0, nRows = 0, nCols = 0;

	SHOW_TIME_ELAPSE_START();

	ret = open_database(DB_NAME, &db);
	if (ret != SQLITE_OK)
		goto end;

	snprintf(querystr, sizeof(querystr), "SELECT name FROM sqlite_master "
		"WHERE type IN ('table', 'view'); ");

	ret = sqlite3_get_table(db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}

	for (i = 0; i < nRows; i++) {
		if (strstr(qResults[(i+1)*nCols], "sqlite_") == NULL)
			add_mutex(qResults[(i+1)*nCols]);
	}

	sqlite3_free_table(qResults);

end:
	close_database(&db);

	SHOW_TIME_ELAPSE_END();

	return ret;
}

/* Get the config settings from config table */
int
get_config_from_db(configs_t *configs)
{
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	char	querystr[512];
	int	i = 0, nRows = 0, nCols = 0, ret = 0, idx = -1;

	SHOW_TIME_ELAPSE_START();

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, %s, %s, %s, %s, %s, %s FROM %s",
		COL_CONFIG_INTERVAL, COL_CONFIG_DBSIZE, COL_CONFIG_ISSTART,
		COL_CONFIG_GATEWAY_IP, COL_CONFIG_ISOVERWRITEDB, COL_CONFIG_ISAUTOSTART,
		COL_CONFIG_WEEKDAYS, COL_CONFIG_FROMTM, COL_CONFIG_TOTM, TABLE_CONFIG);

	lock_db_mutex(TABLE_CONFIG, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	unlock_db_mutex(TABLE_CONFIG, idx);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}

	if (nRows > 0) {
		i = 0;
		configs->interval = atoi(qResults[(i+1)*nCols]);
		configs->dbsize = atoi(qResults[(i+1)*nCols+1]);
		configs->isstart = atoi(qResults[(i+1)*nCols+2]);
		snprintf(configs->gatewayip, sizeof(configs->gatewayip), "%s",
			qResults[(i+1)*nCols+3]);
		configs->isoverwrtdb = atoi(qResults[(i+1)*nCols+4]);
		configs->isautostart = atoi(qResults[(i+1)*nCols+5]);
		configs->weekdays = atoi(qResults[(i+1)*nCols+6]);
		configs->fromtm = atoi(qResults[(i+1)*nCols+7]);
		configs->totm = atoi(qResults[(i+1)*nCols+8]);

		VIS_DB("Interval : %d\tDBSIZE : %d\t ISSTART : %d\t IS Overwrite : %d\n"
			"Is Auto Start : %d\tWeekDays : %d\tFrom : %d\tTo : %d\n\n",
			configs->interval, configs->dbsize, configs->isstart, configs->isoverwrtdb,
			configs->isautostart, configs->weekdays, configs->fromtm, configs->totm);
	}

	sqlite3_free_table(qResults);

end:

	SHOW_TIME_ELAPSE_END();

	return 0;
}

/* Get site survey details from table */
networks_list_t*
get_networks_from_db(int rowid, int band)
{
	char			*szErrMsg = NULL;
	char			**qResults = NULL;
	char			querystr[512];
	int			i = 0, nRows = 0, nCols = 0, ret = 0, idx = -1, tmptimestamp = 0;
	networks_list_t		*networks_list = NULL;

	SHOW_TIME_ELAPSE_START();

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, "
		"%s "
		"FROM %s "
		"WHERE %s = '%d' AND %s != '0' "
		"AND %s = (SELECT %s FROM %s WHERE %s = '%d' "
		"ORDER BY %s DESC LIMIT 1) ORDER BY %s DESC; ",
		COL_SCAN_SSID, COL_SCAN_BSSID, COL_SCAN_RSSI,
		COL_SCAN_NOISE, COL_SCAN_CHANNEL, COL_SCAN_BANDWIDTH,
		COL_SCAN_SPEED, COL_SCAN_STANDARDS, COL_SCAN_MCASTRSN,
		COL_SCAN_UCASTRSN, COL_SCAN_AKMRSN, COL_SCAN_CONTROLCH,
		COL_SCAN_TIME,
		TABLE_SCAN, COL_SCAN_ROWID, rowid,
		COL_SCAN_BSSID,
		COL_SCAN_TIME, COL_SCAN_TIME, TABLE_SCAN,
		COL_SCAN_ROWID, rowid, COL_SCAN_TIME, COL_SCAN_RSSI);

	lock_db_mutex(TABLE_SCAN, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	unlock_db_mutex(TABLE_SCAN, idx);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		return NULL;
	}
	networks_list = (networks_list_t*)malloc(sizeof(networks_list_t) +
		(sizeof(ap_info_t) * nRows));
	if (networks_list == NULL) {
		VIS_DB("%s : Failed to allocate networks_list buffer for %d rows\n",
			__FUNCTION__, nRows);
		sqlite3_free_table(qResults);
		return NULL;
	}
	networks_list->length = nRows;

	for (i = 0; i < nRows; i++) {
		snprintf(networks_list->aps[i].ssid, sizeof(networks_list->aps[i].ssid), "%s",
			qResults[(i+1)*nCols]);
		snprintf(networks_list->aps[i].bssid, sizeof(networks_list->aps[i].bssid), "%s",
			qResults[(i+1)*nCols+1]);
		networks_list->aps[i].rssi = atoi(qResults[(i+1)*nCols+2]);
		networks_list->aps[i].noise = atoi(qResults[(i+1)*nCols+3]);
		networks_list->aps[i].channel = atoi(qResults[(i+1)*nCols+4]);
		if (networks_list->aps[i].channel < 14)
			networks_list->aps[i].band = 2;
		else
			networks_list->aps[i].band = 5;
		networks_list->aps[i].bandwidth = atoi(qResults[(i+1)*nCols+5]);
		networks_list->aps[i].maxrate = atoi(qResults[(i+1)*nCols+6]);
		snprintf(networks_list->aps[i].networktype,
			sizeof(networks_list->aps[i].networktype), "%s", qResults[(i+1)*nCols+7]);
		snprintf(networks_list->aps[i].mcastrsntype,
			sizeof(networks_list->aps[i].mcastrsntype), "%s", qResults[(i+1)*nCols+8]);
		snprintf(networks_list->aps[i].ucastrsntype,
			sizeof(networks_list->aps[i].ucastrsntype), "%s", qResults[(i+1)*nCols+9]);
		snprintf(networks_list->aps[i].akmrsntype,
			sizeof(networks_list->aps[i].akmrsntype), "%s", qResults[(i+1)*nCols+10]);
		networks_list->aps[i].ctrlch = atoi(qResults[(i+1)*nCols+11]);
		tmptimestamp = atoi(qResults[(i+1)*nCols+12]);

		/* Printing for debugging only */
		VIS_DB("SSID : %s\n", qResults[(i+1)*nCols]);
		VIS_DB("BSSID : %s\n", qResults[(i+1)*nCols+1]);
		VIS_DB("RSSI : %d\n", atoi(qResults[(i+1)*nCols+2]));
		VIS_DB("Noise : %d\n", atoi(qResults[(i+1)*nCols+3]));
		VIS_DB("Channel : %d\n", atoi(qResults[(i+1)*nCols+4]));
		VIS_DB("Bandwidth : %d\n", atoi(qResults[(i+1)*nCols+5]));
		VIS_DB("Speed : %d\n", atoi(qResults[(i+1)*nCols+6]));
		VIS_DB("Netrowk Type : %s\n", qResults[(i+1)*nCols+7]);
		VIS_DB("Multicast RSN : %s\n", qResults[(i+1)*nCols+8]);
		VIS_DB("Unicast RSN : %s\n", qResults[(i+1)*nCols+9]);
		VIS_DB("AKM : %s\n", qResults[(i+1)*nCols+10]);
		VIS_DB("Control Channel : %d\n", atoi(qResults[(i+1)*nCols+11]));
	}
	if (tmptimestamp == 0) {
		srand(time(NULL));
		tmptimestamp = rand();
	}
	networks_list->timestamp = tmptimestamp;

	sqlite3_free_table(qResults);

	SHOW_TIME_ELAPSE_END();

	return networks_list;
}

/* Get graph details from table and creates and stores it in json object */
static int
get_graph_details_from_db(char *tablename, int growid, int rowid, int type,
	int persta, json_object **array_x, json_object **array_y, int index, char *tabname,
	char *mac)
{
	char		*szErrMsg = NULL;
	char		**qResults = NULL;
	char		querystr[512] = "";
	int		i = 0, nRows = 0, nCols = 0, ret = 0, idx = -1;
	char		condition[256];

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s",
		COL_CGRAPH_TIME, COL_CGRAPH_XAXIS, COL_CGRAPH_YAXIS);

	if (persta) {
		snprintf(condition, sizeof(condition),
			" FROM %s WHERE %s = '%d' AND %s = '%d' AND %s = '%s'",
			tablename, COL_CGRAPH_ROWID, rowid, COL_CGRAPH_GRAPHROWID, growid,
			COL_CGRAPH_MAC, mac);
	} else {
		snprintf(condition, sizeof(condition), " FROM %s WHERE %s = '%d' AND %s = '%d'",
			tablename, COL_CGRAPH_ROWID, rowid, COL_CGRAPH_GRAPHROWID, growid);
	}

	strncat(querystr, condition, sizeof(querystr) - strlen(querystr) - 1);

	/* If it is against time get the latest 10 entries */
	if (type == GRAPH_TYPE_BAR_AGAINST_TIME || type == GRAPH_TYPE_LINE_AGAINST_TIME) {
		snprintf(condition, sizeof(condition), " ORDER BY %s DESC LIMIT %d;",
			COL_CGRAPH_TIME, MAX_GRAPH_ENTRY);
	} else { /* Get all the entries for the latest time */
		snprintf(condition, sizeof(condition),
			" AND %s = (SELECT %s FROM %s WHERE %s = '%d' AND %s = '%d' "
			"ORDER BY %s DESC LIMIT 1);",
			COL_CGRAPH_TIME, COL_CGRAPH_TIME, tablename,
			COL_CGRAPH_ROWID, rowid, COL_CGRAPH_GRAPHROWID, growid,
			COL_CGRAPH_TIME);
	}
	strncat(querystr, condition, sizeof(querystr) - strlen(querystr) - 1);
	lock_db_mutex(tablename, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		unlock_db_mutex(tablename, idx);
		return -1;
	}

	for (i = 0; i < nRows; i++) {
		/* print for debug purpose only */
		VIS_DB("Time : %d\n", atoi(qResults[(i+1)*nCols]));
		VIS_DB("X Axis : %s\n", qResults[(i+1)*nCols+1]);
		VIS_DB("Y Axis : %s\n", (qResults[(i+1)*nCols+2]));
		if (index == 0) { /* The x-axis is only for first time */
			json_object_array_add(*array_x,
				json_object_new_int(atoi(qResults[(i+1)*nCols+1])));
		}
		json_object_array_add(*array_y, json_object_new_string((qResults[(i+1)*nCols+2])));
	}

	sqlite3_free_table(qResults);
	unlock_db_mutex(tablename, idx);

	return 0;
}

/* Gets the graph data for WEB UI from respective table based on graph name and creates
 * JSON format and sends it to WEB server
 */
int
get_tab_data_from_db(int sockfd, char *tabname, int rowid, char *mac)
{
	char		*szErrMsg = NULL;
	char		**qResults = NULL;
	char		querystr[512];
	int		i = 0, j = 0, k = 0, nRows = 0, nCols = 0, ret = 0, idx = -1, nmainrows = 0;
	int		placeholder = 1, found = 0;
	json_object	*object;
	json_object	*array;
	json_object	*array_x, *array_y_main, *array_graph, *array_main;
	int *mainrows = NULL;

	SHOW_TIME_ELAPSE_START();

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, "
		"%s FROM %s "
		"WHERE %s = '1' AND (%s = '%s' OR %s = '%s' OR %s = '%s'); ",
		COL_GRAPH_ROWID, COL_GRAPH_NAME, COL_GRAPH_HEADING,
		COL_GRAPH_PLOTWITH, COL_GRAPH_TYPE, COL_GRAPH_PERSTA,
		COL_GRAPH_BARHEADING, COL_GRAPH_XAXISNAME, COL_GRAPH_YAXISNAME,
		COL_GRAPH_TABLE, TABLE_GRAPHS, COL_GRAPH_ENABLE,
		COL_GRAPH_NAME, tabname, COL_GRAPH_PLOTWITH, tabname,
		COL_GRAPH_HEADING, tabname);

	lock_db_mutex(TABLE_GRAPHS, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		unlock_db_mutex(TABLE_GRAPHS, idx);
		return -1;
	}
	/* This holds the index values of graphs which are already added to json */
	if (nRows > 0) {
		mainrows = (int*)malloc(sizeof(int) * nRows);
		if (mainrows ==  NULL) {
			VIS_DB("Failed to allocate memory to mainrows of %d\n", nRows);
			sqlite3_free_table(qResults);
			unlock_db_mutex(TABLE_GRAPHS, idx);
			return -1;
		}
		memset(mainrows, 0x00, nRows);
	}

	array_main = json_object_new_array();
	array_graph = json_object_new_array();
	for (i = 0; i < nRows; i++) {
		int selrows[10] = {0};
		int selrowscount = 0;

		/* check if the graph row is already added to json */
		found = 0;
		for (k = 0; k < nmainrows; k++) {
			if (mainrows[k] == i) {
				found = 1;
				break;
			}
		}
		if (found == 1)
			continue;
		object = json_object_new_object();

		json_object_object_add(object, "Name",
			json_object_new_string(qResults[(i+1)*nCols+1]));
		json_object_object_add(object, "Heading",
			json_object_new_string(qResults[(i+1)*nCols+2]));
		json_object_object_add(object, "PlotWith",
			json_object_new_string(qResults[(i+1)*nCols+3]));
		json_object_object_add(object, "GraphType",
			json_object_new_int(atoi(qResults[(i+1)*nCols+4])));
		json_object_object_add(object, "PerSTA",
			json_object_new_int(atoi(qResults[(i+1)*nCols+5])));

		/* Add the bar heading */
		array = json_object_new_array();
		for (j = i; j < nRows; j++) {
			if (strcmp(qResults[(i+1)*nCols+3], qResults[(j+1)*nCols+3]) == 0) {
				json_object_array_add(array,
					json_object_new_string((qResults[(j+1)*nCols+6])));
				selrows[selrowscount++] = j;
				mainrows[nmainrows++] = j;
			}
		}
		json_object_object_add(object, "BarHeading", array);
		json_object_object_add(object, "XAxis",
			json_object_new_string(qResults[(i+1)*nCols+7]));
		json_object_object_add(object, "YAxis",
			json_object_new_string(qResults[(i+1)*nCols+8]));
		json_object_object_add(object, "Total",
			json_object_new_int(selrowscount));
		placeholder = atoi(qResults[(i+1)*nCols]);
		json_object_object_add(object, "Placeholderid",
			json_object_new_int(placeholder));

		/* Print for debug purpose only */
		VIS_DB("rowid : %d\n", atoi(qResults[(i+1)*nCols]));
		VIS_DB("Name : %s\n", qResults[(i+1)*nCols+1]);
		VIS_DB("Heading : %s\n", (qResults[(i+1)*nCols+2]));
		VIS_DB("Plotwith : %s\n", (qResults[(i+1)*nCols+3]));
		VIS_DB("Graph Type : %d\n", atoi(qResults[(i+1)*nCols+4]));
		VIS_DB("Per STA : %d\n", atoi(qResults[(i+1)*nCols+5]));
		VIS_DB("Bar Heading : %s\n", (qResults[(i+1)*nCols+6]));
		VIS_DB("X Axis : %s\n", (qResults[(i+1)*nCols+7]));
		VIS_DB("Y Axis : %s\n", qResults[(i+1)*nCols+8]);
		VIS_DB("Table : %s\n", qResults[(i+1)*nCols+9]);

		array_x = json_object_new_array();

		array_y_main = json_object_new_array();

		/* For every graph add the xaxis and yaxis values */
		for (j = 0; j < selrowscount; j++) {
			int persta = 0, graphtype = 0;
			json_object *array_y;

			k = selrows[j];

			graphtype 	= atoi(qResults[(k+1)*nCols+4]);
			persta 		= atoi(qResults[(k+1)*nCols+5]);

			array_y = json_object_new_array();

			get_graph_details_from_db(qResults[(k+1)*nCols+9],
				atoi(qResults[(k+1)*nCols]), rowid, graphtype,
				persta, &array_x, &array_y, j, tabname, mac);

			json_object_array_add(array_y_main, array_y);
		}
		json_object_object_add(object, "XValue", array_x);
		json_object_object_add(object, "YValue", array_y_main);

		json_object_array_add(array_graph, object);
	}
	json_object_array_add(array_main, array_graph);

	const char *data = json_object_to_json_string(array_main);

	sqlite3_free_table(qResults);
	unlock_db_mutex(TABLE_GRAPHS, idx);
	if (mainrows) {
		free(mainrows);
		mainrows = NULL;
	}

	SHOW_TIME_ELAPSE_END();

	if (data) {
		VIS_JSON("%s\n", data);
		ret = add_length_and_send_data(sockfd, data, strlen(data));
	}
	json_object_put(array_main);

	return 0;
}

/* Gets Associated STA details from the table */
assoc_sta_list_t*
get_associated_sta_from_db(int rowid)
{
	char			*szErrMsg = NULL;
	char			**qResults = NULL;
	char			querystr[512];
	int			i = 0, nRows = 0, nCols = 0, ret = 0, idx = -1;
	assoc_sta_list_t	*stas_list = NULL;

	SHOW_TIME_ELAPSE_START();

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, "
		"%s FROM %s "
		"WHERE %s = '%d' AND %s != '0' AND "
		"%s = (SELECT %s FROM %s WHERE %s = '%d' ORDER BY %s DESC LIMIT 1); ",
		COL_ASSOC_TIME, COL_ASSOC_MAC, COL_ASSOC_RSSI,
		COL_ASSOC_PHYRATE, TABLE_ASSOCSTA,
		COL_ASSOC_ROWID, rowid, COL_ASSOC_MAC,
		COL_ASSOC_TIME, COL_ASSOC_TIME, TABLE_ASSOCSTA,
		COL_ASSOC_ROWID, rowid, COL_ASSOC_TIME);

	lock_db_mutex(TABLE_ASSOCSTA, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	unlock_db_mutex(TABLE_ASSOCSTA, idx);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		return NULL;
	}

	stas_list = (assoc_sta_list_t*)malloc(sizeof(assoc_sta_list_t) +
		(sizeof(assoc_sta_t) * nRows));
	if (stas_list == NULL) {
		VIS_DB("%s : Failed to allocate stas_list buffer for %d rows\n",
			__FUNCTION__, nRows);
		sqlite3_free_table(qResults);
		return NULL;
	}
	stas_list->length = nRows;

	for (i = 0; i < nRows; i++) {
		VIS_DB("TIME : %d\n", atoi(qResults[(i+1)*nCols]));
		VIS_DB("MAC : %s\n", qResults[(i+1)*nCols+1]);
		VIS_DB("RSSI : %d\n", atoi(qResults[(i+1)*nCols+2]));
		VIS_DB("PhyRate : %d\n", atoi(qResults[(i+1)*nCols+3]));

		snprintf(stas_list->sta[i].mac, sizeof(stas_list->sta[i].mac), "%s",
			qResults[(i+1)*nCols+1]);
		stas_list->sta[i].rssi = atoi(qResults[(i+1)*nCols+2]);
		stas_list->sta[i].phyrate = atoi(qResults[(i+1)*nCols+3]);
	}

	sqlite3_free_table(qResults);

	SHOW_TIME_ELAPSE_END();

	return stas_list;
}

/* Saves configuration settings to table */
int
save_configs_in_db(configs_t *configs, int forceupdate)
{
	char	querystr[256];
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	int	rval = 0, nRows = 0, nCols = 0, idx = -1, i = 0;

	SHOW_TIME_ELAPSE_START();

	lock_db_mutex(TABLE_CONFIG, &idx);

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, %s, %s, %s, %s, %s FROM %s",
		COL_CONFIG_INTERVAL, COL_CONFIG_DBSIZE, COL_CONFIG_ISSTART,
		COL_CONFIG_ISOVERWRITEDB, COL_CONFIG_ISAUTOSTART, COL_CONFIG_WEEKDAYS,
		COL_CONFIG_FROMTM, COL_CONFIG_TOTM, TABLE_CONFIG);
	rval = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	if (rval != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}
	if (nRows > 0) { /* UPDATE */
		if (forceupdate == TRUE) {
			snprintf(querystr, sizeof(querystr), "UPDATE %s SET %s = '%d',"
				"%s = '%d', %s = '%d', %s = '%d',"
				"%s = '%d', %s = '%d', %s = '%d', %s = '%d'; ",
				TABLE_CONFIG, COL_CONFIG_INTERVAL, configs->interval,
				COL_CONFIG_DBSIZE, configs->dbsize,
				COL_CONFIG_ISSTART, configs->isstart,
				COL_CONFIG_ISOVERWRITEDB, configs->isoverwrtdb,
				COL_CONFIG_ISAUTOSTART, configs->isautostart,
				COL_CONFIG_WEEKDAYS, configs->weekdays,
				COL_CONFIG_FROMTM, configs->fromtm,
				COL_CONFIG_TOTM, configs->totm);
			rval = execute_sql_statement(g_db, querystr);
		} else {
			i = 0;
			configs->interval = atoi(qResults[(i+1)*nCols]);
			configs->dbsize = atoi(qResults[(i+1)*nCols+1]);
			configs->isstart = atoi(qResults[(i+1)*nCols+2]);
			configs->isoverwrtdb = atoi(qResults[(i+1)*nCols+3]);
			configs->isautostart = atoi(qResults[(i+1)*nCols+4]);
			configs->weekdays = atoi(qResults[(i+1)*nCols+5]);
			configs->fromtm = atoi(qResults[(i+1)*nCols+6]);
			configs->totm = atoi(qResults[(i+1)*nCols+7]);
		}
	}
	else { /* INSERT */
		snprintf(querystr, sizeof(querystr),
			"INSERT INTO %s (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s) "
			"VALUES ('1', '%d', '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d'); ",
			TABLE_CONFIG, COL_CONFIG_ROWID, COL_CONFIG_INTERVAL,
			COL_CONFIG_DBSIZE, COL_CONFIG_ISSTART,
			COL_CONFIG_GATEWAY_IP, COL_CONFIG_ISOVERWRITEDB,
			COL_CONFIG_ISAUTOSTART, COL_CONFIG_WEEKDAYS, COL_CONFIG_FROMTM,
			COL_CONFIG_TOTM,
			configs->interval, configs->dbsize, configs->isstart,
			configs->gatewayip, configs->isoverwrtdb,
			configs->isautostart, configs->weekdays, configs->fromtm,
			configs->totm);
		rval = execute_sql_statement(g_db, querystr);
	}
	sqlite3_free_table(qResults);

end:
	unlock_db_mutex(TABLE_CONFIG, idx);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Gets the DUT details from the table */
int
get_dut_details_from_db(char *mac, dut_info_t *dutdet)
{
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	char	querystr[MAX_QUERY_SIZE];
	int	ret, nRows = 0, nCols = 0, idx = -1;

	SHOW_TIME_ELAPSE_START();

	lock_db_mutex(TABLE_DUTDETAILS, &idx);

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, %s, %s "
		"FROM %s WHERE %s = '%s' AND %s = '1'",
		COL_DUT_ROWID, COL_DUT_SSID, COL_DUT_BSSID, COL_DUT_CHANNEL,
		COL_DUT_BANDWIDTH, COL_DUT_RSSI, COL_DUT_NOISE,
		COL_DUT_CONTROLCH, COL_DUT_BAND, COL_DUT_SPEED,
		COL_DUT_STANDARDS, COL_DUT_MCASTRSN, COL_DUT_UCASTRSN, COL_DUT_AKMRSN,
		COL_DUT_ERRINFO, TABLE_DUTDETAILS, COL_DUT_MAC, mac, COL_DUT_ISENABLED);

	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}

	if (nRows > 0) {
		dutdet->rowid = atoi(qResults[(1)*nCols]);
		snprintf(dutdet->ssid, sizeof(dutdet->ssid), "%s", qResults[(1)*nCols+1]);
		snprintf(dutdet->bssid, sizeof(dutdet->bssid), "%s", qResults[(1)*nCols+2]);
		dutdet->channel = atoi(qResults[(1)*nCols+3]);
		dutdet->bandwidth = atoi(qResults[(1)*nCols+4]);
		dutdet->rssi = atoi(qResults[(1)*nCols+5]);
		dutdet->noise = atoi(qResults[(1)*nCols+6]);
		dutdet->ctrlch = atoi(qResults[(1)*nCols+7]);
		dutdet->band = atoi(qResults[(1)*nCols+8]);
		dutdet->maxrate = atoi(qResults[(1)*nCols+9]);
		snprintf(dutdet->networktype, sizeof(dutdet->networktype), "%s",
			qResults[(1)*nCols+10]);
		snprintf(dutdet->mcastrsntype, sizeof(dutdet->mcastrsntype), "%s",
			qResults[(1)*nCols+11]);
		snprintf(dutdet->ucastrsntype, sizeof(dutdet->ucastrsntype), "%s",
			qResults[(1)*nCols+12]);
		snprintf(dutdet->akmrsntype, sizeof(dutdet->akmrsntype), "%s",
			qResults[(1)*nCols+13]);
		dutdet->errinfo = atoi(qResults[(1)*nCols+14]);

		VIS_DB("ROW ID : %d\n", dutdet->rowid);
	} else {
		dutdet->rowid = 0;
	}

	sqlite3_free_table(qResults);

end:
	unlock_db_mutex(TABLE_DUTDETAILS, idx);

	SHOW_TIME_ELAPSE_END();

	return ret;
}

/* Checks whether the DUT mac is present in dutdetails table or not */
int
is_dut_present(char *mac, int *rowid, int islock, int ischeckenabled)
{
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	char	querystr[256];
	int	ret, nRows = 0, nCols = 0, idx = -1;

	SHOW_TIME_ELAPSE_START();

	if (islock == TRUE)
		lock_db_mutex(TABLE_DUTDETAILS, &idx);

	if (ischeckenabled) {
		snprintf(querystr, sizeof(querystr),
			"SELECT %s FROM %s WHERE %s = '%s' AND %s = '1';",
			COL_DUT_ROWID, TABLE_DUTDETAILS, COL_DUT_MAC, mac, COL_DUT_ISENABLED);
	} else {
		snprintf(querystr, sizeof(querystr),
			"SELECT %s FROM %s WHERE %s = '%s';",
			COL_DUT_ROWID, TABLE_DUTDETAILS, COL_DUT_MAC, mac);
	}

	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}

	if (nRows > 0)	{
		*rowid = atoi(qResults[(1)*nCols]);

		VIS_DB("ROW ID : %d\n", *rowid);
	} else {
		*rowid = 0;
	}

	sqlite3_free_table(qResults);

end:
	if (islock == TRUE)
		unlock_db_mutex(TABLE_DUTDETAILS, idx);

	SHOW_TIME_ELAPSE_END();

	return ret;
}

/* Gets errorinfo from dutinfo table. */
static uint32
get_dut_errinfo(int rowid, int islock)
{
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	char	querystr[256];
	int	ret, nRows = 0, nCols = 0, idx = -1;
	uint32 errinfo = 0;

	SHOW_TIME_ELAPSE_START();

	if (islock == TRUE) {
		lock_db_mutex(TABLE_DUTDETAILS, &idx);
	}

	snprintf(querystr, sizeof(querystr),
		"SELECT %s FROM %s WHERE %s = '%d';",
		COL_DUT_ERRINFO, TABLE_DUTDETAILS, COL_DUT_ROWID, rowid);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}

	if (nRows > 0)	{
		errinfo = atoi(qResults[(1)*nCols]);
		VIS_DB("errinfo : %d\n", errinfo);
	}

	sqlite3_free_table(qResults);

end:
	if (islock == TRUE) {
		unlock_db_mutex(TABLE_DUTDETAILS, idx);
	}

	SHOW_TIME_ELAPSE_END();

	return errinfo;
}

/* Stores DUT details to table */
int
store_dut_details(dut_info_t *dutinfo)
{
	char	querystr[MAX_QUERY_SIZE];
	int	rval = 0, rowid = -1, idx = -1;
	uint32 errorinfo = 0, currerr = 0;

	SHOW_TIME_ELAPSE_START();

	lock_db_mutex(TABLE_DUTDETAILS, &idx);

	rval = is_dut_present(dutinfo->mac, &rowid, FALSE, FALSE);

	if (rowid <= 0) { /* INSERT */
		sqlite3_snprintf(sizeof(querystr), querystr, "INSERT INTO %s (%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s) "
			"VALUES (NULL, '%d', '%s', "
			"'%q', '%s', '%d', "
			"'%d', '%d', '%d', "
			"'%d', '%d', '%d', "
			"'%s', '%s', '%s', "
			"'%s', '%d', '%d'); ",
			TABLE_DUTDETAILS, COL_DUT_ROWID, COL_DUT_ISAP, COL_DUT_BSSID,
			COL_DUT_SSID, COL_DUT_MAC, COL_DUT_CHANNEL,
			COL_DUT_BANDWIDTH, COL_DUT_RSSI, COL_DUT_NOISE,
			COL_DUT_CONTROLCH, COL_DUT_BAND, COL_DUT_SPEED,
			COL_DUT_STANDARDS, COL_DUT_MCASTRSN, COL_DUT_UCASTRSN,
			COL_DUT_AKMRSN, COL_DUT_ISENABLED, COL_DUT_ERRINFO,
			dutinfo->isAP, dutinfo->bssid,
			dutinfo->ssid, dutinfo->mac, dutinfo->channel,
			dutinfo->bandwidth, dutinfo->rssi, dutinfo->noise,
			dutinfo->ctrlch, dutinfo->band, dutinfo->maxrate,
			dutinfo->networktype, dutinfo->mcastrsntype, dutinfo->ucastrsntype,
			dutinfo->akmrsntype, dutinfo->isenabled, dutinfo->errinfo);

		rval = execute_sql_statement(g_db, querystr);
		is_dut_present(dutinfo->mac, &dutinfo->rowid, FALSE, FALSE);
	} else { /* UPDATE */

		/* get the stored errinfo val */
		currerr = get_dut_errinfo(rowid, FALSE);
		errorinfo = dutinfo->errinfo;
		if (VIS_CHECK_BIT(errorinfo, VIS_SCAN_DONE)) {
			if (VIS_CHECK_BIT(errorinfo, VIS_SCAN_ERROR)) {
				VIS_SET_BIT(currerr, VIS_SCAN_ERROR);
			} else {
				VIS_CLEAR_BIT(currerr, VIS_SCAN_ERROR);
			}
		}

		sqlite3_snprintf(sizeof(querystr), querystr, "UPDATE %s SET %s = '%d', "
			"%s = '%d', %s = '%s', "
			"%s = '%q', %s = '%d', "
			"%s = '%d', "
			"%s = '%d', %s = '%d', "
			"%s = '%d', %s = '%s', "
			"%s = '%s', %s = '%s', "
			"%s = '%s', %s = '%d', %s = '%d' WHERE %s = '%d'",
			TABLE_DUTDETAILS, COL_DUT_CHANNEL, dutinfo->channel,
			COL_DUT_BANDWIDTH, dutinfo->bandwidth,
			COL_DUT_BSSID, dutinfo->bssid,
			COL_DUT_SSID, dutinfo->ssid,
			COL_DUT_RSSI, dutinfo->rssi,
			COL_DUT_NOISE, dutinfo->noise,
			COL_DUT_CONTROLCH, dutinfo->ctrlch,
			COL_DUT_BAND, dutinfo->band,
			COL_DUT_SPEED, dutinfo->maxrate,
			COL_DUT_STANDARDS, dutinfo->networktype,
			COL_DUT_MCASTRSN, dutinfo->mcastrsntype,
			COL_DUT_UCASTRSN, dutinfo->ucastrsntype,
			COL_DUT_AKMRSN, dutinfo->akmrsntype,
			COL_DUT_ISENABLED, dutinfo->isenabled,
			COL_DUT_ERRINFO, currerr,
			COL_DUT_ROWID, rowid);
		dutinfo->rowid = rowid; /* Save the rowID */
		rval = execute_sql_statement(g_db, querystr);
	}

	unlock_db_mutex(TABLE_DUTDETAILS, idx);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Stores site survey result to table */
int
insert_networks_details(dut_info_t *dutinfo, networks_list_t *networks_list)
{
	int	rval = 0, i = 0, idx = -1;
	char	*sql = NULL;
	int	szalloc = MAX_QUERY_SIZE, totlen = 0;

	SHOW_TIME_ELAPSE_START();

	sql = (char*)malloc(sizeof(char) * szalloc);
	if (sql == NULL) {
		VIS_DB("Failed to allocate memory in scanresult insert for sql for %d\n",
			szalloc);
		return -1;
	}

	sqlite3_snprintf(szalloc, sql, "INSERT INTO %s (%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s) "
			"VALUES",
			TABLE_SCAN, COL_SCAN_ROWID, COL_SCAN_TIME, COL_SCAN_SSID,
			COL_SCAN_BSSID, COL_SCAN_RSSI, COL_SCAN_CHANNEL,
			COL_SCAN_NOISE, COL_SCAN_BANDWIDTH, COL_SCAN_SPEED,
			COL_SCAN_STANDARDS, COL_SCAN_MCASTRSN, COL_SCAN_UCASTRSN,
			COL_SCAN_AKMRSN, COL_SCAN_CONTROLCH);

	totlen = strlen(sql);

	for (i = 0; i < networks_list->length; i++) {
		char tmpsql[MAX_QUERY_SIZE];

		sqlite3_snprintf(sizeof(tmpsql), tmpsql, " ('%d', '%ld', '%q', "
			"'%s', '%d', '%d', "
			"'%d', '%d', "
			"'%d', '%s', '%s', "
			"'%s', '%s', '%d'),",
			dutinfo->rowid, networks_list->timestamp, networks_list->aps[i].ssid,
			networks_list->aps[i].bssid, networks_list->aps[i].rssi,
			networks_list->aps[i].channel,
			networks_list->aps[i].noise, networks_list->aps[i].bandwidth,
			networks_list->aps[i].maxrate, networks_list->aps[i].networktype,
			networks_list->aps[i].mcastrsntype,
			networks_list->aps[i].ucastrsntype, networks_list->aps[i].akmrsntype,
			networks_list->aps[i].ctrlch);

		if ((totlen + strlen(tmpsql) + 1) > szalloc) {
			szalloc += MAX_QUERY_SIZE;
			char *tmp = (char*)realloc(sql, sizeof(char) * szalloc);
			if (tmp == NULL) {
				VIS_DB("Failed to allocate memory in scanresult insert for %d\n",
					szalloc);
				goto end;
			}
			sql = tmp;
		}

		strncat(sql, tmpsql, (szalloc - totlen - 1));
		totlen += strlen(tmpsql);
	}
	sql[totlen-1] = ';';

	lock_db_mutex(TABLE_SCAN, &idx);

	if (networks_list->length > 0)
		rval = execute_sql_statement(g_db, sql);

end:
	unlock_db_mutex(TABLE_SCAN, idx);
	if (sql)
		free(sql);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Stores empty record to site survey table */
int
insert_empty_networks_details(dut_info_t *dutinfo, networks_list_t *networks_list)
{
	int	rval = 0, idx = -1;
	char	querystr[MAX_QUERY_SIZE];

	SHOW_TIME_ELAPSE_START();

	snprintf(querystr, sizeof(querystr), "INSERT INTO %s (%s, %s, %s, "
			"%s) VALUES ('%d', '%ld', '0', '0');",
			TABLE_SCAN, COL_SCAN_ROWID, COL_SCAN_TIME, COL_SCAN_SSID,
			COL_SCAN_BSSID, dutinfo->rowid, networks_list->timestamp);

	lock_db_mutex(TABLE_SCAN, &idx);

	rval = execute_sql_statement(g_db, querystr);

	unlock_db_mutex(TABLE_SCAN, idx);
	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Stores Associated STA details to table */
int
insert_assocsta_details(dut_info_t *dutinfo, assoc_sta_list_t *stas_list)
{
	int	rval = 0, i = 0, idx = -1;
	char	*sql = NULL;
	int	szalloc = MAX_QUERY_SIZE, totlen = 0;

	SHOW_TIME_ELAPSE_START();

	sql = (char*)malloc(sizeof(char) * szalloc);
	if (sql == NULL) {
		VIS_DB("Failed to allocate memory in assocresult insert for sql of %d\n",
			szalloc);
		return -1;
	}

	snprintf(sql, szalloc, "INSERT INTO %s (%s, %s, "
			"%s, %s, %s) "
			"VALUES ",
			TABLE_ASSOCSTA, COL_ASSOC_ROWID, COL_ASSOC_TIME,
			COL_ASSOC_MAC, COL_ASSOC_RSSI, COL_ASSOC_PHYRATE);
	totlen = strlen(sql);

	lock_db_mutex(TABLE_ASSOCSTA, &idx);

	for (i = 0; i < stas_list->length; i++) {
		char tmpsql[MAX_QUERY_SIZE];

		snprintf(tmpsql, sizeof(tmpsql), " ('%d', '%ld', '%s', '%d', '%d'),",
			dutinfo->rowid, stas_list->timestamp,
			stas_list->sta[i].mac, stas_list->sta[i].rssi, stas_list->sta[i].phyrate);

		if ((totlen + strlen(tmpsql) + 1) > szalloc) {
			szalloc += MAX_QUERY_SIZE;
			char *tmp = (char*)realloc(sql, sizeof(char) * szalloc);
			if (tmp == NULL) {
				VIS_DB("Failed to allocate memory in assocresult insert of %d\n",
					szalloc);
				goto end;
			}
			sql = tmp;
		}

		strncat(sql, tmpsql, (szalloc - totlen - 1));
		totlen += strlen(tmpsql);
	}
	if (stas_list->length <= 0) {
		char tmpsql[MAX_QUERY_SIZE];

		snprintf(tmpsql, sizeof(tmpsql), " ('%d', '%ld', '%s', '%d', '%d'),",
			dutinfo->rowid, stas_list->timestamp,
			"0", 0, 0);
		if ((totlen + strlen(tmpsql) + 1) > szalloc) {
			szalloc += MAX_QUERY_SIZE;
			char *tmp = (char*)realloc(sql, sizeof(char) * szalloc);
			if (tmp == NULL) {
				VIS_DB("Failed to allocate memory in assocresult insert of %d\n",
					szalloc);
				goto end;
			}
			sql = tmp;
		}

		strncat(sql, tmpsql, (szalloc - totlen - 1));
		totlen += strlen(tmpsql);
	}
	sql[totlen-1] = ';';
	rval = execute_sql_statement(g_db, sql);

end:
	unlock_db_mutex(TABLE_ASSOCSTA, idx);
	if (sql)
		free(sql);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Stores channel statistics details to table */
int
insert_channel_stats_details(dut_info_t *dutinfo, congestion_list_t *congestion)
{
	int	rval = 0, i = 0, idx = -1;
	char	*sql = NULL;
	int	szalloc = MAX_QUERY_SIZE, totlen = 0;

	SHOW_TIME_ELAPSE_START();

	sql = (char*)malloc(sizeof(char) * szalloc);
	if (sql == NULL) {
		VIS_DB("Failed to allocate memory insert for sql of %d\n",
			szalloc);
		return -1;
	}

	snprintf(sql, szalloc, "INSERT INTO %s (%s, %s, "
			"%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s) "
			"VALUES ",
			TABLE_CHANNELSTATS, COL_CHSTATS_ROWID, COL_CHSTATS_TIME,
			COL_CHSTATS_CHANNEL, COL_CHSTATS_TX, COL_CHSTATS_INBSS,
			COL_CHSTATS_OBSS, COL_CHSTATS_NOCAT, COL_CHSTATS_NOPKT,
			COL_CHSTATS_DOZE, COL_CHSTATS_TXOP, COL_CHSTATS_GOODTX,
			COL_CHSTATS_BADTX, COL_CHSTATS_GLITCH, COL_CHSTATS_BADPLCP,
			COL_CHSTATS_KNOISE, COL_CHSTATS_IDLE);
	totlen = strlen(sql);

	lock_db_mutex(TABLE_CHANNELSTATS, &idx);

	for (i = 0; i < congestion->length; i++) {
		char tmpsql[MAX_QUERY_SIZE];
		int idle = 0;

		/* As idle is calculated using formula idle = 100 - busy
		 * where busy = inbss+goodtx+badtx+obss+nocat+nopkt.
		 * So, instead of chan_idle use the computed idle
		 */
		idle = 100 - (congestion->congest[i].inbss +
			congestion->congest[i].obss + congestion->congest[i].goodtx +
			congestion->congest[i].badtx + congestion->congest[i].nocat +
			congestion->congest[i].nopkt);

		snprintf(tmpsql, sizeof(tmpsql), " ('%d', '%ld', '%d',"
			"'%d', '%d', '%d', '%d',"
			"'%d', '%d', '%d', '%d',"
			"'%d', '%d', '%d', '%d',"
			"'%d'),",
			dutinfo->rowid, congestion->timestamp, congestion->congest[i].channel,
			congestion->congest[i].tx, congestion->congest[i].inbss,
			congestion->congest[i].obss, congestion->congest[i].nocat,
			congestion->congest[i].nopkt, congestion->congest[i].doze,
			congestion->congest[i].txop, congestion->congest[i].goodtx,
			congestion->congest[i].badtx, congestion->congest[i].glitchcnt,
			congestion->congest[i].badplcp, congestion->congest[i].knoise,
			idle);

		if ((totlen + strlen(tmpsql) + 1) > szalloc) {
			szalloc += MAX_QUERY_SIZE;
			char *tmp = (char*)realloc(sql, sizeof(char) * szalloc);
			if (tmp == NULL) {
				VIS_DB("Failed to allocate memory insert of %d\n",
					szalloc);
				goto end;
			}
			sql = tmp;
		}

		strncat(sql, tmpsql, (szalloc - totlen - 1));
		totlen += strlen(tmpsql);
	}
	sql[totlen-1] = ';';
	if (congestion->length > 0)
		rval = execute_sql_statement(g_db, sql);

end:
	unlock_db_mutex(TABLE_CHANNELSTATS, idx);
	if (sql)
		free(sql);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Stores AMPDU Statistics to table */
int
insert_ampdu_stats_details(dut_info_t *dutinfo, tx_ampdu_list_t *ampdulist)
{
	int	rval = 0, i = 0, idx = -1;
	char	*sql = NULL;
	int	szalloc = MAX_QUERY_SIZE, totlen = 0;

	SHOW_TIME_ELAPSE_START();

	sql = (char*)malloc(sizeof(char) * szalloc);
	if (sql == NULL) {
		VIS_DB("Failed to allocate memory insert for sql of %d\n",
			szalloc);
		return -1;
	}

	snprintf(sql, szalloc, "INSERT INTO %s (%s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, "
			"%s, %s) "
			"VALUES ",
			TABLE_AMPDUSTATS, COL_AMPDU_ROWID, COL_AMPDU_TIME,
			COL_AMPDU_MCS, COL_AMPDU_TXMCS, COL_AMPDU_TXMCSPERCENT,
			COL_AMPDU_TXMCSSGI, COL_AMPDU_TXMCSSGIPERCENT, COL_AMPDU_RXMCS,
			COL_AMPDU_RXMCSPERCENT, COL_AMPDU_RXMCSSGI, COL_AMPDU_RXMCSSGIPERCENT,
			COL_AMPDU_TXVHT, COL_AMPDU_TXVHTPER, COL_AMPDU_TXVHTPERCENT,
			COL_AMPDU_TXVHTSGI, COL_AMPDU_TXVHTSGIPER, COL_AMPDU_TXVHTSGIPERCENT,
			COL_AMPDU_RXVHT, COL_AMPDU_RXVHTPER, COL_AMPDU_RXVHTPERCENT,
			COL_AMPDU_RXVHTSGI, COL_AMPDU_RXVHTSGIPER, COL_AMPDU_RXVHTSGIPERCENT,
			COL_AMPDU_MPDUDENS, COL_AMPDU_MPDUDENSPERCENT);
	totlen = strlen(sql);

	lock_db_mutex(TABLE_AMPDUSTATS, &idx);

	for (i = 0; i < ampdulist->length; i++) {
		char tmpsql[MAX_QUERY_SIZE];

		snprintf(tmpsql, sizeof(tmpsql), " ('%d', '%ld', "
			"'%d', '%d', '%d', "
			"'%d', '%d', '%d', "
			"'%d', '%d', '%d', "
			"'%d', '%d', '%d', "
			"'%d', '%d', '%d', "
			"'%d', '%d', '%d', "
			"'%d', '%d', '%d', "
			"'%d', '%d'),",
			dutinfo->rowid, ampdulist->timestamp,
			ampdulist->ampdus[i].mcs, ampdulist->ampdus[i].txmcs,
			ampdulist->ampdus[i].txmcspercent,
			ampdulist->ampdus[i].txmcssgi, ampdulist->ampdus[i].txmcssgipercent,
			ampdulist->ampdus[i].rxmcs,
			ampdulist->ampdus[i].rxmcspercent, ampdulist->ampdus[i].rxmcssgi,
			ampdulist->ampdus[i].rxmcssgipercent,
			ampdulist->ampdus[i].txvht, ampdulist->ampdus[i].txvhtper,
			ampdulist->ampdus[i].txvhtpercent,
			ampdulist->ampdus[i].txvhtsgi, ampdulist->ampdus[i].txvhtsgiper,
			ampdulist->ampdus[i].txvhtsgipercent,
			ampdulist->ampdus[i].rxvht, ampdulist->ampdus[i].rxvhtper,
			ampdulist->ampdus[i].rxvhtpercent,
			ampdulist->ampdus[i].rxvhtsgi, ampdulist->ampdus[i].rxvhtsgiper,
			ampdulist->ampdus[i].rxvhtsgipercent,
			ampdulist->ampdus[i].mpdudens, ampdulist->ampdus[i].mpdudenspercent);

		if ((totlen + strlen(tmpsql) + 1) > szalloc) {
			szalloc += MAX_QUERY_SIZE;
			char *tmp = (char*)realloc(sql, sizeof(char) * szalloc);
			if (tmp == NULL) {
				VIS_DB("Failed to allocate memory insert of %d\n",
					szalloc);
				goto end;
			}
			sql = tmp;
		}

		strncat(sql, tmpsql, (szalloc - totlen - 1));
		totlen += strlen(tmpsql);
	}
	sql[totlen-1] = ';';
	if (ampdulist->length > 0)
		rval = execute_sql_statement(g_db, sql);

end:
	unlock_db_mutex(TABLE_AMPDUSTATS, idx);
	if (sql)
		free(sql);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Stores RRM STA Side stats to table */
int
insert_sta_side_stats_details(dut_info_t *dutinfo, vis_sta_statslist_t *stastatslist)
{
	int	rval = 0, i = 0, idx = -1;
	char	*sql = NULL;
	int	szalloc = MAX_QUERY_SIZE, totlen = 0;

	SHOW_TIME_ELAPSE_START();

	sql = (char*)malloc(sizeof(char) * szalloc);
	if (sql == NULL) {
		VIS_DB("Failed to allocate memory in sta side stats insert for sql for %d\n",
			szalloc);
		return -1;
	}

	snprintf(sql, szalloc, "INSERT INTO %s (%s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s) "
			"VALUES",
			TABLE_RRM_STA_STATS, COL_RRMSTATS_ROWID, COL_RRMSTATS_MAC,
			COL_RRMSTATS_TIME, COL_RRMSTATS_GENERATION, COL_RRMSTATS_TXSTREAM,
			COL_RRMSTATS_RXSTREAM, COL_RRMSTATS_RATE, COL_RRMSTATS_IDLE);

	totlen = strlen(sql);

	for (i = 0; i < stastatslist->length; i++) {
		char tmpsql[MAX_QUERY_SIZE];

		snprintf(tmpsql, sizeof(tmpsql), " ('%d', '%s', '%ld', "
			"'%d', '%d', '%d', "
			"'%d', '%d'),",
			dutinfo->rowid, stastatslist->stastats[i].mac, stastatslist->timestamp,
			stastatslist->stastats[i].generation_flag,
			stastatslist->stastats[i].txstream, stastatslist->stastats[i].rxstream,
			stastatslist->stastats[i].rate, stastatslist->stastats[i].idle);

		if ((totlen + strlen(tmpsql) + 1) > szalloc) {
			szalloc += MAX_QUERY_SIZE;
			char *tmp = (char*)realloc(sql, sizeof(char) * szalloc);
			if (tmp == NULL) {
				VIS_DB("Failed to allocate memory in sta stats insert for %d\n",
					szalloc);
				goto end;
			}
			sql = tmp;
		}

		strncat(sql, tmpsql, (szalloc - totlen - 1));
		totlen += strlen(tmpsql);
	}
	sql[totlen-1] = ';';

	lock_db_mutex(TABLE_RRM_STA_STATS, &idx);

	if (stastatslist->length > 0)
		rval = execute_sql_statement(g_db, sql);

end:
	unlock_db_mutex(TABLE_RRM_STA_STATS, idx);
	if (sql)
		free(sql);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Stores RRM Advanced STA Side stats to table */
int
insert_rrm_adv_sta_side_stats_details(dut_info_t *dutinfo, vis_rrm_statslist_t *rrmstatlist)
{
	int	rval = 0, i = 0, idx = -1;
	char	*sql = NULL;
	int	szalloc = MAX_QUERY_SIZE, totlen = 0;

	SHOW_TIME_ELAPSE_START();

	sql = (char*)malloc(sizeof(char) * szalloc);
	if (sql == NULL) {
		VIS_DB("Failed to allocate memory in rrm advanced stats insert for sql for %d\n",
			szalloc);
		return -1;
	}

	snprintf(sql, szalloc, "INSERT INTO %s (%s, %s, "
			"%s, %s, %s, "
			"%s, %s, %s, %s) "
			"VALUES",
			TABLE_RRM_STA_ADV_STATS, COL_RRMADVSTATS_ROWID, COL_RRMADVSTATS_MAC,
			COL_RRMADVSTATS_TIME, COL_RRMADVSTATS_TXOP, COL_RRMADVSTATS_PKTREQ,
			COL_RRMADVSTATS_PKTDROP, COL_RRMADVSTATS_PKTSTORED,
			COL_RRMADVSTATS_PKTRETRIED, COL_RRMADVSTATS_PKTACKED);

	totlen = strlen(sql);

	for (i = 0; i < rrmstatlist->length; i++) {
		char tmpsql[MAX_QUERY_SIZE];

		snprintf(tmpsql, sizeof(tmpsql), " ('%d', '%s', '%ld', "
			"'%d', '%d', '%d', "
			"'%d', '%d', '%d'),",
			dutinfo->rowid, rrmstatlist->rrmstats[i].mac, rrmstatlist->timestamp,
			rrmstatlist->rrmstats[i].txop, rrmstatlist->rrmstats[i].pktrequested,
			rrmstatlist->rrmstats[i].pktdropped, rrmstatlist->rrmstats[i].pktstored,
			rrmstatlist->rrmstats[i].pktretried, rrmstatlist->rrmstats[i].pktacked);

		if ((totlen + strlen(tmpsql) + 1) > szalloc) {
			szalloc += MAX_QUERY_SIZE;
			char *tmp = (char*)realloc(sql, sizeof(char) * szalloc);
			if (tmp == NULL) {
				VIS_DB("Failed to allocate memory in rrm advanced insert for %d\n",
					szalloc);
				goto end;
			}
			sql = tmp;
		}

		strncat(sql, tmpsql, (szalloc - totlen - 1));
		totlen += strlen(tmpsql);
	}
	sql[totlen-1] = ';';

	lock_db_mutex(TABLE_RRM_STA_ADV_STATS, &idx);

	if (rrmstatlist->length > 0)
		rval = execute_sql_statement(g_db, sql);

end:
	unlock_db_mutex(TABLE_RRM_STA_ADV_STATS, idx);
	if (sql)
		free(sql);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

#ifdef STORE_GRAPHS_IN_LIST
/* Checks whether the graph name is present in graphs list or not */
static int
is_graph_present_in_list(graphdetails_t *graphdet, iovar_handler_t *handle)
{
	graphdet->rowid = 0;
	if (g_tablelist != NULL) {
		int i;

		for (i = 0; i < g_tablelist->length; i++) {
			if (strcmp(g_tablelist->tables[i].name, handle->name) == 0) {
				graphdet->rowid = g_tablelist->tables[i].rowid;
				snprintf(graphdet->tab, sizeof(graphdet->tab), "%s",
					g_tablelist->tables[i].tab);
				snprintf(graphdet->table, sizeof(graphdet->table), "%s",
					g_tablelist->tables[i].tablename);

				VIS_DB("In List ROW ID : %d, Tab : %s, Table : %s\n",
					graphdet->rowid, graphdet->tab, graphdet->table);
				return 0;
			}
		}
	}

	return -1;
}
#endif /* STORE_GRAPHS_IN_LIST */

#ifdef STORE_GRAPHS_IN_LIST
/* Adds the graph details to structure */
static int
add_graph_to_list(int rowid, char *name, char *table, char *tab)
{
	int i;

	/* allocate memory */
	if (g_tablelist == NULL) {
		g_tablelist = (tablelist_t*)malloc(sizeof(tablelist_t) +
			(sizeof(table_t) * MAX_GRAPHS));
		if (g_tablelist == NULL) {
			VIS_DB("Failed to allocate memory for g_tablelist of %d\n", MAX_GRAPHS);
			return -1;
		}
		g_tablelist->alloclen = MAX_GRAPHS;
		g_tablelist->length = 0;
	} else {
		/* reallocate memory */
		if (g_tablelist->alloclen <= g_tablelist->length) {
			tablelist_t *tmp = NULL;
			tmp = (tablelist_t*)realloc(g_tablelist,
				sizeof(tablelist_t)+(sizeof(table_t)*(g_tablelist->alloclen+1)));
			if (tmp == NULL) {
				VIS_DB("Failed to reallocate memory g_tablelist of %d\n",
					g_tablelist->alloclen);
				return -1;
			}
			g_tablelist = tmp;
			g_tablelist->alloclen++;
		}
	}

	/* now add the graph name */
	i = g_tablelist->length;
	g_tablelist->tables[i].rowid = rowid;
	snprintf(g_tablelist->tables[i].name, sizeof(g_tablelist->tables[i].name), "%s", name);
	snprintf(g_tablelist->tables[i].tablename, sizeof(g_tablelist->tables[i].tablename), "%s",
		table);
	snprintf(g_tablelist->tables[i].tab, sizeof(g_tablelist->tables[i].tab), "%s", tab);
	g_tablelist->length++;

	VIS_DB("Row Id : %d\t Name : %s\t Table : %s\t Tab : %s\n",
		rowid, name, table, tab);

	return 0;
}
#endif /* STORE_GRAPHS_IN_LIST */

/* Checks whether the graph name is present in graphs table or not */
static int
is_graph_present_in_db(graphdetails_t *graphdet, iovar_handler_t *handle)
{
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	char	querystr[256];
	int	ret, nRows = 0, nCols = 0;

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s FROM %s WHERE %s = '%s'",
		COL_GRAPH_ROWID, COL_GRAPH_TAB, COL_GRAPH_TABLE,
		TABLE_GRAPHS, COL_GRAPH_NAME, handle->name);

	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}

	if (nRows > 0) {
		graphdet->rowid = atoi(qResults[(1)*nCols]);
		snprintf(graphdet->tab, sizeof(graphdet->tab), "%s", qResults[(1)*nCols+1]);
		snprintf(graphdet->table, sizeof(graphdet->table), "%s", qResults[(1)*nCols+2]);

	VIS_DB("ROW ID : %d, Tab : %s, Table : %s\n", graphdet->rowid,
		graphdet->tab, graphdet->table);
	} else {
		graphdet->rowid = 0;
	}

	sqlite3_free_table(qResults);

end:

	return ret;
}

/* Get the row entry from graphs table */
int
get_graph_row(graphdetails_t *graphdet, iovar_handler_t *handle)
{
	int	rval = 0, idx = -1;

	SHOW_TIME_ELAPSE_START();

	lock_db_mutex(TABLE_GRAPHS, &idx);

#ifdef STORE_GRAPHS_IN_LIST
	rval = is_graph_present_in_list(graphdet, handle);
#else
	rval = is_graph_present_in_db(graphdet, handle);
#endif /* STORE_GRAPHS_IN_LIST */

	unlock_db_mutex(TABLE_GRAPHS, idx);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* stores particular graph details(xaxis value and yaxis value to table */
int
insert_graph_details(int sockfd, dut_info_t *dutinfo, graph_list_t *graphs,
	graphdetails_t *graphdet, iovar_handler_t *handle)
{
	int	rval = 0, i = 0, idx = -1;
	char	*sql = NULL;
	int	szalloc = MAX_QUERY_SIZE, totlen = 0;

	SHOW_TIME_ELAPSE_START();

	sql = (char*)malloc(sizeof(char) * szalloc);
	if (sql == NULL) {
		VIS_DB("Failed to allocate memory in assocresult insert for sql of %d\n",
			szalloc);
		return -1;
	}
	if (handle->perSTA == 1) {
		snprintf(sql, szalloc, "INSERT INTO %s ('%s', '%s', "
				"'%s', '%s', '%s', '%s') "
				"VALUES ",
				graphdet->table, COL_CGRAPH_ROWID, COL_CGRAPH_GRAPHROWID,
				COL_CGRAPH_MAC, COL_CGRAPH_TIME,
				COL_CGRAPH_XAXIS, COL_CGRAPH_YAXIS);
	} else {
		snprintf(sql, szalloc, "INSERT INTO %s ('%s', '%s', "
				"'%s', '%s', '%s') "
				"VALUES ",
				graphdet->table, COL_CGRAPH_ROWID, COL_CGRAPH_GRAPHROWID,
				COL_CGRAPH_TIME, COL_CGRAPH_XAXIS, COL_CGRAPH_YAXIS);
	}
	totlen = strlen(sql);

	lock_db_mutex(graphdet->table, &idx);

	/* Now insert the values */
	if (handle->perSTA == 1) {
		for (i = 0; i < graphs->length; i++) {
			char tmpsql[MAX_QUERY_SIZE];

			snprintf(tmpsql, sizeof(tmpsql), " ('%d', '%d', '%s', '%ld', '%d', '%s'),",
				dutinfo->rowid, graphdet->rowid, graphs->mac,
				graphs->graph[i].timestamp,
				graphs->graph[i].xvalue, graphs->graph[i].yvalue);

			if ((totlen + strlen(tmpsql) + 1) > szalloc) {
				szalloc += MAX_QUERY_SIZE;
				char *tmp = (char*)realloc(sql, sizeof(char) * szalloc);
				if (tmp == NULL) {
					VIS_DB("Failed to allocate memory in "
						"assocresult insert of %d\n", szalloc);
					goto end;
				}
				sql = tmp;
			}

			strncat(sql, tmpsql, (szalloc - totlen - 1));
			totlen += strlen(tmpsql);
		}
	} else {
		for (i = 0; i < graphs->length; i++) {
			char tmpsql[MAX_QUERY_SIZE];

			snprintf(tmpsql, sizeof(tmpsql), " ('%d', '%d', '%ld', '%d', '%s'),",
				dutinfo->rowid, graphdet->rowid,
				graphs->graph[i].timestamp,
				graphs->graph[i].xvalue, graphs->graph[i].yvalue);
			if ((totlen + strlen(tmpsql) + 1) > szalloc) {
				szalloc += MAX_QUERY_SIZE;
				char *tmp = (char*)realloc(sql, sizeof(char) * szalloc);
				if (tmp == NULL) {
					VIS_DB("Failed to allocate memory in "
						"assocresult insert of %d\n",
						szalloc);
					goto end;
				}
				sql = tmp;
			}
			strncat(sql, tmpsql, (szalloc - totlen - 1));
			totlen += strlen(tmpsql);
		}
	}
	sql[totlen-1] = ';';
	if (graphs->length > 0)
		rval = execute_sql_statement(g_db, sql);

end:
	unlock_db_mutex(graphdet->table, idx);
	if (sql)
		free(sql);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* this will delete all the rows from the table when the DB size exceedes the limit */
static int
delete_rows_from_tables()
{
	char	querystr[256];
	int	rval = 0, nRows = 0, nCols = 0, i = 0, idx = -1, ncount;
	char	*szErrMsg = NULL;
	char	**qResults = NULL;

	/* delete from scanresult table */
	/* First check the number of rows */
	snprintf(querystr, sizeof(querystr), "SELECT COUNT(DISTINCT %s) AS count FROM %s; ",
		COL_SCAN_TIME, TABLE_SCAN);
	ncount = get_number_of_rows(TABLE_SCAN, querystr);
	if (ncount > (MAX_DELETE_ROWS + 10)) {
		snprintf(querystr, sizeof(querystr),
			"DELETE FROM %s WHERE %s IN (SELECT DISTINCT %s "
			"FROM %s ORDER BY %s ASC LIMIT %d); ",
			TABLE_SCAN, COL_SCAN_TIME, COL_SCAN_TIME, TABLE_SCAN,
			COL_SCAN_TIME, MAX_DELETE_ROWS);
		lock_db_mutex(TABLE_SCAN, &idx);
		rval = execute_sql_statement(g_db, querystr);
		unlock_db_mutex(TABLE_SCAN, idx);
	}

	/* delete from assocSTA table */
	/* First check the number of rows */
	snprintf(querystr, sizeof(querystr), "SELECT COUNT(DISTINCT %s) AS count FROM %s; ",
		COL_ASSOC_TIME, TABLE_ASSOCSTA);
	ncount = get_number_of_rows(TABLE_ASSOCSTA, querystr);
	if (ncount > (MAX_DELETE_ROWS + 10)) {
		snprintf(querystr, sizeof(querystr),
			"DELETE FROM %s WHERE %s IN (SELECT DISTINCT %s FROM %s "
			"ORDER BY %s ASC LIMIT %d); ",
			TABLE_ASSOCSTA, COL_ASSOC_TIME, COL_ASSOC_TIME, TABLE_ASSOCSTA,
			COL_ASSOC_TIME, MAX_DELETE_ROWS);
		lock_db_mutex(TABLE_ASSOCSTA, &idx);
		rval = execute_sql_statement(g_db, querystr);
		unlock_db_mutex(TABLE_ASSOCSTA, idx);
	}

	/* delete from Channel Stats table */
	snprintf(querystr, sizeof(querystr),
		"DELETE FROM %s WHERE %s IN (SELECT DISTINCT %s FROM %s "
		"ORDER BY %s ASC LIMIT %d); ",
		TABLE_CHANNELSTATS, COL_CHSTATS_TIME, COL_CHSTATS_TIME, TABLE_CHANNELSTATS,
		COL_CHSTATS_TIME, MAX_DELETE_ROWS);
	lock_db_mutex(TABLE_CHANNELSTATS, &idx);
	rval = execute_sql_statement(g_db, querystr);
	unlock_db_mutex(TABLE_CHANNELSTATS, idx);

	/* delete from RRM STA Side statistics table  */
	snprintf(querystr, sizeof(querystr),
		"DELETE FROM %s WHERE %s IN (SELECT DISTINCT %s FROM %s "
		"ORDER BY %s ASC LIMIT %d); ",
		TABLE_RRM_STA_STATS, COL_RRMSTATS_TIME, COL_RRMSTATS_TIME, TABLE_RRM_STA_STATS,
		COL_RRMSTATS_TIME, MAX_DELETE_ROWS);
	lock_db_mutex(TABLE_RRM_STA_STATS, &idx);
	rval = execute_sql_statement(g_db, querystr);
	unlock_db_mutex(TABLE_RRM_STA_STATS, idx);

	/* delete from RRM STA Side Adv statistics table */
	snprintf(querystr, sizeof(querystr),
		"DELETE FROM %s WHERE %s IN (SELECT DISTINCT %s FROM %s "
		"ORDER BY %s ASC LIMIT %d); ",
		TABLE_RRM_STA_ADV_STATS, COL_RRMADVSTATS_TIME, COL_RRMADVSTATS_TIME,
		TABLE_RRM_STA_ADV_STATS, COL_RRMADVSTATS_TIME, MAX_DELETE_ROWS);
	lock_db_mutex(TABLE_RRM_STA_ADV_STATS, &idx);
	rval = execute_sql_statement(g_db, querystr);
	unlock_db_mutex(TABLE_RRM_STA_ADV_STATS, idx);

	/* delete from AMPDU Stats table */
	/* First check the number of rows */
	snprintf(querystr, sizeof(querystr), "SELECT COUNT(DISTINCT %s) AS count FROM %s; ",
		COL_AMPDU_TIME, TABLE_AMPDUSTATS);
	ncount = get_number_of_rows(TABLE_AMPDUSTATS, querystr);
	if (ncount > (MAX_DELETE_ROWS + 10)) {
		snprintf(querystr, sizeof(querystr),
			"DELETE FROM %s WHERE %s IN (SELECT DISTINCT %s FROM %s "
			"ORDER BY %s ASC LIMIT %d); ",
			TABLE_AMPDUSTATS, COL_AMPDU_TIME, COL_AMPDU_TIME, TABLE_AMPDUSTATS,
			COL_AMPDU_TIME, MAX_DELETE_ROWS);
		lock_db_mutex(TABLE_AMPDUSTATS, &idx);
		rval = execute_sql_statement(g_db, querystr);
		unlock_db_mutex(TABLE_AMPDUSTATS, idx);
	}

	lock_db_mutex(TABLE_GRAPHS, &idx);

	/* Now query the graphs table to get the table name */
	snprintf(querystr, sizeof(querystr), "SELECT DISTINCT %s FROM %s",
		COL_GRAPH_TABLE, TABLE_GRAPHS);

	rval = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	if (rval != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		unlock_db_mutex(TABLE_GRAPHS, idx);
		return rval;
	}
	unlock_db_mutex(TABLE_GRAPHS, idx);

	for (i = 0; i < nRows; i++) {
		char tablename[MAX_TABLE_NAME];

		snprintf(tablename, sizeof(tablename), "%s", qResults[(i+1)*nCols]);

		/* First check the number of rows */
		snprintf(querystr, sizeof(querystr), "SELECT COUNT(DISTINCT %s) AS count FROM %s; ",
			COL_CGRAPH_TIME, tablename);
		ncount = get_number_of_rows(tablename, querystr);
		if (ncount > (MAX_DELETE_ROWS + 10)) {
			snprintf(querystr, sizeof(querystr),
				"DELETE FROM %s WHERE %s IN (SELECT DISTINCT %s FROM %s "
				"ORDER BY %s ASC LIMIT %d); ", tablename, COL_CGRAPH_TIME,
				COL_CGRAPH_TIME, tablename, COL_CGRAPH_TIME, MAX_DELETE_ROWS);
			lock_db_mutex(tablename, &idx);
			rval = execute_sql_statement(g_db, querystr);
			unlock_db_mutex(tablename, idx);
		}
	}
	sqlite3_free_table(qResults);

	return rval;
}

/* Get the list of graph names from table */
int
get_graphs_list_from_db()
{
#ifdef STORE_GRAPHS_IN_LIST
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	char	querystr[256];
	int	ret, nRows = 0, nCols = 0, i;

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, %s FROM %s",
		COL_GRAPH_ROWID, COL_GRAPH_TAB, COL_GRAPH_TABLE,
		COL_GRAPH_NAME, TABLE_GRAPHS);

	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}

	for (i = 0; i < nRows; i++) {
		add_graph_to_list(atoi(qResults[(i+1)*nCols]), (qResults[(i+1)*nCols+3]),
			qResults[(i+1)*nCols+2], qResults[(i+1)*nCols+1]);
	}

	sqlite3_free_table(qResults);

end:

	return ret;
#else
	return 0;
#endif /* STORE_GRAPHS_IN_LIST */
}

/* Gets the list of graph names from table to send it to Web server for
 * config request. Based on graph names the web UI will enable the tabs
 */
graphnamelist_t*
get_json_for_graphs()
{
	char		*szErrMsg = NULL;
	char		**qResults = NULL;
	char		querystr[256];
	int		ret, nRows = 0, nCols = 0, idx = -1, i;
	graphnamelist_t	*graphnames = NULL;

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, "
		"%s FROM %s",
		COL_GRAPH_TAB, COL_GRAPH_NAME, COL_GRAPH_HEADING,
		COL_GRAPH_PLOTWITH, COL_GRAPH_TYPE, COL_GRAPH_PERSTA,
		COL_GRAPH_BARHEADING, COL_GRAPH_XAXISNAME, COL_GRAPH_YAXISNAME,
		COL_GRAPH_ENABLE, TABLE_GRAPHS);

	lock_db_mutex(TABLE_GRAPHS, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}

	graphnames = (graphnamelist_t*)malloc(sizeof(graphnamelist_t) +
		(sizeof(graphname_t) * nRows));

	if (graphnames == NULL) {
		VIS_DB("Failed to allocate memory to graphnames for %d rows\n",
			nRows);
		sqlite3_free_table(qResults);
		unlock_db_mutex(TABLE_GRAPHS, idx);
		return NULL;
	}

	graphnames->length = nRows;

	for (i = 0; i < nRows; i++) {
		snprintf(graphnames->graphname[i].tab, sizeof(graphnames->graphname[i].tab), "%s",
			qResults[(i+1)*nCols]);
		snprintf(graphnames->graphname[i].name, sizeof(graphnames->graphname[i].name),
			"%s", qResults[(i+1)*nCols+1]);
		snprintf(graphnames->graphname[i].heading, sizeof(graphnames->graphname[i].heading),
			"%s", qResults[(i+1)*nCols+2]);
		snprintf(graphnames->graphname[i].plotwith,
			sizeof(graphnames->graphname[i].plotwith), "%s", qResults[(i+1)*nCols+3]);
		graphnames->graphname[i].type = atoi(qResults[(i+1)*nCols+4]);
		graphnames->graphname[i].persta = atoi(qResults[(i+1)*nCols+5]);
		snprintf(graphnames->graphname[i].barheading,
			sizeof(graphnames->graphname[i].barheading), "%s", qResults[(i+1)*nCols+6]);
		snprintf(graphnames->graphname[i].xaxisname,
			sizeof(graphnames->graphname[i].xaxisname), "%s", qResults[(i+1)*nCols+7]);
		snprintf(graphnames->graphname[i].yaxisname,
			sizeof(graphnames->graphname[i].yaxisname), "%s", qResults[(i+1)*nCols+8]);
		graphnames->graphname[i].enable = atoi(qResults[(i+1)*nCols+9]);
	}

	sqlite3_free_table(qResults);

end:
	unlock_db_mutex(TABLE_GRAPHS, idx);
	return graphnames;
}

/* Updates the graph details tables enable flag */
int
update_enable_graphs(onlygraphnamelist_t *graphname)
{
	char	querystr[1024];
	int	rval = 0, idx = -1, i;
	char	names[512] = "", *cur = names;

	SHOW_TIME_ELAPSE_START();

	lock_db_mutex(TABLE_GRAPHS, &idx);

	snprintf(querystr, sizeof(querystr), "UPDATE %s SET %s = '0';",
		TABLE_GRAPHS, COL_GRAPH_ENABLE);
	rval = execute_sql_statement(g_db, querystr);

	if (graphname != NULL) {
		cur += snprintf(cur, names + sizeof(names) - cur, "(");
		for (i = 0; i < graphname->length; i++) {
			/* Add all the individual graph name in the format
			 * "'graphname',". for the end string add ')' instead
			 * of ','
			 */
			cur += snprintf(cur, names + sizeof(names) - cur, "'%s'%s",
				graphname->graphname[i].name,
				(i < (graphname->length - 1)) ? "," : ")");
		}

		snprintf(querystr, sizeof(querystr), "UPDATE %s SET %s = '1' WHERE %s IN %s;",
			TABLE_GRAPHS, COL_GRAPH_ENABLE, COL_GRAPH_NAME, names);
		rval = execute_sql_statement(g_db, querystr);
	}

	unlock_db_mutex(TABLE_GRAPHS, idx);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Get the channel statistics details from table to send it to web server */
congestion_list_t*
get_channel_capacity_from_db(int rowid,
	int freqband, int curchannel, int *capacity)
{
	char			*szErrMsg = NULL;
	char			**qResults = NULL;
	char			querystr[512];
	int			i = 0, nRows = 0, nCols = 0, ret = 0, idx = -1, j = 0;
	congestion_list_t	*congestion = NULL;

	SHOW_TIME_ELAPSE_START();

	snprintf(querystr, sizeof(querystr),
		"SELECT %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s "
		"FROM %s "
		"WHERE %s = '%d' AND %s = (SELECT %s FROM %s WHERE %s = '%d' "
		"ORDER BY %s DESC LIMIT 1); ",
		COL_CHSTATS_CHANNEL, COL_CHSTATS_TX, COL_CHSTATS_INBSS,
		COL_CHSTATS_OBSS, COL_CHSTATS_NOCAT, COL_CHSTATS_NOPKT,
		COL_CHSTATS_DOZE, COL_CHSTATS_TXOP, COL_CHSTATS_GOODTX,
		COL_CHSTATS_BADTX, COL_CHSTATS_GLITCH, COL_CHSTATS_BADPLCP,
		COL_CHSTATS_KNOISE, COL_CHSTATS_IDLE,
		TABLE_CHANNELSTATS,
		COL_CHSTATS_ROWID, rowid,
		COL_CHSTATS_TIME, COL_CHSTATS_TIME, TABLE_CHANNELSTATS,
		COL_CHSTATS_ROWID, rowid, COL_CHSTATS_TIME);

	lock_db_mutex(TABLE_CHANNELSTATS, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	unlock_db_mutex(TABLE_CHANNELSTATS, idx);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		return NULL;
	}

	congestion = (congestion_list_t*)malloc(sizeof(congestion_list_t) +
		(sizeof(congestion_t) * nRows));
	if (congestion == NULL) {
		VIS_DB("Failed to allocate congestion buffer for %d rows\n",
			nRows);
		sqlite3_free_table(qResults);
		return NULL;
	}
	congestion->length = nRows;

	for (i = 0; i < nRows; i++) {
		congestion->congest[j].channel = atoi(qResults[(i+1)*nCols+0]);
		congestion->congest[j].tx = atoi(qResults[(i+1)*nCols+1]);
		congestion->congest[j].inbss = atoi(qResults[(i+1)*nCols+2]);
		congestion->congest[j].obss = atoi(qResults[(i+1)*nCols+3]);
		congestion->congest[j].nocat = atoi(qResults[(i+1)*nCols+4]);
		congestion->congest[j].nopkt = atoi(qResults[(i+1)*nCols+5]);
		congestion->congest[j].doze = atoi(qResults[(i+1)*nCols+6]);
		congestion->congest[j].txop = atoi(qResults[(i+1)*nCols+7]);
		congestion->congest[j].goodtx = atoi(qResults[(i+1)*nCols+8]);
		congestion->congest[j].badtx = atoi(qResults[(i+1)*nCols+9]);
		congestion->congest[j].glitchcnt = atoi(qResults[(i+1)*nCols+10]);
		congestion->congest[j].badplcp = atoi(qResults[(i+1)*nCols+11]);
		congestion->congest[j].knoise = atoi(qResults[(i+1)*nCols+12]);
		congestion->congest[j].chan_idle = atoi(qResults[(i+1)*nCols+13]);

		VIS_DB("Channel Stats : %d\t,%d\t,%d\t,%d\t,%d\t,%d\t,%d\t,%d\t,%d\t,%d\t,"
			"%d\t,%d\t,%d\t,%d\n",
			atoi(qResults[(i+1)*nCols+0]), atoi(qResults[(i+1)*nCols+1]),
			atoi(qResults[(i+1)*nCols+2]), atoi(qResults[(i+1)*nCols+3]),
			atoi(qResults[(i+1)*nCols+4]), atoi(qResults[(i+1)*nCols+5]),
			atoi(qResults[(i+1)*nCols+6]), atoi(qResults[(i+1)*nCols+7]),
			atoi(qResults[(i+1)*nCols+8]), atoi(qResults[(i+1)*nCols+9]),
			atoi(qResults[(i+1)*nCols+10]), atoi(qResults[(i+1)*nCols+11]),
			atoi(qResults[(i+1)*nCols+12]), atoi(qResults[(i+1)*nCols+13]));

		/* Calculate busy, available capacity, wifi and nonwifi interference */
		congestion->congest[j].busy = (congestion->congest[j].inbss +
			congestion->congest[j].obss + congestion->congest[j].goodtx +
			congestion->congest[j].badtx + congestion->congest[j].nocat +
			congestion->congest[j].nopkt);
		congestion->congest[j].availcap = (congestion->congest[j].inbss +
			congestion->congest[j].goodtx + congestion->congest[j].badtx +
			congestion->congest[j].txop);
		congestion->congest[j].wifi = congestion->congest[j].obss;
		congestion->congest[j].nonwifi = (congestion->congest[j].nocat +
			congestion->congest[j].nopkt);

		if (curchannel == congestion->congest[j].channel) {
			*capacity = congestion->congest[j].availcap;
		}
		if (freqband == 5) {
			if (congestion->congest[j].channel <= 14) {
				congestion->length--;
				continue;
			}
		} else {
			if (congestion->congest[j].channel > 14) {
				congestion->length--;
				continue;
			}
		}
		j++;
	}

	sqlite3_free_table(qResults);

	SHOW_TIME_ELAPSE_END();

	return congestion;
}

/* Get Channel MAP from site survey page to display it in channel statistics page */
channel_maplist_t*
get_channel_map(int rowid)
{
	char			*szErrMsg = NULL;
	char			**qResults = NULL;
	char			querystr[512];
	int			i = 0, nRows = 0, nCols = 0, ret = 0, idx = -1;
	channel_maplist_t	*channelmapout = NULL;

	SHOW_TIME_ELAPSE_START();

	/*
	 * There was request to show the last 5 scan results in channel distribution graph
	 * so that the jumping of the AP's can be reduced(SWWLAN-48962).
	 * For that we are getting the scanresult for last 5 distinct timestamps.
	 * And also sorting the result based on SSID in ascending order
	 */
	snprintf(querystr, sizeof(querystr), "SELECT DISTINCT %s, %s, %s, "
		"%s, %s, %s FROM %s "
		"WHERE %s = '%d' AND %s != '0' "
		"AND %s >= (SELECT %s FROM %s WHERE %s = '%d' "
		"ORDER BY %s DESC LIMIT 1)-25 ORDER BY %s ASC; ",
		COL_SCAN_SSID, COL_SCAN_CHANNEL, COL_SCAN_BANDWIDTH,
		COL_SCAN_CONTROLCH, COL_SCAN_RSSI, COL_SCAN_BSSID, TABLE_SCAN,
		COL_SCAN_ROWID, rowid, COL_SCAN_BSSID,
		COL_SCAN_TIME, COL_SCAN_TIME, TABLE_SCAN, COL_SCAN_ROWID, rowid,
		COL_SCAN_TIME, COL_SCAN_SSID);

	lock_db_mutex(TABLE_SCAN, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	unlock_db_mutex(TABLE_SCAN, idx);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		return NULL;
	}
	channelmapout = (channel_maplist_t*)malloc(sizeof(channel_maplist_t) +
		(sizeof(channel_map_t) * nRows));
	if (channelmapout == NULL) {
		VIS_DB("Failed to allocate channelmapout buffer for %d rows\n",
			nRows);
		sqlite3_free_table(qResults);
		return NULL;
	}
	channelmapout->length = nRows;

	for (i = 0; i < nRows; i++) {
		snprintf(channelmapout->channelmap[i].ssid,
			sizeof(channelmapout->channelmap[i].ssid), "%s", qResults[(i+1)*nCols]);
		channelmapout->channelmap[i].channel = atoi(qResults[(i+1)*nCols+1]);
		channelmapout->channelmap[i].bandwidth = atoi(qResults[(i+1)*nCols+2]);
		channelmapout->channelmap[i].ctrlch = atoi(qResults[(i+1)*nCols+3]);
		channelmapout->channelmap[i].rssi = atoi(qResults[(i+1)*nCols+4]);

		VIS_DB("SSID : %s\n", qResults[(i+1)*nCols]);
		VIS_DB("Channel : %d\n", atoi(qResults[(i+1)*nCols+1]));
		VIS_DB("Bandwidth : %d\n", atoi(qResults[(i+1)*nCols+2]));
		VIS_DB("Control Channel : %d\n", atoi(qResults[(i+1)*nCols+3]));
		VIS_DB("RSSI : %d\n", atoi(qResults[(i+1)*nCols+4]));
	}

	sqlite3_free_table(qResults);

	SHOW_TIME_ELAPSE_END();

	return channelmapout;
}

/* Get the AMPDU statistics details from table to send it to web server */
tx_ampdu_list_t*
get_ampdu_stats_from_db(int rowid, int freqband)
{
	char			*szErrMsg = NULL;
	char			**qResults = NULL;
	char			querystr[512];
	int			i = 0, nRows = 0, nCols = 0, ret = 0, idx = -1;
	tx_ampdu_list_t		*ampdulist = NULL;

	SHOW_TIME_ELAPSE_START();

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s "
		"FROM %s "
		"WHERE %s = '%d' AND %s = (SELECT %s FROM %s WHERE %s = '%d' "
		"ORDER BY %s DESC LIMIT 1); ",
		COL_AMPDU_MCS, COL_AMPDU_TXMCS, COL_AMPDU_TXMCSPERCENT,
		COL_AMPDU_TXMCSSGI, COL_AMPDU_TXMCSSGIPERCENT, COL_AMPDU_RXMCS,
		COL_AMPDU_RXMCSPERCENT, COL_AMPDU_RXMCSSGI, COL_AMPDU_RXMCSSGIPERCENT,
		COL_AMPDU_TXVHT, COL_AMPDU_TXVHTPER, COL_AMPDU_TXVHTPERCENT,
		COL_AMPDU_TXVHTSGI, COL_AMPDU_TXVHTSGIPER, COL_AMPDU_TXVHTSGIPERCENT,
		COL_AMPDU_RXVHT, COL_AMPDU_RXVHTPER, COL_AMPDU_RXVHTPERCENT,
		COL_AMPDU_RXVHTSGI, COL_AMPDU_RXVHTSGIPER, COL_AMPDU_RXVHTSGIPERCENT,
		COL_AMPDU_MPDUDENS, COL_AMPDU_MPDUDENSPERCENT,
		TABLE_AMPDUSTATS,
		COL_AMPDU_ROWID, rowid,
		COL_AMPDU_TIME, COL_AMPDU_TIME, TABLE_AMPDUSTATS,
		COL_AMPDU_ROWID, rowid, COL_AMPDU_TIME);

	lock_db_mutex(TABLE_AMPDUSTATS, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	unlock_db_mutex(TABLE_AMPDUSTATS, idx);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		return NULL;
	}

	ampdulist = (tx_ampdu_list_t*)malloc(sizeof(tx_ampdu_list_t) +
		(sizeof(tx_ampdu_t) * nRows));
	if (ampdulist == NULL) {
		VIS_DB("Failed to allocate ampdulist buffer for %d rows\n",
			nRows);
		sqlite3_free_table(qResults);
		return NULL;
	}
	ampdulist->length = nRows;

	for (i = 0; i < nRows; i++) {
		ampdulist->ampdus[i].mcs = atoi(qResults[(i+1)*nCols+0]);
		ampdulist->ampdus[i].txmcs = atoi(qResults[(i+1)*nCols+1]);
		ampdulist->ampdus[i].txmcspercent = atoi(qResults[(i+1)*nCols+2]);
		ampdulist->ampdus[i].txmcssgi = atoi(qResults[(i+1)*nCols+3]);
		ampdulist->ampdus[i].txmcssgipercent = atoi(qResults[(i+1)*nCols+4]);

		ampdulist->ampdus[i].rxmcs = atoi(qResults[(i+1)*nCols+5]);
		ampdulist->ampdus[i].rxmcspercent = atoi(qResults[(i+1)*nCols+6]);
		ampdulist->ampdus[i].rxmcssgi = atoi(qResults[(i+1)*nCols+7]);
		ampdulist->ampdus[i].rxmcssgipercent = atoi(qResults[(i+1)*nCols+8]);

		ampdulist->ampdus[i].txvht = atoi(qResults[(i+1)*nCols+9]);
		ampdulist->ampdus[i].txvhtper = atoi(qResults[(i+1)*nCols+10]);
		ampdulist->ampdus[i].txvhtpercent = atoi(qResults[(i+1)*nCols+11]);
		ampdulist->ampdus[i].txvhtsgi = atoi(qResults[(i+1)*nCols+12]);
		ampdulist->ampdus[i].txvhtsgiper = atoi(qResults[(i+1)*nCols+13]);
		ampdulist->ampdus[i].txvhtsgipercent = atoi(qResults[(i+1)*nCols+14]);

		ampdulist->ampdus[i].rxvht = atoi(qResults[(i+1)*nCols+15]);
		ampdulist->ampdus[i].rxvhtper = atoi(qResults[(i+1)*nCols+16]);
		ampdulist->ampdus[i].rxvhtpercent = atoi(qResults[(i+1)*nCols+17]);
		ampdulist->ampdus[i].rxvhtsgi = atoi(qResults[(i+1)*nCols+18]);
		ampdulist->ampdus[i].rxvhtsgiper = atoi(qResults[(i+1)*nCols+19]);
		ampdulist->ampdus[i].rxvhtsgipercent = atoi(qResults[(i+1)*nCols+20]);

		ampdulist->ampdus[i].mpdudens = atoi(qResults[(i+1)*nCols+21]);
		ampdulist->ampdus[i].mpdudenspercent = atoi(qResults[(i+1)*nCols+22]);
	}

	sqlite3_free_table(qResults);

	SHOW_TIME_ELAPSE_END();

	return ampdulist;
}

/* Saves gateway IP to config table */
int
update_gatewayip_in_db(char *gatewayip)
{
	char	querystr[256];
	int	rval = 0, idx = -1;

	SHOW_TIME_ELAPSE_START();

	lock_db_mutex(TABLE_CONFIG, &idx);

	snprintf(querystr, sizeof(querystr), "UPDATE %s SET %s = '%s'; ",
		TABLE_CONFIG, COL_CONFIG_GATEWAY_IP, gatewayip);
	rval = execute_sql_statement(g_db, querystr);

	unlock_db_mutex(TABLE_CONFIG, idx);

	SHOW_TIME_ELAPSE_END();

	return rval;
}

/* Reallocate the memory for all DUT Settings structure */
static int
reallocate_alldut_settings(int nRows)
{
	alldut_settings_t *tmp = NULL;
	int i;
	int realloclen = (sizeof(alldut_settings_t) +
		(sizeof(dut_settings_t)*(nRows+MAX_DUTS)));

	if (g_alldutsettings == NULL) {
		g_alldutsettings = (alldut_settings_t*)malloc(realloclen);
		if (g_alldutsettings == NULL) {
			VIS_DB("Failed to allocate memory g_alldutsettings of %d\n", realloclen);
			return -1;
		}
		memset(g_alldutsettings, 0x00, realloclen);
		g_alldutsettings->ncount = 0;
	} else {
		tmp = (alldut_settings_t*)realloc(g_alldutsettings, realloclen);
		if (tmp == NULL) {
			VIS_DB("Failed to reallocate memory g_alldutsettings of %d\n",
				nRows+MAX_DUTS);
			return -1;
		}
		g_alldutsettings = tmp;
		for (i = g_alldutsettings->length; i < (nRows+MAX_DUTS); i++) {
			g_alldutsettings->dutset[i].rowid = 0;
		}
	}
	g_alldutsettings->length = (nRows+MAX_DUTS);

	return 0;
}

/* Get the DUT settings for all DUTs from TABLE_DUTSETTINGS table */
int
get_alldut_settings_from_db()
{
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	char	querystr[256];
	int	i = 0, nRows = 0, nCols = 0, ret = 0, idx = -1;

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s FROM %s",
		COL_DUTSET_ROWID, COL_DUTSET_SCAN, TABLE_DUTSETTINGS);

	lock_db_mutex(TABLE_DUTSETTINGS, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	unlock_db_mutex(TABLE_DUTSETTINGS, idx);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}

	lock_alldutsettings_mutex();
	if (g_alldutsettings == NULL || nRows > g_alldutsettings->length) { /* Reallocate memory */
		if (reallocate_alldut_settings(nRows) == -1) {
			unlock_alldutsettings_mutex();
			sqlite3_free_table(qResults);
			return -1;
		}
	}
	for (i = 0; i < nRows; i++) {
		g_alldutsettings->dutset[i].rowid = atoi(qResults[(i+1)*nCols+0]);
		g_alldutsettings->dutset[i].isscan = atoi(qResults[(i+1)*nCols+1]);
		g_alldutsettings->ncount++;

		VIS_DB("rowID : %d\t IsScan : %d\n\n",
			g_alldutsettings->dutset[i].rowid, g_alldutsettings->dutset[i].isscan);
	}
	unlock_alldutsettings_mutex();

	sqlite3_free_table(qResults);
end:

	return 0;
}

/* Update DUT settings table */
int
update_dut_settings_in_db(dut_settings_t *dutset)
{
	char	querystr[256];
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	int	rval = 0, nRows = 0, nCols = 0, idx = -1, i, found = 0;

	lock_db_mutex(TABLE_DUTSETTINGS, &idx);

	snprintf(querystr, sizeof(querystr), "SELECT %s FROM %s WHERE %s = '%d'",
		COL_DUTSET_SCAN, TABLE_DUTSETTINGS, COL_DUTSET_ROWID, dutset->rowid);
	rval = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	if (rval != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}
	if (nRows > 0) { /* UPDATE */
		snprintf(querystr, sizeof(querystr), "UPDATE %s SET %s = '%d' WHERE %s = '%d'; ",
			TABLE_DUTSETTINGS, COL_DUTSET_SCAN, dutset->isscan,
			COL_DUTSET_ROWID, dutset->rowid);
		rval = execute_sql_statement(g_db, querystr);
		/* Also update it in All DUT settings structure */
		lock_alldutsettings_mutex();
		if (g_alldutsettings) {
			for (i = 0; i < g_alldutsettings->ncount; i++) {
				if (g_alldutsettings->dutset[i].rowid == dutset->rowid) {
					g_alldutsettings->dutset[i].isscan = dutset->isscan;
					break;
				}
			}
		}
		unlock_alldutsettings_mutex();
	}
	else { /* INSERT */
		snprintf(querystr, sizeof(querystr), "INSERT INTO %s (%s, %s) "
			"VALUES ('%d', '%d'); ",
			TABLE_DUTSETTINGS, COL_DUTSET_ROWID, COL_DUTSET_SCAN,
			dutset->rowid, dutset->isscan);
		rval = execute_sql_statement(g_db, querystr);
		/* also add it into all DUT settings structure */
		lock_alldutsettings_mutex();
		if (g_alldutsettings) {
			for (i = 0; i < g_alldutsettings->ncount; i++) {
				if (g_alldutsettings->dutset[i].rowid == dutset->rowid) {
					g_alldutsettings->dutset[i].isscan = dutset->isscan;
					found = 1;
					break;
				}
			}
			if (found == 0) {
				if (g_alldutsettings->length < (g_alldutsettings->ncount + 1))
					reallocate_alldut_settings(g_alldutsettings->ncount+1);
				g_alldutsettings->dutset[g_alldutsettings->ncount].rowid =
					dutset->rowid;
				g_alldutsettings->dutset[g_alldutsettings->ncount].isscan =
					dutset->isscan;
				g_alldutsettings->ncount++;
			}
		}
		unlock_alldutsettings_mutex();
	}
	sqlite3_free_table(qResults);

end:
	unlock_db_mutex(TABLE_DUTSETTINGS, idx);

	return rval;
}

/* Get the DUT settings for the DUT */
void
get_current_dut_settings(int rowid, dut_settings_t *dutset)
{
	int i;

	lock_alldutsettings_mutex();
	if (g_alldutsettings) {
		for (i = 0; i < g_alldutsettings->ncount; i++) {
			if (g_alldutsettings->dutset[i].rowid == rowid) {
				dutset->isscan = g_alldutsettings->dutset[i].isscan;
				break;
			}
		}
	}
	unlock_alldutsettings_mutex();
}

/* Get the number of rows in a table */
int
get_number_of_rows(char *table, char *query)
{
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	int	i = 0, nRows = 0, nCols = 0, ret = 0, idx = -1;

	lock_db_mutex(table, &idx);
	ret = sqlite3_get_table(g_db, query, &qResults, &nRows, &nCols,
		&szErrMsg);
	unlock_db_mutex(table, idx);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}
	if (nRows > 0) {
		nRows = atoi(qResults[(i+1)*nCols+0]);
	}

	sqlite3_free_table(qResults);
end:

	return nRows;
}

/* Get the channel statistics for current channel */
congestion_list_t*
get_chanim_from_db(int rowid, int freqband, int curchannel)
{
	char			*szErrMsg = NULL;
	char			**qResults = NULL;
	char			querystr[1024];
	int			i = 0, nRows = 0, nCols = 0, ret = 0, idx = -1, j = 0;
	congestion_list_t	*congestion = NULL;

	SHOW_TIME_ELAPSE_START();

	snprintf(querystr, sizeof(querystr),
		"SELECT %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s "
		"FROM %s "
		"WHERE %s = '%d' AND %s = '%d' ORDER BY %s DESC LIMIT %d; ",
		COL_CHSTATS_CHANNEL, COL_CHSTATS_TX, COL_CHSTATS_INBSS,
		COL_CHSTATS_OBSS, COL_CHSTATS_NOCAT, COL_CHSTATS_NOPKT,
		COL_CHSTATS_DOZE, COL_CHSTATS_TXOP, COL_CHSTATS_GOODTX,
		COL_CHSTATS_BADTX, COL_CHSTATS_GLITCH, COL_CHSTATS_BADPLCP,
		COL_CHSTATS_KNOISE, COL_CHSTATS_IDLE, COL_CHSTATS_TIME,
		TABLE_CHANNELSTATS,
		COL_CHSTATS_ROWID, rowid, COL_CHSTATS_CHANNEL, curchannel,
		COL_CHSTATS_TIME, MAX_GRAPH_ENTRY);

	lock_db_mutex(TABLE_CHANNELSTATS, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	unlock_db_mutex(TABLE_CHANNELSTATS, idx);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		return NULL;
	}

	congestion = (congestion_list_t*)malloc(sizeof(congestion_list_t) +
		(sizeof(congestion_t) * nRows));
	if (congestion == NULL) {
		VIS_DB("Failed to allocate congestion buffer for %d rows\n",
			nRows);
		sqlite3_free_table(qResults);
		return NULL;
	}
	congestion->length = nRows;
	for (i = 0; i < nRows; i++) {
		congestion->congest[j].channel = atoi(qResults[(i+1)*nCols+0]);
		congestion->congest[j].tx = atoi(qResults[(i+1)*nCols+1]);
		congestion->congest[j].inbss = atoi(qResults[(i+1)*nCols+2]);
		congestion->congest[j].obss = atoi(qResults[(i+1)*nCols+3]);
		congestion->congest[j].nocat = atoi(qResults[(i+1)*nCols+4]);
		congestion->congest[j].nopkt = atoi(qResults[(i+1)*nCols+5]);
		congestion->congest[j].doze = atoi(qResults[(i+1)*nCols+6]);
		congestion->congest[j].txop = atoi(qResults[(i+1)*nCols+7]);
		congestion->congest[j].goodtx = atoi(qResults[(i+1)*nCols+8]);
		congestion->congest[j].badtx = atoi(qResults[(i+1)*nCols+9]);
		congestion->congest[j].glitchcnt = atoi(qResults[(i+1)*nCols+10]);
		congestion->congest[j].badplcp = atoi(qResults[(i+1)*nCols+11]);
		congestion->congest[j].knoise = atoi(qResults[(i+1)*nCols+12]);
		congestion->congest[j].chan_idle = atoi(qResults[(i+1)*nCols+13]);
		congestion->congest[j].timestamp = atoi(qResults[(i+1)*nCols+14]);

		VIS_DB("Channel Stats : %d\t,%d\t,%d\t,%d\t,%d\t,%d\t,%d\t,%d\t,%d\t,%d\t,"
			"%d\t,%d\t,%d\t,%d\n",
			atoi(qResults[(i+1)*nCols+0]), atoi(qResults[(i+1)*nCols+1]),
			atoi(qResults[(i+1)*nCols+2]), atoi(qResults[(i+1)*nCols+3]),
			atoi(qResults[(i+1)*nCols+4]), atoi(qResults[(i+1)*nCols+5]),
			atoi(qResults[(i+1)*nCols+6]), atoi(qResults[(i+1)*nCols+7]),
			atoi(qResults[(i+1)*nCols+8]), atoi(qResults[(i+1)*nCols+9]),
			atoi(qResults[(i+1)*nCols+10]), atoi(qResults[(i+1)*nCols+11]),
			atoi(qResults[(i+1)*nCols+12]), atoi(qResults[(i+1)*nCols+13]));

		if (freqband == 5) {
			if (congestion->congest[j].channel <= 14) {
				congestion->length--;
				continue;
			}
		} else {
			if (congestion->congest[j].channel > 14) {
				congestion->length--;
				continue;
			}
		}
		j++;
	}

	sqlite3_free_table(qResults);

	SHOW_TIME_ELAPSE_END();

	return congestion;
}

/* Gets All the DUTs(interfaces) from the table */
dut_list_t*
get_all_duts_from_db()
{
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	char	querystr[MAX_QUERY_SIZE];
	int	ret, nRows = 0, nCols = 0, idx = -1, i = 0;

	dut_list_t *dutlist = NULL;

	lock_db_mutex(TABLE_DUTDETAILS, &idx);

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, %s FROM %s WHERE %s = '1';",
		COL_DUT_SSID, COL_DUT_BSSID, COL_DUT_BAND, COL_DUT_MAC, TABLE_DUTDETAILS,
		COL_DUT_ISENABLED);

	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}

	dutlist = (dut_list_t*)malloc(sizeof(dut_list_t) +
		(sizeof(dut_info_t) * nRows));
	if (dutlist == NULL) {
		VIS_DB("%s : Failed to allocate dutlist buffer for %d rows\n",
			__FUNCTION__, nRows);
		sqlite3_free_table(qResults);
		unlock_db_mutex(TABLE_DUTDETAILS, idx);
		return NULL;
	}
	dutlist->length = nRows;

	for (i = 0; i < nRows; i++) {
		snprintf(dutlist->duts[i].ssid, sizeof(dutlist->duts[i].ssid), "%s",
			qResults[(i+1)*nCols+0]);
		snprintf(dutlist->duts[i].bssid, sizeof(dutlist->duts[i].bssid), "%s",
			qResults[(i+1)*nCols+1]);
		dutlist->duts[i].band = atoi(qResults[(i+1)*nCols+2]);
		snprintf(dutlist->duts[i].mac, sizeof(dutlist->duts[i].mac), "%s",
			qResults[(i+1)*nCols+3]);

		VIS_DB("SSID : %s\nBSSID : %s\nband : %d\nMAC : %s\n",
			dutlist->duts[i].ssid, dutlist->duts[i].bssid,
			dutlist->duts[i].band, dutlist->duts[i].mac);
	}

	sqlite3_free_table(qResults);

end:
	unlock_db_mutex(TABLE_DUTDETAILS, idx);

	return dutlist;
}

/* Gets RRM STA Stats from the table */
int
get_stastats_from_db(int rowid, char *mac, vis_sta_stats_t *stastats)
{
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	char	querystr[MAX_QUERY_SIZE];
	int	ret, nRows = 0, nCols = 0, idx = -1, i = 0;

	lock_db_mutex(TABLE_RRM_STA_STATS, &idx);

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, "
		"%s, %s "
		"FROM %s "
		"WHERE %s = '%d' AND %s = '%s' "
		"AND %s = (SELECT %s FROM %s WHERE %s = '%d' AND %s = '%s' "
		"ORDER BY %s DESC LIMIT 1); ",
		COL_RRMSTATS_GENERATION, COL_RRMSTATS_TXSTREAM, COL_RRMSTATS_RXSTREAM,
		COL_RRMSTATS_RATE, COL_RRMSTATS_IDLE,
		TABLE_RRM_STA_STATS,
		COL_RRMSTATS_ROWID, rowid, COL_RRMSTATS_MAC, mac,
		COL_RRMSTATS_TIME, COL_RRMSTATS_TIME, TABLE_RRM_STA_STATS,
		COL_RRMSTATS_ROWID, rowid, COL_RRMSTATS_MAC, mac, COL_RRMSTATS_TIME);

	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}
	if (nRows > 0) {
		snprintf(stastats->mac, sizeof(stastats->mac), "%s", mac);
		stastats->generation_flag = atoi(qResults[(i+1)*nCols+0]);
		stastats->txstream = atoi(qResults[(i+1)*nCols+1]);
		stastats->rxstream = atoi(qResults[(i+1)*nCols+2]);
		stastats->rate = atoi(qResults[(i+1)*nCols+3]);
		stastats->idle = atoi(qResults[(i+1)*nCols+4]);

		VIS_DB("MAC : %s\nGeneration : %d\nTX Stream : %d\nRX Stream : %d\n"
			"Rate : %d\nIdle : %d\n", stastats->mac,
			stastats->generation_flag, stastats->txstream, stastats->rxstream,
			stastats->rate, stastats->idle);
	}

	sqlite3_free_table(qResults);

end:
	unlock_db_mutex(TABLE_RRM_STA_STATS, idx);

	return 1;
}

/* Gets the delta of current and previous snapshots */
static uint32
verify_and_set_rrm_stats_data(uint32 newval, uint32 oldval)
{
	if (newval < oldval || !oldval) {
		newval = 0;
	} else {
		newval = newval - oldval;
	}

	return newval;
}

/* Gets RRM Advanced STA Stats from the table */
vis_rrm_statslist_t*
get_rrm_adv_stastats_from_db(int rowid, char *mac)
{
	char	*szErrMsg = NULL;
	char	**qResults = NULL;
	char	querystr[MAX_QUERY_SIZE];
	int	ret, nRows = 0, nCols = 0, idx = -1, i = 0;
	vis_rrm_statslist_t *rrmstatslist = NULL;

	lock_db_mutex(TABLE_RRM_STA_ADV_STATS, &idx);

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, %s, "
		"%s, %s, %s "
		"FROM %s "
		"WHERE %s = '%d' AND %s = '%s' "
		"ORDER BY %s DESC LIMIT %d; ",
		COL_RRMADVSTATS_TIME, COL_RRMADVSTATS_TXOP, COL_RRMADVSTATS_PKTREQ,
		COL_RRMADVSTATS_PKTDROP, COL_RRMADVSTATS_PKTSTORED, COL_RRMADVSTATS_PKTRETRIED,
		COL_RRMADVSTATS_PKTACKED, TABLE_RRM_STA_ADV_STATS, COL_RRMADVSTATS_ROWID,
		rowid, COL_RRMADVSTATS_MAC, mac, COL_RRMADVSTATS_TIME, MAX_GRAPH_ENTRY + 2);

	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);
	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		goto end;
	}
	/* Since we are sending the delta so minimum rows should be >= 3 */
	if (nRows < 3) {
		sqlite3_free_table(qResults);
		goto end;
	}
	rrmstatslist = (vis_rrm_statslist_t*)malloc(sizeof(vis_rrm_statslist_t) +
		(sizeof(vis_rrm_stats_t) * (nRows - 2)));

	if (rrmstatslist == NULL) {
		VIS_DB("%s : Failed to allocate rrmstatslist buffer for %d rows\n",
			__FUNCTION__, nRows - 2);
		sqlite3_free_table(qResults);
		unlock_db_mutex(TABLE_RRM_STA_ADV_STATS, idx);
		return NULL;
	}
	rrmstatslist->length = nRows - 2;
	VIS_DB("rrmstatslist->length = %d \n ", rrmstatslist->length);
	/* Debug message for diff values of sta side rrm stats */
	VIS_DB_RRM("\n");
	VIS_DB_RRM("Timestamp\t MAC\t\t AirtimeOp\t PktRequested\t"
		"PktDropped\t PktStored\t PktRetried\t PktAcked\t\n");
	for (i = 0; i < nRows - 2; i++) {
		snprintf(rrmstatslist->rrmstats[i].mac,
			sizeof(rrmstatslist->rrmstats[i].mac), "%s", mac);
		rrmstatslist->rrmstats[i].timestamp = atoi(qResults[(i+1)*nCols+0]);
		rrmstatslist->rrmstats[i].txop = atoi(qResults[(i+1)*nCols+1]);
		rrmstatslist->rrmstats[i].pktrequested =
			verify_and_set_rrm_stats_data(atoi(qResults[(i+1)*nCols+2]),
			atoi(qResults[(i+2)*nCols+2]));
		rrmstatslist->rrmstats[i].pktdropped =
			verify_and_set_rrm_stats_data(atoi(qResults[(i+1)*nCols+3]),
			atoi(qResults[(i+2)*nCols+3]));
		rrmstatslist->rrmstats[i].pktstored =
			verify_and_set_rrm_stats_data(atoi(qResults[(i+1)*nCols+4]),
			atoi(qResults[(i+2)*nCols+4]));
		rrmstatslist->rrmstats[i].pktretried =
			verify_and_set_rrm_stats_data(atoi(qResults[(i+1)*nCols+5]),
			atoi(qResults[(i+2)*nCols+5]));
		rrmstatslist->rrmstats[i].pktacked =
			verify_and_set_rrm_stats_data(atoi(qResults[(i+1)*nCols+6]),
			atoi(qResults[(i+2)*nCols+6]));
		rrmstatslist->rrmstats[i].pktstored = rrmstatslist->rrmstats[i].pktrequested -
			rrmstatslist->rrmstats[i].pktdropped;
		rrmstatslist->rrmstats[i].pktacked =
			verify_and_set_rrm_stats_data(rrmstatslist->rrmstats[i].pktrequested,
			(rrmstatslist->rrmstats[i].pktdropped +
			rrmstatslist->rrmstats[i].pktacked));

		VIS_DB_RRM("%ld\t %s\t %d\t\t %d\t\t"
			"%d\t\t %d\t\t %d\t\t %d\n",
			rrmstatslist->rrmstats[i].timestamp,
			rrmstatslist->rrmstats[i].mac, rrmstatslist->rrmstats[i].txop,
			rrmstatslist->rrmstats[i].pktrequested,
			rrmstatslist->rrmstats[i].pktdropped,
			rrmstatslist->rrmstats[i].pktstored,
			rrmstatslist->rrmstats[i].pktretried,
			rrmstatslist->rrmstats[i].pktacked);
	}

	sqlite3_free_table(qResults);

end:
	unlock_db_mutex(TABLE_RRM_STA_ADV_STATS, idx);

	return rrmstatslist;
}

static int
Is_rrm_stats_graph_enabled(int graph_val_bit)
{
	unsigned int bitflag = 1;
	bitflag = bitflag << graph_val_bit;
	return g_sta_rrm_stats_graphs_state & bitflag;
}

static void
set_rrm_stats_graph_enabled(unsigned int graph_val_bit, int isenabled)
{
	unsigned int bitflag = 1;
	bitflag = bitflag << graph_val_bit;
	if (isenabled) {
		g_sta_rrm_stats_graphs_state |= bitflag;
	} else {
		g_sta_rrm_stats_graphs_state &= (~bitflag);
	}
}

/* check whether the graph is enabled or not based on that only data will be prapared for sta */
void
update_sta_graphs_status(onlygraphnamelist_t *graphname)
{
	int index;
	g_sta_rrm_stats_graphs_state = 0;
	for (index = 0; index < graphname->length; index++) {
		if (strcmp(graphname->graphname[index].name, "Packet Requested") == 0)
			set_rrm_stats_graph_enabled(PKT_REQUESTED, 1);
		else if (strcmp(graphname->graphname[index].name, "Packet Dropped") == 0)
			set_rrm_stats_graph_enabled(PKT_DROPPED, 1);
		else if (strcmp(graphname->graphname[index].name, "Packet Stored") == 0)
			set_rrm_stats_graph_enabled(PKT_STORED, 1);
		else if (strcmp(graphname->graphname[index].name, "Packet Retried") == 0)
			set_rrm_stats_graph_enabled(PKT_RETRIED, 1);
		else if (strcmp(graphname->graphname[index].name, "Acked") == 0)
			set_rrm_stats_graph_enabled(PKT_ACKED, 1);
	}
}

/*
 * Gets Pkt Queue Stats from the DB and creates jSON for both pkt
 * queue stats and RRM Advanced STA Stats
 */
int
get_adv_sta_stats_from_db(int sockfd, char *tabname, int rowid, char *mac,
	vis_rrm_statslist_t *rrmstatslist)
{
	char		*szErrMsg = NULL;
	char		**qResults = NULL;
	char		querystr[512];
	int		nRows = 0, nCols = 0, ret = 0, idx = -1, total = 0;
	json_object	*object_main, *alldataobject;
	json_object	*main_data_array, *sub_data_obj1, *sub_data_obj2;
	json_object	*array_graph, *sta_data_array;

	snprintf(querystr, sizeof(querystr), "SELECT %s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s, %s, "
		"%s, %s FROM %s "
		"WHERE %s = '1' AND (%s = '%s' OR %s = '%s' OR %s = '%s'); ",
		COL_GRAPH_ROWID, COL_GRAPH_NAME, COL_GRAPH_HEADING,
		COL_GRAPH_PLOTWITH, COL_GRAPH_TYPE, COL_GRAPH_PERSTA,
		COL_GRAPH_BARHEADING, COL_GRAPH_XAXISNAME, COL_GRAPH_YAXISNAME,
		COL_GRAPH_TABLE, COL_GRAPH_ENABLE, TABLE_GRAPHS, COL_GRAPH_ENABLE,
		COL_GRAPH_NAME, tabname, COL_GRAPH_PLOTWITH, tabname,
		COL_GRAPH_HEADING, tabname);

	lock_db_mutex(TABLE_GRAPHS, &idx);
	ret = sqlite3_get_table(g_db, querystr, &qResults, &nRows, &nCols,
		&szErrMsg);

	if (ret != SQLITE_OK) {
		SQLITE_QUERY_FAIL(szErrMsg);
		unlock_db_mutex(TABLE_GRAPHS, idx);
		return -1;
	}
	int *mainrows = NULL;
	if (nRows > 0) {
		mainrows = (int*)malloc(sizeof(int) * nRows);
		if (mainrows ==  NULL) {
			VIS_DB("Failed to allocate memory to mainrows of %d\n", nRows);
			sqlite3_free_table(qResults);
			unlock_db_mutex(TABLE_GRAPHS, idx);
			return -1;
		}
		memset(mainrows, 0x00, nRows);
	}

	/* Create and add json for Packet Queue Statistics */
	sub_data_obj2 = json_object_new_object();
	json_object_object_add(sub_data_obj2, "Heading",
	json_object_new_string("Packet Queue Statistics"));
	array_graph = add_json_for_graph_data(qResults, tabname, rowid,
		mac, nRows, nCols, &mainrows);
	json_object_object_add(sub_data_obj2, "Data", array_graph);
	total++;

	/* Create and add json for advanced RRM sta stats */
	sta_data_array = json_object_new_array();
	array_graph = rrmstatslist ? add_sta_adv_rrm_stats(rrmstatslist) : NULL;
	if (array_graph)
		json_object_array_add(sta_data_array, array_graph);

	/* Create and add json for advanced RRM sta stats acked */
	array_graph = rrmstatslist ? add_sta_adv_rrm_stats_acked(rrmstatslist) : NULL;
	if (array_graph)
		json_object_array_add(sta_data_array, array_graph);

	json_object_object_add(sub_data_obj2, "STAData", sta_data_array);

	/* Add both the json objects to main array obj */
	main_data_array = json_object_new_array();
	/* Create and add json for STA Airtime Opportunity */
	if (rrmstatslist && (rrmstatslist->length > 0)) {
		sub_data_obj1 = json_object_new_object();
		add_sta_adv_airtimeoppoutunity(rrmstatslist, &sub_data_obj1);
		json_object_array_add(main_data_array, sub_data_obj1);
		total++;
	}
	json_object_array_add(main_data_array, sub_data_obj2);

	/* Add main data array json object to alldata json obj */
	alldataobject = json_object_new_object();
	json_object_object_add(alldataobject, "Total", json_object_new_int(total));
	json_object_object_add(alldataobject, "Data", main_data_array);

	/* Add alldata json object to main json obj */
	object_main = json_object_new_object();
	json_object_object_add(object_main, "Req",
		json_object_new_string(PACKET_REQRRMADVSTASTATS));
	json_object_object_add(object_main, "Status",
		json_object_new_string("Success"));

	/* Add the sta data available element for auto UI refresh */
	if (json_object_array_length(sta_data_array) > 0) {
		json_object_object_add(object_main, "StaDataAvailability",
			json_object_new_string("true"));
	} else {
		json_object_object_add(object_main, "StaDataAvailability",
			json_object_new_string("false"));
	}

	json_object_object_add(object_main, "AllData", alldataobject);

	const char *data = json_object_to_json_string(object_main);

	sqlite3_free_table(qResults);
	unlock_db_mutex(TABLE_GRAPHS, idx);
	if (mainrows) {
		free(mainrows);
		mainrows = NULL;
	}
	if (data) {
		VIS_JSON("%s\n", data);
		ret = add_length_and_send_data(sockfd, data, strlen(data));
	}
	json_object_put(object_main);
	return 0;
}

/* Creates and adds the json data for the pkt queue statistics */
static json_object*
add_json_for_graph_data(char **qResults, char *tabname, int rowid, char *mac,
	int nRows, int nCols, int **mainrows)
{
	json_object *array_x, *array_y_main, *array;
	json_object *object, *array_graph;
	int i = 0, j = 0, k = 0;
	/* nmainrows holds the index values of graphs which are already added to json */
	int placeholder = 1, found = 0, nmainrows = 0;
	array_graph = json_object_new_array();
	for (i = 0; i < nRows; i++) {
		int selrows[10] = {0};
		int selrowscount = 0;

		/* check if the graph row is already added to json */
		found = 0;
		for (k = 0; k < nmainrows; k++) {
			if ((*mainrows)[k] == i) {
				found = 1;
				break;
			}
		}
		if (found == 1)
			continue;
		object = json_object_new_object();

		json_object_object_add(object, "Name",
			json_object_new_string(qResults[(i+1)*nCols+1]));
		json_object_object_add(object, "Heading",
			json_object_new_string(qResults[(i+1)*nCols+2]));
		json_object_object_add(object, "PlotWith",
			json_object_new_string(qResults[(i+1)*nCols+3]));
		json_object_object_add(object, "GraphType",
			json_object_new_int(atoi(qResults[(i+1)*nCols+4])));
		json_object_object_add(object, "PerSTA",
			json_object_new_int(atoi(qResults[(i+1)*nCols+5])));

		/* Add the bar heading */
		array = json_object_new_array();
		for (j = i; j < nRows; j++) {
			if (strcmp(qResults[(i+1)*nCols+3], qResults[(j+1)*nCols+3]) == 0) {
				json_object_array_add(array,
					json_object_new_string((qResults[(j+1)*nCols+6])));
				selrows[selrowscount++] = j;
				(*mainrows)[nmainrows++] = j;
			}
		}
		json_object_object_add(object, "BarHeading", array);
		json_object_object_add(object, "XAxis",
			json_object_new_string(qResults[(i+1)*nCols+7]));
		json_object_object_add(object, "YAxis",
			json_object_new_string(qResults[(i+1)*nCols+8]));
		json_object_object_add(object, "Total",
			json_object_new_int(selrowscount));
		placeholder = atoi(qResults[(i+1)*nCols]);
		json_object_object_add(object, "Placeholderid",
			json_object_new_int(placeholder));

		/* Print for debug purpose only */
		VIS_DB("rowid : %d\n", atoi(qResults[(i+1)*nCols]));
		VIS_DB("Name : %s\n", qResults[(i+1)*nCols+1]);
		VIS_DB("Heading : %s\n", (qResults[(i+1)*nCols+2]));
		VIS_DB("Plotwith : %s\n", (qResults[(i+1)*nCols+3]));
		VIS_DB("Graph Type : %d\n", atoi(qResults[(i+1)*nCols+4]));
		VIS_DB("Per STA : %d\n", atoi(qResults[(i+1)*nCols+5]));
		VIS_DB("Bar Heading : %s\n", (qResults[(i+1)*nCols+6]));
		VIS_DB("X Axis : %s\n", (qResults[(i+1)*nCols+7]));
		VIS_DB("Y Axis : %s\n", qResults[(i+1)*nCols+8]);
		VIS_DB("Table : %s\n", qResults[(i+1)*nCols+9]);

		array_x = json_object_new_array();

		array_y_main = json_object_new_array();

		/* For every graph add the xaxis and yaxis values */
		for (j = 0; j < selrowscount; j++) {
			int persta = 0, graphtype = 0;
			json_object *array_y;

			k = selrows[j];

			graphtype = atoi(qResults[(k+1)*nCols+4]);
			persta = atoi(qResults[(k+1)*nCols+5]);

			array_y = json_object_new_array();

			get_graph_details_from_db(qResults[(k+1)*nCols+9],
				atoi(qResults[(k+1)*nCols]), rowid, graphtype,
				persta, &array_x, &array_y, j, tabname, mac);

			json_object_array_add(array_y_main, array_y);
		}
		json_object_object_add(object, "XValue", array_x);
		json_object_object_add(object, "YValue", array_y_main);

		json_object_array_add(array_graph, object);
	}
	return array_graph;
}

/* Creates and adds the json data for the STA's Airtime opportunity */
static void
add_sta_adv_airtimeoppoutunity(vis_rrm_statslist_t *rrmstatslist, json_object **sub_data_obj1)
{
	json_object *bar_heading, *array_x_val, *array_y_val, *array_y_main_val;
	json_object *sub_data_obj1_1, *sub_data_array1, *sta_data_array_1;
	int index;

	json_object_object_add(*sub_data_obj1, "Heading",
		json_object_new_string("Air Time Opportunity"));

	sub_data_obj1_1 = json_object_new_object();
	json_object_object_add(sub_data_obj1_1, "GraphType", json_object_new_int(1));
	json_object_object_add(sub_data_obj1_1, "Heading",
		json_object_new_string("Air Time Opportunity"));
	json_object_object_add(sub_data_obj1_1, "PlotWith",
		json_object_new_string("Air Time Opportunity"));
	json_object_object_add(sub_data_obj1_1, "PerSta", json_object_new_int(1));
	bar_heading = json_object_new_array();
	json_object_array_add(bar_heading, json_object_new_string("Air Time Opportunity"));
	json_object_object_add(sub_data_obj1_1, "BarHeading", bar_heading);
	json_object_object_add(sub_data_obj1_1, "XAxis", json_object_new_string("Time"));
	json_object_object_add(sub_data_obj1_1, "YAxis", json_object_new_string("Percentage"));

	/* Fill x-axis and y-axis values */
	array_x_val = json_object_new_array();
	array_y_val = json_object_new_array();
	array_y_main_val = json_object_new_array();
	for (index = 0; index < rrmstatslist->length; index++) {
		if (rrmstatslist->rrmstats[index].txop != -1) {
			json_object_array_add(array_x_val,
				json_object_new_int(rrmstatslist->rrmstats[index].timestamp));
			json_object_array_add(array_y_val,
				json_object_new_int(rrmstatslist->rrmstats[index].txop));
		}
	}
	json_object_array_add(array_y_main_val, array_y_val);
	json_object_object_add(sub_data_obj1_1, "XValue", array_x_val);
	json_object_object_add(sub_data_obj1_1, "YValue", array_y_main_val);
	json_object_object_add(sub_data_obj1_1, "Placeholderid",
		json_object_new_string("AIRTIMEOPPORTUNITY"));
	json_object_object_add(sub_data_obj1_1, "Total", json_object_new_int(1));
	json_object_object_add(sub_data_obj1_1, "IsEnabled", json_object_new_int(1));

	sub_data_array1 = json_object_new_array();
	json_object_array_add(sub_data_array1, sub_data_obj1_1);
	json_object_object_add(*sub_data_obj1, "Data", sub_data_array1);

	sta_data_array_1 = json_object_new_array();
	json_object_object_add(*sub_data_obj1, "STAData", sta_data_array_1);
}

/* Creates and adds the json data for the STA's RRM stats */
static json_object*
add_sta_adv_rrm_stats(vis_rrm_statslist_t *rrmstatslist)
{
	json_object *sta_data_obj1;
	json_object *array_heading, *x_val, *y_val1, *y_val2, *y_val3, *y_val4, *y_val_main;
	int index, total = 0;

	if (rrmstatslist->length <= 0)
		return NULL;

	sta_data_obj1 = json_object_new_object();
	json_object_object_add(sta_data_obj1, "GraphType", json_object_new_int(1));
	json_object_object_add(sta_data_obj1, "Heading",
		json_object_new_string("STAs Packet Queue Statistics"));
	json_object_object_add(sta_data_obj1, "PlotWith",
		json_object_new_string("STAs Packet Queue Statistics"));
	json_object_object_add(sta_data_obj1, "PerSta", json_object_new_int(1));

	array_heading = json_object_new_array();
	/* fill x-axis and y-axis */
	x_val = json_object_new_array();
	y_val1 = json_object_new_array();
	y_val2 = json_object_new_array();
	y_val3 = json_object_new_array();
	y_val4 = json_object_new_array();
	y_val_main = json_object_new_array();
	for (index = 0; index < rrmstatslist->length; index++) {
		json_object_array_add(x_val,
			json_object_new_int(rrmstatslist->rrmstats[index].timestamp));
		json_object_array_add(y_val1,
			json_object_new_int(rrmstatslist->rrmstats[index].pktrequested));
		json_object_array_add(y_val2,
			json_object_new_int(rrmstatslist->rrmstats[index].pktstored));
		json_object_array_add(y_val3,
			json_object_new_int(rrmstatslist->rrmstats[index].pktdropped));
		json_object_array_add(y_val4,
			json_object_new_int(rrmstatslist->rrmstats[index].pktretried));
	}
	json_object_object_add(sta_data_obj1, "XValue", x_val);
	if (Is_rrm_stats_graph_enabled(PKT_REQUESTED)) {
		json_object_array_add(y_val_main, y_val1);
		json_object_array_add(array_heading,
			json_object_new_string("Packet Requested"));
		total++;
	}

	if (Is_rrm_stats_graph_enabled(PKT_STORED)) {
		json_object_array_add(y_val_main, y_val2);
		json_object_array_add(array_heading,
			json_object_new_string("Packet Stored"));
		total++;
	}

	if (Is_rrm_stats_graph_enabled(PKT_DROPPED)) {
		json_object_array_add(y_val_main, y_val3);
		json_object_array_add(array_heading,
			json_object_new_string("Packet Dropped"));
		total++;
	}

	if (Is_rrm_stats_graph_enabled(PKT_RETRIED)) {
		json_object_array_add(y_val_main, y_val4);
		json_object_array_add(array_heading,
			json_object_new_string("Packet Retried"));
		total++;
	}
	json_object_object_add(sta_data_obj1, "BarHeading", array_heading);
	json_object_object_add(sta_data_obj1, "XAxis", json_object_new_string("Time"));
	json_object_object_add(sta_data_obj1, "YAxis", json_object_new_string("Count"));
	json_object_object_add(sta_data_obj1, "YValue", y_val_main);
	json_object_object_add(sta_data_obj1, "Placeholderid",
		json_object_new_string("STAPACKETQUEUEPLACEHOLDER0"));
	json_object_object_add(sta_data_obj1, "Total", json_object_new_int(total));
	json_object_object_add(sta_data_obj1, "Name", json_object_new_string("Sta Stats"));
	json_object_object_add(sta_data_obj1, "IsEnabled", json_object_new_int(1));
	return total ? sta_data_obj1 : NULL;
}

/* Creates and adds the json data for the STA's RRM stats Acked values */
static json_object*
add_sta_adv_rrm_stats_acked(vis_rrm_statslist_t *rrmstatslist)
{
	json_object *sta_data_obj1;
	json_object *array_heading, *x_val, *y_val1, *y_val_main;
	int index, total = 0;
	if (!Is_rrm_stats_graph_enabled(PKT_ACKED))
		return NULL;
	if (rrmstatslist->length <= 0)
		return NULL;

	sta_data_obj1 = json_object_new_object();
	json_object_object_add(sta_data_obj1, "GraphType", json_object_new_int(1));
	json_object_object_add(sta_data_obj1, "Heading",
		json_object_new_string("STAs Packet Queue Statistics"));
	json_object_object_add(sta_data_obj1, "PlotWith",
		json_object_new_string("STAs Packet Queue Statistics"));
	json_object_object_add(sta_data_obj1, "PerSta", json_object_new_int(1));

	array_heading = json_object_new_array();
	/* fill x-axis and y-axis */
	x_val = json_object_new_array();
	y_val1 = json_object_new_array();
	y_val_main = json_object_new_array();
	for (index = 0; index < rrmstatslist->length; index++) {
		json_object_array_add(x_val,
			json_object_new_int(rrmstatslist->rrmstats[index].timestamp));
		json_object_array_add(y_val1,
			json_object_new_int(rrmstatslist->rrmstats[index].pktacked));
	}
	json_object_array_add(y_val_main, y_val1);
	json_object_array_add(array_heading,
		json_object_new_string("Acked"));
	total++;
	json_object_object_add(sta_data_obj1, "BarHeading", array_heading);
	json_object_object_add(sta_data_obj1, "XAxis", json_object_new_string("Time"));
	json_object_object_add(sta_data_obj1, "YAxis", json_object_new_string("Count"));
	json_object_object_add(sta_data_obj1, "XValue", x_val);
	json_object_object_add(sta_data_obj1, "YValue", y_val_main);
	json_object_object_add(sta_data_obj1, "Placeholderid",
		json_object_new_string("STAPACKETQUEUEPLACEHOLDER1"));
	json_object_object_add(sta_data_obj1, "Total", json_object_new_int(total));
	json_object_object_add(sta_data_obj1, "Name", json_object_new_string("Sta Stats"));
	json_object_object_add(sta_data_obj1, "IsEnabled", json_object_new_int(1));
	return sta_data_obj1;
}

/* Delete all items from table */
int
delete_all_from_table(char *table)
{
	char	querystr[256];
	int	idx = -1, rval;

	lock_db_mutex(table, &idx);

	snprintf(querystr, sizeof(querystr), "DELETE FROM %s;", table);
	rval = execute_sql_statement(g_db, querystr);

	unlock_db_mutex(table, idx);

	return rval;
}
