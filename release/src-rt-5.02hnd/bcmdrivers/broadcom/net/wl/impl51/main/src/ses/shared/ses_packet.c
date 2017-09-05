/*
 * SES packet exchange common code
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: ses_packet.c 241182 2011-02-17 21:50:03Z $
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmendian.h>

#ifndef _WIN32
#include <netinet/in.h>
#include <net/if.h>
#else
#include <winsock2.h>
#include <IpTypes.h>	/* IFNAMSIZ */
#define IFNAMSIZ	MAX_ADAPTER_NAME_LENGTH + 4
#endif
#include <proto/ethernet.h>
#include <proto/802.11.h> 
#include <proto/eapol.h>
#include <proto/eap.h>
#include <assert.h>
#ifndef _WIN32
#include <shutils.h>
#else
extern int ether_atoe(const char *a, unsigned char *e);
extern char * ether_etoa(const unsigned char *e, char *a);
#endif
#include <bcmutils.h>
#include <bcmcrypto/aeskeywrap.h>
#include <bcmcrypto/sha1.h>
#include <bcmcrypto/dh.h>

#include <ses_dbg.h>
#include <ses_packet.h>
#include <ses_cmnport.h>
#include <ses_proto.h>

#if !defined(SES_PE_CLIENT) && !defined(SES_PE_CONFIGURATOR)
#error "SES_PE_CLIENT or SES_PE_CONFIGURATOR must be defined"
#endif

/* SES state machine states */
/* configurator states */
#define SES_WAITING_FOR_HELLO 		1
#define SES_WAITING_FOR_CONFIRM 	2
#define SES_WAITING_FOR_KEY2		3
#define SES_WAITING_FOR_ECHO_REQ	4
#define SES_WAITING_FOR_STATUS		5
#define SES_WAITING_FOR_ECHO_STATUS	6

/* client states */
#define SES_WAITING_FOR_KEY1		11
#define SES_WAITING_FOR_INFO		12
#define SES_WAITING_FOR_ECHO_RESP	13

/* common states */
#define SES_SETUP_DONE			21
#define SES_ECHO_SETUP_DONE		22
#define SES_COMPLETE			23

#define SES_AES_KEK_SIZE		16

typedef struct ses_pe_private {
	int pe_state;
	struct ether_addr ea[SES_PE_MAX_INTERFACES];
	struct ether_addr src_ea;
	struct ether_addr wl_ea;
	char *ifname[SES_PE_MAX_INTERFACES];
	int fd[SES_PE_MAX_INTERFACES];
	int num_fd;
	int fd_index;
	ses_timer_id tid;
	int retries;
	bool post_confirm;
	bool three_button;
	int status;
	void *dh;
	int peer_key_len;
	uchar peer_key[SES_MAX_KEY_LEN];
	int kek_len;
	char kek[SES_AES_KEK_SIZE];
	uint8 spacket[1024];		/* Max size packet to send */
} ses_pe_private_t;

static int ses_send_pkt_data(ses_packet_exchange_t *spe, uint8 type, uint8 flags);
static int ses_open_connections(ses_packet_exchange_t *spe);
static int ses_compute_kek(ses_packet_exchange_t *spe, char *kek, int kek_len);
#ifdef SES_PE_CLIENT
static int ses_pe_client(ses_packet_exchange_t *spe, uint8 *pkt, uint len);
#endif
#ifdef SES_PE_CONFIGURATOR
static int ses_pe_configurator(ses_packet_exchange_t *spe, int index, uint8 *pkt, uint len);
#endif
static int ses_retransmit(ses_timer_id tid, ses_packet_exchange_t *spe);
static int ses_set_retransmit_timer(ses_packet_exchange_t *spe);
static void ses_delete_retransmit_timer(ses_packet_exchange_t *spe);
static int ses_get_status(ses_packet_exchange_t *spe);
static void ses_validate_constants(void);
#ifdef BCMDBG
static char *ses_ptype_str(uint type);
static char *ses_pe_state_str(uint state);
#endif


#define	SES_PE_VALIDATE_PACKET_TYPE(recd, expected, flags) \
	if (recd != expected) {	\
		if (flags & SES_PACKET_FLAG_RETRANSMIT) \
			return SES_PE_STATUS_CONTINUE; \
		else { \
			SES_ERROR("recd unexpected packet type %s; expecting %s\n", \
				ses_ptype_str(recd), ses_ptype_str(expected)); \
			return SES_PE_STATUS_EXCH_FAIL; \
		} \
	}

#define SES_PE_VALIDATE_LEN(recd, expected) \
	if (((recd) - SES_HEADER_LEN) < (expected)) { \
		SES_ERROR("recd len %d on pkt; expected at least %d\n", \
			(recd)-SES_HEADER_LEN, (expected)); \
		return SES_PE_STATUS_EXCH_FAIL; \
	}

#define SES_PE_VALIDATE_FD(recd, expected) \
	if (recd != expected) { \
		SES_ERROR("ses packet on unexpected interface\n"); \
		return SES_PE_STATUS_EXCH_FAIL; \
	}

#define SES_PE_VALIDATE_SRC_MAC(recd, expected) \
	if (memcmp(recd, expected, ETHER_ADDR_LEN)) { \
		SES_ERROR("invalid source mac address on packet\n"); \
		SES_ERROR("expected %s; recd %s\n", \
			ether_etoa((unsigned char *)expected, eabuf1), \
			ether_etoa((unsigned char *)recd, eabuf2)); \
		return SES_PE_STATUS_EXCH_FAIL; \
	}

#define SES_PE_HANDLE_STATUS(hdr) \
	if ((hdr)->ses_type == SES_PACKET_TYPE_STATUS) { \
		ses_packet_status_t *s; \
		s = (ses_packet_status_t *)&(hdr)[1]; \
		if (s->status == SES_PACKET_STATUS_SUCCESS) \
			return SES_PE_STATUS_CONTINUE; \
		else \
			return SES_PE_STATUS_PEER_ERROR; \
	}

#define SES_PE_HANDLE_SELF_BCAST_ECHO_REQ(hdr, mac) \
	if ((hdr->ses_type == SES_PACKET_TYPE_ECHO_REQ) && \
            (!memcmp(hdr->eth.ether_dhost, "\xFF\xFF\xFF\xFF\xFF\xFF", ETHER_ADDR_LEN)) && \
            (!memcmp(hdr->eth.ether_shost, mac, ETHER_ADDR_LEN))) { \
		SES_WARNING("ignoring bcast to self\n"); \
		return SES_PE_STATUS_CONTINUE; \
	}

#define SES_PE_HANDLE_ECHO_REQ(spe, hdr) \
	if (hdr->ses_type == SES_PACKET_TYPE_ECHO_REQ) { \
		struct ether_addr tmpaddr; \
		memcpy(&tmpaddr, &spe->remote, ETHER_ADDR_LEN); \
		memcpy(&spe->remote, hdr->eth.ether_shost, ETHER_ADDR_LEN); \
		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_ECHO_RESP, 0, FALSE, FALSE); \
		memcpy(&spe->remote, &tmpaddr, ETHER_ADDR_LEN); \
		return SES_PE_STATUS_CONTINUE; \
	}

