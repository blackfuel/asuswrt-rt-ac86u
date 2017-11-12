#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <shared.h>
#include <rc.h>
#include <wlioctl.h>
#include <bcmendian.h>
#include <wlutils.h>
#include <roamast.h>

#ifdef RTCONFIG_AMAS
#include <amas_path.h>
#endif

#define STA_INFO_PATH "/tmp/ltq_rast_sta_list"
#define HOSTAPD_TO_FAPI_MSG_LENGTH              (4096 * 3)
#define HOSTAPD_TO_FAPI_VALUE_STRING_LENGTH     128

rast_sta_info_t *rast_add_to_assoclist(int bssidx, int vifidx, struct ether_addr *addr);

void get_stainfo(int bssidx, int vifidx)
{
	FILE *fp;
	char *ifname;
	char wif_buf[32];
	char line_buf[300];
	char addr[18];
	char rssi[5];
	char txbytes[32];
	char rxbytes[32];
	unsigned long totalbytes;
	rast_sta_info_t *sta = NULL;
	time_t now = uptime();

	if(vifidx == 0)
		ifname = strdup(get_wififname(bssidx));
	else
		ifname = wl_vifname_wave(bssidx, vifidx);

	if (!ifname)
		return;

	doSystem("iw dev %s station dump > %s", ifname, STA_INFO_PATH);
	fp = fopen(STA_INFO_PATH, "r");
	if (fp) {
		while ( fgets(line_buf, sizeof(line_buf), fp) ) {
			if(strstr(line_buf, "Station")) {
				sscanf(line_buf, "%*s%s", addr);
				while ( fgets(line_buf, sizeof(line_buf), fp) ) {
					if(strstr(line_buf, "rx bytes"))
						sscanf(line_buf, "%*s%*s%s", rxbytes);
					else if(strstr(line_buf, "tx bytes"))
						sscanf(line_buf, "%*s%*s%s", txbytes);
					else if(strstr(line_buf, "signal")) {
						sscanf(line_buf, "%*s%s", rssi);
						break;
					}
				}

				totalbytes = atoi(txbytes) + atoi(rxbytes);
				sta = rast_add_to_assoclist(bssidx, vifidx, ether_aton(addr));
				sta->rssi = atoi(rssi);
				if((now - sta->active) && (totalbytes > sta->last_txrx_bytes))
					sta->datarate = ((float)(totalbytes - sta->last_txrx_bytes)/1024) / (float)(now - sta->active);
				else
					sta->datarate = 0;
				sta->last_txrx_bytes = totalbytes;
				sta->active = now;
			}
		}

		fclose(fp);
		unlink(STA_INFO_PATH);
	}

	free(ifname);
	return;
}


