/*
 * Linux Visualization Data Concentrator XML utility implementation
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
 * $Id: vis_xmlutility.c 672659 2016-11-29 10:24:01Z $
 */

#include <string.h>
#include "vis_xmlutility.h"
#include "vis_xmlshared.h"
#include "database.h"
#include "vis_synchdb.h"
#include "vis_jsonutility.h"
#include "vis_utility.h"
#include "vis_sock_util.h"

extern configs_t g_configs;

static int parse_counters_request(int sockfd, xmlDocPtr doc, xmlNodePtr node, dut_info_t *dutinfo);

static networks_list_t *parse_networks_node(xmlDocPtr doc, xmlNodePtr node);
static int parse_ap_node(xmlDocPtr doc, xmlNodePtr node, int i, networks_list_t *networks_list);

static assoc_sta_list_t *parse_associated_sta_node(xmlDocPtr doc, xmlNodePtr node);
static int parse_sta_node(xmlDocPtr doc, xmlNodePtr node, int i, assoc_sta_list_t *stas_list);

static congestion_list_t *parse_channel_stats_node(xmlDocPtr doc, xmlNodePtr node);
static int parse_stats_node(xmlDocPtr doc, xmlNodePtr node, int i, congestion_list_t *congestion);

static graph_list_t *parse_graphlist_node(xmlDocPtr doc, xmlNodePtr node, char *graphname);

static tx_ampdu_list_t *parse_ampdu_stats_node(xmlDocPtr doc, xmlNodePtr node);

static vis_rrm_statslist_t *parse_rrm_all_sta_stats_node(xmlDocPtr doc, xmlNodePtr node);
static int parse_rrm_sta_node(xmlDocPtr doc, xmlNodePtr node, int i,
	vis_rrm_statslist_t *rrmstatslist);

static vis_sta_statslist_t* parse_all_sta_stats_info_node(xmlDocPtr doc, xmlNodePtr node);
static int parse_sta_info_node(xmlDocPtr doc, xmlNodePtr node, int i,
	vis_sta_statslist_t *stastatslist);

/* To get details of DUT from node */
static int
get_individual_dut(xmlDocPtr doc, xmlNodePtr nodechild, dut_info_t *dutinfo)
{
	int	ret = 0;

	if (nodechild == NULL) /* If there is no node inside */
		return -1;

	while (nodechild != NULL) {
		if (strcmp((const char*)nodechild->name, TAG_ISAP) == 0)
			get_int_element(doc, nodechild, (int*)&dutinfo->isAP);
		else if (strcmp((const char*)nodechild->name, TAG_ISENABLED) == 0)
			get_int_element(doc, nodechild, (int*)&dutinfo->isenabled);
		else if (strcmp((const char*)nodechild->name, TAG_BSSID) == 0)
			get_string_element(doc, nodechild, dutinfo->bssid, (ETHER_ADDR_LEN * 3));
		else if (strcmp((const char*)nodechild->name, TAG_SSID) == 0) {
			get_cdata_element(nodechild, dutinfo->ssid, sizeof(dutinfo->ssid));
		}
		else if (strcmp((const char*)nodechild->name, TAG_MAC) == 0) {
			char tmpmac[30];
			get_string_element(doc, nodechild, tmpmac, 30);
			replace_special_characters(tmpmac, ':');
			snprintf(dutinfo->mac, sizeof(dutinfo->mac), "%s", tmpmac);
		}
		else if (strcmp((const char*)nodechild->name, TAG_CHANNEL) == 0)
			get_int_element(doc, nodechild, (int*)&dutinfo->channel);
		else if (strcmp((const char*)nodechild->name, TAG_STAMAC) == 0) {
			char tmpmac[30];
			get_string_element(doc, nodechild, tmpmac, 30);
			replace_special_characters(tmpmac, ':');
			snprintf(dutinfo->stamac, sizeof(dutinfo->stamac), "%s", tmpmac);
		}
		else if (strcmp((const char*)nodechild->name, TAG_BANDWIDTH) == 0)
			get_int_element(doc, nodechild, (int*)&dutinfo->bandwidth);
		else if (strcmp((const char*)nodechild->name, TAG_RSSI) == 0)
			get_int_element(doc, nodechild, (int*)&dutinfo->rssi);
		else if (strcmp((const char*)nodechild->name, TAG_NOISE) == 0)
			get_int_element(doc, nodechild, (int*)&dutinfo->noise);
		else if (strcmp((const char*)nodechild->name, TAG_CONTROLCH) == 0)
			get_int_element(doc, nodechild, (int*)&dutinfo->ctrlch);
		else if (strcmp((const char*)nodechild->name, TAG_BAND) == 0)
			get_int_element(doc, nodechild, (int*)&dutinfo->band);
		else if (strcmp((const char*)nodechild->name, TAG_RATE) == 0)
			get_int_element(doc, nodechild, &dutinfo->maxrate);
		else if (strcmp((const char*)nodechild->name, TAG_TYPE) == 0)
			get_string_element(doc, nodechild, dutinfo->networktype, MAX_NETWORK_TYPE);
		else if (strcmp((const char*)nodechild->name, TAG_MCAST_RSN) == 0)
			get_string_element(doc, nodechild, dutinfo->mcastrsntype,
				MAX_MULTICAST_RSN);
		else if (strcmp((const char*)nodechild->name, TAG_UCAST_RSN) == 0)
			get_string_element(doc, nodechild, dutinfo->ucastrsntype, MAX_UNICAST_RSN);
		else if (strcmp((const char*)nodechild->name, TAG_AKM) == 0)
			get_string_element(doc, nodechild, dutinfo->akmrsntype, MAX_AKM_TYPE);
		else if (strcmp((const char*)nodechild->name, TAG_ERR_INFO) == 0)
			get_int_element(doc, nodechild, (int*)&dutinfo->errinfo);

		nodechild = nodechild->next;
	}

	return ret;
}

/* To get DUT details from the XML node */
static int
get_dut_details(xmlDocPtr doc, xmlNodePtr node, dut_info_t *dutinfo)
{
	int	ret = -1;

	if (node == NULL) /* If there is no node inside packet version */
		return ret;

	while (node != NULL) {
		if (strcmp((const char*)node->name, TAG_DUT) == 0) {
			get_individual_dut(doc, node->children, dutinfo);
			ret = 0;
			break;
		}
		node = node->next;
	}
	VIS_XML("IS AP : %d\nBSSID : %s\nSSID : %s\n",
		dutinfo->isAP, dutinfo->bssid, dutinfo->ssid);
	VIS_XML("MAC : %s\n STAMAC : %s\nBandwidth : %d\n",
		dutinfo->mac, dutinfo->stamac, dutinfo->bandwidth);

	return ret;
}

/* To get All DUT details from the XML node and store it in DB */
static int
store_all_duts_details(xmlDocPtr doc, xmlNodePtr node)
{
	int		ret = 0;

	if (node == NULL) /* If there is no node inside packet version */
		return -1;

	while (node != NULL) {
		if (strcmp((const char*)node->name, TAG_DUT) == 0) {
			dut_info_t dutinfo = {0};

			if (get_individual_dut(doc, node->children, &dutinfo) == 0)
				store_dut_details(&dutinfo);
		}
		node = node->next;
	}

	return ret;
}

