/*****************************************************************************
 * Binding stack declarations (private)
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

#if !defined(__BIND_SKP_H__)
#define __BIND_SKP_H__


struct bind_sk {
	struct bind_sk *next;
	int (*cb)(void *arg, void *data, int sz);
	void *arg;
};


#endif /* !defined(__BIND_SKP_H__) */
