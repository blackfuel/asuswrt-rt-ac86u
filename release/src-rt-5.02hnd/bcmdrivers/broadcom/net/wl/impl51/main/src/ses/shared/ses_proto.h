/*
 * SES Packet Exchange Protocol Header File
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: ses_proto.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _ses_proto_h_
#define _ses_proto_h_

typedef struct {
	uint8 ver;		/* EAPOL protocol version */
	uint8 type;		/* EAPOL type */
	uint16 length;	/* Length of body */
	/* BODY of EAPOL */
	uint8 eap_code;			/* EAP code */
	uint8 eap_id;			/* Current request ID */
	uint16 eap_length;		/* Length including header */
	uint8 eap_type;			/* EAP type [uses expanded] : 254 */
	uint8 eap_vend_id[3];		/* SMI Network Management Private Enterprise Code */
	uint8 eap_vend_type[4];		/* EAP type */
} bcm_eapol_header_t;

/* SES packet with ilcp ethertype */
typedef struct ses_header {
	struct ether_header eth;		/* 802.3/Ethernet header */
	bcm_eapol_header_t eapol;        /* EAPOL/EAP header */
	uint8 version;			/* ses version */
	uint8 ses_type;			/* ses packet type */
	uint8 flags;			/* packet flags */
	uint8 reserved;			/* for future */
} ses_header_t;

#define SES_PE_MAX_RETRIES		3
#define SES_PE_LARGE_RETRIES		1000
#define SES_PE_RETRY_INTERVAL		1
#define SES_PACKET_ENCR_OVERHEAD	8
#define SES_HEADER_LEN			sizeof (struct ses_header)	
#define SES_SPECIFIC_HEADER_LEN		4 		

/* SES packet types */
#define SES_PACKET_TYPE_STATUS		0	 		
#define SES_PACKET_TYPE_HELLO		1
#define SES_PACKET_TYPE_KEY1		2
#define SES_PACKET_TYPE_KEY2		3
#define SES_PACKET_TYPE_INFO		4
#define SES_PACKET_TYPE_ECHO_REQ	5
#define SES_PACKET_TYPE_ECHO_RESP	6

/* SES packet flags */
#define SES_PACKET_FLAG_RETRANSMIT	1

#define SES_PACKET_ECHO_LEN 		64

#define SES_PACKET_STATUS_SUCCESS	1
#define SES_PACKET_STATUS_ERROR		2

typedef struct ses_packet_status {
	uint8 status;
} ses_packet_status_t;

typedef struct ses_packet_key {
	uint8 pub_key_len;
	char pub_key[SES_MAX_KEY_LEN];
} ses_packet_key_t;

typedef struct ses_packet_info {
	char ssid[SES_MAX_SSID_LEN+1];	/* ssid */
	uint8 passphrase_len;		/* length of passphrase */
	uint8 encr_passphrase[SES_MAX_PASSPHRASE_LEN+1+SES_PACKET_ENCR_OVERHEAD];
	uint8 security;
	uint8 encryption;
} ses_packet_info_t;

/* this is the obsoleted structure with pe version 1 which was for SES 2.0.
 * Currently not bumping up the pe ver to 2, since we are adding new fields
 * at the end */
typedef struct ses_packet_info_v1_old {
	char ssid[SES_MAX_SSID_LEN+1];	/* ssid */
	uint8 passphrase_len;		/* length of passphrase */
	uint8 encr_passphrase[SES_MAX_PASSPHRASE_LEN+1+SES_PACKET_ENCR_OVERHEAD];
} ses_packet_info_v1_old_t;

#endif /* _ses_proto_h_ */
