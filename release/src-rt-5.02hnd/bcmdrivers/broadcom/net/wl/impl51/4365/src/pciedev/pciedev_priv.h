/*
 * PCIEDEV private data structures and macro definitions
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: pciedev_priv.h  $
 */

#ifndef _pciedev_priv_h_
#define _pciedev_priv_h_

#include <typedefs.h>
#include <osl.h>
#include <bcmpcie.h>
#include <bcmmsgbuf.h>
#include <pcie_core.h>
#include <hndsoc.h>
#include <hnddma.h>
#include <dngl_api.h>
#include <rte_fetch.h>

/* **** Private macro definitions **** */

/* Audit pktid received from host */
/* #define PCIEDEV_HOST_PKTID_AUDIT_ENABLED */

#ifdef  PCIEDEV_HOST_PKTID_AUDIT_ENABLED
/* Match MAX_PKTID_ITEMS in dhd_msgbuf.c */
#define PCIEDEV_HOST_PKTID_AUDIT_MAX 8192
#endif /* PCIEDEV_HOST_PKTID_AUDIT_ENABLED */

#define MIN_RXDESC_AVAIL	4	/* one message (2) +  one ioctl rqst (2) */
#define MIN_TXDESC_AVAIL	3

/* If phase bit is supported 3 rx descriptors and
 * 2 tx descriptors are needed.
 * Otherwise 2 rx descriptors and
 * 1 tx descriptor.
 */
#define MIN_RXDESC_NEED_W_PHASE         3
#define MIN_RXDESC_NEED_WO_PHASE        2
#define MIN_TXDESC_NEED_W_PHASE         2
#define MIN_TXDESC_NEED_WO_PHASE        1
#define RXDESC_NEED_IOCTLPYLD           1
#define TXDESC_NEED_IOCTLPYLD           1

/* indexes into tunables array */
#define PCIEBUS_NTXD			1
#define PCIEBUS_NRXD			2
#define RXBUFS				3
#define RXBUFSZ				4
#define MAXHOSTRXBUFS			5
#define TXSTATUSBUFLEN			6
#define MAXRXCMPLT			7
#define RXCPLBUFLEN			8
#define H2DRXPOST			9
#define MAXTUNABLE			10

#ifndef MAX_DMA_QUEUE_LEN_H2D
#define MAX_DMA_QUEUE_LEN_H2D		64
#endif

#ifndef MAX_DMA_QUEUE_LEN_D2H
#define MAX_DMA_QUEUE_LEN_D2H		128
#endif

#define MSGBUF_RING_NAME_LEN	7

#define PCIEDEV_MAX_FLOWRINGS_FETCH_REQUESTS	40

#define LCL_BUFPOOL_AVAILABLE(ring)	((ring)->buf_pool->availcnt)
/* Circular buffer with all items usable, availcnt added for empty/free checks */
#define CIR_BUFPOOL_AVAILABLE(cpool)    (((cpool)->availcnt) ? \
	WRITE_SPACE_AVAIL_CONTINUOUS((cpool)->r_ptr, \
	(cpool)->w_ptr, (cpool)->depth) : 0)

#ifdef PCIEDEV_USE_EXT_BUF_FOR_IOCTL
#ifndef PCIEDEV_MAX_IOCTLRSP_BUF_SIZE
#define PCIEDEV_MAX_IOCTLRSP_BUF_SIZE           8192
#endif /* PCIEDEV_MAX_IOCTLRSP_BUF_SIZE */
#endif /* PCIEDEV_USE_EXT_BUF_FOR_IOCTL */

#define HOST_DMA_BUF_POOL_EMPTY(pool)	((pool->ready == NULL) ? TRUE : FALSE)

#define TRAP_DUE_DMA_RESOURCES(arg) do {PCI_ERROR(arg); while (1);}  while (0)

#define MSGBUF_RING_INIT_PHASE	0x80

#ifndef PCIEDEV_MIN_RXCPLGLOM_COUNT
#define PCIEDEV_MIN_RXCPLGLOM_COUNT 4 /* Min number of rx completions to start glomming */
#endif

/* timeout to delete suppression related info from flow */
#define PCIEDEV_FLOW_RING_SUPP_TIMEOUT	5000

/* Flow ring status */
#define FLOW_RING_ACTIVE	(1 << 0)
#define FLOW_RING_PKT_PENDING	(1 << 1)
#define FLOW_RING_PORT_CLOSED	(1 << 2)
#define FLOW_RING_DELETE_RESP_PENDING	(1 << 4)
#define FLOW_RING_FLUSH_RESP_PENDING	(1 << 5)
#define FLOW_RING_FLUSH_PENDING	(1 << 6)
#define FLOW_RING_SUP_PENDING	(1 << 7)
#define FLOW_RING_NOFLUSH_TXUPDATE	(1 << 8)
#define FLOW_RING_DELETE_RESP_RETRY     (1 << 9)
#define FLOW_RING_FLUSH_RESP_RETRY      (1 << 10)
#define FLOW_RING_PKT_IDLE (1 << 11)
#define FLOW_RING_FETCH_SKIPPED (1 << 12)
#define FLOW_RING_PKT_RESET (1 << 13)
#define FLOW_RING_PKT_RESETING (1 << 14)

/* Flow Ring reset delay 350ms after no pkt */
#define FFSCHED_FLOW_RING_RESET_DELAY	350

/* delay 3sec: in the first 3sec of Flowring active, no. of lbuf is not
 * limited by txpkts measured in 17ms, defined in PCIEDEV_SHCEDLFRAG_TXPKTS_ADJUST
 */
#define FFSCHED_FLOW_RING_ACTIVE_DELAY	3000

#define FLOW_RING_CREATE	1
#define FLOW_RING_DELETE	2
#define FLOW_RING_FLUSH		3
#define FLOW_RING_OPEN		4
#define FLOW_RING_CLOSED	5
#define FLOW_RING_FLUSHED	6
#define FLOW_RING_TIM_SET	7
#define FLOW_RING_TIM_RESET	8
#define FLOW_RING_RESET_WEIGHT	9
#define FLOW_RING_UPD_PRIOMAP	10
#define FLOW_RING_FLUSH_TXFIFO	11

