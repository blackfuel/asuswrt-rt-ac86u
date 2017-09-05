/*
 * NetBSD  2.0 OS Common defines
 * Private between the OSL and BSD per-port code
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: bcmbsd.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _bcmbsd_h_
#define _bcmbsd_h_

/* This macro checks to see if the data area is within the same page */
#define IN_PAGE(p, len) ((((PAGE_SIZE-1) & (uint)(p)) + ((len) - 1)) < PAGE_SIZE)

#endif /* _bcmbsd_h_ */
