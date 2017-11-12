#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <shared.h>
#include <rc.h>
#include <wlioctl.h>
#include <bcmendian.h>
#include <wlutils.h>
#include <roamast.h>

#if 0
#ifdef RTCONFIG_ADV_RAST
static uint8 bss_token = 0;
#endif
#endif

void get_stainfo(int bssidx, int vifidx)
{
	struct maclist *mac_list;
        int mac_list_size;
        scb_val_t scb_val;
        int mcnt;
        char wlif_name[32];
        int32 rssi;
        rast_sta_info_t *sta = NULL;
        sta_info_t *sta_info = NULL;

        if(vifidx > 0)  snprintf(wlif_name, sizeof(wlif_name), "wl%d.%d", bssidx, vifidx);

        else            strcpy(wlif_name, bssinfo[bssidx].wlif_name);

        mac_list_size = sizeof(mac_list->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
        mac_list = malloc(mac_list_size);

        if(!mac_list)
                goto exit;

        memset(mac_list, 0, mac_list_size);

        /* query authentication sta list */
        strcpy((char*) mac_list, "authe_sta_list");
        if(wl_ioctl(wlif_name, WLC_GET_VAR, mac_list, mac_list_size))
                goto exit;

        for(mcnt=0; mcnt < mac_list->count; mcnt++) {
                sta_info = wl_sta_info(wlif_name, &mac_list->ea[mcnt]);
                if(!sta_info)   continue;
	 	if(!(sta_info->flags & WL_STA_ASSOC) && !sta_info->in) continue;

                memcpy(&scb_val.ea, &mac_list->ea[mcnt], ETHER_ADDR_LEN);
                if(wl_ioctl(wlif_name, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t)))
                        continue;

                rssi = scb_val.val;

                /* add to assoclist */
                sta = rast_add_to_assoclist(bssidx, vifidx, &(mac_list->ea[mcnt]));
                sta->rssi = rssi;
                sta->active = uptime();
	}

exit:
        if(mac_list) free(mac_list);

	return;
}

sta_info_t *
wl_sta_info(char *ifname, struct ether_addr *ea)
{
        static char buf[sizeof(sta_info_t)];
        sta_info_t *sta = NULL;

        strcpy(buf, "sta_info");
        memcpy(buf + strlen(buf) + 1, (void *)ea, ETHER_ADDR_LEN);

        if (!wl_ioctl(ifname, WLC_GET_VAR, buf, sizeof(buf))) {
                sta = (sta_info_t *)buf;
                sta->ver = dtoh16(sta->ver);

                /* Report unrecognized version */
                if (sta->ver > WL_STA_VER) {
                        RAST_DBG("ERROR: unknown driver station info version %d\n", sta->ver);
                        return NULL;
                }

                sta->len = dtoh16(sta->len);
                sta->cap = dtoh16(sta->cap);
#ifdef RTCONFIG_BCMARM
                sta->aid = dtoh16(sta->aid);
#endif
                sta->flags = dtoh32(sta->flags);
                sta->idle = dtoh32(sta->idle);
                sta->rateset.count = dtoh32(sta->rateset.count);
                sta->in = dtoh32(sta->in);
                sta->listen_interval_inms = dtoh32(sta->listen_interval_inms);
#ifdef RTCONFIG_BCMARM
                sta->ht_capabilities = dtoh16(sta->ht_capabilities);
                sta->vht_flags = dtoh16(sta->vht_flags);
#endif
        }

        return sta;
}

