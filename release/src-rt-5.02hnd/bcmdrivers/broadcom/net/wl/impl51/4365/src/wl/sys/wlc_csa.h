/*
 * 802.11h CSA module header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_csa.h 662097 2016-09-28 09:04:57Z $
*/

/**
 * Channel Switch Announcement and Extended Channel Switch Announcement
 * Related to radar avoidance
 */


#ifndef _wlc_csa_h_
#define _wlc_csa_h_

/* APIs */
#ifdef WLCSA
typedef struct wlc_csa_notif_cb_data {
	wlc_bsscfg_t *cfg;
	int status;
	int signal;
	chanspec_t chanspec;
} wlc_csa_notif_cb_data_t;

enum CSA_NOTIF_SIGNALS {
	CSA_CHANNEL_CHANGE_START = 1,
	CSA_CHANNEL_CHANGE_END = 2
};

typedef void (*wlc_csa_notif_cb_fn_t)(void *ctx, wlc_csa_notif_cb_data_t *notif_data);

/* module */
extern wlc_csa_info_t *wlc_csa_attach(wlc_info_t *wlc);
extern void wlc_csa_detach(wlc_csa_info_t *csam);

/* recv/send */
extern void wlc_recv_public_csa_action(wlc_csa_info_t *csam,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
extern void wlc_recv_csa_action(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
extern void wlc_recv_ext_csa_action(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len);

extern int wlc_send_action_switch_channel(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg);

#ifdef CLIENT_CSA
extern int wlc_send_unicast_action_switch_channel(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	const struct ether_addr *dst, wl_chan_switch_t *csa, uint8 action_id);
#endif /* CLIENT_CSA */
/* actions */
extern void wlc_csa_do_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	chanspec_t chspec);
extern void wlc_csa_count_down(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg);
extern void wlc_csa_reset_all(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg);
extern void wlc_csa_do_csa(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	wl_chan_switch_t *cs, bool docs);

extern int wlc_csa_do_channel_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	chanspec_t chanspec, uint8 mode, uint8 count, uint8 reg_class, uint8 frame_type);

/* IE build/parse */
#ifdef WL11AC
extern uint8 *wlc_csa_write_chan_switch_wrapper_ie(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	uint8 *cp, int buflen);
#endif /* WL11AC */

extern bool wlc_csa_quiet_mode(wlc_csa_info_t *csam, uint8 *tag, uint tag_len);

/* accessors */
extern uint8 wlc_csa_get_csa_count(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg);
extern int wlc_csa_obss_dynbw_notif_cb_register(wlc_csa_info_t *csam,
	wlc_csa_notif_cb_fn_t cb, void *arg);
extern int wlc_csa_obss_dynbw_notif_cb_unregister(wlc_csa_info_t *csam,
	wlc_csa_notif_cb_fn_t cb, void *arg);

#else /* !WLCSA */

#define wlc_csa_attach(wlc) NULL
#define wlc_csa_detach(csam) do {} while (0)

#define wlc_recv_public_csa_action(csam, hdr, body, body_len) do {} while (0)
#define wlc_recv_csa_action(csam, cfg, hdr, body, body_len) do {} while (0)
#define wlc_recv_ext_csa_action(csam, cfg, hdr, body, body_len) do {} while (0)

#define wlc_send_action_switch_channel(csam, cfg) do {} while (0)

#define wlc_csa_do_switch(csam, cfg, chspec) do {} while (0)
#define wlc_csa_count_down(csam, cfg) do {} while (0)
#define wlc_csa_reset_all(csam, cfg) do {} while (0)
#define wlc_csa_do_csa(csam, cfg, cs, docs) do {} while (0)

#define wlc_csa_write_chan_switch_wrapper_ie(csam, cfg, cp, buflen) (cp)

#define wlc_csa_quiet_mode(csam, tag, tag_len) FALSE

#define wlc_csa_get_csa_count(csam, cfg) 0

#define wlc_csa_obss_dynbw_notif_cb_register(csam, cb, arg)  0
#define wlc_csa_obss_dynbw_notif_cb_unregister(csam, cb, arg)  0

#endif /* !WLCSA */

#endif /* _wlc_csa_h_ */
