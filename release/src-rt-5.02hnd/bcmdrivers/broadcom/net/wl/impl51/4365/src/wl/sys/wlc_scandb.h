/*
 * Scan Data Base
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_scandb.h 467328 2014-04-03 01:23:40Z $
 */
#ifndef _WLC_SCANDB_H_
#define _WLC_SCANDB_H_

#include "typedefs.h"
#include "osl.h"
#include "proto/ethernet.h"
#include "bcmutils.h"
#include "wlioctl.h"

#ifndef WLC_SCANDB_DEFAULT_TIMEOUT
#define WLC_SCANDB_DEFAULT_TIMEOUT	64	/* default timeout in seconds */
#endif

typedef struct wlc_scandb wlc_scandb_t;

typedef void (*scandb_iter_fn_t)(void *arg1, void *arg2, uint timestamp,
                                 struct ether_addr *BSSID, wlc_ssid_t *SSID,
                                 int BSS_type, chanspec_t chanspec,
                                 void *data, uint datalen);

#ifdef WLSCANCACHE

extern wlc_scandb_t* wlc_scandb_create(osl_t *osh, uint unit);
extern void wlc_scandb_free(wlc_scandb_t *sdb);

#else /* WLSCANCACHE */

#define wlc_scandb_free(sdb) do {(void)(sdb);} while (0)

#endif /* WLSCANCACHE */

extern void wlc_scandb_clear(wlc_scandb_t *sdb);
extern void wlc_scandb_ageout(wlc_scandb_t *sdb, uint timestamp);

/* accessors for Scan DB timeout in ms */
extern uint wlc_scandb_timeout_get(wlc_scandb_t *sdb);
extern void wlc_scandb_timeout_set(wlc_scandb_t *sdb, uint timeout);


extern void wlc_scandb_add(wlc_scandb_t *sdb,
                           const struct ether_addr *BSSID, const wlc_ssid_t *SSID,
                           uint8 BSS_type, chanspec_t chanspec, uint timestamp,
                           void *data, uint datalen);

extern int wlc_scandb_iterate(wlc_scandb_t *sdb,
                              const struct ether_addr *BSSID, int nssid, const wlc_ssid_t *SSID,
                              int BSS_type, const chanspec_t *chanspec_list, uint chanspec_num,
                              scandb_iter_fn_t fn, void *fn_arg1, void *fn_arg2);

#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && !defined(SCANOL)
extern int wlc_scandb_dump(void *handle, struct bcmstrbuf *b);
#endif

#endif /* _WLC_SCANDB_H_ */
