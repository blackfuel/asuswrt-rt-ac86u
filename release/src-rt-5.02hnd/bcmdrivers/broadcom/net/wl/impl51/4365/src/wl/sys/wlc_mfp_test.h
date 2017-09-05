/*
 * Broadcom 802.11 Networking Device Driver
 * Management Frame Protection (MFP)
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_mfp_test.h 479837 2014-05-22 08:38:37Z $
 */

#ifndef _wlc_mfp_test_h_
#define _wlc_mfp_test_h_


#include <wlc_types.h>

/* test iovars */
enum {
	IOV_MFP_TEST_SA_QUERY,
	IOV_MFP_TEST_DISASSOC,
	IOV_MFP_TEST_DEAUTH,
	IOV_MFP_TEST_ASSOC,
	IOV_MFP_TEST_AUTH,
	IOV_MFP_TEST_REASSOC,
	IOV_MFP_TEST_BIP,
	IOV_MFP_TEST_INVALID
};

/* handle iovar */
int mfp_test_doiovar(wlc_info_t *wlc, const bcm_iovar_t *vi,
	uint32 actionid, const char *name, void *params, uint p_len,
	void *arg, int len, int val_size, struct wlc_if *wlcif);

void* mfp_test_send_sa_query(wlc_info_t *wlc, struct scb *scb,
    uint8 action, uint16 id);
void mfp_test_bip_mic(const struct dot11_management_header *hdr,
    const uint8 *body, int body_len, const uint8 *key, uint8 * mic);

/* translation between registered iov and above */
int mfp_test_iov(const int mfp_iov);

#endif	/* !_wlc_test_mfp_h_ */