/* bit 7, indicating if is TID(1) or AC(0) mapped info in tid field) */
#define PCIEDEV_IS_AC_TID_MAP_MASK 	0x80

/* Flow ring flags */
#define FLOW_RING_FLAG_LAST_TIM (1 << 0)
#define FLOW_RING_FLAG_INFORM_PKTPEND (1 << 1)
#define FLOW_RING_FLAG_PKT_REQ (1 << 2)

#define PCIE_IN_D3_SUSP_PKTMAX	2
#define PCIEDEV_EVENT_BUFPOOL_MAX	32
#define PCIEDEV_IOCTRESP_BUFPOOL_MAX	16

#define PCIEDEV_MAX_PACKETFETCH_LEN	0x4000
#ifndef PCIEDEV_MAX_PACKETFETCH_COUNT
#define PCIEDEV_MAX_PACKETFETCH_COUNT	32
#endif
#define PCIEDEV_MIN_PACKETFETCH_COUNT	4
#define PCIEDEV_INC_PACKETFETCH_COUNT	2
#ifndef PCIEDEV_MAX_LOCALITEM_COUNT
#define PCIEDEV_MAX_LOCALITEM_COUNT	32
#endif
#ifndef PCIEDEV_MAX_LOCALBUF_PKT_COUNT
#define PCIEDEV_MAX_LOCALBUF_PKT_COUNT	200
#endif

#define PCIEDEV_MIN_PKTQ_DEPTH		256

#define PCIEDEV_MIN_SHCEDLFRAG_COUNT	1
#define PCIEDEV_MAX_SHCEDLFRAG_COUNT	400

#define PCIEDEV_SHCEDLFRAG_TXPKTS_MIN	1

/* period = 50ms/3(TXPKTS_ADJUST)=17ms.  No. of lbuf
 * for each flowring is no more than by txpkts sent
 * in the period(17ms).
 */
#define PCIEDEV_SHCEDLFRAG_TXPKTS_ADJUST	3
#define PCIEDEV_SHCEDLFRAG_TXPKTS_ADJUST_MU	1


/* flowring Index */
#define FLRING_INX(x) ((x) - BCMPCIE_H2D_MSGRING_TXFLOW_IDX_START)

#define PCIEDEV_INTERNAL_SENT_D2H_PHASE		0x01
#define PCIEDEV_INTERNAL_D2HINDX_UPD		0x02

#define INUSEPOOL(ring)	((ring)->buf_pool->inuse_pool)

/* Size of local queue to store completions */
#ifndef PCIEDEV_CNTRL_CMPLT_Q_SIZE
#define PCIEDEV_CNTRL_CMPLT_Q_SIZE 8
#endif

/* Status Flag for local queue to store completions */
#define CTRL_RESP_Q_FULL        (1 << 0)

/* In the local queue,
 * PCIEDEV_CNTRL_CMPLT_Q_IOCTL_ENTRY reserved for IOCTLs
 * 1 for ACK, 1 for Completion
 */
#define PCIEDEV_CNTRL_CMPLT_Q_IOCTL_ENTRY       2
/* In the local queue,
 * PCIEDEV_CNTRL_CMPLT_Q_STATUS_ENTRY reserved for
 * general status messages
 */
#define PCIEDEV_CNTRL_CMPLT_Q_STATUS_ENTRY      1

/* H2D control submission buffer count */
#define CTRL_SUB_BUFCNT	4
#if (PCIEDEV_CNTRL_CMPLT_Q_SIZE < (PCIEDEV_CNTRL_CMPLT_Q_IOCTL_ENTRY + \
	PCIEDEV_CNTRL_CMPLT_Q_STATUS_ENTRY + CTRL_SUB_BUFCNT))
#error "PCIEDEV_CNTRL_CMPLT_Q_SIZE is too small!"
#endif

/* MAX_HOST_RXBUFS could be tuned from chip specific Makefiles */
#ifndef MAX_HOST_RXBUFS
#define MAX_HOST_RXBUFS			256
#endif

#define MAX_DMA_XFER_SZ			1540
#define MIN_DMA_LEN			5
#define MAX_DMA_BATCH_SZ		16384
#define DMA_XFER_PKTID			0xdeadbeaf
#define WL_HRD_LEN_EXPECTED		4

#define PAD_ALIGN			3

#ifdef FLOWRING_SLIDING_WINDOW
#ifndef FLOWRING_SLIDING_WINDOW_SIZE
#define	FLOWRING_SLIDING_WINDOW_SIZE	512 /* default flowring depth */
#endif
#endif /* FLOWRING_SLIDING_WINDOW */

#ifndef MAX_TX_STATUS_BUF_LEN
#define MAX_TX_STATUS_BUF_LEN		128
#endif

#ifndef MAX_TX_STATUS_QUEUE
#define MAX_TX_STATUS_QUEUE		64
#endif

#ifndef MAX_RX_CMPLT_COMBINED
#define MAX_RX_CMPLT_COMBINED		32
#endif

#ifndef MAX_RX_CMPLT_QUEUE
#define MAX_RX_CMPLT_QUEUE		64
#endif

#define MSGBUF_TIMER_D2H_QUEUE_DELAY	1
#define MSGBUF_TIMER_DELAY		1

#define PD_DMA_INT_MASK_H2D		0x1DC00
#define PD_DMA_INT_MASK_D2H		0x1DC00
#define PD_DB_INT_MASK			0xFF0000
#define PD_DEV0_DB_INTSHIFT		16

#define PD_DEV0_DB0_INTMASK       (0x1 << PD_DEV0_DB_INTSHIFT)
#define PD_DEV0_DB1_INTMASK       (0x2 << PD_DEV0_DB_INTSHIFT)
#define PD_DEV0_DB_INTMASK        ((PD_DEV0_DB0_INTMASK) | (PD_DEV0_DB1_INTMASK))
#define PD_DEV0_DMA_INTMASK       0x80


#define PD_FUNC0_MB_INTSHIFT		8
#define PD_FUNC0_MB_INTMASK		(0x3 << PD_FUNC0_MB_INTSHIFT)

