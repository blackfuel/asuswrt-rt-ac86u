#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <unistd.h>
//#include <linux/wireless.h>
#include "wireless.h"
#include <assert.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdarg.h>
#include <signal.h>
#include "air_monitor.h"

#define DEBUG 1

uint32_t RTDebugLevel = RT_DEBUG_ERROR;
AIR_MONITOR_CFG g_air_cfg;

#ifdef DEBUG
#define DBGPRINT(Level, fmt, ...) 					\
{                                   \
    if (Level <= RTDebugLevel)      \
    {                               \
        printf(NIC_DBG_STRING);   \
		printf( fmt, ##__VA_ARGS__);			\
    }                               \
}
#else
#define DBGPRINT(Level, fmt, args...)
#endif

void hex_dump(char *str, uint8_t *pSrcBufVA, uint32_t SrcBufLen)
{
	unsigned char *pt;
	int x;

	pt = pSrcBufVA;
	printf("len = %d\n", SrcBufLen);
	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			printf("0x%04X : ", x);
		printf("%02X ", ((unsigned char)pt[x]));
		if (x % 16 == 15)
			printf("\n");
	}
}

static char *phy_mode_str[5]={"CCK", "OFDM", "HTMIX", "GF", "VHT"};
char* get_phymode_str(int Mode)
{
	if ((Mode >= 0) && (Mode < 5))
		return phy_mode_str[Mode];
	else
		return "N/A";
}

static char *phy_bw_str[4] = {"20M", "40M", "80M", "10M"};
char* get_bw_str(int bandwidth)
{
	if ((bandwidth >= 0) && (bandwidth < 4))
		return phy_bw_str[bandwidth];
	else
		return "N/A";
}

static int mtk_open_notify_socket( const char* if_name )
{
    DBGPRINT(RT_DEBUG_ERROR,"entering, if_name=%s\n", if_name );

    int ifindex = if_nametoindex( if_name );
    if ( ifindex == 0 ) {
        DBGPRINT(RT_DEBUG_ERROR, "if_nametoindex(%s) failed\n", if_name );
        return -1;
    }

    DBGPRINT(RT_DEBUG_ERROR,"entering, ifindex=%d\n", ifindex );

    int sock = socket( PF_PACKET, SOCK_RAW, htons(/*ETH_P_PAE*/ETH_P_AIR_MONITOR/*ETH_P_ALL*/) );
    if ( sock < 0 ) {
        DBGPRINT(RT_DEBUG_ERROR, "%s(%d): %s\n", __FILE__, __LINE__, strerror(errno) );
        return -1;
    }

    struct sockaddr_ll addr;
    memset( &addr, 0, sizeof(addr) );
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = ifindex;

    int rc = bind( sock, (struct sockaddr*)&addr, sizeof(addr) );
    if ( rc < 0 ) {
        DBGPRINT(RT_DEBUG_ERROR, "%s(%d): %s\n", __FILE__, __LINE__, strerror(errno) );
        close( sock );
        return -1;
    }

    return sock;
}

static void mtk_handle_l2(uint8_t *buf, int len)
{
    AIR_RADIO_INFO *pwlan_radio_tap = NULL;
    HEADER_802_11_4_ADDR *pwlan_header = NULL;
	AIR_RAW *recv = NULL;
    char *pTypeStr = NULL;
    static int valid_len = (LENGTH_802_3 + sizeof(AIR_RAW));

	if (buf && (len >= valid_len))
	{
		recv = (AIR_RAW *)(buf + LENGTH_802_3);
        pwlan_radio_tap = &recv->wlan_radio_tap;
        pwlan_header = &recv->wlan_header;
		switch (pwlan_header->FC.Type)
		{
			case BTYPE_DATA:
            case BTYPE_MGMT:
            case BTYPE_CNTL:
                if(pwlan_header->FC.Type == BTYPE_DATA)
                    pTypeStr = "BTYPE_DATA";
                else if(pwlan_header->FC.Type == BTYPE_MGMT)
                    pTypeStr = "BTYPE_MGMT";
                else
                    pTypeStr = "BTYPE_CNTL";

                DBGPRINT(RT_DEBUG_TRACE,"###############################\n");
                DBGPRINT(RT_DEBUG_TRACE,"type = %s\n",pTypeStr);
                DBGPRINT(RT_DEBUG_TRACE,"RSSI = %d,%d,%d,%d\n", pwlan_radio_tap->RSSI[0],pwlan_radio_tap->RSSI[1],
                        pwlan_radio_tap->RSSI[2],pwlan_radio_tap->RSSI[3]);
                DBGPRINT(RT_DEBUG_TRACE,"RATE = %lu\n",pwlan_radio_tap->RATE);
                DBGPRINT(RT_DEBUG_TRACE,"PHYMODE = %s\n",get_phymode_str(pwlan_radio_tap->PHYMODE));
                DBGPRINT(RT_DEBUG_TRACE,"SS = %d\n",pwlan_radio_tap->SS);
                DBGPRINT(RT_DEBUG_TRACE,"MCS = %d\n",pwlan_radio_tap->MCS);
                DBGPRINT(RT_DEBUG_TRACE,"BW = %s\n",get_bw_str(pwlan_radio_tap->BW));
                DBGPRINT(RT_DEBUG_TRACE,"ShortGI = %d\n",pwlan_radio_tap->ShortGI);
                DBGPRINT(RT_DEBUG_TRACE,"FrDs = %d, ToDs = %d\n",pwlan_header->FC.FrDs, pwlan_header->FC.ToDs);
                DBGPRINT(RT_DEBUG_TRACE,"Duration = %d, SN = %d, FN = %d\n", pwlan_header->Duration, pwlan_header->SN, pwlan_header->FN);
	       		hex_dump("$$$$ mtk_handle_l2 dump ==>", (uint8_t*)recv, (len - LENGTH_802_3));
                DBGPRINT(RT_DEBUG_TRACE,"###############################\n\n");
				break;
			default:
	       		//hex_dump("$$$$ mtk_handle_l2 drop ==>", (uint8_t*)recv, (len - LENGTH_802_3));
				break;
		}
	}
}

