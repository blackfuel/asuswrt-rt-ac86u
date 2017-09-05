/*
 * NDIS-specific portion of
 * Broadcom 802.11abg Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: bcmwlsdioif.h,v 1.1.2.1 2011-01-25 23:24:48 $
 */

#ifndef BCMWLSDIOIF_H
#define BCMWLSDIOIF_H

#include <wdf.h>
#include <WdfMiniport.h>
#include <initguid.h>
#include <ntddsd.h>

/*
 * Define an Interface Guid to access the proprietary sdio interface.
 * This guid is used to identify a specific interface in IRP_MN_QUERY_INTERFACE
 * handler.
 */
/* {5C279E39-D180-4e39-A1D4-65FB02B92472} */
DEFINE_GUID(GUID_BCMWLSDIO_INTERFACE_STANDARD,
0x5c279e39, 0xd180, 0x4e39, 0xa1, 0xd4, 0x65, 0xfb, 0x2, 0xb9, 0x24, 0x72);

#define VERSION_BCMWLSDIO_INTERFACE_STANDARD	1

/*
 * Define Interface reference/dereference routines for
 *  Interfaces exported by IRP_MN_QUERY_INTERFACE
 */

typedef VOID (*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID (*PINTERFACE_DEREFERENCE)(PVOID Context);

typedef
NTSTATUS
(*PSDBUSOPENINTERFACE)(
	PDEVICE_OBJECT  Pdo,
	PSDBUS_INTERFACE_STANDARD InterfaceStandard,
	USHORT Size,
	USHORT Version);

typedef
NTSTATUS
(*PSDBUSSUBMITREQUEST)(
	PVOID InterfaceContext,
	PSDBUS_REQUEST_PACKET Packet);

typedef
NTSTATUS
(*PSDBUSSUBMITREQUESTASYNC)(
	PVOID InterfaceContext,
	PSDBUS_REQUEST_PACKET Packet,
	PIRP Irp,
	PIO_COMPLETION_ROUTINE CompletionRoutine,
	PVOID UserContext);

/*
 * Interface for getting and setting power level etc.,
 */
typedef struct _BCMWLSDIO_INTERFACE_STANDARD {
    INTERFACE                   InterfaceHeader;
    PDEVICE_OBJECT		Pdo;
    PDEVICE_OBJECT		TargetObject;
    PSDBUSOPENINTERFACE		SdBusOpenInterface;
    PSDBUSSUBMITREQUEST		SdBusSubmitRequest;
    PSDBUSSUBMITREQUESTASYNC    SdBusSubmitRequestAsync;
} BCMWLSDIO_INTERFACE_STANDARD, *PBCMWLSDIO_INTERFACE_STANDARD;


#endif /* BCMWLSDIOIF_H */
