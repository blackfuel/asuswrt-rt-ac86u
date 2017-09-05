/*
<:copyright-BRCM:2009:proprietary:standard

   Copyright (c) 2009 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/

/*
 *******************************************************************************
 * File Name  : bpm.c
 *
 *******************************************************************************
 */
/* -----------------------------------------------------------------------------
 *                      Global Buffer Pool Manager (BPM)
 * -----------------------------------------------------------------------------
 * When the system boots up all the buffers are owned by BPM. 
 *
 * Interface Initialization:
 * ------------------------
 * When an interface is initialized, the interface assigns buffers to
 * descriptors in it's private RX ring by requesting buffer allocation
 * from BPM (APIs: gbpm_alloc_mult_buf() or gbpm_alloc_buf()), 
 * and also informs BPM how many buffers were assigned to RX ring
 * (gbpm_resv_rx_buf()).
 *
 * Similarly, when an interface is uninitialized, the interface frees 
 * buffers from descriptors in it's private RX ring by requesting buffer 
 * free to BPM (APIs: gbpm_free_mult_buf() or gbpm_free_buf()), 
 * and also informs BPM how many buffers were freed from RX ring
 * (gbpm_unresv_rx_buf()).
 *
 * Knowing the size of RX ring allows BPM to keep track of the reserved
 * buffers (assigned to rings) and hence find out how many dynamic buffers
 * are available, which can be shared between the interfaces in the system. 
 *
 * Buffer Allocation:
 * ------------------
 * When a packet is received, the buffer is not immediately replenished 
 * into RX ring, rather a count is incremented, to keep track of how many
 * buffer allocation requests are pending. This is done to delay as much
 * as possible going to BPM because of overheads (locking, cycles, etc.),
 * and it likely that the RX ring will be soon replenished with a recycled
 * buffer.
 *
 * When an interface's RX ring buffers usage (RX DMA queue depth) exceeds
 * the configured threshold (because the earlier used buffers are not yet
 * recycled to RX ring), the interface requests BPM for allocation of
 * more buffers. The buffer request is fullfilled from the available 
 * dynamic buffers in BPM. After one or two such buffer allocation requests
 * equilibirium is established where the newly used buffers are replenished
 * with the recycled buffers.
 *
 * Buffer Free/Recycle:
 * --------------------
 *  After a packet is transmitted or dropped, the recycle function of 
 *  RX interface is invoked. Recycle function first checks whether there
 *  is a space in the RX ring. If yes, the buffer is recycled to ring.
 *  If not, buffer is freed to BPM.
 *
 *
 * TX Queue Threhsolds:
 * --------------------
 * TX Queue Thresholds are configured for the slower interfaces like XTM, 
 * MoCA, so that a lot of buffers are not held up in the TX queues of 
 * these interfaces.
 *
 * There are two TX Q thresholds per queue of an interface: low and high.
 * Only one of the low or high threshold is used to compare against the 
 * current queue depth. If the TX Q depth is lower than the compared 
 * threshold, the packet is enqueued, else the packet is dropped.
 *
 * The low or high threshold to be used is decided based on the current
 * level of dynamic buffers available in the system 
 * (API gbpm_get_dyn_buf_level() ). If the current dynamic buffer 
 * level is less than the configured threshold, then the low threshold 
 * is used else the high threshold is used.
 *
 *
 * Note: API prototypes are given in gbpm.h file.
 */
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>
#include <linux/fs.h>
#include <linux/bcm_log_mod.h>
#include <linux/bcm_log.h>
#include <linux/export.h>
#include <board.h>
#include <linux/gbpm.h>
#include <bpmctl_common.h>
#include <bcm_pkt_lengths.h>
#include <bcmPktDma_defines.h>
#include <bpm.h>

extern int kerSysGetSdramSize( void );
#if !(defined(CONFIG_BCM_RDPA) && !defined(CONFIG_BCM_RDPA_MODULE))
#if defined(CONFIG_BCM_PKTDMA)    
extern int bcmPktDma_GetTotRxBds( void );
#endif
#endif

/*----- Globals -----*/

extern gbpm_status_hook_t gbpm_enet_status_hook_g;

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
extern gbpm_status_hook_t gbpm_fap_status_hook_g;

extern gbpm_thresh_hook_t gbpm_fap_thresh_hook_g;
extern gbpm_thresh_hook_t gbpm_fap_enet_thresh_hook_g;
extern gbpm_thresh_hook_t gbpm_enet_thresh_hook_g;
extern gbpm_upd_buf_lvl_hook_t gbpm_fap_upd_buf_lvl_hook_g;
#endif


#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
extern gbpm_status_hook_t gbpm_xtm_status_hook_g;
extern gbpm_thresh_hook_t gbpm_xtm_thresh_hook_g;
#endif


