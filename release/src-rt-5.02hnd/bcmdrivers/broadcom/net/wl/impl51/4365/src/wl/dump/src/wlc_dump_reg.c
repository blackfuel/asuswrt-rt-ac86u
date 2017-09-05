/*
 * Named dump callback registry functions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <wl_dbg.h>
#include <wlc_dump_reg.h>


#ifndef WLC_DUMP_REG_STR_SIZE
#define WLC_DUMP_REG_STR_SIZE	16
#endif

typedef struct {
	char 	name[WLC_DUMP_REG_STR_SIZE];
	wlc_dump_reg_fn_t	fn;
	const void	*ctx;
} wlc_dump_reg_ent_t;

struct wlc_dump_reg_info {
	uint16	cur;
	uint16	max;
	wlc_dump_reg_ent_t	ent[1];
};

/* create a registry with 'count' entries */
wlc_dump_reg_info_t *
wlc_dump_reg_create(osl_t *osh, uint16 count)
{
	wlc_dump_reg_info_t *reg;

	if (count == 0)
		return NULL;

	reg = MALLOCZ(osh, OFFSETOF(wlc_dump_reg_info_t, ent) + sizeof(wlc_dump_reg_ent_t)*count);
	if (!reg) {
		WL_ERROR(("%s: MALLOC failed; total mallocs %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		return NULL;
	}
	reg->cur = 0;
	reg->max = count;

	return reg;
}

/* destroy a registry */
void
wlc_dump_reg_destroy(osl_t *osh, wlc_dump_reg_info_t *reg)
{
	if (reg == NULL)
		return;

	MFREE(osh, reg, OFFSETOF(wlc_dump_reg_info_t, ent) + sizeof(wlc_dump_reg_ent_t)*reg->max);
}

/* look up a name in a registry */
static int
wlc_dump_reg_lookup(wlc_dump_reg_info_t *reg, char *name)
{
	int i;

	ASSERT(reg);
	if (!reg)
		return BCME_ERROR;

	for (i = 0; i < reg->cur; i++) {
		if (!strcmp(name, reg->ent[i].name))
			return i;
	}

	return BCME_NOTFOUND;
}

/* add a name and its callback function to a registry */
int
wlc_dump_reg_add_fn(wlc_dump_reg_info_t *reg, char *name, wlc_dump_reg_fn_t fn, const void *ctx)
{
	int ret;
	int namelen;

	ASSERT(reg);
	ASSERT(name);
	ASSERT(fn);
	if (!reg || !name || !fn)
		return BCME_ERROR;

	/* do not add to table if namelen is 0 or longer than WLC_DUMP_REG_STR_SIZE */
	namelen = strlen(name);
	if (namelen == 0 || namelen >= WLC_DUMP_REG_STR_SIZE) {
		WL_ERROR(("%s: name len should be in [1, %d]\n",
			__FUNCTION__, WLC_DUMP_REG_STR_SIZE - 1));
		return BCME_BADARG;
	}

	ret = wlc_dump_reg_lookup(reg, name);
	if (ret == BCME_NOTFOUND) {
		if (reg->cur < reg->max) {
			strncpy(reg->ent[reg->cur].name, name, namelen + 1);
			reg->ent[reg->cur].fn = fn;
			reg->ent[reg->cur].ctx = ctx;
			reg->cur++;
			ret = BCME_OK;
		}
		else {
			WL_ERROR(("%s: registry is full\n", __FUNCTION__));
			ret = BCME_NORESOURCE;
		}
	}
	else if (ret >= 0) {
		WL_ERROR(("%s: %s already in registry\n", name, __FUNCTION__));
		ret = BCME_ERROR;
	}

	return ret;
}

/* invoke a callback function in a registry by name */
int
wlc_dump_reg_invoke_fn(wlc_dump_reg_info_t *reg, char *name, void *arg)
{
	int ret;

	ASSERT(reg);
	ASSERT(name);
	if (!reg || !name)
		return BCME_ERROR;

	ret = wlc_dump_reg_lookup(reg, name);
	if (ret >= 0) {
		ASSERT(reg->ent[ret].fn != NULL);
		return (reg->ent[ret].fn)(reg->ent[ret].ctx, arg);
	}

	return ret;
}
