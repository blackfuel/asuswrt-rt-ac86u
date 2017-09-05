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

#ifndef _BCM_OMCI_PM_API_
#define _BCM_OMCI_PM_API_

#include <linux/timer.h>

#include "bcmtypes.h"

/**
 * Parameter Tags indicating whether the parameter is an input, output, or input/output argument
 **/

#ifndef IN
#define IN
#endif /*IN*/

#ifndef OUT
#define OUT
#endif /*OUT*/

#ifndef INOUT
#define INOUT
#endif /*INOUT*/

/**
 * OMCI_PM Driver user API
 **/

#define OMCI_PM_DEVICE_NAME "bcmomcipm"
#define OMCI_PM_MAJOR 231   /* it should match with bcmomcipm device in targets/makeDevs */

/**
 * OMCI_PM Driver ioctl command IDs
 **/

#define BCM_OMCI_PM_IOCTL_GET_DRIVER_VERSION \
    _IOWR(OMCI_PM_MAJOR, 0, STS_OMCI_PM_DRIVER_VERSION)
#define BCM_OMCI_PM_IOCTL_REINITIALIZE \
    _IOWR(OMCI_PM_MAJOR, 1, STS_OMCI_PM_STATUS_ONLY)

/* Threshold Data */
#define BCM_OMCI_PM_IOCTL_CREATE_THRESHOLD_DATA \
    _IOWR(OMCI_PM_MAJOR, 2, STS_OMCI_PM_THRESHOLD_DATA)
#define BCM_OMCI_PM_IOCTL_DELETE_THRESHOLD_DATA \
    _IOWR(OMCI_PM_MAJOR, 3, STS_OMCI_PM_THRESHOLD_DATA)
#define BCM_OMCI_PM_IOCTL_GET_THRESHOLD_DATA \
    _IOWR(OMCI_PM_MAJOR, 4, STS_OMCI_PM_THRESHOLD_DATA)
#define BCM_OMCI_PM_IOCTL_SET_THRESHOLD_DATA \
    _IOWR(OMCI_PM_MAJOR, 5, STS_OMCI_PM_THRESHOLD_DATA)

/* GEM Port PM History Data */
#define BCM_OMCI_PM_IOCTL_CREATE_GEM_PORT \
    _IOWR(OMCI_PM_MAJOR, 6, STS_OMCI_PM_GEM_PORT)
#define BCM_OMCI_PM_IOCTL_DELETE_GEM_PORT \
    _IOWR(OMCI_PM_MAJOR, 7, STS_OMCI_PM_GEM_PORT)
#define BCM_OMCI_PM_IOCTL_GET_GEM_PORT \
    _IOWR(OMCI_PM_MAJOR, 8, STS_OMCI_PM_GEM_PORT)
#define BCM_OMCI_PM_IOCTL_GET_CURRENT_GEM_PORT \
    _IOWR(OMCI_PM_MAJOR, 9, STS_OMCI_PM_GEM_PORT)
#define BCM_OMCI_PM_IOCTL_SET_GEM_PORT \
    _IOWR(OMCI_PM_MAJOR, 10, STS_OMCI_PM_GEM_PORT)

/* FEC PM History Data */
#define BCM_OMCI_PM_IOCTL_CREATE_FEC \
    _IOWR(OMCI_PM_MAJOR, 11, STS_OMCI_PM_FEC)
#define BCM_OMCI_PM_IOCTL_DELETE_FEC \
    _IOWR(OMCI_PM_MAJOR, 12, STS_OMCI_PM_FEC)
#define BCM_OMCI_PM_IOCTL_GET_FEC \
    _IOWR(OMCI_PM_MAJOR, 13, STS_OMCI_PM_FEC)
#define BCM_OMCI_PM_IOCTL_GET_CURRENT_FEC \
    _IOWR(OMCI_PM_MAJOR, 14, STS_OMCI_PM_FEC)
#define BCM_OMCI_PM_IOCTL_SET_FEC \
    _IOWR(OMCI_PM_MAJOR, 15, STS_OMCI_PM_FEC)

