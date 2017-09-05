/*
 * PCIEDEV device API
 * pciedev is the bus device for pcie full dongle
 * Core bus operations are as below
 * 1. interrupt mechanism
 * 2. circular buffer
 * 3. message buffer protocol
 * 4. DMA handling
 * pciedev.h exposes pciedev functins required outside
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: pciedev.h  $
 */

#ifndef	_pciedev_h_
#define	_pciedev_h_

#include <typedefs.h>
#include <osl.h>
#include <pciedev_priv.h>
#include <circularbuf.h>
struct pcidev;

#ifdef PCIE_PHANTOM_DEV
struct pcie_phtm;
struct dngl_bus;
#endif /* PCIE_PHANTOM_DEV */

/* gpioX interrupt handler */
typedef void (*pciedev_gpio_handler_t)(uint32 stat, void *arg);
typedef struct pciedev_gpioh pciedev_gpioh_t;

/* Chip operations */
extern struct dngl_bus *pciedev_attach(void *drv, uint vendor, uint device, osl_t *osh,
                                      void *regs, uint bus);

extern void *pciedev_dngl(struct dngl_bus *pciedev);
void pciedev_bus_rebinddev(void *bus, void *dev, int ifindex);
int pciedev_bus_binddev(void *bus, void *dev, uint numslaves);
extern void pciedev_init(struct dngl_bus *pciedev);
extern void pciedev_intrsoff(struct dngl_bus *pciedev);
extern void pciedev_intrs_deassert(struct dngl_bus *pciedev);
extern bool pciedev_dispatch(struct dngl_bus *pciedev);
extern void pciedev_intrson(struct dngl_bus *pciedev);
extern void pciedev_health_check(uint32 ms);

#define DEV_DMA_IOCT_PYLD 	0x1
#define DEV_DMA_IOCT_CMPLT 	0x2
#define DEV_DMA_TX_CMPLT	0x4
#define DEV_DMA_RX_CMPLT	0x8
#define	DEV_DMA_RX_PYLD		0x10
#define DEV_DMA_WL_EVENT	0x20
#define DEV_DMA_LOOPBACK_REQ	0x40
#define PCIE_ERROR1			1
#define PCIE_ERROR2			2
#define PCIE_ERROR3			3
#define PCIE_ERROR4			4
#define PCIE_ERROR5			5
#define PCIE_ERROR6			6
#define PCIE_WARNING			7

#define IOCTL_PKTBUFSZ          2048

enum {
	HTOD = 0,
	DTOH
};
enum {
	RXDESC = 0,
	TXDESC
};

#ifdef PCIE_PHANTOM_DEV
extern	struct pcie_phtm * pcie_phtm_attach(struct dngl_bus *pciedev, osl_t *osh, si_t *sih,
	void *regs, pciedev_shared_t *pcie_sh, uint32 rxoffset);
extern void pcie_phtm_init(struct pcie_phtm *phtm);
extern void pcie_phtm_bit0_intr_process(struct pcie_phtm *phtm);
extern void pcie_phtm_msgbuf_dma(struct pcie_phtm *phtm, void *dst,
	dma64addr_t src, uint16 src_len);
extern void pcie_phtm_reset(struct pcie_phtm *phtm);
extern void pcie_phtm_dma_sts_update(struct pcie_phtm *phtm, uint32 status);
extern void pcie_phtm_tx(struct pcie_phtm *phtm, void *p,
	dma64addr_t addr, uint16 msglen, uint8 msgtype);
extern void phtm_ring_dtoh_doorbell(struct pcie_phtm *phtm);
extern void pcie_phtm_sd_isr(struct dngl_bus *pciedev);
extern void pcie_phtm_usb_isr(struct dngl_bus *pciedev);
#endif /* PCIE_PHANTOM_DEV */
extern void pciedev_process_tx_payload(struct dngl_bus *pciedev);
extern circularbuf_t *pciedev_htodlcl(struct dngl_bus *pciedev);
extern circularbuf_t *pciedev_htodlcl_ctrl(struct dngl_bus *pciedev);
extern circularbuf_t *pciedev_htod_ctrlbuf(struct dngl_bus *pciedev);
extern circularbuf_t *pciedev_htodmsgbuf(struct dngl_bus *pciedev);
extern bool  pciedev_msgbuf_intr_process(struct dngl_bus *pciedev);
extern struct pcie_phtm *pciedev_phtmdev(struct dngl_bus *pciedev);
extern void pciedev_intrsupd(struct dngl_bus *pciedev);
extern bool pciedev_dpc(struct dngl_bus *pciedev);
extern void pciedev_detach(struct dngl_bus *pciedev);
void pciedev_bus_sendctl(struct dngl_bus *bus, void *p);
uint32 pciedev_bus_iovar(struct dngl_bus *pciedev, char *buf,
	uint32 inlen, uint32 *outlen, bool set);