/* Parse the XML response from buffer */
int
parse_response(int sockfd, unsigned char **data, unsigned int size)
{
	xmlDocPtr		doc;
	xmlNodePtr		node;
	int			ret = 0, retdut = 0, version;
	packet_header_t		pktheader;
	dut_info_t		dutinfo;
	dut_settings_t dutset;

	memset(&pktheader, 	0x00, sizeof(packet_header_t));
	memset(&dutinfo, 	0x00, sizeof(dut_info_t));
	memset(&dutset,		0x00, sizeof(dut_settings_t));

	pktheader.recordstofetch.count = MAX_GRAPH_ENTRY; /* Default records to fetch */
	pktheader.recordstofetch.records = 2; /* Deafult is get Next record */
	doc = xmlParseMemory((const char*)*data, size);
	if (doc == NULL) {
		VIS_XML("Data is not in XML format for %d\n", sockfd);
		send_json_response(sockfd, "Data is not in XML format");
		return -1;
	}

	if (*data != NULL) {
		free(*data);
		*data = NULL;
	}

	node = doc->children;
	if (node == NULL) { /* if there is no packet version */
		VIS_XML("No packet Version Tag for %d\n", sockfd);
		send_json_response(sockfd, "No Packet Version Tag");
		xmlFreeDoc(doc);
		return -1;
	}

	if (strcmp((const char*)node->name, TAG_PACKETVERSION) == 0) {
		get_int_attribute(node, ATTRIB_NAME, &version);
	}

	get_packet_header(doc, node->children, &pktheader);

	/* get the DUT tag */
	retdut = get_dut_details(doc, node->children, &dutinfo);

	/* Data From data collector */
	if (pktheader.from == PACKET_FROM_DC) {
		configs_t		configs;

		memset(&configs, 0x00, sizeof(configs_t));
		ret = get_config_from_packet(doc, node->children, &configs);
		if (configs.isremoteenab == 0 && strlen(configs.gatewayip) > 0) {
			update_gatewayip_in_db(configs.gatewayip);
			copy_gateway_ip_to_config(configs.gatewayip);
		}

		/* Now store the DUT details */
		if (retdut != 0) { /* If there are no DUT details node */
			VIS_XML("No DUT Details for %d\n", sockfd);
			send_config(sockfd, &g_configs);
			xmlFreeDoc(doc);
			return -1;
		}

		if (strlen(dutinfo.mac) <= 0 || dutinfo.band == 0) {
			pktheader.result = PACKET_SENT_FAILED;
			send_response(sockfd, &pktheader, NULL);
			xmlFreeDoc(doc);
			return -1;
		}
		store_dut_details(&dutinfo);

		if (pktheader.type == PACKET_TYPE_REQ) {
			if (strcmp(pktheader.reqresptype, PACKET_REQLOGIN) == 0) {
				send_config(sockfd, &g_configs);
			} else if (strcmp(pktheader.reqresptype, PACKET_REQCOUNTERS) == 0) {
				SHOW_TIME_ELAPSE_START();
				dut_settings_t tmpdutset;

				/* Get the counters from DC, parse it and save it in DB */
				parse_counters_request(sockfd, doc, node->children, &dutinfo);
				remove_rows_on_db_size_exceedes();
				pktheader.result = PACKET_SENT_SUCCESS;
				/* we also needs to send the current dut settings */
				memset(&tmpdutset, 0x00, sizeof(dut_settings_t));
				get_current_dut_settings(dutinfo.rowid, &tmpdutset);
				/* copy the band and mac address of the DUT */
				tmpdutset.band = dutinfo.band;
				snprintf(tmpdutset.mac, sizeof(tmpdutset.mac), "%s", dutinfo.mac);
				send_response(sockfd, &pktheader, &tmpdutset);

				SHOW_TIME_ELAPSE_END();
			} else if (strcmp(pktheader.reqresptype, PACKET_REQALLDUT) == 0) {
				store_all_duts_details(doc, node->children);
				send_config(sockfd, &g_configs);
			}
		}
	} else if (pktheader.from == PACKET_FROM_WEB) { /* Form WebUI */
		/* Replace + with space in reqresptype as web adds + in place of space */
		replace_plus_with_space(pktheader.reqresptype);

		/* Get the Dut Settings */
		get_dut_settings_tag(doc, node->children, &dutset);

		if (pktheader.type == PACKET_TYPE_REQ) {
			if ((strcmp(pktheader.reqresptype, PACKET_REQLOGIN) == 0) ||
				(strcmp(pktheader.reqresptype, PACKET_REQCONFIG) == 0)) {
				configs_t	configs;
				graphnamelist_t	*graphnames = NULL;

				memset(&configs, 0x00, sizeof(configs_t));
				get_config_from_db(&configs);
				if (ret != 0) {
					pktheader.result = PACKET_ERR_DB_READ;
					snprintf(pktheader.reason, sizeof(pktheader.reason),
						"DB Read Error");
					send_json_response(sockfd, pktheader.reason);
				} else {
					graphnames = get_json_for_graphs();
					send_config_to_web(sockfd, &configs, graphnames);
					if (graphnames != NULL) {
						free(graphnames);
						graphnames = NULL;
					}
				}
			} else if (strcmp(pktheader.reqresptype, PACKET_REQGETDBFILE) == 0) {
				char *filedata = NULL;
				char tmpdbname[100];
				unsigned long nfilelen = 0;

				snprintf(tmpdbname, sizeof(tmpdbname), "%s/%s",
					DB_FOLDER_NAME, DB_NAME);
				nfilelen = read_file_content(tmpdbname, &filedata);
				if (nfilelen > 0 && filedata != NULL) {
					add_length_and_send_data(sockfd, filedata, nfilelen);
				} else {
					add_length_and_send_data(sockfd, " ", 1); /* Empty file */
				}
				if (filedata != NULL) {
					free(filedata);
					filedata = NULL;
				}
			} else if (strcmp(pktheader.reqresptype, PACKET_REQNETWORKS) == 0) {
				networks_list_t *networks_list = NULL;
				dut_info_t		tmpdutinfo;

				memset(&tmpdutinfo, 0x00, sizeof(dut_info_t));

				ret = get_dut_details_from_db(dutinfo.mac, &tmpdutinfo);
				dutset.rowid = tmpdutinfo.rowid;
				if (ret != 0) {
					snprintf(pktheader.reason, sizeof(pktheader.reason),
						"DB Read Error");
					send_json_response(sockfd, pktheader.reason);
				} else {
					/* If there is an request to do the scan from UI */
					if (dutset.isscan == 1) {
						/* Update the DB with the value */
						update_dut_settings_in_db(&dutset);
					}
					networks_list = get_networks_from_db(tmpdutinfo.rowid,
						pktheader.band);
					if (networks_list == NULL) {
						snprintf(pktheader.reason, sizeof(pktheader.reason),
							"DB Read Error");
						send_json_response(sockfd, pktheader.reason);
					} else {
						send_networks(sockfd, &tmpdutinfo, networks_list);
						if (networks_list != NULL) {
							free(networks_list);
							networks_list = NULL;
						}
					}
				}
			} else if (strcmp(pktheader.reqresptype, PACKET_ASSOC_STA) == 0) {
				assoc_sta_list_t	*stas_list = NULL;
				dut_info_t		tmpdutinfo;

				memset(&tmpdutinfo, 0x00, sizeof(dut_info_t));

				ret = get_dut_details_from_db(dutinfo.mac, &tmpdutinfo);
				if (ret != 0) {
					snprintf(pktheader.reason, sizeof(pktheader.reason),
						"DUT Not Present");
					send_json_response(sockfd, pktheader.reason);
				} else {
					stas_list = get_associated_sta_from_db(tmpdutinfo.rowid);
					if (stas_list == NULL) {
						snprintf(pktheader.reason, sizeof(pktheader.reason),
							"No Associated STA's");
						send_json_response(sockfd, pktheader.reason);
					} else {
						send_associated_sta(sockfd, &tmpdutinfo, stas_list);
						if (stas_list != NULL) {
							free(stas_list);
							stas_list = NULL;
						}
					}
				}
			} else if (strcmp(pktheader.reqresptype, PACKET_REQCHANNELCAPACITY) == 0) {
				dut_info_t		dutdet;
				congestion_list_t	*congestion;
				int			capacity = 0;

				memset(&dutdet, 0x00, sizeof(dut_info_t));

				/* ret = is_dut_present(dutinfo.mac, &dutinfo.rowid, TRUE); */
				ret = get_dut_details_from_db(dutinfo.mac, &dutdet);
				if (ret != 0) {
					snprintf(pktheader.reason, sizeof(pktheader.reason),
						"DB Read Error");
					send_json_response(sockfd, pktheader.reason);
				} else {
					channel_maplist_t *chanmap = NULL;
					assoc_sta_list_t *stas_list = NULL;

					congestion = get_channel_capacity_from_db(dutdet.rowid,
						pktheader.band,
						dutdet.ctrlch, &capacity);
					chanmap = get_channel_map(dutdet.rowid);
					stas_list = get_associated_sta_from_db(dutdet.rowid);
					send_channel_capacity(sockfd, &dutdet,
						congestion, capacity, chanmap, pktheader.band,
						stas_list);
					if (congestion != NULL) {
						free(congestion);
						congestion = NULL;
					}
					if (chanmap != NULL) {
						free(chanmap);
						chanmap = NULL;
					}
					if (stas_list != NULL) {
						free(stas_list);
						stas_list = NULL;
					}
				}
			} else if (strcmp(pktheader.reqresptype, PACKET_REQCHANIM) == 0) {
				/* Chanim statistics of current channel for metrics page */
				dut_info_t		dutdet;
				congestion_list_t	*congestion;

				memset(&dutdet, 0x00, sizeof(dut_info_t));

				ret = get_dut_details_from_db(dutinfo.mac, &dutdet);
				if (ret != 0) {
					snprintf(pktheader.reason, sizeof(pktheader.reason),
						"DB Read Error");
					send_json_response(sockfd, pktheader.reason);
				} else {
					congestion = get_chanim_from_db(dutdet.rowid,
						pktheader.band, dutdet.ctrlch);
					send_chanim_metrics_stats(sockfd, &dutdet,
						congestion, pktheader.band);
					if (congestion != NULL) {
						free(congestion);
						congestion = NULL;
					}
				}
			} else if (strcmp(pktheader.reqresptype, PACKET_REQAMPDUSTATISTICS) == 0) {
				tx_ampdu_list_t	*ampdulist;

				ret = is_dut_present(dutinfo.mac, &dutinfo.rowid, TRUE, TRUE);
				if (ret != 0) {
					snprintf(pktheader.reason, sizeof(pktheader.reason),
						"DB Read Error");
					send_json_response(sockfd, pktheader.reason);
				} else {
					ampdulist = get_ampdu_stats_from_db(dutinfo.rowid,
						pktheader.band);
					send_ampdu_stats(sockfd, ampdulist);
					if (ampdulist != NULL) {
						free(ampdulist);
						ampdulist = NULL;
					}
				}
			} else if (strcmp(pktheader.reqresptype, PACKET_REQSETCONFIG) == 0) {
				configs_t		configs;
				onlygraphnamelist_t	*graphname = NULL;

				memset(&configs, 0x00, sizeof(configs_t));
				configs.version = version;
				ret = get_config_from_packet(doc, node->children, &configs);

				graphname = get_enable_grphnames_from_packet(doc, node->children);

				if (ret == 0) {
					if (configs.dbsize > 0)
						set_max_db_size(configs.dbsize);
					ret = save_configs_in_db(&configs, TRUE);
					ret = update_enable_graphs(graphname);
					if (ret == 0)
						snprintf(pktheader.reason, sizeof(pktheader.reason),
							"Successfully Saved");
					else
						snprintf(pktheader.reason, sizeof(pktheader.reason),
							"Failed to Save to DB");
					update_sta_graphs_status(graphname);
					copy_config_settings(&configs, FALSE);
				} else
					snprintf(pktheader.reason, sizeof(pktheader.reason),
						"Failed to Parse Request");

				if (graphname != NULL) {
					free(graphname);
					graphname = NULL;
				}
				send_json_response(sockfd, pktheader.reason);
			} else if (strcmp(pktheader.reqresptype, PACKET_REQBANDS) == 0) {
				dut_list_t *dutlist = NULL;

				dutlist = get_all_duts_from_db();
				send_all_duts(sockfd, dutlist);
				if (dutlist != NULL) {
					free(dutlist);
					dutlist = NULL;
				}
			} else if (strcmp(pktheader.reqresptype, PACKET_REQRRMSTASTATS) == 0) {
				ret = is_dut_present(dutinfo.mac, &dutinfo.rowid, TRUE, TRUE);
				if (ret != 0) {
					snprintf(pktheader.reason, sizeof(pktheader.reason),
						"DB Read Error");
					send_json_response(sockfd, pktheader.reason);
				} else {
					vis_sta_stats_t stastats;

					memset(&stastats, 0, sizeof(stastats));
					get_stastats_from_db(dutinfo.rowid,
						dutinfo.stamac, &stastats);
					send_rrm_stastats(sockfd, &stastats);
				}
			} else if (strcmp(pktheader.reqresptype, PACKET_REQRRMADVSTASTATS) == 0) {
				ret = is_dut_present(dutinfo.mac, &dutinfo.rowid, TRUE, TRUE);
				if (ret != 0) {
					snprintf(pktheader.reason, sizeof(pktheader.reason),
						"DB Read Error");
					send_json_response(sockfd, pktheader.reason);
				} else {
					vis_rrm_statslist_t *rrmstatslist;
					rrmstatslist = get_rrm_adv_stastats_from_db(dutinfo.rowid,
						dutinfo.stamac);
					ret = get_adv_sta_stats_from_db(sockfd,
						"Packet Queue Statistics", dutinfo.rowid,
						dutinfo.stamac, rrmstatslist);
					if (rrmstatslist != NULL) {
						free(rrmstatslist);
						rrmstatslist = NULL;
					}
				}
			} else {
				ret = is_dut_present(dutinfo.mac, &dutinfo.rowid, TRUE, TRUE);
				if (ret != 0) {
					snprintf(pktheader.reason, sizeof(pktheader.reason),
						"DB Read Error");
					send_json_response(sockfd, pktheader.reason);
				} else {
					ret = get_tab_data_from_db(sockfd,
						pktheader.reqresptype, dutinfo.rowid,
						dutinfo.stamac);
					if (ret != 0) {
						VIS_XML("Failed to send graph data for %d\n",
							sockfd);
					}
				}
			}
		}
	}

	xmlFreeDoc(doc);

	return ret;
}