#define PD_DEV0_PWRSTATE_INTSHIFT	24
#define PD_DEV0_PWRSTATE_INTMASK	(1 << 24)
#define PD_DEV0_PERST_INTMASK		(1 << 6)

#define PD_DEV0_INTMASK		\
	(PD_DEV0_DMA_INTMASK | PD_DEV0_DB0_INTMASK | PD_DEV0_PWRSTATE_INTMASK | \
	PD_DEV0_PERST_INTMASK)

#define PCIE_D2H_DB0_VAL	(0x12345678)

#ifndef PD_NTXD
#define PD_NTXD			256
#endif
#ifndef PD_NRXD
#define PD_NRXD			256
#endif
#define PD_RXBUF_SIZE		PKTBUFSZ
#define PD_RX_HEADROOM		0
#define PD_NRXBUF_POST		8

#ifndef H2D_PD_RX_OFFSET
#define H2D_PD_RX_OFFSET	8
#endif

#ifndef D2H_PD_RX_OFFSET
#define D2H_PD_RX_OFFSET	8
#endif

#ifndef MAX_RXCPL_BUF_LEN
#define MAX_RXCPL_BUF_LEN		128
#endif

#ifndef PD_NBUF_H2D_RXPOST
#define PD_NBUF_H2D_RXPOST	4
#endif

#define PCIE_DS_CHECK_INTERVAL 10
#define	PCIE_BM_CHECK_INTERVAL 5

#define PCIE_PWRMGMT_CHECK		1
#define IOCTL_PKTPOOL_SIZE	1

#define TXSTATUS_LEN            1

/* Need these to account for the rxoffsets in dma engines */
#define D2H_MSGLEN(a, b) ((a)->d2h_dma_rxoffset + (b))
#define H2D_MSGLEN(a, b) ((a)->h2d_dma_rxoffset + (b))

#define PCIE_MEM2MEM_DMA_MIN_LEN	8

/* Max TCM pkt size */
/* This includes rx offset and d11 hdr size; final trnsfer will be always less than this */
/* this has to be 4 byte aligned for both pcie and d11 dma to have aligned dest addresses */
/* tunables default values */
#define DEF_PCIE_INTMASK	0x3 << 8 /* F0 interrupt */
#define	PCIE_ADDR_OFFSET	(1 << 31)

#define MODX(x, n)       (x & (n -1))
#define align(x, n)     (MODX(x, n) ? (x - MODX(x, n) + n) : (x - MODX(x, n)))

/* Incrementing error counters */
#define PCIEDEV_MALLOC_ERR_INCR(pciedev)        (pciedev->err_cntr->mem_alloc_err_cnt++)
#define PCIEDEV_DMA_ERR_INCR(pciedev)           (pciedev->err_cntr->dma_err_cnt++)
#define PCIEDEV_RXCPL_ERR_INCR(pciedev)         (pciedev->err_cntr->rxcpl_err_cnt++)
#define PCIEDEV_PKTFETCH_CANCEL_CNT_INCR(pciedev)       (pciedev->err_cntr->pktfetch_cancel_cnt++)


#define PCIE_MAX_TID_COUNT	8
#define PCIE_MAX_DS_LOG_ENTRY 20
#define PCIE_MAX_DSCHECK_FAIL_LOG 20

/* It is desirable to minimize TX status data for performance reason */
#define TXSTATUS_SINGLE_BYTE_LEN	1

#define MAX_MEM2MEM_DMA_CHANNELS	2 /* Tx,Rx or DTOH,HTOD */
#define PCIEDEV_GET_AVAIL_DESC(pciedev, d, e) (*((pciedev)->avail[(d)][(e)]))

/*
 * PCIE Mem2Mem D2H DMA Completion Sync Options using marker in each work-item.
 * - Modulo-253 sequence number in marker
 * - XOR Checksum in marker, with module-253 seqnum in cmn msg hdr.
 */
#if defined(PCIE_M2M_D2H_SYNC_SEQNUM)
/* modulo-253 sequence number marker in D2H messages. */
#define PCIE_M2M_D2H_SYNC   PCIE_SHARED_D2H_SYNC_SEQNUM
#define PCIE_M2M_D2H_SYNC_MARKER_INSERT(msg, msglen, _epoch) \
	({ \
		BCM_REFERENCE(msglen); \
		(msg)->marker = (_epoch) % D2H_EPOCH_MODULO; \
		(_epoch)++; \
	})
#define PCIE_M2M_D2H_SYNC_MARKER_REPLACE(msg, msglen) \
	({ BCM_REFERENCE(msg); BCM_REFERENCE(msglen); })

#elif defined(PCIE_M2M_D2H_SYNC_XORCSUM)
/* Checksum in marker and (modulo-253 sequence number) in D2H Messages. */
#define PCIE_M2M_D2H_SYNC   PCIE_SHARED_D2H_SYNC_XORCSUM
#define PCIE_M2M_D2H_SYNC_MARKER_INSERT(msg, msglen, _epoch) \
	({ \
		(msg)->cmn_hdr.epoch = (_epoch) % D2H_EPOCH_MODULO; \
		(_epoch)++; \
		(msg)->marker = 0U; \
		(msg)->marker = \
			bcm_compute_xor32((uint32*)(msg), (msglen) / sizeof(uint32)); \
	})
#define PCIE_M2M_D2H_SYNC_MARKER_REPLACE(msg, msglen) \
	({ \
		(msg)->marker = 0U; \
		(msg)->marker = \
			bcm_compute_xor32((uint32*)(msg), (msglen) / sizeof(uint32)); \
	})

#else /* ! (PCIE_M2M_D2H_SYNC_SEQNUM || PCIE_M2M_D2H_SYNC_XORCSUM) */
/* Marker is unused. Either Read Barrier, MSI or alternate solution required. */
#undef PCIE_M2M_D2H_SYNC
#define PCIE_M2M_D2H_SYNC_MARKER_INSERT(msg, msglen, epoch) \
	({ BCM_REFERENCE(msg); BCM_REFERENCE(msglen); BCM_REFERENCE(epoch); })
