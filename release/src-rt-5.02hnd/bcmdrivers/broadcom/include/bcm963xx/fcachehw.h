#ifndef __FHW_H_INCLUDED__
#define __FHW_H_INCLUDED__
/*
*
*  Copyright 2011, Broadcom Corporation
*
* <:label-BRCM:2007:proprietary:standard
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#include <pktHdr.h>
#include <fcache.h>

#define CC_CONFIG_FHW_DBGLVL    0
#define CC_CONFIG_FHW_COLOR

/* Functional interface return status */
#define FHW_ERROR                (-1)    /* Functional interface error     */
#define FHW_SUCCESS              0       /* Functional interface success   */

#undef FHW_DECL
#define FHW_DECL(x)      x,  /* for enum declaration in H file */

#if defined(CC_CONFIG_FCACHE_DEBUG)    /* Runtime debug level setting */
int fcacheFhwDebug(int lvl);
#endif

typedef enum
{
    FHW_DECL(FHW_PRIO_0)
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)|| defined(CONFIG_BCM96858)
    FHW_DECL(FHW_PRIO_1)
#endif
    FHW_DECL(FHW_PRIO_MAX)
} FhwHwAccPrio_t; 


/*
 *------------------------------------------------------------------------------
 * Conditional Compile configuration for Packet HWACC
 *------------------------------------------------------------------------------
 */


/*
 *------------------------------------------------------------------------------
 * Implementation Constants 
 *------------------------------------------------------------------------------
 */

/* Special tuple to signify an invalid tuple. */
#define FHW_TUPLE_INVALID   FLOW_HW_INVALID
#define EFLOW_NULL          ((ExtFlow_t*)NULL)

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#ifdef XRDP
#define FHW_MAX_ENT         65536     /* Maximum number of entries. should be 131072 for B0     */
#else
#define FHW_MAX_ENT         16384     /* Maximum number of entries      */
#endif
#if (FHW_MAX_ENT > (128*1024))
#error "Invalid number of HW Accelerator entries, DO NOT EXCEED 128K" 
#endif
#else
/* Maximum number of entries: (Engine types) * (max # of flows for FAP)*/
#define FHW_MAX_ENT         4096     /* Maximum number of entries      */
#if (FHW_MAX_ENT > 4096)
#error "Invalid number of HW Accelerator entries, DO NOT EXCEED 4096" 
#endif
#endif

typedef struct {
    HOOKP           activate_fn; 
    HOOK4PARM       deactivate_fn;
    HOOK3PARM       refresh_fn; 
    HOOK4PARM       stats_fn;
    HOOK32          reset_stats_fn; 
    HOOK32          clear_fn; 
    FC_CLEAR_HOOK  *fhw_clear_fn;
    HOOK32          get_hw_entix_fn; 
    uint32_t        cap;
    uint32_t        max_ent;
} FhwBindHwHooks_t;

/*
 *------------------------------------------------------------------------------
 * HWACC binding to HW to register HW upcalls and downcalls
 * Upcalls from HWACC to HW: activate, deactivate and refresh functions.
 * Downcalls from HW to HWACC: clear hardware associations function.
 *------------------------------------------------------------------------------
 */
extern int fhw_bind_hw(FhwHwAccPrio_t prioIx, FhwBindHwHooks_t *hwHooks_p);

/*
 *------------------------------------------------------------------------------
 * Manual enabling and disabling of HWACC to Flow cache binding
 *  flag = 0 : disables binding to fc. No more HW Acceleration.
 *  flag != 0: enables binding to fc.
 *------------------------------------------------------------------------------
 */
extern void fhw_bind_fc(int flag);         /* disable[flag=0] enable[flag=1] */

/*
 *------------------------------------------------------------------------------
 * HWACC Entry Key:
 * A 32bit key that contains:
 *  - 16bit hardware connection id
 *------------------------------------------------------------------------------
 */
typedef struct {
    union {
        struct {
            BE_DECL(
                uint32_t accl    :  1; /* accelerator bit         */
                uint32_t reserved:  1; /* reserved bits           */
                uint32_t hw      : 30; /* hardware connection id  */
            )
            LE_DECL(
                uint32_t hw      : 30; /* hardware connection id  */
                uint32_t reserved:  1; /* reserved bits           */
                uint32_t accl    :  1; /* accelerator bit         */
            )
        } id;
        uint32_t word;
    };
} FhwKey_t;

/*
 *------------------------------------------------------------------------------
 * HW ACC Table Entry:
 *------------------------------------------------------------------------------
 */
typedef struct {
    FhwKey_t            key;
    uint32_t            hw_hits;
    uint32_t            hw_bytes_curr;
    unsigned long long  hw_bytes_cumm;
    uint32_t            flow_type;
} HwEnt_t;

extern uint32_t fhw_stats_hw(uint32_t flowIx, FhwKey_t key, unsigned long hw_hits_p,
    unsigned long long *hw_bytes_p);

extern uint32_t fhw_get_flow_type(uint32_t hwAccIx, uint32_t hwIx);

#endif  /* defined(__FHW_H_INCLUDED__) */
