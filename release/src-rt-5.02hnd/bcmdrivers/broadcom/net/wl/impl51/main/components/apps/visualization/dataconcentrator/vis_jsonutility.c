/*
 * Linux Visualization Data Concentrator json utility implementation
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
 * $Id: vis_jsonutility.c 672659 2016-11-29 10:24:01Z $
 */

#include "vis_jsonutility.h"
#include <string.h>
#include "vis_sock_util.h"
#include "vis_utility.h"

#define AMPDU_NAME_XAXIS		"MCS Index"
#define AMPDU_NAME_YAXIS		"Packets"
#define AMPDU_NAME_DENSITY_XAXIS	"MPDU's in AMPDU"
#define AMPDU_NAME_DENSITY_YAXIS	"No. of AMPDU's"

extern int is_valid_5g_40m_channel(int channel, int *upperch);
extern int is_valid_5g_80m_channel(int channel, int *upperch);

/* AMPDU stats iovar's
 */
iovar_handler_t json_iovar_handler[] = {
	{ IOVAR_NAME_AMPDUTXWITHOUTSGI, "AMPDU Statistics", IOVAR_NAME_AMPDUTXWITHOUTSGI,
	GRAPH_TYPE_BAR, 0,
	"AMPDU Tx MCS Without SGI", AMPDU_NAME_XAXIS, AMPDU_NAME_YAXIS},

	{ IOVAR_NAME_AMPDUTXWITHSGI, "AMPDU Statistics", IOVAR_NAME_AMPDUTXWITHOUTSGI,
	GRAPH_TYPE_BAR, 0,
	"AMPDU Tx MCS With SGI", AMPDU_NAME_XAXIS, AMPDU_NAME_YAXIS},

	{ IOVAR_NAME_AMPDURXWITHOUTSGI, "AMPDU Statistics", IOVAR_NAME_AMPDUTXWITHOUTSGI,
	GRAPH_TYPE_BAR, 0,
	"AMPDU Rx MCS Without SGI", AMPDU_NAME_XAXIS, AMPDU_NAME_YAXIS},

	{ IOVAR_NAME_AMPDURXWITHSGI, "AMPDU Statistics", IOVAR_NAME_AMPDUTXWITHOUTSGI,
	GRAPH_TYPE_BAR, 0,
	"AMPDU Rx MCS With SGI", AMPDU_NAME_XAXIS, AMPDU_NAME_YAXIS},

	{ IOVAR_NAME_AMPDUTXVHTWITHOUTSGI, "AMPDU Statistics", IOVAR_NAME_AMPDUTXWITHOUTSGI,
	GRAPH_TYPE_BAR, 0,
	"AMPDU Tx VHT Without SGI", AMPDU_NAME_XAXIS, AMPDU_NAME_YAXIS},

	{ IOVAR_NAME_AMPDUTXVHTWITHSGI, "AMPDU Statistics", IOVAR_NAME_AMPDUTXWITHOUTSGI,
	GRAPH_TYPE_BAR, 0,
	"AMPDU Tx VHT With SGI", AMPDU_NAME_XAXIS, AMPDU_NAME_YAXIS},

	{ IOVAR_NAME_AMPDURXVHTWITHOUTSGI, "AMPDU Statistics", IOVAR_NAME_AMPDUTXWITHOUTSGI,
	GRAPH_TYPE_BAR, 0,
	"AMPDU Rx VHT Without SGI", AMPDU_NAME_XAXIS, AMPDU_NAME_YAXIS},

	{ IOVAR_NAME_AMPDURXVHTWITHSGI, "AMPDU Statistics", IOVAR_NAME_AMPDUTXWITHOUTSGI,
	GRAPH_TYPE_BAR, 0,
	"AMPDU Rx VHT With SGI", AMPDU_NAME_XAXIS, AMPDU_NAME_YAXIS},

	{ IOVAR_NAME_AMPDUMPDUDENSITY, "AMPDU Statistics", IOVAR_NAME_AMPDUTXWITHOUTSGI,
	GRAPH_TYPE_BAR, 0,
	"MPDU Density", AMPDU_NAME_DENSITY_XAXIS, AMPDU_NAME_DENSITY_YAXIS},
};

/* Gets the iovar structure handle from the iovar name */
iovar_handler_t*
json_find_iovar(const char *name)
{
	iovar_handler_t *tmpiovar;

	for (tmpiovar = json_iovar_handler;
		tmpiovar->name && strcmp(tmpiovar->name, name); tmpiovar++);
	if (tmpiovar->name == NULL) {
		tmpiovar = NULL;
	}

	return tmpiovar;
}

/* Adds dut info struct to json object */
static void
add_dut_to_json_object(dut_info_t *dut_info, json_object **object)
{
	char	security[250];

	json_object_object_add(*object, "SSID",
		json_object_new_string(dut_info->ssid));
	json_object_object_add(*object, "Signal",
		json_object_new_int(dut_info->rssi));
	json_object_object_add(*object, "Noise",
		json_object_new_int(dut_info->noise));
	json_object_object_add(*object, "SNR",
		json_object_new_int((dut_info->rssi - dut_info->noise)));
	json_object_object_add(*object, "Channel",
		json_object_new_int(dut_info->channel));
	json_object_object_add(*object, "ControlChannel",
		json_object_new_int(dut_info->ctrlch));

	if (strlen(dut_info->ucastrsntype) > 0) {
		snprintf(security, sizeof(security), "%s, %s",
			dut_info->ucastrsntype, dut_info->akmrsntype);
		remove_trailing_character(security, ' ');
		remove_trailing_character(security, ',');
	} else {
		snprintf(security, sizeof(security), " ");
	}
	json_object_object_add(*object, "Security",
		json_object_new_string(security));
	json_object_object_add(*object, "Frequencyband",
		json_object_new_int(dut_info->band));
	json_object_object_add(*object, "Type",
		json_object_new_string(dut_info->networktype));
	json_object_object_add(*object, "Bandwidth",
		json_object_new_int(dut_info->bandwidth));
	json_object_object_add(*object, "Speed",
		json_object_new_int(dut_info->maxrate));
	json_object_object_add(*object, "Networkaddress",
		json_object_new_string(dut_info->bssid));
	json_object_object_add(*object, "IsCurrent",
		json_object_new_int(1));
	json_object_object_add(*object, "ErrorInfo",
		json_object_new_int(dut_info->errinfo));
}

