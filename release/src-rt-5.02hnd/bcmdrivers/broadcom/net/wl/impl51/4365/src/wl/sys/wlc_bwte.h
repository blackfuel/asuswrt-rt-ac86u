/*
* BT WLAN TUNNEL ENGINE public header file
*
* Broadcom Proprietary and Confidential. Copyright (C) 2016,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: wlc_bwte.h 467328 2014-04-03 01:23:40Z $
*
*/
#ifndef _WLC_BWTE_H_
#define _WLC_BWTE_H_

typedef enum {
	WLC_BWTE_CLIENT_TBOW = 0
} wlc_bwte_client_t;

typedef enum {
	WLC_BWTE_CTL_MSG = 0,
	WLC_BWTE_LO_DATA = 1,
	WLC_BWTE_HI_DATA = 2,
	WLC_BWTE_PAYLOAD_CNT
} wlc_bwte_payload_t;

typedef int (*wlc_bwte_cb)(void* arg, uchar* p, int len);

void* wlc_bwte_attach(wlc_info_t* wlc);
void wlc_bwte_detach(bwte_info_t *bwte_info);

/* Register/Unregister client to BT WLAN TUNNEL ENGINE
*
*  To register, client need provide valid client id and proper callback function pointer.
*  BWTE support 3 type payload: control message, low priority data and high priority data.
*  Client can selectively register the callback function, but at least one.
*
*  When the provided callback get called, client need process the provided buffer synchronously
*  and give back the ownership of the buffer when exit from the callback function.
*
*  bwte_info - bwte module context pointer
*  client_id - pre-assigned unique id
*	       shared with corresponding WLAN and BT module for this client
*  f_ctl - callback function pointer for control message
*  f_lo_data - callback function pointer for low priority data
*  f_hi_data - callback function pointer for high priority data
*  arg - argument will be passed in callback function
*
*  return - bcm error return
*/
int wlc_bwte_register_client(bwte_info_t *bwte_info, int client_id, wlc_bwte_cb f_ctl,
	wlc_bwte_cb f_lo_data, wlc_bwte_cb f_hi_data, void* arg);
void wlc_bwte_unregister_client(bwte_info_t *bwte_info, int client_id);

/* Send msg/data to BT module through the tunnel
*
*  bwte_info - bwte module context pointer
*  client_id - pre-assigned unique id
*	       shared with corresponding WLAN and BT module for this client
*  payload - payload type
*  buf - pointer to buffer containing control message/low_high priority data.
*	 Clien will give up ownership from succeed return of this function
*        until free callback function get called.
*  len - lenght of provided buffer
*  free_func - callback function pointer to return owner ship of provided buffer
*  arg - argument will be passed in callback function
*
*  return - bcm error return
*/
int wlc_bwte_send(bwte_info_t *bwte_info, int client_id, wlc_bwte_payload_t payload, uchar* buf,
	int len, wlc_bwte_cb free_func, void* arg);


/* Help function for client, optional for most clients */

/* Manually inovke BT->Wlan ISR
*
*  Sometimes ISR processing may be skipped because of client ia in special condition.
*  After the special condition cleared, client may want to manually trigger ISR processing.
*
*  bwte_info - bwte module context pointer
*  client_id - pre-assigned unique id
*	       shared with corresponding WLAN and BT module for this client
*/
void wlc_bwte_process_bt_intr(bwte_info_t *bwte_info, int client_id);

/* Manually reclaim wlan buf from BWTE
*
*  Sometimes client may want to force reclaim all wlan buffer.
*
*  bwte_info - bwte module context pointer
*  client_id - pre-assigned unique id
*	       shared with corresponding WLAN and BT module for this client
*  payload - payload type
*  cleanup - indication if need force reclaim ownership of wlan buffer
*/
void wlc_bwte_reclaim_wlan_buf(bwte_info_t *bwte_info, int client_id, wlc_bwte_payload_t payload,
	bool cleanup);
#endif /* _WLC_BWTE_H_ */
