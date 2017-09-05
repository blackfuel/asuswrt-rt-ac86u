/*
 * JTAG master (Broadcom Chipcommon) access interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: jtagm.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef _jtagm_h_
#define _jtagm_h_

/* Jtag master config */
extern unsigned short jtagm_pciid;	/* PCI id of the jtag master */
extern unsigned short jtagm_chipid;	/* Chipcommon chipid of the jtag master */
extern unsigned short jtagm_crev;	/* Chipcommon rev */
extern chipcregs_t *jtagm_regs;		/* For local (pci) access */
extern uint jtagm_pcicfg;		/* For local (pci config) access */
extern uint jtagm_rembase;		/* For use with tcp/ip */
extern uint jtagm_clkd;			/* Jtag clock divisor */
extern uint jtagm_inttap;		/* Internal TAP or external targets? */

/* function prototypes */
extern int jtagm_init(uint16 devid, uint32 sbidh, void *regsva, bool remote);
extern void jtagm_cleanup(void);

extern uint jtagm_scmd(uint ir, uint irw, uint dr, uint drw);

extern uint32 jtagm_rreg(uint reg);
extern void jtagm_wreg(uint reg, uint32 data);

#endif	/* _jtagm_h_ */