/* creates json for site survey tab and sends to server */
int
send_networks(int sockfd, dut_info_t *dut_info, networks_list_t *networks_list)
{
	json_object	*array_main, *array_next;
	json_object	*object;
	int		i = 0, ret = 0;
	const char	*data = NULL;
	char		security[250];

	/* Add Networks Details */
	if (networks_list != NULL) {
		array_main = json_object_new_array();
		object = json_object_new_object();
		json_object_object_add(object, "Total",
			json_object_new_int(networks_list->length+1));
		json_object_object_add(object, "Timestamp",
			json_object_new_int(networks_list->timestamp));
		json_object_array_add(array_main, object);

		array_next = json_object_new_array();

		for (i = 0; i < networks_list->length; i++) {
			object = json_object_new_object();
			json_object_object_add(object, "SSID",
				json_object_new_string(networks_list->aps[i].ssid));
			json_object_object_add(object, "Signal",
				json_object_new_int(networks_list->aps[i].rssi));
			json_object_object_add(object, "Noise",
				json_object_new_int(networks_list->aps[i].noise));
			json_object_object_add(object, "SNR",
				json_object_new_int(
				(networks_list->aps[i].rssi - networks_list->aps[i].noise)));
			json_object_object_add(object, "Channel",
				json_object_new_int(networks_list->aps[i].channel));
			json_object_object_add(object, "ControlChannel",
				json_object_new_int(networks_list->aps[i].ctrlch));

			if (strlen(networks_list->aps[i].ucastrsntype) > 0) {
				snprintf(security, sizeof(security), "%s, %s",
					networks_list->aps[i].ucastrsntype,
					networks_list->aps[i].akmrsntype);
				remove_trailing_character(security, ' ');
				remove_trailing_character(security, ',');
			} else {
				snprintf(security, sizeof(security), " ");
			}
			json_object_object_add(object, "Security",
				json_object_new_string(security));
			json_object_object_add(object, "Frequencyband",
				json_object_new_int(networks_list->aps[i].band));
			json_object_object_add(object, "Type",
				json_object_new_string(networks_list->aps[i].networktype));
			json_object_object_add(object, "Bandwidth",
				json_object_new_int(networks_list->aps[i].bandwidth));
			json_object_object_add(object, "Speed",
				json_object_new_int(networks_list->aps[i].maxrate));
			json_object_object_add(object, "Networkaddress",
				json_object_new_string(networks_list->aps[i].bssid));
			json_object_object_add(object, "IsCurrent",
				json_object_new_int(0));

			json_object_array_add(array_next, object);
		}

		object = json_object_new_object();
		add_dut_to_json_object(dut_info, &object);
		json_object_array_add(array_next, object);

		json_object_array_add(array_main, array_next);
	} else { /* add length as 0 */
		array_main = json_object_new_array();
		object = json_object_new_object();
		json_object_object_add(object, "Total",
			json_object_new_int(0));
		json_object_object_add(object, "Timestamp",
			json_object_new_int(0));
		json_object_array_add(array_main, object);
	}

	data = json_object_to_json_string(array_main);

	if (data) {
		VIS_JSON("%s\n", data);

		ret = add_length_and_send_data(sockfd, data, strlen(data));
	}
	json_object_put(array_main);

	return ret;
}

/* creates json for config settings and sends it to server */
int
send_config_to_web(int sockfd, configs_t *configs, graphnamelist_t *graphnames)
{
	json_object	*array_main, *array_next;
	json_object	*object;
	int		i = 0, ret = 0;
	const char	*data = NULL;

	array_main = json_object_new_array();

	array_next = json_object_new_array();

	object = json_object_new_object();

	json_object_object_add(object, "Interval",
		json_object_new_int(configs->interval));
	json_object_object_add(object, "DBSize",
		json_object_new_int(configs->dbsize));
	json_object_object_add(object, "StartStop",
		json_object_new_int(configs->isstart));
	json_object_object_add(object, "GatewayIP",
		json_object_new_string(configs->gatewayip));
	json_object_object_add(object, "isoverwritedb",
		json_object_new_int(configs->isoverwrtdb));
	json_object_object_add(object, "AutoStart",
		json_object_new_int(configs->isautostart));
	json_object_object_add(object, "WeekDays",
		json_object_new_int(configs->weekdays));
	json_object_object_add(object, "FromTm",
		json_object_new_int(configs->fromtm));
	json_object_object_add(object, "ToTm",
		json_object_new_int(configs->totm));
	json_object_object_add(object, "IsInternal",
		json_object_new_int(0));

	json_object_array_add(array_next, object);

	json_object_array_add(array_main, array_next);

	/* Add graphnames Details */
	if (graphnames != NULL) {
		array_next = json_object_new_array();

		for (i = 0; i < graphnames->length; i++) {
			object = json_object_new_object();
			json_object_object_add(object, "Tab",
				json_object_new_string(graphnames->graphname[i].tab));
			json_object_object_add(object, "Name",
				json_object_new_string(graphnames->graphname[i].name));
			json_object_object_add(object, "Heading",
				json_object_new_string(graphnames->graphname[i].heading));
			json_object_object_add(object, "PlotWith",
				json_object_new_string(graphnames->graphname[i].plotwith));
			json_object_object_add(object, "Type",
				json_object_new_int(graphnames->graphname[i].type));
			json_object_object_add(object, "PerSTA",
				json_object_new_int(graphnames->graphname[i].persta));
			json_object_object_add(object, "BarHeading",
				json_object_new_string(graphnames->graphname[i].barheading));
			json_object_object_add(object, "XAxisName",
				json_object_new_string(graphnames->graphname[i].xaxisname));
			json_object_object_add(object, "YAxisName",
				json_object_new_string(graphnames->graphname[i].yaxisname));
			json_object_object_add(object, "Enable",
				json_object_new_int(graphnames->graphname[i].enable));

			json_object_array_add(array_next, object);
		}

		json_object_array_add(array_main, array_next);
	}

	data = json_object_to_json_string(array_main);

	if (data) {
		VIS_JSON("%s\n", data);

		ret = add_length_and_send_data(sockfd, data, strlen(data));
	}
	json_object_put(array_main);

	return ret;
}

/* creates json for associated STA's and sends it to server */
int
send_associated_sta(int sockfd, dut_info_t *dut_info, assoc_sta_list_t *stas_list)
{
	json_object	*array_main, *array;
	json_object	*object, *object1;
	int		i = 0, ret = 0;
	const char	*data = NULL;
	char		tmpchannel[10] = {0};

	array_main = json_object_new_array();

	/* get channel string */
	get_channel_str(dut_info->channel, dut_info->bandwidth, dut_info->ctrlch,
		tmpchannel, sizeof(tmpchannel));
	/* Add DUT Details */
	object = json_object_new_object();
	json_object_object_add(object, "SSID",
		json_object_new_string(dut_info->ssid));
	json_object_object_add(object, "BSSID",
		json_object_new_string(dut_info->bssid));
	json_object_object_add(object, "Channel",
		json_object_new_string(tmpchannel));
	json_object_array_add(array_main, object);

	array = json_object_new_array();

	/* Add Networks Details */
	if (stas_list != NULL) {
		for (i = 0; i < stas_list->length; i++) {
			object1 = json_object_new_object();
			json_object_object_add(object1, "MAC",
				json_object_new_string(stas_list->sta[i].mac));
			json_object_object_add(object1, "RSSI",
				json_object_new_int(stas_list->sta[i].rssi));
			json_object_object_add(object1, "PhyRate",
				json_object_new_int(stas_list->sta[i].phyrate));

			json_object_array_add(array, object1);
		}
	}

	json_object_array_add(array_main, array);

	data = json_object_to_json_string(array_main);

	if (data) {
		VIS_JSON("%s\n", data);

		ret = add_length_and_send_data(sockfd, data, strlen(data));
	}
	json_object_put(array_main);

	return ret;
}

/* sends the response in json format */
int
send_json_response(int sockfd, char *response)
{
	json_object	*array_main;
	json_object	*object;
	int		ret = 0;
	const char	*data = NULL;

	array_main = json_object_new_array();

	object = json_object_new_object();

	json_object_object_add(object, "Response",
		json_object_new_string(response));

	json_object_array_add(array_main, object);

	data = json_object_to_json_string(array_main);

	if (data) {
		VIS_JSON("%s\n", data);

		ret = add_length_and_send_data(sockfd, data, strlen(data));
	}
	json_object_put(array_main);

	return ret;
}

/* adds the json header for graphs to json object
 * nwhichtype is 1 for channel capacity, 2 for interference
 * 3 for adjacent channels
 */
static void
add_json_header(json_object **object, char *heading, int nwhichtype)
{
	json_object	*array;

	json_object_object_add(*object, "GraphType",
		json_object_new_int(GRAPH_TYPE_BAR));
	json_object_object_add(*object, "Heading",
		json_object_new_string(heading));
	json_object_object_add(*object, "PlotWith",
		json_object_new_string("None"));
	json_object_object_add(*object, "PerSTA",
		json_object_new_int(0));

	array = json_object_new_array();
	if (nwhichtype == 1) {
		json_object_array_add(array, json_object_new_string("Available Capacity"));
	} else if (nwhichtype == 2) {
		json_object_array_add(array, json_object_new_string("Non-WiFi Traffic"));
	} else {
		json_object_array_add(array, json_object_new_string("Adjacent AP"));
		json_object_array_add(array, json_object_new_string("Associated STA"));
	}

	json_object_object_add(*object, "BarHeading",
		array);
	json_object_object_add(*object, "XAxis",
		json_object_new_string("Channels"));
	json_object_object_add(*object, "YAxis",
		json_object_new_string("Percentage(%)"));
}


