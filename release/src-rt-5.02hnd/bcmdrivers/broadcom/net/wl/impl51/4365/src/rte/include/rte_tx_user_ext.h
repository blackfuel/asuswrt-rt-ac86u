/*
 * Threadx application support.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_tx_user_ext.h 483472 2014-06-09 22:49:49Z $
 */

#ifndef _rte_tx_user_ext_h_
#define _rte_tx_user_ext_h_

/* forward declarations */
struct TX_EVENT_FLAGS_GROUP_STRUCT;
struct timer_queue;

/* our extension */
#define TX_THREAD_USER_EXTENSION \
	struct TX_EVENT_FLAGS_GROUP_STRUCT *event; \
	struct timer_queue *timerq; \
	unsigned int (*coreid2event)(unsigned int coreid); \
	unsigned long tx_thread_exec_time;

#endif /* _rte_tx_user_ext_h_ */
