/*
 * Simple singly link
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wps_sslist.h>

WPS_SSLIST *
wps_sslist_add(WPS_SSLIST **head, void *item)
{
	WPS_SSLIST *obj = (WPS_SSLIST *)item;
	WPS_SSLIST *curr;

	if (!head || !item)
		return NULL;

	/* NULL ended the list */
	obj->next = 0;

	/* First empty ? */
	if (*head == NULL) {
		*head = obj;
	}
	else {
		/* Insert to tail */
		curr = *head;
		while (curr->next)
			curr = curr->next;

		curr->next = obj;
	}

	return obj;
}
