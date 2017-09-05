/*
 * Required functions exported by the wlc_relmcast.c
 * to common (os-independent) driver code.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_relmcast.h 421032 2013-08-30 03:02:09Z $
 */

#ifndef wlc_relmcast_h_
#define wlc_relmcast_h_

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <proto/bcmip.h>
#include <bcmsrom.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <bcmwpa.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wl_export.h>

#ifdef WL_RELMCAST

/* Following are used in external wlc modules */

#define ERMC_NUM_OF_MC_STREAMS		4

#ifdef IBSS_RMC
#define RMC_MAGIC_CODE_LEN	        6		/* sizeof("OXYGEN")  */
#endif

#include<packed_section_start.h>

/*  Transmitter Notification ACT frame format  */
typedef BWL_PRE_PACKED_STRUCT struct rmc_notify_af {
	uint8	category;	        /* 0x7f */
	uint8	oui[DOT11_OUI_LEN];	/* 0x00 00 f0 BCM or ? */

#ifdef IBSS_RMC
	uint8	magic[RMC_MAGIC_CODE_LEN];
	uint8	version;
	uint8	subtype;
	uint32	dialog_counter;
#else
	uint8	type;
	uint8	subtype;		/* action frame subtype */
	uint8	version;		/* 0x01 */
#endif
	/* this field is zero for initiator->transmitter uni act frames */
	/* transmitter will announce leader's mcmac here, 0 - means :	*/
	/* the leader is canceled/ or not selected	*/
	struct ether_addr leader_mac;

#ifdef IBSS_RMC
	uint8   bcm_category;   /* 0x7f */
	uint8   bcm_oui[3];
	uint8   bcm_action;
#endif
	/* act_mcbmp bits will specify which stream(s) to act on */
	/* one bit per stream indication = 1 active, 0-inactive */
	uint8	act_mcbmp;
	struct ether_addr mctable[ERMC_NUM_OF_MC_STREAMS]; /* mcast mac addr */
	uint16  artmo;
} BWL_POST_PACKED_STRUCT rmc_notify_af_t;

#include<packed_section_end.h>
/* -------------------------------------------------------------------------------------- */
#endif /* WL_RELMCAST */

/*
 *  Functional Decscription:
 *
 *  This feature provides acknowledgement for multicasting data packets(data packet only) so
 *  that some specific multicasting data packets can be reliably delivered. Current implementation
 *  can enable the acknowledgement for the multicast data frames by specifying the multicast ip
 *  address. The transmit rate of the reliable multicast frames also can be configured using
 *  IOVARs.
 *  For Tx side, use mcast_ackreq(ACK required) to enable waiting for ACK of multicasting data
 *  packets.
 *  Use mcast_ackip to specify the multicast addresses needs to be ACKed. After these packets
 *  transmitted, waiting for ACK to arrive else the packets will be retransmitted. Which station
 *  sending back ACK is chosen by RSSI value. The lower RSSI station and only one station will
 *  send ACK. The beacon IE includes the information about MAC address of ACK sender and
 *  which multicast address needs to be acked.
 *
 *  For Rx side, parsing the beacon IE to see whether to transmit ACK to the specified multicast
 *  packets. When a rxed multicast packet's MAC address(bottom 23bit) is matched, transmit an
 *  ACK.
 */

/*
 * Function Purpose:
 * Check reliable multicast acknowledgement needed or not for selective multicast packets.
 * PARAMETERS:
 *	relmcastp - multicast structure pointer
 *	type - packet type(data packet only)
 *	h - d11 header pointer needed to check the mac address.
 *	mlcp - mlc value to set waiting for ACK. (OUTPUT)
 *	rspecp, rspec_fallbackp - rate/fallback rate value to set tx rate. (OUTPUT)
 * Return Value:
 *	0 - no match. 1 - multicast packet matched and processed.
 */
int wlc_rmc_process(wlc_rmc_info_t *rmcp, uint16 type, void *p,
	struct dot11_header *h, uint16 *mclp, uint32 *rspecp);

/*
 * Function Purpose:
 * AP or P2P_GO updates the RSSI value of each STA based on the management or control packet's rssi.
 * PARAMETERS:
 *	rmcp - multicast structure pointer
 *	wrxh - packet d11 header pointer
 *	scb - STA control block
 * Return Value:
 *	None.
 */
void wlc_rmc_mgmtctl_rssi_update(wlc_rmc_info_t * rmcp, wlc_d11rxhdr_t *wrxh,
	struct scb *scb, bool flag);

/*
 * Funciton Purpose:
 * It detaches the module. The reliable multicast interface will
 * not be used/necessary. (The methods will/should not be called after
 * the interface is freed)
 * PARAMETERS:
 *  rmc_infop - Pointer to reliable multicast data structure. This pionter will be
 *  initialized by wlc_rmc_attach function .
 * Return value:
 *	None.
 */
void wlc_rmc_detach(wlc_rmc_info_t *rmc);

/*
 * Function Purpose:
 * This function attaches the reliable multicast module.
 * PARAMETERS:
 *	wlc - wlc_info_t pointer.
 * Return value:
 *  on sucess returns a pointer to the rmc_info_t
 *  on failure returns NULL
 */
wlc_rmc_info_t *wlc_rmc_attach(wlc_info_t *wlc);

/*
 * Function Purpose:
 * This function parses the control action frames that
 * is send out by the RMC transmitters
 * PARAMETERS:
 *	wlc - wlc_info_t pointer.
 *	hdr - pointer to dot11_management_header
 *	body
 *	body_len
 *	wrxh - pointer to wlc_d11rxhdr_t
 *
 * Return value:
 *	on sucess returns a pointer to the rmc_info_t
 *	on failure returns NULL
 */
int wlc_rmc_recv_action_frames(wlc_info_t *wlc,
	struct dot11_management_header *hdr, uint8 *body, int body_len, wlc_d11rxhdr_t *wrxh);

/*
 * Function Purpose:
 * This function checks to see whether the RMC multi-cast packet is a duplicate one
 * PARAMETERS:
 *	wlc - wlc_info_t pointer.
 * Return value:
 *	TRUE
 *	FALSE
 */
uint8 wlc_rmc_verify_dup_pkt(wlc_info_t *wlc, struct wlc_frminfo *f);

/*
 * Function Purpose:
 * This function validates whether the received frame is rmc action frame or not
 * PARAMETERS:
 *	wlc - wlc_info_t pointer.
 *	body
 *	body_len
 * Return value:
 *	TRUE
 *	FALSE
 */
bool wlc_rmc_check_actframe(wlc_info_t *wlc, uint8 *body, int body_len);

/*
 * Function Purpose:
 * This function increments mcast tx frame counter
 * PARAMETERS:
 *      wlc - wlc_info_t pointer.
 * Return value: None
 */
void wlc_rmc_tx_frame_inc(wlc_info_t *wlc);

#endif	/* _wlc_relmcast_h_ */