#define SES_PE_SEND_PKT_DATA(spe, type, flags, timer, ret) \
	{ \
	if (ses_send_pkt_data(spe, type, flags) != SES_PE_STATUS_SUCCESS) { \
		SES_ERROR("ses_send_pkt_data() failed\n");\
		if (ret) \
			return SES_PE_STATUS_INTERNAL_ERROR; \
	} \
	if (timer) \
		ses_set_retransmit_timer(spe); \
	}


#ifdef NOT_NEEDED
unsigned char p192[]={
        0xD4,0xA0,0xBA,0x02,0x50,0xB6,0xFD,0x2E,
        0xC6,0x26,0xE7,0xEF,0xD6,0x37,0xDF,0x76,
        0xC7,0x16,0xE2,0x2D,0x09,0x44,0xB8,0x8B,
        };

unsigned char p1024[]={
        0x97,0xF6,0x42,0x61,0xCA,0xB5,0x05,0xDD,
        0x28,0x28,0xE1,0x3F,0x1D,0x68,0xB6,0xD3,
        0xDB,0xD0,0xF3,0x13,0x04,0x7F,0x40,0xE8,
        0x56,0xDA,0x58,0xCB,0x13,0xB8,0xA1,0xBF,
        0x2B,0x78,0x3A,0x4C,0x6D,0x59,0xD5,0xF9,
        0x2A,0xFC,0x6C,0xFF,0x3D,0x69,0x3F,0x78,
        0xB2,0x3D,0x4F,0x31,0x60,0xA9,0x50,0x2E,
        0x3E,0xFA,0xF7,0xAB,0x5E,0x1A,0xD5,0xA6,
        0x5E,0x55,0x43,0x13,0x82,0x8D,0xA8,0x3B,
        0x9F,0xF2,0xD9,0x41,0xDE,0xE9,0x56,0x89,
        0xFA,0xDA,0xEA,0x09,0x36,0xAD,0xDF,0x19,
        0x71,0xFE,0x63,0x5B,0x20,0xAF,0x47,0x03,
        0x64,0x60,0x3C,0x2D,0xE0,0x59,0xF5,0x4B,
        0x65,0x0A,0xD8,0xFA,0x0C,0xF7,0x01,0x21,
        0xC7,0x47,0x99,0xD7,0x58,0x71,0x32,0xBE,
        0x9B,0x99,0x9B,0xB9,0xB7,0x87,0xE8,0xAB,
        };
#endif

#ifdef SES_TEST_DH_TIMING
unsigned char p512[]={
        0xDA,0x58,0x3C,0x16,0xD9,0x85,0x22,0x89,
        0xD0,0xE4,0xAF,0x75,0x6F,0x4C,0xCA,0x92,
        0xDD,0x4B,0xE5,0x33,0xB8,0x04,0xFB,0x0F,
        0xED,0x94,0xEF,0x9C,0x8A,0x44,0x03,0xED,
        0x57,0x46,0x50,0xD3,0x69,0x99,0xDB,0x29,
        0xD7,0x76,0x27,0x6B,0xA2,0xD3,0xD4,0x12,
        0xE2,0x18,0xF4,0xDD,0x1E,0x08,0x4C,0xF6,
        0xD8,0x00,0x3E,0x7C,0x47,0x74,0xE8,0x33,
        };

unsigned char p1024[]={  /* self-generated */
	0xB4,0x85,0x52,0x1D,0x13,0xAA,0x84,0x72,
	0x23,0x51,0xBA,0x00,0x63,0x08,0x7C,0xA6,
	0x4B,0xDA,0xE9,0xF0,0x12,0x05,0x84,0x9D,
	0xE7,0x3E,0x98,0xF7,0x79,0x4E,0xF0,0x91,
	0x5E,0xA2,0x08,0x87,0x80,0xC7,0x86,0x0C,
	0x27,0x8A,0x12,0x38,0x0B,0x58,0x5C,0xD8,
	0x8F,0x54,0x01,0x92,0xA3,0xDF,0x0C,0xBE,
	0xDF,0x22,0x1E,0xA0,0xBF,0x3B,0x58,0x02,
	0x70,0x2C,0x13,0xD1,0xC3,0xB6,0x05,0x81,
	0x91,0xD7,0x8B,0x20,0xF1,0x91,0x7D,0x6A,
	0xEF,0x37,0x21,0xAF,0x3E,0x6A,0xF5,0x25,
	0xAF,0x01,0xA8,0x2A,0x7C,0x45,0x29,0xD9,
	0xCC,0xB2,0x6A,0x90,0x5F,0x93,0x7F,0x99,
	0xF3,0x15,0x33,0x47,0x10,0xEA,0xE7,0x47,
	0xCC,0x06,0x0C,0xA2,0xA4,0x70,0x4A,0xB6,
	0x7F,0x69,0x30,0x24,0xE9,0x58,0xB9,0x23,
	};

unsigned char p2048[]={
	0xE6,0x92,0x4E,0x95,0xE6,0x27,0x44,0x16,
	0xAB,0x60,0xA4,0x6D,0x9B,0x3F,0xF1,0x5D,
	0x4E,0xAC,0x45,0xB3,0x3F,0xEC,0x60,0x92,
	0x1F,0xB7,0x07,0x52,0x2F,0x36,0xA7,0x29,
	0x44,0xF2,0x03,0x91,0x56,0x01,0x4D,0x22,
	0xCA,0x29,0x6C,0x1A,0x6D,0x18,0xC9,0xD0,
	0x01,0x89,0x1D,0xC8,0x96,0xA7,0x64,0x54,
	0x22,0xE8,0x25,0x0F,0x58,0xC8,0x4C,0x51,
	0xB6,0xBC,0x5C,0x74,0xE4,0xBC,0xB5,0x08,
	0x00,0x6D,0xD1,0xB7,0x0D,0x6E,0x2A,0x89,
	0x88,0xEB,0xCF,0xC4,0x47,0x3A,0x59,0xF4,
	0xFF,0xE4,0x2E,0xCC,0x00,0x09,0x0E,0x3A,
	0x50,0x95,0x7E,0x10,0x29,0x48,0x1A,0xE3,
	0x7B,0xF8,0xCC,0x9E,0x76,0x44,0xB3,0x2C,
	0xDC,0xD0,0x7D,0x60,0xD3,0xF0,0xBA,0x26,
	0x47,0xD4,0x01,0xC4,0xEA,0xE3,0xC7,0x6D,
	0x93,0x69,0x9E,0x5B,0xC6,0x8A,0x99,0xED,
	0x64,0x8A,0x69,0x2B,0x66,0x36,0xB4,0x33,
	0x3A,0x27,0xEE,0x66,0xE7,0x5F,0xEB,0x27,
	0xA5,0x80,0xC0,0x31,0x78,0xF9,0xB5,0x16,
	0x31,0xBC,0xCF,0x6F,0x01,0xFF,0x68,0xE7,
	0xBF,0x71,0x55,0x73,0xA0,0xA5,0x45,0x39,
	0x64,0x84,0x39,0x9C,0xA8,0xEA,0x51,0xF6,
	0x96,0x54,0xB1,0xF4,0x67,0x55,0x50,0x9F,
	0xBD,0xEB,0x8C,0x42,0x1B,0x01,0x31,0x10,
	0xDD,0xFA,0x3F,0x2B,0x82,0x5B,0x88,0x8B,
	0x55,0x11,0xDB,0xCC,0x08,0xE3,0xEA,0x11,
	0xE4,0xBB,0xE3,0x38,0x49,0x79,0x51,0xB1,
	0x89,0xF1,0x01,0xF7,0x42,0xE8,0x20,0xF9,
	0x06,0xE6,0x56,0x96,0x0E,0xEE,0xD7,0x9B,
	0x75,0x46,0x11,0x34,0x35,0x41,0x07,0x50,
	0xDA,0x82,0x68,0x3B,0xFA,0x79,0x03,0x2B,
	};