/* Sends the config to data collector in XML format */
int
send_config(int sockfd, configs_t *configs)
{
	int			ret = 0;
	xmlDocPtr		doc = NULL;
	xmlNodePtr		root_node = NULL;
	packet_header_t		pktheader;

	memset(&pktheader, 0x00, sizeof(packet_header_t));

	doc = xmlNewDoc((const xmlChar*)"1.0");
	add_packet_version(&root_node, PACKET_VERSION);
	xmlDocSetRootElement(doc, root_node);

	pktheader.type = PACKET_TYPE_RESP;
	pktheader.from = PACKET_FROM_DCON;
	snprintf(pktheader.reqresptype, sizeof(pktheader.reqresptype), "%s", PACKET_REQLOGIN);
	pktheader.result = PACKET_SENT_SUCCESS;

	add_packet_header_node(&root_node, &pktheader);

	/* Add Config Details */
	add_config_node(&root_node, configs);

	ret = send_xml_data(sockfd, &doc);

	return ret;
}

/* sends XML response to data collector */
int
send_response(int sockfd, packet_header_t *pktheaderin, dut_settings_t *dutset)
{
	xmlDocPtr		doc = NULL;
	xmlNodePtr		root_node = NULL;
	int			ret = 0;
	packet_header_t		pktheader;
	configs_t		configs;

	memset(&configs, 0x00, sizeof(configs_t));
	memset(&pktheader, 0x00, sizeof(packet_header_t));

	doc = xmlNewDoc((const xmlChar*)"1.0");
	add_packet_version(&root_node, PACKET_VERSION);
	xmlDocSetRootElement(doc, root_node);

	pktheader.type = PACKET_TYPE_RESP;
	pktheader.from = PACKET_FROM_DCON;
	snprintf(pktheader.reqresptype, sizeof(pktheader.reqresptype), "%s",
		pktheaderin->reqresptype);
	pktheader.result = pktheaderin->result;

	if (pktheaderin->result != PACKET_SENT_SUCCESS && strlen(pktheaderin->reason) > 0)
		snprintf(pktheader.reason, sizeof(pktheader.reason), "%s", pktheaderin->reason);

	add_packet_header_node(&root_node, &pktheader);

	/* Add Config Settings */
	copy_config_settings(&configs, TRUE);
	add_config_node(&root_node, &g_configs);
	add_dut_settings_node(&root_node, dutset);
	ret = send_xml_data(sockfd, &doc);

	return ret;
}

