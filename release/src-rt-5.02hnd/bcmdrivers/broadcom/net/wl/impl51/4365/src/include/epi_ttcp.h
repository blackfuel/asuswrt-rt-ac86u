/*
 * Entry points for epi_ttcp application for platforms where the epi_ttcp utility
 * is not a standalone application.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id: epi_ttcp.h 241182 2011-02-17 21:50:03Z $
 */


#ifndef epi_ttcp_h
#define epi_ttcp_h

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Include Files ---------------------------------------------------- */
/* ---- Constants and Types ---------------------------------------------- */
/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */


/****************************************************************************
* Function:   epi_ttcp_main_args
*
* Purpose:    Entry point for epi_ttcp utility application. Uses same prototype
*             as standard main() functions - argc and argv parameters.
*
* Parameters: argc (in) Number of 'argv' arguments.
*             argv (in) Array of command-line arguments.
*
* Returns:    0 on success, else error code.
*****************************************************************************
*/
int epi_ttcp_main_args(int argc, char **argv);


/****************************************************************************
* Function:   epi_ttcp_main_str
*
* Purpose:    Alternate entry point for epi_ttcp utility application. This function
*             will tokenize the input command line string, before calling
*             epi_ttcp_main_args() to process the command line arguments.
*
* Parameters: str (mod) Input command line string. Contents will be modified!
*
* Returns:    0 on success, else error code.
*****************************************************************************
*/
int epi_ttcp_main_str(char *str);


#ifdef __cplusplus
	}
#endif

#endif  /* epi_ttcp_h  */
