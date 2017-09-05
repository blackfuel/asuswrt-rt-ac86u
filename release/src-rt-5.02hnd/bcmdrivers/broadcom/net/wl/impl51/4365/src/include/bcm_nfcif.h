/*
 * API's to access NFC chip connected to WIFI module
 * through debub UART
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: $
 */

#ifndef	_BCM_NFCIF_H_
#define _BCM_NFCIF_H_

#define NFC_MAX_XFER_SIZE      1

#define	SLIP_HDR_ACK_PKT	0x0
#define	SLIP_HDR_WIFI_PKT	0x1
#define	SLIP_HDR_HOST_PKT	0x2
#define	SLIP_HDR_LNK_CTRL_PKT	0x3

typedef void (*rx_cb_t)(void *ctx, const char *buf, uint16 len);

void bcm_nfcif_init(rx_cb_t rx_cb, void *param);
void bcm_nfcif_send_host_cmd(char *buf, uint16 len);
int bcm_nfcif_send_to_nfc(char *buf, uint16 len, uint8 pkt_type);
uint8 bcm_nfcif_rx(char *buf, uint8 count);

#endif /* _BCM_NFCIF_H_ */