/* Parse the counters request in XML format which is from data collector */
static int
parse_counters_request(int sockfd, xmlDocPtr doc, xmlNodePtr node, dut_info_t *dutinfo)
{
	int			ret = 0;
	networks_list_t		*networks_list = NULL;
	assoc_sta_list_t	*stas_list = NULL;
	congestion_list_t	*congestion = NULL;
	tx_ampdu_list_t		*ampdulist = NULL;
	vis_rrm_statslist_t	*rrmstatslist = NULL;
	vis_sta_statslist_t	*stastatslist = NULL;

	if (node == NULL) /* If there is no node inside packet version */
		return -1;

	while (node != NULL) { /* search for tags */
		if (strcmp((const char*)node->name, TAG_NETWORKS) == 0) {
			networks_list = parse_networks_node(doc, node->children);
			if (networks_list != NULL) {
				/* now store the networks details into DB */
				if (networks_list->length > 0)
					insert_networks_details(dutinfo, networks_list);
				else
					insert_empty_networks_details(dutinfo, networks_list);
				if (networks_list != NULL) {
					free(networks_list);
					networks_list = NULL;
				}
				/* Now update the dut settings table for scan settings */
				dut_settings_t tmpdutset;

				memset(&tmpdutset, 0x00, sizeof(dut_settings_t));
				get_current_dut_settings(dutinfo->rowid, &tmpdutset);
				if (tmpdutset.isscan == 1) { /* if 1 make it to 0 */
					tmpdutset.rowid = dutinfo->rowid;
					tmpdutset.isscan = 0;
					update_dut_settings_in_db(&tmpdutset);
				}
			}
		} else if (strcmp((const char*)node->name, TAG_ASSOCIATEDSTA) == 0) {
			stas_list = parse_associated_sta_node(doc, node->children);
			if (stas_list != NULL) {
				insert_assocsta_details(dutinfo, stas_list);
				if (stas_list != NULL) {
					free(stas_list);
					stas_list = NULL;
				}
			}
		} else if (strcmp((const char*)node->name, TAG_CHANNELSTATS) == 0) {
			congestion = parse_channel_stats_node(doc, node->children);
			if (congestion != NULL) {
				insert_channel_stats_details(dutinfo, congestion);
				if (congestion != NULL) {
					free(congestion);
					congestion = NULL;
				}
			}
		} else if (strcmp((const char*)node->name, TAG_AMPDUSTATS) == 0) {
			ampdulist = parse_ampdu_stats_node(doc, node->children);
			if (ampdulist != NULL) {
				insert_ampdu_stats_details(dutinfo, ampdulist);
				if (ampdulist != NULL) {
					free(ampdulist);
					ampdulist = NULL;
				}
			}
		} else if (strcmp((const char*)node->name, TAG_RRMSTASTATS) == 0) {
			rrmstatslist = parse_rrm_all_sta_stats_node(doc, node->children);
			if (rrmstatslist != NULL) {
				insert_rrm_adv_sta_side_stats_details(dutinfo, rrmstatslist);
				if (rrmstatslist != NULL) {
					free(rrmstatslist);
					rrmstatslist = NULL;
				}
			}
		} else if (strcmp((const char*)node->name, TAG_STAINFOSTATS) == 0) {
			stastatslist = parse_all_sta_stats_info_node(doc, node->children);
			if (stastatslist != NULL) {
				insert_sta_side_stats_details(dutinfo, stastatslist);
				if (stastatslist != NULL) {
					free(stastatslist);
					stastatslist = NULL;
				}
			}
		} else if (strcmp((const char*)node->name, TAG_GRAPH) == 0) {
			graph_list_t		*graphs = NULL;
			graphdetails_t		graphdet;
			iovar_handler_t		*handle;
			char			graphname[MAX_GRAPH_NAME];
			int			ret1 = 0;

			memset(&graphdet, 0x00, sizeof(graph_list_t));

			graphs = parse_graphlist_node(doc, node->children, graphname);
			if (graphs != NULL) {
				handle = find_iovar(graphname);
				if (handle != NULL) {
					ret1 = get_graph_row(&graphdet, handle);
					if (ret1 == 0)
						insert_graph_details(sockfd, dutinfo,
							graphs, &graphdet, handle);
				}

				free(graphs);
				graphs = NULL;
			}
		}
		node = node->next;
	}

	return ret;
}

