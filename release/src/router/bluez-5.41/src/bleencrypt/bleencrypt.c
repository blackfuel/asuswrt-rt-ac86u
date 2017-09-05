/*
 *	ble data encrypt
*/

#include "bleencrypt.h"
#include "include/adv_string.h"

#ifdef ENCRYPT
#define SSL_VERSION     (SSLeay_version(SSLEAY_VERSION))
#else
#define SSL_VERSION     ""
#endif

struct cmd_handler_clnt cmd_handlers_clnt[] = {
	{ "reqpublickey",	BLE_COMMAND_REQ_PUBLICKEY,		PackBLECommandReqPublicKey,		UnpackBLEResponseReqPublicKey },
	{ "reqservernonce",	BLE_COMMAND_REQ_SERVERNONCE,		PackBLECommandReqServerNonce,		UnpackBLEResponseReqServerNonce },
	{ "apply",		BLE_COMMAND_APPLY,			PackBLECommandAPPLY,			UnpackBLEResponseOnly },
	{ "reset",		BLE_COMMAND_RESET,			PackBLECommandRESET,			UnpackBLEResponseOnly },
	{ "getwanstatus",	BLE_COMMAND_GET_WAN_STATUS,		PackBLECommandGetWanStatus,		UnpackBLEResponseGetWanStatus },
	{ "getwifistatus",	BLE_COMMAND_GET_WIFI_STATUS,		PackBLECommandGetWifiStatus,		UnpackBLEResponseGetWifiStatus },
	{ "setwantype",		BLE_COMMAND_SET_WAN_TYPE,		PackBLECommandSetWanType,		UnpackBLEResponseOnly },
	{ "setwanpppoename",	BLE_COMMAND_SET_WAN_PPPOE_NAME,		PackBLECommandSetWanPPPoEName,		UnpackBLEResponseOnly },
	{ "setwanpppoepwd",	BLE_COMMAND_SET_WAN_PPPOE_PWD,		PackBLECommandSetWanPPPoEPWD,		UnpackBLEResponseOnly },
	{ "setwanipaddr",	BLE_COMMAND_SET_WAN_IPADDR,		PackBLECommandSetWanIPAddr,		UnpackBLEResponseOnly },
	{ "setwanipsubnetmask",	BLE_COMMAND_SET_WAN_SUBNET_MASK,	PackBLECommandSetWanSubMask,		UnpackBLEResponseOnly },
	{ "setwandefgateway",	BLE_COMMAND_SET_WAN_GATEWAY,		PackBLECommandSetWanGateway,		UnpackBLEResponseOnly },
	{ "setwandns1",		BLE_COMMAND_SET_WAN_DNS1,		PackBLECommandSetWanDns1,		UnpackBLEResponseOnly },
	{ "setwandns2",		BLE_COMMAND_SET_WAN_DNS2,		PackBLECommandSetWanDns2,		UnpackBLEResponseOnly },
	{ "setlanport",		BLE_COMMAND_SET_WAN_PORT,		PackBLECommandSetWanPort,		UnpackBLEResponseOnly },
	{ "setwifiname",	BLE_COMMAND_SET_WIFI_NAME,		PackBLECommandSetWifiName,		UnpackBLEResponseOnly },
	{ "setwifipwd",		BLE_COMMAND_SET_WIFI_PWD,		PackBLECommandSetWifiPWD,		UnpackBLEResponseOnly },
	{ "setgroupid",		BLE_COMMAND_SET_GROUP_ID,		PackBLECommandSetGroupID,		UnpackBLEResponseOnly },
	{ "setadminname",	BLE_COMMAND_SET_ADMIN_NAME,		PackBLECommandSetAdminName,		UnpackBLEResponseOnly },
	{ "setadminpwd",	BLE_COMMAND_SET_ADMIN_PWD,		PackBLECommandSetAdminPWD,		UnpackBLEResponseOnly },
	{ "setuserlocation",	BLE_COMMAND_SET_USER_LOCATION,		PackBLECommandSetUserLocation,		UnpackBLEResponseOnly },
	{ "setuserplace",	BLE_COMMAND_SET_USER_PLACE,		PackBLECommandSetUserPlace,		UnpackBLEResponseOnly },
	{ "setswmode",		BLE_COMMAND_SET_SW_MODE,		PackBLECommandSetSWMode,		UnpackBLEResponseOnly },
	{ "setwandnsenable",	BLE_COMMAND_SET_WAN_DNS_ENABLE,		PackBLECommandSetWanDnsEnable,		UnpackBLEResponseOnly },
	{ "getmacbleversion",	BLE_COMMAND_GET_MAC_BLE_VERSION,	PackBLECommandGetMacBleVersion,		UnpackBLEResponseGetMacBleVersion },
	{ NULL, -1, NULL, NULL }
};

