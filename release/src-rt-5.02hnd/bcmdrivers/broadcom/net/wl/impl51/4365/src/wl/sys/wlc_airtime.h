/*
 * wlc_airtime.h
 *
 * This module contains the public external definitions for the airtime fairness utilities.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_airtime.h 554261 2015-05-04 23:56:00Z $
 *
 */


#if !defined(__WLC_AIRTIME_H__)
#define __WLC_AIRTIME_H__

#define WLC_AIRTIME_USERTS		0x00000001
#define WLC_AIRTIME_USECTS		0x00000002
#define WLC_AIRTIME_SHORTSLOT		0x00000004
#define WLC_AIRTIME_MIXEDMODE		0x00000008
#define WLC_AIRTIME_AMPDU		0x00000010


#define WLC_AIRTIME_RTSCTS		(WLC_AIRTIME_USERTS | WLC_AIRTIME_USECTS)
#define WLC_AIRTIME_CTS2SELF		(WLC_AIRTIME_USECTS)
#define WLC_AIRTIME_RTS(f)		((f) & WLC_AIRTIME_USERTS)
#define WLC_AIRTIME_CTS(f)		((f) & WLC_AIRTIME_USECTS)
#define WLC_AIRTIME_SS(f)		((f) & WLC_AIRTIME_SHORTSLOT)
#define WLC_AIRTIME_MM(f)		((f) & WLC_AIRTIME_MIXEDMODE)
#define WLC_AIRTIME_BA(f)		((f) & WLC_AIRTIME_AMPDU)

#define WLC_AIRTIME_PMODE		2 /* ATF Pig mode.
					   * If enabled ATF will release up to airtime limit
					   */
/*
 * Packet overhead not including PLCP header of payload
 * Partial calculation to help speed up AMPDU datapath
 * Returns time is microseconds.
 */
extern uint BCMFASTPATH wlc_airtime_pkt_overhead_us(uint flags,
	uint32 ctl_rate_kbps, uint32 ack_rate_kbps, wlc_bsscfg_t *bsscfg, uint ac);

/*
 * Payload packet time and PLCP excluding overhead
 * Partial calculation to help speed up AMPDU datapath
 * Returns time is microseconds.
 */
extern uint BCMFASTPATH wlc_airtime_packet_time_us(uint32 flg,
	uint32 rspec, uint size_in_bytes);

/*
 * Packet payload only time based on number of bytes in frame and rate.
 * Returns time is microseconds.
 */
extern uint BCMFASTPATH wlc_airtime_payload_time_us(uint32 flg,
	uint32 rspec, uint size_in_bytes);

/*
 * Calculate the number of bytes of 802.11 Protocol overhead,
 * assuming a 3 address header and QoS format. Includes security wrapper and FCS.
 * Returns Number of bytes.
 */
extern uint BCMFASTPATH wlc_airtime_dot11hdrsize(uint32 wsec);

/* Time of the PLCP header given rate.
 * Returns time in microseconds.
 */
extern uint BCMFASTPATH wlc_airtime_plcp_time_us(uint32 rspec, uint32 flg);

extern uint BCMFASTPATH airtime_rts_usec(uint32 flg, uint ctl_rspec);
extern uint BCMFASTPATH airtime_cts_usec(uint32 flg, uint ctl_rspec);
extern uint BCMFASTPATH airtime_ba_usec(uint32 flg, uint ctl_rspec);

#endif /* __WLC_AIRTIME_H__ */
