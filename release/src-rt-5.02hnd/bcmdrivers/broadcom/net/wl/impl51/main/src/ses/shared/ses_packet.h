/*
 * SES Packet Exchange API header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: ses_packet.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _ses_packet_h_
#define _ses_packet_h_

#include "ses.h"

/* mode */
#define SES_PE_MODE_CONFIGURATOR	1
#define SES_PE_MODE_CLIENT		2

/* bit-or phase fields (configurator mode only) */
#define SES_PE_3BUTTON_PRE_CONFIRM	1
#define SES_PE_3BUTTON_POST_CONFIRM	2

/* status returned by SES_packet_exchange() */
#define SES_PE_STATUS_SUCCESS		0
#define SES_PE_STATUS_WFC		1	/* configurator only */
#define SES_PE_STATUS_WFI		2	/* client only */
#define SES_PE_STATUS_EXCH_FAIL		3
#define SES_PE_STATUS_PEER_ERROR	4
#define SES_PE_STATUS_INTERNAL_ERROR	5
#define SES_PE_STATUS_CONTINUE		6
#define SES_PE_STATUS_INITIATED		7	/* configurator only */

/* The following structure is the means of exchanging data between 
 * packet_exchange library and its caller through the SES_setup() call.
 * The status of exchange is returned by function SES_packet_exchange() through 
 * SES_PE_STATUS_* defines
 * The exchange can be cancelled any time using SES_cancel().
 * The data structures need to be cleaned up using SES_cleanup() at the end.
 * The following fields are used in configurator mode for pe:
 *      IN mode
 * 	IN phase
 * 	IN ssid
 *	IN passphrase
 *	OUT remote
 *	IN ifnames
 *	OUT pe_ifname
 *	IN security
 *	IN encryption
 *
 * The following fields are used in client mode for pe:
 *      IN mode
 * 	OUT ssid
 *	OUT passphrase
 *	IN remote
 *	IN ifnames
 *	OUT pe_ifname
 *	OUT security
 *	OUT encryption
 *
 * The following fields are used for echo test:
 *      IN mode
 *	INOUT remote
 *	IN ifnames
 */
typedef struct ses_packet_exchange {
	int version;			/* version number */
	int mode; 			/* configurator or client */
	int phase;			/* phase1 and/or phase2 */
	char ssid[SES_MAX_SSID_LEN+1];	/* ssid */
	char passphrase[SES_MAX_PASSPHRASE_LEN+1]; 	/* passphrase */
	struct ether_addr remote;	/* ethernet address of peer */
	char ifnames[SES_IFNAME_SIZE*SES_PE_MAX_INTERFACES]; /* interface names */
	char pe_ifname[SES_IFNAME_SIZE];/* interface on which exch happened */
	char wl_ifname[SES_IFNAME_SIZE];/* wl interface name (cf only) */
	uint8 security;			/* wpa/wpa2 */
	uint8 encryption;		/* tkip/aes */
#ifdef SES_TEST_DH_TIMING
	int dh_len;			/* length of dh key */
#endif
	void *private;
} ses_packet_exchange_t;


/* APIs for packet exchange module */
int SES_setup(ses_packet_exchange_t *spe);
int SES_packet_exchange(ses_packet_exchange_t *spe, int timeout);
int SES_cancel(ses_packet_exchange_t *spe);
void SES_cleanup(ses_packet_exchange_t *spe);
int SES_echo_setup(ses_packet_exchange_t *spe);

#endif /* _ses_packet_h_ */