#if defined(RTCONFIG_BCMARM) || defined(RTCONFIG_BCMWL6)
void rast_retrieve_bs_data(int bssidx, int vifidx)
{
#ifdef RTCONFIG_BCMARM
        int ret;
        int argn;
        char ioctl_buf[4096];
        iov_bs_data_struct_t *data = (iov_bs_data_struct_t *)ioctl_buf;
        iov_bs_data_record_t *rec;
        iov_bs_data_counters_t *ctr;
        float datarate;
        rast_sta_info_t *sta = NULL;
        char wlif_name[32];

        if(vifidx > 0)  snprintf(wlif_name, sizeof(wlif_name), "wl%d.%d", bssidx, vifidx);
        else            strcpy(wlif_name, bssinfo[bssidx].wlif_name);

        memset(ioctl_buf, 0, sizeof(ioctl_buf));
        strcpy(ioctl_buf, "bs_data");
        ret = wl_ioctl(wlif_name, WLC_GET_VAR, ioctl_buf, sizeof(ioctl_buf));
        if(ret < 0) {
                return;
        }

        for(argn = 0; argn < data->structure_count; argn++) {
                rec = &data->structure_record[argn];
                ctr = &rec->station_counters;

                if(ctr->acked == 0) continue;

                datarate = (ctr->time_delta) ?
                        (float)ctr->throughput * 8000.0 / (float)ctr->time_delta : 0.0;
                sta = rast_add_to_assoclist(bssidx, vifidx, &(rec->station_address));
                if(sta) {
                        sta->datarate = datarate;
                }
        }
        return;

#else   //BRCM MIPS platform

        rast_sta_info_t *sta;
        sta_info_t *sta_info = NULL;
        char wlif_name[32];
        float datarate = 0;
        int interval;
        uint32 curpkts;

        interval = RAST_POLL_INTV_NORMAL;

        if(vifidx > 0)  snprintf(wlif_name, sizeof(wlif_name), "wl%d.%d", bssidx, vifidx);
        else            strcpy(wlif_name, bssinfo[bssidx].wlif_name);

        sta = bssinfo[bssidx].assoclist[vifidx];
        while(sta) {
                        sta_info = wl_sta_info(wlif_name, &sta->addr);
                        if(sta_info) {
                                RAST_DBG("sta "MACF": idle:%d, tx_packets:%d, prepkts=%d\n",
                                        ETHERP_TO_MACF(&sta->addr),
                                        dtoh32(sta_info->idle),
                                        dtoh32(sta_info->tx_pkts),
                                        sta->prepkts);
				
				curpkts = dtoh32(sta_info->tx_pkts);
                                datarate = (float)(curpkts - sta->prepkts)/(float)interval;     //using pkt rate for mips platform
                                sta->prepkts = curpkts;
                                sta->datarate = datarate;
                        }

                sta = sta->next;
        }
#endif
}
#endif


#ifdef RTCONFIG_ADV_RAST
int rast_stamon_get_rssi(int bssidx, struct ether_addr *addr)
{
	wlc_stamon_sta_config_t stamon_cfg;
        stamon_info_t *pbuf;
        char data_buf[WLC_IOCTL_MAXLEN];
        char wlif_name[64];
        int ret=0, i=0;
        int sta_rssi=0;

        snprintf(wlif_name, sizeof(wlif_name), "%s", bssinfo[bssidx].wlif_name);
        memset(&stamon_cfg, 0, sizeof(wlc_stamon_sta_config_t));
        memset(data_buf, 0, WLC_IOCTL_MAXLEN);

        // enable sta_monitor feature
        stamon_cfg.cmd = STAMON_CFG_CMD_ENB;
        ret = wl_iovar_set(wlif_name, "sta_monitor", &stamon_cfg, sizeof(wlc_stamon_sta_config_t));
        if (ret < 0) {
                RAST_INFO("stamon enable failure\n");
                return sta_rssi;
        }

        // add sta into sta_monitor list
        stamon_cfg.cmd = STAMON_CFG_CMD_ADD;
        stamon_cfg.ea = *addr;
        ret = wl_iovar_set(wlif_name, "sta_monitor", &stamon_cfg, sizeof(wlc_stamon_sta_config_t));
        if (ret < 0) {
                RAST_INFO("stamon add failure\n");
                return sta_rssi;
        }

        // retrieve sta rssi
        stamon_cfg.cmd = STAMON_CFG_CMD_GET_STATS;
        for (i=0; i<3; i++)
        {
		usleep(500000);
                ret = wl_iovar_getbuf(wlif_name, "sta_monitor", &stamon_cfg, sizeof(wlc_stamon_sta_config_t), data_buf, WLC_IOCTL_MAXLEN);
                pbuf = (stamon_info_t*)data_buf;
                if (pbuf->count != 0) {
                        if(pbuf->sta_data[0].rssi !=0) {
                                sta_rssi = (sta_rssi == 0 ? pbuf->sta_data[0].rssi : (pbuf->sta_data[0].rssi > sta_rssi ? pbuf->sta_data[0].rssi : sta_rssi));
                                RAST_DBG("[%d]sta: "MACF" rssi = %d dBm\n", i,ETHER_TO_MACF(pbuf->sta_data[0].ea), sta_rssi);
                        }
                }
        }

        // remove sta from sta_monitor list
        stamon_cfg.cmd = STAMON_CFG_CMD_DEL;
        ret = wl_iovar_set(wlif_name, "sta_monitor", &stamon_cfg, sizeof(wlc_stamon_sta_config_t));
        if (ret < 0) {
                RAST_INFO("stamon del failure\n");
        }


        // disable sta_monitor
        stamon_cfg.cmd = STAMON_CFG_CMD_DSB;
        ret = wl_iovar_set(wlif_name, "sta_monitor", &stamon_cfg, sizeof(wlc_stamon_sta_config_t));
        if (ret < 0) {
                RAST_INFO("stamon disable failure\n");
        }

        return sta_rssi;
}

