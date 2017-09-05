/*
 * Required functions exported by the wlc_led.c
 * to common (os-independent) driver code.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_led.h 541876 2015-03-18 04:12:11Z $
 */


#ifndef _wlc_led_h_
#define _wlc_led_h_


#ifdef WLLED

#define	LED_BLINK_TIME		10	/* 10ms wlc_led_blink_timer() period */
/* PMU override bit starting point for the GPIO line controlling the LED */
#define PMU_CCA1_OVERRIDE_BIT_GPIO0	16

struct bmac_led {
	uint	pin;		/* gpio pin# == led index# */
	bool	pin_ledbh;	/* gpio pin is defined by nvram ledbh */
	bool	activehi;	/* led behavior of this pin */
	uint32	msec_on;	/* milliseconds or timer ticks on */
	uint32	msec_off;	/* milliseconds or timer ticks off */
#if OSL_SYSUPTIME_SUPPORT
	uint32	timestamp;	/* OSL_SYSUPTIME of the last action */
	bool	next_state;	/*  transitioning from on->off or off->on? */
	bool 	restart;	/* start the LED blinking from the beginning of ON cycle */
#else
	int32	blinkmsec;	/* total number of on/off ticks */
#endif
};

struct bmac_led_info {
	void		*wlc_hw;
	struct bmac_led	led[WL_LED_NUMGPIO];	/* led fanciness */
	uint32		gpioout_cache;		/* cache the gpio pin values */
	uint32		gpiomask_cache;		/* cache the gpio mask */
	uint		led_blink_time;		/* timer blink interval */
	struct wl_timer *led_blink_timer;	/* led_blink_time duration (ms) led blink timer */
	bool 		blink_start;
	bool		blink_adjust;
};

#define WLACTINCR(a)		((a)++) /* Increment by 1 */
extern led_info_t *wlc_led_attach(wlc_info_t *wlc);
extern int wlc_led_detach(led_info_t *ledh);
extern void wlc_led_init(led_info_t *ledh);
extern void wlc_led_deinit(led_info_t *ledh);
extern int wlc_led_event(led_info_t *ledh);
extern int wlc_led_set(led_info_t *ledh, wl_led_info_t *ed);
extern void wlc_led_radioset(led_info_t *ledh, bool led_state);
extern void wlc_led_activityset(led_info_t *ledh, bool led_state);
extern void wlc_led_up(wlc_info_t *wlc);
extern uint wlc_led_down(wlc_info_t *wlc);
extern uint wlc_led_start_activity_timer(led_info_t *ledh);
extern uint wlc_led_stop_activity_timer(led_info_t *ledh);

#else
#define WLACTINCR(a)
#define wlc_led_attach(a, b)				(led_info_t *)0x0dadbeef
static INLINE int wlc_led_detach(led_info_t *ledh)	{ return 0; }
#define wlc_led_init(a)					do {} while (0)
#define wlc_led_deinit(a)				do {} while (0)
#define wlc_led_event(a)				do {} while (0)
#define wlc_led_set(a, b)				do {} while (0)
#define wlc_led_radioset(a, b)				do {} while (0)
#define wlc_led_activityset(a, b)			do {} while (0)
#define wlc_led_up(a)					do {} while (0)
#define wlc_led_down(a)					0
#endif /* WLLED */

#endif	/* _wlc_led_h_ */