#ifdef RTCONFIG_ADV_RAST
#define true 1
#define false 0
static bool fieldValuesGet(char *buf, char *stringOfValues, const char *stringToSearch, char *endFieldName[])
{  /* handles list of fields, one by one in the same row */
        char *stringStart;
        char *stringFraction;
        char *localBuf = NULL;
        char *localStringToSearch = NULL;
        int  i;

        localBuf = (char *)malloc((size_t)(strlen(buf) + 1));
        if (localBuf == NULL)
        {
                printf("%s; malloc failed ==> ABORT!\n", __FUNCTION__);
                return false;
        }

        /* Add ' ' at the beginning of a string - to handle a case in which the buf starts with the
           value of stringToSearch, like buf= 'candidate=d8:fe:e3:3e:bd:14,2178,83,5,7,255 candidate=...' */
        sprintf(localBuf, " %s", buf);

        /* localStringToSearch set to stringToSearch with addition of " " at the beginning -
           it is a MUST in order to differentiate between "ssid" and "bssid" */
        localStringToSearch = (char *)malloc(strlen(stringToSearch) + 1);
        if(localStringToSearch == NULL)
        {
                printf("%s; localStringToSearch is NULL ==> Abort!\n", __FUNCTION__);
                free((void *)localBuf);
                return false;
        }

        sprintf(localStringToSearch, " %s", stringToSearch);
        stringStart = strstr(localBuf, localStringToSearch);
        if (stringStart == NULL)
        {
                //printf("%s; stringToSearch ('%s') was not found!\n", __FUNCTION__, stringToSearch);
                free((void *)localBuf);
                free((void *)localStringToSearch);
                return false;
        }

        /* Get the first value of the field */
        stringFraction = strtok(stringStart, " ");
        if (stringFraction == NULL)
        {
                //printf("%s; stringFraction ('%s') is NULL! ABORT!\n", __FUNCTION__, stringFraction);
                free((void *)localBuf);
                free((void *)localStringToSearch);
                return false;
        }

        stringFraction += strlen(stringToSearch);
        strcpy(stringOfValues, stringFraction);

        while (1)
        {
                stringFraction = strtok(NULL, " ");

                if (stringFraction == NULL)
                {  /* end of string reached ==> finish */
                        free((void *)localBuf);
                        free((void *)localStringToSearch);
                        return true;
                }

                i = 0;
                while (strcmp(endFieldName[i], "\n"))
                {  /* run over all field names in the string */
                        if (!strncmp(stringFraction, endFieldName[i], strlen(endFieldName[i])))
                        {  /* field name reached ==> finish */
                                free((void *)localBuf);
                                free((void *)localStringToSearch);
                                return true;
                        }

                        i++;
                }

                sprintf(stringOfValues, "%s %s", stringOfValues, stringFraction);  /* add the following value to the list */
        }

        free((void *)localBuf);
        free((void *)localStringToSearch);

        return true;
}

/* return 0 means get nothing */
static int unconnected_sta_rssi_parse_test(char *buf)
{
	char    *opCode;
	char    *VAPName;
	char    *MACAddress;
	char    *rx_bytes;
	char    *rx_packets;
	char    stringOfValues[HOSTAPD_TO_FAPI_VALUE_STRING_LENGTH];
	char    *completeBuf = strdup(buf);
	char    *endFieldName[] =
	{ "rssi",
	"\n" };

	int rssi_ret=0,rssi_1=0,rssi_2=0,rssi_3=0,rssi_4=0;

	if (completeBuf == NULL) {
		printf("%s; strdup() failed ==> ABORT!\n", __FUNCTION__);
		return rssi_ret;
	}

	opCode = strtok(buf, " ");
	if (strstr(opCode, "UNCONNECTED-STA-RSSI") == NULL) {
		printf("%s; wrong opCode ('%s') ==> Abort!\n", __FUNCTION__, opCode);
		free((void *)completeBuf);
		return rssi_ret;
	}

	VAPName = strtok(NULL, " ");
	if (strncmp(VAPName, "wlan", 4)) {
		printf("%s; VAP Name ('%s') is NOT supported ==> Abort!\n", __FUNCTION__, VAPName);
		free((void *)completeBuf);
		return rssi_ret;
	}

	MACAddress = strtok(NULL, " ");
	rx_bytes   = strtok(NULL, " ") + strlen("rx_bytes=");
	rx_packets = strtok(NULL, " ") + strlen("rx_packets=");

	if (fieldValuesGet(completeBuf, stringOfValues, "rssi=", endFieldName)) {
		if(  (strncmp("-128 -128 -128 -128",stringOfValues,19) == 0) ) {
			//printf("...\n");
			//retry_times ++;
			free((void *)completeBuf);
			return rssi_ret;
		} else {
			sscanf(stringOfValues,"%d %d %d %d",&rssi_1,&rssi_2,&rssi_3,&rssi_4);
			//printf("%5d %5d %5d %5d\n",rssi_1,rssi_2,rssi_3,rssi_4);
			rssi_ret = rssi_1;
			rssi_ret = rssi_ret > rssi_2 ? rssi_ret : rssi_2;
			rssi_ret = rssi_ret > rssi_3 ? rssi_ret : rssi_3;
			rssi_ret = rssi_ret > rssi_4 ? rssi_ret : rssi_4;
			//retry_times ++;
			free((void *)completeBuf);
			return rssi_ret;
		}
	}

	//printf("%s\n",stringOfValues);
	free((void *)completeBuf);
	return rssi_ret;
}

