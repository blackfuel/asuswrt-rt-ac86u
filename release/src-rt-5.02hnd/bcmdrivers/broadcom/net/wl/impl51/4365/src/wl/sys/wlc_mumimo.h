/*
 * MU-MIMO private header file. Includes things that are needed by
 * both wlc_murx.h and wlc_mutx.h.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlc_mumimo.h 632135 2016-04-18 10:04:49Z $
 */

#ifndef _wlc_mumimo_h_
#define _wlc_mumimo_h_


#include <bcmutils.h>
#include <proto/802.11.h>

#define MU_CLIENT_INDEX_NONE  0xFFFF

#define MIMO_GROUP_NUM    (VHT_SIGA1_GID_MAX_GID + 1)

/* Maximum value for MU user position */
#define MU_USER_POS_MAX  3

/* MU group ID range. Groups 0 and 63 are reserved for SU. */
#define MU_GROUP_ID_MIN 1
#define MU_GROUP_ID_MAX (VHT_SIGA1_GID_MAX_GID - 1)

/* Can be used as an MU group ID wildcard. Same value as SU. */
#define MU_GROUP_ID_ANY 0

/* Maximum number of MU clients */
#define MUCLIENT_NUM  8

/* Minimum number of MU clients */
#define MUCLIENT_NUM_MIN  2
#define MUCLIENT_NUM_4  4
#define MUCLIENT_NUM_6  6

/* MU client scheduler duration */
#define MUCLIENT_SCHEDULER_DUR  60

/* Number of bytes in membership bit mask for a STA. */
#define MU_MEMBERSHIP_SIZE  (ROUNDUP(MIMO_GROUP_NUM, NBBY)/NBBY)

/* Number of bits used to identify a user position within a group. */
#define MU_POSITION_BIT_LEN  2

/* Number of bytes in user position bit string for a STA. */
#define MU_POSITION_SIZE  (ROUNDUP(MIMO_GROUP_NUM * MU_POSITION_BIT_LEN, NBBY)/NBBY)

#endif   /* _wlc_mumimo_h_ */
