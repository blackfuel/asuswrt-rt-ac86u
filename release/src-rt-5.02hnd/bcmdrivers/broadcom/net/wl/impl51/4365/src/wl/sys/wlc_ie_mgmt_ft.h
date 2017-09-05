/*
 * IE management module Frame Type specific structures.
 *
 * Used to communicate between IE management module users (caller and callbacks).
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_ie_mgmt_ft.h 664112 2016-10-10 13:27:40Z $
 */

#ifndef _wlc_ie_mgmt_ft_h_
#define _wlc_ie_mgmt_ft_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <wlc_types.h>
#include <wlioctl.h>
#include <bcmwifi_channels.h>

/*
 * 'calc_len' and 'build' parameters.
 *
 * Passed from the caller of either wlc_ieml_calc_len or wlc_ieml_build_frame
 * APIs to the registered 'calc_len' and/or 'build' callbacks.
 *
 * Please add any new fields at the end of each type struct.
 */
union wlc_iem_ft_cbparm {
	/* for auth calc/build */
	struct {
		int alg;		/* auth algo */
		int seq;		/* sequence # */
		struct scb *scb;	/* maybe NULL when 'status' isn't DOT11_SC_SUCCESS */
		uint8 *challenge;	/* challenge text for seq 3 when 'alg' is shared key */
		uint16 status;		/* Output: Status Code */
	} auth;
	/* for (re)assocreq calc/build */
	struct {
		scb_t *scb;
		wlc_bss_info_t *target;	/* Association Target */
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint8 *md_ie;		/* Mobility Domain IE */
		uint8 *wpa_ie;		/* Output: WPA IE */
		uint8 *wpa2_ie;		/* Output: WPA2 IE */
		uint8 narrow_bw;	/* need drop current bw to narrow bw. */
	} assocreq;
	/* for (re)assocresp calc/build */
	struct {
		scb_t *scb;
		uint8 *mcs;		/* Preferred MCS */
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint status;
#ifdef WLFBT
		uint8 *md_ie;		/* Mobility Domain IE */
		uint8 *wpa2_ie;		/* WPA2 IE */
#endif /* WLFBT */
	} assocresp;
	/* for prbreq calc/build */
	struct {
		uint8 *mcs;		/* Preferred MCS */
		const uint8 *ssid;
		uint8 ssid_len;
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
	} prbreq;
	/* for bcn/prbrsp calc/build */
	struct {
		uint8 *mcs;		/* Preferred MCS */
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint8 *tim_ie;		/* Output: TIM IE */
	} bcn;
	/* for CS wrapper IE */
	struct {
		chanspec_t chspec;	/* new chanspec */
	} csw;
	/* for TDLS Setup frames */
	struct {
		scb_t *scb;
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint8 *cap;	/* Extended Capabilities */
		chanspec_t chspec;	/* chanspec on which the STA-AP connection runs on */
		uint8 *ft_ie;	/* Output: FT IE pointer */
		uint8 action;
		bool ht_op_ie;
		bool vht_op_ie;
	} tdls;
	/* for TDLS Discovery frames */
	struct {
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint8 *linkid_ie;	/* Link ID */
		uint8 *cap;	/* Extended Capabilities */
		uint8 ext_cap_len;	/* Extended Capabilities len */
	} disc;
	struct {
		int rde_count;	/* RDE IE count in RIC */
		int ts_count;	/* WME TSPEC IE count in RIC */
		uint8 *ts;	/* TSPEC list */
#ifdef WLFBT
		uint8 rde_id;   /* RDE Identifier from station */
		uint16 status;  /* status code for each TSPEC. AP Mode only */
#endif /* WLFBT */
	} fbtric;
};

/*
 * 'parse' parameters.
 *
 * Passed from the wlc_ieml_parse_frame API to the registered 'parse' callbacks.
 *
 * Please add any new fields at the end of each type struct.
 */
union wlc_iem_ft_pparm {
	/* for auth parse */
	struct {
		int alg;	/* auth algo */
		int seq;	/* sequence # */
		scb_t *scb;
		uint8 *challenge;	/* Output: Seq #2 Challenge text */
		uint16 status;	/* Output: Status Code */
	} auth;
	/* for (re)assocreq parse */
	struct {
		scb_t *scb;
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint8 *ht_cap_ie;	/* Output: HT Capability IE */
		uint8 *vht_cap_ie;	/* Output: VHT Capability IE */
		uint8 *vht_op_ie;	/* Output: VHT Operation IE */
		uint8 vht_ratemask;	/* Output: VHT BRCM Ratemask */
		uint8 *wps_ie;	/* Output: WPS IE */
		uint16 status;	/* Output: Status Code */
#ifdef WLFBT
		uint8 *md_ie;		/* Mobility Domain IE */
		uint8 *wpa2_ie;		/* WPA2 IE */
		uint8 *ft_ie;		/* FBT FT IE */
#endif /* WLFBT */
	} assocreq;
	/* for (re)assocresp parse */
	struct {
		scb_t *scb;
		uint16 status;	/* Output: Status Code */
		uint8 *md_ie;		/* Mobility Domain IE */
		uint8 *wpa2_ie;		/* WPA2 IE */
		uint8 *ft_ie;		/* FBT FT IE */
	} assocresp;
	/* for bcn parse in bcn proc (for Infra STA)
	 * when the bcn is from the associated AP
	 */
	struct {
		scb_t *scb;
		uint8 chan;	/* DS channel or rx channel */
		uint16 cap;
		bool erp;
	} bcn;
	/* for bcn/prbrsp parse during scan
	 * when the bcn/prbrsp is parsed by wlc_recv_parse_bcn_prb
	 */
	struct {
		wlc_bss_info_t *result;
		bool cap_bw_40;	/* Output: 40Mhz Capable */
		bool op_bw_any;	/* Output: Any Bandwidth */
		uint8 chan;	/* DS channel or rx channel */
	} scan;
	/* for TDLS Setup frames parse */
	struct {
		wlc_bss_info_t *result;
	} tdls;
};

#endif /* _wlc_ie_mgmt_ft_h_ */