#endif

unsigned char p1536[]={
	0xEB,0xDC,0x6A,0xFE,0xA4,0x34,0x39,0xBD,
	0xB1,0x9D,0x13,0x31,0xEC,0xC9,0x6F,0x71,
	0x0E,0x4D,0x46,0x4F,0x72,0x37,0x78,0x96,
	0xD7,0xC9,0x22,0x6C,0xD5,0x34,0x7E,0x1B,
	0xE7,0xB1,0xD9,0x40,0xB1,0x9C,0xDF,0x55,
	0x20,0xC8,0xBE,0xDF,0x63,0xC6,0x53,0xB0,
	0xA6,0x77,0x45,0x8E,0xCA,0xF5,0xD6,0x88,
	0x3F,0x06,0x43,0x71,0xD6,0x3E,0xCF,0x09,
	0x9A,0x24,0xFC,0x2A,0xF4,0xFD,0xA0,0x25,
	0x04,0x9C,0x93,0x0C,0x44,0xAD,0x51,0x44,
	0xFC,0x2E,0x0A,0xE6,0x47,0x97,0xF1,0x67,
	0xF7,0xB2,0x0B,0x84,0x30,0x98,0x1F,0xE3,
	0xD7,0x0A,0xD3,0x66,0x7F,0x8C,0x29,0x36,
	0xF6,0x2A,0x80,0xB3,0x75,0xB0,0x79,0x9C,
	0x21,0xB5,0x5E,0xC1,0xD9,0xBA,0x2A,0xC7,
	0x44,0xC2,0x0E,0x9D,0x0E,0x5D,0x38,0x19,
	0x6A,0x22,0x86,0x93,0x99,0x0C,0x58,0x93,
	0x96,0xA6,0x83,0x39,0x37,0xC8,0x35,0xA7,
	0x09,0x4E,0x4F,0x97,0xED,0x01,0xB1,0x91,
	0x71,0xF6,0x3F,0x62,0x0C,0x7D,0x54,0x8C,
	0xAC,0xB0,0x7C,0x6D,0x20,0xFF,0x77,0x13,
	0x11,0x0A,0xC5,0xF1,0x53,0x11,0x07,0xF5,
	0xBA,0x58,0x97,0x84,0x5C,0xE0,0x92,0x5F,
	0x4D,0xFA,0x49,0xE1,0x92,0x25,0x06,0xAB,
	};


int
SES_setup(ses_packet_exchange_t *spe)
{
	ses_pe_private_t *spep;
	int status;
	bool post_confirm = FALSE; /* if coming in for post confirm in 3 button sol */
	bool three_button = FALSE;

	ses_validate_constants();

	assert(spe);

	assert((spe->mode == SES_PE_MODE_CONFIGURATOR) ||
	       (spe->mode == SES_PE_MODE_CLIENT));

#ifdef SES_PE_CONFIGURATOR
	if (spe->mode == SES_PE_MODE_CONFIGURATOR) {
		/* at least one phase should be set for configurator */
		assert(spe->phase & (SES_PE_3BUTTON_PRE_CONFIRM | 
				     SES_PE_3BUTTON_POST_CONFIRM));

		/* set 3-button if only pre_confirm is set */
		if ((spe->phase & SES_PE_3BUTTON_PRE_CONFIRM) &&
	    	   !(spe->phase & SES_PE_3BUTTON_POST_CONFIRM))
			three_button = TRUE;

		/* set reentry if only post_confirm is set */
		if ((spe->phase & SES_PE_3BUTTON_POST_CONFIRM) &&
	    	   !(spe->phase & SES_PE_3BUTTON_PRE_CONFIRM)) {
			post_confirm = TRUE;
		}

		SES_INFO("Entry with ssid %s passphrase %s\n", 
			spe->ssid, spe->passphrase);
	}
#endif

	if (!post_confirm) {
		if (!strcmp(spe->ifnames, "")) {
			SES_ERROR("null ifnames list\n");
			return SES_PE_STATUS_INTERNAL_ERROR;
		}

		/* Allocate private memory */
		spep = (ses_pe_private_t *)malloc(sizeof(ses_pe_private_t));
		if (!spep) {
			SES_ERROR("malloc() failed\n");
			return SES_PE_STATUS_INTERNAL_ERROR;
		}
		memset(spep, 0, sizeof(ses_pe_private_t));
	
		spe->private = spep;
		spep->three_button = three_button;
	
		status = ses_open_connections(spe);
		if (status != SES_PE_STATUS_SUCCESS) {
			SES_ERROR("ses_open_connections() failed\n");
			free(spep);
			return status;
		}	

		BN_register_RAND(ses_random);

#ifdef SES_TEST_DH_TIMING
		if (spe->dh_len == 512)
			spep->dh = DH_init(p512, sizeof(p512), 2);
		else if (spe->dh_len == 1024)
			spep->dh = DH_init(p1024, sizeof(p1024), 2);
		else if (spe->dh_len == 2048)
			spep->dh = DH_init(p2048, sizeof(p2048), 2);
		else
#endif
		spep->dh = DH_init(p1536, sizeof(p1536), 2);
		if (spep->dh == NULL) {
			SES_ERROR("DH_init() failed\n");
			free(spep);
			return SES_PE_STATUS_INTERNAL_ERROR;
		}
	} else {
		spep = spe->private;
		assert(spep);
		assert(spep->pe_state == SES_WAITING_FOR_CONFIRM);
	}

	spep->post_confirm = post_confirm;

#ifdef SES_PE_CLIENT
	if (spe->mode == SES_PE_MODE_CLIENT) {
		/* client initiates and has only one active fd */
		if (spep->num_fd != 1) {
			SES_ERROR("client numfd != 1\n");
			free(spep);
			return SES_PE_STATUS_INTERNAL_ERROR;
		}
		spep->fd_index = 0;
		strcpy(spe->pe_ifname, spep->ifname[0]);

		if (!strcmp(spe->wl_ifname, "")) {
                	if (ses_wl_hwaddr(spe->pe_ifname, spep->src_ea.octet) < 0) {
                        	SES_ERROR("cannot obtain hw address for %s\n", 
					spe->pe_ifname);
				free(spep);
                        	return SES_PE_STATUS_INTERNAL_ERROR;
                	}
		} else {
                	if (ses_wl_hwaddr(spe->wl_ifname, spep->src_ea.octet) < 0) {
                        	SES_ERROR("cannot obtain hw address for %s\n", 
					spe->wl_ifname);
				free(spep);
                        	return SES_PE_STATUS_INTERNAL_ERROR;
                	}
		}

		/* set remote address to bcast if not set */
		if (!memcmp(&spe->remote, "\0\0\0\0\0\0", ETHER_ADDR_LEN)) {
			SES_ERROR("setting remote ethr address broadcast\n");
			memset(&spe->remote, 0xFF, ETHER_ADDR_LEN);
		}
	} else
#endif /* SES_PE_CLIENT */
                if (ses_wl_hwaddr(spe->wl_ifname, spep->wl_ea.octet) < 0) {
                        SES_ERROR("cannot obtain hw address for %s\n", spe->wl_ifname);
                        return SES_PE_STATUS_INTERNAL_ERROR;
                }

	spep->status = SES_PACKET_STATUS_SUCCESS;
	spep->pe_state = SES_SETUP_DONE;

	return SES_PE_STATUS_SUCCESS;
}