#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
DEFINE_SPINLOCK(bpm_buf_lock_g);
EXPORT_SYMBOL(bpm_buf_lock_g);
#define BPM_FLAGS_DEF()            unsigned long flags
#define BPM_BUF_LOCK_IRQ()         spin_lock_irqsave(&bpm_buf_lock_g, flags)
#define BPM_BUF_UNLOCK_IRQ()       spin_unlock_irqrestore(&bpm_buf_lock_g, flags)
#define BPM_BUF_LOCK_BH()          spin_lock_bh(&bpm_buf_lock_g)
#define BPM_BUF_UNLOCK_BH()        spin_unlock_bh(&bpm_buf_lock_g)
#else
#define BPM_FLAGS_DEF()             /* */
#define BPM_BUF_LOCK_IRQ()         local_irq_disable()
#define BPM_BUF_UNLOCK_IRQ()       local_irq_enable()
#define BPM_BUF_LOCK_BH()          local_bh_disable()
#define BPM_BUF_UNLOCK_BH()        local_bh_enable()
#endif

#undef BPM_DECL
#define BPM_DECL(x) #x,

const char *bpmctl_ioctl_name[] =
{
    BPM_DECL(BPMCTL_IOCTL_SYS)
    BPM_DECL(BPMCTL_IOCTL_MAX)
};

const char *bpmctl_subsys_name[] =
{
    BPM_DECL(BPMCTL_SUBSYS_STATUS)
    BPM_DECL(BPMCTL_SUBSYS_THRESH)
    BPM_DECL(BPMCTL_SUBSYS_BUFFERS)
    BPM_DECL(BPMCTL_SUBSYS_MAX)
};

const char *bpmctl_op_name[] =
{   
    BPM_DECL(BPMCTL_OP_SET)
    BPM_DECL(BPMCTL_OP_GET)
    BPM_DECL(BPMCTL_OP_ADD)
    BPM_DECL(BPMCTL_OP_REM)
    BPM_DECL(BPMCTL_OP_DUMP)
    BPM_DECL(BPMCTL_OP_MAX)
};

const char *strBpmPort[] = 
{ 
    "ETH ",
    "XTM ",
    "FWD ",
    "WLAN",
    "USB ",
    "MAX "
};

typedef struct {
    uint32_t head           ____cacheline_aligned;
    uint32_t tail;
    uint32_t avail;
    uint32_t last;
    uint32_t buf_ix;
    uint32_t no_buf;
    uint32_t alloc;
    uint32_t free;
    uint32_t mem_ix;
    void **  mem_pool;
    void **  buf_pool;
    uint32_t dyn_buf_lo_thresh;
    uint32_t max_dyn;
    uint32_t rxbds;
    uint32_t fap_resv;
    uint32_t tot_resv_buf;
    uint32_t tot_rx_ring_buf;
    uint32_t tot_alloc_trig;
    uint32_t status[GBPM_PORT_MAX][GBPM_RXCHNL_MAX];
    uint32_t num_rx_buf[GBPM_PORT_MAX][GBPM_RXCHNL_MAX];
    uint32_t alloc_trig_thresh[GBPM_PORT_MAX][GBPM_RXCHNL_MAX];
} bpm_buf_t;

bpm_buf_t bpm_buf_g; 
bpm_buf_t *bpm_buf_pg = &bpm_buf_g; 
static uint32_t bpm_buf_pool_init_done_g = 0;



/*
 *------------------------------------------------------------------------------
 * function   : bpm_get_dyn_buf_level
 * description: finds the current dynamic buffer level is high or low.
 *------------------------------------------------------------------------------
 */
static int bpm_get_dyn_buf_level(void) 
{
    if (bpm_buf_pg->avail > bpm_buf_pg->dyn_buf_lo_thresh)
        return 1;
    return 0;
}


/*
 *------------------------------------------------------------------------------
 * function   : bpm_validate_resv_rx_buf
 * description: validates the port enable
 *------------------------------------------------------------------------------
 */
static int bpm_validate_resv_rx_buf( gbpm_port_t port, uint32_t chnl,
        uint32_t num_rx_buf, uint32_t alloc_trig_thresh) 
{
    /* validate parameters */
    if ( (port >= GBPM_PORT_MAX ) || (chnl >= GBPM_RXCHNL_MAX) )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM,
            "invalid port=%d or chnl=%d", port, chnl );
        return BPM_ERROR;
    }

    if ( (num_rx_buf < alloc_trig_thresh) )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM,
            "invalid alloc_trig_thresh=%d num_rx_buf=%d", 
            alloc_trig_thresh, num_rx_buf );
        return BPM_ERROR;
    }

    return BPM_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * function   : bpm_upd_dyn_buf_lo_thresh
 * description: updates the dynamic buffer low threshold
 *------------------------------------------------------------------------------
 */