struct cmd_handler_svr cmd_handlers_svr[] = {
	{ "reqpublickey",	BLE_COMMAND_REQ_PUBLICKEY,		UnpackBLECommandReqPublicKey,		PackBLEResponseReqPublicKey },
	{ "reqservernonce",	BLE_COMMAND_REQ_SERVERNONCE,		UnpackBLECommandReqServerNonce,		PackBLEResponseReqServerNonce },
	{ "apply",		BLE_COMMAND_APPLY,			UnpackBLECommandAPPLY,			PackBLEResponseOnly },
	{ "reset",		BLE_COMMAND_RESET,			UnpackBLECommandRESET,			PackBLEResponseOnly },
	{ "getwanstatus",	BLE_COMMAND_GET_WAN_STATUS,		UnpackBLECommandGetWanStatus,		PackBLEResponseGetWanStatus },
	{ "getwifistatus",	BLE_COMMAND_GET_WIFI_STATUS,		UnpackBLECommandGetWifiStatus,		PackBLEResponseGetWifiStatus },
	{ "setwantype",		BLE_COMMAND_SET_WAN_TYPE,		UnpackBLECommandSetWanType,		PackBLEResponseOnly },
	{ "setwanpppoename",	BLE_COMMAND_SET_WAN_PPPOE_NAME,		UnpackBLECommandSetWanPPPoEName,	PackBLEResponseOnly },
	{ "setwanpppoepwd",	BLE_COMMAND_SET_WAN_PPPOE_PWD,		UnpackBLECommandSetWanPPPoEPWD,		PackBLEResponseOnly },
	{ "setwanipaddr",	BLE_COMMAND_SET_WAN_IPADDR,		UnpackBLECommandSetWanIPAddr,		PackBLEResponseOnly },
	{ "setwanipsubnetmask",	BLE_COMMAND_SET_WAN_SUBNET_MASK,	UnpackBLECommandSetWanSubMask,		PackBLEResponseOnly },
	{ "setwandefgateway",	BLE_COMMAND_SET_WAN_GATEWAY,		UnpackBLECommandSetWanGateway,		PackBLEResponseOnly },
	{ "setwandns1",		BLE_COMMAND_SET_WAN_DNS1,		UnpackBLECommandSetWanDns1,		PackBLEResponseOnly },
	{ "setwandns2",		BLE_COMMAND_SET_WAN_DNS2,		UnpackBLECommandSetWanDns2,		PackBLEResponseOnly },
	{ "setlanport",		BLE_COMMAND_SET_WAN_PORT,		UnpackBLECommandSetWanPort,		PackBLEResponseOnly },
	{ "setwifiname",	BLE_COMMAND_SET_WIFI_NAME,		UnpackBLECommandSetWifiName,		PackBLEResponseOnly },
	{ "setwifipwd",		BLE_COMMAND_SET_WIFI_PWD,		UnpackBLECommandSetWifiPWD,		PackBLEResponseOnly },
	{ "setgroupid",		BLE_COMMAND_SET_GROUP_ID,		UnpackBLECommandSetGroupID,		PackBLEResponseOnly },
	{ "setadminname",	BLE_COMMAND_SET_ADMIN_NAME,		UnpackBLECommandSetAdminName,		PackBLEResponseOnly },
	{ "setadminpwd",	BLE_COMMAND_SET_ADMIN_PWD,		UnpackBLECommandSetAdminPWD,		PackBLEResponseOnly },
	{ "setuserlocation",	BLE_COMMAND_SET_USER_LOCATION,		UnpackBLECommandSetUserLocation,	PackBLEResponseOnly },
	{ "setuserplace",	BLE_COMMAND_SET_USER_PLACE,		UnpackBLECommandSetUserPlace,		PackBLEResponseOnly },
	{ "setswmode",		BLE_COMMAND_SET_SW_MODE,		UnpackBLECommandSetSWMode,		PackBLEResponseOnly },
	{ "setwandnsenable",	BLE_COMMAND_SET_WAN_DNS_ENABLE,		UnpackBLECommandSetWanDnsEnable,	PackBLEResponseOnly },
	{ "getmacbleversion",	BLE_COMMAND_GET_MAC_BLE_VERSION,	UnpackBLECommandGetMacBleVersion,	PackBLEResponseGetMacBleVersion },
	{ NULL,			-1,					NULL,					NULL }
};

