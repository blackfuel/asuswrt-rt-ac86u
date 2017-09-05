/*
 * Required functions exported by the wlc_pdsvc.c
 * to common driver code
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_pdsvc.h 491282 2014-07-15 19:26:17Z $
 */

/********************************************************************
* Funcitonal Specification:
* pdsvc (proximity detection service) is a frond end interface to wlc layer. The service
* creates a particular proximity detection method based on WL IOVAR configuration.
* Each method has interfaces that are defined in wlc_pdmthd.h. Each method has an
* object which abstracts the implementation from the service layer.
*********************************************************************
*/

#ifndef _wlc_pdscv_h
#define _wlc_pdsvc_h

#include <typedefs.h>
#include <wlc_types.h>

/************************************************************
 * Function Purpose:
 * This funciton attaches the Proximity detection service module to wlc.
 * It reserves space for wlc_pdsvc_info_t.
 * This function is common across all proximity detection objects.
 * PARAMETERS:
 * wlc_info_t pointer
 * Return value:
 *  on Sucess: It returns a pointer to the wlc_pdsvc_info_t.
 *  on Failure: It returns NULL.
**************************************************************
*/
extern wlc_pdsvc_info_t *wlc_proxd_attach(wlc_info_t *wlc);

/*************************************************************
 * Function Purpose:
 * This funciton detaches the Proximity detection  service module from wlc.
 * The pdsvc interfaces will not be called after
 * the detach.
 * PARAMETERS:
 * wlc_pdsvc_info_t pointer to wlc structure
 * Return value: positive integer(>0) on success, and 0 on any internal errors.
**************************************************************
*/
extern int wlc_proxd_detach(wlc_pdsvc_info_t *const pdsvc);

/**********************************************************
 * Function Purpose:
 * This funciton process the action frame from the network.
 * PARAMETERS:
 * wlc_info_t pointer
 * dot11_management_header pointer
 * uint8 pointer to the action frame body
 * int action frame length.
 * wlc_d11rxhdr_t pointer to d11 header
 * Return value:
 * None.
************************************************************
*/
int
wlc_proxd_recv_action_frames(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len,
	wlc_d11rxhdr_t *wrxh, uint32 rspec);


/******************************************************
 * Function Purpose:
 * This function configures the transmit power of proximity frames.
 * PARAMETERS:
 * wlc_info_t pointer to wlc
 * wlc_bsscfg_t pointer to bsscfg
 * txpwr_offset pointer to txpower offset
 * Return value:
*******************************************************
 */
extern int
wlc_proxd_txpwr_offset(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint16 fc, int8 *txpwr_offset);

/******************************************************
 * Function Purpose:
 * This function configures mac control / phy control  used for TOF measurement pkts
 * PARAMETERS:
 * wlc_info_t pointer to wlc
 * phyctl pointer to phyctl
 * mch pointer to upper 16 bits of mac control word
 * pkttag pointer to pkttag struct
 * Return value:
*******************************************************
 */
extern void
wlc_proxd_tx_conf(wlc_info_t *wlc, uint16 *phyctl, uint16 *mch, wlc_pkttag_t *pkttag);

/* payload size of proximity detection action frame */
#define PROXD_AF_FIXED_LEN	20

/******************************************************
 * Function Purpose:
 * This function to determine if proxd is supported or not
 * PARAMETERS:
 * wlc_info_t pointer to wlc
 * Return value: returns TRUE if supported
*******************************************************
*/
extern bool wlc_is_proxd_supported(wlc_info_t *wlc);

#endif /* _wlc_pdscv_h */
