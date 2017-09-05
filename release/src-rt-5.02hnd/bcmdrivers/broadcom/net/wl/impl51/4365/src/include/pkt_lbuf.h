/*
 * Network interface packet buffer routines. These are used internally by the
 * driver to allocate/de-allocate packet buffers, and manipulate the buffer
 * contents and attributes.
 *
 * This implementation is specific to LBUF (linked buffer) packet buffers.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: pkt_lbuf.h 412662 2013-07-16 02:07:14Z $
 */


#ifndef _pkt_lbuf_h_
#define _pkt_lbuf_h_

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Include Files ---------------------------------------------------- */
/* ---- Constants and Types ---------------------------------------------- */

/* Handle for packet buffer module. */
typedef int pkt_handle_t;

#define OSH_NULL	NULL

/* Macros to allocate/de-allocate packet buffers, and manipulate the buffer
 * contents and attributes.
 */
#define PKTGET(osh, len, send)		pktget((osh), (len), (send))
extern void *pktget(osl_t *osh, uint len, bool send);

#define PKTFREE(osh, lb, send)		pktfree((osh), (lb), (send))
extern void pktfree(osl_t *osh, struct lbuf * lb, bool send);

#define PKTFRMNATIVE(osh, x, len)	pkt_frmnative((osh), (x), (len))
void* pkt_frmnative(osl_t *osh, void *p, int len);

#define PKTTONATIVE(osh, pkt)		pkt_tonative((osh), (pkt))
void* pkt_tonative(osl_t *osh, void *pkt);

#define PKTSETLEN(osh, lb, len)		pktsetlen((osh), (lb), (len))

#define PKTPUSH(osh, lb, bytes)		pktpush((osh), (lb), (bytes))

#define PKTPULL(osh, lb, bytes)		pktpull((osh), (lb), (bytes))

#define PKTSETHEADROOM(osh, tx, headroom) pkt_set_headroom((osh), (tx), (headroom))
extern void pkt_set_headroom(osl_t *osh, bool tx, unsigned int headroom);

#define PKTDATA(osh, lb)		((struct lbuf *)(lb))->data
#define PKTLEN(osh, lb)			((struct lbuf *)(lb))->len
#define PKTHEADROOM(osh, lb)		(PKTDATA(osh, lb)-(((struct lbuf *)(lb))->head))
#define PKTTAILROOM(osh, lb)		((((struct lbuf *)(lb))->end)-(((struct lbuf *)(lb))->tail))
#define PKTNEXT(osh, lb)		((struct lbuf *)(lb))->next
#define PKTSETNEXT(osh, lb, x)		((struct lbuf *)(lb))->next = (struct lbuf*)(x)
#define PKTTAG(lb)			(((void *) ((struct lbuf *)(lb))->pkttag))
#define PKTLINK(lb)			((struct lbuf *)(lb))->link
#define PKTSETLINK(lb, x)		((struct lbuf *)(lb))->link = (struct lbuf*)(x)
#define PKTPRIO(lb)			((struct lbuf *)(lb))->priority
#define PKTSETPRIO(lb, x)		((struct lbuf *)(lb))->priority = (x)
#define PKTSUMNEEDED(lb)		(0)
#define PKTSETSUMGOOD(lb, x)		((void)(0))
#define PKTSETPOOL(osh, lb, x, y)   do {} while (0)
#define PKTPOOL(osh, lb)            FALSE
#define PKTFREELIST(lb)             PKTLINK(lb)
#define PKTSETFREELIST(lb, x)       PKTSETLINK((lb), (x))
#define PKTPTR(lb)                  (lb)
#define PKTID(lb)                   (0)
#define PKTSETID(lb, id)            do {} while (0)

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

/****************************************************************************
* Function:   pkt_init
*
* Purpose:    Initialize packet buffer module.
*
* Parameters: osh (mod) Operating system handle.
*
* Returns:    Packet buffer module handle.
*****************************************************************************
*/
pkt_handle_t* pkt_init(osl_t *osh);

/****************************************************************************
* Function:   pkt_deinit
*
* Purpose:    De-initialize packet buffer module.
*
* Parameters: osh      (mod) Operating system handle.
*             pkt_info (mod) Packet buffer module handle.
*
* Returns:    Nothing.
*****************************************************************************
*/
void pkt_deinit(osl_t *osh, pkt_handle_t *pkt_info);

/****************************************************************************
* Function:   pkt_alloc_native
*
* Purpose:    Allocate network interface packet.
*
* Parameters: len (in) Length of packet to allocate.
*
* Returns:    Allocated packet.
*****************************************************************************
*/
void* pkt_alloc_native(osl_t *osh, unsigned int len);

/****************************************************************************
* Function:   pkt_free_native
*
* Purpose:    Free network interface packet.
*
* Parameters: native_pkt (in) Packet to free.
*
* Returns:    0 on success, else -1.
*****************************************************************************
*/
int pkt_free_native(osl_t *osh, void* native_pkt);

/****************************************************************************
* Function:   pkt_set_native_pkt_data
*
* Purpose:    Copy specified data contents into network interface packet.
*
* Parameters: native_pkt  (mod) Network interface packet.
*             data        (in)  Source data buffer.
*             len         (in)  Source data length.
*
* Returns:    0 on success, else -1.
*****************************************************************************
*/
int pkt_set_native_pkt_data(osl_t *osh, void* native_pkt, const uint8 *data, unsigned int len);

/****************************************************************************
* Function:   pkt_get_native_pkt_data
*
* Purpose:    Copy network interface packet contents to specified data buffer.
*
* Parameters: native_pkt  (mod) Network interface packet.
*             data        (in)  Destination data buffer.
*             len         (in)  Maximum destination data buffer length.
*
* Returns:    0 on success, else -1.
*****************************************************************************
*/
int pkt_get_native_pkt_data(osl_t *osh, const void* native_pkt, uint8 *data, unsigned int len);


/* ---- Inline Functions -------------------------------------------------- */


static INLINE void
pktsetlen(osl_t *osh, struct lbuf *lb, uint len)
{
	ASSERT(len + PKTHEADROOM(osh, lb) <= LBUFSZ);

	/* ASSERT(len >= 0); */
	lb->len = len;
	lb->tail = lb->data + len;
}

static INLINE uchar*
pktpush(osl_t *osh, struct lbuf *lb, uint bytes)
{
	if (bytes) {
		ASSERT(PKTHEADROOM(osh, lb) >= bytes);

		lb->data -= bytes;
		lb->len += bytes;
	}

	return (lb->data);
}

static INLINE uchar*
pktpull(osl_t *osh, struct lbuf *lb, uint bytes)
{
	if (bytes) {
		ASSERT(bytes <= lb->len);

		lb->data += bytes;
		lb->len -= bytes;
	}

	return (lb->data);
}


#ifdef __cplusplus
	}
#endif

#endif  /* _pkt_lbuf_h_  */