#define PCIE_M2M_D2H_SYNC_MARKER_REPLACE(msg, msglen) \
	({ BCM_REFERENCE(msg); BCM_REFERENCE(msglen); })

#endif /* ! (PCIE_M2M_D2H_SYNC_SEQNUM || PCIE_M2M_D2H_SYNC_XORCSUM) */

/*
 * In TCM, the RD and WR indices are saved in individual RD and WR index arrays
 * for the H2D and D2H rings. pcie_rw_index_t is used to reference each RD or
 * WR index, in this array.
 *
 * DHD typically directly updates or fetches an index from dongle's memory.
 *
 * When the PCIE_DMA_INDEX feature is enabled, dongle will use the mem2mem DMA
 * to transfer an entire array of RD or WR indices to/from host memory and DHD
 * may directly access local system memory.
 *
 * Legacy -DPCIE_DMA_INDEX uses 32bit RD and WR index sizes.
 *
 * Dongle build -dmaindex16- or -dmaindex32- selects 16bit or 32bit indices with
 * dongle DMAing of indices. Legacy -DPCIE_DMA_INDEX implies 32bit inidices.
 *
 * Dongle will advertize the size of a WR or RD DMA index to DHD.
 */
#if defined(PCIE_DMA_INDEX) || defined(PCIE_DMAINDEX32)
typedef uint32 pcie_rw_index_t; /* 32bit WR and RD indices */
#elif defined(PCIE_DMAINDEX16)
typedef uint16 pcie_rw_index_t; /* 16bit WR and RD indices */
#else
typedef uint32 pcie_rw_index_t; /* 32bit WR and RD indices (default) */
#endif /* ! PCIE_DMA_INDEX32 */

#define PCIE_RW_INDEX_SZ    sizeof(pcie_rw_index_t)

#if defined(PCIE_DMAINDEX16) || defined(PCIE_DMAINDEX32)
#define PCIE_DMA_INDEX /* Dongle supports DMAing of indices to/from Host */
#endif /* PCIE_DMAINDEX16 || PCIE_DMAINDEX32 */

#define PCIEDEV_SCHEDULER_AC_PRIO	0
#define PCIEDEV_SCHEDULER_TID_PRIO	1

extern const uint8 prio2fifo[];
#define PCIEDEV_PRIO2AC(prio)  prio2fifo[(prio)]

extern const uint8 tid_prio_map[];
#define PCIEDEV_TID_PRIO_MAP(prio)  tid_prio_map[(prio)]

extern const uint8 AC_prio_map[];
#define PCIEDEV_AC_PRIO_MAP(prio)  AC_prio_map[(prio)]

extern const uint8 tid2AC_map[];
#define PCIEDEV_TID2AC(prio)  tid2AC_map[(prio)]

/* Helper Macros */
#define FL_MAXPKTCNT(zfl) ((zfl)->flow_info.maxpktcnt)

#define FL_W_NEW(zfl) ((zfl)->w_new)
#define FL_DA(zfl) ((zfl)->flow_info.da)
#define FL_TID_AC(zfl) ((zfl)->flow_info.tid_ac)
#define FL_TID_IFINDEX(zfl) ((zfl)->flow_info.ifindex)
#define FL_PKTINFLIGHT(zfl) ((zfl)->flow_info.pktinflight)
#define FL_ID(zfl) ((zfl)->ringid)
#define FL_STATUS(zfl) ((zfl)->status)

#define SCHEDCXT_SUM_W(zpciedev) ((zpciedev)->schedcxt_fl_sum_w)
#ifdef FFSHCED_SATURATED_MODE
#define SCHEDCXT_MAX_W(zpciedev) ((zpciedev)->schedcxt_fl_max_w)
#define FFSHCED_SATURATED(zpciedev) ((zpciedev)->_ffsched_saturated == 1)
#endif /* FFSHCED_SATURATED_MODE */
#define SCHEDCXT_FL_MAXLFRAGS(zfl) ((zfl)->schedcxt_weighted_max_lfrags)
#define SCHEDCXT_FL_W(zfl) ((zfl)->schedcxt_weight)
#define FFSHCED_ENAB(zpciedev) ((zpciedev)->_ffsched)
#define FFSHCED_ST(zpciedev) ((zpciedev)->_ffsched_st)
#define FFSHCED_ST_UNKNOWN(zpciedev) \
	((zpciedev)->_ffsched_st == FFSCH_STATE_UNKNOWN)
#define FFSHCED_ST_ENABLED(zpciedev) \
	((zpciedev)->_ffsched_st == FFSCH_STATE_ENABLED)
#define FFSHCED_ST_DISABLED(zpciedev) \
	((zpciedev)->_ffsched_st == FFSCH_STATE_DISABLED)

/* **** Structure definitons **** */
/* Queue to store resp to ctrl msg */
typedef struct pciedev_ctrl_resp_q {
	uint8   w_indx;
	uint8   r_indx;
	uint8   status;
	uint8   num_flow_ring_delete_resp_pend;
	uint8   num_flow_ring_flush_resp_pend;
	uint8   ctrl_resp_q_full;       /* Cnt: ring_cmplt_q is full */
	uint8   dma_desc_lack_cnt;      /* Cnt: not enough DMA descriptors */
	uint8   lcl_buf_lack_cnt;       /* Cnt: not enough local bufx */
	uint8   ring_space_lack_cnt;    /* Cnt: no space in host ring */
	uint8   dma_problem_cnt;        /* Cnt: problem w. DMA */
	ctrl_completion_item_t  response[PCIEDEV_CNTRL_CMPLT_Q_SIZE];
} pciedev_ctrl_resp_q_t;

typedef struct flow_info {
	char	sa[ETHER_ADDR_LEN];
	char	da[ETHER_ADDR_LEN];
	uint8	tid_ac;
	uint8	ifindex;
	uint16	pktinflight;
	uint16	maxpktcnt;	/* Number of packets need to keep flow to max throughput */
	uint8	flags;
	uint8	reqpktcnt;	/* Number of packets FW requested */
} flow_info_t;

/* Node to store pkt next/orev pointers */
typedef struct	lcl_buf {
	struct  lcl_buf * nxt;
	struct  lcl_buf * prv;
	void	*p;
} lcl_buf_t;