static void mtk_if_notify_thread(AIR_MONITOR_CFG *pair_cfg)
{
    fd_set rfds;
    uint8_t buf[1024];
    int len;
    int ret =0;

    if ( pair_cfg == NULL ) {
        DBGPRINT(RT_DEBUG_ERROR, "invalid priv pointer" );
        return;
    }

	while (1) {
        if (pair_cfg->notif_sock == 0) {
            const int sock = mtk_open_notify_socket(pair_cfg->if_name);
            if (sock < 0) {
                DBGPRINT(RT_DEBUG_ERROR, "unable to open notify socket; will sleep 1s and retry!\n");
                sleep(1);
                continue;
            }
            pair_cfg->notif_sock = sock;
        }

        FD_ZERO( &rfds );
        FD_SET( pair_cfg->notif_sock, &rfds );
        ret = select( pair_cfg->notif_sock + 1, &rfds, NULL, NULL, NULL );

        if ( ! FD_ISSET( pair_cfg->notif_sock, &rfds ) ) {
            continue;
        }

        len = recv( pair_cfg->notif_sock, buf, sizeof(buf), 0 );
        if ( len <= 0 ) {
            DBGPRINT(RT_DEBUG_ERROR, "didn't recv() any data: %s\n", strerror(errno) );
            close(pair_cfg->notif_sock);
            pair_cfg->notif_sock = 0;
            return;
        }
        mtk_handle_l2(buf, len);
    }
}

void signal_catch(int msgno) {
    printf("***air_monitor Exit***\n");
    if(g_air_cfg.notif_sock > 0)
        close(g_air_cfg.notif_sock);
    exit(0);
}

static void usage(void)
{
	DBGPRINT(RT_DEBUG_OFF, "USAGE : air_monitor [optional command]\n");
	DBGPRINT(RT_DEBUG_OFF, "[optional command] : \n");
	DBGPRINT(RT_DEBUG_OFF, "-i<bridge> : indicate which bridge is used\n");
	DBGPRINT(RT_DEBUG_OFF, "-d<debug_level> : set debug level\n");
	exit(1);
}

int air_monitor_main(int argc, char *argv[])
{
	int ret = 1, i;
	int c;
	int sig;
	sigset_t sigs_to_catch;
    pid_t auth_pid;
    char *infName = NULL;

	memset(&g_air_cfg, 0 , sizeof(g_air_cfg));
	for (;;)
    {
		c = getopt(argc, argv, "d:i:h");
		if (c < 0)
			break;

		switch (c)
        {
			case 'd':
				/* 	set Debug level -
						RT_DEBUG_OFF		0
						RT_DEBUG_ERROR		1
						RT_DEBUG_WARN		2
						RT_DEBUG_TRACE		3
						RT_DEBUG_INFO		4
				*/
				printf("Set debug level as %s\n", optarg);
				RTDebugLevel = (int)strtol(optarg, 0, 10);
				break;

			case 'i':
				infName = optarg;
				if (strlen(infName))
					strncpy(&g_air_cfg.if_name[0], infName, IFNAMSIZ);
				else
					strcpy(&g_air_cfg.if_name[0], "br0");

			    break;

			case 'h':
		    default:
				usage();
			    break;
		}
	}
    signal(SIGINT,signal_catch);
    signal(SIGTERM,signal_catch);

    DBGPRINT(RT_DEBUG_TRACE, "inf_name = '%s'\n", g_air_cfg.if_name);
    mtk_if_notify_thread(&g_air_cfg);

    if(g_air_cfg.notif_sock > 0)
        close(g_air_cfg.notif_sock);
}