int
SES_packet_exchange(ses_packet_exchange_t *spe, int time)
{
	ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;
	int status = SES_PE_STATUS_CONTINUE;

	if (!spep)
		return SES_PE_STATUS_INTERNAL_ERROR;

	/* handle init setup states */
#ifdef SES_PE_CLIENT
	if (spe->mode == SES_PE_MODE_CLIENT) {
		if (spep->pe_state == SES_SETUP_DONE) {
			spep->pe_state = SES_WAITING_FOR_KEY1;
			SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_HELLO, 0, TRUE, TRUE);
		}
		if (spep->pe_state == SES_ECHO_SETUP_DONE) {
			spep->pe_state = SES_WAITING_FOR_ECHO_RESP;
			SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_ECHO_REQ, 0, TRUE, TRUE);
		}
	}
#endif

#ifdef SES_PE_CONFIGURATOR
	if (spe->mode == SES_PE_MODE_CONFIGURATOR) {
		if (spep->pe_state == SES_SETUP_DONE) {
			if (spep->post_confirm == FALSE) {
				spep->pe_state = SES_WAITING_FOR_HELLO;
			} else {
				spep->pe_state = SES_WAITING_FOR_STATUS;
				SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_INFO, 0, TRUE, TRUE);
			}
		}
		if (spep->pe_state == SES_ECHO_SETUP_DONE) {
			spep->pe_state = SES_WAITING_FOR_ECHO_REQ;
		}
	}
#endif

	assert(time);

	while (time--) {
		status = ses_wait_for_packet(spe, spep->num_fd, spep->fd, 1);

		if (status == SES_PE_STATUS_SUCCESS)
			break;

		if ((status == SES_PE_STATUS_EXCH_FAIL) ||
	    	    (status == SES_PE_STATUS_INTERNAL_ERROR) ||
	    	    (status == SES_PE_STATUS_PEER_ERROR)) {
			SES_cancel(spe);
			break;
		}

		status = ses_get_status(spe);
		if (status != SES_PE_STATUS_CONTINUE)
			break;
	}

	SES_INFO("packet exchange returning with status %d\n", status);
	return status;
}


int
SES_cancel(ses_packet_exchange_t *spe)
{
	ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;

	SES_INFO("packet exchange cancelled\n");

	if (!spep)
		return SES_PE_STATUS_INTERNAL_ERROR;

	if ((spep->pe_state != SES_ECHO_SETUP_DONE) &&
	    (spep->pe_state != SES_WAITING_FOR_ECHO_RESP)) {
		spep->status = SES_PACKET_STATUS_ERROR;
		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_STATUS, 0, FALSE, FALSE);
	}
	ses_delete_retransmit_timer(spe);

	return SES_PE_STATUS_SUCCESS;
}

void
SES_cleanup(ses_packet_exchange_t *spe)
{
	ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;
	int i;

	SES_INFO("packet exchange cleanup\n");

	if (!spep)
		return;

	ses_delete_retransmit_timer(spe);
	for(i = 0; i < spep->num_fd; i++) {
		ses_close_socket(spep->fd[i]);
	}
	DH_free(spep->dh);
	free(spep);

	return;
}

int
SES_echo_setup(ses_packet_exchange_t *spe)
{
	ses_pe_private_t *spep;
	int status;

	ses_validate_constants();
	assert(spe);
	assert((spe->mode == SES_PE_MODE_CONFIGURATOR) ||
	       (spe->mode == SES_PE_MODE_CLIENT));
	assert(strcmp(spe->ifnames, ""));

	/* Allocate private memory */
	spep = (ses_pe_private_t *)malloc(sizeof(ses_pe_private_t));
	if (!spep) {
		SES_ERROR("malloc() failed\n");
		return SES_PE_STATUS_INTERNAL_ERROR;
	}
	memset(spep, 0, sizeof(ses_pe_private_t));
	spe->private = spep;
	
	status = ses_open_connections(spe);
	if (status != SES_PE_STATUS_SUCCESS) {
		SES_ERROR("ses_open_connections() failed\n");
		free(spep);
		return status;
	}	

        if (ses_wl_hwaddr(spep->ifname[0], spep->src_ea.octet) < 0) {
                SES_ERROR("cannot obtain hw address for %s\n", spe->pe_ifname);
		free(spep);
                return SES_PE_STATUS_INTERNAL_ERROR;
        }

	assert(spep->num_fd == 1);
	spep->fd_index = 0;

	spep->status = SES_PACKET_STATUS_SUCCESS;
	spep->pe_state = SES_ECHO_SETUP_DONE;

	return SES_PE_STATUS_SUCCESS;
}