int pciedev_bus_unbinddev(void *bus, void *dev);
extern uint16 pciedev_sendup(struct dngl_bus *pciedev, void *p,
	uint16 pktlen, uint16 msglen, uint16 ring);
extern void pciedev_up_host_rptr(struct dngl_bus *pciedev, uint16 msglen);
extern void pciedev_up_lcl_rptr(struct dngl_bus *pciedev, uint16 msglen);
extern void pciedev_xmit_ioct_cmplt(struct dngl_bus *bus);
extern bool pciedev_handle_d2h_dmacomplete(struct dngl_bus *pciedev, void *buf);
extern void pciedev_set_h2dring_rxoffset(struct dngl_bus *pciedev, uint16 offset, bool ctrl);
extern int pciedev_create_d2h_messages_tx(struct dngl_bus *pciedev, void *p);
extern int pciedev_bus_flring_ctrl(void *bus, uint32 op, void *data);
extern void pciedev_upd_flr_hanlde_map(struct dngl_bus * pciedev, uint8 handle,
	bool add, uint8 *addr);
extern void pciedev_upd_flr_tid_state(struct dngl_bus * pciedev, uint8 tid_ac, bool open);
extern void pciedev_upd_flr_if_state(struct dngl_bus * pciedev, uint8 ifindex, bool open);
extern void pciedev_upd_flr_port_handle(struct dngl_bus * pciedev, uint8 handle, bool open);
extern void pciedev_process_reqst_packet(struct dngl_bus * pciedev,
	uint8 handle, uint8 ac_bitmap, uint8 count);
extern void pciedev_upd_flr_weight(struct dngl_bus * pciedev, uint8 mac_handle,
	uint8 ac_tid, void *params);
extern void pciedev_set_ffsched(struct dngl_bus * pciedev, void *params);
extern bool pciedev_handle_h2d_dma(struct dngl_bus *pciedev);
extern void pciedev_flow_schedule_timerfn(dngl_timer_t *t);
extern void pciedev_flow_ageing_timerfn(dngl_timer_t *t);
extern int pciedev_read_flow_ring_host_buffer(struct dngl_bus *pciedev,
	msgbuf_ring_t *flow_ring, uint16 pciedev_fetch_pending);
extern void pciedev_schedule_flow_ring_read_buffer(struct dngl_bus *pciedev);
extern void* pciedev_get_lclbuf_pool(msgbuf_ring_t * ring);
extern void pciedev_free_lclbuf_pool(msgbuf_ring_t * ring, void* p);
extern int pciedev_fillup_rxcplid(pktpool_t *pool, void* arg, void* frag, bool dummy);
extern int pciedev_fillup_haddr(pktpool_t *pool, void* arg, void* frag, bool rxcpl_needed);
extern void pciedev_flush_chained_pkts(struct dngl_bus *pciedev);
extern void pciedev_flush_glommed_rxstatus(dngl_timer_t *t);
extern void pciedev_queue_rxcomplete_local(struct dngl_bus *pciedev, rxcpl_info_t *rxcpl_info,
	msgbuf_ring_t *ring, bool flush);
extern uint8 pciedev_xmit_msgbuf_packet(struct dngl_bus *pciedev, void *p, uint16  msglen,
	msgbuf_ring_t *msgbuf);
extern bool pciedev_lbuf_callback(void *arg, void* p);
extern int pciedev_tx_pyld(struct dngl_bus *bus, void *p, ret_buf_t *ret_buf, uint16 msglen,
	ret_buf_t *metadata_buf, uint16 metadata_buf_len, uint8 msgtype);
extern uint16 pciedev_handle_h2d_msg_rxsubmit(struct dngl_bus *pciedev, void *p,
	uint16 pktlen, msgbuf_ring_t *msgbuf);
