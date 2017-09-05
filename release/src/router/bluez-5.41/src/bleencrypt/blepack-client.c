#define __CLIENT__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blepack.h"
#include "utility.h"

// Key Exchange Related function

// KeyReset: Reset myData structure
// KeyInit: read public/private key from file

typedef struct TLV_Header_t
{
        unsigned int len;
} __attribute__((__packed__)) TLV_Header;


KeyData_c keyData_c;

void KeyInit_C()
{
	memset((void *)&keyData_c, 0, sizeof(KeyData_c));
}

void KeyReset_C()
{
        if (!IsNULL_PTR(keyData_c.kp)) MFREE(keyData_c.kp);
        if (!IsNULL_PTR(keyData_c.ku)) MFREE(keyData_c.ku);
        if (!IsNULL_PTR(keyData_c.km)) MFREE(keyData_c.km);
        if (!IsNULL_PTR(keyData_c.ns)) MFREE(keyData_c.ns);
        if (!IsNULL_PTR(keyData_c.nc)) MFREE(keyData_c.nc);
        if (!IsNULL_PTR(keyData_c.ks)) MFREE(keyData_c.ks);
        if (!IsNULL_PTR(keyData_c.iv)) MFREE(keyData_c.iv);

	KeyInit_C();
}

// Client Send Command
//
// PackBLECommandData  
// PackBLECommandReqPublicKey		: Send command to get public key
// PackBLECommandReqServerNonce		: Send command to get nonce
// PackBLECommandGetWanStatus		: Send command to get wan status
// PackBLECommandGetWifiStatus		: Send command to get wifi status
// PackBLECommandGetMacBleVersion	: Send command to get mac & BLE Version
//
// PackBLECommandSetWanPPPoEName	: Send command to set Name of PPPoE
// PackBLECommandSetWanPPPoEPWD		: Send command to set Password of PPPoE
// PackBLECommandSetWanIPAddr		: Send command to set IP address of Wan
// PackBLECommandSetWanSubMask		: Send command to set Subnet mask of Wan
// PackBLECommandSetWanGateway		: Send command to set Gateway of Wan
// PackBLECommandSetWanDns1		: Send command to set Dns1 of Wan
// PackBLECommandSetWanDns2		: Send command to set Dns2 of Wan
// PackBLECommandSetWanPort		: Send command to set Lan port
// PackBLECommandSetWifiName		: Send command to set SSID of Wifi
// PackBLECommandSetWifiPWD		: Send command to set Password of Wifi
// PackBLECommandSetGroupID		: Send command to set Group ID of Device
// PackBLECommandSetAdminName		: Send command to set Name of Admin
// PackBLECommandSetAdminPWD		: Send command to set Password of Admin
// PackBLECommandSetUserLocation	: Send command to set Location of User
// PackBLECommandSetUserPlace		: Send command to set Place of User

// Client Command Response
//
// UnpackBLEResponseData		: Receive status for those status only command
// UnpackBLEResponseOnly		: Receive data 
// UnpackBLEResponseGetWifiStatus	: Send command to get wifi status
// UnpackBLEResponseReqPublicKey	: Receive public key
// UnpackBLEResponseReqServerNonce	: Receive nonce
// UnpackBLEResponseGetMacBleVersion	: 


void PackBLECommandData(int cmdno, unsigned char *data, int datalen, unsigned char *pdu, int *pdulen, int flag)
{
	BLE_CHUNK_T *chunk;
	unsigned char *final_data;
	unsigned char final_cmdno;
	unsigned short offset;
	unsigned short final_pdulen;
	size_t final_datalen;
	unsigned short final_dataleft;
	unsigned short final_chunkcount, i;
	unsigned short final_payloadlen;
	unsigned short size;

	final_cmdno = (unsigned char)cmdno;
	final_data = data;
	final_datalen = datalen;

#ifdef ENCRYPT
	if((flag&BLE_FLAG_WITH_ENCRYPT)&&keyData_c.ks_len) {
		final_data = aes_encrypt(keyData_c.ks, data, datalen, &final_datalen);
		if (final_data==NULL) {
			final_data = data;
			final_datalen = datalen;
		}
		else {
			final_cmdno |= BLE_COMMAND_WITH_ENCRYPT;	
		}
	}
#endif
	offset = 0;
	i = 0;


	final_payloadlen = final_datalen+BLE_COMMAND_CODE_SIZE+BLE_COMMAND_SEQNO_SIZE+BLE_COMMAND_LEN_SIZE+BLE_COMMAND_CSUM_SIZE;

	final_chunkcount = final_payloadlen/BLE_MAX_MTU_SIZE;
	if(final_payloadlen%BLE_MAX_MTU_SIZE) final_chunkcount++;
	final_pdulen = final_payloadlen;

	final_dataleft = final_datalen;

	for(i=0;i<final_chunkcount;i++)
	{
		chunk=(BLE_CHUNK_T *)&pdu[i*BLE_MAX_MTU_SIZE];
		if(i==0) {
	                chunk->u.firstcmd.cmdno = final_cmdno;
                	chunk->u.firstcmd.seqno = 0;
			chunk->u.firstcmd.length = htons((unsigned short)final_pdulen);
			chunk->u.firstcmd.csum = htons(0);
			size = sizeof(chunk->u.firstcmd.chunkdata)>final_dataleft?final_dataleft:sizeof(chunk->u.firstcmd.chunkdata);
			memcpy(chunk->u.firstcmd.chunkdata, final_data+offset, size);
		}
		else {
			size = sizeof(chunk->u.other.chunkdata)>final_dataleft?final_dataleft:sizeof(chunk->u.other.chunkdata);
			memcpy(chunk->u.other.chunkdata, final_data+offset, size);
		}
		offset += size;
	}
	
	*pdulen = final_pdulen;
	chunk=(BLE_CHUNK_T *)pdu;
	chunk->u.firstcmd.csum = htons(Adv_CRC16(pdu, *pdulen));	
	return;
}