#ifdef SES_PE_CLIENT
static int
ses_pe_client(ses_packet_exchange_t *spe, uint8 *pkt, uint len)
{
	ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;
	ses_header_t *hdr = (ses_header_t *)pkt; 
#ifdef BCMDBG
	char eabuf1[ETHER_ADDR_STR_LEN], eabuf2[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */
	int i;

	SES_INFO("recd pkt %s in state %s\n",
		ses_ptype_str(hdr->ses_type), ses_pe_state_str(spep->pe_state));

	switch (spep->pe_state) {

	case SES_WAITING_FOR_KEY1:
		{
		ses_packet_key_t *key;

		SES_PE_HANDLE_STATUS(hdr);
		SES_PE_VALIDATE_PACKET_TYPE(hdr->ses_type, SES_PACKET_TYPE_KEY1, 
					    hdr->flags);
		SES_PE_VALIDATE_LEN(len, sizeof(ses_packet_key_t));

		ses_delete_retransmit_timer(spe);

		/* get remote mac address if specified as broadcast */
		if (!memcmp(&spe->remote, "\xFF\xFF\xFF\xFF\xFF\xFF", ETHER_ADDR_LEN)) {
			memcpy(&spe->remote, hdr->eth.ether_shost, ETHER_ADDR_LEN);
			SES_ERROR("updating remote mac address to %s\n",
				ether_etoa((unsigned char *)&spe->remote, eabuf1));
		}

		key = (ses_packet_key_t *)&hdr[1];

		memcpy(spep->peer_key, key->pub_key, key->pub_key_len);
		spep->peer_key_len = key->pub_key_len;

		spep->pe_state = SES_WAITING_FOR_INFO;

		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_KEY2, 0, TRUE, FALSE);

		return SES_PE_STATUS_CONTINUE;
		}

	case SES_WAITING_FOR_INFO:
		{
		ses_packet_info_t *info;
		unsigned char kek[SES_AES_KEK_SIZE];

		SES_PE_HANDLE_STATUS(hdr);
		SES_PE_VALIDATE_PACKET_TYPE(hdr->ses_type, SES_PACKET_TYPE_INFO, 
					    hdr->flags);
		SES_PE_VALIDATE_LEN(len, sizeof(ses_packet_info_v1_old_t));
		SES_PE_VALIDATE_SRC_MAC(hdr->eth.ether_shost, &spe->remote);

		ses_delete_retransmit_timer(spe);

		info = (ses_packet_info_t *)&hdr[1];

		strcpy(spe->ssid, info->ssid);

		if (ses_compute_kek(spe, (char *)kek, sizeof(kek)) != SES_PE_STATUS_SUCCESS) {
			SES_ERROR("ses_compute_kek() failed\n");
			return SES_PE_STATUS_INTERNAL_ERROR;
		}

		if (aes_unwrap(sizeof(kek), kek, 
			       ROUNDUP(info->passphrase_len, AKW_BLOCK_LEN) + AKW_BLOCK_LEN,
			       info->encr_passphrase, (uint8 *)spe->passphrase) != 0) {
			SES_ERROR("aes_unwrap(%d) failed\n", info->passphrase_len);
			return SES_PE_STATUS_INTERNAL_ERROR;
		}

		spe->passphrase[info->passphrase_len] = 0;

		if ((len - SES_HEADER_LEN) == sizeof(ses_packet_info_v1_old_t)) {
			/* version 1 of configurator with only WPA */
			spe->security = SES_SECURITY_WPA_PSK;
			spe->encryption = SES_ENCRYPTION_TKIP;
		} else {
			/* configurator with wpa2 support */
			spe->security = info->security;
			spe->encryption = info->encryption;
		}

		spep->pe_state = SES_COMPLETE;

		/* no retransmit timer; send only once */
		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_STATUS, 0, FALSE, FALSE);

		return SES_PE_STATUS_SUCCESS;
		}

	case SES_WAITING_FOR_ECHO_RESP: 
		{
		char *data;

		SES_PE_HANDLE_SELF_BCAST_ECHO_REQ(hdr, &spep->ea[0]);
		SES_PE_HANDLE_STATUS(hdr);
		SES_PE_VALIDATE_PACKET_TYPE(hdr->ses_type, SES_PACKET_TYPE_ECHO_RESP, 
					    hdr->flags);
		SES_PE_VALIDATE_LEN(len, SES_PACKET_ECHO_LEN);

		/* save remote mac address */
		memcpy(&spe->remote, hdr->eth.ether_shost, ETHER_ADDR_LEN);

		for(i = 0, data = (char *)&hdr[1]; i < SES_PACKET_ECHO_LEN; i++) {
			if (data[i] != (2*i % 256)) {
				SES_ERROR("unexpected data on echo resp\n");
				return SES_PE_STATUS_EXCH_FAIL;
			}
		}

		/* no retransmit timer; send only once */
		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_STATUS, 0, FALSE, FALSE);

		return SES_PE_STATUS_SUCCESS;
		}

	default:
		SES_ERROR("unexpected state %s\n", 
			ses_pe_state_str(spep->pe_state));
		assert(0);
	}

	return SES_PE_STATUS_CONTINUE;
}
#endif

#ifdef SES_PE_CONFIGURATOR
static int
ses_pe_configurator(ses_packet_exchange_t *spe, int fd_index, uint8 *pkt, uint len)
{
	ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;
	ses_header_t *hdr = (ses_header_t *)pkt; 
#ifdef BCMDBG
	char eabuf1[ETHER_ADDR_STR_LEN], eabuf2[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	assert(fd_index < spep->num_fd);
	SES_INFO("recd pkt %s in state %s\n",
		ses_ptype_str(hdr->ses_type), ses_pe_state_str(spep->pe_state));

	switch (spep->pe_state) {

	case SES_WAITING_FOR_HELLO:
		{
		int i = 0;
		char *c;

		/* store source ea that is to be set in the sent packets.
		 * If the first packet is received with a broadcast dest address,
		 * we assume that the exchange is happening over the wired
		 * interface and thus set our source address as that of received
		 * interface (bridge or eth) else we set it to wl interface 
		 * This is not a good assumption but.... (improve it later)
		 * This issue arises when we are listening on the bridge interface
		 * and dont know which specific interface the packet came on
		 */
		if (!memcmp(&spep->src_ea, "\x00\x00\x00\x00\x00\x00", ETHER_ADDR_LEN)) {
			if (!memcmp(hdr->eth.ether_dhost, "\xFF\xFF\xFF\xFF\xFF\xFF", ETHER_ADDR_LEN))
				memcpy(&spep->src_ea, &spep->ea[spep->fd_index], ETHER_ADDR_LEN);
			else
				memcpy(&spep->src_ea, &spep->wl_ea, ETHER_ADDR_LEN);
		}

		SES_PE_HANDLE_ECHO_REQ(spe, hdr);
		SES_PE_HANDLE_STATUS(hdr);
		SES_PE_VALIDATE_PACKET_TYPE(hdr->ses_type, SES_PACKET_TYPE_HELLO, 
					    hdr->flags);

		/* get remote mac address */
		memcpy(&spe->remote, hdr->eth.ether_shost, ETHER_ADDR_LEN);

		spep->fd_index = fd_index;

		c = spep->ifname[fd_index];
		while ((*c != ' ') && (*c != '\0'))
			spe->pe_ifname[i++] = *c++;
		spe->pe_ifname[i] = '\0';

		spep->pe_state = SES_WAITING_FOR_KEY2;

		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_KEY1, 0, TRUE, FALSE);

		return SES_PE_STATUS_CONTINUE;
		}

	case SES_WAITING_FOR_KEY2:
		{
		ses_packet_key_t *key;

		SES_PE_HANDLE_ECHO_REQ(spe, hdr);
		SES_PE_HANDLE_STATUS(hdr);
		SES_PE_VALIDATE_PACKET_TYPE(hdr->ses_type, SES_PACKET_TYPE_KEY2, 
					    hdr->flags);
		SES_PE_VALIDATE_FD(fd_index, spep->fd_index);
		SES_PE_VALIDATE_SRC_MAC(hdr->eth.ether_shost, &spe->remote);
		SES_PE_VALIDATE_LEN(len, sizeof(ses_packet_key_t));

		ses_delete_retransmit_timer(spe);

		key = (ses_packet_key_t *)&hdr[1];

		memcpy(spep->peer_key, key->pub_key, key->pub_key_len);
		spep->peer_key_len = key->pub_key_len;

		if (spep->three_button == FALSE) {
			spep->pe_state = SES_WAITING_FOR_STATUS;

			SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_INFO, 0, TRUE, FALSE);

			return SES_PE_STATUS_CONTINUE;
		} else {
			spep->pe_state = SES_WAITING_FOR_CONFIRM;

			return SES_PE_STATUS_WFC;
		}
		break;
		}

	case SES_WAITING_FOR_STATUS:
	case SES_WAITING_FOR_ECHO_STATUS:
		{
		ses_packet_status_t *s;

		SES_PE_VALIDATE_PACKET_TYPE(hdr->ses_type, SES_PACKET_TYPE_STATUS, 
					    hdr->flags);
		SES_PE_VALIDATE_FD(fd_index, spep->fd_index);
		SES_PE_VALIDATE_SRC_MAC(hdr->eth.ether_shost, &spe->remote);

		ses_delete_retransmit_timer(spe);

		s = (ses_packet_status_t *)&hdr[1];

		spep->pe_state = SES_COMPLETE;

		if (s->status == SES_PACKET_STATUS_SUCCESS)
			return SES_PE_STATUS_SUCCESS;
		else
			return SES_PE_STATUS_PEER_ERROR;
		}

	case SES_WAITING_FOR_ECHO_REQ: 
		{
		char *data;
		int i;

		SES_PE_HANDLE_STATUS(hdr);
		SES_PE_VALIDATE_PACKET_TYPE(hdr->ses_type, SES_PACKET_TYPE_ECHO_REQ, 
					    hdr->flags);
		SES_PE_VALIDATE_LEN(len, SES_PACKET_ECHO_LEN);
		SES_PE_VALIDATE_SRC_MAC(hdr->eth.ether_shost, &spe->remote);

		for(i = 0, data = (char *)&hdr[1]; i < SES_PACKET_ECHO_LEN; i++) {
			if (data[i] != 2*i) {
				SES_ERROR("unexpected data on echo req\n");
				return SES_PE_STATUS_CONTINUE;
			}
		}

		spep->pe_state = SES_WAITING_FOR_ECHO_STATUS;

		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_ECHO_RESP, 0, TRUE, TRUE);

		return SES_PE_STATUS_CONTINUE;
		}

	default:
		SES_ERROR("unexpected state %s\n", 
			ses_pe_state_str(spep->pe_state));
		assert(0);
	}

	return SES_PE_STATUS_CONTINUE;
}
#endif

