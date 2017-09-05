/*
 * HND image/memory layout
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: rte_mem.h 627697 2016-03-28 06:15:02Z $
 */

#ifndef	_RTE_MEM_H
#define	_RTE_MEM_H

#include <typedefs.h>

/* Use standard symbols for Armulator build which does not use the hndrte.lds linker script */
#if defined(_RTE_SIM_) || defined(EXT_CBALL)
#define text_start	_start
#define text_end	etext
#define data_start	__data_start
#define data_end	edata
#define rodata_start	etext
#define rodata_end	__data_start
#define bss_start	__bss_start
#define bss_end		_end
#endif

extern char text_start[], text_end[];
extern char rodata_start[], rodata_end[];
extern char data_start[], data_end[];
extern char bss_start[], bss_end[], _end[];

typedef struct {
	char *_text_start, *_text_end;
	char *_rodata_start, *_rodata_end;
	char *_data_start, *_data_end;
	char *_bss_start, *_bss_end;
	char *_reclaim1_start, *_reclaim1_end;
	char *_reclaim2_start, *_reclaim2_end;
	char *_reclaim3_start, *_reclaim3_end;
	char *_reclaim4_start, *_reclaim4_end;
	char *_boot_patch_start, *_boot_patch_end;
	char *_reclaim5_start, *_reclaim5_end;
	char *_ucodes_start, *_ucodes_end;
} hnd_image_info_t;

extern void hnd_image_info(hnd_image_info_t *info);

extern uint32 _memsize;
extern uint32 _rambottom;
extern uint32 _atcmrambase;

extern uint32 hnd_get_memsize(void);
extern uint32 hnd_get_rambase(void);
extern uint32 hnd_get_rambottom(void);

#ifdef DONGLEBUILD
extern void hnd_reclaim(void);
#else
#define hnd_reclaim() do {} while (0)
#endif /* DONGLEBUILD */

#endif	/* _RTE_MEM_H */
