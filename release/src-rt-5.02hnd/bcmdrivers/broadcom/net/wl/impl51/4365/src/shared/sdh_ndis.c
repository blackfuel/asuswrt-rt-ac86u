/*
 * sdh_ndis.c: SDIO bus driver abstraction layer using
 * Windows SD bus driver interface to provides the bcmsdh API
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: sdh_ndis.c 467150 2014-04-02 17:30:43Z $
 */

#ifdef BCMSPI
#error "SPI defined"
#endif /* BCMSPI */

#include <typedefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <hndsoc.h>

#include <osl.h>
#include <initguid.h>
#include <ntddsd.h>

#include <siutils.h>
#include <bcmsrom.h>

#include <sdio.h>
#include <sbsdio.h>
#include <bcmsdh.h>

#if (NDISVER < 0x0620)
#error "NDIS version not supported"
#endif /* (NDISVER < 0x0620) */

/* SDIO Drive Strength (in milliamps) */
uint dhd_sdiod_drive_strength = 6;
void *def_sdos_info = NULL;
static int regfail = BCME_OK;
extern uint8 dbus_sdos_cfg_read(void *sdos_info, uint func_num, uint32 addr, int *err);
extern void dbus_sdos_cfg_write(void *sdos_info, uint func_num, uint32 addr, uint8 data, int *err);
extern int dbus_sdos_cis_read(void *sdos_info, uint func_num, uint8 *cis, uint32 length);
extern uint32 dbus_sdos_reg_write(void *sdos_info, uint32 addr, uint size, uint32 data);
extern uint32 dbus_sdos_reg_read(void *sdos_info, uint32 addr, uint size, uint32 *data);

uint
bcmsdh_query_iofnum(void *sdh)
{
	UNREFERENCED_PARAMETER(sdh);

	return 1;
}

int
bcmsdh_iovar_op(void *sdh, const char *name,
	void *params, int plen, void *arg, int len, bool set)
{
	return 0;

}

bool
bcmsdh_regfail(void *sdh)
{
	return (regfail != BCME_OK);
}

uint32
bcmsdh_get_dstatus(void *sdh)
{
	return 0;
}

uint8
bcmsdh_cfg_read(void *sdos_info, uint func_num, uint32 addr, int *err)
{
	return dbus_sdos_cfg_read(sdos_info, func_num, addr, err);
}

void
bcmsdh_cfg_write(void *sdos_info, uint func_num, uint32 addr, uint8 data, int *err)
{
	dbus_sdos_cfg_write(sdos_info, func_num, addr, data, err);
}

int
bcmsdh_cis_read(void *sdos_info, uint func_num, uint8 *cis, uint32 length)
{
	if (sdos_info == NULL)
		sdos_info = def_sdos_info;

	return dbus_sdos_cis_read(sdos_info, func_num, cis, length);
}

int
bcmsdh_intr_enable(void *sdos_info)
{
	return 0;
}

uint32
bcmsdh_reg_read(void *sdos_info, uint32 addr, uint length)
{
	uint32 data;

	if (sdos_info == NULL)
		sdos_info = def_sdos_info;

	regfail =  dbus_sdos_reg_read(sdos_info, addr, length, &data);

	return data;
}

uint32
bcmsdh_reg_write(void *sdos_info, uint32 addr, uint length, uint32 data)
{
	if (sdos_info == NULL)
		sdos_info = def_sdos_info;

	return dbus_sdos_reg_write(sdos_info, addr, length, data);
}