void rast_retrieve_static_maclist(int bssidx, int vifidx)
{
	int ret, size;
        char wlif_name[64];
        struct maclist *maclist = (struct maclist *) maclist_buf;

        if(vifidx > 0)
                snprintf(wlif_name, sizeof(wlif_name), "wl%d.%d", bssidx, vifidx);
        else
                snprintf(wlif_name, sizeof(wlif_name), "%s", bssinfo[bssidx].wlif_name);

        ret = wl_ioctl(wlif_name, WLC_GET_MACMODE, &(bssinfo[bssidx].static_macmode[vifidx]), sizeof(bssinfo[bssidx].static_macmode[vifidx]));
        if(ret < 0) {
                RAST_INFO("[WARNING] %s get macmode error!!!\n", wlif_name);
                return;
        }

        RAST_DBG("[%s] macmode = %s\n",
                wlif_name,
                bssinfo[bssidx].static_macmode[vifidx]==WLC_MACMODE_DISABLED ? "DISABLE" :
                bssinfo[bssidx].static_macmode[vifidx]==WLC_MACMODE_DENY ? "DENY" : "ALLOW");

        ret = wl_ioctl(wlif_name, WLC_GET_MACLIST, (void *)maclist, sizeof(maclist_buf));
        if(ret < 0) {
                RAST_INFO("[WARNING] %s get macmode error!!!\n", wlif_name);
                return;
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
                        RAST_DBG("[%s] (%d)mac:"MACF"\n",wlif_name, size, ETHER_TO_MACF(maclist->ea[size]));
                }
        } else if (maclist->count != 0) {
                RAST_INFO("Err: %s maclist cnt [%d] too large\n",
                wlif_name, maclist->count);
                return;
        }
}