extern void pciedev_process_pending_fetches(struct dngl_bus *pciedev);
extern uint16 pciedev_handle_h2d_msg_txsubmit(struct dngl_bus *pciedev, void *p,
	uint16 pktlen, msgbuf_ring_t *msgbuf, uint16 fetch_indx, uint32 flags);
extern void pciedev_dma_tx_account(struct dngl_bus *pciedev, uint16 msglen, uint8 msgtype,
	uint8 flags, msgbuf_ring_t *msgbuf);
#ifndef PCIE_PHANTOM_DEV
extern bool pciedev_handle_d2h_dma(struct dngl_bus *pciedev);
extern void pciedev_D3_ack(struct dngl_bus *pciedev);
#endif
extern void pciedev_queue_d2h_req_send(struct dngl_bus *pciedev);
extern void pciedev_process_ioctl_pend(struct dngl_bus *pciedev);
extern uint8 pciedev_ctrl_resp_q_avail(struct dngl_bus *pciedev);
extern int pciedev_process_ctrl_cmplt(struct dngl_bus *pciedev);
extern int pciedev_process_d2h_wlevent(struct dngl_bus *pciedev, void* p);
#if defined(PCIE_D2H_DOORBELL_RINGER)
void pciedev_d2h_doorbell_ringer(struct dngl_bus *pciedev,
    uint32 db_data, uint32 db_addr);
#endif /* PCIE_D2H_DOORBELL_RINGER */
extern void pciedev_generate_host_db_intr(struct dngl_bus *pciedev, uint32 data, uint32 db);
extern int pciedev_init_sharedbufs(struct dngl_bus *pciedev);
extern void pciedev_send_flow_ring_flush_resp(struct dngl_bus * pciedev, uint16 flowid,
	uint32 status);
extern int pciedev_send_flow_ring_delete_resp(struct dngl_bus * pciedev,
	uint16 flowid, uint32 status);
extern void pciedev_process_ioctl_done(struct dngl_bus *pciedev);
extern void pciedev_process_pending_flring_resp(struct dngl_bus * pciedev,
	msgbuf_ring_t *flow_ring);
extern void pciedev_host_wake_gpio_enable(struct dngl_bus *pciedev, bool state);
extern void pciedev_process_ioctl_pend(struct dngl_bus *pciedev);
extern void pciedev_queue_d2h_req(struct dngl_bus *pciedev, void *p);
extern int pciedev_dispatch_fetch_rqst(struct fetch_rqst *fr, void *arg);
#ifdef PCIE_DMAXFER_LOOPBACK
extern uint32 pciedev_process_do_dest_lpbk_dmaxfer_done(struct dngl_bus *pciedev, void *p);
extern int pciedev_process_d2h_dmaxfer_pyld(struct dngl_bus *pciedev, void *p);
#endif
extern void pciedev_rxreorder_queue_flush_cb(void *ctx, uint16 list_start_id, uint32 count);
#ifndef PCIE_PHANTOM_DEV
extern void pciedev_send_ltr(void *dev, uint8 state);
#endif
extern bool pciedev_init_flowrings(struct dngl_bus *pciedev);
#ifdef PCIE_DMA_INDEX
extern void pciedev_dma_set_indices(struct dngl_bus *pciedev, msgbuf_ring_t *msgbuf_ring);
extern void pciedev_dma_get_indices(struct dngl_bus *pciedev);
#endif /* PCIE_DMA_INDEX */
void pciedev_set_copycount_bytes(struct dngl_bus *pciedev, uint32 copycount, uint32 d11rxoffset);
#ifdef PCIEDEV_DBG_LOCAL_POOL
extern void pciedev_check_free_p(msgbuf_ring_t * ring, lcl_buf_t *free, void *p,
	const char *caller);
#endif

extern void pciedev_xmit_txstatus(struct dngl_bus *pciedev, msgbuf_ring_t *ring);
extern pciedev_gpioh_t * pciedev_gpio_handler_register(uint32 event, bool level,
	pciedev_gpio_handler_t cb, void *arg);
extern void pciedev_gpio_handler_unregister(pciedev_gpioh_t *gi);
extern void pciedev_flowring_fetch_cb(struct fetch_rqst *fr, bool cancelled);
extern void pciedev_reset_flowring_weight(struct dngl_bus *pciedev, msgbuf_ring_t *flow_ring);
extern void pciedev_reset_all_flowrings_weight(struct dngl_bus *pciedev);
#endif	/* _pciedev_h_ */