/* gets the maximum availcapacity from list */
static int
get_maximum_availcap(int upperch, int i, congestion_list_t *congestion)
{
	int k = 0, wifi;

	wifi = congestion->congest[i].availcap;
	for (k = i; k < congestion->length; k++) {
		if (congestion->congest[k].channel <= upperch) {
			wifi = (wifi <
				congestion->congest[k].availcap)?
				congestion->congest[k].availcap:
				wifi;
		} else
			break;
		}

	return wifi;
}


/* gets the maximum non-wifi intereference from list */
static int
get_maximum_nonwifi_interference(int upperch, int i, congestion_list_t *congestion)
{
	int k = 0, wifi;

	wifi = congestion->congest[i].nonwifi;
	for (k = i; k < congestion->length; k++) {
		if (congestion->congest[k].channel <= upperch) {
			wifi = (wifi <
				congestion->congest[k].nonwifi)?
				congestion->congest[k].nonwifi:
				wifi;
		} else
			break;
		}

	return wifi;
}

/* TO get all the extension channels by control channel, center channel and bandwidth */
static int
get_extension_channels(int ctrlch, int centerch, int bandwidth, int *extch)
{
	int totch = 0;

	if (bandwidth == 40) {
		totch = 1;
		if (ctrlch < centerch)
			extch[0] = centerch + 2;
		else
			extch[0] = centerch - 2;
	} else if (bandwidth == 80) {
		if (ctrlch == (centerch - 2)) {
			extch[0] = centerch - 6;
			extch[1] = centerch + 2;
			extch[2] = centerch + 6;
			totch = 3;
		} else if (ctrlch == (centerch - 6)) {
			extch[0] = centerch - 2;
			extch[1] = centerch + 2;
			extch[2] = centerch + 6;
			totch = 3;
		} else if (ctrlch == (centerch + 2)) {
			extch[0] = centerch - 6;
			extch[1] = centerch - 2;
			extch[2] = centerch + 6;
			totch = 3;
		} else if (ctrlch == (centerch + 6)) {
			extch[0] = centerch - 6;
			extch[1] = centerch - 2;
			extch[2] = centerch + 2;
			totch = 3;
		}
	}

	return totch;
}

/* Add DUT info to chanmap json object */
static void
add_dut_info_to_chanmap(dut_info_t *dut_info, json_object **object)
{
	json_object *array_ext;
	int extch[10];
	int totchs = 0, j;

	json_object_object_add(*object, "ssid",
		json_object_new_string(dut_info->ssid));
	json_object_object_add(*object, "ctrlch",
		json_object_new_int(dut_info->ctrlch));
	json_object_object_add(*object, "IsCurrent",
		json_object_new_int(1));
	totchs = get_extension_channels(dut_info->ctrlch, dut_info->channel,
		dut_info->bandwidth, extch);
	array_ext = json_object_new_array();
	for (j = 0; j < totchs; j++) {
		json_object_array_add(array_ext, json_object_new_int(extch[j]));
	}
	json_object_object_add(*object, "extch", array_ext);
}

static void
add_channel_capacity_for_twenty(congestion_list_t *congestion, json_object **x_20,
	json_object **y_20_1, json_object **y_20_2, json_object **y_20_3)
{
	int i;

	if (congestion != NULL) {
		for (i = 0; i < congestion->length; i++) {
			json_object_array_add(*x_20,
				json_object_new_int(congestion->congest[i].channel));
			json_object_array_add(*y_20_3,
				json_object_new_int(congestion->congest[i].availcap));
		}
	}
}

static void
add_channel_capacity_for_fourty(congestion_list_t *congestion, json_object **x_40,
	json_object **y_40_1, json_object **y_40_2, json_object **y_40_3)
{
	int i;

	if (congestion != NULL) {
		for (i = 0; i < congestion->length; i++) {
			if (congestion->congest[i].channel <= 14) { /* for 2g channels */
				if (congestion->congest[i].channel <= 7) {

					int availcap = (congestion->congest[i].availcap <
							congestion->congest[i+4].availcap)?
							congestion->congest[i+4].availcap:
							congestion->congest[i].availcap;

					json_object_array_add(*x_40,
						json_object_new_int(
							congestion->congest[i].channel+2));
					json_object_array_add(*y_40_3,
						json_object_new_int(availcap));
				}
			} else {
				int upperch = 0;

				if (is_valid_5g_40m_channel(congestion->congest[i].channel,
					&upperch) == TRUE) {
					if (congestion->congest[i+1].channel >
						(congestion->congest[i].channel + 9)) {
						continue;
					}

					int availcap = get_maximum_availcap(upperch, i,
						congestion);

					json_object_array_add(*x_40,
						json_object_new_int(
							congestion->congest[i].channel));
					json_object_array_add(*y_40_3,
						json_object_new_int(availcap));
				}
			}
		}
	}
}

static void
add_channel_capacity_for_eighty(congestion_list_t *congestion, json_object **x_80,
	json_object **y_80_1, json_object **y_80_2, json_object **y_80_3)
{
	int i;

	if (congestion != NULL) {
		for (i = 0; i < congestion->length; i++) {
			if (congestion->congest[i].channel > 14) {
				int upperch = 0;

				if (is_valid_5g_80m_channel(congestion->congest[i].channel,
					&upperch) == TRUE) {
					if (congestion->congest[i+1].channel >
						(congestion->congest[i].channel + 9))
						continue;


					int availcap = get_maximum_availcap(upperch, i,
						congestion);

					json_object_array_add(*x_80,
						json_object_new_int(
							congestion->congest[i].channel));
					json_object_array_add(*y_80_3,
						json_object_new_int(availcap));
				}
			}
		}
	}
}

static void
add_channel_capacity_to_json(json_object **object_20, congestion_list_t *congestion,
	int bandwidth, int freqband, int pholderid)
{
	char heading[25];
	json_object	*x_20, *y_20, *y_20_1, *y_20_2, *y_20_3;

	snprintf(heading, sizeof(heading), "Channel Capacity %d MHz", bandwidth);
	add_json_header(object_20, heading, 1);

	/* array for 20 MHz Xvalues and Yvalues */
	x_20 = json_object_new_array();
	y_20 = json_object_new_array();
	y_20_1 = json_object_new_array();
	y_20_2 = json_object_new_array();
	y_20_3 = json_object_new_array();

	if (bandwidth == 20)
		add_channel_capacity_for_twenty(congestion, &x_20, &y_20_1, &y_20_2, &y_20_3);
	if (bandwidth == 40)
		add_channel_capacity_for_fourty(congestion, &x_20, &y_20_1, &y_20_2, &y_20_3);
	if (bandwidth == 80)
		add_channel_capacity_for_eighty(congestion, &x_20, &y_20_1, &y_20_2, &y_20_3);
	json_object_put(y_20_1);
	json_object_put(y_20_2);
	json_object_array_add(y_20, y_20_3);
	json_object_object_add(*object_20, "XValue", x_20);
	json_object_object_add(*object_20, "YValue", y_20);
	json_object_object_add(*object_20, "Placeholderid", json_object_new_int(pholderid));
	json_object_object_add(*object_20, "Total", json_object_new_int(1));
	json_object_object_add(*object_20, "Bandwidth", json_object_new_int(bandwidth));
	json_object_object_add(*object_20, "CHStatType",
		json_object_new_string("ChannelCapacity"));
}