/* Ethernet PM History Data */
#define BCM_OMCI_PM_IOCTL_CREATE_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 16, STS_OMCI_PM_ETHERNET)
#define BCM_OMCI_PM_IOCTL_DELETE_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 17, STS_OMCI_PM_ETHERNET)
#define BCM_OMCI_PM_IOCTL_GET_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 18, STS_OMCI_PM_ETHERNET)
#define BCM_OMCI_PM_IOCTL_GET_CURRENT_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 19, STS_OMCI_PM_ETHERNET)
#define BCM_OMCI_PM_IOCTL_SET_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 20, STS_OMCI_PM_ETHERNET)

/* Ethernet PM History Data 2 */
#define BCM_OMCI_PM_IOCTL_CREATE_ETHERNET_2 \
    _IOWR(OMCI_PM_MAJOR, 21, STS_OMCI_PM_ETHERNET_2)
#define BCM_OMCI_PM_IOCTL_DELETE_ETHERNET_2 \
    _IOWR(OMCI_PM_MAJOR, 22, STS_OMCI_PM_ETHERNET_2)
#define BCM_OMCI_PM_IOCTL_GET_ETHERNET_2 \
    _IOWR(OMCI_PM_MAJOR, 23, STS_OMCI_PM_ETHERNET_2)
#define BCM_OMCI_PM_IOCTL_GET_CURRENT_ETHERNET_2 \
    _IOWR(OMCI_PM_MAJOR, 24, STS_OMCI_PM_ETHERNET_2)
#define BCM_OMCI_PM_IOCTL_SET_ETHERNET_2 \
    _IOWR(OMCI_PM_MAJOR, 25, STS_OMCI_PM_ETHERNET_2)

/* Ethernet PM History Data 3 */
#define BCM_OMCI_PM_IOCTL_CREATE_ETHERNET_3 \
    _IOWR(OMCI_PM_MAJOR, 26, STS_OMCI_PM_ETHERNET_3)
#define BCM_OMCI_PM_IOCTL_DELETE_ETHERNET_3 \
    _IOWR(OMCI_PM_MAJOR, 27, STS_OMCI_PM_ETHERNET_3)
#define BCM_OMCI_PM_IOCTL_GET_ETHERNET_3 \
    _IOWR(OMCI_PM_MAJOR, 28, STS_OMCI_PM_ETHERNET_3)
#define BCM_OMCI_PM_IOCTL_GET_CURRENT_ETHERNET_3 \
    _IOWR(OMCI_PM_MAJOR, 29, STS_OMCI_PM_ETHERNET_3)
#define BCM_OMCI_PM_IOCTL_SET_ETHERNET_3 \
    _IOWR(OMCI_PM_MAJOR, 30, STS_OMCI_PM_ETHERNET_3)

/* MoCA Ethernet PM History Data */
#define BCM_OMCI_PM_IOCTL_CREATE_MOCA_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 31, STS_OMCI_PM_MOCA_ETHERNET)
#define BCM_OMCI_PM_IOCTL_DELETE_MOCA_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 32, STS_OMCI_PM_MOCA_ETHERNET)
#define BCM_OMCI_PM_IOCTL_GET_MOCA_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 33, STS_OMCI_PM_MOCA_ETHERNET)
#define BCM_OMCI_PM_IOCTL_SET_MOCA_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 34, STS_OMCI_PM_MOCA_ETHERNET)

/* MoCA Interface PM History Data */
#define BCM_OMCI_PM_IOCTL_CREATE_MOCA_INTERFACE \
    _IOWR(OMCI_PM_MAJOR, 35, STS_OMCI_PM_MOCA_INTERFACE)
#define BCM_OMCI_PM_IOCTL_DELETE_MOCA_INTERFACE \
    _IOWR(OMCI_PM_MAJOR, 36, STS_OMCI_PM_MOCA_INTERFACE)
