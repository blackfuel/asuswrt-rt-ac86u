/*
 * Wake-on-Wireless related header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_wowlpf.h 498377 2014-08-22 22:21:57Z $
*/


#ifndef _wlc_wowlpf_h_
#define _wlc_wowlpf_h_

#ifdef WOWLPF

#include <bcmcrypto/aes.h>

#ifdef SECURE_WOWL
#include <bcmcrypto/prf.h>
#include <proto/bcmip.h>
#include <proto/bcmtcp.h>
#include <bcmutils.h>
#endif /* #ifdef SECURE_WOWL */

#define WOWL_TM_DONGLE_DOWN                  1
#define WOWL_TM_PACKET_ACK                   2
#define WOWL_TM_PACKET_KEEPALIVE             3

#define TLS_MAX_KEY_LENGTH	48
#define TLS_MAX_MAC_KEY_LENGTH	32
#define TLS_MAX_IV_LENGTH	32
#define TLS_MAX_DEGIST_LENGTH	32
#define TLS_MAX_SEQUENCE_LENGTH	8

/* add supported cipher suite according rfc5246#appendix-A.5 */
typedef enum {
	TLS_RSA_WITH_AES_128_CBC_SHA     =     0x002F,
	TLS_RSA_WITH_AES_256_CBC_SHA     =     0x0035
} Cipher_Suite_e;
typedef enum {
    CONTENTTYPE_CHANGE_CIPHER_SPEC = 20,
    CONTENTTYPE_ALERT,
    CONTENTTYPE_HANDSHAKE,
    CONTENTTYPE_APPLICATION_DATA
} ContentType_e;
typedef enum {
	COMPRESSIONMETHOD_NULL = 0
} CompressionMethod_e;
typedef enum {
	BULKCIPHERALGORITHM_NULL,
	BULKCIPHERALGORITHM_RC4,
	BULKCIPHERALGORITHM_3DES,
	BULKCIPHERALGORITHM_AES
} BulkCipherAlgorithm_e;
typedef enum {
	CIPHERTYPE_STREAM = 1,
	CIPHERTYPE_BLOCK,
	CIPHERTYPE_AEAD
} CipherType_e;
typedef enum {
	MACALGORITHM_NULL,
	MACALGORITHM_HMAC_MD5,
	MACALGORITHM_HMAC_SHA1,
	MACALGORITHM_HMAC_SHA256,
	MACALGORITHM_HMAC_SHA384,
	MACALGORITHM_HMAC_SHA512
} MACAlgorithm_e;

typedef struct {
	uint8 major;
	uint8 minor;
} ProtocolVersion;

typedef struct {
	ProtocolVersion version;
	CompressionMethod_e compression_algorithm;
	BulkCipherAlgorithm_e cipher_algorithm;
	CipherType_e cipher_type;
	MACAlgorithm_e mac_algorithm;
	uint32 keepalive_interval; /* keepalive interval, in seconds */
	uint8 read_master_key[TLS_MAX_KEY_LENGTH];
	uint32 read_master_key_len;
	uint8 read_iv[TLS_MAX_IV_LENGTH];
	uint32 read_iv_len;
	uint8 read_mac_key[TLS_MAX_MAC_KEY_LENGTH];
	uint32 read_mac_key_len;
	uint8 read_sequence[TLS_MAX_SEQUENCE_LENGTH];
	uint32 read_sequence_len;
	uint8 write_master_key[TLS_MAX_KEY_LENGTH];
	uint32 write_master_key_len;
	uint8 write_iv[TLS_MAX_IV_LENGTH];
	uint32 write_iv_len;
	uint8 write_mac_key[TLS_MAX_MAC_KEY_LENGTH];
	uint32 write_mac_key_len;
	uint8 write_sequence[TLS_MAX_SEQUENCE_LENGTH];
	uint32 write_sequence_len;
	uint32 tcp_ack_num;
	uint32 tcp_seq_num;
	uint8 local_ip[IPV4_ADDR_LEN];
	uint8 remote_ip[IPV4_ADDR_LEN];
	uint16 local_port;
	uint16 remote_port;
	uint8 local_mac_addr[ETHER_ADDR_LEN];
	uint8 remote_mac_addr[ETHER_ADDR_LEN];
	uint32 app_syncid;
} tls_param_info_t;

typedef struct {
	wlc_info_t *wlc;

	uint32 tlsparam_size;
	tls_param_info_t *tlsparam;
	uint32 size_bytes;
	uint8 *mask_and_pattern;
	uint8 block_length;
	uint8 iv_length;
	uint8 explicit_iv_length;
	uint8 digest_length;
	uint8 mac_key_length;
	uint32 read_ks[4 * (AES_MAXROUNDS + 1)];
	uint32 write_ks[4 * (AES_MAXROUNDS + 1)];
} tls_info_t;

#define TLS_RECORD_HEADER_LENGTH		5
#define TLS_RECORD_HEADER_CONTENTTYPE_LENGTH	1
#define TLS_RECORD_HEADER_VERSION_LENGTH	2

#define TLS_OFFSET_CONTENTTYPE			0
#define TLS_OFFSET_VERSION_MAJOR		1
#define TLS_OFFSET_VERSION_MINOR		2
#define TLS_OFFSET_LENGTH_HIGH			3
#define TLS_OFFSET_LENGTH_LOW			4

extern wowlpf_info_t *wlc_wowlpf_attach(wlc_info_t *wlc);
extern void wlc_wowlpf_detach(wowlpf_info_t *wowl);
extern bool wlc_wowlpf_cap(struct wlc_info *wlc);
extern bool wlc_wowlpf_enable(wowlpf_info_t *wowl);
extern uint32 wlc_wowlpf_clear(wowlpf_info_t *wowl);
extern bool wlc_wowlpf_pktfilter_cb(wlc_info_t *wlc,
	uint32 type, uint32 id, const void *patt, const void *sdu);
extern bool wlc_wowlpf_event_cb(wlc_info_t *wlc, uint32 event, uint32 reason);
#endif /* WOWLPF */

/* number of WOWL patterns supported */
#ifndef MAXPATTERNS
#define MAXPATTERNS 8
#endif /* MAXPATTERNS */

#endif /* _wlc_wowlpf_h_ */