/* parse site survey result from XML and return the structure */
static networks_list_t*
parse_networks_node(xmlDocPtr doc, xmlNodePtr node)
{
	networks_list_t *networks_list;
	xmlNodePtr	nodetotal;
	int		total = 0, i = 0, szalloclen = 0;

	if (node == NULL) /* If there is no node inside */
		return NULL;

	/* get the total AP's around */
	nodetotal = node;
	while (nodetotal != NULL) { /* search for tag Total to get the total number of AP's */
		if (strcmp((const char*)nodetotal->name, TAG_TOTAL) == 0) {
			get_int_element(doc, nodetotal, &total);
			break;
		}
		nodetotal = nodetotal->next;
	}

	szalloclen = (sizeof(networks_list_t) + (sizeof(ap_info_t) * total));
	/* Now allocate memory for networks_list structure */
	networks_list = (networks_list_t*)malloc(szalloclen);
	if (networks_list == NULL) {
		VIS_XML("Failed to allocate networks_list buffer of %d\n",
			szalloclen);
		return NULL;
	}
	memset(networks_list, 0x00, szalloclen);

	networks_list->length = total;

	while (node != NULL) { /* search for tag's AP */
		if (strcmp((const char*)node->name, TAG_TIMESTAMP) == 0) {
			get_long_element(doc, node, &networks_list->timestamp);
		} else if (strcmp((const char*)node->name, TAG_AP) == 0) {
			if (parse_ap_node(doc, node->children, i, networks_list) == 0)
				i++;
		}
		node = node->next;
	}

	return networks_list;
}

/* Parse the AP node in site survey result */
static int
parse_ap_node(xmlDocPtr doc, xmlNodePtr node, int i, networks_list_t *networks_list)
{
	int ret = 0;

	if (node == NULL) /* If there is no node inside */
		return -1;

	while (node != NULL) { /* search for subtag's under AP tag */
		if (strcmp((const char*)node->name, TAG_SSID) == 0) {
			get_cdata_element(node, networks_list->aps[i].ssid,
				sizeof(networks_list->aps[i].ssid));
		} else if (strcmp((const char*)node->name, TAG_BSSID) == 0) {
			get_string_element(doc, node, networks_list->aps[i].bssid,
				(ETHER_ADDR_LEN * 3));
		} else if (strcmp((const char*)node->name, TAG_RSSI) == 0) {
			get_int_element(doc, node, (int*)&networks_list->aps[i].rssi);
		} else if (strcmp((const char*)node->name, TAG_NOISE) == 0) {
			get_int_element(doc, node, (int*)&networks_list->aps[i].noise);
		} else if (strcmp((const char*)node->name, TAG_CHANNEL) == 0) {
			get_int_element(doc, node, (int*)&networks_list->aps[i].channel);
		} else if (strcmp((const char*)node->name, TAG_CONTROLCH) == 0) {
			get_int_element(doc, node, (int*)&networks_list->aps[i].ctrlch);
		} else if (strcmp((const char*)node->name, TAG_BAND) == 0) {
			get_int_element(doc, node, (int*)&networks_list->aps[i].band);
		} else if (strcmp((const char*)node->name, TAG_BANDWIDTH) == 0) {
			get_int_element(doc, node, (int*)&networks_list->aps[i].bandwidth);
		} else if (strcmp((const char*)node->name, TAG_RATE) == 0) {
			get_int_element(doc, node, (int*)&networks_list->aps[i].maxrate);
		} else if (strcmp((const char*)node->name, TAG_TYPE) == 0) {
			get_string_element(doc, node, networks_list->aps[i].networktype,
				MAX_NETWORK_TYPE);
		} else if (strcmp((const char*)node->name, TAG_MCAST_RSN) == 0) {
			get_string_element(doc, node, networks_list->aps[i].mcastrsntype,
				MAX_MULTICAST_RSN);
		} else if (strcmp((const char*)node->name, TAG_UCAST_RSN) == 0) {
			get_string_element(doc, node, networks_list->aps[i].ucastrsntype,
				MAX_UNICAST_RSN);
		} else if (strcmp((const char*)node->name, TAG_AKM) == 0) {
			get_string_element(doc, node, networks_list->aps[i].akmrsntype,
				MAX_AKM_TYPE);
		}
		node = node->next;
	}

	return ret;
}

/* Parse the associated STA node from XML and copy it to structure */
static assoc_sta_list_t*
parse_associated_sta_node(xmlDocPtr doc, xmlNodePtr node)
{
	assoc_sta_list_t	*stas_list = NULL;
	xmlNodePtr		nodetotal;
	int			total = 0, i = 0;

	if (node == NULL) /* If there is no node inside */
		return NULL;

	/* get the total associated STA's around */
	nodetotal = node;
	while (nodetotal != NULL) { /* search for tag Total */
		if (strcmp((const char*)nodetotal->name, TAG_TOTAL) == 0) {
			get_int_element(doc, nodetotal, &total);
			break;
		}
		nodetotal = nodetotal->next;
	}

	/* Now allocate memory for networks_list structure */
	stas_list = (assoc_sta_list_t*)malloc(sizeof(assoc_sta_list_t) +
		(sizeof(assoc_sta_t) * total));
	if (stas_list == NULL) {
		VIS_XML("Failed to allocate stas_list buffer of %d\n",
			total);
		return NULL;
	}

	stas_list->length = total;

	while (node != NULL) { /* search for tag's STA */
		if (strcmp((const char*)node->name, TAG_TIMESTAMP) == 0) {
			get_long_element(doc, node, &stas_list->timestamp);
		} else if (strcmp((const char*)node->name, TAG_STA) == 0) {
			if (parse_sta_node(doc, node->children, i, stas_list) == 0)
				i++;
		}
		node = node->next;
	}

	return stas_list;
}