#define BCM_OMCI_PM_IOCTL_GET_MOCA_INTERFACE \
    _IOWR(OMCI_PM_MAJOR, 37, STS_OMCI_PM_MOCA_INTERFACE)
#define BCM_OMCI_PM_IOCTL_SET_MOCA_INTERFACE \
    _IOWR(OMCI_PM_MAJOR, 38, STS_OMCI_PM_MOCA_INTERFACE)

/* MoCA Interface Entry PM History Data */
#define BCM_OMCI_PM_IOCTL_GET_MOCA_INTERFACE_ENTRY \
    _IOWR(OMCI_PM_MAJOR, 39, STS_OMCI_PM_MOCA_INTERFACE_ENTRY)
#define BCM_OMCI_PM_IOCTL_GET_NEXT_MOCA_INTERFACE_ENTRY \
    _IOWR(OMCI_PM_MAJOR, 40, STS_OMCI_PM_MOCA_INTERFACE_ENTRY)

/* GAL Ethernet PM History Data */
#define BCM_OMCI_PM_IOCTL_CREATE_GAL_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 41, STS_OMCI_PM_GAL_ETHERNET)
#define BCM_OMCI_PM_IOCTL_DELETE_GAL_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 42, STS_OMCI_PM_GAL_ETHERNET)
#define BCM_OMCI_PM_IOCTL_GET_GAL_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 43, STS_OMCI_PM_GAL_ETHERNET)
#define BCM_OMCI_PM_IOCTL_GET_CURRENT_GAL_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 44, STS_OMCI_PM_GAL_ETHERNET)
#define BCM_OMCI_PM_IOCTL_SET_GAL_ETHERNET \
    _IOWR(OMCI_PM_MAJOR, 45, STS_OMCI_PM_GAL_ETHERNET)

/* MAC bridge PM History Data */
#define BCM_OMCI_PM_IOCTL_CREATE_MAC_BRIDGE \
    _IOWR(OMCI_PM_MAJOR, 46, STS_OMCI_PM_MAC_BRIDGE)
#define BCM_OMCI_PM_IOCTL_DELETE_MAC_BRIDGE \
    _IOWR(OMCI_PM_MAJOR, 47, STS_OMCI_PM_MAC_BRIDGE)
#define BCM_OMCI_PM_IOCTL_GET_MAC_BRIDGE \
    _IOWR(OMCI_PM_MAJOR, 48, STS_OMCI_PM_MAC_BRIDGE)
#define BCM_OMCI_PM_IOCTL_GET_CURRENT_MAC_BRIDGE \
    _IOWR(OMCI_PM_MAJOR, 49, STS_OMCI_PM_MAC_BRIDGE)
#define BCM_OMCI_PM_IOCTL_SET_MAC_BRIDGE \
    _IOWR(OMCI_PM_MAJOR, 50, STS_OMCI_PM_MAC_BRIDGE)

#define MAX_BCM_OMCI_PM_IOCTL_COMMANDS   51

/* OMCI PM Class ID*/
#define BCM_OMCI_PM_CLASS_GEM_PORT      267
#define BCM_OMCI_PM_CLASS_FEC           312
#define BCM_OMCI_PM_CLASS_ENET          24
#define BCM_OMCI_PM_CLASS_ENET_2        89
#define BCM_OMCI_PM_CLASS_ENET_3        296
#define BCM_OMCI_PM_CLASS_MOCA_ENET     163
#define BCM_OMCI_PM_CLASS_MOCA_IF       164
#define BCM_OMCI_PM_CLASS_GAL_ENET      276
#define BCM_OMCI_PM_CLASS_BRIDGE        51
#define BCM_OMCI_PM_CLASS_BRIDGE_PORT   52

/**
 * Typedefs.
 **/