/* return value : rssi value; 0 means error or gets nothing */
int report_check(const char *iface,struct wpa_ctrl *wpaCtrlPtr)
{
	char    *buf;
	size_t  len = HOSTAPD_TO_FAPI_MSG_LENGTH * sizeof(char);
	int     rssi_ret=0;

	if (wpaCtrlPtr == NULL) {
		return rssi_ret;
	}

	buf = (char *)malloc((size_t)(HOSTAPD_TO_FAPI_MSG_LENGTH * sizeof(char)));
	if (buf == NULL) {
		printf("%s; malloc error ==> ABORT!\n", __FUNCTION__);
		return rssi_ret;
	}

	memset(buf, 0, HOSTAPD_TO_FAPI_MSG_LENGTH * sizeof(char));  /* Clear the output buffer */
	if (wpa_ctrl_recv(wpaCtrlPtr, buf, &len) == 0) {
	//printf("%s; len= %d\nbuf= '%s'\n", __FUNCTION__, len, buf);
		if (len <= 5) {
			printf("%s; '%s' is NOT a report - continue!\n", __FUNCTION__, buf);
			free((void *)buf);
			return rssi_ret;
		}

		rssi_ret = unconnected_sta_rssi_parse_test( buf);
		free((void *)buf);
		return rssi_ret;
	} else {
		printf("%s; wpa_ctrl_recv() returned ERROR\n", __FUNCTION__);
		free((void *)buf);
		return rssi_ret;
	}

	free((void *)buf);

	return rssi_ret;
}
int get_channel(char *wifname,int *width,int *center_freq1,int *center_freq)
{
	FILE *fp;
	char line_buf[300];
	int ret_conut=0;

	doSystem("cat /proc/net/mtlk/%s/channel > %s", wifname, STA_INFO_PATH);
	fp = fopen(STA_INFO_PATH, "r");
	if (fp) {
		while ( fgets(line_buf, sizeof(line_buf), fp) ) {
			if(strstr(line_buf, "width")) {
				sscanf(line_buf, "%*s%d", width);
				ret_conut++;
			} else if(strstr(line_buf, "center_freq1")) {
				sscanf(line_buf, "%*s%d", center_freq1);
				ret_conut++;
			} else if(strstr(line_buf, "center_freq"))  {
				sscanf(line_buf, "%*s%d", center_freq);
				ret_conut++;
			}
		}

		fclose(fp);
		unlink(STA_INFO_PATH);
	}

	if(ret_conut == 3)
		return 0;
	return -1;
}