/* Parse individual STA node from associated STA node */
static int
parse_sta_node(xmlDocPtr doc, xmlNodePtr node, int i, assoc_sta_list_t *stas_list)
{
	int ret = 0;

	if (node == NULL) /* If there is no node inside */
		return -1;

	while (node != NULL) { /* search for subtag's under STA tag */
		if (strcmp((const char*)node->name, TAG_MAC) == 0) {
			get_string_element(doc, node, stas_list->sta[i].mac, (ETHER_ADDR_LEN * 3));
		} else if (strcmp((const char*)node->name, TAG_RSSI) == 0) {
			get_int_element(doc, node, (int*)&stas_list->sta[i].rssi);
		} else if (strcmp((const char*)node->name, TAG_PHYRATE) == 0) {
			get_int_element(doc, node, (int*)&stas_list->sta[i].phyrate);
		}
		node = node->next;
	}

	return ret;
}

/* Parse the individual value node from graphs node */
static int
parse_value_node(xmlDocPtr doc, xmlNodePtr node, int i, graph_list_t *graphs)
{
	int ret = 0;

	if (node == NULL) /* If there is no node inside */
		return -1;

	while (node != NULL) { /* search for subtag's under VALUE tag */
		if (strcmp((const char*)node->name, TAG_TIMESTAMP) == 0) {
			get_long_element(doc, node, &graphs->graph[i].timestamp);
		} else if (strcmp((const char*)node->name, TAG_X_VALUE) == 0) {
			get_int_element(doc, node, (int*)&graphs->graph[i].xvalue);
		} else if (strcmp((const char*)node->name, TAG_Y_VALUE) == 0) {
			get_string_element(doc, node, graphs->graph[i].yvalue, MAX_YVALUE_LEN);
		}
		node = node->next;
	}

	return ret;
}

/* Parses graph list node from data collector */
static graph_list_t*
parse_graphlist_node(xmlDocPtr doc, xmlNodePtr node, char *graphname)
{
	graph_list_t	*graphs = NULL;
	xmlNodePtr	nodetotal;
	int		total = 0, i = 0;

	if (node == NULL) /* If there is no node inside */
		return NULL;

	/* get the total graph nodes */
	nodetotal = node;
	while (nodetotal != NULL) { /* search for tag Total */
		if (strcmp((const char*)nodetotal->name, TAG_TOTAL) == 0) {
			get_int_element(doc, nodetotal, &total);
			break;
		}
		nodetotal = nodetotal->next;
	}

	if (total <= 0)
		return NULL;

	/* Now allocate memory for graphs structure */
	graphs = (graph_list_t*)malloc(sizeof(graph_list_t) +
		(sizeof(graphs_t) * total));
	if (graphs == NULL) {
		VIS_XML("Failed to allocate graphs buffer of %d\n",
			total);
		return NULL;
	}

	memset(graphs, 0x00, total);
	graphs->length = total;

	while (node != NULL) { /* search for tag's */
		if (strcmp((const char*)node->name, TAG_TIMESTAMP) == 0) {
			get_long_element(doc, node, &graphs->timestamp);
		} else if (strcmp((const char*)node->name, TAG_MAC) == 0) {
			get_string_element(doc, node, graphs->mac, (ETHER_ADDR_LEN * 3));
		} else if (strcmp((const char*)node->name, TAG_NAME) == 0) {
			get_string_element(doc, node, graphname, MAX_GRAPH_NAME);
		} else if (strcmp((const char*)node->name, TAG_VALUE) == 0) {
			if (total == i)
				break;
			if (parse_value_node(doc, node->children, i, graphs) == 0)
				i++;
		}
		node = node->next;
	}

	return graphs;
}

/* Parse the enabled graphs list from web UI */
onlygraphnamelist_t*
get_enable_grphnames_from_packet(xmlDocPtr doc, xmlNodePtr node)
{
	int			i = 0;
	onlygraphnamelist_t	*graphname = NULL;
	int			total = 0;
	xmlNodePtr		nodetotal;

	if (node == NULL) /* If there is no node inside packet version */
		return NULL;

	while (node != NULL) { /* search for tag config */
		if (strcmp((const char*)node->name, TAG_CONFIG) == 0) {
			break;
		}
		node = node->next;
	}

	if (node == NULL) /* no tag named Config */
		return NULL;

	node = node->children;
	while (node != NULL) {
		if (strcmp((const char*)node->name, TAG_ENABLEDGRAPHS) == 0) {
			break;
		}
		node = node->next;
	}
	if (node == NULL) /* no tag named EnabledGraphs */
		return NULL;

	/* get the total graphs */
	nodetotal = node->children;
	while (nodetotal != NULL) { /* search for tag Total to get the total number of graphs */
		if (strcmp((const char*)nodetotal->name, TAG_TOTAL) == 0) {
			get_int_element(doc, nodetotal, &total);
			break;
		}
		nodetotal = nodetotal->next;
	}

	if (total <= 0)
		return NULL;

	graphname = (onlygraphnamelist_t*)malloc(sizeof(onlygraphnamelist_t) +
		(sizeof(onlygraphname_t) * total));
	if (graphname == NULL) {
		VIS_XML("Failed to allocate onlygraphnamelist_t of size : %d\n",
			total);
		return NULL;
	}
	graphname->length = total;

	node = node->children;
	while (node != NULL) {
		if (strcmp((const char*)node->name, TAG_GRAPHNAME) == 0) {
			get_string_element(doc, node, graphname->graphname[i].name, MAX_GRAPH_NAME);
			replace_plus_with_space(graphname->graphname[i].name);
			i++;
		}
		node = node->next;
	}

#ifdef SHOW_XML_DEBUG_MSG
	for (i = 0; i < graphname->length; i++) {
		printf("GRAPH NAME %d : %s\n", i, graphname->graphname[i].name);
	}
#endif /* SHOW_XML_DEBUG_MSG */

	return graphname;
}

/* Copy config structure to and from global config structure */
void
copy_config_settings(configs_t *configs, int fromgconfig)
{
	lock_config_mutex();

	if (fromgconfig) {
		configs->version	= g_configs.version;
		configs->interval	= g_configs.interval;
		configs->dbsize		= g_configs.dbsize;
		configs->isstart	= g_configs.isstart;
		configs->isremoteenab	= g_configs.isremoteenab;
		snprintf(configs->gatewayip, sizeof(configs->gatewayip), "%s", g_configs.gatewayip);
		configs->isoverwrtdb	= g_configs.isoverwrtdb;
		configs->isautostart	= g_configs.isautostart;
		configs->weekdays	= g_configs.weekdays;
		configs->fromtm	= g_configs.fromtm;
		configs->totm	= g_configs.totm;
	} else {
		g_configs.version	= configs->version;
		g_configs.interval	= configs->interval;
		g_configs.dbsize	= configs->dbsize;
		g_configs.isstart	= configs->isstart;
		g_configs.isoverwrtdb	= configs->isoverwrtdb;
		g_configs.isautostart	= configs->isautostart;
		g_configs.weekdays	= configs->weekdays;
		g_configs.fromtm	= configs->fromtm;
		g_configs.totm	= configs->totm;
	}

	unlock_config_mutex();
}

/* Copies the gateway ip to global config structure */
void
copy_gateway_ip_to_config(char *gatewayip)
{
	lock_config_mutex();
	snprintf(g_configs.gatewayip, sizeof(g_configs.gatewayip), "%s", gatewayip);
	unlock_config_mutex();
}

