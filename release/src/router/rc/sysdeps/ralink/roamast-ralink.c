#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <shared.h>
#include <rc.h>
#include <bcmnvram.h>
#include <ralink.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include  <float.h>
#include <ap_priv.h>
#include <wlutils.h>
#include <roamast.h>

#define RAST_INFO(fmt, arg...) \
        do {    \
                _dprintf("RAST %lu: "fmt, uptime(), ##arg); \
        } while (0)
#define RAST_DBG(fmt, arg...) \
        do {    \
               if(rast_dbg) \
                _dprintf("RAST %lu: "fmt, uptime(), ##arg); \
        } while (0)

int xTxR = 0;

typedef struct _roaming_info {
       	char mac[19];
       	char rssi_xR[xR_MAX][7];
		char curRate[33];
       	char dump;
} roam;

typedef struct _roam_sta {
       	roam sta[128];
} roam_sta;


typedef struct _acl_maclist {
       	char mac[19];
} acl_maclist;

typedef struct _acl_sta {
       	acl_maclist list[128];
} acl_sta;

#define MAX_NUMBER_OF_ACL				64

void get_stainfo(int bssidx, int vifidx)
{
	char *sp = NULL, *op = NULL;
	char wlif_name[32] = {0}, header[128] = {0}, data[2048] = {0};
	int hdrLen = 0, staCount = 0, getLen = 0;
	struct iwreq wrq;
	roam_sta *ssap = NULL;
	struct rast_sta_info *staInfo = NULL;
	char prefix[] = "wlXXXXXXXXXX_";
	char header_t[128] = {0};
	int stream = 0;
	char tmp[128] = {0};
	char rssinum[16] = {0};
	unsigned long long cur_txrx_bytes = 0;
	int i = 0;
	int32 rssi_xR[xR_MAX] = {0};
	int rssi_total = 0;

	snprintf(prefix, sizeof(prefix), "wl%d_", bssidx);
	if (!(xTxR = nvram_get_int(strcat_r(prefix, "HT_RxStream", tmp))))
		return;

	if(xTxR > xR_MAX)
		xTxR = xR_MAX;


	if (vifidx > 0) {
		sprintf(data, "wl%d.%d_ifname", bssidx, vifidx);
		strcpy(wlif_name, nvram_safe_get(data));
	}
	else
		strcpy(wlif_name, bssinfo[bssidx].wlif_name);

		memset(data, 0x00, sizeof(data));
       	wrq.u.data.length = sizeof(data);
       	wrq.u.data.pointer = (caddr_t) data;
       	wrq.u.data.flags = ASUS_SUBCMD_GROAM;

	if (wl_ioctl(wlif_name, RTPRIV_IOCTL_ASUSCMD, &wrq) < 0) {
		RAST_INFO("[%s]: WI[%s] Access to StaInfo failure\n", __FUNCTION__, wlif_name);
		return;
	}

	memset(header, 0, sizeof(header));
	memset(header_t, 0, sizeof(header_t));
	hdrLen = sprintf(header_t, "%-19s", "MAC");
	strcpy(header, header_t);

	for (stream = 0; stream < xTxR; stream++) {
		sprintf(rssinum, "RSSI%d", stream);
		memset(header_t, 0, sizeof(header_t));
		hdrLen += sprintf(header_t, "%-7s", rssinum);
		strncat(header, header_t, strlen(header_t));
    }
	hdrLen += sprintf(header_t, "%-33s", "CURRATE");
	strncat(header, header_t, strlen(header_t));
	strcat(header,"\n");
	hdrLen++;


	if (wrq.u.data.length > 0 && data[0] != 0) {

		getLen = strlen(wrq.u.data.pointer + hdrLen);

		ssap = (roam_sta *)(wrq.u.data.pointer + hdrLen);
		op = sp = wrq.u.data.pointer + hdrLen;
		while (*sp && ((getLen - (sp-op)) >= 0)) {
			ssap->sta[staCount].mac[18]='\0';
			for (stream = 0; stream < xTxR; stream++) {
				ssap->sta[staCount].rssi_xR[stream][6]='\0';
			}
			ssap->sta[staCount].curRate[32]='\0';
			sp += hdrLen;
			staCount++;
		}

#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_CONCURRENTREPEATER
	char word[256], *next;
	int unit = 0;
	int skip = 0;
	char *ifName = NULL;
	unsigned char pap_bssid[2][18];	
	struct iwreq wrq1;
	if(sw_mode() == SW_MODE_REPEATER) {	
		   foreach (word, nvram_safe_get("wl_ifnames"), next) {

		#if defined(RTCONFIG_RALINK_MT7620) || defined(RTCONFIG_RALINK_MT7621)
				if(unit == 0)
		#else
				if(unit == 1)
		#endif
					ifName = APCLI_2G;
				else
					ifName = APCLI_5G;

				if(wl_ioctl(ifName, SIOCGIWAP, &wrq1)>=0); {
					       	wrq1.u.ap_addr.sa_family = ARPHRD_ETHER;
							sprintf(pap_bssid[unit], "%02X:%02X:%02X:%02X:%02X:%02X",
							       	(unsigned char)wrq1.u.ap_addr.sa_data[0], (unsigned char)wrq1.u.ap_addr.sa_data[1],
							       	(unsigned char)wrq1.u.ap_addr.sa_data[2], (unsigned char)wrq1.u.ap_addr.sa_data[3],
							       	(unsigned char)wrq1.u.ap_addr.sa_data[4], (unsigned char)wrq1.u.ap_addr.sa_data[5]  );
				}
				unit++;
			}	
	}
#else		
		char *aif;
		unsigned char pap_bssid[18];
		struct iwreq wrq1;
		if(sw_mode() == SW_MODE_REPEATER && nvram_get_int("wlc_band") == bssidx) {
			memset(header, 0, sizeof(header));
		       	aif = nvram_get( strcat_r(bssinfo[bssidx].prefix, "vifs", header) );
		       	if(wl_ioctl(aif, SIOCGIWAP, &wrq1)>=0); {
			       	wrq1.u.ap_addr.sa_family = ARPHRD_ETHER;
			       	sprintf(pap_bssid, "%02X:%02X:%02X:%02X:%02X:%02X",
					       	(unsigned char)wrq1.u.ap_addr.sa_data[0], (unsigned char)wrq1.u.ap_addr.sa_data[1],
					       	(unsigned char)wrq1.u.ap_addr.sa_data[2], (unsigned char)wrq1.u.ap_addr.sa_data[3],
					       	(unsigned char)wrq1.u.ap_addr.sa_data[4], (unsigned char)wrq1.u.ap_addr.sa_data[5]  );
		       	}
	       	}
#endif	       	
#endif
	       	
       	

		if( !staCount ) return;
		// add to assoclist //
		for(hdrLen=0; hdrLen< staCount; hdrLen++) {
#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_CONCURRENTREPEATER
			if(sw_mode() == SW_MODE_REPEATER) {	
				unit = 0;
				skip = 0;
				foreach (word, nvram_safe_get("wl_ifnames"), next) {
					// pap bssid,skip //
					if( !strncmp(pap_bssid[unit], ssap->sta[hdrLen].mac, strlen(pap_bssid[unit]))) {
						RAST_DBG("======SKIP======= [%s]: P-AP BSSID[(band:%d)%s]\n", __FUNCTION__, unit, pap_bssid[unit]);
						skip = 1;
						break;
					}
					unit++;
				}
			}	
			if (skip == 1)
				continue;		
#else			
			// pap bssid,skip //
			if( !strncmp(pap_bssid, ssap->sta[hdrLen].mac, sizeof(pap_bssid))) {
				RAST_DBG("[%s]: P-AP BSSID[%s]\n", __FUNCTION__, pap_bssid);
				continue;
			}
#endif			
#endif
		    staInfo = rast_add_to_assoclist(bssidx, vifidx, ether_aton(ssap->sta[hdrLen].mac));
		    rssi_total = 0;
			for( getLen=0; getLen<xTxR; getLen++ ) {
				rssi_xR[getLen] = !atoi(ssap->sta[hdrLen].rssi_xR[getLen]) ? -100 : atoi(ssap->sta[hdrLen].rssi_xR[getLen]);
				rssi_total = rssi_total + rssi_xR[getLen];
			}
			memcpy(staInfo->mac_addr, ssap->sta[hdrLen].mac, strlen(ssap->sta[hdrLen].mac));

		staInfo->rssi = rssi_total / xTxR ;
		cur_txrx_bytes = atoi(ssap->sta[hdrLen].curRate);
		staInfo->datarate = (float)((cur_txrx_bytes - staInfo->last_txrx_bytes) >> 7/* bytes to Kbits*/) / RAST_POLL_INTV_NORMAL/* Kbps */;
		staInfo->last_txrx_bytes = cur_txrx_bytes;
		staInfo->active = uptime();
#if 0
		if (rast_dbg) {
			RAST_DBG("[%s]: [%s][%d][%s] RATE[%f]\t", __FUNCTION__, wlif_name, hdrLen,
					       	staInfo->mac_addr, staInfo->datarate);
			for (i=0;i<xTxR;i++) {
				RAST_DBG("RSSI[%d:%d]\t", i, rssi_xR[i]);
			}
			RAST_DBG(" RSSI Avg = %d\n", staInfo->rssi );
		}
#endif
		}
	}
	return;
}


#ifdef RTCONFIG_ADV_RAST
int rast_stamon_get_rssi(int bssidx, struct ether_addr *addr)
{
	char *sp = NULL, *op = NULL;
	char wlif_name[32] = {0}, header[128] = {0}, data[2048] = {0};
	int hdrLen = 0, staCount = 0, getLen = 0;
	struct iwreq wrq;
	roam_sta *ssap = NULL;
	struct rast_sta_info *staInfo = NULL;
	char prefix[] = "wlXXXXXXXXXX_";
	char header_t[128] = {0};
	int stream = 0;
	char tmp[128] = {0};
	char rssinum[16] = {0};
	int i = 0, j = 0;
	int32 rssi_xR[xR_MAX] = {0};
	int rssi_total = 0;
	int sta_rssi = -100;
	char tr_mac[32] ={0};

	snprintf(prefix, sizeof(prefix), "wl%d_", bssidx);
	if (!(xTxR = nvram_get_int(strcat_r(prefix, "HT_RxStream", tmp))))
		return 0;

	if(xTxR > xR_MAX)
		xTxR = xR_MAX;
	
	for (i = 0; i<3; i++) {
	// enable sta_monitor feature
	doSystem("iwpriv %s set mnt_en=1", get_wifname(bssidx));

	// set sta monitor rule
	doSystem("iwpriv %s set mnt_rule=1:1:1", get_wifname(bssidx));

	// add sta into sta_monitor list
	sprintf(tr_mac, ""MACF"", ETHERP_TO_MACF(addr));
	doSystem("iwpriv %s set mnt_sta0=%s",  get_wifname(bssidx), tr_mac);

	//doSystem("iwpriv %s set mnt_show=1", get_wifname(bssidx));  //for test

		snprintf(wlif_name, sizeof(wlif_name), "%s", bssinfo[bssidx].wlif_name);

		memset(data, 0x00, sizeof(data));
       	wrq.u.data.length = sizeof(data);
       	wrq.u.data.pointer = (caddr_t) data;
       	wrq.u.data.flags = ASUS_SUBCMD_GMONITOR_RSSI;

    	
    		usleep(500000);
			if (wl_ioctl(wlif_name, RTPRIV_IOCTL_ASUSCMD, &wrq) < 0) {
				RAST_INFO("[%s]: WI[%s] Access to StaInfo failure\n", __FUNCTION__, wlif_name);
				goto done;
			}

			memset(header, 0, sizeof(header));
			memset(header_t, 0, sizeof(header_t));
			hdrLen = sprintf(header_t, "%-19s", "MAC");
			strcpy(header, header_t);

			for (stream = 0; stream < xTxR; stream++) {
				sprintf(rssinum, "RSSI%d", stream);
				memset(header_t, 0, sizeof(header_t));
				hdrLen += sprintf(header_t, "%-7s", rssinum);
				strncat(header, header_t, strlen(header_t));
		    }
			hdrLen += sprintf(header_t, "%-33s", "CURRATE");
			strncat(header, header_t, strlen(header_t));
			strcat(header,"\n");
			hdrLen++;
					
			if (wrq.u.data.length > 0 && data[0] != 0) {

				getLen = strlen(wrq.u.data.pointer + hdrLen);

				ssap = (roam_sta *)(wrq.u.data.pointer + hdrLen);
				op = sp = wrq.u.data.pointer + hdrLen;
				while (*sp && ((getLen - (sp-op)) >= 0)) {
					ssap->sta[staCount].mac[18]='\0';
					for (stream = 0; stream < xTxR; stream++) {
						ssap->sta[staCount].rssi_xR[stream][6]='\0';
					}
					ssap->sta[staCount].curRate[32]='\0';
					sp += hdrLen;
					staCount++;
				}

				if( !staCount ) goto done;
				
				//translate to capital.
				for(j = 0; j < sizeof(tr_mac); j++)
            		tr_mac[j] = toupper(tr_mac[j]);
            	
				// add to assoclist //
				for(hdrLen=0; hdrLen< staCount; hdrLen++) {
					//if (addr == ether_aton(ssap->sta[hdrLen].mac))
					if (strncmp(tr_mac, ssap->sta[hdrLen].mac, strlen(tr_mac)) == 0)
					{
						rssi_total = 0;

						for( getLen=0; getLen<xTxR; getLen++ ) {
							rssi_xR[getLen] = !atoi(ssap->sta[hdrLen].rssi_xR[getLen]) ? -100 : atoi(ssap->sta[hdrLen].rssi_xR[getLen]);
							rssi_total = rssi_total + rssi_xR[getLen];
						}
						RAST_DBG("count %d : RSSI Avg = %d\n",i, (rssi_total / xTxR));
						if ((sta_rssi < (rssi_total / xTxR))  && ((rssi_total / xTxR) < 0))
							sta_rssi = rssi_total / xTxR ;

						if (rast_dbg) {
							for (j=0;j<xTxR;j++) {
								RAST_DBG("RSSI[%d:%d]\t", j, rssi_xR[j]);
							}
							RAST_DBG(" RSSI Avg = %d\n",sta_rssi);
						}
						//goto done;
					}
				} // for(hdrLen=0
			} //if (wrq.u.data.length >
			doSystem("iwpriv %s set mnt_clr=1",  get_wifname(bssidx));
			doSystem("iwpriv %s set mnt_en=0",  get_wifname(bssidx));	
		} //for (i = 0; i<3; i++) {
done:			
	doSystem("iwpriv %s set mnt_clr=1",  get_wifname(bssidx));
	doSystem("iwpriv %s set mnt_en=0",  get_wifname(bssidx));
	RAST_DBG("#### return sta_rssi= %d  ######\n",sta_rssi);
	return sta_rssi;
}

void rast_retrieve_static_maclist(int bssidx, int vifidx)
{

	char *sp = NULL, *op = NULL;
	struct iwreq wrq;
	char data[2048] = {0}, header[128] = {0};
	unsigned long macmode = 0;
	int staCount = 0, i = 0, getLen = 0, hdrLen = 0, size = 0;
	acl_sta *ssap = NULL;
	struct maclist *maclist = (struct maclist *) maclist_buf;
	struct ether_addr *ea = NULL;
	
	wrq.u.data.length = sizeof(macmode);
	wrq.u.data.pointer = (caddr_t)&macmode;
	wrq.u.data.flags = ASUS_SUBCMD_MACMODE;


	
    if (wl_ioctl(get_wifname(bssidx), RTPRIV_IOCTL_ASUSCMD, &wrq) < 0) {
		RAST_INFO("[WARNING] %s get macmode error!!!\n", get_wifname(bssidx));
		return;
	}



	bssinfo[bssidx].static_macmode[vifidx] = macmode;
	RAST_DBG("[%s] macmode = %s\n",
		__FUNCTION__,
		bssinfo[bssidx].static_macmode[vifidx]==WLC_MACMODE_DISABLED ? "DISABLE" :
		bssinfo[bssidx].static_macmode[vifidx]==WLC_MACMODE_DENY ? "DENY" : "ALLOW");
	
	memset(data, 0x00, sizeof(data));
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t)data;
	wrq.u.data.flags = ASUS_SUBCMD_MACLIST;
	
	if (wl_ioctl(get_wifname(bssidx), RTPRIV_IOCTL_ASUSCMD, &wrq) < 0) {
		RAST_INFO("[%s]: WI[%s] Get ACL List failure\n", __FUNCTION__, get_wifname(bssidx));
		return;
	}
	
	
	memset(header, 0, sizeof(header));
	//hdrLen = sprintf(header, "%-7s%-19s\n", "COUNT", "MAC");
	hdrLen = sprintf(header, "%-19s","MAC");
	
	if (wrq.u.data.length > 0 && data[0] != 0) {

		getLen = strlen(wrq.u.data.pointer + hdrLen);

		ssap = (acl_sta *)(wrq.u.data.pointer + hdrLen);
		op = sp = wrq.u.data.pointer + hdrLen;
			
		while (*sp && ((getLen - (sp-op)) >= 0)) {
			ssap->list[staCount].mac[18]='\0';			
			RAST_DBG("ssap->list[staCount].mac= (%s)\n", ssap->list[staCount].mac);
			ea = &(maclist->ea[staCount]);
			memcpy(ea, ether_aton(ssap->list[staCount].mac), sizeof(struct ether_addr));
			sp += hdrLen;
			staCount++;
			ea++;
		}
		
		maclist->count = staCount;
#if 0		
		for (i=0; i< staCount;i++){					
		RAST_DBG("[%s] (%d)mac:"MACF"\n",__FUNCTION__, __LINE__, ETHER_TO_MACF(maclist->ea[i]));
		}		
#endif		
	}	
	
		if (maclist->count > 0 && maclist->count < 128) {
		size = sizeof(uint) + sizeof(struct ether_addr) * (maclist->count + 1);

		RAST_DBG("count[%d] size[%d]\n", maclist->count, size);

		bssinfo[bssidx].static_maclist[vifidx] = (struct maclist *)malloc(size);
		if (!(bssinfo[bssidx].static_maclist[vifidx])) {
			RAST_INFO("%s malloc [%d] failure... \n", __FUNCTION__, size);
			return;
		}
		memcpy(bssinfo[bssidx].static_maclist[vifidx], maclist, size);
		maclist = bssinfo[bssidx].static_maclist[vifidx];
		for (size = 0; size < maclist->count; size++) {
			RAST_DBG("[%s] (%d)mac:"MACF"\n",__FUNCTION__, size, ETHER_TO_MACF(maclist->ea[size]));
		}
	} else if (maclist->count != 0) {
		RAST_INFO("Err: %s maclist cnt [%d] too large\n",
		__FUNCTION__, maclist->count);
		return;
	}
	return;
}

void rast_set_maclist(int bssidx, int vifidx)
{
	rast_maclist_t *r_maclist = bssinfo[bssidx].maclist[vifidx];
	struct maclist *maclist = (struct maclist *)maclist_buf;
	struct maclist *static_maclist = bssinfo[bssidx].static_maclist[vifidx];
	int static_macmode = bssinfo[bssidx].static_macmode[vifidx];
	int ret, val;
	struct ether_addr *ea;
	int cnt, match;
	char ether_tmp[19] = {0};
	char list_entry[2048] = {0};
	
	if (static_macmode == WLC_MACMODE_DENY || static_macmode == WLC_MACMODE_DISABLED)
		val = WLC_MACMODE_DENY;
        else
		val = WLC_MACMODE_ALLOW;
	

	doSystem("iwpriv %s set AccessPolicy=%d", get_wifname(bssidx), val);

	memset(maclist_buf, 0, sizeof(maclist_buf));

	if (static_macmode == WLC_MACMODE_DENY || static_macmode == WLC_MACMODE_DISABLED) {
		if (static_maclist && static_macmode == WLC_MACMODE_DENY) {
			RAST_DBG("Deny mode: Adding static maclist\n");
			maclist->count = static_maclist->count;
			memcpy(maclist_buf, static_maclist,
				sizeof(uint) + ETHER_ADDR_LEN * (maclist->count));
		}

		ea = &(maclist->ea[maclist->count]);
		while (r_maclist) {
			memcpy(ea, &(r_maclist->addr), sizeof(struct ether_addr));
			maclist->count++;
			RAST_DBG("Deny mode: cnt[%d] mac:"MACF"\n",
					maclist->count, ETHERP_TO_MACF(ea));
			ea++;
			r_maclist = r_maclist->next;
                }
        }
	else {	//ALLOW MODE
		ea = &(maclist->ea[0]);

		if (!static_maclist) {
			RAST_INFO("[ERROR] %s macmode:%d static_list is NULL\n",
				__FUNCTION__, static_macmode);
			return;
		}

		for (cnt = 0; cnt < static_maclist->count; cnt++) {
			RAST_DBG("Allow mode: static mac[%d] addr:"MACF"\n", cnt,
				ETHER_TO_MACF(static_maclist->ea[cnt]));
			/* if mac in static maclist match rast maclist, skip it */
			match = 0;
			while(r_maclist) {
				RAST_DBG("Checking "MACF"\n", ETHER_TO_MACF(r_maclist->addr));
#if (defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA))
				if(memcmp(&(r_maclist->addr), &(static_maclist->ea[cnt]), ETHER_ADDR_STR_LEN/3) == 0)
#else				
				if (eacmp(&(r_maclist->addr), &(static_maclist->ea[cnt])) == 0) 
#endif					
				{
					RAST_DBG("MATCH maclist "MACF"\n", ETHER_TO_MACF(r_maclist->addr));
					match = 1;
					break;
				}
				r_maclist++;
			}
 			if (!match) {
				memcpy(ea, &(static_maclist->ea[cnt]), sizeof(struct ether_addr));
				maclist->count++;
				RAST_DBG("Adding to Allow list: cnt[%d] addr:"MACF"\n",
					maclist->count, ETHERP_TO_MACF(ea));
				ea++;
			}
                }
        }
	memset(list_entry, 0x00, sizeof(list_entry));		
	for (cnt = 0; cnt < maclist->count; cnt++) {
		if (cnt == 0){
				sprintf(list_entry, "%02x:%02x:%02x:%02x:%02x:%02x;", ETHER_TO_MACF(maclist->ea[cnt]));
		}
		else if (cnt < MAX_NUMBER_OF_ACL) {	
				memset(ether_tmp, 0x00, sizeof(ether_tmp));		
				sprintf(ether_tmp, "%02x:%02x:%02x:%02x:%02x:%02x;", ETHER_TO_MACF(maclist->ea[cnt]));
				strncat(list_entry, ether_tmp, sizeof(ether_tmp));
		}

	RAST_DBG("list_entry =[%s] \n", list_entry);

	RAST_DBG("maclist: "MACF"\n",
			ETHER_TO_MACF(maclist->ea[cnt]));
	}
	
	if(!strcmp(list_entry, ""))
		doSystem("iwpriv %s set ACLClearAll=1", get_wifname(bssidx));
	else
		doSystem("iwpriv %s set ACLAddEntry=\"%s\"", get_wifname(bssidx), list_entry);
	
}

#endif

#if defined(RTCONFIG_RALINK_MT7621)
#define IDLE_CPU 2
int Set_RAST_CPU(void)
{
	pid_t pid2 = getpid();
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	//sched_getaffinity(0, sizeof(cpuset), &cpuset);
	//dbg(" %s:%d, pid=%d  ori_cur_mask = %08lx\n", __FUNCTION__,__LINE__,  pid2, cpuset);	
	
	CPU_SET(IDLE_CPU, &cpuset);
	//dbg(" ##### %s:%d, set cur_mask = %08lx\n", __FUNCTION__,__LINE__, cpuset);					
	sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
	
	//sched_getaffinity(0, sizeof(cpuset), &cpuset);
	//dbg(" ##### %s:%d, confirm set cur_mask = %08lx\n", __FUNCTION__,__LINE__, cpuset);		
}
#endif