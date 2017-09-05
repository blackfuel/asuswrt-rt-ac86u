/*
 * PHY utils - nvram access functions.
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

#ifndef _phy_utils_var_h_
#define _phy_utils_var_h_

#include <typedefs.h>
#include <bcmdefs.h>

#include <wlc_phy_int.h>

/*
 * Search the name=value vars for a specific one and return its
 * value.  Returns NULL if not found.  This version of getvar uses a
 * phy specific instance of the vars.  The phy specific instance of
 * the get var routines guarantee that they are only used during
 * the execution of phy attach.  Any usage after this time will
 * assert/fail.  This is done so the Linux hybrid, where the top
 * of the driver released in source form and the bottom is released
 * as a linkable object file, protects against simple modification
 * of the vars string which might potentially affect regulatory
 * controlled aspects.  Linux hybrid builds also don't search NVRAM
 * if a name is not found in SROM.
 *
 * As an aid in locating any post wlc_phy_attach usage of
 * getvar/getintvar, a BCMDBG build passes the calling function
 * for output.
 */

char * phy_utils_getvar_internal(phy_info_t *pi, const char *name);
int phy_utils_getintvar_default(phy_info_t *pi, const char *name, int default_value);
#ifdef BCMDBG
char * phy_utils_getvar(phy_info_t *pi, const char *name, const char *function);
char * phy_utils_getvar_fabid(phy_info_t *pi, const char *name, const char *function);
char * phy_utils_getvar_fabid_internal(phy_info_t *pi, const char *name, const char *function);
int phy_utils_getintvar(phy_info_t *pi, const char *name, const char *function);
int phy_utils_getintvararray(phy_info_t *pi, const char *name, int idx, const char *function);
int phy_utils_getintvararray_default(phy_info_t *pi, const char *name, int idx, int default_value,
	const char *function);
int phy_utils_getintvararray_default_internal(phy_info_t *pi, const char *name, int idx,
	int default_value, const char *function);
#define PHY_GETVAR(pi, name) phy_utils_getvar_fabid(pi, name, __FUNCTION__)
/* Search the vars for a specific one and return its value as an integer. Returns 0 if not found */
#define PHY_GETINTVAR(pi, name) phy_utils_getintvar(pi, name, __FUNCTION__)
#define PHY_GETINTVAR_DEFAULT(pi, name, default_value) \
	phy_utils_getintvar_default(pi, name, default_value)
#define PHY_GETINTVAR_ARRAY(pi, name, idx) \
	phy_utils_getintvararray(pi, name, idx, __FUNCTION__)
#define PHY_GETINTVAR_ARRAY_DEFAULT(pi, name, idx, default_value) \
	phy_utils_getintvararray_default(pi, name, idx, default_value, __FUNCTION__)
#else
char * phy_utils_getvar(phy_info_t *pi, const char *name);
char * phy_utils_getvar_fabid(phy_info_t *pi, const char *name);
char * phy_utils_getvar_fabid_internal(phy_info_t *pi, const char *name);
int phy_utils_getintvar(phy_info_t *pi, const char *name);
int phy_utils_getintvararray(phy_info_t *pi, const char *name, int idx);
int phy_utils_getintvararray_default(phy_info_t *pi, const char *name, int idx, int default_value);
int phy_utils_getintvararray_default_internal(phy_info_t *pi, const char *name, int idx,
	int default_value);
#define PHY_GETVAR(pi, name)	phy_utils_getvar_fabid(pi, name)
#define PHY_GETINTVAR(pi, name)	phy_utils_getintvar(pi, name)
#define PHY_GETINTVAR_DEFAULT(pi, name, default_value) \
	phy_utils_getintvar_default(pi, name, default_value)
#define PHY_GETINTVAR_ARRAY(pi, name, idx) \
	phy_utils_getintvararray(pi, name, idx)
#define PHY_GETINTVAR_ARRAY_DEFAULT(pi, name, idx, default_value) \
	phy_utils_getintvararray_default(pi, name, idx, default_value)
#endif /* BCMDBG */

#endif /* _phy_utils_var_h_ */