static void
add_interference_for_twenty(congestion_list_t *congestion, json_object **x_20,
	json_object **y_20_1, json_object **y_20_2)
{
	int i;

	if (congestion != NULL) {
		for (i = 0; i < congestion->length; i++) {
			json_object_array_add(*x_20,
				json_object_new_int(congestion->congest[i].channel));
			json_object_array_add(*y_20_2,
				json_object_new_int(congestion->congest[i].nonwifi));
		}
	}
}

static void
add_interference_for_fourty(congestion_list_t *congestion, json_object **x_40,
	json_object **y_40_1, json_object **y_40_2)
{
	int i;

	if (congestion != NULL) {
		for (i = 0; i < congestion->length; i++) {
			if (congestion->congest[i].channel <= 14) { /* for 2g channels */
				if (congestion->congest[i].channel <= 7) {
					int nonwifi = (congestion->congest[i].nonwifi <
							congestion->congest[i+4].nonwifi)?
							congestion->congest[i+4].nonwifi:
							congestion->congest[i].nonwifi;
					json_object_array_add(*x_40,
						json_object_new_int(
							congestion->congest[i].channel+2));
					json_object_array_add(*y_40_2,
						json_object_new_int(nonwifi));
				}
			} else { /* for 5g channels */
				int upperch = 0;

				if (is_valid_5g_40m_channel(congestion->congest[i].channel,
					&upperch) == TRUE) {
					if (congestion->congest[i+1].channel >
						(congestion->congest[i].channel + 9)) {
						continue;
					}


					int nonwifi = get_maximum_nonwifi_interference(upperch, i,
						congestion);

					json_object_array_add(*x_40,
						json_object_new_int(
							congestion->congest[i].channel));
					json_object_array_add(*y_40_2,
						json_object_new_int(nonwifi));
				}
			}
		}
	}
}

static void
add_interference_for_eighty(congestion_list_t *congestion, json_object **x_80,
	json_object **y_80_1, json_object **y_80_2)
{
	int i;

	if (congestion != NULL) {
		for (i = 0; i < congestion->length; i++) {
			if (congestion->congest[i].channel > 14) {
				int upperch = 0;

				if (is_valid_5g_80m_channel(congestion->congest[i].channel,
					&upperch) == TRUE) {
					if (congestion->congest[i+1].channel >
						(congestion->congest[i].channel + 9))
						continue;


					int nonwifi = get_maximum_nonwifi_interference(upperch, i,
						congestion);

					json_object_array_add(*x_80,
						json_object_new_int(
							congestion->congest[i].channel));
					json_object_array_add(*y_80_2,
						json_object_new_int(nonwifi));
				}
			}
		}
	}
}

static void
add_channel_interference_to_json(json_object **object_20, congestion_list_t *congestion,
	int bandwidth, int freqband, int pholderid)
{
	char heading[25];
	json_object	*x_20, *y_20, *y_20_1, *y_20_2;

	snprintf(heading, sizeof(heading), "Interference %d MHz", bandwidth);
	add_json_header(object_20, heading, 2);

	/* array for 20 MHz Xvalues and Yvalues */
	x_20 = json_object_new_array();
	y_20 = json_object_new_array();
	y_20_1 = json_object_new_array();
	y_20_2 = json_object_new_array();

	if (bandwidth == 20)
		add_interference_for_twenty(congestion, &x_20, &y_20_1, &y_20_2);
	if (bandwidth == 40)
		add_interference_for_fourty(congestion, &x_20, &y_20_1, &y_20_2);
	if (bandwidth == 80)
		add_interference_for_eighty(congestion, &x_20, &y_20_1, &y_20_2);
	json_object_put(y_20_1);
	json_object_array_add(y_20, y_20_2);
	json_object_object_add(*object_20, "XValue", x_20);
	json_object_object_add(*object_20, "YValue", y_20);
	json_object_object_add(*object_20, "Placeholderid", json_object_new_int(pholderid));
	json_object_object_add(*object_20, "Total", json_object_new_int(1));
	json_object_object_add(*object_20, "Bandwidth", json_object_new_int(bandwidth));
	json_object_object_add(*object_20, "CHStatType",
		json_object_new_string("Interference"));
}

/* Creates All channels json array for 5G */
static void
create_all_channels_json_data(congestion_list_t *congestion, json_object **x)
{
	int i;

	if (congestion == NULL)
		return;
	for (i = 0; i < congestion->length; i++) {
		if ((i >= 1) && (congestion->congest[i-1].channel <
			congestion->congest[i].channel-4)) {
			json_object_array_add(*x,
				json_object_new_string(""));
			json_object_array_add(*x,
				json_object_new_string(""));
		}
		json_object_array_add(*x,
			json_object_new_int(congestion->congest[i].channel));
		if ((i != (congestion->length - 1)) &&
			(congestion->congest[i].channel+4 == congestion->congest[i+1].channel)) {
			json_object_array_add(*x,
				json_object_new_string(""));
		}
	}
}
inline int
is_value_found_in_array(int *intarr, int size, int value)
{
	int j;

	for (j = 0; j < size; j++) {
		if (intarr[j] == value) {
			return j;
		}
	}

	return -1;
}

/* populates the X and Y value json objects for adjacent channel graph */
static void
create_adjacent_x_and_y_values(channel_maplist_t *chanmap, json_object **x_20,
	json_object **y_20_1, json_object **y_20_2, int bandwidth, int freqband,
	int *chnls_to_discard, int nchnls_to_discard)
{
	int i, j;
	int chnlscompleted[165];
	int nchnlscompleted = 0;

	if (chanmap != NULL) {
		for (i = 0; i < chanmap->length; i++) {
			if (bandwidth != chanmap->channelmap[i].bandwidth)
				continue;
			int maxrssi = chanmap->channelmap[i].rssi;
			int chnl = chanmap->channelmap[i].ctrlch;
			int cntrch = chanmap->channelmap[i].channel; /* Center channel */
			int tmpbandwidth = chanmap->channelmap[i].bandwidth;
			int no_ofaps = 1, lowerchnl = 0;

			/*
			 * Other AP with max RSSI might have been added for that channel.
			 * So dont include other AP for that channel
			 */
			if (is_value_found_in_array(chnlscompleted, nchnlscompleted, cntrch) != -1)
				continue;

			for (j = i+1; j < chanmap->length; j++) {
				if (cntrch == chanmap->channelmap[j].channel &&
					bandwidth == chanmap->channelmap[j].bandwidth) {
					if (maxrssi < chanmap->channelmap[j].rssi)
						maxrssi = chanmap->channelmap[j].rssi;
					no_ofaps++;
				}
			}
			get_lower_sideband_channel(chnl, &lowerchnl, tmpbandwidth);
			if (lowerchnl == 0)
				lowerchnl = chnl;
			/* Dont include the channels on which visualization is running */
			if (is_value_found_in_array(chnls_to_discard, nchnls_to_discard,
				lowerchnl) != -1)
				continue;
			if (freqband != 5 && bandwidth == 40) {
				json_object_array_add(*x_20,
					json_object_new_int(chanmap->channelmap[i].channel));
			} else {
				json_object_array_add(*x_20,
					json_object_new_int(lowerchnl));
			}
			json_object_array_add(*y_20_1,
				json_object_new_int(maxrssi));
			json_object_array_add(*y_20_2,
				json_object_new_int(no_ofaps));
			chnlscompleted[nchnlscompleted++] = cntrch;
		}
	}
}