int
ses_get_status(ses_packet_exchange_t *spe)
{
	ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;

	if (((spep->pe_state == SES_WAITING_FOR_STATUS) ||
	     (spep->pe_state == SES_WAITING_FOR_ECHO_STATUS)) &&
	    (spep->retries >= SES_PE_MAX_RETRIES)) {
		assert(spe->mode == SES_PE_MODE_CONFIGURATOR);
		SES_ERROR("ack not recd during status wait (assume success)\n");
		return SES_PE_STATUS_SUCCESS;
	}
	 
	if (spep->pe_state == SES_WAITING_FOR_CONFIRM) {
		assert(spe->mode == SES_PE_MODE_CONFIGURATOR);
		return SES_PE_STATUS_WFC;
	}

	if (spe->mode == SES_PE_MODE_CONFIGURATOR) {
		if ((spep->pe_state != SES_WAITING_FOR_HELLO) &&
		    (spep->pe_state != SES_COMPLETE)) 
			return SES_PE_STATUS_INITIATED;
	}

	if (spep->pe_state == SES_WAITING_FOR_INFO) {
		assert(spe->mode == SES_PE_MODE_CLIENT);
		return SES_PE_STATUS_WFI;
	}

	return SES_PE_STATUS_CONTINUE;
}

int
ses_packet_dispatch(void *handle, int fd_index, uint8 *pkt, int len)
{
	ses_header_t *hdr = (ses_header_t *)pkt;
	ses_packet_exchange_t *spe = (ses_packet_exchange_t *)handle;

	/* Ignore non 802.1x EAP BRCM packets (dont error) */
	if ((ntohs(hdr->eth.ether_type) != ETHER_TYPE_802_1X)
		|| (hdr->eapol.type != EAP_PACKET)
		|| (hdr->eapol.eap_type != EAP_EXPANDED)
		|| (hdr->eapol.eap_vend_id[2] != (BCM_SMI_ID & 0xff))
		|| (hdr->eapol.eap_vend_id[1] != ((BCM_SMI_ID & 0xff00) >> 8))
		|| (hdr->eapol.eap_vend_id[0] != ((BCM_SMI_ID & 0xff000) >> 16))) {
		SES_ERROR("Non 802.1x BRCM EAP pkt was ignored\n");
		return SES_PE_STATUS_CONTINUE;
	}

	SES_PACKET(pkt, len);

	/* ignore short frames */
	if (len < SES_HEADER_LEN) {
		SES_ERROR("ignore short ses packet\n");
		return SES_PE_STATUS_CONTINUE;
	}
	
	/* validate full length pkt header length include everything except the ether header and the two bytes subtype and length*/
	if (len < ((ntohs(hdr->eapol.length)) + ETHER_HDR_LEN + 4)) {
		SES_ERROR("ignore incomplete ses packet\n");
		return SES_PE_STATUS_CONTINUE;
	}

#ifdef SES_PE_CLIENT
	if (spe->mode == SES_PE_MODE_CLIENT) {
		return ses_pe_client(spe, pkt, len);
	}
#endif

#ifdef SES_PE_CONFIGURATOR
	if (spe->mode == SES_PE_MODE_CONFIGURATOR) {
		return ses_pe_configurator(spe, fd_index, pkt, len);
	}
#endif

	assert(0);
	return SES_PE_STATUS_INTERNAL_ERROR;
}

static int
ses_retransmit(ses_timer_id tid, ses_packet_exchange_t *spe)
{
	ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;
	int max_retries = SES_PE_MAX_RETRIES;

	spep->retries++;

	SES_INFO("retry timer(%d) kicked in state %s\n", 
		spep->retries, ses_pe_state_str(spep->pe_state));

	switch (spep->pe_state) {

#ifdef SES_PE_CLIENT
	case SES_WAITING_FOR_KEY1:
		assert(spe->mode == SES_PE_MODE_CLIENT);
		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_HELLO, 
				     SES_PACKET_FLAG_RETRANSMIT, FALSE, FALSE);
		max_retries = SES_PE_LARGE_RETRIES;   /* keep trying */
		break;

	case SES_WAITING_FOR_INFO:
		assert(spe->mode == SES_PE_MODE_CLIENT);
		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_KEY2,
				     SES_PACKET_FLAG_RETRANSMIT, FALSE, FALSE);
		break;

	case SES_WAITING_FOR_ECHO_RESP:
		assert(spe->mode == SES_PE_MODE_CLIENT);
		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_ECHO_REQ,
				     SES_PACKET_FLAG_RETRANSMIT, FALSE, FALSE);
		max_retries = SES_PE_LARGE_RETRIES;   /* keep trying */
		break;