/* Return status values. */
typedef enum OMCI_PM_Status
{
    OMCI_PM_STATUS_SUCCESS = 0,
    OMCI_PM_STATUS_INIT_FAILED,
    OMCI_PM_STATUS_ERROR,
    OMCI_PM_STATUS_LOAD_ERROR,
    OMCI_PM_STATUS_STATE_ERROR,
    OMCI_PM_STATUS_PARAMETER_ERROR,
    OMCI_PM_STATUS_ALLOC_ERROR,
    OMCI_PM_STATUS_RESOURCE_ERROR,
    OMCI_PM_STATUS_IN_USE,
    OMCI_PM_STATUS_NOT_FOUND,
    OMCI_PM_STATUS_NOT_SUPPORTED,
    OMCI_PM_STATUS_NOT_REGISTERED,
    OMCI_PM_STATUS_TIMEOUT
} BCM_OMCI_PM_STATUS;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
} STS_OMCI_PM_STATUS_ONLY, *PSTS_OMCI_PM_STATUS_ONLY;

typedef struct
{
    OUT UINT8 driverMajor;
    OUT UINT8 driverMinor;
    OUT UINT8 driverFix;
    OUT UINT8 apiMajor;
    OUT UINT8 apiMinor;
} BCM_OMCI_PM_DRIVER_VERSION, *PBCM_OMCI_PM_DRIVER_VERSION;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
    BCM_OMCI_PM_DRIVER_VERSION    version;
} STS_OMCI_PM_DRIVER_VERSION, *PSTS_OMCI_PM_DRIVER_VERSION;

/* Threshold Data */
typedef struct
{
    INOUT UINT16 thresholdId;
    INOUT UINT32 thresholdValue1;
    INOUT UINT32 thresholdValue2;
    INOUT UINT32 thresholdValue3;
    INOUT UINT32 thresholdValue4;
    INOUT UINT32 thresholdValue5;
    INOUT UINT32 thresholdValue6;
    INOUT UINT32 thresholdValue7;
    INOUT UINT32 thresholdValue8;
    INOUT UINT32 thresholdValue9;
    INOUT UINT32 thresholdValue10;
    INOUT UINT32 thresholdValue11;
    INOUT UINT32 thresholdValue12;
    INOUT UINT32 thresholdValue13;
    INOUT UINT32 thresholdValue14;
} BCM_OMCI_PM_THRESHOLD_DATA, *PBCM_OMCI_PM_THRESHOLD_DATA;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
    BCM_OMCI_PM_THRESHOLD_DATA    thresholdData;
} STS_OMCI_PM_THRESHOLD_DATA, *PSTS_OMCI_PM_THRESHOLD_DATA;

typedef struct 
{
    INOUT UINT16 omcipmId;
    OUT   UINT8  intervalEndTime;
    INOUT UINT16 thresholdId;
} BCM_OMCI_PM, *PBCM_OMCI_PM;

typedef struct
{
    BCM_OMCI_PM                   omcipm;
} BCM_OMCI_PM_ID_ONLY, *PBCM_OMCI_PM_ID_ONLY;

/* GEM Port PM History Data */
typedef struct 
{
    BCM_OMCI_PM  omcipm;
    OUT   UINT32 lostPackets;          /*Partially Compliant: counts aggregate uncorrectable HEC. Not buffer overflows*/
    OUT   UINT32 misinsertedPackets;   /*Not Compliant - Always returns 0*/
    OUT   UINT32 receivedPackets;      /*Compliant*/
    OUT   UINT32 receivedBlocks;       /*Compliant*/
    OUT   UINT32 transmittedBlocks;    /*Compliant*/
    OUT   UINT32 impairedBlocks;       /*Not Compliant - Always returns 0*/
    OUT   UINT32 transmittedPackets;   /*Compliant - Non-standard Counter*/
} BCM_OMCI_PM_GEM_PORT, *PBCM_OMCI_PM_GEM_PORT;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
    BCM_OMCI_PM_GEM_PORT          omcipmGemPort;
} STS_OMCI_PM_GEM_PORT, *PSTS_OMCI_PM_GEM_PORT;

