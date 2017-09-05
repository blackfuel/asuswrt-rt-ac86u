/*
 * Common code for visualization system
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: vis_common.c 617465 2016-02-05 10:52:38Z $
 */

#ifdef WIN32
#include <windows.h>
#endif

#include "vis_common.h"
#include "vis_wl.h"
#include <bcmendian.h>
#include "vis_struct.h"

const char *g_wlu_av0;

/* dword align allocation */
union {
	char bufdata[WLC_IOCTL_MAXLEN];
	uint32 alignme;
} bufstruct_wlu;
char *g_vis_cmdoutbuf = (char*) &bufstruct_wlu.bufdata;

extern int wl_get(void *wl, int cmd, void *buf, int len);
extern int wl_set(void *wl, int cmd, void *buf, int len);

/* now IOCTL GET commands shall call wlu_get() instead of wl_get() so that the commands
 * can be batched when needed
 */
int
wlu_get(void *wl, int cmd, void *cmdbuf, int len)
{
	return wl_get(wl, cmd, cmdbuf, len);
}

/*
 * format an iovar buffer
 * iovar name is converted to lower case
 */
static uint
wl_iovar_mkbuf(const char *name, char *data, uint datalen,
	char *iovar_buf, uint buflen, int *perr)
{
	uint iovar_len;
	char *p;

	iovar_len = strlen(name) + 1;

	/* check for overflow */
	if ((iovar_len + datalen) > buflen) {
		*perr = BCME_BUFTOOSHORT;
		return 0;
	}

	/* copy data to the buffer past the end of the iovar name string */
	if (datalen > 0)
		memmove(&iovar_buf[iovar_len], data, datalen);

	/* copy the name to the beginning of the buffer */
	strcpy(iovar_buf, name);

	/* wl command line automatically converts iovar names to lower case for
	 * ease of use
	 */
	p = iovar_buf;
	while (*p != '\0') {
		*p = tolower((int)*p);
		p++;
	}

	*perr = 0;
	return (iovar_len + datalen);
}


/*
 * get named iovar providing both parameter and i/o buffers
 * iovar name is converted to lower case
 */
int
wlu_iovar_getbuf(void* wl, const char *iovar,
	void *param, int paramlen, void *bufptr, int buflen)
{
	int err;

	wl_iovar_mkbuf(iovar, param, paramlen, bufptr, buflen, &err);
	if (err)
		return err;

	return wlu_get(wl, WLC_GET_VAR, bufptr, buflen);
}

/*
 * get named iovar without parameters into a given buffer
 * iovar name is converted to lower case
 */
int
wlu_iovar_get(void *wl, const char *iovar, void *outbuf, int len)
{
	char smbuf[WLC_IOCTL_SMLEN];
	int err;

	/* use the return buffer if it is bigger than what we have on the stack */
	if (len > (int)sizeof(smbuf)) {
		err = wlu_iovar_getbuf(wl, iovar, NULL, 0, outbuf, len);
	} else {
		memset(smbuf, 0, sizeof(smbuf));
		err = wlu_iovar_getbuf(wl, iovar, NULL, 0, smbuf, sizeof(smbuf));
		if (err == 0)
			memcpy(outbuf, smbuf, len);
	}

	return err;
}

/* now IOCTL SET commands shall call wlu_set() instead of wl_set() so that the commands
 * can be batched when needed
 */
int
wlu_set(void *wl, int cmd, void *cmdbuf, int len)
{
	return wl_set(wl, cmd, cmdbuf, len);
}

/* Get buffer for medium sizes upto 1500 bytes */
int
wlu_var_getbuf_med(void *wl, const char *iovar, void *param, int param_len, void **bufptr)
{
	int len;

	memset(g_vis_cmdoutbuf, 0, WLC_IOCTL_MEDLEN);
	strncpy(g_vis_cmdoutbuf, iovar, WLC_IOCTL_MEDLEN);

	/* include the null */
	len = strlen(iovar) + 1;

	if (param_len)
		memcpy(&g_vis_cmdoutbuf[len], param, param_len);

	*bufptr = g_vis_cmdoutbuf;

	return wlu_get(wl, WLC_GET_VAR, &g_vis_cmdoutbuf[0], WLC_IOCTL_MEDLEN);
}

int
wlu_var_getbuf(void *wl, const char *iovar, void *param, int param_len, void **bufptr)
{
	int len;

	memset(g_vis_cmdoutbuf, 0, WLC_IOCTL_MAXLEN);
	strncpy(g_vis_cmdoutbuf, iovar, WLC_IOCTL_MAXLEN);

	/* include the null */
	len = strlen(iovar) + 1;

	if (param_len)
		memcpy(&g_vis_cmdoutbuf[len], param, param_len);

	*bufptr = g_vis_cmdoutbuf;

	return wlu_get(wl, WLC_GET_VAR, &g_vis_cmdoutbuf[0], WLC_IOCTL_MAXLEN);
}