#endif

#ifdef SES_PE_CONFIGURATOR
	case SES_WAITING_FOR_KEY2:
		assert(spe->mode == SES_PE_MODE_CONFIGURATOR);
		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_KEY1,
				     SES_PACKET_FLAG_RETRANSMIT, FALSE, FALSE);
		break;

	case SES_WAITING_FOR_STATUS:
		assert(spe->mode == SES_PE_MODE_CONFIGURATOR);
		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_INFO,
				     SES_PACKET_FLAG_RETRANSMIT, FALSE, FALSE);
		break;

	case SES_WAITING_FOR_ECHO_STATUS:
		assert(spe->mode == SES_PE_MODE_CONFIGURATOR);
		SES_PE_SEND_PKT_DATA(spe, SES_PACKET_TYPE_ECHO_RESP,
				     SES_PACKET_FLAG_RETRANSMIT, FALSE, FALSE);
		break;
#endif

	default:
		SES_ERROR("unexpected state %s\n", 
			ses_pe_state_str(spep->pe_state));
		assert(0);

	}

	if (spep->retries >= max_retries) {
		ses_delete_retransmit_timer(spe);
	}
	return SES_PE_STATUS_SUCCESS;
}

static int
ses_send_pkt_data(ses_packet_exchange_t *spe, uint8 type, uint8 flags)
{
	ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;
	ses_header_t *hdr;
#ifdef SES_TEST_PACKET_DROPS
	uint8 rand;
#endif
	int i;
	uint16 ses_payload_len = 0;

	SES_INFO("sending ses packet of type %s\n", ses_ptype_str(type));

#ifdef SES_TEST_PACKET_DROPS
	/* test random drops of packets */
	ses_random(&rand, 1);
	if (rand%2) {
		SES_INFO("dropping ses packet of type %s\n", ses_ptype_str(type));
		return SES_PE_STATUS_SUCCESS;
	}
#endif

	hdr = (ses_header_t *)spep->spacket;
	/* ethernet packet header*/
	memcpy(hdr->eth.ether_dhost, &spe->remote, ETHER_ADDR_LEN);
	memcpy(hdr->eth.ether_shost, &spep->src_ea, ETHER_ADDR_LEN);
	hdr->eth.ether_type = htons(ETHER_TYPE_802_1X);

	/* Fill in the eapol fields */
	hdr->eapol.ver = SES_EAPOL_VERSION;
	hdr->eapol.type = EAP_PACKET;
	hdr->eapol.eap_type = EAP_EXPANDED;
	hdr->eapol.eap_vend_id[2] = BCM_SMI_ID & 0xff;
	hdr->eapol.eap_vend_id[1] = (BCM_SMI_ID & 0xff00) >> 8;
	hdr->eapol.eap_vend_id[0] = (BCM_SMI_ID & 0xff000) >> 16;
	hdr->eapol.eap_vend_type[3] = BCM_EAP_SES;
	hdr->eapol.eap_vend_type[2] = 0;
	hdr->eapol.eap_vend_type[1] = 0;
	hdr->eapol.eap_vend_type[0] = 0;

	hdr->version = spe->version;
	hdr->ses_type = type;
	hdr->flags = flags;
	hdr->reserved = 0;

	switch (type) {

#ifdef SES_PE_CLIENT
	case SES_PACKET_TYPE_HELLO:
		hdr->eapol.eap_code = EAP_RESPONSE; 
		assert(spe->mode == SES_PE_MODE_CLIENT);
		break;

	case SES_PACKET_TYPE_KEY2:
#endif
#ifdef SES_PE_CONFIGURATOR
	case SES_PACKET_TYPE_KEY1:
#endif
		{
		int len;
		uchar buf[SES_MAX_KEY_LEN];
		ses_packet_key_t *key = (ses_packet_key_t *)&hdr[1];

		if (type == SES_PACKET_TYPE_KEY1) {
			assert(spe->mode == SES_PE_MODE_CONFIGURATOR);
			hdr->eapol.eap_code = EAP_REQUEST; 
		} else {
			assert(spe->mode == SES_PE_MODE_CLIENT);
			hdr->eapol.eap_code = EAP_RESPONSE;
		}

		/* SES_ERROR("begin generate: %s\n", file2str("/proc/uptime")); */

		len = DH_generate_key(buf, spep->dh);
		if (!len) {
			SES_ERROR("DH_generate_key() failed\n");
			return SES_PE_STATUS_INTERNAL_ERROR;
		}

		/* SES_ERROR("end generate: %s\n", file2str("/proc/uptime")); */

		key->pub_key_len = len;
		memcpy(key->pub_key, buf, len);

		ses_payload_len = sizeof(ses_packet_key_t);
		break;
		}

#ifdef SES_PE_CONFIGURATOR
	case SES_PACKET_TYPE_INFO: 
		{
		char kek[SES_AES_KEK_SIZE];
		ses_packet_info_t *info = (ses_packet_info_t *)&hdr[1];

		assert(spe->mode == SES_PE_MODE_CONFIGURATOR);
		hdr->eapol.eap_code = EAP_REQUEST; 

		if (ses_compute_kek(spe, kek, sizeof(kek)) != SES_PE_STATUS_SUCCESS) {
			SES_ERROR("ses_compute_kek() failed\n");
			return SES_PE_STATUS_INTERNAL_ERROR;
		}

		memset(info, 0, sizeof(info));
		strcpy(info->ssid, spe->ssid);
		info->passphrase_len = strlen(spe->passphrase);
		info->security = spe->security;
		info->encryption = spe->encryption;

		if (aes_wrap(sizeof(kek), (uint8 *)kek, 
		             ROUNDUP(info->passphrase_len, AKW_BLOCK_LEN), 
			     (uint8*)spe->passphrase, info->encr_passphrase) != 0) {
			SES_ERROR("aes_wrap() failed\n");
			return SES_PE_STATUS_INTERNAL_ERROR;
		}

		ses_payload_len = sizeof(ses_packet_info_t);
		break;
		}
#endif

	case SES_PACKET_TYPE_ECHO_REQ:
	case SES_PACKET_TYPE_ECHO_RESP:
		{
		char *data = (char *)&hdr[1];

		if (type == SES_PACKET_TYPE_ECHO_REQ)
			hdr->eapol.eap_code = EAP_RESPONSE; 
		else
			hdr->eapol.eap_code = EAP_REQUEST; 

		for(i = 0; i < SES_PACKET_ECHO_LEN; i++)
			data[i] = 2*i;

		ses_payload_len = SES_PACKET_ECHO_LEN;
		break;
		}

	case SES_PACKET_TYPE_STATUS:
		{
		ses_packet_status_t *s = (ses_packet_status_t *)&hdr[1];

		s->status = spep->status;
		hdr->eapol.eap_code = EAP_REQUEST; 

		ses_payload_len = sizeof(ses_packet_status_t);
		break;
		}

	default:
		assert(0);
	}

	/* SES_SPECIFIC_HEADER_LEN 4 */
	hdr->eapol.length = hdr->eapol.eap_length = htons((uint16)(BCM_EAP_EXP_LEN + SES_SPECIFIC_HEADER_LEN + ses_payload_len));

	return ses_send_packet(spep->fd[spep->fd_index], 
			       (uint8 *)hdr, SES_HEADER_LEN + ses_payload_len);
}

