/*
 * bcm_llist.h
 * Linked list manipulation routines header
 * See discussion below for more info
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: bcm_llist.h,v 1.2 2010-03-08 22:49:20 $
 */

#ifndef _bcm_llist_h_
#define _bcm_llist_h_


extern int bcm_llist_add_member(void *pphead, void *pnew);
extern int bcm_llist_del_member(void *pphead, void *pdel);

extern void * bcm_llist_del_membercmp(void *pplhd, void *arg,
	bool (*pcmp)(void *, void *)) ;


#endif /* _bcm_llist_h_ */