void PackBLECommandReqPublicKey(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_REQ_PUBLICKEY, NULL, 0, pdu, pdulen, BLE_COMMAND_FLAGS);
}

void PackBLECommandReqServerNonce(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        unsigned char *PPP = NULL, *P1 = NULL, encode[4098];
	size_t encode_len;
	TLV_Header tlv_hdr;

#ifdef ENCRYPT
        MALLOC(PPP, unsigned char, (sizeof(TLV_Header)+keyData_c.km_len+sizeof(TLV_Header)+keyData_c.nc_len));
        if (IsNULL_PTR(PPP))
        {
                DBG_ERR("Failed to MALLOC() !!!");
                return ;
        }

        P1 = &PPP[0];
        memset(&tlv_hdr, 0, sizeof(tlv_hdr));
        tlv_hdr.len = htonl(keyData_c.km_len);
        memcpy((unsigned char *)P1, (unsigned char *)&tlv_hdr, sizeof(tlv_hdr));
        P1 += sizeof(tlv_hdr);
        memcpy((unsigned char *)P1, (unsigned char *)&keyData_c.km[0], keyData_c.km_len);
        P1 += keyData_c.km_len;

        memset(&tlv_hdr, 0, sizeof(tlv_hdr));
        tlv_hdr.len = htonl(keyData_c.nc_len);
        memcpy((unsigned char *)P1, (unsigned char *)&tlv_hdr, sizeof(tlv_hdr));
        P1 += sizeof(tlv_hdr);
        memcpy((unsigned char *)P1, (unsigned char *)&keyData_c.nc[0], keyData_c.nc_len);

        memset(encode, 0, sizeof(encode));
        encode_len = rsa_encrypt((unsigned char *)&PPP[0], sizeof(TLV_Header)+keyData_c.km_len+sizeof(TLV_Header)+keyData_c.nc_len, keyData_c.ku, keyData_c.ku_len, encode, sizeof(encode), 1 );


        if (encode_len <= 0)
        {
                DBG_ERR("Failed to rsa_encrypt() !!!");
                MFREE(PPP);
        }

        PackBLECommandData(BLE_COMMAND_REQ_SERVERNONCE, encode, encode_len, pdu, pdulen, BLE_COMMAND_FLAGS);
#endif
}

