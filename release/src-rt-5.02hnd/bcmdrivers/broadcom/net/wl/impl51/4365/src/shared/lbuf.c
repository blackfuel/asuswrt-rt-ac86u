/*
 * Linked buffer (lbuf) pool, with static buffer size.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: lbuf.c 467150 2014-04-02 17:30:43Z $
 */

/* ---- Include Files ---------------------------------------------------- */

#include <typedefs.h>
#include <osl.h>
#include <osl_ext.h>
#include <bcmutils.h>
#include <lbuf.h>


/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define LBUF_ERROR(x)	printf x
#define LBUF_TRACE(x)	printf x


/* ---- Private Variables ------------------------------------------------ */
/* ---- Private Function Prototypes -------------------------------------- */

static bool
lbuf_addbuf(lbuf_info_t *info, struct lbfree *list, uchar *buf, struct lbuf *lb);


/* ---- Functions -------------------------------------------------------- */

/****************************************************************************
* Function:   lbuf_addbuf
*
* Purpose:    Initialize a linked buffer, and add it to the specified list.
*
* Parameters: info (mod) Linked buffers info structure.
*             list (mod) List to add buffer to.
*             buf  (in)  Allocated buffer.
*             lb   (out) Linked buffer to initialize and add to list.
*
* Returns:    TRUE on success, else FALSE.
*****************************************************************************
*/
static bool
lbuf_addbuf(lbuf_info_t *info, struct lbfree *list, uchar *buf, struct lbuf *lb)
{
	/* Initialize lbuf fields */
	memset(lb, 0, sizeof(struct lbuf));
	lb->head = buf;
	lb->end  = buf + list->size;
	lb->list = list;

	/* Put it on the freelist */
	lbuf_put(list, lb);

	return (TRUE);
}


/* ----------------------------------------------------------------------- */
bool
lbuf_alloc_list(lbuf_info_t *lbuf_info, struct lbfree *list, uint total)
{
	bool status;
	int i;
	uchar *dbuf = NULL;
	uchar *sbuf = NULL;

	LBUF_TRACE(("lbuf_alloc_list: total %d\n", total));

	memset(list, 0, sizeof(struct lbfree));
	list->total = total;
	list->size = LBUFSZ;

	if (osl_ext_mutex_create("lbuf", &list->mutex) != OSL_EXT_SUCCESS)
		goto enomem;


	if ((list->dbuf = (uchar *) MALLOC(lbuf_info->osh, (list->total * list->size))) == NULL)
		goto enomem;
	memset(list->dbuf, 0, (list->total * list->size));
	dbuf = list->dbuf;

	list->sbuf = (uchar *) MALLOC(lbuf_info->osh, (list->total * sizeof(struct lbuf)));
	if (list->sbuf == NULL)
		goto enomem;
	memset(list->sbuf, 0, (list->total * sizeof(struct lbuf)));
	sbuf = list->sbuf;

	/* fill the freelist */
	for (i = 0; i < total; i++) {
		status = lbuf_addbuf(lbuf_info, list, dbuf,
		                     (struct lbuf *)(&(sbuf[i * sizeof(struct lbuf)])));
		if (!status)
			goto enomem;
		else
			dbuf += list->size;
	}

	return (TRUE);

enomem:
	LBUF_ERROR(("lbuf_alloc_list: out of memory, malloced %d bytes\n",
	            MALLOCED(lbuf_info->osh)));
	lbuf_free_list(lbuf_info, list);
	return (FALSE);

}


/* ----------------------------------------------------------------------- */
void
lbuf_free_list(lbuf_info_t *lbuf_info, struct lbfree *list)
{
	LBUF_TRACE(("lbuf_free_list\n"));

	ASSERT(list->count <= list->total);

	osl_ext_mutex_delete(&list->mutex);

	if (list->sbuf) {
		MFREE(lbuf_info->osh, list->sbuf, (list->total * sizeof(struct lbuf)));
	}

	if (list->dbuf) {
		MFREE(lbuf_info->osh, list->dbuf, (list->total * list->size));
	}

	memset(list, 0, sizeof(struct lbfree));

	return;
}


/* ----------------------------------------------------------------------- */
struct lbuf *
lbuf_get(struct lbfree *list)
{
	struct lbuf		*lb;
	osl_ext_status_t	osl_status;

	ASSERT(list->count <= list->total);

	osl_status = osl_ext_mutex_acquire(&list->mutex, OSL_EXT_TIME_FOREVER);
	ASSERT(osl_status == OSL_EXT_SUCCESS);

	if ((lb = list->free) != NULL) {
		list->free = lb->link;
		lb->link = lb->next = NULL;
		lb->data = lb->tail = lb->head;
		lb->priority = 0;
		lb->len = 0;
		lb->native_pkt = NULL;
		list->count--;
		memset(lb->pkttag, 0, OSL_PKTTAG_SZ);
	}

	osl_status = osl_ext_mutex_release(&list->mutex);
	ASSERT(osl_status == OSL_EXT_SUCCESS);

	return (lb);
}


/* ----------------------------------------------------------------------- */
void
lbuf_put(struct lbfree *list, struct lbuf *lb)
{
	osl_ext_status_t	osl_status;

	ASSERT(list->count <= list->total);
	ASSERT(lb->link == NULL);
	ASSERT(lb->next == NULL);
	ASSERT(lb->list == list);

	osl_status = osl_ext_mutex_acquire(&list->mutex, OSL_EXT_TIME_FOREVER);
	ASSERT(osl_status == OSL_EXT_SUCCESS);

	lb->data = lb->tail = (uchar*)(uintptr)0xdeadbeef;
	lb->len = 0;
	lb->link = list->free;
	list->free = lb;
	list->count++;

	osl_status = osl_ext_mutex_release(&list->mutex);
	ASSERT(osl_status == OSL_EXT_SUCCESS);
}