/* FEC PM History Data */
typedef struct 
{
    BCM_OMCI_PM  omcipm;
    OUT   UINT32 correctedBytes;         /*Not Compliant. Always returns 0*/
    OUT   UINT32 correctedCodeWords;     /*Compliant*/
    OUT   UINT32 uncorrectedCodeWords;   /*Compliant*/
    OUT   UINT32 totalCodeWords;         /*Compliant*/
    OUT   UINT32 fecSeconds;             /*Compliant*/
} BCM_OMCI_PM_FEC, *PBCM_OMCI_PM_FEC;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
    BCM_OMCI_PM_FEC               omcipmFec;
} STS_OMCI_PM_FEC, *PSTS_OMCI_PM_FEC;

/* Ethernet PM History Data */
typedef struct 
{
    BCM_OMCI_PM  omcipm;
    OUT   UINT32 fcsErrors;
    OUT   UINT32 excessiveCollisionCounter;
    OUT   UINT32 lateCollisionCounter;
    OUT   UINT32 frameTooLongs;
    OUT   UINT32 bufferOverflowsOnReceive;
    OUT   UINT32 bufferOverflowsOnTransmit;
    OUT   UINT32 singleCollisionFrameCounter;
    OUT   UINT32 multipleCollisionsFrameCounter;
    OUT   UINT32 sqeCounter;
    OUT   UINT32 deferredTransmissionCounter;
    OUT   UINT32 internalMacTransmitErrorCounter;
    OUT   UINT32 carrierSenseErrorCounter;
    OUT   UINT32 alignmentErrorCounter;
    OUT   UINT32 internalMacReceiveErrorCounter;
} BCM_OMCI_PM_ETHERNET, *PBCM_OMCI_PM_ETHERNET;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
    BCM_OMCI_PM_ETHERNET          omcipmEthernet;
} STS_OMCI_PM_ETHERNET, *PSTS_OMCI_PM_ETHERNET;

/* Ethernet PM History Data 2 */
typedef struct 
{
    BCM_OMCI_PM  omcipm;
    OUT   UINT32 pppoeFilterFrameCounter;
} BCM_OMCI_PM_ETHERNET_2, *PBCM_OMCI_PM_ETHERNET_2;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
    BCM_OMCI_PM_ETHERNET_2        omcipmEthernet2;
} STS_OMCI_PM_ETHERNET_2, *PSTS_OMCI_PM_ETHERNET_2;

/* Ethernet PM History Data 3 */
typedef struct 
{
    BCM_OMCI_PM  omcipm;
    OUT   UINT32 dropEvents;
    OUT   UINT32 octets;
    OUT   UINT32 packets;
    OUT   UINT32 broadcastPackets;
    OUT   UINT32 multicastPackets;
    OUT   UINT32 undersizePackets;
    OUT   UINT32 fragments;
    OUT   UINT32 jabbers;
    OUT   UINT32 packets64Octets;
    OUT   UINT32 packets127Octets;
    OUT   UINT32 packets255Octets;
    OUT   UINT32 packets511Octets;
    OUT   UINT32 packets1023Octets;
    OUT   UINT32 packets1518Octets;
} BCM_OMCI_PM_ETHERNET_3, *PBCM_OMCI_PM_ETHERNET_3;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
    BCM_OMCI_PM_ETHERNET_3        omcipmEthernet3;
} STS_OMCI_PM_ETHERNET_3, *PSTS_OMCI_PM_ETHERNET_3;

/* MoCA Ethernet PM History Data */
typedef struct 
{
    BCM_OMCI_PM  omcipm;
    OUT   UINT32 incomingUnicastPackets;
    OUT   UINT32 incomingDiscardedPackets;
    OUT   UINT32 incomingErroredPackets;
    OUT   UINT32 incomingUnknownPackets;
    OUT   UINT32 incomingMulticastPackets;
    OUT   UINT32 incomingBroadcastPackets;
    OUT   UINT32 incomingOctets;
    OUT   UINT32 outgoingUnicastPackets;
    OUT   UINT32 outgoingDiscardedPackets;
    OUT   UINT32 outgoingErroredPackets;
    OUT   UINT32 outgoingUnknownPackets;
    OUT   UINT32 outgoingMulticastPackets;
    OUT   UINT32 outgoingBroadcastPackets;
    OUT   UINT32 outgoingOctets;
} BCM_OMCI_PM_MOCA_ETHERNET, *PBCM_OMCI_PM_MOCA_ETHERNET;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
    BCM_OMCI_PM_MOCA_ETHERNET     omcipmMocaEthernet;
} STS_OMCI_PM_MOCA_ETHERNET, *PSTS_OMCI_PM_MOCA_ETHERNET;

