/*
 * Linux Visualization Data Concentrator utility functions header
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
 * $Id: vis_utility.h 568904 2015-07-06 11:04:01Z $
 */

#ifndef _VIS_UTILITY_H_
#define _VIS_UTILITY_H_

#include "vis_struct.h"

extern char *remove_spaces(char *src, char *dst);

extern void create_directory(char *dir);

extern void replace_plus_with_space(char *str);

extern void replace_special_characters(char *str, char replacechar);

extern int get_channel_str(int centerch, int bw, int ctrlch, char *channel, size_t n);

extern int is_valid_5g_40m_channel(int channel, int *upperch);

extern int is_valid_5g_80m_channel(int channel, int *upperch);

extern void get_lower_sideband_channel(int ctrl, int *sidechnl, int bw);

extern void fill_graph_details_struct(graphdetails_t *graphdet, iovar_handler_t *handle);

extern void get_center_channel(int ctrl, int *centerchnl, int bw);

extern unsigned long read_file_content(char *filename, char **data);

extern char* remove_trailing_character(char *str, char chartoremove);

#endif /* _VIS_UTILITY_H_ */