/* Add the adjacent channel details to json */
static void
add_adjacent_channels_to_json(json_object **object_20, channel_maplist_t *chanmap,
	int bandwidth, int freqband, int pholderid, int curchnl, int curbandwidth,
	assoc_sta_list_t *stas_list)
{
	int lowerchnl = 0, i;
	int chnls_to_discard[10], nchnls_to_discard = 0, centerch;
	char heading[30];
	json_object	*x_20, *y_20, *y_20_1, *y_20_2, *Ay, *Ay_1, *ssids, *Ax;

	snprintf(heading, sizeof(heading), "Adjacent Channels %d MHz", bandwidth);
	add_json_header(object_20, heading, 3);

	/* array for 20 MHz Xvalues and Yvalues */
	x_20 = json_object_new_array();
	y_20 = json_object_new_array();
	y_20_1 = json_object_new_array();
	y_20_2 = json_object_new_array();
	Ax = json_object_new_array();
	Ay = json_object_new_array();
	Ay_1 = json_object_new_array();
	ssids = json_object_new_array();

	/*
	 * As per the feedback we should not show the adjacent AP on the channel
	 * on which visualization is running. So we need to get the all the channels
	 * to be discarded
	 */
	/* Get the center channel for the current channel. This is to get the ext channels */
	get_center_channel(curchnl, &centerch, curbandwidth);
	/* get all the extension channels for the given channel and bandwidth */
	nchnls_to_discard = get_extension_channels(curchnl, centerch,
		curbandwidth, chnls_to_discard);
	/* Now add the current channel to the list of channels to discard */
	chnls_to_discard[nchnls_to_discard++] = curchnl;
	create_adjacent_x_and_y_values(chanmap, &x_20, &y_20_1, &y_20_2, bandwidth,
		freqband, chnls_to_discard, nchnls_to_discard);

	/* add the current channels x value and stas_list */
	get_lower_sideband_channel(curchnl, &lowerchnl, bandwidth);
	if (lowerchnl == 0)
		lowerchnl = curchnl;
	for (i = 0; i < stas_list->length; i++) {
		json_object_array_add(Ay_1,
			json_object_new_int(stas_list->sta[i].rssi));
		if (curbandwidth == bandwidth) {
			json_object_array_add(ssids,
				json_object_new_string(stas_list->sta[i].mac));
		}
	}
	json_object_array_add(Ay, Ay_1);
	json_object_array_add(Ax, json_object_new_int(lowerchnl));

	json_object_array_add(y_20, y_20_1);
	json_object_array_add(y_20, y_20_2);
	json_object_object_add(*object_20, "XValue", x_20);
	json_object_object_add(*object_20, "YValue", y_20);
	json_object_object_add(*object_20, "AXValue", Ax);
	json_object_object_add(*object_20, "AYValue", Ay);
	json_object_object_add(*object_20, "SSIDs", ssids);
	json_object_object_add(*object_20, "Placeholderid", json_object_new_int(pholderid));
	json_object_object_add(*object_20, "Total", json_object_new_int(2));
	json_object_object_add(*object_20, "Bandwidth", json_object_new_int(bandwidth));
	json_object_object_add(*object_20, "CHStatType",
		json_object_new_string("AdjacentChannels"));
}

/* Creates json for channel stats and sends to server */
int
send_channel_capacity(int sockfd, dut_info_t *dut_info, congestion_list_t *congestion,
	int capacity, channel_maplist_t *chanmap, int freqband, assoc_sta_list_t *stas_list)
{
	json_object	*array_main, *array_values;
	json_object	*object_dut, *object_20, *object_40, *object_80;
	json_object *object_chmap, *x_chmap, *x_chmap_1;
	json_object *allchnls;
	int		i = 0, ret = 0;
	const char	*data = NULL;
	char		tmpchannel[10] = {0};

	array_main = json_object_new_array();

	object_dut = json_object_new_object();

	get_channel_str(dut_info->channel, dut_info->bandwidth, dut_info->ctrlch,
		tmpchannel, sizeof(tmpchannel));
	json_object_object_add(object_dut, "Channel",
		json_object_new_string(tmpchannel));
	json_object_object_add(object_dut, "Bandwidth",
		json_object_new_int(dut_info->bandwidth));
	json_object_object_add(object_dut, "Capacity",
		json_object_new_int(capacity));
	if (freqband == 5) {
		allchnls = json_object_new_array();
		create_all_channels_json_data(congestion, &allchnls);
		json_object_object_add(object_dut, "AllChnls",
			allchnls);
	}

	json_object_array_add(array_main, object_dut);

	/* Create the array to hold all 20 MHz 40 MHz and 80 MHz values */
	array_values = json_object_new_array();

	/* *********** CHANNEL CAPACITY STARTS *********** */
	/* *************************************************** */
	/* Object to hold 20 MHz details */
	object_20 = json_object_new_object();

	add_channel_capacity_to_json(&object_20, congestion, 20, freqband, 1);

	json_object_array_add(array_values, object_20);
	/* Done with the 20 MHz */
	/* *************************************************** */

	/* *************************************************** */
	/* Object to hold 40 MHz details */
	object_40 = json_object_new_object();

	add_channel_capacity_to_json(&object_40, congestion, 40, freqband, 2);

	json_object_array_add(array_values, object_40);
	/* Done with the 40 MHz */
	/* *************************************************** */

	/* *************************************************** */
	/* Object to hold 80 MHz details */
	object_80 = json_object_new_object();

	add_channel_capacity_to_json(&object_80, congestion, 80, freqband, 3);

	json_object_array_add(array_values, object_80);
	/* Done with the 80 MHz */
	/* *************************************************** */
	/* *********** CHANNEL CAPACITY ENDS *********** */

	/* *****************INTERFERENCE STARTS************* */
	/* Object to hold 20 MHz details */
	object_20 = json_object_new_object();

	add_channel_interference_to_json(&object_20, congestion, 20, freqband, 4);

	json_object_array_add(array_values, object_20);
	/* Done with the 20 MHz */
	/* ********************intereference**************** */

	/* *****************interference************* */
	/* Object to hold 40 MHz details */
	object_40 = json_object_new_object();

	add_channel_interference_to_json(&object_40, congestion, 40, freqband, 5);

	json_object_array_add(array_values, object_40);
	/* Done with the 40 MHz */
	/* ********************intereference**************** */

	/* *****************interference************* */
	/* Object to hold 80 MHz details */
	object_80 = json_object_new_object();

	add_channel_interference_to_json(&object_80, congestion, 80, freqband, 6);

	json_object_array_add(array_values, object_80);
	/* Done with the 80 MHz */
	/* *****************INTERFERENCE ENDS************* */


	/* *****************Adjacent Channels************* */
	/* Object to hold 20 MHz details */
	object_20 = json_object_new_object();

	add_adjacent_channels_to_json(&object_20, chanmap, 20, freqband, 7,
		dut_info->ctrlch, dut_info->bandwidth, stas_list);

	json_object_array_add(array_values, object_20);
	/* Done with the 20 MHz */

	/* Object to hold 40 MHz details */
	object_20 = json_object_new_object();

	add_adjacent_channels_to_json(&object_20, chanmap, 40, freqband, 8,
		dut_info->ctrlch, dut_info->bandwidth, stas_list);

	json_object_array_add(array_values, object_20);
	/* Done with the 40 MHz */

	/* Object to hold 80 MHz details */
	object_20 = json_object_new_object();

	add_adjacent_channels_to_json(&object_20, chanmap, 80, freqband, 9,
		dut_info->ctrlch, dut_info->bandwidth, stas_list);

	json_object_array_add(array_values, object_20);
	/* Done with the 80 MHz */
	/* ********************Adjacent Channels**************** */


	/* *************************************************** */
	/* Object to hold Channel Map details */
	object_chmap = json_object_new_object();

	json_object_object_add(object_chmap, "Heading",
		json_object_new_string("ChannelMap"));

	/* array for 80 MHz Xvalues */
	x_chmap = json_object_new_array();
	if (chanmap != NULL && freqband == 5) {
		/* first add current DUT info */
		json_object *objecttmp;

		x_chmap_1 = json_object_new_array();
		objecttmp = json_object_new_object();
		add_dut_info_to_chanmap(dut_info, &objecttmp);
		json_object_array_add(x_chmap_1, objecttmp);
		json_object_array_add(x_chmap, x_chmap_1);
		for (i = 0; i < chanmap->length; i++) {
			json_object *object, *array_ext;
			int extch[10];
			int totchs = 0, j;

			x_chmap_1 = json_object_new_array();
			object = json_object_new_object();
			json_object_object_add(object, "ssid",
				json_object_new_string(chanmap->channelmap[i].ssid));
			json_object_object_add(object, "ctrlch",
				json_object_new_int(chanmap->channelmap[i].ctrlch));
			json_object_object_add(object, "IsCurrent",
				json_object_new_int(0));
			totchs = get_extension_channels(chanmap->channelmap[i].ctrlch,
				chanmap->channelmap[i].channel,
				chanmap->channelmap[i].bandwidth, extch);
			array_ext = json_object_new_array();
			for (j = 0; j < totchs; j++) {
				json_object_array_add(array_ext, json_object_new_int(extch[j]));
			}
			json_object_object_add(object, "extch", array_ext);
			json_object_array_add(x_chmap_1, object);
			json_object_array_add(x_chmap, x_chmap_1);
		}
	}
	json_object_object_add(object_chmap, "XValue", x_chmap);

	json_object_object_add(object_chmap, "Placeholderid", json_object_new_int(10));
	json_object_object_add(object_chmap, "Total", json_object_new_int(3));

	json_object_array_add(array_values, object_chmap);
	/* Done with the ChannelMap */
	/* *************************************************** */

	json_object_array_add(array_main, array_values);

	data = json_object_to_json_string(array_main);

	if (data) {
		VIS_JSON("%s\n", data);

		ret = add_length_and_send_data(sockfd, data, strlen(data));
	}
	json_object_put(array_main);

	return ret;
}