int rast_stamon_get_rssi(int bssidx, int vifidx, struct ether_addr *addr)
{
	char *ifname=NULL;
	//char wif_buf[32]={0};
	char macaddr_str[32]={0};
	int width=0,center_freq1=0,center_freq=0;
	struct wpa_ctrl *wpaCtrlPtr=NULL;
	int  fd,res,res_wpa_ctrl;
	fd_set rfds;
	struct timeval timeout;
	int notfirsttimeout=0;
	size_t len;
	int retry_times=0;
	char localBuf[HOSTAPD_TO_FAPI_MSG_LENGTH]={0};

	int rssi_sum=0;
	int rssi_cnt=0;

	char command[] = "UNCONNECTED_STA_RSSI xx:xx:xx:xx:xx:xx xxxx center_freq1=xxxx bandwidth=xx";

	/* interface name */
	//todo: wlan0.0
	//if(vifidx == 0)
	if(vifidx == 0)
		ifname = strdup(get_wififname(bssidx));
	else
		ifname = wl_vifname_wave(bssidx, vifidx);
	if (!ifname) 	goto rast_stamon_get_rssi_return;
	/* MAC address */
	sprintf(macaddr_str,MACF,ETHERP_TO_MACF(addr));
	/* freq & bandwidth */
	if( get_channel(ifname,&width,&center_freq1,&center_freq) )
		goto rast_stamon_get_rssi_return;

	sprintf(command,"UNCONNECTED_STA_RSSI %s %d center_freq1=%d bandwidth=%d",macaddr_str,center_freq,center_freq1,width);

printf("command %s\n",command);

	if(bssidx == 1)
		wpaCtrlPtr = wpa_ctrl_open("/var/run/hostapd/wlan2");
	else
		wpaCtrlPtr = wpa_ctrl_open("/var/run/hostapd/wlan0");

	if (wpaCtrlPtr == NULL) {
		printf("%s; ERROR: hostapd_socket_get on band '%d' failed!\n", __FUNCTION__, bssidx);
		goto rast_stamon_get_rssi_return;
	} else if (wpa_ctrl_attach(wpaCtrlPtr) != 0) {
		printf("%s; ERROR: wpa_ctrl_attach for band '%d' failed!\n", __FUNCTION__, bssidx);
		goto rast_stamon_get_rssi_return;
	} else {
		fd = wpa_ctrl_get_fd(wpaCtrlPtr);
	}
    /* Main event loop */

    timeout.tv_sec = 0;
    timeout.tv_usec= 0;

    while (1)
    {
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		//printf("%d\n",__LINE__);
		res = select(fd + 1, &rfds, NULL, NULL, &timeout);
		if (res < 0)
		{
			printf("%s; select() return value= %d ==> CONTINUE!!!\n", __FUNCTION__, res);
			continue;
		} else if( res == 0 ) {
			if(!notfirsttimeout)
			{
				notfirsttimeout = 1;
				timeout.tv_sec = 1;
				timeout.tv_usec= 0;
				len=HOSTAPD_TO_FAPI_MSG_LENGTH;
				memset(localBuf,0,(4096 * 3) );                             
				wpa_ctrl_request(wpaCtrlPtr, command, strlen(command), localBuf, &len, NULL);                     
			} else {
				timeout.tv_sec = 1;
				timeout.tv_usec= 0;                             
			}
			continue;
		}
		if (FD_ISSET(fd, &rfds)) {
			while (1) {
				res_wpa_ctrl = wpa_ctrl_pending(wpaCtrlPtr);
				if (res_wpa_ctrl != 1)
					break;  /* quit the 'while' loop */

				if ( retry_times > 50 )
					break;

				res = report_check(ifname,wpaCtrlPtr);
				if ( res == 0 ) {
					len=HOSTAPD_TO_FAPI_MSG_LENGTH;
					memset(localBuf,0,(4096 * 3) );     
					wpa_ctrl_request(wpaCtrlPtr, command, strlen(command), localBuf, &len, NULL);
					break;
				} else {

					rssi_sum += res;
					rssi_cnt ++;

					len=HOSTAPD_TO_FAPI_MSG_LENGTH;
					memset(localBuf,0,(4096 * 3) );     
					wpa_ctrl_request(wpaCtrlPtr, command, strlen(command), localBuf, &len, NULL);
					break;
				}
				retry_times ++;
				usleep(50000);
			}

			if ( res_wpa_ctrl == (-1) ) {  
			/* ERROR - issue a trace */
				printf("wpa_ctrl_pending() returned ERROR\n");
			}
		}
		if ( retry_times > 50 )
			break;
	}

	if (wpaCtrlPtr != NULL)
		wpa_ctrl_detach(wpaCtrlPtr);

	wpa_ctrl_close(wpaCtrlPtr);
	wpaCtrlPtr = NULL;

rast_stamon_get_rssi_return:

	if(ifname)
		free(ifname);

	if(rssi_cnt>0)
	{
		printf("%d\n",rssi_sum/rssi_cnt);
		return rssi_sum/rssi_cnt;
	}

	return 0;
}