static void bpm_upd_dyn_buf_lo_thresh( void )
{
    /* calc the low thresh for dynamic buffers in the global buffer pool */ 
    bpm_buf_pg->dyn_buf_lo_thresh = 
       (bpm_buf_pg->max_dyn * BPM_PCT_DYN_BUF_LO_THRESH / 100);
}


/*
 *------------------------------------------------------------------------------
 * function   : bpm_resv_rx_buf
 * description: reserves num of rxbufs and updates thresholds
 *------------------------------------------------------------------------------
 */
static int bpm_resv_rx_buf( gbpm_port_t port, uint32_t chnl,
        uint32_t num_rx_buf, uint32_t alloc_trig_thresh ) 
{
	
	BPM_FLAGS_DEF();
    /* validate parameters */
    if (bpm_validate_resv_rx_buf( port, chnl, num_rx_buf, alloc_trig_thresh ) 
            == BPM_ERROR)
        return BPM_ERROR;

    BPM_BUF_LOCK_IRQ();

    /* flag the chnl has been enabled */
    bpm_buf_pg->status[port][chnl] = GBPM_RXCHNL_ENABLED;
    bpm_buf_pg->num_rx_buf[port][chnl] = num_rx_buf;
    bpm_buf_pg->alloc_trig_thresh[port][chnl] = alloc_trig_thresh;
    bpm_buf_pg->tot_rx_ring_buf += num_rx_buf;
    bpm_buf_pg->tot_alloc_trig += alloc_trig_thresh;
    

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, 
                "port=%d chnl=%d resv_rx_buf=%d alloc_trig_thresh=%d", 
                port, chnl, bpm_buf_pg->num_rx_buf[port][chnl],
                bpm_buf_pg->alloc_trig_thresh[port][chnl]);
 
    BPM_BUF_UNLOCK_IRQ();
    return BPM_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * function   : bpm_unresv_rx_buf
 * description: unreserves the previously reserved rx bufs
 *------------------------------------------------------------------------------
 */
static int bpm_unresv_rx_buf( gbpm_port_t port, uint32_t chnl ) 
{
    /* flag the chnl has been disabled */
    bpm_buf_pg->status[port][chnl] = GBPM_RXCHNL_DISABLED;

    bpm_buf_pg->tot_alloc_trig -= bpm_buf_pg->alloc_trig_thresh[port][chnl];
    bpm_buf_pg->tot_rx_ring_buf -= bpm_buf_pg->num_rx_buf[port][chnl];

    bpm_buf_pg->num_rx_buf[port][chnl] = 0;
    bpm_buf_pg->alloc_trig_thresh[port][chnl] = 0;

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, "port=%d chnl=%d unresv_rx_buf=%d",
                port, chnl, bpm_buf_pg->num_rx_buf[port][chnl] );

    return BPM_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function   : bpm_alloc_buf
 * Description: allcates a buffer from global buffer pool
 *------------------------------------------------------------------------------
 */
