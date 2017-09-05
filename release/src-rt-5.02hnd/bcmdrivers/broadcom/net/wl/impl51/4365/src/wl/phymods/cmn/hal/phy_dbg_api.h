/*
 * Debug module public interface (to MAC driver).
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _phy_dbg_api_h_
#define _phy_dbg_api_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_api.h>

/*
 * Invoke dump function for module 'id'. Return BCME_XXXX.
 */
int phy_dbg_dump(phy_info_t *pi, char *name, struct bcmstrbuf *b);

#endif /* _phy_dbg_api_h_ */