void PackBLECommandAPPLY(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_APPLY, NULL, 0, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandRESET(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_RESET, NULL, 0, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandGetWanStatus(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_GET_WAN_STATUS, NULL, 0, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}

void PackBLECommandGetWifiStatus(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_GET_WIFI_STATUS, NULL, 0, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetWanType(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_WAN_TYPE, data, datalen>=1?1:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetWanPPPoEName(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_WAN_PPPOE_NAME, data, datalen>=64?64:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetWanPPPoEPWD(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_WAN_PPPOE_PWD, data, datalen>=64?64:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetWanIPAddr(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_WAN_IPADDR, data, datalen>=12?12:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetWanSubMask(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_WAN_SUBNET_MASK, data, datalen>=12?12:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetWanGateway(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_WAN_GATEWAY, data, datalen>=12?12:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetWanDns1(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_WAN_DNS1, data, datalen>=8?8:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}

void PackBLECommandSetWanDns2(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_WAN_DNS2, data, datalen>=8?8:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetWanPort(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_WAN_PORT, data, datalen>=1?1:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetWifiName(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_WIFI_NAME, data, datalen>=64?64:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetWifiPWD(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_WIFI_PWD, data, datalen>=64?64:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetGroupID(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_GROUP_ID, data, datalen>=32?32:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetAdminName(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_ADMIN_NAME, data, datalen>=32?32:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetAdminPWD(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_ADMIN_PWD, data, datalen>=32?32:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetUserLocation(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_USER_LOCATION, data, datalen>=32?32:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetUserPlace(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_USER_PLACE, data, datalen>=32?32:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetSWMode(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_SW_MODE, data, datalen>=1?1:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandSetWanDnsEnable(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_SET_SW_MODE, data, datalen>=1?1:datalen, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}
void PackBLECommandGetMacBleVersion(unsigned char *data, int datalen, unsigned char *pdu, int *pdulen)
{
        PackBLECommandData(BLE_COMMAND_GET_MAC_BLE_VERSION, NULL, 0, pdu, pdulen, BLE_COMMAND_FLAGS|BLE_FLAG_WITH_ENCRYPT);
}

int UnpackBLEResponseData(unsigned char *pdu, int pdulen, int *cmdno, int *status, unsigned char *data, unsigned int *datalen)
{
	BLE_CHUNK_T *chunk;

        unsigned char payload[MAX_PACKET_SIZE];
	unsigned char *final_data;
	unsigned short payloadlen;
	unsigned short payloadleft;
	unsigned short crc16;
	unsigned short chunkcount;
	unsigned short i;
	unsigned short size, offset;
	size_t final_datalen;

        chunk=(BLE_CHUNK_T *)pdu;
	crc16 = ntohs(chunk->u.firstres.csum);
      	chunk->u.firstres.csum = 0;

        if(Adv_CRC16(pdu, pdulen)!=crc16)
                return BLE_RESULT_CHECKSUM_INVALID;

        offset = 0;

        chunkcount = pdulen/BLE_MAX_MTU_SIZE;
	if((pdulen%BLE_MAX_MTU_SIZE)!=0)
		chunkcount++;

	payloadlen = pdulen - BLE_COMMAND_CODE_SIZE - BLE_COMMAND_SEQNO_SIZE - BLE_COMMAND_LEN_SIZE - BLE_COMMAND_STATUS_SIZE - BLE_COMMAND_CSUM_SIZE;
	payloadleft = payloadlen;

	for(i=0;i<chunkcount;i++)
	{
		chunk=(BLE_CHUNK_T *)&pdu[i*BLE_MAX_MTU_SIZE];
                if(i==0) {
			*cmdno = chunk->u.firstres.cmdno;
			*status = chunk->u.firstres.status;
                        size = sizeof(chunk->u.firstres.chunkdata)>payloadleft?payloadleft:sizeof(chunk->u.firstres.chunkdata);
                        memcpy(payload+offset, chunk->u.firstres.chunkdata, size);
                        offset += size;
			payloadleft -= size;
                }
                else {
			size = sizeof(chunk->u.other.chunkdata)>payloadleft?payloadleft:sizeof(chunk->u.other.chunkdata);

                        memcpy(payload+offset, chunk->u.other.chunkdata, size);
                        offset += size;
			payloadleft -= size;
                }
        }

#ifdef ENCRYPT
        if(*cmdno&BLE_COMMAND_WITH_ENCRYPT) {
             final_data = aes_decrypt(keyData_c.ks, payload, payloadlen, &final_datalen);
             if(!final_data) return BLE_RESULT_KEY_INVALID;
        
	     *datalen = (unsigned int)final_datalen;
	     memcpy(data, final_data, final_datalen);	
	}
	else 
#endif	
	{
	     *datalen = offset;
	     memcpy(data, payload, *datalen);	
	}

	*cmdno = *cmdno&(~BLE_COMMAND_WITH_ENCRYPT);	

        return (BLE_RESULT_OK);
}

void UnpackBLEResponseOnly(unsigned char *data, int datalen)
{
}

/*
typedef struct SiteSurvey_t
{            
        unsigned char rssi;
        unsigned char securitymode;
        unsigned char ssid_len; 
        char ssid[];
} SiteSurvey; 

void UnpackBLEResponseSiteSurvey(unsigned char *data, int datalen)
{
	char ssid[33];
	int offset;
	SiteSurvey *s;

	offset = 0;
	s=(SiteSurvey *)data;
	
	while(1) {
		if(offset>datalen) break;
		if(s->rssi==0&&s->securitymode==0&&s->ssid_len==0) break;
		memcpy(ssid, s->ssid, s->ssid_len);
		ssid[s->ssid_len]=0;
		printf("rssi: %d, security mode: %d, ssid(%d): %s\n", s->rssi, s->securitymode, s->ssid_len, ssid); 
		offset+=s->ssid_len+3;
		s=(SiteSurvey *)(data+offset);
	} 
}
*/

void UnpackBLEResponseGetWanStatus(unsigned char *data, int datalen)
{
}
void UnpackBLEResponseGetWifiStatus(unsigned char *data, int datalen)
{
}
void UnpackBLEResponseGetMacBleVersion(unsigned char *data, int datalen)
{
}

void UnpackBLEResponseReqPublicKey(unsigned char *data, int datalen)
{
#ifdef ENCRYPT 
	// get public key and generate master key and client nonce 
        keyData_c.ku_len = datalen;
        MALLOC(keyData_c.ku, unsigned char, datalen);
        if (IsNULL_PTR(keyData_c.ku))
        {
                DBG_ERR("Failed to MALLOC() !!!");
                return;
        }

        memcpy((unsigned char *)&keyData_c.ku[0], (unsigned char *)data, datalen);

        keyData_c.km = gen_rand((size_t *)&keyData_c.km_len);
        if (IsNULL_PTR(keyData_c.km))
        {
                DBG_ERR("Failed to gen_rand() !!!");
                return;
        }


        keyData_c.nc = gen_rand((size_t *)&keyData_c.nc_len);


        if (IsNULL_PTR(keyData_c.nc))
        {
                DBG_ERR("Failed to gen_rand() !!!");
                return;
        }
#endif
        return;
}

void UnpackBLEResponseReqServerNonce(unsigned char *data, int datalen)
{
        unsigned char *P1 = NULL, *dec = NULL;
	int dec_len=0;
	TLV_Header tlv_hdr;
	int tlv_len;

#ifdef ENCRYPT
        dec = aes_decrypt(keyData_c.km, data, datalen, (size_t *)&dec_len);

        if (IsNULL_PTR(dec))
        {
        	DBG_ERR("Failed to aes_decrypt() !!!");
        	return;
	}

        P1 = (unsigned char *)&dec[0];
        memset(&tlv_hdr, 0, sizeof(tlv_hdr));
        memcpy(&tlv_hdr, P1, sizeof(tlv_hdr));
	
	tlv_len = ntohl(tlv_hdr.len);
        if (tlv_len <= 0)
        {
                DBG_ERR("Parsing data error !!!");
                MFREE(dec);
		return;
	}
        
        P1 += sizeof(TLV_Header);
        dec_len -= sizeof(TLV_Header);

        if (tlv_len > dec_len)
        {
                DBG_ERR("Parsing data error !!!");
                MFREE(dec);
		return;
        }

        keyData_c.ns_len = tlv_len;
        MALLOC(keyData_c.ns, unsigned char, keyData_c.ns_len);

        if (IsNULL_PTR(keyData_c.ns))
                                {
                DBG_ERR("Failed to MALLOC() !!!");
        	MFREE(dec);
		return;
	}

        memcpy((unsigned char *)&keyData_c.ns[0], (unsigned char *)P1, keyData_c.ns_len);
        P1 += tlv_len;
        dec_len -= tlv_len;

        if (sizeof(TLV_Header) > (unsigned int)dec_len)
        {
                DBG_ERR("Parsing data error !!!");
                MFREE(dec);
		return;
	}
        
	memset(&tlv_hdr, 0, sizeof(tlv_hdr));
        memcpy(&tlv_hdr, P1, sizeof(tlv_hdr));
        
	tlv_len = ntohl(tlv_hdr.len);

        if (tlv_len <= 0)
        {
                DBG_ERR("Parsing data error !!!");
                MFREE(dec);
        	return;
	}

        P1 += sizeof(TLV_Header);
        dec_len -= sizeof(TLV_Header);

        if (tlv_len > dec_len)
        {
                DBG_ERR("Parsing data error !!!");
                MFREE(dec);
		return;
        }

        if (memcmp((unsigned char *)P1, (unsigned char *)&keyData_c.nc[0], keyData_c.nc_len) != 0)
        {
                DBG_ERR("Error on varify client nonce !!!");
                MFREE(dec);
		return;
        }
                                
        keyData_c.ks = gen_session_key(keyData_c.km, keyData_c.km_len, keyData_c.ns, keyData_c.ns_len, keyData_c.nc, keyData_c.nc_len, (size_t *)&keyData_c.ks_len);

        if (IsNULL_PTR(keyData_c.ks))
        {
                DBG_ERR("Failed to gen_session_key() !!!");
                MFREE(dec);
		return;
        }
#endif
}