static void * bpm_alloc_buf( void )
{
    void * buf_p;
    BPM_FLAGS_DEF();
	
    BPM_BUF_LOCK_IRQ();
    if (bpm_buf_pg->avail)
    {/* buffers available in global buffer pool */
        buf_p = bpm_buf_pg->buf_pool[bpm_buf_pg->head];

        if ( bpm_buf_pg->head >= bpm_buf_pg->last )
            bpm_buf_pg->head = 0;
        else
            bpm_buf_pg->head++;

        bpm_buf_pg->avail--;
        bpm_buf_pg->alloc++;

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        if (likely(gbpm_fap_upd_buf_lvl_hook_g !=
                                (gbpm_upd_buf_lvl_hook_t)NULL))
            gbpm_fap_upd_buf_lvl_hook_g(bpm_get_dyn_buf_level());
#endif
    }
    else
    {
        buf_p = NULL;
        bpm_buf_pg->no_buf++;
    }
    BPM_BUF_UNLOCK_IRQ();

    return buf_p;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_free_buf
 * Description: frees a buffer to global buffer pool
 *------------------------------------------------------------------------------
 */
static void bpm_free_buf( void * buf_p )
{
    BPM_FLAGS_DEF();
    BCM_ASSERT( buf_p != NULL );

    BPM_BUF_LOCK_IRQ();
    bpm_buf_pg->buf_pool[bpm_buf_pg->tail] = buf_p;

    if ( bpm_buf_pg->tail >= bpm_buf_pg->last )
        bpm_buf_pg->tail = 0;
    else
        bpm_buf_pg->tail++;

    bpm_buf_pg->avail++;
    bpm_buf_pg->free++;
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (likely(gbpm_fap_upd_buf_lvl_hook_g !=
                                (gbpm_upd_buf_lvl_hook_t)NULL))
        gbpm_fap_upd_buf_lvl_hook_g(bpm_get_dyn_buf_level());
#endif

    BPM_BUF_UNLOCK_IRQ();
}



/*
 *------------------------------------------------------------------------------
 * Function   : bpm_alloc_mult_buf
 * Description: allcates a buffer from global buffer pool
 *------------------------------------------------------------------------------
 */
static int bpm_alloc_mult_buf( uint32_t num, void ** buf_p )
{
    int ret = BPM_SUCCESS;
	BPM_FLAGS_DEF();
	
    BCM_ASSERT( buf_p != NULL );

    BPM_BUF_LOCK_IRQ();
    if (bpm_buf_pg->avail >= num)
    {/* buffers available in global buffer pool */
        int buf_ix;
        uint32_t head = bpm_buf_pg->head;

        for( buf_ix=0; buf_ix < num; buf_ix++ )
        {
            buf_p[buf_ix] = bpm_buf_pg->buf_pool[head];        
            if ( head >= bpm_buf_pg->last )
                head = 0;    
            else
                head++;
        }

        bpm_buf_pg->head = head;

        bpm_buf_pg->avail -= num;
        bpm_buf_pg->alloc += num;
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        if (likely(gbpm_fap_upd_buf_lvl_hook_g !=
                                        (gbpm_upd_buf_lvl_hook_t)NULL))
            gbpm_fap_upd_buf_lvl_hook_g(bpm_get_dyn_buf_level());
#endif
    }
    else
    {
        bpm_buf_pg->no_buf++;
        ret = BPM_ERROR;
    }
    BPM_BUF_UNLOCK_IRQ();

    return ret;
}


/*
 *------------------------------------------------------------------------------
 * Function   : bpm_free_mult_buf
 * Description: frees a buffer to global buffer pool
 *------------------------------------------------------------------------------
 */
static void bpm_free_mult_buf( uint32_t num, void ** buf_p )
{
	BPM_FLAGS_DEF();
    BCM_ASSERT( buf_p != NULL );

    BPM_BUF_LOCK_IRQ();
    if (bpm_buf_pg->avail <= (bpm_buf_pg->buf_ix - num))
    {
        int buf_ix;
        uint32_t tail = bpm_buf_pg->tail;

        for( buf_ix=0; buf_ix < num; buf_ix++ )
        {
            bpm_buf_pg->buf_pool[tail] = buf_p[buf_ix];
            if ( tail >= bpm_buf_pg->last )
                tail = 0;
            else
                tail++;
        }

        bpm_buf_pg->tail = tail;

        bpm_buf_pg->avail += num;
        bpm_buf_pg->free += num;
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (likely(gbpm_fap_upd_buf_lvl_hook_g != 
            (gbpm_upd_buf_lvl_hook_t)NULL))
        gbpm_fap_upd_buf_lvl_hook_g(bpm_get_dyn_buf_level());
#endif

    }
    BPM_BUF_UNLOCK_IRQ();
}


/*
 *------------------------------------------------------------------------------
 * Function   : bpm_buf_mem_alloc
 * Description: Allocate a large memory chunk for carving out RX buffers
 *  The memory allocated is reset and flushed. A pointer to a cache aligned
 *  address of the requested size is returned. A pointer to the allocated
 *  memory is saved.
 *
 *  PS. Although kmalloc guarantees L1_CACHE_ALIGN, we do not assume so.
 *------------------------------------------------------------------------------
 */
static void *bpm_buf_mem_alloc( size_t memsz )
{
    void *mem_p;

    BCM_LOG_FUNC( BCM_LOG_ID_BPM );

    memsz += L1_CACHE_BYTES;

    if ( bpm_buf_pg->mem_ix >= BPM_MAX_MEM_POOL_IX )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                "too many memory pools %d", bpm_buf_pg->mem_ix );
        return NULL;
    }

    if ( (mem_p = kmalloc( memsz, GFP_ATOMIC ) ) == NULL )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM, "kmalloc %d failure", (int)memsz );
        return NULL;
    }

    /* Future kfree */
    bpm_buf_pg->mem_pool[bpm_buf_pg->mem_ix] = mem_p;
    bpm_buf_pg->mem_ix++;

    memset( mem_p, 0, memsz );
    cache_flush_len( mem_p, memsz );                  /* Flush invalidate */

    mem_p = (void *)L1_CACHE_ALIGN((uintptr_t)mem_p );  /* L1 cache aligned */

    return mem_p;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_init_buf_pool
 * Description: Initialize buffer pool, with num buffers
 *  Pointers to pre-allocated memory pools are saved. See bpm_buf_mem_alloc()
 *------------------------------------------------------------------------------
 */
