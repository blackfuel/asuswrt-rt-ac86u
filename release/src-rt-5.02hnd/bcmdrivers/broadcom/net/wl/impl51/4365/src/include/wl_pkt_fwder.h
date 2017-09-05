/*
 * This defines an interface between the WLAN driver and clients which receive/transmit
 * native packets via the WLAN driver.
 * This interface provides APIs :
 *   - To send packets to the WLAN driver.
 *   - For clients like WPS, CnxAPI or TCP/IP stack to register their callbacks to receive frames.
 *   - Init pkt_forwarder registers callbacks with the WLAN driver.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: wl_pkt_fwder.h 241182 2011-02-17 21:50:03Z $
 */


#ifndef wl_pkt_fwder_h
#define wl_pkt_fwder_h

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Include Files ---------------------------------------------------- */

#include "wl_drv.h"

/* ---- Constants and Types ---------------------------------------------- */
#define WL_PKT_FWD_MAX_CLIENTS 5

typedef enum _wl_pkt_fwd_client_pri {
/* Clients priority needs to be appended if max clients is changed */
					WL_PKT_FWD_CLIENT_PRI_0 = 0,
					WL_PKT_FWD_CLIENT_PRI_1 = 1,
					WL_PKT_FWD_CLIENT_PRI_2 = 2,
					WL_PKT_FWD_CLIENT_PRI_3 = 3,
					WL_PKT_FWD_CLIENT_PRI_4 = 4
} wl_pkt_fwd_client_pri;

/* Packet forwarder handle. */
typedef void* wl_pkt_fwd_hdl;

/* Client  handle for Packet forwarder */
typedef void* wl_pkt_fwd_client_hdl;

/****************************************************************************
* Function:   wl_pkt_fwd_init
*
* Purpose:    This api intializes the packet forwarder by registering the pkt_fwd rx
*             callbacks with the wlan driver as per the wl_drv.h (wl_drv_netif_callbacks_t).
*
* Parameters: hdl (in) WLAN driver handle.
*
* Returns:    Packet forwarder handle.
*****************************************************************************
*/
wl_pkt_fwd_hdl wl_pkt_fwd_init(wl_drv_hdl hdl);


/****************************************************************************
* Function:   wl_pkt_fwd_deinit
*
* Purpose:    De-initialize packet forwarder.
*
* Parameters: hdl (in) Packet forwarder handle.
*
* Returns:    Nothing.
*****************************************************************************
*/
void wl_pkt_fwd_deinit(wl_pkt_fwd_hdl hdl);

/****************************************************************************
* Function:   wl_pkt_fwd_get_handle
*
* Purpose:    To provide Packet Forwarder handle
*
* Parameters: Nothing.
*
* Returns:     hdl (out) Packet forwarder handle.
*****************************************************************************
*/
wl_pkt_fwd_hdl wl_pkt_fwd_get_handle(void);


/****************************************************************************
* Function:   wl_pkt_fwd_register_netif
*
* Purpose:    Register callbacks for processing the rx frame of the client.
*
* Parameters: hdl       (in) Packet forwarder handle.
*             callbacks (in) Client network interface callbacks.
*
* Returns:   client_hdl for the packet forwarder
*****************************************************************************
*/
wl_pkt_fwd_client_hdl wl_pkt_fwd_register_netif(wl_pkt_fwd_hdl hdl,
                                wl_drv_netif_callbacks_t *callbacks,
                                wl_pkt_fwd_client_pri pri);


/****************************************************************************
* Function:   wl_pkt_fwd_unregister_netif
*
* Purpose:    Unregisters callbacks for processing the rx frame for the corresponding client.
*
* Parameters: hdl       (in) Packet forwarder handle.
*             client_hdl (in) Client hdl to unregister.
*
* Returns:    0 on success, else -1.
*****************************************************************************
*/
int wl_pkt_fwd_unregister_netif(wl_pkt_fwd_hdl hdl, wl_pkt_fwd_client_hdl client_hdl);


/****************************************************************************
* Function:   wl_pkt_fwd_tx
*
* Purpose:    Transmit packet from network interface to driver.
*
* Parameters: hdl       (mod) Packet forwarder handle.
*             client_hdl (in)  Client hdl.
*             pkt       (in)  Network interface packet. The packet format is
*                             network interface stack-specific.
*             len       (in)  Length of packet to transmit in bytes.
*
* Returns:	  0 on success, else error code.
*****************************************************************************
*/
int wl_pkt_fwd_tx(wl_pkt_fwd_hdl hdl, wl_pkt_fwd_client_hdl client_hdl,
                                                wl_drv_netif_pkt pkt, unsigned int len);


#ifdef __cplusplus
	}
#endif

#endif  /* wl_pkt_fwder_h  */