typedef struct inuse_lcl_buf {
	void* p;
	uint8 max_items;
	uint8 used_items;
} inuse_lcl_buf_t;

/* inuse lcl buf pool */
typedef struct inuse_lclbuf_pool {
	uint8 depth;
	uint8 w_ptr;
	uint8 r_ptr;
	uint8 inited;
	inuse_lcl_buf_t	buf[0];
} inuse_lclbuf_pool_t;

/** Pool of circular buffers */
typedef struct cir_buf_pool {
	uint16		depth;
	uint16		item_size;
	uint16		w_ptr;
	uint16		r_ptr;
	char*		buf;
	uint16		availcnt;
	uint16		r_pend;
	uint16		pend_item_cnt;
} cir_buf_pool_t;

/* Pool of lcl bufs */
typedef struct lcl_buf_pool {
	uint8		buf_cnt;
	uint8		item_cnt;
	uint8		availcnt;
	uint8		rsvd;
	lcl_buf_t	*head;
	lcl_buf_t	*tail;
	lcl_buf_t	*free;
	uchar		*local_buf_in_use;
	uint32		pend_item_cnt;
	inuse_lclbuf_pool_t	* inuse_pool;	/* pointer to an inuse list */
	lcl_buf_t	buf[0];
} lcl_buf_pool_t;

typedef enum bcmpcie_mb_intr_type {
	PCIE_BOTH_MB_DB1_INTR = 0,
	PCIE_MB_INTR_ONLY,
	PCIE_DB1_INTR_ONLY
} bcmpcie_mb_intr_type_t;

struct msgbuf_ring;
struct dngl_bus;
typedef struct dngl_bus pciedev_t;

typedef uint16 (*msgring_process_message_t)(struct dngl_bus *, void *p, uint16 data_len,
	struct msgbuf_ring *msgbuf);

/* double linked list node for priority ring */
struct prioring_pool {
	dll_t		node;					/* double linked node */
	dll_t		active_flowring_list;	/* Active list */
	dll_t		*last_fetch_node;		/* last fetch node in the active list */
	uint8		tid_ac;					/* 0-7 for tid, 0-3 for AC prios */
	bool		inited;					/* is in active_prioring_list */
	uint16		maxpktcnt;				/* max fetch counter */
	bool		schedule;
};

typedef struct flowring_pool	flowring_pool_t;
typedef struct prioring_pool	prioring_pool_t;

typedef struct ring_ageing_info {
	uint16 sup_cnt;
} ring_ageing_info_t;

/**
 * The 'counterpart' of a ring in host memory. Consists of a ring of local buffers, in which
 * information from host memory is transferred.
 */
typedef struct msgbuf_ring {
	bool		inited;
	uchar		name[MSGBUF_RING_NAME_LEN];
	ring_mem_t	*ringmem;
	pcie_rw_index_t *tcm_rs_w_ptr;
	pcie_rw_index_t *tcm_rs_r_ptr;
	void		*pciedev;
	uint16		wr_pending;
	uint16		rd_pending;
	bool		phase_supported;
	uint8		current_phase;
	uchar		handle;		/* upper layer ID */
	uint8		PAD;	/* increased ROM compatibility */
	uint16		ringid;
	uint16		status;
	flow_info_t	flow_info;
	lcl_buf_pool_t	* buf_pool;	/**< local buffer pool */
	msgring_process_message_t process_rtn;
	/* Bit map of packets that are being processed (fetch/queued to wl) */
	uint8		*inprocess;
	/* Bit map of packets that have completed tx (sucessfully or dropped) */
	uint8		*status_cmpl;

	/* Store for request id if we are going to send responses later */
	uint32		request_id;

	flowring_pool_t *lcl_pool;	/**< local flow ring pool */

	bool		dma_h2d_indices_supported; /* Can we DMA r/w indices from host? */
	bool		dma_d2h_indices_supported; /* Can we DMA r/w indices to host? */
	/* Bit Map of suppressed pkts to reuse d11 seq numbers */
	uint8		*reuse_sup_seq;
	/* Place holder for d11 Seq numbers from the wl */
	uint16		*reuse_seq_list;
	ring_ageing_info_t	ring_ageing_info;
#ifdef H2D_CHECK_SEQNUM
	uint8		h2d_seqnum;
#endif /* H2D_CHECK_SEQNUM */
	cir_buf_pool_t	*cbuf_pool;	/**< local circular buffer pool */
	uint32	w_new; /* flowring weight */
	 /* scheduling cycle context weight */
	uint32 schedcxt_weight;
	/* scheduling cycle context weighted lbuf max number */
	uint32 schedcxt_weighted_max_lfrags;
	uint16 bitmap_size;	/* size of bitmap arrays */

	uint32 idletimer;
	uint32 activetimer;
	uint32 lfrag_max;
	uint32 phyrate;
	uint32 mumimo;
	uint16		fetch_pending;
} msgbuf_ring_t;

/* double linked list node */
struct flowring_pool {
	dll_t node;
	msgbuf_ring_t * ring;
	prioring_pool_t *prioring;
};

typedef struct pcie_dma_info {
	uint16	len;
	uint8	msg_type;
	uint8	flags;
	msgbuf_ring_t *msgbuf;
} dma_queue_t;

typedef struct host_dma_buf {
	struct host_dma_buf *next;
	addr64_t  buf_addr;
	uint32 pktid;
	uint16	len;
	uint16	rsvd;
} host_dma_buf_t;

typedef struct host_dma_buf_pool {
	host_dma_buf_t	*free;
	host_dma_buf_t	*ready;
	uint16		max;
	uint16		ready_cnt;
	host_dma_buf_t	pool_array[0];
} host_dma_buf_pool_t;

typedef struct pciedev_err_cntrs {
	uint16 mem_alloc_err_cnt;
	uint16 rxcpl_err_cnt;
	uint16 dma_err_cnt;
	uint16 pktfetch_cancel_cnt;
} pciedev_err_cntrs_t;

typedef struct pciedev_fetch_cmplt_q {
	struct fetch_rqst *head;
	struct fetch_rqst *tail;
	uint32 count;
} pciedev_fetch_cmplt_q_t;

