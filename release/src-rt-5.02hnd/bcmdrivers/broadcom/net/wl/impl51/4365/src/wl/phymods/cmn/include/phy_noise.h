/*
 * NOISEmeasure module internal interface (to other PHY modules).
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

#ifndef _phy_noise_h_
#define _phy_noise_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_noise_info phy_noise_info_t;

/* attach/detach */
phy_noise_info_t *phy_noise_attach(phy_info_t *pi, int bandtype);
void phy_noise_detach(phy_noise_info_t *nxi);

/* set mode */
int phy_noise_set_mode(phy_noise_info_t *ii, int mode, bool init);

/* common dump functions for non-ac phy */
int phy_noise_dump_common(phy_info_t *pi, struct bcmstrbuf *b);
#endif /* _phy_noise_h_ */