int
wlu_var_setbuf(void *wl, const char *iovar, void *param, int param_len)
{
	int len;

	memset(g_vis_cmdoutbuf, 0, WLC_IOCTL_MAXLEN);
	strcpy(g_vis_cmdoutbuf, iovar);

	/* include the null */
	len = strlen(iovar) + 1;

	if (param_len)
		memcpy(&g_vis_cmdoutbuf[len], param, param_len);

	len += param_len;

	return wlu_set(wl, WLC_SET_VAR, &g_vis_cmdoutbuf[0], len);
}

/*
 * set named iovar providing both parameter and i/o buffers
 * iovar name is converted to lower case
 */
int
wlu_iovar_setbuf(void* wl, const char *iovar,
	void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	int iolen;

	iolen = wl_iovar_mkbuf(iovar, param, paramlen, bufptr, buflen, &err);
	if (err)
		return err;

	return wlu_set(wl, WLC_SET_VAR, bufptr, iolen);
}

/*
 * set named iovar given the parameter buffer
 * iovar name is converted to lower case
 */
int
wlu_iovar_set(void *wl, const char *iovar, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN*2];

	memset(smbuf, 0, sizeof(smbuf));

	return wlu_iovar_setbuf(wl, iovar, param, paramlen, smbuf, sizeof(smbuf));
}

#ifndef ATE_BUILD
/* given a chanspec value from the driver, do the endian and chanspec version conversion to
 * a chanspec_t value
 * Returns INVCHANSPEC on error
 */
chanspec_t
wl_chspec_from_driver(chanspec_t chanspec)
{
	chanspec = dtohchanspec(chanspec);
	if (ioctl_version == 1) {
		chanspec = wl_chspec_from_legacy(chanspec);
	}

	return chanspec;
}
#endif /* !ATE_BUILD */

/* Return a new chanspec given a legacy chanspec
 * Returns INVCHANSPEC on error
 */
chanspec_t
wl_chspec_from_legacy(chanspec_t legacy_chspec)
{
	chanspec_t chspec;

	/* get the channel number */
	chspec = LCHSPEC_CHANNEL(legacy_chspec);

	/* convert the band */
	if (LCHSPEC_IS2G(legacy_chspec)) {
		chspec |= WL_CHANSPEC_BAND_2G;
	} else {
		chspec |= WL_CHANSPEC_BAND_5G;
	}

	/* convert the bw and sideband */
	if (LCHSPEC_IS20(legacy_chspec)) {
		chspec |= WL_CHANSPEC_BW_20;
	} else {
		chspec |= WL_CHANSPEC_BW_40;
		if (LCHSPEC_CTL_SB(legacy_chspec) == WL_LCHANSPEC_CTL_SB_LOWER) {
			chspec |= WL_CHANSPEC_CTL_SB_L;
		} else {
			chspec |= WL_CHANSPEC_CTL_SB_U;
		}
	}

	if (wf_chspec_malformed(chspec)) {
		VIS_ADAPTER("wl_chspec_from_legacy: output chanspec (0x%04X) malformed\n",
		        chspec);
		return INVCHANSPEC;
	}

	return chspec;
}

/* Return a center and other channels, given chanspec and control channel and bandwidth
 */
void
get_center_channels(chanspec_t chspec, uint32 ctrl, uint32 *center, int bw)
{
	int i = 0;

	*center = 0;

	if (bw == 40) {
		if (ctrl <= 14) {
			if (CHSPEC_SB_UPPER(chspec)) {
				*center = ctrl - 2;
			}
			else {
				*center = ctrl + 2;
			}
		}
		else {
			for (i = 0; i < WF_NUM_5G_40M_CHANS; i++) {
				if (wf_5g_40m_chans[i] == (ctrl+2)) {
					*center = wf_5g_40m_chans[i];
					break;
				}
				else if (wf_5g_40m_chans[i] == (ctrl-2)) {
					*center = wf_5g_40m_chans[i];
					break;
				}
			}
		}
	}
	else if (bw == 80) {
		for (i = 0; i < WF_NUM_5G_80M_CHANS; i++) {
			int k, sub = -6, found = 0;

			for (k = 0; k < 4; k++) {
				if (wf_5g_80m_chans[i] == (ctrl+sub)) {
					*center = wf_5g_80m_chans[i];
					found = 1;
					break;
				}
				sub += 4;
			}
			if (found == 1)
				break;
		}
	}
	else if (bw == 160) {
		for (i = 0; i < WF_NUM_5G_160M_CHANS; i++) {
			int k, sub = -14, found = 0;

			for (k = 0; k < 8; k++) {
				if (wf_5g_160m_chans[i] == (ctrl+sub)) {
					*center = wf_5g_160m_chans[i];
					found = 1;
					break;
				}
				sub += 4;
			}
			if (found == 1)
				break;
		}
	}
}

/* Converts string MAC address to structure */
int
wl_ether_atoe(const char *a, struct ether_addr *n)
{
	char *c = NULL;
	int i = 0;

	memset(n, 0, ETHER_ADDR_LEN);
	for (;;) {
		n->octet[i++] = (uint8)strtoul(a, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
		a = c;
	}
	return (i == ETHER_ADDR_LEN);
}