/* Parse channel statistics node from data collector */
static congestion_list_t*
parse_channel_stats_node(xmlDocPtr doc, xmlNodePtr node)
{
	congestion_list_t	*congestion = NULL;
	xmlNodePtr		nodetotal;
	int			total = 0, i = 0;

	if (node == NULL) /* If there is no node inside */
		return NULL;

	/* get the total channel stats */
	nodetotal = node;
	while (nodetotal != NULL) { /* search for tag Total */
		if (strcmp((const char*)nodetotal->name, TAG_TOTAL) == 0) {
			get_int_element(doc, nodetotal, &total);
			break;
		}
		nodetotal = nodetotal->next;
	}

	/* Now allocate memory for structure */
	congestion = (congestion_list_t*)malloc(sizeof(congestion_list_t) +
		(sizeof(congestion_t) * total));
	if (congestion == NULL) {
		VIS_XML("Failed to allocate congestion buffer for %d channels\n",
			total);
		return NULL;
	}

	congestion->length = total;

	while (node != NULL) { /* search for tag's STats */
		if (strcmp((const char*)node->name, TAG_TIMESTAMP) == 0) {
			get_long_element(doc, node, &congestion->timestamp);
		} else if (strcmp((const char*)node->name, TAG_STATS) == 0) {
			if (parse_stats_node(doc, node->children, i, congestion) == 0)
				i++;
		}
		node = node->next;
	}

	return congestion;
}

/* Parse each channel statistics node and copy it to structure */
static int
parse_stats_node(xmlDocPtr doc, xmlNodePtr node, int i, congestion_list_t *congestion)
{
	int ret = 0;

	if (node == NULL) /* If there is no node inside */
		return -1;

	while (node != NULL) { /* search for subtag's under STats tag */
		if (strcmp((const char*)node->name, TAG_CHANNEL) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].channel);
		} else if (strcmp((const char*)node->name, TAG_TX) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].tx);
		} else if (strcmp((const char*)node->name, TAG_INBSS) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].inbss);
		} else if (strcmp((const char*)node->name, TAG_OBSS) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].obss);
		} else if (strcmp((const char*)node->name, TAG_NOCAT) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].nocat);
		} else if (strcmp((const char*)node->name, TAG_NOPKT) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].nopkt);
		} else if (strcmp((const char*)node->name, TAG_DOZE) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].doze);
		} else if (strcmp((const char*)node->name, TAG_TXOP) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].txop);
		} else if (strcmp((const char*)node->name, TAG_GOODTX) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].goodtx);
		} else if (strcmp((const char*)node->name, TAG_BADTX) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].badtx);
		} else if (strcmp((const char*)node->name, TAG_GLITCH) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].glitchcnt);
		} else if (strcmp((const char*)node->name, TAG_BADPLCP) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].badplcp);
		} else if (strcmp((const char*)node->name, TAG_KNOISE) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].knoise);
		} else if (strcmp((const char*)node->name, TAG_IDLE) == 0) {
			get_int_element(doc, node, (int*)&congestion->congest[i].chan_idle);
		}
		node = node->next;
	}

	return ret;
}

/* Parse each ampdu node and copy it to structure */
static int
parse_ampdu_node(xmlDocPtr doc, xmlNodePtr node, int i, tx_ampdu_list_t *ampdulist)
{
	int ret = 0;

	if (node == NULL) /* If there is no node inside */
		return -1;

	while (node != NULL) { /* search for subtag's under AMPDU tag */
		if (strcmp((const char*)node->name, TAG_MCS) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].mcs);
		} else if (strcmp((const char*)node->name, TAG_TXMCS) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].txmcs);
		} else if (strcmp((const char*)node->name, TAG_TXMCSPERCENT) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].txmcspercent);
		} else if (strcmp((const char*)node->name, TAG_TXMCSSGI) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].txmcssgi);
		} else if (strcmp((const char*)node->name, TAG_TXMCSSGIPERCENT) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].txmcssgipercent);
		} else if (strcmp((const char*)node->name, TAG_RXMCS) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].rxmcs);
		} else if (strcmp((const char*)node->name, TAG_RXMCSPERCENT) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].rxmcspercent);
		} else if (strcmp((const char*)node->name, TAG_RXMCSSGI) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].rxmcssgi);
		} else if (strcmp((const char*)node->name, TAG_RXMCSSGIPERCENT) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].rxmcssgipercent);
		} else if (strcmp((const char*)node->name, TAG_TXVHT) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].txvht);
		} else if (strcmp((const char*)node->name, TAG_TXVHTPER) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].txvhtper);
		} else if (strcmp((const char*)node->name, TAG_TXVHTPERCENT) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].txvhtpercent);
		} else if (strcmp((const char*)node->name, TAG_TXVHTSGI) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].txvhtsgi);
		} else if (strcmp((const char*)node->name, TAG_TXVHTSGIPER) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].txvhtsgiper);
		} else if (strcmp((const char*)node->name, TAG_TXVHTSGIPERCENT) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].txvhtsgipercent);
		} else if (strcmp((const char*)node->name, TAG_RXVHT) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].rxvht);
		} else if (strcmp((const char*)node->name, TAG_RXVHTPER) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].rxvhtper);
		} else if (strcmp((const char*)node->name, TAG_RXVHTPERCENT) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].rxvhtpercent);
		} else if (strcmp((const char*)node->name, TAG_RXVHTSGI) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].rxvhtsgi);
		} else if (strcmp((const char*)node->name, TAG_RXVHTSGIPER) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].rxvhtsgiper);
		} else if (strcmp((const char*)node->name, TAG_RXVHTSGIPERCENT) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].rxvhtsgipercent);
		} else if (strcmp((const char*)node->name, TAG_MPDUDENS) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].mpdudens);
		} else if (strcmp((const char*)node->name, TAG_MPDUDENSPERCENT) == 0) {
			get_int_element(doc, node, (int*)&ampdulist->ampdus[i].mpdudenspercent);
		}
		node = node->next;
	}

	return ret;
}

/* Parse ampdu statistics node from data collector */
static tx_ampdu_list_t*
parse_ampdu_stats_node(xmlDocPtr doc, xmlNodePtr node)
{
	tx_ampdu_list_t	*ampdulist = NULL;
	xmlNodePtr		nodetotal;
	int			total = 0, i = 0, szalloclen;

	if (node == NULL) /* If there is no node inside */
		return NULL;

	/* get the total ampdu stats */
	nodetotal = node;
	while (nodetotal != NULL) { /* search for tag Total */
		if (strcmp((const char*)nodetotal->name, TAG_TOTAL) == 0) {
			get_int_element(doc, nodetotal, &total);
			break;
		}
		nodetotal = nodetotal->next;
	}

	/* Now allocate memory for structure */
	szalloclen = (sizeof(tx_ampdu_list_t) + (sizeof(tx_ampdu_t) * total));
	ampdulist = (tx_ampdu_list_t*)malloc(szalloclen);
	if (ampdulist == NULL) {
		VIS_XML("Failed to allocate ampdulist buffer for %d ampdus\n", total);
		return NULL;
	}
	memset(ampdulist, 0x00, szalloclen);
	ampdulist->length = total;

	while (node != NULL) { /* search for tag's AMPDU */
		if (strcmp((const char*)node->name, TAG_TIMESTAMP) == 0) {
			get_long_element(doc, node, &ampdulist->timestamp);
		} else if (strcmp((const char*)node->name, TAG_AMPDU) == 0) {
			if (parse_ampdu_node(doc, node->children, i, ampdulist) == 0)
				i++;
		}
		node = node->next;
	}

	return ampdulist;
}

