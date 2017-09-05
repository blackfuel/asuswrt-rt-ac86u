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
 * $Id: lbuf.h 241182 2011-02-17 21:50:03Z $
 */


#ifndef _LBUF_H_
#define _LBUF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Include Files ---------------------------------------------------- */
#include <osl_ext.h>


/* ---- Constants and Types ---------------------------------------------- */

#define LBUFSZ		2048	/* largest reasonable packet buffer, driver uses for ethernet MTU */

#define	NTXBUF		16	/* # local transmit buffers */
#define	NRXBUF		16	/* # local receive buffers */


/* Forward declaration */
struct lbfree;


/* Linked buffer. */
struct lbuf {
	struct lbuf	*next;			/* pointer to next lbuf if in a chain */
	struct lbuf	*link;			/* pointer to next lbuf if in a list */
	uchar       	*head;			/* start of buffer */
	uchar		*end;			/* end of buffer */
	uchar		*data;			/* start of data */
	uchar		*tail;			/* end of data */
	uint		len;			/* nbytes of data */
	uint		priority;		/* packet priority */
	uint16		flags;
	uchar		pkttag[OSL_PKTTAG_SZ];	/* pkttag area  */
	struct lbfree	*list;			/* lbuf's parent freelist */
	void		*native_pkt;		/* native pkt associated with lbuf. */
};


/* Simple, static list of free lbufs. */
typedef struct lbfree {
	struct lbuf	*free;		/* the linked list */
	uint		total;		/* # total packets */
	uint		count;		/* # packets currently on free */
	uint		headroom;
	uint		size;		/* # bytes packet buffer memory */
	uchar		*dbuf;
	uchar 		*sbuf;
	osl_ext_mutex_t	mutex;
} lbfree_struct;


typedef struct lbuf_info {
	void 		*osh;		/* pointer to OS handle */
	struct lbfree	txfree;		/* tx packet freelist */
	struct lbfree	rxfree;		/* rx packet freelist */
} lbuf_info_t;


/****************************************************************************
* Function:   lbuf_alloc_list
*
* Purpose:    Allocate global pool of linked buffers.
*
* Parameters: info  (mod) Linked buffers info structure.
*             list  (mod) Pointer to list of linked buffers to allocate.
*             total (in)  Number of buffers to allocate.
*
* Returns:    TRUE on success, else FALSE.
*****************************************************************************
*/
bool
lbuf_alloc_list(lbuf_info_t *info, struct lbfree *list, uint total);

/****************************************************************************
* Function:   lbuf_free_list
*
* Purpose:    Free global pool of linked buffers.
*
* Parameters: info (mod) Linked buffers info structure.
*             list (mod) List of linked buffer to free.
*
* Returns:    Nothing.
*****************************************************************************
*/
void
lbuf_free_list(lbuf_info_t *info, struct lbfree *list);

/****************************************************************************
* Function:   lbuf_get
*
* Purpose:    Get a buffer from list of free buffers.
*
* Parameters: list (mod) List to get buffer from.
*
* Returns:    Buffer.
*****************************************************************************
*/
struct lbuf *
lbuf_get(struct lbfree *list);

/****************************************************************************
* Function:   lbuf_put
*
* Purpose:    Return buffer to linked list of free buffers.
*
* Parameters: list (mod) List to return buffer to.
*             lb   (mod) Buffer to return.
*
* Returns:    Nothing.
*****************************************************************************
*/
void
lbuf_put(struct lbfree *list, struct lbuf *lb);


/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */


#ifdef __cplusplus
	}
#endif

#endif  /* _LBUF_H_  */
