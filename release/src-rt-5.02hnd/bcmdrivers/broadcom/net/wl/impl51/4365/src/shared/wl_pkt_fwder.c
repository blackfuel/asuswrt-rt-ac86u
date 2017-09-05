/*
 * WL Packet Forwarder Interface definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: wl_pkt_fwder.c 473326 2014-04-29 00:37:35Z $
 */

/* ---- Include Files ---------------------------------------------------- */

#include "typedefs.h"
#include "wl_pkt_fwder.h"

/* Map between public WL driver handle and WL Pkt handle info struct. */
#define WL_PKT_FWD_HDL_TO_WL_PKT_FWD_INFO(hdl) ((wl_pkt_fwd_info *) (hdl))
#define WL_PKT_FWD_INFO_TO_WL_PKT_FWD_HDL(hdl) ((wl_pkt_fwd_hdl) (&hdl))

/* Packet forwarder cb handle */
typedef struct wl_pkt_fwd_client_cb {
	wl_pkt_fwd_client_pri priority;
	wl_drv_netif_callbacks_t	callbacks;
	bool inUse;
} wl_pkt_fwd_client_cb_t;

typedef struct _wl_pkt_fwd_info {
	wl_drv_hdl	drv_handle;
	wl_pkt_fwd_client_cb_t *list;
} wl_pkt_fwd_info;

static wl_pkt_fwd_client_cb_t g_Client_cblist[WL_PKT_FWD_MAX_CLIENTS];

static wl_pkt_fwd_info g_Pkt_fwd_info;

/* ----------------------------------------------------------------------- */
static wl_pkt_fwd_client_cb_t *
wl_client_cb_get(wl_drv_netif_callbacks_t *callbacks, wl_pkt_fwd_client_pri pri)
{
	wl_pkt_fwd_client_cb_t *client;

	if (pri >= WL_PKT_FWD_MAX_CLIENTS)
		return NULL; /* Priority can't be more than or equal to max Clients */

	client = &(g_Client_cblist[pri]);


	if (client->inUse)
		return NULL; /* Already a client with same priority registered */

	memset((void *)client, 0, sizeof(wl_pkt_fwd_client_cb_t));

	client->priority = pri;
	client->callbacks = *callbacks;
	client->inUse = 1;

	return client;
}

/* ----------------------------------------------------------------------- */
static void
wl_client_cb_free(wl_pkt_fwd_client_cb_t  *client)
{
	if (client->inUse)
		memset((void *)client, 0, sizeof(wl_pkt_fwd_client_cb_t));

}

/* ----------------------------------------------------------------------- */
int
wl_pkt_fwd_process_rx_pkt(wl_drv_netif_pkt pkt, unsigned int len)
{
	wl_pkt_fwd_client_cb_t *client = g_Client_cblist;
	wl_drv_netif_callbacks_t *cb;
	int     index = 0;


	for (; index < WL_PKT_FWD_MAX_CLIENTS; client++, index++) {

		if (!client->inUse)
			continue;

		cb = &(client->callbacks);

		if (!cb->rx_pkt)
			continue;

		/* If a client absorbs the packet it would return 0/success */
		if (!cb->rx_pkt(pkt, len))
			return 0;

	}

	return -1; /* None of the client has absorbed the Rx Packet */
}

/* ----------------------------------------------------------------------- */
int
wl_pkt_fwd_process_start_queue(void)
{

	wl_pkt_fwd_client_cb_t *client = g_Client_cblist;
	wl_drv_netif_callbacks_t *cb;
	int 	index = 0;


	for (; index < WL_PKT_FWD_MAX_CLIENTS; client++, index++) {

		if (!client->inUse)
			continue;

		cb = &(client->callbacks);

		if (cb->start_queue != NULL)
			cb->start_queue();
	}

	return 0;

}


/* ----------------------------------------------------------------------- */
int
wl_pkt_fwd_process_stop_queue(void)
{

	wl_pkt_fwd_client_cb_t *client = g_Client_cblist;
	wl_drv_netif_callbacks_t *cb;
	int 	index = 0;


	for (; index < WL_PKT_FWD_MAX_CLIENTS; client++, index++) {

		if (!client->inUse)
			continue;

		cb = &(client->callbacks);

		if (cb->stop_queue != NULL)
			cb->stop_queue();
	}

	return 0;

}


