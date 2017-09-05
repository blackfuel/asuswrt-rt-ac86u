/*
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: autoupdt.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _autoupdt_h_
#define _autoupdt_h_

typedef enum tagAUTOSERVERMODE {
	e_LIVE = 0,
	e_DEBUG
} AUTOSERVERMODE;

typedef enum tagUPDATECHECK {
	e_UPDATE_CHECK_UP_TO_DATE = 0,
	e_UPDATE_CHECK_DOWNLOAD_AVAILABLE,
	e_UPDATE_CHECK_DOWNLOAD_AVAILABLE_RESUME,
	e_UPDATE_CHECK_SERVER_UNAVAILABLE,
	e_UPDATE_SERVER_ASSGINED_INSTANCEID
} UPDATECHECK;

typedef enum tagDOWNLOADSTATUS {
	e_IDLE = 0,
	e_DOWNLOADING,
	e_SUSPENDED
} DOWNLOADSTATUS;

typedef struct tagAUTO_UPDATE_STRUCT
{
	AUTOSERVERMODE eMode;
	CString strServerAddress;
	CString strInstanceId;
	CString strInstanceName;
	CString strProgId;
	CString strVersion;
	CString strCacheDir;

} AUTO_UPDATE_STRUCT, FAR * LPAUTO_UPDATE_STRUCT;

typedef struct tagINSTALL_INFO_STRUCT
{
	CString strFileSpec;
	DWORD dwExpectedSize;
	CString strVersion;
	BOOL bForced;

} INSTALL_INFO_STRUCT, FAR * LPINSTALL_INFO_STRUCT;

#endif /* _autoupdt_h_ */
