/*
 * Required external functions and definitions for proximity detection
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_pddefs.h 475037 2014-05-02 23:55:49Z $
 */
#ifndef _wlc_pddefs_h
#define _wlc_pddefs_h

/******************************************************************************************
 * This Header (internal) file contains all the messges and its declacations that are used by
 * proximity detection.
 * The first section contains  RSSI based proximity detection messages.
*******************************************************************************************
*/
#define CHECK_SIGNATURE(obj, value)       ASSERT((obj)->signature == (uint32)(value))
#define ASSIGN_SIGNATURE(obj, value)      ((obj)->signature = value)

/* Singature declarations for service and each methods */
#define WLC_PDSVC_SIGNATURE		0x80706041
#define PDSVC_RSSI_MTHD_SIGNATURE	0x80706042
#define PDSVC_TOF_MTHD_SIGNATURE	0x80706043

#ifdef	TOF_DEBUG
#define PROXD_TRACE(x)			printf x
#define FUNC_ENTER			printf("----------> %s \n", __FUNCTION__)
#define FUNC_EXIT			printf("<---------- %s \n", __FUNCTION__)
#else
#define PROXD_TRACE(x)
#define FUNC_ENTER
#define FUNC_EXIT
#endif


#define FTM_PROTO_REV 1   /*  TOF extended protocol version */

/* PROXD_MAX_METHOD_NUM should be updated when new methtod enum is added */
#define PROXD_MAX_METHOD_NUM PROXD_TOF_METHOD

/* RSSI Proximity method configuration parameters  definition */
#define PROXD_DEFAULT_CHANSPEC		CH80MHZ_CHSPEC(42, WL_CHANSPEC_CTL_SB_UU) /* 44/80 */
#define PROXD_DEFAULT_INTERVAL		100			/* 100 TU */
#define PROXD_DEFAULT_DURATION		10			/* 10 TU */
#define PROXD_DEFAULT_RSSI_THRESH	-30			/* -30 dBm */
#define PROXD_DEFAULT_TX_POWER		14			/* 14 dBm */
#define PROXD_DEFAULT_TX_RATE		12			/* 6 Mbps */
#define PROXD_DEFAULT_TIMEOUT		20			/* 20 Ms */
#define PROXD_DEFAULT_RETRY_CNT		6			/* retry 6 times */
#define PROXD_LOW_RSSI_VALUE		-127			/* lowest rssi */
/* Debug RSSI threshold */
#define DEBUG_RSSI_THERSH		255			/* rssi theshold */

/* TOF proximity specific default params  */
#define PROXD_DEFAULT_FTM_COUNT		6	/* target: num of ftm frames to send */
#define PROXD_DEFAULT_FTM_SEQ_COUNT	1

#define PROXD_NO_GDADJ			0
#define PROXD_GD_NADJ			1

#define PROXD_MEASUREMENT_PKTID		0x80000000
#define PROXD_FTM_PACKET_TAG		((DOT11_ACTION_CAT_UWNM << 24) | PROXD_MEASUREMENT_PKTID |\
						(DOT11_UWNM_ACTION_TIMING_MEASUREMENT << 16))

#ifdef RSSI_REFINE
#define TOF_MAXSCALE			30
#endif

/**********************************************************
 * Function Purpose:
 * Call back interface for proximity detection notificaiton.
 * PARAMETERS:
 * wlc_info_t pointer
 * ether_addr pointer to ethernet addr
 * result
 * status
 * body
 * body_len
 * Return value:
 * If sucess it returns positive else negative.
*************************************************************
*/
typedef int (*notifypd)(void *ctx, struct ether_addr *ea, uint result, uint status,
	uint8 *body, int body_len);


/***************************************************************************************
 * Proximity detection service supplies transmit funciton pointer to rssi pd method, so that,
 * once the action frame is ready it can pass it to this interface. The reason is that the
 * application may transport TLVs in the proxmity action frames. Application can pass the TLV
 * to pdsvc, and pdsvc can copy the TLVs in the action frames.
**************************************************************************************
*/
typedef int (*transmitaf)(wlc_pdsvc_info_t *const pdsvc, wl_action_frame_t *af,
	ratespec_t rate_override, pkcb_fn_t fn, struct ether_addr *addr);

/***************************************************************************************
 * Proximity detection service supplies AVB timer factor the factor = (2 * Divior * 1000000)/VCO.
 * The factor for 4335b0 and 4335c0 is 6.19834710... to keep good accuracy. Left Shift it 15 bit.
 * After calculation, right shift the result 15 bit.
**************************************************************************************
*/
typedef uint32 (*avbtimerfactorf)(wlc_pdsvc_info_t *const pdsvc, uint8 shift, uint32 *ki,
	uint32 *kt);

typedef int (*pdsvc_func_t)(wlc_info_t *wlc, uint8 action, chanspec_t chanspec,
	struct ether_addr *addr, int8 frmcnt, int8 retrycnt, int timeout, uint32 flags);

typedef struct pdsvc_funcs
{
	transmitaf	txaf;		/* action frame transmit function */
	notifypd	notify;		/* result notification function */
	avbtimerfactorf	clock_factor;	/* get avb timer clock factor function */
	void		*notifyptr;	/* notify function pointer */
} pdsvc_funcs_t;

typedef struct pdsvc_payload
{
	uint16	len;
	char *	data;
} pdsvc_payload_t;

pdsvc_func_t wlc_pdsvc_register(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, notifypd notify,
	void *notifyptr, int8 fmtcnt, struct ether_addr *allow_addr, bool setonly, uint32 flags);
int wlc_pdsvc_deregister(wlc_info_t *wlc, pdsvc_func_t funcp);

#endif /* _wlc_pddefs_h */