/* adds the json header for graphs to json object */
static void
add_json_graph_header(json_object **object, iovar_handler_t *handler)
{
	json_object	*array;

	json_object_object_add(*object, "GraphType",
		json_object_new_int(handler->graphtype));
	json_object_object_add(*object, "Heading",
		json_object_new_string(handler->heading));
	json_object_object_add(*object, "PlotWith",
		json_object_new_string("None"));
	json_object_object_add(*object, "PerSTA",
		json_object_new_int(0));

	array = json_object_new_array();
	json_object_array_add(array, json_object_new_string(handler->barheading));

	json_object_object_add(*object, "BarHeading",
		array);
	json_object_object_add(*object, "XAxis",
		json_object_new_string(handler->xaxisname));
	json_object_object_add(*object, "YAxis",
		json_object_new_string(handler->yaxisname));
}

/* store AMPDU without SGI in json YVALUES */
static void store_ampdu_without_SGI(json_object **x, json_object **y1, json_object **y2,
	tx_ampdu_list_t *ampdulist)
{
	int i;

	for (i = 0; i < ampdulist->length; i++)
	{
		char xval[6];

		snprintf(xval, sizeof(xval), "%dx%d", ampdulist->ampdus[i].mcs/10,
			ampdulist->ampdus[i].mcs%10);
		json_object_array_add(*x, json_object_new_string(xval));
		json_object_array_add(*y1, json_object_new_int(ampdulist->ampdus[i].txmcs));
		json_object_array_add(*y2, json_object_new_int(ampdulist->ampdus[i].txmcspercent));
	}
}

/* store AMPDU with SGI in json */
static void store_ampdu_with_SGI(json_object **x, json_object **y1, json_object **y2,
	tx_ampdu_list_t *ampdulist)
{
	int i;

	for (i = 0; i < ampdulist->length; i++)
	{
		char xval[6];

		snprintf(xval, sizeof(xval), "%dx%d", ampdulist->ampdus[i].mcs/10,
			ampdulist->ampdus[i].mcs%10);
		json_object_array_add(*x, json_object_new_string(xval));
		json_object_array_add(*y1, json_object_new_int(ampdulist->ampdus[i].txmcssgi));
		json_object_array_add(*y2,
			json_object_new_int(ampdulist->ampdus[i].txmcssgipercent));
	}
}

/* store AMPDU rx without SGI in json */
static void store_ampdu_rx_without_SGI(json_object **x, json_object **y1, json_object **y2,
	tx_ampdu_list_t *ampdulist)
{
	int i;

	for (i = 0; i < ampdulist->length; i++)
	{
		char xval[6];

		snprintf(xval, sizeof(xval), "%dx%d", ampdulist->ampdus[i].mcs/10,
			ampdulist->ampdus[i].mcs%10);
		json_object_array_add(*x, json_object_new_string(xval));
		json_object_array_add(*y1, json_object_new_int(ampdulist->ampdus[i].rxmcs));
		json_object_array_add(*y2,
			json_object_new_int(ampdulist->ampdus[i].rxmcspercent));
	}
}

/* store AMPDU rx with SGI in json */
static void store_ampdu_rx_with_SGI(json_object **x, json_object **y1, json_object **y2,
	tx_ampdu_list_t *ampdulist)
{
	int i;

	for (i = 0; i < ampdulist->length; i++)
	{
		char xval[6];

		snprintf(xval, sizeof(xval), "%dx%d", ampdulist->ampdus[i].mcs/10,
			ampdulist->ampdus[i].mcs%10);
		json_object_array_add(*x, json_object_new_string(xval));
		json_object_array_add(*y1, json_object_new_int(ampdulist->ampdus[i].rxmcssgi));
		json_object_array_add(*y2,
			json_object_new_int(ampdulist->ampdus[i].rxmcssgipercent));
	}
}

/* store AMPDU TX VHT without SGI in json */
static void store_ampdu_tx_vht_without_sgi(json_object **x, json_object **y1, json_object **y2,
	tx_ampdu_list_t *ampdulist)
{
	int i;

	for (i = 0; i < ampdulist->length; i++)
	{
		char xval[6];

		snprintf(xval, sizeof(xval), "%dx%d", ampdulist->ampdus[i].mcs/10,
			ampdulist->ampdus[i].mcs%10);
		json_object_array_add(*x, json_object_new_string(xval));
		json_object_array_add(*y1, json_object_new_int(ampdulist->ampdus[i].txvht));
		json_object_array_add(*y2,
			json_object_new_int(ampdulist->ampdus[i].txvhtpercent));
	}
}

/* store AMPDU TX VHT with SGI in json */
static void store_ampdu_tx_vht_with_sgi(json_object **x, json_object **y1, json_object **y2,
	tx_ampdu_list_t *ampdulist)
{
	int i;

	for (i = 0; i < ampdulist->length; i++)
	{
		char xval[6];

		snprintf(xval, sizeof(xval), "%dx%d", ampdulist->ampdus[i].mcs/10,
			ampdulist->ampdus[i].mcs%10);
		json_object_array_add(*x, json_object_new_string(xval));
		json_object_array_add(*y1, json_object_new_int(ampdulist->ampdus[i].txvhtsgi));
		json_object_array_add(*y2,
			json_object_new_int(ampdulist->ampdus[i].txvhtsgipercent));
	}
}

/* store AMPDU RX VHT without SGI in json */
static void store_ampdu_rx_vht_without_sgi(json_object **x, json_object **y1, json_object **y2,
	tx_ampdu_list_t *ampdulist)
{
	int i;

	for (i = 0; i < ampdulist->length; i++)
	{
		char xval[6];

		snprintf(xval, sizeof(xval), "%dx%d", ampdulist->ampdus[i].mcs/10,
			ampdulist->ampdus[i].mcs%10);
		json_object_array_add(*x, json_object_new_string(xval));
		json_object_array_add(*y1, json_object_new_int(ampdulist->ampdus[i].rxvht));
		json_object_array_add(*y2,
			json_object_new_int(ampdulist->ampdus[i].rxvhtpercent));
	}
}