/* MoCA Interface PM History Data */
typedef struct 
{
    BCM_OMCI_PM  omcipm;
    OUT   UINT32 phyTxBroadcastRate;
} BCM_OMCI_PM_MOCA_INTERFACE;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
    BCM_OMCI_PM_MOCA_INTERFACE    omcipmMocaInterface;
} STS_OMCI_PM_MOCA_INTERFACE, *PSTS_OMCI_PM_MOCA_INTERFACE;

/* MoCA Interface Entry PM History Data */
typedef struct 
{
    BCM_OMCI_PM  omcipm;
    OUT   UINT32 phyTxRate;
    OUT   UINT32 txPowerControlReduction;
    OUT   UINT32 phyRxRate;
    OUT   UINT32 rxPowerLevel;
    OUT   UINT32 phyRxBroadcastRate;
    OUT   UINT32 rxBroadcastPowerLevel;
    OUT   UINT32 txPackets;
    OUT   UINT32 rxPackets;
    OUT   UINT32 erroredMissedRxPackets;
    OUT   UINT32 erroredRxPackets;
} BCM_OMCI_PM_MOCA_INTERFACE_ENTRY, *PBCM_OMCI_PM_MOCA_INTERFACE_ENTRY;

typedef struct
{
    BCM_OMCI_PM_STATUS                  omcipmStatus;
    IN    UINT16                        omcipmMocaInterfaceId;
    BCM_OMCI_PM_MOCA_INTERFACE_ENTRY    omcipmMocaInterfaceEntry;
} STS_OMCI_PM_MOCA_INTERFACE_ENTRY, *PSTS_OMCI_PM_MOCA_INTERFACE_ENTRY;

/* GAL Ethernet PM History Data */
typedef struct 
{
    BCM_OMCI_PM  omcipm;
    OUT   UINT32 discardedFrames;     /*Not Compliant. Always returns 0*/
    OUT   UINT32 transmittedFrames;   /*Compliant - Non-standard Counter*/
    OUT   UINT32 receivedFrames;      /*Compliant - Non-standard Counter*/
} BCM_OMCI_PM_GAL_ETHERNET, *PBCM_OMCI_PM_GAL_ETHERNET;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
    BCM_OMCI_PM_GAL_ETHERNET      omcipmGalEthernet;
} STS_OMCI_PM_GAL_ETHERNET, *PSTS_OMCI_PM_GAL_ETHERNET;

/* MAC bridge PM History Data */
typedef struct 
{
    BCM_OMCI_PM  omcipm;
    OUT   UINT32 discardCount;
} BCM_OMCI_PM_MAC_BRIDGE, *PBCM_OMCI_PM_MAC_BRIDGE;

typedef struct
{
    BCM_OMCI_PM_STATUS            omcipmStatus;
    BCM_OMCI_PM_MAC_BRIDGE        omcipmBridge;
} STS_OMCI_PM_MAC_BRIDGE, *PSTS_OMCI_PM_MAC_BRIDGE;

/* Max number of Threshold Crossing Alert */
#define BCM_OMCI_PM_TCA_MAX   100

/* Threshold Crossing Alert */
typedef struct
{
    OUT UINT16 classId;
    OUT UINT16 instanceId;
    OUT UINT16 tcaMask;
} BCM_OMCI_PM_TCA, *PBCM_OMCI_PM_TCA;


#endif /*_BCM_OMCI_PM_API_*/
