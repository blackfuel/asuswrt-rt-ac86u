/*
 * wlc_dngl_ol	definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_dngl_ol.h 473826 2014-04-29 23:43:06Z $
 */


#ifndef _wlc_dngl_ol_h_
#define _wlc_dngl_ol_h_

#include <bcm_ol_msg.h>
#include <wlc_dngl_ol_compat.h>
#include <wlc_keymgmt.h>
#include <rte_dev.h>

#define WAKE_FOR_PMMODE		(1<<0)
#define WAKE_FOR_PSPOLL		(1<<1)
#define WAKE_FOR_UATBTT		(1<<2)

/* begin support for updating shared info */
#define RXOESHARED(_dngl_ol) ((_dngl_ol)->shared)
#define RXOETXINFO(_dngl_ol) (&(RXOESHARED(_dngl_ol)->wowl_host_info.wake_tx_info.txinfo))
#define RXOEMICFAILINFO(_dngl_ol) (&(RXOESHARED(_dngl_ol)->wowl_host_info.mic_fail_info))
#define RXOEWOWLINFO(_dngl_ol) (&RXOESHARED(_dngl_ol)->wowl_host_info)

#define RXOEINC(_dngl_ol, a)  (RXOESHARED(_dngl_ol)->stats.a++)
#define RXOEINCCNTR(_dngl_ol) (RXOESHARED(_dngl_ol)->dngl_watchdog_cntr++)
#define RXOEINCIE(_dngl_ol, a, i) ((RXOESHARED(_dngl_ol)->stats.a[i])++)

#define RXOEINC_N(a, n) (a = ((a + 1) & ((n) - 1)))
#define RXOEADDARPENTRY(_dngl_ol, entry) (\
	{ uint8 i = RXOESHARED(_dngl_ol)->stats.rxoe_arp_statidx; \
    bcopy(&entry, \
		(void *)&RXOESHARED(_dngl_ol)->stats.rxoe_arp_stats[i], \
		sizeof(olmsg_arp_stats)); \
	RXOEINC_N(RXOESHARED(_dngl_ol)->stats.rxoe_arp_statidx, MAX_STAT_ENTRIES); })

#define RXOEADDNDENTRY(_dngl_ol, entry) (\
	{ uint8 i = RXOESHARED(_dngl_ol)->stats.rxoe_nd_statidx; \
    bcopy(&entry, \
		(void *)&RXOESHARED(_dngl_ol)->stats.rxoe_nd_stats[i], \
		sizeof(olmsg_nd_stats)); \
	RXOEINC_N(RXOESHARED(_dngl_ol)->stats.rxoe_nd_statidx, MAX_STAT_ENTRIES); })

#define RXOEADD_PKT_FILTER_ENTRY(_dngl_ol, entry) (\
	{ uint8 i = RXOESHARED(_dngl_ol)->stats.rxoe_pkt_filter_statidx; \
    bcopy(&entry, \
		(void *)&RXOESHARED(_dngl_ol)->stats.rxoe_pkt_filter_stats[i], \
		sizeof(olmsg_pkt_filter_stats)); \
	RXOEINC_N(RXOESHARED(_dngl_ol)->stats.rxoe_pkt_filter_statidx, MAX_STAT_ENTRIES); })

#define RXOEUPDREPLAYCNT(_dngl_ol, _rcnt)({ \
	void *wake_ctr = (void *)&RXOETXINFO(_dngl_ol)->replay_counter[0]; \
	bcopy((void *)(_rcnt), wake_ctr, EAPOL_KEY_REPLAY_LEN); })

/* update key rotation state in dngl_ol */
#define RXOEUPDKEYROT(_dngl_ol, _mask) \
	RXOETXINFO(_dngl_ol)->sec_info.key_rot_id_mask |= (_mask)

/* end support for updating shared info */

struct wlc_dngl_ol_info {
	wlc_info_t	*wlc;			/* pointer to os-specific private state */
	uint unit;		/* device instance number */
	osl_t *osh;
	hnd_dev_t *dev;
	wlc_hw_info_t	*wlc_hw;			/* HW module (private h/w states) */
	void *regs;
	uint16 pso_blk;
	pktpool_t *shared_msgpool;
	volatile olmsg_shared_info_t *shared;		/* reference to pcie shared */
	struct ether_addr cur_etheraddr;
	uint8 TX;
	uint8 pme_asserted;
	uint8 radio_hw_disabled;
	uint32 counter;
	uint32 stay_awake;

	/* WoWL cfg info */
	wowl_cfg_t  wowl_cfg;

	wlc_dngl_ol_bcn_info_t *bcn_ol;
	wlc_dngl_ol_pkt_filter_info_t *pkt_filter_ol;
	wlc_dngl_ol_wowl_info_t *wowl_ol;
	wlc_dngl_ol_l2keepalive_info_t *l2keepalive_ol;
	wlc_dngl_ol_gtk_info_t *ol_gtk;
	wlc_dngl_ol_mdns_info_t *mdns_ol;
	wlc_dngl_ol_rssi_info_t *rssi_ol;
	wlc_dngl_ol_eventlog_info_t *eventlog_ol;
	wlc_dngl_ol_ltr_info_t *ltr_ol;

	uint16		 max_bsscfg;
	wlc_bsscfg_t *bsscfg;			/* wlc_bsscfg_t[max_bsscfg] */
	uint16		 max_scb;
	scb_t		 *scb;				/* scb_t[max_scb] */
	int			 last_mic_fail_time;