void rast_mac_deny_list_add_and_disassciate(int bssidx, int vifidx, struct ether_addr *addr)
{
	char command_1[] = "DENY_MAC XX:XX:XX:XX:XX:XX 0";
	char command_2[] = "DISASSOCIATE wlanX XX:XX:XX:XX:XX:XX";
	char mac[]= "XX:XX:XX:XX:XX:XX";
	int len=HOSTAPD_TO_FAPI_MSG_LENGTH;
	struct wpa_ctrl *wpaSocket=NULL;
	char localBuf[HOSTAPD_TO_FAPI_MSG_LENGTH]={0};

	/* check mac address */
	//if(strlen(mac) != strlen("XX:XX:XX:XX:XX:XX")) {
	//	printf("Parameter error\n");
	//	return;
	//}

	if(bssidx == 0) wpaSocket = wpa_ctrl_open("/var/run/hostapd/wlan0");
	else if(bssidx == 1) wpaSocket = wpa_ctrl_open("/var/run/hostapd/wlan2");
	else return;
	if (wpaSocket == NULL) {
		printf("%s; ERROR: fapi_wlan_hostapd_socket_get failed!\n", __FUNCTION__);
		return;
	}

	sprintf(mac,"%2x:%2x:%2x:%2x:%2x:%2x",	addr->ether_addr_octet[0],
										  	addr->ether_addr_octet[1],
											addr->ether_addr_octet[2],
											addr->ether_addr_octet[3],
											addr->ether_addr_octet[4],
											addr->ether_addr_octet[5]);

	sprintf(command_1,"DENY_MAC %s 0",mac);

	if(bssidx == 0) sprintf(command_2,"DISASSOCIATE wlan0 %s",mac);
	else if(bssidx == 1) sprintf(command_2,"DISASSOCIATE wlan1 %s",mac);
	else {
		wpa_ctrl_close(wpaSocket);
		return;
	}

	printf("%s\n",command_1);
	printf("%s\n",command_2);

	wpa_ctrl_request(wpaSocket, command_1, strlen(command_1), localBuf, &len, NULL);
	wpa_ctrl_request(wpaSocket, command_2, strlen(command_2), localBuf, &len, NULL);

	wpa_ctrl_close(wpaSocket);

}
void rast_mac_deny_list_remove(int bssidx, int vifidx, struct ether_addr *addr)
{
	char command[] = "DENY_MAC XX:XX:XX:XX:XX:XX 1";
	int len=HOSTAPD_TO_FAPI_MSG_LENGTH;
	struct wpa_ctrl *wpaSocket=NULL;
	char localBuf[HOSTAPD_TO_FAPI_MSG_LENGTH]={0};
	char mac[]= "XX:XX:XX:XX:XX:XX";

	if(bssidx == 0) wpaSocket = wpa_ctrl_open("/var/run/hostapd/wlan0");
	else if(bssidx == 1) wpaSocket = wpa_ctrl_open("/var/run/hostapd/wlan2");
	else return;
	if (wpaSocket == NULL){
		printf("%s; ERROR: fapi_wlan_hostapd_socket_get failed!\n", __FUNCTION__);
		return;
	}

	sprintf(mac,"%2x:%2x:%2x:%2x:%2x:%2x",	addr->ether_addr_octet[0],
										  	addr->ether_addr_octet[1],
											addr->ether_addr_octet[2],
											addr->ether_addr_octet[3],
											addr->ether_addr_octet[4],
											addr->ether_addr_octet[5]);

	sprintf(command,"DENY_MAC %s 0",mac);

	wpa_ctrl_request(wpaSocket, command, strlen(command), localBuf, &len, NULL);

	wpa_ctrl_close(wpaSocket);	
}

void rast_mac_deny_list_clean(int bssidx, int vifidx)
{
	char command[] = "STA_ALLOW";
	int len=HOSTAPD_TO_FAPI_MSG_LENGTH;
	struct wpa_ctrl *wpaSocket=NULL;
	char localBuf[HOSTAPD_TO_FAPI_MSG_LENGTH]={0};

	if(bssidx == 0) wpaSocket = wpa_ctrl_open("/var/run/hostapd/wlan0");
	else if(bssidx == 1) wpaSocket = wpa_ctrl_open("/var/run/hostapd/wlan2");
	else return;
	if (wpaSocket == NULL){
		printf("%s; ERROR: fapi_wlan_hostapd_socket_get failed!\n", __FUNCTION__);
		return;
	}

	wpa_ctrl_request(wpaSocket, command, strlen(command), localBuf, &len, NULL);

	wpa_ctrl_close(wpaSocket);	
}

rast_maclist_t *r_maclist_table[MAX_IF_NUM];

