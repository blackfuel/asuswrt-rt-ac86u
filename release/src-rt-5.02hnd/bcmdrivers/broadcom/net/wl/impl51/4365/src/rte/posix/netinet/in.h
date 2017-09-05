/*
 * in.h - Broadcom RTE-specific POSIX replacement library definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: in.h 473326 2014-04-29 00:37:35Z $
 */

#if !defined(_IN_H_)
#define _IN_H_

#include <bcmendian.h>
#define htons hton16
#define ntohs ntoh16
#define htonl hton32
#define ntohl ntoh32

#endif /* !defined(_IN_H_) */
