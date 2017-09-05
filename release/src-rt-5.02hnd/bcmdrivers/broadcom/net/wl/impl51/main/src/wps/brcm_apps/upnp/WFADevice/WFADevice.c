/*
 * Broadcom WPS module (for libupnp), WFADevice.c
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WFADevice.c 574865 2015-07-28 09:45:01Z $
 */
#include <upnp.h>
#include <WFADevice.h>
#include <wps_upnp.h>

extern UPNP_SCBRCHAIN * get_subscriber_chain(UPNP_CONTEXT *context, UPNP_SERVICE *service);

/* Perform the SetSelectedRegistrar action */
int
wfa_SetSelectedRegistrar(UPNP_CONTEXT *context,	UPNP_TLV *NewMessage)
{
	int rc;
	UPNP_WPS_CMD *cmd;

	if (NewMessage->len < 0)
		return SOAP_INVALID_ARGS;

	cmd = (UPNP_WPS_CMD *)calloc(UPNP_WPS_CMD_SIZE + 1 + NewMessage->len, 1);
	if (!cmd)
		return SOAP_OUT_OF_MEMORY;

	/* Cmd */
	cmd->type = UPNP_WPS_TYPE_SSR;
	strncpy((char *)cmd->dst_addr, inet_ntoa(context->dst_addr.sin_addr), sizeof(cmd->dst_addr));
	cmd->length = NewMessage->len;

	/* Data */
	memcpy(cmd->data, NewMessage->val.str, NewMessage->len);

	/* Direct call wps_libupnp_ProcessMsg */
	rc = wps_libupnp_ProcessMsg(context->focus_ifp->ifname, (char *)cmd,
		UPNP_WPS_CMD_SIZE + NewMessage->len);
	free(cmd);
	return 0;
}

/* Perform the PutMessage action */
int
wfa_PutMessage(UPNP_CONTEXT *context, UPNP_TLV *NewInMessage, UPNP_TLV *NewOutMessage)
{
	int rc;
	char *info = "";
	int info_len = 0;
	UPNP_WPS_CMD *cmd;

	if (NewInMessage->len < 0)
		return SOAP_INVALID_ARGS;

	cmd = (UPNP_WPS_CMD *)calloc(UPNP_WPS_CMD_SIZE + 1 + NewInMessage->len, 1);
	if (!cmd)
		return SOAP_OUT_OF_MEMORY;

	/* Cmd */
	cmd->type = UPNP_WPS_TYPE_PMR;
	strncpy((char *)cmd->dst_addr, inet_ntoa(context->dst_addr.sin_addr), sizeof(cmd->dst_addr));
	cmd->length = NewInMessage->len;

	/* Data */
	memcpy(cmd->data, NewInMessage->val.str, NewInMessage->len);

	/* Direct call wps_libupnp_ProcessMsg */
	rc = wps_libupnp_ProcessMsg(context->focus_ifp->ifname, (char *)cmd,
		UPNP_WPS_CMD_SIZE + NewInMessage->len);
	free(cmd);

	/* Always check the out messge len */
	info_len = wps_libupnp_GetOutMsgLen(context->focus_ifp->ifname);
	if (info_len > 0 &&
	    (info = wps_libupnp_GetOutMsg(context->focus_ifp->ifname)) == NULL) {
		info = "";
		info_len = 0;
	}

	upnp_tlv_set_bin(NewOutMessage, (int)info, info_len);
	return 0;
}

/* Perform the GetDeviceInfo action */
int
wfa_GetDeviceInfo(UPNP_CONTEXT *context, UPNP_TLV *NewDeviceInfo)
{
	int rc;
	char *info = "";
	int info_len = 0;
	UPNP_WPS_CMD *cmd;

	cmd = (UPNP_WPS_CMD *)calloc(UPNP_WPS_CMD_SIZE + 1, 1);
	if (!cmd)
		return SOAP_OUT_OF_MEMORY;

	/* Cmd */
	cmd->type = UPNP_WPS_TYPE_GDIR;
	strncpy((char *)cmd->dst_addr, inet_ntoa(context->dst_addr.sin_addr), sizeof(cmd->dst_addr));
	cmd->length = 0;

	/* Direct call wps_libupnp_ProcessMsg */
	rc = wps_libupnp_ProcessMsg(context->focus_ifp->ifname, (char *)cmd,
		UPNP_WPS_CMD_SIZE);
	free(cmd);

	/* Always check the out messge len */
	info_len = wps_libupnp_GetOutMsgLen(context->focus_ifp->ifname);
	if (info_len > 0 &&
	    (info = wps_libupnp_GetOutMsg(context->focus_ifp->ifname)) == NULL) {
		info = "";
		info_len = 0;
	}

	upnp_tlv_set_bin(NewDeviceInfo, (int)info, info_len);
	return 0;
}

