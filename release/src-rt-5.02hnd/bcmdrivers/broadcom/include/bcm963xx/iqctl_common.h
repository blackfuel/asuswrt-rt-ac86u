#ifndef __IQCTL_COMMON_H_INCLUDED__
#define __IQCTL_COMMON_H_INCLUDED__

/*
* <:copyright-BRCM:2007:proprietary:standard 
* 
*    Copyright (c) 2007 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>

*/


/*
 *******************************************************************************
 * File Name : ingqosctl.h
 *
 *******************************************************************************
 */

#define IQ_NAME                 "ingqos"
#define IQ_DRV_NAME             IQ_NAME
#define IQ_DRV_DEVICE_NAME      "/dev/" IQ_DRV_NAME

#define IQCTL_ERROR             (-1)
#define IQCTL_SUCCESS           0

/*
 * Ioctl definitions.
 */
typedef enum {
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
    IQCTL_IOCTL_SYS = 100,
    IQCTL_IOCTL_MAX
} iqctl_ioctl_t;

typedef enum {
    IQCTL_SUBSYS_STATUS,
    IQCTL_SUBSYS_DPORT,
    IQCTL_SUBSYS_DPORTTBL,
    IQCTL_SUBSYS_QUE,
    IQCTL_SUBSYS_MAX
} iqctl_subsys_t;

typedef enum {
    IQCTL_OP_GET,
    IQCTL_OP_SET,
    IQCTL_OP_ADD,
    IQCTL_OP_REM,
    IQCTL_OP_DUMP,
    IQCTL_OP_FLUSH,
    IQCTL_OP_MAX
} iqctl_op_t;

typedef enum {
    IQCTL_PROTO_TCP,
    IQCTL_PROTO_UDP,
    IQCTL_PROTO_MAX
} iqctl_proto_t;

typedef enum {
    IQCTL_ENT_DYN,
    IQCTL_ENT_STAT,
    IQCTL_ENT_MAX
} iqctl_ent_t;

typedef enum {
    IQCTL_PRIO_LOW,
    IQCTL_PRIO_HIGH,
    IQCTL_PRIO_MAX
} iqctl_prio_t;

typedef enum {
    IQCTL_CONG_STATUS_LO,
    IQCTL_CONG_STATUS_HI,
    IQCTL_CONG_STATUS_MAX
} iqctl_cong_status_t;

typedef enum {
    IQCTL_STATUS_DISABLE,
    IQCTL_STATUS_ENABLE,
    IQCTL_STATUS_MAX
} iqctl_status_t;

typedef struct {
    iqctl_subsys_t  subsys;
    iqctl_op_t      op;
    iqctl_status_t  status;
    iqctl_proto_t   proto;
    int             dport;
    iqctl_ent_t     ent;
    iqctl_prio_t    prio;
    int             nextIx;
    int             refcnt;
} iqctl_data_t;


#endif  /* defined(__IQCTL_COMMON_H_INCLUDED__) */
