/*
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: emfu.h 527353 2015-01-17 01:00:08Z $
 */

#ifndef _EMFU_H_
#define _EMFU_H_

#define	EMF_ARGC_ENABLE_FWD             2
#define	EMF_ARGC_DISABLE_FWD            2
#define	EMF_ARGC_GET_FWD                2
#define	EMF_ARGC_ADD_BRIDGE             3
#define	EMF_ARGC_DEL_BRIDGE             3
#define	EMF_ARGC_LIST_BRIDGE            3
#define	EMF_ARGC_ADD_IF                 4
#define	EMF_ARGC_DEL_IF                 4
#define	EMF_ARGC_LIST_IF                3
#define	EMF_ARGC_ADD_UFFP               4
#define	EMF_ARGC_DEL_UFFP               4
#define	EMF_ARGC_LIST_UFFP              3
#define	EMF_ARGC_ADD_RTPORT             4
#define	EMF_ARGC_DEL_RTPORT             4
#define	EMF_ARGC_LIST_RTPORT            3
#define	EMF_ARGC_ADD_MFDB               5
#define	EMF_ARGC_DEL_MFDB               5
#define	EMF_ARGC_LIST_MFDB              3
#define	EMF_ARGC_CLEAR_MFDB             3
#define	EMF_ARGC_SHOW_STATS             3
#define	EMF_ARGC_SET_LOG_LEVEL          3
#define	EMF_ARGC_SHOW_LOG_LEVEL         2

#define EMF_USAGE \
"Usage: emf  start   <bridge>\n"\
"            stop    <bridge>\n"\
"            status  <bridge>\n"\
"            add     bridge  <bridge>\n"\
"            del     bridge  <bridge>\n"\
"            add     iface   <bridge>  <if-name>\n"\
"            del     iface   <bridge>  <if-name>\n"\
"            list    iface   <bridge>\n"\
"            add     uffp    <bridge>  <if-name>\n"\
"            del     uffp    <bridge>  <if-name>\n"\
"            list    uffp    <bridge>\n"\
"            add     rtport  <bridge>  <if-name>\n"\
"            del     rtport  <bridge>  <if-name>\n"\
"            list    rtport  <bridge>\n"\
"            add     mfdb    <bridge>  <group-ip>  <if-name>\n"\
"            del     mfdb    <bridge>  <group-ip>  <if-name>\n"\
"            list    mfdb    <bridge>\n"\
"            clear   mfdb    <bridge>\n"\
"            show    stats   <bridge>\n"\
"            set     loglevel <loglevel>  0x1-debug 0x2-error 0x4-warn 0x8-info 0x10-mfdb\n"\
"                                        0x20-pktdump \n"\
"            show    loglevel \n"

typedef struct emf_cmd_arg
{
	char *cmd_oper_str;             /* Operation type string */
	char *cmd_id_str;               /* Command id string */
	int  (*input)(char *[]);        /* Command process function */
	int  arg_count;                 /* Arguments count */
} emf_cmd_arg_t;


#endif /* _EMFU_H_ */