typedef struct flow_fetch_rqst {
	bool	used;
	uint8	index;
	uint16	start_ringinx;
	struct msgbuf_ring  *msg_ring;
	fetch_rqst_t	rqst;
	uint16  offset; /* offset location from where next processing need to done */
	uint8   flags;  /* flags to fetch node state See below */
	struct flow_fetch_rqst *next;
} flow_fetch_rqst_t;

/* Flags to indicate if fetch node went through processing once/
 * can it be freed if its order is met
 */
#define PCIEDEV_FLOW_FETCH_FLAG_REPROCESS       0x01
#define PCIEDEV_FLOW_FETCH_FLAG_FREE            0x02

typedef struct fetch_req_list {
	flow_fetch_rqst_t       *head;
	flow_fetch_rqst_t       *tail;
} fetch_req_list_t;

typedef enum bcmpcie_deepsleep_state {
	DS_INVALID_STATE = -2,
	DS_DISABLED_STATE = -1,
	NO_DS_STATE = 0,
	DS_CHECK_STATE,
	DS_D0_STATE,
	NODS_D3COLD_STATE,
	DS_D3COLD_STATE,
	DS_LAST_STATE
} bcmpcie_deepsleep_state_t;

typedef enum bcmpcie_deepsleep_event {
    DW_ASSRT_EVENT = 0,
    DW_DASSRT_EVENT,
    PERST_ASSRT_EVENT,
    PERST_DEASSRT_EVENT,
    DB_TOH_EVENT,
    DS_ALLOWED_EVENT,
    DS_NOT_ALLOWED_EVENT,
    HOSTWAKE_ASSRT_EVENT,
    DS_LAST_EVENT
} bcmpcie_deepsleep_event_t;

typedef enum ffsched_state {
	FFSCH_STATE_UNKNOWN = 0,
	FFSCH_STATE_DISABLED = 1,
	FFSCH_STATE_ENABLED = 2,
	FFSCH_STATE_LAST
} ffsched_state_t;

typedef void (*pciedev_ds_cbs_t)(void *cbarg);
typedef struct pciedev_ds_state_tbl_entry {
	pciedev_ds_cbs_t                action_fn;
	bcmpcie_deepsleep_state_t       transition;
} pciedev_ds_state_tbl_entry_t;

typedef enum _ds_check_fail {
	DS_NO_CHECK = 0,
	DS_LTR_CHECK,
	DS_BUFPOOL_PEND_CHECK,
	DS_DMA_PEND_CHECK
} ds_check_fail_t;

typedef struct _ds_check_fail_log {
	uint32 ltr_check_fail_ct;
	uint32 rx_pool_pend_fail_ct;
	uint32 tx_pool_pend_fail_ct;
	uint32 h2d_dma_txactive_fail_ct;
	uint32 h2d_dma_rxactive_fail_ct;
	uint32 d2h_dma_txactive_fail_ct;
	uint32 d2h_dma_rxactive_fail_ct;
} ds_check_fail_log_t;

typedef struct pciedev_deepsleep_log {
	bcmpcie_deepsleep_state_t       ds_state;
	bcmpcie_deepsleep_event_t ds_event;
	bcmpcie_deepsleep_state_t       ds_transition;
	uint32  ds_time;
	ds_check_fail_log_t ds_check_fail_cntrs;
} pciedev_deepsleep_log_t;

/* used to record start time for metric */
typedef struct metric_ref {
	uint32 active;
	uint32 d3_suspend;
	uint32 perst;
	uint32 ltr_active;
	uint32 ltr_sleep;
} metric_ref_t;

#ifdef PCIE_DMA_INDEX
#define PCIEDEV_MAX_INDXDMA_FETCH_REQUESTS      2
#define PCIEDEV_H2D_INDXDMA_MAXFETCHPENDING     1
#define PCIEDEV_D2H_INDXDMA_MAXFETCHPENDING     1

/* used to dma w/r indices to host memory */
typedef struct dma_indexupdate_block {
	bool host_dmabuf_inited;
	dma64addr_t host_dmabuf;
	uint32 host_dmabuf_size;
	dma64addr_t local_dmabuf;
	uint32 local_dmabuf_size;
} dma_indexupdate_block_t;
#endif /* PCIE_DMA_INDEX */

#if defined(PCIE_D2H_DOORBELL_RINGER)
/*
 * Dongle may ring doorbell on host by directly writing to a 32bit host address
 * using sbtopcie translation 0.
 */
typedef void (*d2h_doorbell_ringer_fn_t)(struct dngl_bus *pciedev,
	uint32 value, uint32 addr);

typedef struct d2h_doorbell_ringer {
	bcmpcie_soft_doorbell_t db_info; /* host doorbell, with remapped haddr */
	d2h_doorbell_ringer_fn_t db_fn;  /* callback function to ring doorbell */
} d2h_doorbell_ringer_t;

#endif /* PCIE_D2H_DOORBELL_RINGER */

/* Externally opaque dongle bus structure */
struct dngl_bus {
	uint coreid;			/* pcie core ID */
	uint corerev;			/* pcie device core rev */
	osl_t *osh;			/* Driver osl handle */
	si_t *sih;			/* SiliconBackplane handle */
	struct dngl *dngl;		/* dongle handle */
	sbpcieregs_t *regs;		/* PCIE registers */

	pciedev_shared_t *pcie_sh;	/* shared area between host & dongle */

	/* dtoh dma queu info */
	dma_queue_t dtoh_dma_q[MAX_DMA_QUEUE_LEN_D2H];
	uint32 dtoh_dma_rd_idx;
	uint32 dtoh_dma_wr_idx;

	/* hto d dma queue info */
	dma_queue_t htod_dma_q[MAX_DMA_QUEUE_LEN_H2D];
	uint16 htod_dma_rd_idx;
	uint16 htod_dma_wr_idx;

	uint32 intmask;		/* Current PCIEDEV interrupt mask */
	uint32 defintmask;	/* Default PCIEDEV intstatus mask */
	uint32 dma_defintmask;	/* default dma intmask */
	uint32 intstatus;	/* intstatus bits to process in dpc */
	hnddma_t *di;		/* DMA engine handle */

