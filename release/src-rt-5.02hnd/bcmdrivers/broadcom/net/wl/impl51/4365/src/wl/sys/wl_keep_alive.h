/*
 * Keep-alive offloading.
 *
 *	This feature implements periodic keep-alive packet transmission offloading.
 * The intended purpose is to keep an active session within a network address
 * translator (NAT) with a public server. This allows incoming packets sent
 * by the public server to the STA to traverse the NAT.
 *
 * An example application is to keep an active session between the STA and
 * a call control server in order for the STA to be able to receive incoming
 * voice calls.
 *
 * The keep-alive functionality is offloaded from the host processor to the
 * WLAN processor to eliminate the need for the host processor to wake-up while
 * it is idle; therefore, conserving power.
 *
 *
 *   Broadcom Proprietary and Confidential. Copyright (C) 2017,
 *   All Rights Reserved.
 *   
 *   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 *   the contents of this file may not be disclosed to third parties, copied
 *   or duplicated in any form, in whole or in part, without the prior
 *   written permission of Broadcom.
 *
 *   $Id: wl_keep_alive.h 467328 2014-04-03 01:23:40Z $
 */


#ifndef _wl_keep_alive_h_
#define _wl_keep_alive_h_


/* ---- Include Files ---------------------------------------------------- */
/* ---- Constants and Types ---------------------------------------------- */

/* Forward declaration */
typedef struct wl_keep_alive_info wl_keep_alive_info_t;

#define WL_MKEEP_ALIVE_IDMAX		3


/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef KEEP_ALIVE

/*
*****************************************************************************
* Function:   wl_keep_alive_attach
*
* Purpose:    Initialize keep-alive private context.
*
* Parameters: wlc	(mod)	Common driver context.
*
* Returns:    Pointer to the keep-alive private context. Returns NULL on error.
*****************************************************************************
*/
extern wl_keep_alive_info_t *wl_keep_alive_attach(wlc_info_t *wlc);


/*
*****************************************************************************
* Function:   wl_keep_alive_detach
*
* Purpose:    Cleanup keep-alive private context.
*
* Parameters: info	(mod)	Keep-alive private context.
*
* Returns:    Nothing.
*****************************************************************************
*/
extern void wl_keep_alive_detach(wl_keep_alive_info_t *info);


/*
*****************************************************************************
* Function:   wl_keep_alive_up
*
* Purpose:	  Install periodic timer.
*
* Parameters: info	(mod)	Keep-alive private context.
*
* Returns:    0 on success.
*****************************************************************************
*/
extern int wl_keep_alive_up(wl_keep_alive_info_t *info);


/*
*****************************************************************************
* Function:   wl_keep_alive_down
*
* Purpose:    Cancel periodic timer.
*
* Parameters: info	(mod)	Keep-alive private context.
*
* Returns:    Number of periodic timers canceled..
*****************************************************************************
*/
extern unsigned int wl_keep_alive_down(wl_keep_alive_info_t *info);

extern int wl_keep_alive_upd_override_period(wlc_info_t *wlc, uint8 mkeepalive_index,
	uint32 override_period);
#else	/* stubs */

#define wl_keep_alive_attach(a)		(wl_keep_alive_info_t *)0x0dadbeef
#define wl_keep_alive_detach(a)		do {} while (0)
#define wl_keep_alive_up(a)		(0)
#define wl_keep_alive_down(a)		((void)(0))

#endif /* KEEP_ALIVE */

#endif	/* _wl_keep_alive_h_ */