/* store AMPDU RX VHT with SGI in json */
static void store_ampdu_rx_vht_with_sgi(json_object **x, json_object **y1, json_object **y2,
	tx_ampdu_list_t *ampdulist)
{
	int i;

	for (i = 0; i < ampdulist->length; i++)
	{
		char xval[6];

		snprintf(xval, sizeof(xval), "%dx%d", ampdulist->ampdus[i].mcs/10,
			ampdulist->ampdus[i].mcs%10);
		json_object_array_add(*x, json_object_new_string(xval));
		json_object_array_add(*y1, json_object_new_int(ampdulist->ampdus[i].rxvhtsgi));
		json_object_array_add(*y2,
			json_object_new_int(ampdulist->ampdus[i].rxvhtsgipercent));
	}
}

/* store AMPDU mpdu density in json */
static void store_ampdu_mpdu_density(json_object **x, json_object **y1, json_object **y2,
	tx_ampdu_list_t *ampdulist)
{
	int i;

	for (i = 0; i < ampdulist->length; i++)
	{
		int ampdus = 0;

		ampdus = ((ampdulist->ampdus[i].mcs/10) * 8) - 7 + (ampdulist->ampdus[i].mcs%10);
		json_object_array_add(*x, json_object_new_int(ampdus));
		json_object_array_add(*y1, json_object_new_int(ampdulist->ampdus[i].mpdudens));
		json_object_array_add(*y2,
			json_object_new_int(ampdulist->ampdus[i].mpdudenspercent));
	}
}

/* Add AMPDU stats to JSON */
static void
add_ampdu_stats_to_json(json_object **object_20, tx_ampdu_list_t *ampdulist,
	int pholderid, char *iovarname, void (*store_ampdu)(json_object **, json_object **,
	json_object **, tx_ampdu_list_t*))
{
	json_object	*x_20, *y_20, *y_20_1, *y_20_2;
	iovar_handler_t *handler;

	handler = json_find_iovar(iovarname);

	add_json_graph_header(object_20, handler);

	/* array for Xvalues and Yvalues */
	x_20 = json_object_new_array();
	y_20 = json_object_new_array();
	y_20_1 = json_object_new_array();
	y_20_2 = json_object_new_array();

	(*store_ampdu)(&x_20, &y_20_1, &y_20_2, ampdulist);

	json_object_array_add(y_20, y_20_1);
	json_object_array_add(y_20, y_20_2);
	json_object_object_add(*object_20, "XValue", x_20);
	json_object_object_add(*object_20, "YValue", y_20);
	json_object_object_add(*object_20, "Placeholderid", json_object_new_int(pholderid));
	json_object_object_add(*object_20, "Total", json_object_new_int(2));
	json_object_object_add(*object_20, "IsEnabled", json_object_new_int(0));
}

/* creates json for AMPDU Statistics and sends it to server */
int
send_ampdu_stats(int sockfd, tx_ampdu_list_t *ampdulist)
{
	json_object	*array_main, *array_values;
	json_object	*object;
	int	ret = 0;
	const char	*data = NULL;

	array_main = json_object_new_array();

	/* Create the array to hold all AMpdu stats values */
	array_values = json_object_new_array();

	/* *************************************************** */
	/* Object to hold Tx ampdu without SGI details */
	object = json_object_new_object();

	add_ampdu_stats_to_json(&object, ampdulist, 1, IOVAR_NAME_AMPDUTXWITHOUTSGI,
		store_ampdu_without_SGI);

	json_object_array_add(array_values, object);
	/* *************************************************** */

	/* *************************************************** */
	/* Object to hold Tx ampdu with SGI details */
	object = json_object_new_object();

	add_ampdu_stats_to_json(&object, ampdulist, 1, IOVAR_NAME_AMPDUTXWITHSGI,
		store_ampdu_with_SGI);

	json_object_array_add(array_values, object);
	/* *************************************************** */

	/* *************************************************** */
	/* Object to hold Rx ampdu without SGI details */
	object = json_object_new_object();

	add_ampdu_stats_to_json(&object, ampdulist, 1, IOVAR_NAME_AMPDURXWITHOUTSGI,
		store_ampdu_rx_without_SGI);

	json_object_array_add(array_values, object);
	/* *************************************************** */

	/* *************************************************** */
	/* Object to hold Rx ampdu with SGI details */
	object = json_object_new_object();

	add_ampdu_stats_to_json(&object, ampdulist, 1, IOVAR_NAME_AMPDURXWITHSGI,
		store_ampdu_rx_with_SGI);

	json_object_array_add(array_values, object);
	/* *************************************************** */

	/* *************************************************** */
	/* Object to hold Tx ampdu VHT without SGI details */
	object = json_object_new_object();

	add_ampdu_stats_to_json(&object, ampdulist, 1, IOVAR_NAME_AMPDUTXVHTWITHOUTSGI,
		store_ampdu_tx_vht_without_sgi);

	json_object_array_add(array_values, object);
	/* *************************************************** */

	/* *************************************************** */
	/* Object to hold Tx ampdu VHT with SGI details */
	object = json_object_new_object();

	add_ampdu_stats_to_json(&object, ampdulist, 1, IOVAR_NAME_AMPDUTXVHTWITHSGI,
		store_ampdu_tx_vht_with_sgi);

	json_object_array_add(array_values, object);
	/* *************************************************** */

	/* *************************************************** */
	/* Object to hold Rx ampdu VHT without SGI details */
	object = json_object_new_object();

	add_ampdu_stats_to_json(&object, ampdulist, 1, IOVAR_NAME_AMPDURXVHTWITHOUTSGI,
		store_ampdu_rx_vht_without_sgi);

	json_object_array_add(array_values, object);
	/* *************************************************** */

	/* *************************************************** */
	/* Object to hold Rx ampdu VHT with SGI details */
	object = json_object_new_object();

	add_ampdu_stats_to_json(&object, ampdulist, 1, IOVAR_NAME_AMPDURXVHTWITHSGI,
		store_ampdu_rx_vht_with_sgi);

	json_object_array_add(array_values, object);
	/* *************************************************** */

	/* *************************************************** */
	/* Object to hold AMPDU MPDU density details */
	object = json_object_new_object();

	add_ampdu_stats_to_json(&object, ampdulist, 1, IOVAR_NAME_AMPDUMPDUDENSITY,
		store_ampdu_mpdu_density);

	json_object_array_add(array_values, object);
	/* *************************************************** */

	json_object_array_add(array_main, array_values);

	data = json_object_to_json_string(array_main);

	if (data) {
		VIS_JSON("%s\n", data);

		ret = add_length_and_send_data(sockfd, data, strlen(data));
	}
	json_object_put(array_main);

	return ret;
}

