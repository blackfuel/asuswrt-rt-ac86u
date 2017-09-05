/*****************************************************************************
 * Binding stack declarations
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *****************************************************************************
*/

#if !defined(__BIND_SK_H__)
#define __BIND_SK_H__


extern void
bind_sk_init(struct bind_sk *sk, void (*cb)(void *, void *, int), void *arg);

extern void
bind_sk_push(struct bind_sk **top, struct bind_sk *elt);

extern struct bind_sk *
bind_sk_pop(struct bind_sk **top);


#endif /* !defined(__BIND_SK_H__) */