void rast_set_maclist(int bssidx, int vifidx)
{
	char wlif_name[64];
        rast_maclist_t *r_maclist = bssinfo[bssidx].maclist[vifidx];
        struct maclist *maclist = (struct maclist *)maclist_buf;
        struct maclist *static_maclist = bssinfo[bssidx].static_maclist[vifidx];
        int static_macmode = bssinfo[bssidx].static_macmode[vifidx];
        int ret, val;
        struct ether_addr *ea;
        int cnt, match;

        if(vifidx > 0)
                snprintf(wlif_name, sizeof(wlif_name), "wl%d.%d", bssidx, vifidx);
        else
                snprintf(wlif_name, sizeof(wlif_name), "%s", bssinfo[bssidx].wlif_name);

        if (static_macmode == WLC_MACMODE_DENY || static_macmode == WLC_MACMODE_DISABLED)
                val = WLC_MACMODE_DENY;
        else
                val = WLC_MACMODE_ALLOW;

        ret = wl_ioctl(wlif_name, WLC_SET_MACMODE, &val, sizeof(val));
        if(ret < 0) {
                RAST_INFO("[WARNING] %s set macmode error!!!\n", wlif_name);
                return;
        }

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
	else {  //ALLOW MODE
                ea = &(maclist->ea[0]);

                if (!static_maclist) {
                        RAST_INFO("[ERROR] %s macmode:%d static_list is NULL\n",
                                wlif_name, static_macmode);
                        return;
                }

                for (cnt = 0; cnt < static_maclist->count; cnt++) {
                        RAST_DBG("Allow mode: static mac[%d] addr:"MACF"\n", cnt,
                                ETHER_TO_MACF(static_maclist->ea[cnt]));
                        /* if mac in static maclist match rast maclist, skip it */
                        match = 0;
                        while(r_maclist) {
                                RAST_DBG("Checking "MACF"\n", ETHER_TO_MACF(r_maclist->addr));
                                if (eacmp(&(r_maclist->addr), &(static_maclist->ea[cnt])) == 0) {
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

	RAST_DBG("maclist count[%d] \n", maclist->count);
        for (cnt = 0; cnt < maclist->count; cnt++) {
                RAST_DBG("maclist: "MACF"\n",
                        ETHER_TO_MACF(maclist->ea[cnt]));
        }

        ret = wl_ioctl(wlif_name, WLC_SET_MACLIST, maclist, sizeof(maclist_buf));
        if (ret < 0) {
                RAST_DBG("Err: [%s] set maclist...\n", wlif_name);
        }
}
# if 0
uint8 rast_get_rclass(int bssidx, int vifidx)
{
	char wlif_name[32];
	char ioctl_buf[256];
	char *param;
	int buflen;
	uint8 rclass = 0;
	chanspec_t chanspec;

        if(vifidx > 0)  snprintf(wlif_name, sizeof(wlif_name), "wl%d.%d", bssidx, vifidx);
        else            strcpy(wlif_name, bssinfo[bssidx].wlif_name);

	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	strcpy(ioctl_buf, "chanspec");
	if(wl_ioctl(wlif_name, WLC_GET_VAR, ioctl_buf, sizeof(ioctl_buf))){
		RAST_INFO("Error to read chanspec: %s\n", wlif_name);
	}
	else {
		chanspec = (chanspec_t)(*((uint32 *)ioctl_buf));
		RAST_DBG("[%s] chanspec: 0x%x\n", wlif_name, chanspec);

		memset(ioctl_buf, 0, sizeof(ioctl_buf));
		strcpy(ioctl_buf, "rclass");
		buflen = strlen(ioctl_buf) + 1;
		param = (char *)(ioctl_buf + buflen);
		memcpy(param, &chanspec, sizeof(chanspec_t));

		if(wl_ioctl(wlif_name, WLC_GET_VAR, ioctl_buf, sizeof(ioctl_buf))){
			RAST_INFO("Error to read rclass: %s\n", wlif_name);
		}
		rclass = (uint8)(*((uint32 *)ioctl_buf));
		RAST_DBG("[%s] rclass: 0x%x\n", wlif_name, rclass);
	}

	return rclass;
}

uint8 rast_get_channel(int bssidx, int vifidx)
{
	char wlif_name[32];
	char ioctl_buf[256];
	uint8 channel = 0;
	chanspec_t chanspec;

        if(vifidx > 0)  snprintf(wlif_name, sizeof(wlif_name), "wl%d.%d", bssidx, vifidx);
        else            strcpy(wlif_name, bssinfo[bssidx].wlif_name);

	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	strcpy(ioctl_buf, "chanspec");
	if(wl_ioctl(wlif_name, WLC_GET_VAR, ioctl_buf, sizeof(ioctl_buf))){
		RAST_INFO("Error to read chanspec: %s\n", wlif_name);
	}
	else {
		chanspec = (chanspec_t)(*((uint32 *)ioctl_buf));
		RAST_DBG("[%s] chanspec: 0x%x\n", wlif_name, chanspec);

		channel = wf_chspec_ctlchan(chanspec);
		RAST_DBG("[%s] channel: 0x%x\n", wlif_name, channel);
	}

	return channel;
}

int rast_send_bsstrans_req(int bssidx, int vifidx, struct ether_addr *sta_addr, struct ether_addr *nbr_bssid)
{
#if 0
/* BSS Management Transition Request frame header */
BWL_PRE_PACKED_STRUCT struct dot11_bsstrans_req {
        uint8 category;                 /* category of action frame (10) */
        uint8 action;                   /* WNM action: trans_req (7) */
        uint8 token;                    /* dialog token */
        uint8 reqmode;                  /* transition request mode */
        uint16 disassoc_tmr;            /* disassociation timer */
        uint8 validity_intrvl;          /* validity interval */
        uint8 data[1];                  /* optional: BSS term duration, ... */
                                                /* ...session info URL, candidate list */
} BWL_POST_PACKED_STRUCT;

/* Neighbor Report element (11k & 11v) */
BWL_PRE_PACKED_STRUCT struct dot11_neighbor_rep_ie {
        uint8 id;
        uint8 len;
        struct ether_addr bssid;
        uint32 bssid_info;
        uint8 reg;              /* Operating class */
        uint8 channel;
        uint8 phytype;
        uint8 data[1];          /* Variable size subelements */
} BWL_POST_PACKED_STRUCT;
#endif
	char wlif_name[32];
	char ioctl_buf[4096];
	int buflen;
	char *param;

	dot11_bsstrans_req_t *transreq;		/* BSS Management Transition Request frame header */
	dot11_neighbor_rep_ie_t *nbr_ie;	/* Neighbor Report element */

	wl_af_params_t *af_params;
	wl_action_frame_t *action_frame;	/* action frame */

	if(vifidx > 0)  snprintf(wlif_name, sizeof(wlif_name), "wl%d.%d", bssidx, vifidx);
        else            strcpy(wlif_name, bssinfo[bssidx].wlif_name);

	memset(ioctl_buf, 0, sizeof(ioctl_buf));
        strcpy(ioctl_buf, "actframe");
        buflen = strlen(ioctl_buf) + 1;
        param = (char *)(ioctl_buf + buflen);

	af_params = (wl_af_params_t *)param;
        action_frame = &af_params->action_frame;

	af_params->channel = 0;
        af_params->dwell_time = -1;

        memcpy(&action_frame->da, (char *)(sta_addr), ETHER_ADDR_LEN);
        action_frame->packetId = (uint32)(uintptr)action_frame;
        action_frame->len = DOT11_NEIGHBOR_REP_IE_FIXED_LEN + TLV_HDR_LEN + DOT11_BSSTRANS_REQ_LEN;

        transreq = (dot11_bsstrans_req_t *)&action_frame->data[0];
        transreq->category = DOT11_ACTION_CAT_WNM;
        transreq->action = DOT11_WNM_ACTION_BSSTRANS_REQ;
        if (++bss_token == 0)
                bss_token = 1;
        transreq->token = bss_token;
        transreq->reqmode = DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL;
        /* set bit1 to tell STA the BSSID in list recommended */
        transreq->reqmode |= DOT11_BSSTRANS_REQMODE_ABRIDGED;
        /*
                remove bit2 DOT11_BSSTRANS_REQMODE_DISASSOC_IMMINENT
                because bsd will deauth sta based on BSS response
        */
        transreq->disassoc_tmr = 0x0000;
        transreq->validity_intrvl = 0x00;

	nbr_ie = (dot11_neighbor_rep_ie_t *)&transreq->data[0];
        nbr_ie->id = DOT11_MNG_NEIGHBOR_REP_ID;
        nbr_ie->len = DOT11_NEIGHBOR_REP_IE_FIXED_LEN;
	memcpy(&nbr_ie->bssid, nbr_bssid, ETHER_ADDR_LEN);
        nbr_ie->bssid_info = 0x00000000;
        nbr_ie->reg = rast_get_rclass(bssidx, vifidx);		// assume the same as cueernt ap
	nbr_ie->channel = rast_get_channel(bssidx, vifidx);	// assume the same as current ap
        nbr_ie->phytype = 0x00;

	RAST_DBG("[%s] Sending 11v bss transition frame to sta "MACF" with candicate ap "MACF"\n", 
			wlif_name, ETHER_TO_MACF(*sta_addr), ETHER_TO_MACF(nbr_ie->bssid));

	if(wl_ioctl(wlif_name, WLC_SET_VAR, ioctl_buf, WL_WIFI_AF_PARAMS_SIZE)){
		RAST_INFO("Error to send action frame: %s\n", wlif_name);
		return 0;
	}

        return 1;
}
#endif
#endif
