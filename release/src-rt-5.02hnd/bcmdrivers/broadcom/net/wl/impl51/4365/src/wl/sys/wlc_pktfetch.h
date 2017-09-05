/*
 * Header for the common Pktfetch use cases in WLC
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_pktfetch.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_pktfetch_h_
#define _wlc_pktfetch_h_

#ifdef BCMSPLITRX
#include <wlc_types.h>
#include <rte_pktfetch.h>
#include <wlc_frmutil.h>
#include <bcmendian.h>

typedef struct wlc_eapol_pktfetch_ctx {
	wlc_info_t *wlc;
	struct scb *scb;
	wlc_frminfo_t f;
	bool ampdu_path;
	bool ordered;
	struct pktfetch_info *pinfo;
	bool promisc;
} wlc_eapol_pktfetch_ctx_t;

#define LLC_SNAP_HEADER_CHECK(lsh) \
			lsh->dsap == 0xaa && \
			lsh->ssap == 0xaa && \
			lsh->ctl == 0x03 && \
			lsh->oui[0] == 0 && \
			lsh->oui[1] == 0 && \
			lsh->oui[2] == 0

#define EAPOL_PKTFETCH_REQUIRED(lsh) \
	(ntoh16(lsh->type) == ETHER_TYPE_802_1X && \
		LLC_SNAP_HEADER_CHECK(lsh))

#ifdef WLNDOE
#define ICMP6_MIN_BODYLEN	(DOT11_LLC_SNAP_HDR_LEN + sizeof(struct ipv6_hdr)) + \
				sizeof(((struct icmp6_hdr *)0)->icmp6_type)
#define ICMP6_NEXTHDR_OFFSET	(sizeof(struct dot11_llc_snap_header) + \
				OFFSETOF(struct ipv6_hdr, nexthdr))
#define ICMP6_TYPE_OFFSET	(sizeof(struct dot11_llc_snap_header) + \
				sizeof(struct ipv6_hdr) + \
				OFFSETOF(struct icmp6_hdr, icmp6_type))

#define NDOE_PKTFETCH_REQUIRED(wlc, lsh, pbody, body_len) \
	(lsh->type == hton16(ETHER_TYPE_IPV6) && \
	LLC_SNAP_HEADER_CHECK(lsh) && \
	body_len >= (((uint8 *)lsh - (uint8 *)pbody) + ICMP6_MIN_BODYLEN) && \
	*((uint8 *)lsh + ICMP6_NEXTHDR_OFFSET) == ICMPV6_HEADER_TYPE && \
	(*((uint8 *)lsh + ICMP6_TYPE_OFFSET) == ICMPV6_PKT_TYPE_NS || \
	*((uint8 *)lsh + ICMP6_TYPE_OFFSET) == ICMPV6_PKT_TYPE_RA) && \
	NDOE_ENAB(wlc->pub))
#endif /* WLNDOE */

#ifdef WLTDLS
#define	WLTDLS_PKTFETCH_REQUIRED(wlc, lsh)	\
	(TDLS_ENAB(wlc->pub) && ntoh16(lsh->type) == ETHER_TYPE_89_0D)
#endif

extern void wlc_recvdata_schedule_pktfetch(wlc_info_t *wlc, struct scb *scb,
	wlc_frminfo_t *f, bool promisc_frame, bool ordered);
extern bool wlc_pktfetch_required(wlc_info_t *wlc, void *p, uchar *pbody, uint body_len,
	wlc_key_info_t *key_info, bool skip_iv);
#if defined(PKTC) || defined(PKTC_DONGLE)
extern void wlc_sendup_schedule_pktfetch(wlc_info_t *wlc, void *pkt);
#endif
#endif /* BCMSPLITRX */

#endif	/* _wlc_pktfetch_h_ */
