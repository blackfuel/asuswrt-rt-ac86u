/*
 * stddef.h - Broadcom RTE-specific POSIX replacement library definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: stddef.h 473326 2014-04-29 00:37:35Z $
 */

#if !defined(_STDDEF_H_)
#define _STDDEF_H_

typedef int wchar_t;

#define offsetof(type, field) ((size_t)(&((type *)0)->field))

#endif /* !defined(_STDDEF_H_) */