struct param_handler_svr param_handlers_svr[] = {
	{ BLE_COMMAND_REQ_PUBLICKEY,		NULL,				NULL,				BLE_DATA_TYPE_NULL },
	{ BLE_COMMAND_REQ_SERVERNONCE,		NULL,				NULL,				BLE_DATA_TYPE_NULL },
	{ BLE_COMMAND_APPLY,			NULL,				NULL,				BLE_DATA_TYPE_NULL },
	{ BLE_COMMAND_RESET,			NULL,				NULL,				BLE_DATA_TYPE_NULL },
	{ BLE_COMMAND_GET_WAN_STATUS,		NULL,				NULL,				BLE_DATA_TYPE_NULL },
	{ BLE_COMMAND_GET_WIFI_STATUS,		NULL,				NULL,				BLE_DATA_TYPE_NULL },
	{ BLE_COMMAND_SET_WAN_TYPE,		"wan_proto",			NULL,				BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_SET_WAN_PPPOE_NAME,	"wan_pppoe_username",		"restart_wan_if 0",		BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_SET_WAN_PPPOE_PWD,	"wan_pppoe_passwd",		"restart_wan_if 0",		BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_SET_WAN_IPADDR,		"wan_ipaddr_x",			"restart_wan_if 0",		BLE_DATA_TYPE_IP },
	{ BLE_COMMAND_SET_WAN_SUBNET_MASK,	"wan_netmask_x",		"restart_wan_if 0",		BLE_DATA_TYPE_IP },
	{ BLE_COMMAND_SET_WAN_GATEWAY,		"wan_gateway_x",		"restart_wan_if 0",		BLE_DATA_TYPE_IP },
	{ BLE_COMMAND_SET_WAN_DNS1,		"wan_dns1_x",			"restart_wan_if 0",		BLE_DATA_TYPE_IP },
	{ BLE_COMMAND_SET_WAN_DNS2,		"wan_dns2_x",			"restart_wan_if 0",		BLE_DATA_TYPE_IP },
	{ BLE_COMMAND_SET_WAN_PORT,		"bt_wanport",			NULL,				BLE_DATA_TYPE_INTEGER },
	{ BLE_COMMAND_SET_WIFI_NAME,		"wlc_ssid",			"restart_wireless",		BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_SET_WIFI_PWD,		"wlc_wpa_psk",			"restart_wireless",		BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_SET_GROUP_ID,		"cfg_group",			"restart_cfgsync",		BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_SET_ADMIN_NAME,		"http_username",		"chpass",			BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_SET_ADMIN_PWD,		"http_passwd",			"chpass",			BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_SET_USER_LOCATION,	"cfg_alias",			NULL,				BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_SET_USER_PLACE,		"bt_user_place",		NULL,				BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_SET_SW_MODE,		"sw_mode",			"ble_qis_done",			BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_SET_WAN_DNS_ENABLE,	"wan_dnsenable_x",		"restart_wan_if 0",		BLE_DATA_TYPE_STRING },
	{ BLE_COMMAND_GET_MAC_BLE_VERSION,	NULL,				NULL,				BLE_DATA_TYPE_NULL },
	{ -1,					NULL,				NULL,				-1 }
};

/*
 * @func: Check if the file exists
 * @FileName: search the name of file.
 *
 * @return: 1=exist, 0=no file
 * */
static int fileExists(char *FileName)
{
	int ret = 0;
	FILE *stream = NULL;

	stream = fopen(FileName, "r");
	ret = (stream == NULL) ? 0 : 1;
	if (ret == 1) fclose(stream);
	return ret;
}

void print_data_topic(char *topic, int length)
{
	char str_tmp[MAX_PACKET_SIZE];
	int index;

	memset(str_tmp, '\0', MAX_PACKET_SIZE);
	snprintf(str_tmp, sizeof(str_tmp), "%s[%4s] ", str_tmp, "");

	for(index=0; index<MAX_LE_DATALEN; index++)
			snprintf(str_tmp, sizeof(str_tmp), "%s%2d%c", str_tmp, index, ' ');

	if(GATTDATABASE_DEBUG)
		printf("\n[%s total data length]: %d\n%s\n", topic, length, str_tmp);

	logmessage("BLUEZ", "[%s total data length]: %d\n", topic, length);
	logmessage("BLUEZ", "%s\n", str_tmp);

	return;
}

/*
 * @func: printf uint8 value 
 * @value:
 * @val_len:
 * @row_len:
 * @val_index:
 * @flag:
 *
 * @return:
 * */
void print_data_info(uint8_t *value, int val_len, int row_len, int val_index, int flag)
{
	uint8_t *val_tmp=malloc(MAX_PACKET_SIZE);
	char str_tmp[MAX_PACKET_SIZE];
	int index;

	memset(val_tmp, '\0', val_len);
	memset(str_tmp, '\0', MAX_PACKET_SIZE);
	memcpy(val_tmp, value, val_len);

	if (flag == 1)
		snprintf(str_tmp, sizeof(str_tmp), "Data value:\n%s\n", str_tmp);
	else
		snprintf(str_tmp, sizeof(str_tmp), "[%4d] ", val_index);

	for(index=0; index<val_len; index++) if (flag == 1)
			snprintf(str_tmp, sizeof(str_tmp), "%s%02x%c", str_tmp, val_tmp[index], (((index+1)%row_len)==0 && index!=0)?'\n':' ');
		else
			snprintf(str_tmp, sizeof(str_tmp), "%s%02x%c", str_tmp, val_tmp[index], ' ');

	if(GATTDATABASE_DEBUG)
		printf("%s\n", str_tmp);
	logmessage("BLUEZ", "%s\n", str_tmp);

	free(val_tmp);

	return;
}

