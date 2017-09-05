/*
 * P2P discovery state machine.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: bcm_p2p_disc.h 468737 2014-04-08 17:59:20Z $
 */

#ifndef _BCM_P2P_DISC_H
#define _BCM_P2P_DISC_H

typedef struct bcm_p2p_disc_s bcm_p2p_disc_t;

/* Opaque driver handle type. In dongle this is struct wlc_info_t, representing
 * the driver. On linux host this is struct ifreq, representing the primary OS
 * interface for a driver instance. To specify a virtual interface this should
 * be used together with a bsscfg index.
 */
struct bcm_p2p_wl_drv_hdl;

#ifndef BCMDRIVER
/* initialize P2P discovery */
int bcm_p2p_disc_init(void);

/* deinitialize P2P discovery */
int bcm_p2p_disc_deinit(void);
#endif /* BCMDRIVER */

/* create P2P discovery */
bcm_p2p_disc_t *bcm_p2p_disc_create(struct bcm_p2p_wl_drv_hdl *drv, uint16 listenChannel);

/* destroy P2P discovery */
int bcm_p2p_disc_destroy(bcm_p2p_disc_t *disc);

/* reset P2P discovery */
int bcm_p2p_disc_reset(bcm_p2p_disc_t *disc);

/* start P2P discovery */
int bcm_p2p_disc_start_discovery(bcm_p2p_disc_t *disc);

/* start P2P extended listen */
/* for continuous listen set on=non-zero (e.g. 5000), off=0 */
int bcm_p2p_disc_start_ext_listen(bcm_p2p_disc_t *disc,
	uint16 listenOnTimeout, uint16 listenOffTimeout);

/* get bsscfg index of P2P discovery interface */
/* bsscfg index is valid only after started */
int bcm_p2p_disc_get_bsscfg_index(bcm_p2p_disc_t *disc);

/* wlan event handler */
void bcm_p2p_disc_process_wlan_event(void *context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length);

/* Initialize module persistent data */
int bcm_p2p_disc_config_init(void);

/* Clean up module persistent data */
int bcm_p2p_disc_config_cleanup(struct bcm_p2p_wl_drv_hdl *drv);

/* Set module persistent data */
int bcm_p2p_disc_config_set(struct bcm_p2p_wl_drv_hdl *drv, int32 home_time,
	uint8 flags, uint8 num_social_channels, uint16 *social_channels);

/* Get module persistent data */
int bcm_p2p_disc_config_get(int32 *home_time, uint8 *flags,
	uint8 *num_social_channels, uint16 **social_channels);

#endif /* _BCM_P2P_DISC_H */