static int bpm_init_buf_pool( uint32_t num )
{
    void *mem_p;
    uint8_t *buf_p;
    uint32_t bufs_per_mem, memsz, i;
	BPM_FLAGS_DEF();
	
    BPM_BUF_LOCK_IRQ();

    bpm_buf_pg->mem_ix = 0; /* Index into memory pools allocated */
    bpm_buf_pg->buf_ix = 0;

    /* Allocate chunks of memory, carve buffers */
    bufs_per_mem = (BPM_MAX_MEMSIZE - L1_CACHE_BYTES) / BCM_PKTBUF_SIZE;
    while ( num )
    {
        uint8_t *data_p;

        /* Chunk size */
        bufs_per_mem = (bufs_per_mem < num) ? bufs_per_mem : num;
        memsz = bufs_per_mem * BCM_PKTBUF_SIZE;

        if( (mem_p = bpm_buf_mem_alloc( memsz ) ) == NULL) 
            return BPM_ERROR;

        buf_p = (uint8_t *)mem_p;                   /* Buffers are cached */

        for ( i=0; i < bufs_per_mem; i++, buf_p += BCM_PKTBUF_SIZE )
        {

            /* L1 cache aligned */
            data_p = (void *)L1_CACHE_ALIGN( (uintptr_t)buf_p );
            bpm_buf_pg->buf_pool[bpm_buf_pg->buf_ix++] = (void *) data_p;
        }

        BCM_LOG_DEBUG( BCM_LOG_ID_BPM, 
            "allocated %4u %8s @ mem_p<%p> memsz<%06u>",
                    bufs_per_mem, "RxBufs", (void *)mem_p, memsz );

        num -= bufs_per_mem;
    }

    bpm_buf_pg->head = 0;
    bpm_buf_pg->tail = 0;
    bpm_buf_pg->avail = bpm_buf_pg->buf_ix; /* all buffers are available */
    bpm_buf_pg->last = bpm_buf_pg->buf_ix - 1;

#if !(defined(CONFIG_BCM_RDPA) && !defined(CONFIG_BCM_RDPA_MODULE)) 
#if defined(CONFIG_BCM_PKTDMA)    
    bpm_buf_pg->rxbds = bcmPktDma_GetTotRxBds();
#endif    
#endif
    bpm_buf_pg->fap_resv = FAP_BPM_BUF_RESV;
    bpm_buf_pg->tot_resv_buf = bpm_buf_pg->rxbds + bpm_buf_pg->fap_resv;
    bpm_buf_pg->max_dyn = bpm_buf_pg->buf_ix - bpm_buf_pg->tot_resv_buf;

    bpm_upd_dyn_buf_lo_thresh();
    bpm_buf_pool_init_done_g = 1;
    BPM_BUF_UNLOCK_IRQ();

    return BPM_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_free_buf_pool
 * Description: Releases all buffers of the pool
 *------------------------------------------------------------------------------
 */
static void bpm_free_buf_pool( void )
{
    void *mem_p;
    uint32_t i ;
	BPM_FLAGS_DEF();
	
    BCM_LOG_FUNC( BCM_LOG_ID_BPM );

    BPM_BUF_LOCK_IRQ();
    /*
     * Release all memory pools allocated
     */
    for ( i=0; i<bpm_buf_pg->mem_ix; i++ )
    {
        mem_p = (uint8_t *) bpm_buf_pg->mem_pool[ i ];
        if ( mem_p )
            kfree( mem_p );
    }

    memset( (void*)bpm_buf_pg, 0, sizeof(bpm_buf_t) );
    BPM_BUF_UNLOCK_IRQ();

    return;
}


/** Return the total number of buffers managed by BPM
 *
 *@returns total number of buffers managed by BPM
 */
static uint32_t bpm_get_total_bufs( void )
{
    return bpm_buf_pg->buf_ix;
}


/** Return the current number of free buffers in the BPM pool
 *
 *@returns the current number of free buffers in the BPM pool
 */
static uint32_t bpm_get_avail_bufs( void )
{
    return bpm_buf_pg->avail;
}

/** Return the current number of free buffers in the BPM pool
 *
 *@returns the current number of free buffers in the BPM pool
 */
static uint32_t bpm_get_max_dyn_bufs( void )
{
    return bpm_buf_pg->max_dyn;
}


void bpm_test( void )
{
    void * buf_p;

    bpm_init_buf_pool( 896 );
    buf_p = bpm_alloc_buf();
    bpm_free_buf( buf_p );
    bpm_free_buf_pool();
}


/*
 *------------------------------------------------------------------------------
 * Function   : bpm_dump_status
 * Description: function handler for dumping the status
 *------------------------------------------------------------------------------
 */
static void bpm_dump_status(void)
{
    bpm_buf_t *p = bpm_buf_pg;

    printk( "\n------------------------ BPM Status -----------------------\n" );
    printk( " tot_buf avail  head  tail no_buf_err  cum_alloc   cum_free\n" );
    printk( "%7u %5u %5u %5u %10u %10u %10u",
        p->buf_ix, p->avail, p->head, p->tail, p->no_buf, p->alloc, p->free ); 
    printk( "\nmax_dyn tot_resv_buf rxbds fap_resv\n" );
    printk( "%7u %12u %5u %8u",
        p->max_dyn, p->tot_resv_buf, p->rxbds, p->fap_resv ); 

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, "buf_pool mem_ix   mem_pool \n" );
    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, " 0x%p %6u 0x%p\n", 
        p->buf_pool, p->mem_ix, p->mem_pool );
    printk("\n-------------------------------------------------"
           "---------------------------\n");
    printk("        dev chnl  cum_alloc   cum_free avail trig"
           " bulk       reqt       resp\n");
    printk("------ ---- ---- ---------- ---------- ----- ----"
           " ---- ---------- ----------\n");

    if ( likely(gbpm_enet_status_hook_g != (gbpm_status_hook_t)NULL) )
    {
        gbpm_enet_status_hook_g(); 
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    if ( likely(gbpm_xtm_status_hook_g != (gbpm_status_hook_t)NULL) )
    {
        gbpm_xtm_status_hook_g(); 
    }
#endif

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if ( likely(gbpm_fap_status_hook_g != (gbpm_status_hook_t)NULL) )
        gbpm_fap_status_hook_g(); 
#endif


    return;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_dump_buffers
 * Description: function handler for dumping the buffers
 *------------------------------------------------------------------------------
 */
static void bpm_dump_buffers(void)
{
    bpm_buf_t *p = bpm_buf_pg;
    int bufix = 0;

    printk( "\n----------------- Buffer Pool ----------------\n" );
    printk( "\n  Idx Address0 Address1 Address2 Address3" );
    printk( " Address4 Address5 Address6 Address7\n" );

    for (; (bufix <= p->last); bufix+=8 )
    {
        if ((bufix % 256) == 0)
            printk("\n");

        printk( "[%3u] %p %p %p %p %p %p %p %p\n", 
                        bufix, 
                        p->buf_pool[bufix], 
                        p->buf_pool[bufix+1], 
                        p->buf_pool[bufix+2], 
                        p->buf_pool[bufix+3], 
                        p->buf_pool[bufix+4], 
                        p->buf_pool[bufix+5], 
                        p->buf_pool[bufix+6], 
                        p->buf_pool[bufix+7] ); 
    }

    printk( "\n" );

    return;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_dump_thresh
 * Description: function for dumping the thresh
 *------------------------------------------------------------------------------
 */
static void bpm_dump_thresh( void )
{
    bpm_buf_t *p = bpm_buf_pg;
    gbpm_port_t port; 
    uint32_t chnl;

    printk( "\n-------------------- BPM Thresh -------------------\n" );
    printk( "tot_buf tot_resv_buf max_dyn   avail dyn_buf_lo_thr\n" );
    printk( "%7u %12u %7u %7u %14u\n", 
       p->buf_ix, p->tot_resv_buf, p->max_dyn, p->avail, p->dyn_buf_lo_thresh );

    printk("\n---------------------------------\n");
    printk( "port chnl rx_ring_buf alloc_trig\n" );
    printk( "---- ---- ----------- ----------\n");
    for( port=0; port<GBPM_PORT_MAX; port++ )
    {
        for( chnl=0; chnl<GBPM_RXCHNL_MAX; chnl++ )
        {
            if (bpm_buf_pg->status[port][chnl] == GBPM_RXCHNL_ENABLED)
            {
                printk( "%4s %4u %11u %10u\n", strBpmPort[port], chnl, 
                p->num_rx_buf[port][chnl], p->alloc_trig_thresh[port][chnl] );
            }
        }
    }
    printk( "\n" );


    printk("\n---------------------------------------\n");
    printk("        dev  txq loThr hiThr    dropped\n");
    printk("------ ---- ---- ----- ----- ----------\n");

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    if (likely(gbpm_xtm_thresh_hook_g != (gbpm_thresh_hook_t)NULL))
        gbpm_xtm_thresh_hook_g(); 
#endif

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (likely(gbpm_fap_thresh_hook_g != (gbpm_thresh_hook_t)NULL) )
        gbpm_fap_thresh_hook_g(); 

    printk("\n\n-------------------------------------------\n");
    printk("        dev chnl  txq dropThresh    dropped\n");
    printk("------ ---- ---- ---- ---------- ----------\n");
    if (likely(gbpm_enet_thresh_hook_g != (gbpm_thresh_hook_t)NULL) )
        gbpm_enet_thresh_hook_g(); 

    if (likely(gbpm_fap_enet_thresh_hook_g != (gbpm_thresh_hook_t)NULL) )
        gbpm_fap_enet_thresh_hook_g(); 
#endif
}


/*
 *------------------------------------------------------------------------------
 * function   : bpm_calc_num_buf
 * description: Finds the total memory available on the board, and assigns 
 * one-ourth of the total memory to the buffers. Finally calculates the 
 * number of buffers to be allocated based on buffer memory and buffer size.
 *------------------------------------------------------------------------------
 */
static int bpm_calc_num_buf( void )
{
    uint32_t tot_mem_size = kerSysGetSdramSize();
    uint32_t buf_mem_size = (tot_mem_size/100) * CONFIG_BCM_BPM_BUF_MEM_PRCNT;

    printk( "BPM: tot_mem_size=%dB (%dMB), ", tot_mem_size, tot_mem_size/MB );
    printk( "buf_mem_size <%d%%> =%dB (%dMB), ", CONFIG_BCM_BPM_BUF_MEM_PRCNT,buf_mem_size, buf_mem_size/MB );
    printk( "num of buffers=%d, buf size=%d\n",
                            (unsigned int)(buf_mem_size/BCM_PKTBUF_SIZE), (unsigned int)BCM_PKTBUF_SIZE);

    return (buf_mem_size/BCM_PKTBUF_SIZE);
}


/*
 *------------------------------------------------------------------------------
 * Function Name: bpm_drv_ioctl
 * Description  : Main entry point to handle user applications IOCTL requests
 *                from BPM Control Utility.
 * Returns      : 0 - success or error
 *------------------------------------------------------------------------------
 */
static int bpm_drv_ioctl(struct inode *inode, struct file *filep,
                       unsigned int command, unsigned long arg)
{
    bpmctl_ioctl_t cmd;
    bpmctl_data_t  bpm;
    bpmctl_data_t *bpm_p = &bpm;
    int ret = BPM_SUCCESS;
	BPM_FLAGS_DEF();
	
    if ( command > BPMCTL_IOCTL_MAX )
        cmd = BPMCTL_IOCTL_MAX;
    else
        cmd = (bpmctl_ioctl_t)command;

    copy_from_user( bpm_p, (uint8_t *) arg, sizeof(bpm) );

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, 
            "cmd<%d> %s subsys<%d> %s op<%d> %s arg<0x%lx>",
            command, bpmctl_ioctl_name[command], 
            bpm_p->subsys, bpmctl_subsys_name[bpm_p->subsys], 
            bpm_p->op, bpmctl_op_name[bpm_p->op], arg );

    switch ( cmd )
    {
        case BPMCTL_IOCTL_SYS :
        {
            switch (bpm_p->subsys)
            {
                case BPMCTL_SUBSYS_STATUS:
                    switch (bpm_p->op)
                    {
                        case BPMCTL_OP_DUMP:
                            bpm_dump_status();
                            break;

                        default:
                            BCM_LOG_ERROR(BCM_LOG_ID_BPM, 
                                        "Invalid op[%u]", bpm_p->op );
                     }
                     break;

                case BPMCTL_SUBSYS_THRESH:
                    switch (bpm_p->op)
                    {
                        case BPMCTL_OP_DUMP:
                            BPM_BUF_LOCK_IRQ();
                            bpm_dump_thresh();
                            BPM_BUF_UNLOCK_IRQ();
                            break;

                        default:
                            BCM_LOG_ERROR(BCM_LOG_ID_BPM, 
                                        "Invalid op[%u]", bpm_p->op );
                     }
                     break;

                case BPMCTL_SUBSYS_BUFFERS:
                    switch (bpm_p->op)
                    {
                        case BPMCTL_OP_DUMP:
                            BPM_BUF_LOCK_IRQ();
                            bpm_dump_buffers();
                            BPM_BUF_UNLOCK_IRQ();
                            break;

                        default:
                            BCM_LOG_ERROR(BCM_LOG_ID_BPM, 
                                        "Invalid op[%u]", bpm_p->op );
                     }
                     break;

                default:
                    BCM_LOG_ERROR(BCM_LOG_ID_BPM, 
                                    "Invalid subsys[%u]", bpm_p->subsys);
                    break;
            }
            break;
        }

        default :
        {
            BCM_LOG_ERROR(BCM_LOG_ID_BPM, "Invalid cmd[%u]", command );
            ret = BPM_ERROR;
        }
    }

    return ret;

} /* bpm_drv_ioctl */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static DEFINE_MUTEX(bpmIoctlMutex);

static long bpm_drv_unlocked_ioctl(struct file *filep,
    unsigned int cmd, unsigned long arg )
{
    struct inode *inode;
    long rt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)    
    inode = filep->f_dentry->d_inode;
#else
    inode = file_inode(filep);
#endif

    mutex_lock(&bpmIoctlMutex);
    rt = bpm_drv_ioctl( inode, filep, cmd, arg );
    mutex_unlock(&bpmIoctlMutex);
    
    return rt;
}
#endif