void rast_retrieve_static_maclist(int bssidx, int vifidx)
{
	//init
	//allow all
	//if there are other process or functions use this ACL, need to check
	r_maclist_table[bssidx] = NULL;
	rast_mac_deny_list_clean(bssidx,vifidx);
}

void rast_set_maclist(int bssidx, int vifidx)
{
	rast_maclist_t *r_maclist = bssinfo[bssidx].maclist[vifidx];
	rast_maclist_t *maclist_tmp = NULL;
	rast_maclist_t *maclist_old = NULL,*maclist_old_pre = NULL,*maclist_old_tmp = NULL;
	rast_maclist_t *maclist_new = NULL;
	//struct maclist_r * *t_maclist_shared = NULL;
	int match=0;
	
	maclist_old=r_maclist_table[bssidx];

	//rast_mac_deny_list_clean(bssidx,vifidx);
	for(maclist_tmp = r_maclist; maclist_tmp != NULL ; maclist_tmp = maclist_tmp->next){
		/* compare with old list, if match, do nothing */
		for( maclist_old_tmp=maclist_old ; maclist_old_tmp!=NULL ; maclist_old_pre=maclist_old_tmp,maclist_old_tmp=maclist_old_tmp->next )
		{
			if( !memcmp(&maclist_tmp->addr.ether_addr_octet[0],&maclist_old->addr.ether_addr_octet[0],sizeof(maclist_old->addr)) )
			{
				match=1;
				if(maclist_new == NULL) {
					// add the mathc entry to maclist_new and remove form old list
					maclist_new = maclist_old_tmp;
					if(maclist_old_pre)
						maclist_old_pre->next = maclist_old_tmp->next;
					else
						maclist_old = maclist_old->next;
				} else {
					maclist_new->next = maclist_old_tmp;
					maclist_old_pre->next = maclist_old_tmp->next;
					maclist_new = maclist_old_tmp;
				}
			}
		}

		if(match == 0) {
			if(maclist_new){
				maclist_new->next = calloc(1,sizeof(rast_maclist_t));
				maclist_new = maclist_new->next;
			}
			else
				maclist_new = calloc(1,sizeof(rast_maclist_t));

			memcpy(&maclist_new->addr,&maclist_tmp->addr,sizeof(maclist_tmp->addr));
			rast_mac_deny_list_add_and_disassciate(bssidx,vifidx,&maclist_tmp->addr);
		}
	}

	for(;;)
	{
		if(!maclist_old)
			break;

		rast_mac_deny_list_remove(bssidx,vifidx,&maclist_old->addr);

		maclist_old_tmp = maclist_old->next;
		free(maclist_old);
		maclist_old =maclist_old_tmp;
	}

	r_maclist_table[bssidx] = maclist_new;

/*
httpd_handle_request:[Advanced_Wireless_Content.asp] from wireless
httpd_handle_request:[Advanced_ACL_Content.asp] for mac filter
update_variables: [apply_new] [restart_wireless] [3]
value preferred_lang=TW
value wl_unit=0
value wl_subunit=-1
value wl_macmode=allow
value wl_maclist_x=<12:12:12:12:12:13<0C:D7:46:26:04:54
set wl0_maclist_x=<12:12:12:12:12:13<0C:D7:46:26:04:54
1051: check_action 0
1051: set_action 7
1051: set_action 0
*/


}
#if 1
void set_wl_macfilter(int bssidx, int vifidx, struct maclist_r *client_list, int block)
{
	int i=0;

	for( i=0;i < client_list->count;i++ )
	{
		if(block) rast_mac_deny_list_add_and_disassciate(bssidx,vifidx,&(client_list->ea[i]));
		else rast_mac_deny_list_remove(bssidx,vifidx,&(client_list->ea[i]));
	}

}
#endif
#if 0
uint8 rast_get_rclass(int bssidx, int vifidx)
{

}

uint8 rast_get_channel(int bssidx, int vifidx)
{

}

int rast_send_bsstrans_req(int bssidx, int vifidx, struct ether_addr *sta_addr, struct ether_addr *nbr_bssid)
{

}
#endif
#endif