	/* enab feature flags */
	bool		_ltr;			/* LTR cap enabled/disabled */
};

#define DNGL_OL_KEYMGMT(_dngl_ol) ((_dngl_ol)->wlc->keymgmt)

/* Counter amoung various offload modules */
enum counter_index {
	TXSUPPRESS,
	TXACKED,
	TXPROBEREQ
};

typedef void (*wlc_dngl_ol_event_handler_t)(wlc_dngl_ol_info_t *wlc_dngl_ol,
	uint32 event,
	void *event_data);

extern wlc_dngl_ol_info_t *wlc_dngl_ol_attach(wlc_info_t *wlc);
extern void wlc_dngl_ol_sendup(wlc_dngl_ol_info_t *wlc_dngl_ol, void* resp);
extern bool wlc_dngl_ol_sendpkt(wlc_dngl_ol_info_t *wlc_dngl_ol, void *sdu);
extern void wlc_dngl_ol_watchdog(wlc_dngl_ol_info_t *wlc_dngl_ol);
extern void wlc_dngl_ol_event(wlc_dngl_ol_info_t *wlc_dngl_ol, uint32 event, void *event_data);

extern void wlc_dngl_ol_push_to_host(wlc_info_t *wlc);
extern void wlc_dngl_ol_sec_info_from_host(wlc_dngl_ol_info_t *dngl_ol,
	const struct ether_addr *host_addr, const struct ether_addr *bssid,
	const ol_sec_info *sec_info);
extern void wlc_dngl_ol_tx_info_to_host(wlc_dngl_ol_info_t *dngl_ol,
	wlc_key_t *key, const wlc_key_info_t *key_info);
extern void wlc_dngl_ol_mic_fail_info_to_host(wlc_dngl_ol_info_t *dngl_ol);

/* called rx/tx path. could be inlined if needed */
extern void wlc_dngl_ol_iv_update(wlc_dngl_ol_info_t *dngl_ol,
	const wlc_key_info_t *key_info, const uint8 *seq, size_t seq_len,
	wlc_key_seq_id_t seq_id, bool tx);

extern bool wlc_dngl_ol_supr_frame(wlc_info_t	*wlc, uint16 frame_ptr);
extern void wlc_dngl_ol_recv(wlc_dngl_ol_info_t *wlc_dngl_ol, void *p);
extern void wlc_dngl_ol_armtx(wlc_dngl_ol_info_t *wlc_dngl_ol, void *buf, int len);
extern void wlc_dngl_ol_reset(wlc_dngl_ol_info_t *wlc_dngl_ol);
extern void* wlc_dngl_ol_frame_get_ctl(wlc_dngl_ol_info_t *wlc_dngl_ol, uint len);
extern void* wlc_dngl_ol_frame_get_ps_ctl(wlc_dngl_ol_info_t *wlc_dngl_ol,
	const struct ether_addr *bssid,
	const struct ether_addr *sa);

extern bool wlc_dngl_ol_sendpspoll(wlc_dngl_ol_info_t *wlc_dngl_ol);
extern void wlc_dngl_ol_intstatus(wlc_dngl_ol_info_t *wlc_dngl_ol);
extern bool wlc_dngl_arm_dotx(wlc_dngl_ol_info_t *wlc_dngl_ol);
extern int wlc_dngl_ol_process_msg(wlc_dngl_ol_info_t *wlc_dngl_ol, void *buf, int len);
extern bool arm_dotx(wlc_info_t *wlc);
extern void *wlc_dngl_ol_sendnulldata(wlc_dngl_ol_info_t *wlc_dngl_ol, int prio);
extern int generic_send_packet(wlc_dngl_ol_info_t *ol_info, uchar *params, uint p_len);
extern void wlc_dngl_cntinc(wlc_dngl_ol_info_t *ol_info, uint counter);


extern wlc_dngl_ol_rssi_info_t *wlc_dngl_ol_rssi_attach(wlc_dngl_ol_info_t *wlc_dngl_ol);
extern void wlc_dngl_ol_rssi_send_proc(wlc_dngl_ol_rssi_info_t *rssi_ol, void *buf, int len);
extern int wlc_dngl_ol_phy_rssi_compute_offload(wlc_dngl_ol_rssi_info_t *rssi_ol,
	wlc_d11rxhdr_t *wlc_rxh);
extern wlc_dngl_ol_eventlog_info_t *wlc_dngl_ol_eventlog_attach(wlc_dngl_ol_info_t *wlc_dngl_ol);
extern void wlc_dngl_ol_staywake_check(wlc_dngl_ol_info_t *wlc_dngl_ol, bool tim_set);

extern wlc_bsscfg_t *wlc_dngl_ol_get_bsscfg(wlc_dngl_ol_info_t *wlc_dngl_ol, int idx);
extern scb_t *wlc_dngl_ol_get_scb(wlc_dngl_ol_info_t *wlc_dngl_ol, int idx);

#if defined(BCMDBG) || defined(BCMDBG_ERR)
extern const char *bcm_ol_event_str[];
#endif

#ifdef BCMDBG
#define ENTER() WL_TRACE(("%s: Enter\n", __FUNCTION__));
#define EXIT()  WL_TRACE(("%s: line (%d) Exit\n", __FUNCTION__, __LINE__));
#else
#define ENTER()
#define EXIT()
#endif
#endif /* _wlc_dngl_ol_h_ */