/* ----------------------------------------------------------------------- */
wl_pkt_fwd_hdl wl_pkt_fwd_init(wl_drv_hdl hdl)
{
	wl_drv_netif_callbacks_t netif_callbacks;

	if (!hdl)
		return NULL;

	if (g_Pkt_fwd_info.drv_handle) /* Packet Fwder is already initialised */
		return WL_PKT_FWD_INFO_TO_WL_PKT_FWD_HDL(g_Pkt_fwd_info);

	memset((void *)&netif_callbacks, 0, sizeof(wl_drv_netif_callbacks_t));

	/* Register network interface callbacks. */
	netif_callbacks.rx_pkt        = wl_pkt_fwd_process_rx_pkt;
	netif_callbacks.start_queue   = wl_pkt_fwd_process_start_queue;
	netif_callbacks.stop_queue    = wl_pkt_fwd_process_stop_queue;

	wl_drv_register_netif_callbacks(hdl, &netif_callbacks);

	memset((void *)&g_Pkt_fwd_info, 0, sizeof(g_Pkt_fwd_info));
	memset((void *)&g_Client_cblist, 0, sizeof(g_Client_cblist));

	g_Pkt_fwd_info.drv_handle = hdl;
	g_Pkt_fwd_info.list = g_Client_cblist;

	return WL_PKT_FWD_INFO_TO_WL_PKT_FWD_HDL(g_Pkt_fwd_info);
}


/* ----------------------------------------------------------------------- */
void wl_pkt_fwd_deinit(wl_pkt_fwd_hdl hdl)
{
	wl_pkt_fwd_info *info;

	if (hdl) {
		info = WL_PKT_FWD_HDL_TO_WL_PKT_FWD_INFO(hdl);
		memset((void *)&g_Pkt_fwd_info, 0, sizeof(g_Pkt_fwd_info));
		memset((void *)&g_Client_cblist, 0, sizeof(g_Client_cblist));
	}

}

/* ----------------------------------------------------------------------- */
wl_pkt_fwd_hdl wl_pkt_fwd_get_handle()
{
	return WL_PKT_FWD_INFO_TO_WL_PKT_FWD_HDL(g_Pkt_fwd_info);
}


/* ----------------------------------------------------------------------- */
wl_pkt_fwd_client_hdl wl_pkt_fwd_register_netif(wl_pkt_fwd_hdl hdl,
                                            wl_drv_netif_callbacks_t *callbacks,
                                            wl_pkt_fwd_client_pri pri)
{
	wl_pkt_fwd_client_cb_t *cb;


	if (!hdl)
		return NULL;

	cb = wl_client_cb_get(callbacks, pri);

	return ((wl_pkt_fwd_client_hdl) cb);

}

/* ----------------------------------------------------------------------- */
int wl_pkt_fwd_unregister_netif(wl_pkt_fwd_hdl hdl, wl_pkt_fwd_client_hdl client_hdl)
{
	wl_pkt_fwd_client_cb_t *cb = (wl_pkt_fwd_client_cb_t *)client_hdl;
	int status = -1;

	if (!hdl)
		return status;

	if (cb) {
		wl_client_cb_free(cb);
		status = 0;
	}

	return status;

}


/* ----------------------------------------------------------------------- */
int wl_pkt_fwd_tx(wl_pkt_fwd_hdl hdl, wl_pkt_fwd_client_hdl client_hdl,
                          wl_drv_netif_pkt pkt, unsigned int len)
{
	wl_pkt_fwd_info *info = WL_PKT_FWD_HDL_TO_WL_PKT_FWD_INFO(hdl);
	int status = -1;


	if (!client_hdl || !info)
		return status;

	/* Length of the packet cant be 0 */
	ASSERT(len);

	return wl_drv_tx_pkt(info->drv_handle, pkt, len);

}