	struct pktq txq;	/* Transmit packet queue */

	bool up;			/* device is operational */
	uint32 tunables[MAXTUNABLE];

	/* BCMPKTPOOL related vars */
#ifdef BCMFRAGPOOL
	pktpool_t *pktpool_lfrag;	/* TX frag */
#endif
	pktpool_t *pktpool_rxlfrag;	/* RX frag */
	pktpool_t *pktpool;		/* pktpool lbuf */

	void*	savedpkt;
	uint16 savedlen;
	uint32 lcl_rdptr;
	uint8 *dummy_rxoff;

	/* pointers to dma engines */
	hnddma_t        *h2d_di;
	hnddma_t        *d2h_di;

	uint32		h2dintstatus;
	uint32		d2hintstatus;
	uint32		rx_dma_pending;
	struct pcie_phtm *phtm;
	uint32		d2h_dma_rxoffset;
	uint32		h2d_dma_rxoffset;
	void*		ioctl_pktptr;

	/* Pointers to DTOH/HTOD TX/RXavail */
	uint* avail[MAX_MEM2MEM_DMA_CHANNELS][MAX_MEM2MEM_DMA_CHANNELS];

#ifdef PKTC_TX_DONGLE
	void		*pkthead;
	void		*pkttail;
	uint8		pkt_chain_cnt;
#endif /* PKTC_TX_DONGLE */
	uint8 prev_ifidx;
	uint32		dropped_txpkts;
	uint32		dropped_chained_rxpkts;
	dngl_timer_t	*ltr_test_timer; /* 0 delay timer used to send txstatus and rxcomplete */
	uint8		usr_ltr_test_state;
	bool		usr_ltr_test_pend;
#ifdef PCIE_DMAXFER_LOOPBACK
	struct pktq	lpbk_dma_txq; /* Loopback Transmit packet queue */
	struct pktq	lpbk_dma_txq_pend; /* Loopback Transmit packet queue */
	uint32		pend_dmaxfer_len;
	uint32		pend_dmaxfer_srcaddr_hi;
	uint32		pend_dmaxfer_srcaddr_lo;
	uint32		pend_dmaxfer_destaddr_hi;
	uint32		pend_dmaxfer_destaddr_lo;
	uint32			lpbk_dma_src_delay;
	uint32			lpbk_dma_dest_delay;
	uint32		lpbk_dmaxfer_fetch_pend;
	uint32		lpbk_dmaxfer_push_pend;
	uint32		lpbk_dma_pkt_fetch_len;
	uint32		lpbk_dma_req_id;
	dngl_timer_t	*lpbk_src_dmaxfer_timer;	/* delay timer to add delay in src DMA */
	dngl_timer_t	*lpbk_dest_dmaxfer_timer;	/* delay timer to add delay in dest DMA */
	uint32		txlpbk_pkts;
#endif /* PCIE_DMAXFER_LOOPBACK */


	bool ioctl_lock;

	uint32 *h2d_mb_data_ptr;
	uint32 *d2h_mb_data_ptr;
	uint8	cur_ltr_state;
	bool	deepsleep_disable_state;
	uint32	device_link_state;
	uint8 	host_wake_gpio;
#ifdef PCIE_DELAYED_HOSTWAKE
	dngl_timer_t	*delayed_hostwake_timer;
	bool	delayed_hostwake_timer_active;
#endif /* PCIE_DELAYED_HOSTWAKE */
	bool	in_d3_suspend;
	bool	no_device_inited_d3_exit;
	uint32	in_d3_pktcount;
	bool	real_d3;
	bool	bm_not_enabled;
	dngl_timer_t	*bm_enab_timer;
	bool	bm_enab_timer_on;
	uint32	bm_enab_check_interval;
	bool	d3_ack_pending;

	struct pktq	d2h_req_q; /* rx completion queue */

	dma64addr_t d2h_dma_scratchbuf;
	uint32 d2h_dma_scratchbuf_len;

	msgbuf_ring_t	*dtoh_txcpl;
	msgbuf_ring_t	*dtoh_rxcpl;
	msgbuf_ring_t	*dtoh_ctrlcpl;

	msgbuf_ring_t	*htod_tx;
	msgbuf_ring_t	*htod_rx;
	msgbuf_ring_t	*htod_ctrl;

	bool	common_rings_attached;
	bool	scratch_inited;
	bool	d2h_ringupd_inited;

	msgbuf_ring_t	cmn_msgbuf_ring[BCMPCIE_COMMON_MSGRINGS];
	msgbuf_ring_t	txp_msgbuf;
	host_dma_buf_pool_t	*event_pool;
	host_dma_buf_pool_t	*ioctl_resp_pool;

	pciedev_fetch_cmplt_q_t *fcq;
	uint32 copycount;	/* used for fifo split mode */
	uint32 tcmsegsz;	/* used in descriptr split mode */
	uint32 d11rxoffset;	/* rxoffset used for d11 dma */
	uint16	msg_pending;
	dngl_timer_t	*delayed_ioctlunlock_timer; /* 0 delay timer used to resched
			                             * the msgq processing
			                             */
	struct fetch_rqst *ioctptr_fetch_rqst;

#ifdef PCIE_DMA_INDEX
	/* flag to check for rx/tx desc. availability for DMAing indices */
	uint8	dma_d2h_indices_pending;

	/* Ring Update Buffer for Write/Read Indices for Submissions/Completions */
	dma_indexupdate_block_t *h2d_writeindx_dmablock;
	dma_indexupdate_block_t *h2d_readindx_dmablock;
	dma_indexupdate_block_t *d2h_writeindx_dmablock;
	dma_indexupdate_block_t *d2h_readindx_dmablock;

	struct	fetch_rqst h2d_indxdma_fetch_rqst;
	uint16	h2d_indexdma_fetch_pending;
	bool	h2d_indexdma_fetch_needed;

	struct	fetch_rqst d2h_indxdma_fetch_rqst;
	uint16	d2h_indexdma_fetch_pending;
	bool	d2h_fetch_done;
	bool	d2h_indexdma_fetch_needed;
#endif /* PCIE_DMA_INDEX */