/*
 * @func: server and client key init/reset action 
 * @type: Server or Client
 * @action: do Init or Reset
 *
 * @return:
 * */
void ble_key_act(char *type,  char *action)
{
	int typenum = !strcmp(type, "Server")? 1 : !strcmp(type, "Client")? 0: 2;
	int actionnum = !strcmp(action, "Init")? 1 : !strcmp(action, "Reset")? 0: 2;

	if (fileExists(DEFAULT_PUBLIC_PEM_FILE) == 0) {
		printf("Not found file %s \n", DEFAULT_PUBLIC_PEM_FILE);
		goto err;
	}

	if (fileExists(DEFAULT_PRIVATE_PEM_FILE) == 0) {
		printf("Not found file %s]n", DEFAULT_PRIVATE_PEM_FILE);
		goto err;
	}

	switch (typenum) {
	case 0:
		if (actionnum == 0)
			KeyReset_C();
		else if (actionnum == 1)
			KeyInit_C();
		else
			goto err;
	case 1:
		if (actionnum == 0)
			KeyReset_S();
		else if (actionnum == 1)
			KeyInit_S(DEFAULT_PUBLIC_PEM_FILE, DEFAULT_PRIVATE_PEM_FILE);
		else
			goto err;
	}

err:
	return ;
}

int ble_encrypt_clnt(unsigned char *input, unsigned char *output, size_t input_len)
{
	unsigned char *pdu=malloc(MAX_PACKET_SIZE);
	unsigned char *data=malloc(MAX_PACKET_SIZE);
	unsigned int datalen=0;
	int cmdno, status, pdulen;
	struct cmd_handler_clnt *handler;
	int ret;

	memset(pdu, '\0', MAX_PACKET_SIZE);
	memset(data, '\0', MAX_PACKET_SIZE);

	memcpy(pdu, input, input_len);
	pdulen = (int) input_len;

	if(BLEENCRYPT_DEBUG) {
		printf("Clnt Read data \n");
		dumpHEX(pdu, pdulen);
	}

	if((ret=UnpackBLEResponseData(pdu, pdulen, &cmdno, &status, data, &datalen))==BLE_RESULT_OK) {
		for(handler = &cmd_handlers_clnt[0]; handler->command; handler++)
		{
			if(handler->cmdno==cmdno) break;
		}
		if(handler->command!=NULL) {
			handler = &cmd_handlers_clnt[cmdno];
			handler->unpack(data, datalen);
		}
	}

	if(BLEENCRYPT_DEBUG) {
		printf("Clnt Write data \n");
		dumpHEX(pdu, pdulen);
	}

	memcpy(output, pdu, pdulen);
	free(pdu);
	free(data);

	return pdulen;
}

int ble_encrypt_svr(unsigned char *input, unsigned char *output, size_t input_len)
{
	unsigned char *pdu=malloc(MAX_PACKET_SIZE);
	unsigned char *data=malloc(MAX_PACKET_SIZE);
	unsigned int datalen;
	int ret, cmdno, pdulen=0;
	struct cmd_handler_svr *handler;
	struct param_handler_svr *param_hander=param_handlers_svr;

	memset(pdu, '\0', MAX_PACKET_SIZE);
	memset(data, '\0', MAX_PACKET_SIZE);

	pdulen = (int)input_len;
	memcpy(pdu, input, input_len);

	ret=UnpackBLECommandData(pdu, pdulen, &cmdno, data, &datalen);

	if(BLEENCRYPT_DEBUG) {
		printf("o---v\n");
		printf("UnPack info:\n");
		dumpHEX(data, datalen);
	}

	if(ret==BLE_RESULT_OK) {
		for(handler = &cmd_handlers_svr[0]; handler->command; handler++, param_hander++)
			if(handler->cmdno==cmdno) break;

		if(handler->command!=NULL) {
			handler->unpack(param_hander, data, datalen);
			handler->pack(cmdno, BLE_RESULT_OK, (unsigned char *)pdu, &pdulen);
		}
	}
	else { // error
		PackBLEResponseData(cmdno, ret, NULL, 0, (unsigned char *)pdu, &pdulen, BLE_RESPONSE_FLAGS);
	}

	if(BLEENCRYPT_DEBUG) {
		printf("Pack info:\n");
		dumpHEX(pdu, pdulen);
	}

	memcpy(output, pdu, pdulen);
	free(pdu);
	free(data);

	return pdulen;
}