static int
ses_open_connections(ses_packet_exchange_t *spe)
{
        ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;
        char name[SES_IFNAME_SIZE], *c;
#ifdef BCMDBG
        char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */
        int fd, i, ret;

        /* build fd list alongwith mac address */
	c = spe->ifnames;
	while (*c != '\0') {
		spep->ifname[spep->num_fd] = c;
		i = 0;
		while ((*c != ' ') && (*c != '\0'))
			name[i++] = *c++;
		name[i] = '\0';
		while (*c == ' ')
			c++;

                ret = ses_open_socket(name, &fd);
                if (ret < 0) {
                        SES_ERROR("ses_open_socket(%s) failed with err %d\n", 
				name, fd);
                        continue;
		}

                if (ses_wl_hwaddr(name, spep->ea[spep->num_fd].octet) < 0) {
                        SES_ERROR("cannot obtain hw address for %s\n", name);
                        return SES_PE_STATUS_INTERNAL_ERROR;
                }

                SES_INFO("%s: hwaddr %s\n", name, 
			ether_etoa((unsigned char *)&spep->ea[spep->num_fd], eabuf));

                spep->fd[spep->num_fd] = fd;

                spep->num_fd++;
        }

        if (spep->num_fd == 0) {
                SES_ERROR("no open sockets\n");
                return SES_PE_STATUS_INTERNAL_ERROR;
        }

        return SES_PE_STATUS_SUCCESS;
}

static int
ses_compute_kek(ses_packet_exchange_t *spe, char *kek, int kek_len)
{
        ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;
	int dh_key_len;
	uchar dh_key_buf[SES_MAX_KEY_LEN];
	SHA1Context sha;
	uint8 Message_Digest[SHA1HashSize];

	assert(kek_len <= SHA1HashSize);

	if (spep->kek_len) {
		assert(spep->kek_len == kek_len);
		memcpy(kek, spep->kek, kek_len);
		return SES_PE_STATUS_SUCCESS;
	}

	/* SES_ERROR("begin compute: %s\n", file2str("/proc/uptime")); */

	dh_key_len = DH_compute_key(dh_key_buf, spep->peer_key,
	                            spep->peer_key_len, spep->dh);

	/* SES_ERROR("end compute: %s\n", file2str("/proc/uptime")); */

	if (!dh_key_len) {
		SES_ERROR("DH_compute_key() failed\n");
		return SES_PE_STATUS_INTERNAL_ERROR;
	}

	if (SHA1Reset(&sha)) {
		SES_ERROR("SHA1Reset() failed\n");
		return SES_PE_STATUS_INTERNAL_ERROR;
	}

	if (SHA1Input(&sha, dh_key_buf, dh_key_len)) {
		SES_ERROR("SHA1Input() failed\n");
		return SES_PE_STATUS_INTERNAL_ERROR;
	}

	if (SHA1Result(&sha, Message_Digest)) {
		SES_ERROR("SHA1Result() failed\n");
		return SES_PE_STATUS_INTERNAL_ERROR;
	}

	memcpy(kek, Message_Digest, kek_len);

	/* save kek */
	memcpy(spep->kek, Message_Digest, kek_len);
	spep->kek_len = kek_len;

	return SES_PE_STATUS_SUCCESS;
}

static int
ses_set_retransmit_timer(ses_packet_exchange_t *spe)
{
	ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;

	SES_INFO("starting timer in state %s\n",
		ses_pe_state_str(spep->pe_state));

	spep->retries = 0;

	return ses_timer_start(&spep->tid, SES_PE_RETRY_INTERVAL, 
			       (ses_timer_cb)ses_retransmit, spe);
}

static void
ses_delete_retransmit_timer(ses_packet_exchange_t *spe)
{
	ses_pe_private_t *spep = (ses_pe_private_t *)spe->private;

        if (spep->tid) {
		SES_INFO("deleting timer in state %s\n", 
			ses_pe_state_str(spep->pe_state));
                ses_timer_stop(spep->tid);
                spep->tid = 0;
        }
}

static void
ses_validate_constants(void)
{
	assert(SES_PACKET_ENCR_OVERHEAD >= AKW_BLOCK_LEN);
	assert(SES_IFNAME_SIZE >= IFNAMSIZ);
}

#ifdef BCMDBG
static char *
ses_pe_state_str(uint state)
{
	switch (state) {
		case SES_WAITING_FOR_HELLO:
			return "SES_WAITING_FOR_HELLO";
		case SES_WAITING_FOR_KEY1:
			return "SES_WAITING_FOR_KEY1";
		case SES_WAITING_FOR_CONFIRM:
			return "SES_WAITING_FOR_CONFIRM";
		case SES_WAITING_FOR_KEY2:
			return "SES_WAITING_FOR_KEY2";
		case SES_WAITING_FOR_INFO:
			return "SES_WAITING_FOR_INFO";
		case SES_COMPLETE:
			return "SES_COMPLETE";
		case SES_WAITING_FOR_ECHO_REQ:
			return "SES_WAITING_FOR_ECHO_REQ";
		case SES_WAITING_FOR_ECHO_RESP:
			return "SES_WAITING_FOR_ECHO_RESP";
		case SES_WAITING_FOR_STATUS:
			return "SES_WAITING_FOR_STATUS";
		case SES_WAITING_FOR_ECHO_STATUS:
			return "SES_WAITING_FOR_ECHO_STATUS";
		default:
			return "UNKNOWN";
	}
}

static char *
ses_ptype_str(uint type)
{
	switch (type) {
		case SES_PACKET_TYPE_STATUS:
			return "SES_PACKET_TYPE_STATUS";
		case SES_PACKET_TYPE_HELLO:
			return "SES_PACKET_TYPE_HELLO";
		case SES_PACKET_TYPE_KEY1:
			return "SES_PACKET_TYPE_KEY1";
		case SES_PACKET_TYPE_KEY2:
			return "SES_PACKET_TYPE_KEY2";
		case SES_PACKET_TYPE_INFO:
			return "SES_PACKET_TYPE_INFO";
		case SES_PACKET_TYPE_ECHO_REQ:
			return "SES_PACKET_TYPE_ECHO_REQ";
		case SES_PACKET_TYPE_ECHO_RESP:
			return "SES_PACKET_TYPE_ECHO_RESP";
		default:
			return "UNKNOWN";
	}
}
#endif