	msgbuf_ring_t	*flow_ring_msgbuf;
	dngl_timer_t	*flow_schedule_timer;
	bool		flow_sch_timer_on;
	flow_fetch_rqst_t	flowring_fetch_rqsts[PCIEDEV_MAX_FLOWRINGS_FETCH_REQUESTS];
	uint8		last_fetch_ring;
	bool		ioctl_pend;
	bool		ioctl_ack_pend;
	bool		ioctl_cmplt_pend;
	cir_buf_pool_t  *cir_flowring_buf;

	uint32	mailboxintmask;
	bool	pciecoreset;
	bool	txcompletion_pend;
	bool	rxcompletion_pend;
	bool	force_no_tx_metadata;
	bool	force_no_rx_metadata;
	bool	health_check_timer_on;
	uint32	health_check_period; /* ms */
	uint32 last_health_check_time; /* ms */
	uint32	pend_user_tx_pkts;
	uint32	d3_wait_for_txstatus;

	/* ampdu buffer reorder info */
	uint32		rxcpl_pend_cnt;
	rxcpl_info_t	*rxcpl_list_h;
	rxcpl_info_t	*rxcpl_list_t;
	bcmpcie_deepsleep_state_t ds_state;
	dngl_timer_t	*ds_check_timer;
	bool	ds_check_timer_on;
	bool	ds_check_timer_del;
	uint32	ds_check_timer_max;
	uint32	ds_check_interval;
	pciedev_deepsleep_log_t ds_log[PCIE_MAX_DS_LOG_ENTRY];
	uint32 ds_log_count;

	pktpool_t *ioctl_req_pool;	/* ioctl req buf pool */

	/* Keep track of overall pkt fetches pending */
	uint16	fetch_pending;
	pcie_bus_metrics_t metrics; /* tallies used to measure power state behavior */
	metric_ref_t metric_ref; /* used to record start time for metric */
	dll_pool_t * flowring_pool;	/* pool of flow rings */
	prioring_pool_t *prioring;	/* pool of prio ring */
	dll_t active_prioring_list; /* Active priority ring list */
	uint8	uarttrans_enab;
	uint8	uart_event_inds_mask[WL_EVENTING_MASK_LEN];
	uint16	event_seqnum;
	bool event_delivery_pend;
	bool	lpback_test_mode;
	uint32	lpback_test_drops;
	uchar  *ioct_resp_buf;          /* buffer to store ioctl response from dongle */
	uint32  ioct_resp_buf_len;      /* buffer to store ioctl response from dongle */
	pciedev_err_cntrs_t *err_cntr;

#if defined(PCIEDEV_HOST_PKTID_AUDIT_ENABLED)
	struct bcm_mwbmap * host_pktid_audit; /* PCIEDEV_HOST_PKTID_AUDIT_ENABLED */
#endif /* PCIEDEV_HOST_PKTID_AUDIT_ENABLED */

#if defined(PCIE_M2M_D2H_SYNC)
	/* Per D2H ring, modulo-253 sequence number state. */
	uint32  rxbuf_cmpl_epoch;
	uint32  txbuf_cmpl_epoch;
	uint32  ctrl_compl_epoch;
#endif /* PCIE_M2M_D2H_SYNC */

#if defined(PCIE_D2H_DOORBELL_RINGER)
	/* doorbell ringing using sbtopcie translation 0 host memory write */
	d2h_doorbell_ringer_t d2h_doorbell_ringer[BCMPCIE_D2H_COMMON_MSGRINGS];
	bool	d2h_doorbell_ringer_inited;
#endif /* PCIE_D2H_DOORBELL_RINGER */

	uint32 mb_intmask;  /* mail box/db1 intmask */
	bcmpcie_mb_intr_type_t  db1_for_mb; /* whether to use db1 or mb for mail box interrupt */
	dngl_timer_t    *glom_timer;    /* Timer to glom rx status and send up */
	bool glom_timer_active;     /* timer active or not */
	uint8 dw_gpio; /* GPIO usied for DEVICE_WAKE */
	bool  force_ht_on;
	bool		d0_mb_in_use; /* Use db interrupt to enter D0 */
	uint8	schedule_prio;	/* use tid or AC based priority scheduler */
	bool ltr_sleep_after_d0; /* Flag to indicate whether ltr_sleep sent */
	uint32 d0_miss_counter; /* Count how many D0 miss occurred */
	uint32	h2d_doorbell_ignore;
	dngl_timer_t	*flow_ageing_timer;
	bool		flow_ageing_timer_on;
	uint8		flow_supp_enab; /* the no.of flowrings capable of handling suppression */
	uint16		flow_age_timeout; /* flow age timeout */
	/* Queue control completions */
	pciedev_ctrl_resp_q_t *ctrl_resp_q;
	fetch_req_list_t	fetch_req_pend_list; /* pending fetch list to be processed */
	/* Scheduler context */
	uint32 schedcxt_fl_sum_w; /* sum of weight of all pending flowrings per scheduling cycle */
	uint32 schedcxt_pktpool_avail; /* avaliable lfrags in pkt pool per scheduling context */
	bool _ffsched; /* Enable/Disable flag Fair Fetch Scheduling Feature */
	ffsched_state_t _ffsched_st; /* The current state flag Fair Fetch Scheduling Feature */
#ifdef BCM_HOST_MEM
	dma64addr_t d2h_dma_hostbuf;
	uint32 d2h_dma_hostbuf_len;
#endif /* BCM_HOST_MEM */
	uint32 flr_lfrag_max;
	uint32 flr_lfrag_txpkts_min;
	uint32 flr_lfrag_txpkts_adjust;
	uint32 flr_lfrag_txpkts_adjust_mu;
	uint32 ffsched_flr_rst_delay;
#ifdef FFSHCED_SATURATED_MODE
	uint32 _ffsched_saturated; /* make sure each ring can get max lfrags */
	uint32 schedcxt_fl_max_w; /* Max weight among tx flowring weighting */
#endif /* FFSHCED_SATURATED_MODE */
};

#endif /* _pciedev_priv_h */
