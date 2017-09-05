/*
 * Linux Visualization Data Collector RRM(802.11k) processing implementation
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
 * $Id: vis_rrmprocess.c 655745 2016-08-23 12:02:42Z $
 */

#include <string.h>
#include "vis_rrmprocess.h"
#include "vis_common.h"
#include <bcmendian.h>
#include <pthread.h>
#include <security_ipc.h>
#include "vis_sock_util.h"
#include <unistd.h>

#define IS_REPORT_READ(read_flag, bit) do { if (read_flag & bit) { \
		VIS_ADAPTER("%d bit already read\n", bit); \
		break; \
	} } while (0)

#define VISDCOLL_BUFSIZE_4K	4096

#define VIS_THREAD_STATUS_CLOSED	1
#define VIS_THREAD_STATUS_RUNNING	2
#define VIS_THREAD_STATUS_TOBE_CLSOED	3
#define VIS_RRM_BYTE			255
#define VIS_RRM_MIN_DURATION		4096 /* (ms)time for channel load measurement duration */
#define RRM_ACTFRAME_OPERATING_CLASS	0x16
typedef struct evt_data_queue {
	char *evt_data;
	struct evt_data_queue *next;
	struct evt_data_queue *prev;
} evt_data_queue_t;

/* Mutex lock for queue which stores 802.11k reports */
static pthread_mutex_t g_rrm_queue_lock;
/* Mutex lock for thread used to continously poll the 802.11k through events */
static pthread_mutex_t g_rrm_thread_lock;
/* Mutex lock for thread flag to signal main thread after receiving events */
static pthread_mutex_t g_rrm_thread_flag;
static pthread_cond_t g_rrm_thread_flag_cond;
/* Front and Rear pointer for the queue */
static evt_data_queue_t *g_start = NULL;

/* Status of the thread which tells whether its closed, running or to be closed */
static unsigned int g_thread_status = VIS_THREAD_STATUS_CLOSED;

extern long g_timestamp;
extern char *g_vis_cmdoutbuf;

static void rrm_report_add_node(char *pvt_data, int length);

/*
 * Convert Ethernet address binary data to string representation
 * @param       e       binary data
 * @param       a       string in xx:xx:xx:xx:xx:xx notation
 * @return      a
 */
static char *
ether_etoa(const unsigned char *e, char *a)
{
	char *c = a;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}

/* Initialize the mutexes used for 802.11k report query */
void
rrm_initialize_mutexes()
{
	pthread_mutex_init(&g_rrm_queue_lock, NULL);
	pthread_mutex_init(&g_rrm_thread_lock, NULL);
	pthread_mutex_init(&g_rrm_thread_flag, NULL);
	pthread_cond_init(&g_rrm_thread_flag_cond, NULL);
}

/* Destroys the mutexes used for 802.11k report query */
void
rrm_destroy_mutexes()
{
	pthread_mutex_destroy(&g_rrm_queue_lock);
	pthread_mutex_destroy(&g_rrm_thread_lock);
	pthread_mutex_destroy(&g_rrm_thread_flag);
	pthread_cond_destroy(&g_rrm_thread_flag_cond);
}

/* Set the timed wait on condition variable */
int
rrm_timed_wait()
{
	struct timespec time_to_wait;
	int ret = 0;

	pthread_mutex_lock(&g_rrm_thread_flag);
	clock_gettime(CLOCK_REALTIME, &time_to_wait);
	time_to_wait.tv_sec += 1;
	ret = pthread_cond_timedwait(&g_rrm_thread_flag_cond, &g_rrm_thread_flag, &time_to_wait);
	pthread_mutex_unlock(&g_rrm_thread_flag);

	return ret;
}

/* Signal the condition variable */
void
rrm_signal_thread()
{
	pthread_mutex_lock(&g_rrm_thread_flag);
	pthread_cond_broadcast(&g_rrm_thread_flag_cond);
	pthread_mutex_unlock(&g_rrm_thread_flag);
}

