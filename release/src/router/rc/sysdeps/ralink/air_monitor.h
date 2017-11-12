//
// Copied from oid.h
//

#define MAC_ADDR_LEN 6
#define LENGTH_802_3                14

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif
#ifndef ETH_P_ALL
#define ETH_P_ALL 0x0003
#endif

#define NIC_DBG_STRING      ("[air_monitor] ")

#define RT_DEBUG_OFF		0
#define RT_DEBUG_ERROR		1
#define RT_DEBUG_WARN		2
#define RT_DEBUG_TRACE		3
#define RT_DEBUG_INFO		4

#define	OID_GET_SET_TOGGLE							0x8000
#define RT_PRIV_IOCTL                               (SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET                            (SIOCIWFIRSTPRIV + 0x02)

#ifndef ETH_P_PAE
#define ETH_P_PAE 0x888E /* Port Access Entity (IEEE 802.1X) */
#endif /* ETH_P_PAE */

#ifndef ETH_P_AIR_MONITOR
#define ETH_P_AIR_MONITOR 0x51A0
#endif /* ETH_P_AIR_MONITOR */


#ifndef ETH_P_ALL
#define ETH_P_ALL 0x0003
#endif


#ifndef PACKED
#define PACKED  __attribute__ ((packed))
#endif

#define BTYPE_MGMT                  0
#define BTYPE_CNTL                  1
#define BTYPE_DATA                  2


/* value domain of 802.11 MGMT frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header */
#define SUBTYPE_ASSOC_REQ           0
#define SUBTYPE_ASSOC_RSP           1
#define SUBTYPE_REASSOC_REQ         2
#define SUBTYPE_REASSOC_RSP         3
#define SUBTYPE_PROBE_REQ           4
#define SUBTYPE_PROBE_RSP           5
#define SUBTYPE_BEACON              8
#define SUBTYPE_ATIM                9
#define SUBTYPE_DISASSOC            10
#define SUBTYPE_AUTH                11
#define SUBTYPE_DEAUTH              12
#define SUBTYPE_ACTION              13
#define SUBTYPE_ACTION_NO_ACK       14

/* value domain of 802.11 CNTL frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header */
#define SUBTYPE_WRAPPER       	7
#define SUBTYPE_BLOCK_ACK_REQ       8
#define SUBTYPE_BLOCK_ACK           9
#define SUBTYPE_PS_POLL             10
#define SUBTYPE_RTS                 11
#define SUBTYPE_CTS                 12
#define SUBTYPE_ACK                 13
#define SUBTYPE_CFEND               14
#define SUBTYPE_CFEND_CFACK         15

/* value domain of 802.11 DATA frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header */
#define SUBTYPE_DATA                0
#define SUBTYPE_DATA_CFACK          1
#define SUBTYPE_DATA_CFPOLL         2
#define SUBTYPE_DATA_CFACK_CFPOLL   3
#define SUBTYPE_NULL_FUNC           4
#define SUBTYPE_CFACK               5
#define SUBTYPE_CFPOLL              6
#define SUBTYPE_CFACK_CFPOLL        7
#define SUBTYPE_QDATA               8
#define SUBTYPE_QDATA_CFACK         9
#define SUBTYPE_QDATA_CFPOLL        10
#define SUBTYPE_QDATA_CFACK_CFPOLL  11
#define SUBTYPE_QOS_NULL            12
#define SUBTYPE_QOS_CFACK           13
#define SUBTYPE_QOS_CFPOLL          14
#define SUBTYPE_QOS_CFACK_CFPOLL    15

/* ACK policy of QOS Control field bit 6:5 */
#define NORMAL_ACK                  0x00	/* b6:5 = 00 */
#define NO_ACK                      0x20	/* b6:5 = 01 */
#define NO_EXPLICIT_ACK             0x40	/* b6:5 = 10 */
#define BLOCK_ACK                   0x60	/* b6:5 = 11 */


/* 2-byte Frame control field */
typedef struct _FRAME_CONTROL {
#ifdef __BIG_ENDIAN__
	uint16_t Order:1;		/* Strict order expected */
	uint16_t Wep:1;		/* Wep data */
	uint16_t MoreData:1;	/* More data bit */
	uint16_t PwrMgmt:1;	/* Power management bit */
	uint16_t Retry:1;		/* Retry status bit */
	uint16_t MoreFrag:1;	/* More fragment bit */
	uint16_t FrDs:1;		/* From DS indication */
	uint16_t ToDs:1;		/* To DS indication */
	uint16_t SubType:4;	/* MSDU subtype */
	uint16_t Type:2;		/* MSDU type */
	uint16_t Ver:2;		/* Protocol version */
#else
    uint16_t Ver:2;		/* Protocol version */
	uint16_t Type:2;		/* MSDU type */
	uint16_t SubType:4;	/* MSDU subtype */
	uint16_t ToDs:1;		/* To DS indication */
	uint16_t FrDs:1;		/* From DS indication */
	uint16_t MoreFrag:1;	/* More fragment bit */
	uint16_t Retry:1;		/* Retry status bit */
	uint16_t PwrMgmt:1;	/* Power management bit */
	uint16_t MoreData:1;	/* More data bit */
	uint16_t Wep:1;		/* Wep data */
	uint16_t Order:1;		/* Strict order expected */
#endif	/* __BIG_ENDIAN__ */
} FRAME_CONTROL;

typedef struct _HEADER_802_11_4_ADDR {
    FRAME_CONTROL           FC;
    unsigned short          Duration;
    unsigned short			SN;
    unsigned char           FN;
    unsigned char           Addr1[MAC_ADDR_LEN];
    unsigned char           Addr2[MAC_ADDR_LEN];
	unsigned char			Addr3[MAC_ADDR_LEN];
    unsigned char	        Addr4[MAC_ADDR_LEN];
} HEADER_802_11_4_ADDR, *PHEADER_802_11_4_ADDR;

typedef struct _AIR_RADIO_INFO{
	char PHYMODE;
    char SS;
	char MCS;
	char BW;
	char ShortGI;
	unsigned long RATE;
    char RSSI[4];
} AIR_RADIO_INFO, *PAIR_RADIO_INFO;

typedef struct _AIR_RAW{
	 AIR_RADIO_INFO wlan_radio_tap;
	 HEADER_802_11_4_ADDR wlan_header;
} AIR_RAW, *PAIR_RAW;

typedef struct _AIR_MONITOR_CFG{
    char       if_name[IFNAMSIZ];
    int        notif_sock;
} AIR_MONITOR_CFG, *PAIR_MONITOR_CFG;