/* Creates json for chanim stats for metrics page and sends to webserver */
int
send_chanim_metrics_stats(int sockfd, dut_info_t *dut_info, congestion_list_t *congestion,
	int freqband)
{
	int index = 0, ret = 0;
	json_object	*object;
	json_object	*array;
	json_object	*array_x, *array_y_main, *array_graph, *array_main;

	/* All the chanim stats required to be send */
	char *stats[] = {"tx", "inbss", "obss", "nocat", "nopkt", "doze", "txop",
		"goodtx", "badtx", "glitch", "badplcp", "knoise", "idle"};
	int totalstats = (sizeof(stats)/sizeof(char*));

	array_main = json_object_new_array();
	array_graph = json_object_new_array();
	object = json_object_new_object();

	/* Create graph headers */
	json_object_object_add(object, "Name",
		json_object_new_string("Chanim Statistics"));
	json_object_object_add(object, "Heading",
		json_object_new_string("Chanim Statistics"));
	json_object_object_add(object, "PlotWith",
		json_object_new_string("Chanim Statistics"));
	json_object_object_add(object, "GraphType",
		json_object_new_int(4));
	json_object_object_add(object, "PerSTA",
		json_object_new_int(0));

	/* Add the bar heading. It includes all the chanim stats names */
	array = json_object_new_array();
	for (index = 0; index < totalstats; index++) {
		json_object_array_add(array,
			json_object_new_string(stats[index]));
	}
	json_object_object_add(object, "BarHeading", array);
	json_object_object_add(object, "XAxis",
		json_object_new_string("Time"));
	json_object_object_add(object, "YAxis",
		json_object_new_string("Count"));
	json_object_object_add(object, "Total",
		json_object_new_int(totalstats));
	json_object_object_add(object, "Placeholderid",
		json_object_new_string("CHANIMSTATSFORMETRICS"));

	array_x = json_object_new_array();

	array_y_main = json_object_new_array();

	/* For every stats add y axis values */
	json_object *array_tx, *array_inbss, *array_obss, *array_nocat, *array_nopkt;
	json_object *array_doze, *array_txop, *array_goodtx, *array_badtx;
	json_object *array_glitch, *array_badplcp, *array_knoise, *array_idle;

	/* Create array to add all chanim stats. Individual array for each stat */
	array_tx = json_object_new_array();
	array_inbss = json_object_new_array();
	array_obss = json_object_new_array();
	array_nocat = json_object_new_array();
	array_nopkt = json_object_new_array();
	array_doze = json_object_new_array();
	array_txop = json_object_new_array();
	array_goodtx = json_object_new_array();
	array_badtx = json_object_new_array();
	array_glitch = json_object_new_array();
	array_badplcp = json_object_new_array();
	array_knoise = json_object_new_array();
	array_idle = json_object_new_array();
	for (index = 0; index < congestion->length; index++) {
		json_object_array_add(array_x,
			json_object_new_int(congestion->congest[index].timestamp));
		json_object_array_add(array_tx,
			json_object_new_int(congestion->congest[index].tx));
		json_object_array_add(array_inbss,
			json_object_new_int(congestion->congest[index].inbss));
		json_object_array_add(array_obss,
			json_object_new_int(congestion->congest[index].obss));
		json_object_array_add(array_nocat,
			json_object_new_int(congestion->congest[index].nocat));
		json_object_array_add(array_nopkt,
			json_object_new_int(congestion->congest[index].nopkt));
		json_object_array_add(array_doze,
			json_object_new_int(congestion->congest[index].doze));
		json_object_array_add(array_txop,
			json_object_new_int(congestion->congest[index].txop));
		json_object_array_add(array_goodtx,
			json_object_new_int(congestion->congest[index].goodtx));
		json_object_array_add(array_badtx,
			json_object_new_int(congestion->congest[index].badtx));
		json_object_array_add(array_glitch,
			json_object_new_int(congestion->congest[index].glitchcnt));
		json_object_array_add(array_badplcp,
			json_object_new_int(congestion->congest[index].badplcp));
		json_object_array_add(array_knoise,
			json_object_new_int(congestion->congest[index].knoise));
		json_object_array_add(array_idle,
			json_object_new_int(congestion->congest[index].chan_idle));
	}

	/* Add all individual array to main array */
	json_object_array_add(array_y_main, array_tx);
	json_object_array_add(array_y_main, array_inbss);
	json_object_array_add(array_y_main, array_obss);
	json_object_array_add(array_y_main, array_nocat);
	json_object_array_add(array_y_main, array_nopkt);
	json_object_array_add(array_y_main, array_doze);
	json_object_array_add(array_y_main, array_txop);
	json_object_array_add(array_y_main, array_goodtx);
	json_object_array_add(array_y_main, array_badtx);
	json_object_array_add(array_y_main, array_glitch);
	json_object_array_add(array_y_main, array_badplcp);
	json_object_array_add(array_y_main, array_knoise);
	json_object_array_add(array_y_main, array_idle);

	/* Now add x axis value array and y axis value array to main object */
	json_object_object_add(object, "XValue", array_x);
	json_object_object_add(object, "YValue", array_y_main);
	json_object_array_add(array_graph, object);

	json_object_array_add(array_main, array_graph);
	const char *data = json_object_to_json_string(array_main);
	if (data) {
		VIS_JSON("%s\n", data);

		ret = add_length_and_send_data(sockfd, data, strlen(data));
	}
	json_object_put(array_main);

	return ret;
}

/* Send All duts (All Bands) to webserver */
int
send_all_duts(int sockfd, dut_list_t *dutlist)
{
	json_object	*array_main;
	int i = 0, ret = 0;

	array_main = json_object_new_array();
	if (dutlist) {
		for (i = 0; i < dutlist->length; i++) {
			json_object *object;

			object = json_object_new_object();
			json_object_object_add(object, "band",
				json_object_new_int(dutlist->duts[i].band));
			json_object_object_add(object, "ssid",
				json_object_new_string(dutlist->duts[i].ssid));
			json_object_object_add(object, "bssid",
				json_object_new_string(dutlist->duts[i].bssid));
			json_object_object_add(object, "mac",
				json_object_new_string(dutlist->duts[i].mac));
			json_object_array_add(array_main, object);
		}
	}

	const char *data = json_object_to_json_string(array_main);
	if (data) {
		VIS_JSON("%s\n", data);

		ret = add_length_and_send_data(sockfd, data, strlen(data));
	}
	json_object_put(array_main);

	return ret;
}

static char*
get_generation_str_from_flag(int genflag, char *genstr)
{
	if (genflag & NETWORK_TYPE_A)
		strcat(genstr, "a");
	if (genflag & NETWORK_TYPE_B)
		strcat(genstr, "b");
	if (genflag & NETWORK_TYPE_G)
		strcat(genstr, "g");
	if (genflag & NETWORK_TYPE_N)
		strcat(genstr, "n");
	if (genflag & NETWORK_TYPE_AC)
		strcat(genstr, "ac");

	return genstr;
}

/* Send STA Stats to webserver */
int
send_rrm_stastats(int sockfd, vis_sta_stats_t *stastats)
{
	int ret = 0;
	char tmpoui[10] = {0}, genstr[10] = {0};
	json_object *object_main, *alldataobject;

	object_main = json_object_new_object();
	json_object_object_add(object_main, "Req",
		json_object_new_string(PACKET_REQRRMSTASTATS));
	json_object_object_add(object_main, "Status",
		json_object_new_string("Success"));

	alldataobject = json_object_new_object();
	json_object_object_add(alldataobject, "MAC",
		json_object_new_string(stastats->mac));
	json_object_object_add(alldataobject, "OUI",
		json_object_new_string(strncpy(tmpoui, stastats->mac, 8)));
	json_object_object_add(alldataobject, "Generation",
		json_object_new_string(get_generation_str_from_flag(stastats->generation_flag,
		genstr)));
	json_object_object_add(alldataobject, "TXStream",
		json_object_new_int(stastats->txstream));
	json_object_object_add(alldataobject, "RXStream",
		json_object_new_int(stastats->rxstream));
	json_object_object_add(alldataobject, "Rate",
		json_object_new_int(stastats->rate));
	json_object_object_add(alldataobject, "Active",
		json_object_new_int(stastats->idle));
	json_object_object_add(object_main, "AllData", alldataobject);

	const char *data = json_object_to_json_string(object_main);
	if (data) {
		VIS_JSON("%s\n", data);

		ret = add_length_and_send_data(sockfd, data, strlen(data));
	}
	json_object_put(object_main);

	return ret;
}