/* Parse the associated STA node from XML and copy it to structure */
static vis_rrm_statslist_t*
parse_rrm_all_sta_stats_node(xmlDocPtr doc, xmlNodePtr node)
{
	vis_rrm_statslist_t	*rrmstatslist = NULL;
	xmlNodePtr		nodetotal;
	int			total = 0, i = 0;

	if (node == NULL) /* If there is no node inside */
		return NULL;

	/* get the total STA's */
	nodetotal = node;
	while (nodetotal != NULL) { /* search for tag Total */
		if (strcmp((const char*)nodetotal->name, TAG_TOTAL) == 0) {
			get_int_element(doc, nodetotal, &total);
			break;
		}
		nodetotal = nodetotal->next;
	}

	/* Now allocate memory for vis_rrm_statslist_t structure */
	rrmstatslist = (vis_rrm_statslist_t*)malloc(sizeof(vis_rrm_statslist_t) +
		(sizeof(vis_rrm_stats_t) * total));
	if (rrmstatslist == NULL) {
		VIS_XML("Failed to allocate rrmstatslist buffer of %d\n",
			total);
		return NULL;
	}

	rrmstatslist->length = total;

	while (node != NULL) { /* search for tag's STA */
		if (strcmp((const char*)node->name, TAG_TIMESTAMP) == 0) {
			get_long_element(doc, node, &rrmstatslist->timestamp);
		} else if (strcmp((const char*)node->name, TAG_RRMSTA) == 0) {
			if (parse_rrm_sta_node(doc, node->children, i, rrmstatslist) == 0)
				i++;
		}
		node = node->next;
	}

	return rrmstatslist;
}

/* Parse individual RRM STA node */
static int
parse_rrm_sta_node(xmlDocPtr doc, xmlNodePtr node, int i, vis_rrm_statslist_t *rrmstatslist)
{
	int ret = 0;

	if (node == NULL) /* If there is no node inside */
		return -1;

	while (node != NULL) { /* search for subtag's under RRMSTA tag */
		if (strcmp((const char*)node->name, TAG_MAC) == 0) {
			get_string_element(doc, node, rrmstatslist->rrmstats[i].mac,
				(ETHER_ADDR_LEN * 3));
		} else if (strcmp((const char*)node->name, TAG_TXOP) == 0) {
			get_int_element(doc, node, (int*)&rrmstatslist->rrmstats[i].txop);
		} else if (strcmp((const char*)node->name, TAG_PKTREQUESTED) == 0) {
			get_int_element(doc, node, (int*)&rrmstatslist->rrmstats[i].pktrequested);
		} else if (strcmp((const char*)node->name, TAG_PKTDROPPED) == 0) {
			get_int_element(doc, node, (int*)&rrmstatslist->rrmstats[i].pktdropped);
		} else if (strcmp((const char*)node->name, TAG_PKTSTORED) == 0) {
			get_int_element(doc, node, (int*)&rrmstatslist->rrmstats[i].pktstored);
		} else if (strcmp((const char*)node->name, TAG_PKTRETRIED) == 0) {
			get_int_element(doc, node, (int*)&rrmstatslist->rrmstats[i].pktretried);
		} else if (strcmp((const char*)node->name, TAG_PKTACKED) == 0) {
			get_int_element(doc, node, (int*)&rrmstatslist->rrmstats[i].pktacked);
		}
		node = node->next;
	}

	return ret;
}

/* Parse the associated STA node from XML and copy it to structure */
static vis_sta_statslist_t*
parse_all_sta_stats_info_node(xmlDocPtr doc, xmlNodePtr node)
{
	vis_sta_statslist_t	*stastatslist = NULL;
	xmlNodePtr		nodetotal;
	int			total = 0, i = 0;

	if (node == NULL) /* If there is no node inside */
		return NULL;

	/* get the total STA's */
	nodetotal = node;
	while (nodetotal != NULL) { /* search for tag Total */
		if (strcmp((const char*)nodetotal->name, TAG_TOTAL) == 0) {
			get_int_element(doc, nodetotal, &total);
			break;
		}
		nodetotal = nodetotal->next;
	}

	/* Now allocate memory for vis_sta_statslist_t structure */
	stastatslist = (vis_sta_statslist_t*)malloc(sizeof(vis_sta_statslist_t) +
		(sizeof(vis_rrm_stats_t) * total));
	if (stastatslist == NULL) {
		VIS_XML("Failed to allocate stastatslist buffer of %d\n",
			total);
		return NULL;
	}

	stastatslist->length = total;

	while (node != NULL) { /* search for tag's STA */
		if (strcmp((const char*)node->name, TAG_TIMESTAMP) == 0) {
			get_long_element(doc, node, &stastatslist->timestamp);
		} else if (strcmp((const char*)node->name, TAG_STASTATS) == 0) {
			if (parse_sta_info_node(doc, node->children, i, stastatslist) == 0)
				i++;
		}
		node = node->next;
	}

	return stastatslist;
}

/* Parse individual STA info node */
static int
parse_sta_info_node(xmlDocPtr doc, xmlNodePtr node, int i, vis_sta_statslist_t *stastatslist)
{
	int ret = 0;

	if (node == NULL) /* If there is no node inside */
		return -1;

	while (node != NULL) { /* search for subtag's under STAINFO tag */
		if (strcmp((const char*)node->name, TAG_MAC) == 0) {
			get_string_element(doc, node, stastatslist->stastats[i].mac,
				(ETHER_ADDR_LEN * 3));
		} else if (strcmp((const char*)node->name, TAG_GENERATION) == 0) {
			get_int_element(doc, node,
				(int*)&stastatslist->stastats[i].generation_flag);
		} else if (strcmp((const char*)node->name, TAG_TXSTREAM) == 0) {
			get_int_element(doc, node, (int*)&stastatslist->stastats[i].txstream);
		} else if (strcmp((const char*)node->name, TAG_RXSTREAM) == 0) {
			get_int_element(doc, node, (int*)&stastatslist->stastats[i].rxstream);
		} else if (strcmp((const char*)node->name, TAG_RATE) == 0) {
			get_int_element(doc, node, (int*)&stastatslist->stastats[i].rate);
		} else if (strcmp((const char*)node->name, TAG_IDLE) == 0) {
			get_int_element(doc, node, (int*)&stastatslist->stastats[i].idle);
		}
		node = node->next;
	}

	return ret;
}