/* Perform the PutWLANResponse action */
int
wfa_PutWLANResponse(UPNP_CONTEXT *context, UPNP_TLV *NewMessage)
{
	int rc;
	UPNP_WPS_CMD *cmd;

	if (NewMessage->len < 0)
		return SOAP_INVALID_ARGS;

	cmd = (UPNP_WPS_CMD *)calloc(UPNP_WPS_CMD_SIZE + 1 + NewMessage->len, 1);
	if (!cmd)
		return SOAP_OUT_OF_MEMORY;

	/* Cmd */
	cmd->type = UPNP_WPS_TYPE_PWR;
	strncpy((char *)cmd->dst_addr, inet_ntoa(context->dst_addr.sin_addr), sizeof(cmd->dst_addr));
	cmd->length = NewMessage->len;

	/* Data */
	memcpy(cmd->data, NewMessage->val.str, NewMessage->len);

	/* Direct call wps_libupnp_ProcessMsg */
	rc = wps_libupnp_ProcessMsg(context->focus_ifp->ifname, (char *)cmd,
		UPNP_WPS_CMD_SIZE + NewMessage->len);
	free(cmd);

	return 0;
}

/* Close wfa */
static void
wfa_free(UPNP_CONTEXT *context)
{
	return;
}

/* Open wfa */
int
wfa_init(UPNP_CONTEXT *context)
{
	return 0;
}

/*
 * WARNNING: PLEASE IMPLEMENT YOUR CODES AFTER
 *          "<< USER CODE START >>"
 * AND DON'T REMOVE TAG :
 *          "<< AUTO GENERATED FUNCTION: "
 *          ">> AUTO GENERATED FUNCTION"
 *          "<< USER CODE START >>"
 */

/* << AUTO GENERATED FUNCTION: WFADevice_open() */
int
WFADevice_open(UPNP_CONTEXT *context)
{
	/* << USER CODE START >> */
	int retries;

	/* Setup default gena connect retries for WFA */
	retries = WFADevice.gena_connect_retries;
	if (retries < 1 || retries > 5)
		retries = 3;

	WFADevice.gena_connect_retries = retries;

	if (wfa_init(context) != 0)
		return -1;

	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_close() */
int
WFADevice_close(UPNP_CONTEXT *context)
{
	/* << USER CODE START >> */
	wfa_free(context);
	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_timeout() */
int
WFADevice_timeout(UPNP_CONTEXT *context, time_t now)
{
	/* << USER CODE START >> */
	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_notify() */
int
WFADevice_notify(UPNP_CONTEXT *context, UPNP_SERVICE *service,
	UPNP_SUBSCRIBER *subscriber, int notify)
{
	/* << USER CODE START >> */
	int rc;
	UPNP_WPS_CMD *cmd;

	if (notify == DEVICE_NOTIFY_TIMEOUT ||
	    notify == DEVICE_NOTIFY_UNSUBSCRIBE) {
		cmd = (UPNP_WPS_CMD *)calloc(UPNP_WPS_CMD_SIZE + 1, 1);
		if (!cmd)
			return SOAP_OUT_OF_MEMORY;

		/* Cmd */
		cmd->type = UPNP_WPS_TYPE_DISCONNECT;
		strcpy((char *)cmd->dst_addr, inet_ntoa(subscriber->ipaddr));
		cmd->length = 0;

		/* Direct call wps_libupnp_ProcessMsg */
		rc = wps_libupnp_ProcessMsg(context->focus_ifp->ifname, (char *)cmd,
			UPNP_WPS_CMD_SIZE);
		free(cmd);
	}

	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_scbrchk() */
int
WFADevice_scbrchk(UPNP_CONTEXT *context, UPNP_SERVICE *service,
	UPNP_SUBSCRIBER *subscriber, struct in_addr ipaddr, unsigned short port, char *uri)
{
	/* << USER CODE START >> */

	if (subscriber->ipaddr.s_addr == ipaddr.s_addr && subscriber->port == port)
		return 1;

	return 0;
}
/* >> AUTO GENERATED FUNCTION */