/* Set the flag to be closed if it's running */
void
rrm_thread_close()
{
	/* Lock the thread mutex to check the status of thread */
	pthread_mutex_lock(&g_rrm_thread_lock);
	/* If running set the status to be closed */
	if (g_thread_status == VIS_THREAD_STATUS_RUNNING) {
		g_thread_status = VIS_THREAD_STATUS_TOBE_CLSOED;
	}
	pthread_mutex_unlock(&g_rrm_thread_lock);
}

/* Initializes the socket to interact with EAPD event handler */
static int
rrm_eapd_socket_init()
{
	int reuse = 1;
	struct sockaddr_in sockaddr;
	int event_socket = -1;

	/* open loopback socket to communicate with EAPD */
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sockaddr.sin_port = htons(EAPD_WKSP_VISDCOLL_UDP_SPORT);

	if ((event_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		VIS_ERROR("Unable to create loopback socket\n");
		return -1;
	}

	if (setsockopt(event_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
		VIS_ERROR("Unable to setsockopt to loopback socket %d.\n", event_socket);
		goto exit;
	}

	if (bind(event_socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
		VIS_ERROR("Unable to bind to loopback socket %d\n", event_socket);
		goto exit;
	}

	VIS_ERROR("opened loopback socket %d\n", event_socket);

	return event_socket;

/* error handling */
exit:
	close(event_socket);

	return -1;
}

/* Adds the 802.11k reports to the doubly list */
static void
rrm_report_add_node(char *pkt, int length)
{
	evt_data_queue_t *temp = NULL;
	int i = 0;
	pthread_mutex_lock(&g_rrm_queue_lock);

	temp = (evt_data_queue_t*)malloc(sizeof(*temp));
	if (temp == NULL) {
		VIS_ADAPTER("Failed to allocate memory for evt_data_queue_t\n");
		return;
	}
	temp->next = NULL;
	temp->prev = NULL;
	temp->evt_data = NULL;
	temp->evt_data = (char *)malloc(length);
	if (temp->evt_data == NULL) {
		VIS_ADAPTER("Failed to allocate memory for evt_data_queue_t->evt_data\n");
		if (temp) {
			free(temp);
			temp = NULL;
		}
		return;
	}

	while (i < length) {
		*(temp->evt_data + i) = *(pkt + i);
		i++;
	}
	if (g_start == NULL) {
		g_start = temp;
		g_start->prev = NULL;
		g_start->next = NULL;
	} else {
		temp->next = g_start;
		g_start->prev = temp;
		g_start = temp;
	}
	pthread_mutex_unlock(&g_rrm_queue_lock);
}

/* delete the 802.11k reports from the doubly list */
void rrm_report_delete_node(evt_data_queue_t *node_to_be_del)
{
	/* Base check */
	if ((g_start == NULL) || (node_to_be_del == NULL))
		return;

	/* If head == node_to_be_del */
	if (g_start == node_to_be_del)
		g_start = node_to_be_del->next;

	/* Update next only if node to be deleted is NOT the last node */
	if (node_to_be_del->next != NULL)
		node_to_be_del->next->prev = node_to_be_del->prev;

	/* update prev only if node to be deleted is NOT the first node */
	if (node_to_be_del->prev != NULL)
		node_to_be_del->prev->next = node_to_be_del->next;

	/* Now free the node which is parsed */
	if (node_to_be_del->evt_data != NULL) {
		free(node_to_be_del->evt_data);
		node_to_be_del->evt_data = NULL;
	}
	free(node_to_be_del);
	node_to_be_del = NULL;
}

/* Gets the report from socket and adds it to queue */
void
rrm_get_report_main_loop(int sock)
{
	struct timeval tv = {1, 0};	/* timed out every second */
	fd_set fdset;
	int status, fdmax, bytes;
	char pkt[VISDCOLL_BUFSIZE_4K];

	FD_ZERO(&fdset);
	fdmax = -1;
	if (sock >= 0) {
		FD_SET(sock, &fdset);
		if (sock > fdmax)
			fdmax = sock;
	} else {
		VIS_ERROR("wrong socket .. \n");
		return;
	}

	status = select(fdmax+1, &fdset, NULL, NULL, &tv);
	if ((status > 0) && FD_ISSET(sock, &fdset)) {
		if ((bytes = recv(sock, pkt, VISDCOLL_BUFSIZE_4K, 0)) > IFNAMSIZ) {
			VIS_ADAPTER("recved raw data and pkt size = %d\n", sizeof(pkt));
			rrm_report_add_node(pkt, sizeof(pkt));
			rrm_signal_thread();
		}
	}
	return;
}

/* Thread handler to collect 802.11k report */
void*
rrm_collect_thread_handler(void *pnewsock)
{
	int sockfd = *(int*)pnewsock;

	/* Loop continuously till the thread status flag is set to be closed */
	while (1) {
		rrm_get_report_main_loop(sockfd);
		pthread_mutex_lock(&g_rrm_thread_lock);
		/* If to be closed */
		if (g_thread_status == VIS_THREAD_STATUS_TOBE_CLSOED) {
			g_thread_status = VIS_THREAD_STATUS_CLOSED;
			pthread_mutex_unlock(&g_rrm_thread_lock);
			VIS_DEBUG("RRM Thread is closing\n");
			goto exit;
		}
		pthread_mutex_unlock(&g_rrm_thread_lock);
	}

exit:
	close_socket(&sockfd);
	free(pnewsock);

	return 0;
}

/* Get the STA Info structure from driver */
int
vis_wl_get_sta_info(void *wl, vis_sta_stats_t *stastats, uint32 band)
{
	sta_info_t *sta;
	struct ether_addr ea;
	char *param;
	int buflen, err;
	int i, nss = 0, maxrate = 0;

	memset(g_vis_cmdoutbuf, 0, WLC_IOCTL_MEDLEN);
	strcpy(g_vis_cmdoutbuf, "sta_info");

	/* convert the ea string into an ea struct */
	if ((strlen(stastats->mac) <= 0) || !wl_ether_atoe(stastats->mac, &ea)) {
		VIS_ADAPTER(" ERROR: no valid ether addr provided\n");
		return -1;
	}

	buflen = strlen(g_vis_cmdoutbuf) + 1;
	param = (char *)(g_vis_cmdoutbuf + buflen);
	memcpy(param, (char*)&ea, ETHER_ADDR_LEN);

	if ((err = wlu_get(wl, WLC_GET_VAR, g_vis_cmdoutbuf, WLC_IOCTL_MEDLEN)) < 0) {
		return err;
	}
	/* get the sta info */
	sta = (sta_info_t *)g_vis_cmdoutbuf;
	sta->ver = dtoh16(sta->ver);

	/* Report unrecognized version */
	if (sta->ver > WL_STA_VER) {
		VIS_ADAPTER(" ERROR: unknown driver station info version %d\n", sta->ver);
		return -1;
	}

	sta->len = dtoh16(sta->len);
	sta->cap = dtoh16(sta->cap);
	sta->aid = dtoh16(sta->aid);
	sta->flags = dtoh32(sta->flags);
	stastats->idle = dtoh32(sta->idle);

	/* Check the generation of the sta */
	if (sta->flags & WL_STA_VHT_CAP) {
		stastats->generation_flag = NETWORK_TYPE_AC; /* ac */
	} else if (sta->flags & WL_STA_N_CAP) {
		stastats->generation_flag = NETWORK_TYPE_N; /* n */
	} else {
		/*
		 * check for b/g/a using below formula
		 * band = 5G && maxrate <= 54 => 'a' generation.
		 * band = 2.4G && maxrate <= 54 => 'g' generation.
		 * band = 2.4G && maxrate <= 11 => 'b' generation.
		 */
		maxrate = (sta->rateset.rates[sta->rateset.count-1] & 0x7F)/2;
		if (band == 5 && maxrate <= 54) {
			stastats->generation_flag = NETWORK_TYPE_A; /* a */
		} else if (maxrate <= 54) {
			stastats->generation_flag = NETWORK_TYPE_G; /* g */
		} else if (maxrate <= 11) {
			stastats->generation_flag = NETWORK_TYPE_B; /* b */
		}
	}
	/* Driver didn't return extended station info */
	if (sta->len < sizeof(sta_info_t)) {
		return 0;
	}

	/*
	 * Calculate MIMO capabilities
	 * first check the vht mcs and calculate the nss
	 * If vht mcs is not present than check mcs
	 * If both are not there the default will be 1X1.
	 */
	if (sta->ver >= 5) {
		for (i = 0; i < VHT_CAP_MCS_MAP_NSS_MAX; i++) {
			if (sta->rateset_adv.vht_mcs[i]) {
				nss = i + 1;
			} else {
				break;
			}
		}

		if (!nss) {
			for (i = 0; i < MCSSET_LEN; i++) {
				if ((sta->rateset_adv.mcs[i] & VIS_RRM_BYTE) == VIS_RRM_BYTE) {
					stastats->txstream++;
					stastats->rxstream++;
				}
			}
		} else {
			stastats->txstream = stastats->rxstream = nss;
		}

		if (!stastats->txstream) { /* both mcs and vht-mcs are not present */
			stastats->txstream = stastats->rxstream = 1;
		}
	}
	return (0);
}
/* Initializes 802.11k report generation process
 * Here it creates the thread to get the 802.11k reports and initializes the socket
 */
static int
initialize_rrm_report_processing()
{
	pthread_t thread;
	pthread_attr_t attr;
	int rc = 0;
	int *new_sock = NULL;
	int new_fd = INVALID_SOCKET;

	/* Loack the thread mutex to check the status of thread */
	pthread_mutex_lock(&g_rrm_thread_lock);
	/* If already running no need to create it again */
	if (g_thread_status == VIS_THREAD_STATUS_RUNNING) {
		pthread_mutex_unlock(&g_rrm_thread_lock);
		return 0;
	}
	pthread_mutex_unlock(&g_rrm_thread_lock);

	/* Connect to EAPD socket */
	new_fd = rrm_eapd_socket_init();
	if (new_fd == INVALID_SOCKET)
		return -1;

	new_sock = (int*)malloc(sizeof(int) * 1);
	if (new_sock == NULL) {
		VIS_ERROR("Failed to allocate memory for new_sock : %d\n", 1);
		goto exit;
	}

	*new_sock = new_fd;

	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		VIS_ERROR("pthread_attr_init");
		goto exit;
	}

	rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (rc != 0) {
		VIS_ERROR("pthread_attr_setdetachstate");
		pthread_attr_destroy(&attr);
		goto exit;
	}

	/* Create the thread to get the 802.11k report events from EAPD */
	if (pthread_create(&thread, &attr, rrm_collect_thread_handler, (void*)new_sock) != 0) {
		VIS_ERROR("Failed to create thread : %d\n", 1);
		pthread_attr_destroy(&attr);
		goto exit;
	}
	/* Update the thread status to running */
	pthread_mutex_lock(&g_rrm_thread_lock);
	g_thread_status = VIS_THREAD_STATUS_RUNNING;
	pthread_mutex_unlock(&g_rrm_thread_lock);

	pthread_attr_destroy(&attr);

	return 0;
exit:
	close_socket(&new_fd);
	if (new_sock != NULL)
		free(new_sock);

	return -1;
}

/* Gets the wl_sta_info for associated STA's */
vis_sta_statslist_t*
get_sta_stats_for_all_sta(void *wl, assoc_sta_list_t *stas_list, uint32 band)
{
	int i;
	int szalloclen = 0;
	vis_sta_statslist_t *stastatslist = NULL;

	if (!stas_list)
		return NULL;

	/* Allocate memory to hold report for all the associated stations */
	szalloclen = (sizeof(vis_sta_statslist_t) +
		(sizeof(vis_sta_stats_t) * (stas_list->length)));
	stastatslist = (vis_sta_statslist_t*)malloc(szalloclen);

	if (stastatslist == NULL) {
		VIS_METRICS("Failed to allocate stastatslist buffer of sz  %d\n", szalloclen);
		return NULL;
	}

	memset(stastatslist, 0x00, szalloclen);
	stastatslist->length = stas_list->length;
	stastatslist->timestamp = g_timestamp;

	/* Now get the STA stats for all the associated STA's */
	for (i = 0; i < stas_list->length; i++) {
		/* Copy the MAC address */
		memcpy(stastatslist->stastats[i].mac, stas_list->sta[i].mac, (ETHER_ADDR_LEN * 3));

		/* Get the wl sta_info */
		if (vis_wl_get_sta_info(wl, &stastatslist->stastats[i], band) != 0)
			VIS_DEBUG("Failed to get sta_info\n");
	}

	return stastatslist;
}

/* calls the 802.11k requests */
vis_rrm_statslist_t*
get_rrm_stats_for_all_sta(void *wl, assoc_sta_list_t *stas_list, char *bssid, uint32 ctrlch)
{
	int i, no_of_timeouts, err = 0;
	int szalloclen = 0;
	vis_rrm_statslist_t *rrmstatslist = NULL;
	statreq_t sreq_buf;
	rrmreq_t chreq_buf;

	if (!stas_list)
		return NULL;

	/* If there are any associated station's create thread to collect 802.11k report */
	if (stas_list->length > 0) {
		if (initialize_rrm_report_processing() != 0) {
			VIS_ADAPTER("Failed to process 802.11k data\n");
			return NULL;
		}
	}

	/* Allocate memory to hold report for all the associated stations */
	szalloclen = (sizeof(vis_rrm_statslist_t) +
		(sizeof(vis_rrm_stats_t) * (stas_list->length)));
	rrmstatslist = (vis_rrm_statslist_t*)malloc(szalloclen);
	if (rrmstatslist == NULL) {
		VIS_METRICS("Failed to allocate rrmstatslist buffer of sz  %d\n", szalloclen);
		return NULL;
	}
	memset(rrmstatslist, 0x00, szalloclen);
	rrmstatslist->length = stas_list->length;
	rrmstatslist->timestamp = g_timestamp;

	/*
	 * Now get the RRM stats for all the associated STA's
	 * If we don't get the data for three consucative requests
	 * that means sta is not providing rrm data so just break out of the loop
	 * And no need to send the channel load rpt req.
	 */
	for (i = 0; i < stas_list->length; i++) {
		uint8 grpid = 0;
		no_of_timeouts = 0;

		/* Copy the MAC address */
		memcpy(rrmstatslist->rrmstats[i].mac, stas_list->sta[i].mac, (ETHER_ADDR_LEN * 3));

		/* init sreq_buf and copy the sta and ap mac to sreq_buf */
		memset(&sreq_buf, 0, sizeof(statreq_t));
		if (!wl_ether_atoe(rrmstatslist->rrmstats[i].mac, &sreq_buf.da)) {
			continue;
		}
		if (!wl_ether_atoe(bssid, &sreq_buf.peer)) {
			continue;
		}
		sreq_buf.random_int  = 0;
		sreq_buf.dur = 0;

		/* Now send sta statistics request for 0 to 9 group ID's */
		for (grpid = 0; grpid <= 9; grpid++) {
			sreq_buf.group_id = grpid;
			if ((err = wlu_iovar_set(wl, "rrm_stat_req", &sreq_buf,
				sizeof(sreq_buf))) < 0) {
				break;
			}
			if (rrm_timed_wait() == ETIMEDOUT) {
				no_of_timeouts++;
			}
			if (no_of_timeouts > 1) {
				break;
			}
		}

		/* Issue Channel load 802.11k report request */
		if (no_of_timeouts <= 1) {
			memset(&chreq_buf, 0, sizeof(rrmreq_t));
			if (!wl_ether_atoe(rrmstatslist->rrmstats[i].mac, &chreq_buf.da)) {
				break;
			}
			chreq_buf.reg = RRM_ACTFRAME_OPERATING_CLASS;
			chreq_buf.dur = VIS_RRM_MIN_DURATION;
			chreq_buf.chan = ctrlch;
			chreq_buf.random_int = 0;
			if ((err = wlu_iovar_set(wl, "rrm_chload_req", &chreq_buf,
				sizeof(chreq_buf))) < 0) {
				break;
			}
		}
	}

	return rrmstatslist;
}

/* Checks whether any of the STA stats report is already read or not
 * If not read, set the bit flag to read
 */
static int
check_sta_stats_report_read(uint8 group_id, unsigned long *read_flag)
{
	int already_read = 1;

	switch (group_id) {
		case DOT11_RRM_STATS_GRP_ID_2:
			IS_REPORT_READ(*read_flag, RRM_BIT_STASTAT2);
			already_read = 0;
			(*read_flag) |= RRM_BIT_STASTAT2;
			break;
		case DOT11_RRM_STATS_GRP_ID_3:
			IS_REPORT_READ(*read_flag, RRM_BIT_STASTAT3);
			already_read = 0;
			(*read_flag) |= RRM_BIT_STASTAT3;
			break;
		case DOT11_RRM_STATS_GRP_ID_4:
			IS_REPORT_READ(*read_flag, RRM_BIT_STASTAT4);
			already_read = 0;
			(*read_flag) |= RRM_BIT_STASTAT4;
			break;
		case DOT11_RRM_STATS_GRP_ID_5:
			IS_REPORT_READ(*read_flag, RRM_BIT_STASTAT5);
			already_read = 0;
			(*read_flag) |= RRM_BIT_STASTAT5;
			break;
		case DOT11_RRM_STATS_GRP_ID_6:
			IS_REPORT_READ(*read_flag, RRM_BIT_STASTAT6);
			already_read = 0;
			(*read_flag) |= RRM_BIT_STASTAT6;
			break;
		case DOT11_RRM_STATS_GRP_ID_7:
			IS_REPORT_READ(*read_flag, RRM_BIT_STASTAT7);
			already_read = 0;
			(*read_flag) |= RRM_BIT_STASTAT7;
			break;
		case DOT11_RRM_STATS_GRP_ID_8:
			IS_REPORT_READ(*read_flag, RRM_BIT_STASTAT8);
			already_read = 0;
			(*read_flag) |= RRM_BIT_STASTAT8;
			break;
		case DOT11_RRM_STATS_GRP_ID_9:
			IS_REPORT_READ(*read_flag, RRM_BIT_STASTAT9);
			already_read = 0;
			(*read_flag) |= RRM_BIT_STASTAT9;
			break;
	}

	return already_read;
}

/* Parse channel load report from IE */
static void
parse_channel_load_report(dot11_rm_ie_t *ie, vis_rrm_stats_t *rrmstats)
{
	dot11_rmrep_chanload_t *rmrep_chload;

	/* Check whether the report is already read */
	if (rrmstats->read_flag & RRM_BIT_CHLOAD) {
		VIS_ADAPTER("Channel Load report already read\n");
		return;
	}
	/* If not read set the bit flag to read */
	rrmstats->read_flag |= RRM_BIT_CHLOAD;
	rmrep_chload = (dot11_rmrep_chanload_t *)&ie[1];
	rrmstats->txop = (((VIS_RRM_BYTE - rmrep_chload->channel_load) * 100)
		/ VIS_RRM_BYTE);
}

/* Parse STA statistics report from IE */
static void
parse_sta_stats_report(dot11_rm_ie_t *ie, vis_rrm_stats_t *rrmstats)
{
	dot11_rmrep_stat_t *rmrep_stat;

	rmrep_stat = (dot11_rmrep_stat_t *)&ie[1];
	switch (rmrep_stat->group_id) {
		case DOT11_RRM_STATS_GRP_ID_0:
			{
				/* Check whether the report is already read */
				if (rrmstats->read_flag & RRM_BIT_STASTAT0) {
					VIS_ADAPTER("STA stats0 report already read\n");
					return;
				}
				/* If not read set the bit flag to read */
				rrmstats->read_flag |= RRM_BIT_STASTAT0;
				rrm_stat_group_0_t *data0;
				data0 = (rrm_stat_group_0_t *)&rmrep_stat[1];
				rrmstats->pktrequested = data0->txframe;
				VIS_ADAPTER("rrmstats->pktrequested : %d\n",
					rrmstats->pktrequested);
			}
			break;
		case DOT11_RRM_STATS_GRP_ID_1:
			{
				/* Check whether the report is already read */
				if (rrmstats->read_flag & RRM_BIT_STASTAT1) {
					VIS_ADAPTER("STA stats1 report already read\n");
					return;
				}
				/* If not read set the bit flag to read */
				rrmstats->read_flag |= RRM_BIT_STASTAT1;
				rrm_stat_group_1_t *data1;
				data1 = (rrm_stat_group_1_t *)&rmrep_stat[1];
				rrmstats->pktretried = data1->txretry;
				rrmstats->pktacked = data1->ackfail;
				VIS_ADAPTER("rrmstats->pktretried : %d\n", rrmstats->pktretried);
			}
			break;
		case DOT11_RRM_STATS_GRP_ID_2:
		case DOT11_RRM_STATS_GRP_ID_3:
		case DOT11_RRM_STATS_GRP_ID_4:
		case DOT11_RRM_STATS_GRP_ID_5:
		case DOT11_RRM_STATS_GRP_ID_6:
		case DOT11_RRM_STATS_GRP_ID_7:
		case DOT11_RRM_STATS_GRP_ID_8:
		case DOT11_RRM_STATS_GRP_ID_9:
			{
				rrm_stat_group_qos_t *dataqos;
				dataqos = (rrm_stat_group_qos_t *)&rmrep_stat[1];
				if (check_sta_stats_report_read(rmrep_stat->group_id,
					&rrmstats->read_flag) == 1) {
					VIS_ADAPTER("STA stats group id %d already read\n",
						rmrep_stat->group_id);
					return;
				}
				/* Adding because we need txdrop from all the group ID's */
				rrmstats->pktdropped += dataqos->txdrop;
			}
			break;
		default:
			VIS_ADAPTER("STA stats Not needed for WiFi Insight. Group Id: %d\n",
				rmrep_stat->group_id);
			break;
	}
}

/* Parses the 802.11k report to get each 802.11k measure types */
static void
parse_each_measure_type(bcm_event_t *pvt_data, vis_rrm_stats_t *rrmstats)
{
	wl_rrm_event_t *evt;
	dot11_rm_ie_t *ie;

	evt = (wl_rrm_event_t *)(pvt_data + 1);
	ie = (dot11_rm_ie_t *)(evt->payload);
	switch (evt->subevent) {
		case DOT11_MEASURE_TYPE_CHLOAD:
			parse_channel_load_report(ie, rrmstats);
			break;
		case DOT11_MEASURE_TYPE_STAT:
			parse_sta_stats_report(ie, rrmstats);
			break;
		default:
			VIS_ADAPTER("Not needed for WiFi Insight : 0x%2X\n", evt->subevent);
			break;
	}
}

/* Get the 802.11k reports stored in the queue */
void
get_rrm_report_from_queue(vis_rrm_statslist_t *rrmstatslist)
{
	evt_data_queue_t *node, *nextnode;
	bcm_event_t *pvt_data;
	char tmp[32];
	int i;

	pthread_mutex_lock(&g_rrm_queue_lock);
	/* For each node in the queue */
	/* Now check for which MAC address the 802.11k report belongs */
	for (i = 0; i < rrmstatslist->length; i++) {
		node = g_start;
		while (node != NULL) {
			pvt_data = (bcm_event_t *)(node->evt_data + IFNAMSIZ);
			ether_etoa(pvt_data->event.addr.octet, tmp);
			if (strncmp(rrmstatslist->rrmstats[i].mac,
				tmp, sizeof(rrmstatslist->rrmstats[i].mac)) == 0) {
					parse_each_measure_type(pvt_data,
						&rrmstatslist->rrmstats[i]);
					nextnode = node->next;
					rrm_report_delete_node(node);
			}
			node = node ? node->next : nextnode;
		}
		/* Calculate the pktstored */
		/* Debug message */
		VIS_RRM("\nMAC : %s\nTXOP : %d\nPacket Requested : %d\nPacket Dropped : %d\n"
			"Packet Stored : %d\nPacket Retried : %d\nPacket Acked : %d\n",
			rrmstatslist->rrmstats[i].mac, rrmstatslist->rrmstats[i].txop,
			rrmstatslist->rrmstats[i].pktrequested,
			rrmstatslist->rrmstats[i].pktdropped,
			rrmstatslist->rrmstats[i].pktstored,
			rrmstatslist->rrmstats[i].pktretried,
			rrmstatslist->rrmstats[i].pktacked);
	}
	pthread_mutex_unlock(&g_rrm_queue_lock);
}
