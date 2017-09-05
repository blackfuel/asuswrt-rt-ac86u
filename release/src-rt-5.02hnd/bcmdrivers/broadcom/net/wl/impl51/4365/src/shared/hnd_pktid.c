/*
 * Packet ID to pointer mapping source
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: $
 */

#if defined(BCMPKTIDMAP)

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <hnd_pktid.h>
#include <rte_cons.h>

#ifdef BCMDBG
#define PKTID_MSG(x) printf x
#else
#define PKTID_MSG(x)
#endif

/* Hierarchical multiword bit map for unique 16bit id allocator */
static struct bcm_mwbmap * hnd_pktid_map = (struct bcm_mwbmap *)NULL;
static uint32 hnd_pktid_max = 0U; /* mwbmap allocator dimension */
static uint32 hnd_pktid_failure_count = 0U; /* pktid alloc failure count */

#ifdef RTE_CONS
static void hnd_print_pktid(void *arg, int argc, char *argv[]);
#endif


/* Associative array of pktid to pktptr */
struct lbuf **hnd_pktptr_map = NULL;


/* Support for 32bit pktptr to 16bit pktid for memory conservation */
/*
 * Prior to constructing packet pools, a packet ID to packet PTR mapping service
 * must be initialized.
 * Instantiate a hierarchical multiword bit map for 16bit unique pktid allocator
 * Instantiate a reverse pktid to pktptr map.
 */
void
BCMATTACHFN(hnd_pktid_init)(osl_t * osh, uint32 pktids_total)
{
	uint32 mapsz;
	pktids_total += 1; /* pktid 0 is reserved */

	ASSERT(PKT_MAXIMUM_ID < 0xFFFF);
	ASSERT(pktids_total <= PKT_MAXIMUM_ID);
	ASSERT((uint16)(BCM_MWBMAP_INVALID_IDX) == PKT_INVALID_ID);

	/* Instantiate a hierarchical multiword bitmap for unique pktid allocator */
	hnd_pktid_map = bcm_mwbmap_init(osh, pktids_total);
	if (hnd_pktid_map == (struct bcm_mwbmap *)NULL) {
		ASSERT(0);
		return;
	}

	/* Instantiate a pktid to pointer associative array */
	mapsz = sizeof(struct lbuf *) * pktids_total;
	if ((hnd_pktptr_map = MALLOCZ(osh, mapsz)) == NULL) {
		ASSERT(0);
		goto error;
	}

	/* reserve pktid #0 and setup mapping of pktid#0 to NULL pktptr */
	ASSERT(PKT_NULL_ID == (uint16)0);
	bcm_mwbmap_force(hnd_pktid_map, PKT_NULL_ID);

	ASSERT(!bcm_mwbmap_isfree(hnd_pktid_map, PKT_NULL_ID));
	hnd_pktptr_map[PKT_NULL_ID] = (struct lbuf *)(NULL);

	/* pktid to pktptr mapping successfully setup, with pktid#0 reserved */
	hnd_pktid_max = pktids_total;

#ifdef RTE_CONS
	hnd_cons_add_cmd("pktid", hnd_print_pktid, 0);
#endif

	return;

error:
	bcm_mwbmap_fini(osh, hnd_pktid_map);
	hnd_pktid_max = 0U;
	hnd_pktid_map = (struct bcm_mwbmap *)NULL;
	return;
}

void /* Increment pktid alloc failure count */
hnd_pktid_inc_fail_cnt(void)
{
	hnd_pktid_failure_count++;
}

uint32 /* Fetch total number of pktid alloc failures */
hnd_pktid_fail_cnt(void)
{
	return hnd_pktid_failure_count;
}

uint32 /* Fetch total number of free pktids */
hnd_pktid_free_cnt(void)
{
	return bcm_mwbmap_free_cnt(hnd_pktid_map);
}

/* Allocate a unique pktid and associate the pktptr to it */
uint16
hnd_pktid_allocate(const struct lbuf * pktptr)
{
	uint32 pktid;

	pktid = bcm_mwbmap_alloc(hnd_pktid_map); /* allocate unique id */

	ASSERT(pktid < hnd_pktid_max);
	if (pktid < hnd_pktid_max) { /* valid unique id allocated */
		ASSERT(pktid != 0U);
		/* map pktptr @ pktid */
		hnd_pktptr_map[pktid] = (struct lbuf *)pktptr;
	}

	return (uint16)(pktid);
}

/* Release a previously allocated unique pktid */
void
hnd_pktid_release(const struct lbuf * pktptr, const uint16 pktid)
{
	ASSERT(pktid != 0U);
	ASSERT(pktid < hnd_pktid_max);
	ASSERT(hnd_pktptr_map[pktid] == (struct lbuf *)pktptr);

	hnd_pktptr_map[pktid] = (struct lbuf *)NULL; /* unmap pktptr @ pktid */
	/* BCMDBG:
	 * hnd_pktptr_map[pktid] = (struct lbuf *) (0xdead0000 | pktid);
	 */
	bcm_mwbmap_free(hnd_pktid_map, (uint32)pktid);
}

bool
hnd_pktid_sane(const struct lbuf * pktptr)
{
	int insane = 0;
	uint16 pktid = 0;
	struct lbuf * lb = (struct lbuf *)(pktptr);

	pktid = PKTID(lb);

	insane |= pktid >= hnd_pktid_max;
	insane |= (hnd_pktptr_map[pktid] != lb);

	if (insane) {
		ASSERT(pktid < hnd_pktid_max);
		ASSERT(hnd_pktptr_map[pktid] == lb);
		PKTID_MSG(("hnd_pktid_sane pktptr<%p> pktid<%u>\n", pktptr, pktid));
	}

	return (!insane);
}

#ifdef RTE_CONS
static void
hnd_print_pktid(void *arg, int argc, char *argv[])
{
	printf("\tPktId Total: %d, Free: %d, Failed: %d\n",
		hnd_pktid_max, hnd_pktid_free_cnt(), hnd_pktid_fail_cnt());
}
#endif /* RTE_CONS */
#endif /* BCMPKTIDMAP */