/*
 *------------------------------------------------------------------------------
 * Function Name: bpm_drv_open
 * Description  : Called when a user application opens this device.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static int bpm_drv_open(struct inode *inode, struct file *filp)
{
    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, "Access BPM Char Device" );
    return BPM_SUCCESS;
} /* bpm_drv_open */


/* Global file ops */
static struct file_operations bpm_fops =
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
    .unlocked_ioctl = bpm_drv_unlocked_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = bpm_drv_unlocked_ioctl,
#endif
#else
    .ioctl  = bpm_drv_ioctl,
#endif
    .open   = bpm_drv_open,
};


/*
 *------------------------------------------------------------------------------
 * Function Name: bpm_drv_construct
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
static int bpm_drv_construct(void)
{
    if ( register_chrdev( BPM_DRV_MAJOR, BPM_DRV_NAME, &bpm_fops ) )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                "%s Unable to get major number <%d>" CLRnl,
                  __FUNCTION__, BPM_DRV_MAJOR);
        return BPM_ERROR;
    }

    printk( BPM_MODNAME " Char Driver " BPM_VER_STR " Registered<%d>" 
                                                    CLRnl, BPM_DRV_MAJOR );

    return BPM_DRV_MAJOR;
}



static int __init bpm_module_init( void )
{
    uint32_t tot_num_buf = bpm_calc_num_buf();
    bcmLog_setLogLevel(BCM_LOG_ID_BPM, BCM_LOG_LEVEL_NOTICE);

    bpm_drv_construct();

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, "%s: bpm_buf_pg<0x%p>", __FUNCTION__, 
            bpm_buf_pg );

    if (!bpm_buf_pool_init_done_g)
    {
        uint32_t tot_mem_pool = 0;
        uint32_t mem_pool_sz = 0;
        uint32_t buf_pool_sz = tot_num_buf * sizeof(void *);
        void *buf_pool_p = NULL, *mem_pool_p = NULL;

        memset( (void*)bpm_buf_pg, 0, sizeof(bpm_buf_t) );

        if (tot_num_buf > BPM_MAX_BUF_POOL_IX)
        {
            BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                    "too many buffers %d \n", tot_num_buf );
            return BPM_ERROR;
        }

        tot_mem_pool = (tot_num_buf * BCM_PKTBUF_SIZE/BPM_MAX_MEMSIZE) + 1;
        BCM_LOG_DEBUG( BCM_LOG_ID_BPM, "tot_mem_pool=%u\n", tot_mem_pool);

        if (tot_mem_pool > BPM_MAX_MEM_POOL_IX)
        {
            BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                    "too many memory pools %d", tot_mem_pool );
            return BPM_ERROR;
        }

        mem_pool_sz = tot_mem_pool * sizeof(void *);
        if ( (mem_pool_p = kmalloc( mem_pool_sz, GFP_ATOMIC ) ) == NULL )
        {
            BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                "kmalloc %d failure for mem_pool_p", mem_pool_sz);
            return BPM_ERROR;
        }
        bpm_buf_pg->mem_pool = (void **) mem_pool_p;


        if ( (buf_pool_p = kmalloc( buf_pool_sz, GFP_ATOMIC ) ) == NULL )
        {
            BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                "kmalloc %d failure for buf_pool_p", buf_pool_sz);
            return BPM_ERROR;
        }

        bpm_buf_pg->buf_pool = (void **) buf_pool_p;

        bpm_init_buf_pool( tot_num_buf );

        gbpm_bind( bpm_get_dyn_buf_level, 
            bpm_alloc_mult_buf, bpm_free_mult_buf, 
            bpm_alloc_buf, bpm_free_buf, 
            bpm_resv_rx_buf, bpm_unresv_rx_buf,
            bpm_get_total_bufs, bpm_get_avail_bufs,
            bpm_get_max_dyn_bufs );

        bpm_get_max_dyn_bufs();
    }

    return 0;
}

module_init( bpm_module_init );

#if 0
/*
 *------------------------------------------------------------------------------
 * Function Name: bpm_drv_destruct
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
static void bpm_drv_destruct(void)
{
    unregister_chrdev( BPM_DRV_MAJOR, BPM_DRV_NAME );

    printk( BPM_MODNAME " Char Driver " BPM_VER_STR " Unregistered<%d>" 
                                                    CLRnl, BPM_DRV_MAJOR);
}


/* 
 * Cannot remove BPM module because buffers allocated by BPM are already 
 * in use with all the RX rings.
 */
static void bpm_module_exit( void )
{
    BCM_LOG_ERROR( BCM_LOG_ID_BPM, "Cannot remove BPM Module !!!" ); 

    if (bpm_buf_pool_init_done_g)
    {
        gbpm_unbind();
        bpm_free_buf_pool();
    }
    BCM_LOG_NOTICE( BCM_LOG_ID_BPM, "BPM Module Exit" ); 
    bpm_drv_destruct();
}
module_exit( bpm_module_exit );
#endif



