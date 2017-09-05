/*
 * SES common port header file. 
 * Needed by vendors porting ses (client or configurator) to a different OS
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: ses_cmnport.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _ses_cmnport_h_
#define _ses_cmnport_h_

typedef int ses_timer_id;
typedef void (*ses_timer_cb)(ses_timer_id id, int data);

/* API between packet exchange common module and port specific file */

/* common ==> port specific */
int ses_open_socket(char *ifname, int *fd);
void ses_close_socket(int fd);
int ses_wait_for_packet(void *handle, int num_fd, int fd[], int timeout);
int ses_send_packet(int fd, uint8 *buf, int len);
int ses_wl_hwaddr(char *name, unsigned char *hwaddr);
void ses_random(uint8 *rand, int len);
int ses_timer_start(ses_timer_id *id, int time, ses_timer_cb cb, void *data);
void ses_timer_stop(ses_timer_id id);

/* port specific ==> common */
int ses_packet_dispatch(void *handle, int fd, uint8 *pkt, int len);

#endif /* _ses_cmnport_h_ */
