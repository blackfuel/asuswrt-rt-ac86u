/*
 * HND packet buffer routines.
 *
 * No caching,
 * Just a thin packet buffering data structure layer atop malloc/free .
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: hnd_lbuf.c 626003 2016-03-18 11:29:07Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>

#include <hnd_lbuf.h>
#include <hnd_pt.h>

#ifdef BCMDBG
#define LBUF_MSG(x) printf x
#else
#define LBUF_MSG(x)
#endif

#define LBUF_BIN_SIZE	6

#if defined(BCMLFRAG) && !defined(BCMLFRAG_DISABLED)
bool _bcmlfrag = TRUE;
#else
bool _bcmlfrag = FALSE;
#endif

typedef struct {
	lbuf_free_cb_t cb;
	void* arg;
} lbuf_freeinfo_t;

static lbuf_freeinfo_t lbuf_info;

static lbuf_freeinfo_t* lbuf_info_get(void);

static lbuf_freeinfo_t*
BCMRAMFN(lbuf_info_get)(void)
{
	return (&lbuf_info);
}

void lbuf_free_register(lbuf_free_cb_t cb, void* arg)
{
	lbuf_freeinfo_t *lbuf_infoptr;

	lbuf_infoptr = lbuf_info_get();

	lbuf_infoptr->cb = cb;
	lbuf_infoptr->arg = arg;
}

static const uint lbsize[LBUF_BIN_SIZE] = {
	MAXPKTBUFSZ >> 4,
	MAXPKTBUFSZ >> 3,
	MAXPKTBUFSZ >> 2,
	MAXPKTBUFSZ >> 1,
	MAXPKTBUFSZ,
#ifdef HNDLBUFCOMPACT
	/* The compact lbuf scheme is implemented based on imposing limit
	   on max size to LB_BUFSIZE_MAX
	*/
	LB_BUFSIZE_MAX + LBUFSZ
#else
	4096 + LBUFSZ		/* ctrl queries on bus can be 4K */
#endif
};

#if !defined(BCMPKTPOOL) || defined(BCMDBG)
static uint lbuf_hist[LBUF_BIN_SIZE] = {0};
#endif
static uint lbuf_allocfail = 0;

#ifdef BCMPCIEDEV
static lbuf_free_global_cb_t lbuf_free_cb = NULL;
#endif

void
lb_init()
{
	ASSERT(sizeof(struct lbuf) == LBUFSZ);
}

/* Set flags and size according to pkt type */
static void
set_lbuf_params(enum lbuf_type lbuf_type, uint16* flags, uint* lbufsz)
{
	switch (lbuf_type) {
	case lbuf_basic:
		/* legacy pkt */
		*flags = 0;
		*lbufsz = LBUFSZ;
		break;
	case lbuf_frag:
		/* tx frag */
		*flags = LBF_TX_FRAG;
		*lbufsz = LBUFFRAGSZ;
		break;
	case lbuf_rxfrag:
		/* rx frag */
		*flags = 0;
		*lbufsz = LBUFFRAGSZ;
		break;
	default:
		break;
	}
}

struct lbuf *
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
lb_alloc(uint size, enum lbuf_type lbuf_type, const char *file, int line)
#else
lb_alloc(uint size, enum lbuf_type lbuf_type)
#endif
{
	uchar * head;
	struct lbuf *lb;
	uint tot, lbufsz, end_off;
	uint16 flags;

	if (hnd_pktid_free_cnt() == 0) {
		LBUF_MSG(("lb_alloc: hnd_pktid_free_cnt 0\n"));
		hnd_pktid_inc_fail_cnt();
		goto error;
	}

	tot = 0;
	flags = 0;
	lbufsz = 0;

	/* set lbuf params based on type */
	set_lbuf_params(lbuf_type, &flags, &lbufsz);

#ifdef HND_PT_GIANT
	if (size > MAXPKTBUFSZ + lbufsz) {	/* giant rx pkt, get from fixed block partition */

		tot = lbufsz + ROUNDUP(size, sizeof(int));
		/* ASSERT(tot <= MEM_PT_BLKSIZE); */
		if ((lb = (struct lbuf*)hnd_malloc_pt(tot)) != NULL) {
			flags |= LBF_PTBLK;
			goto success;
		}

		LBUF_MSG(("lb_alloc: size (%u); alloc failed;\n", tot));
		goto error;
	}
#endif

#ifdef BCMPKTPOOL
	/* Don't roundup size if PKTPOOL enabled */
	tot = lbufsz + ROUNDUP(size, sizeof(int));
	if (tot > lbsize[ARRAYSIZE(lbsize)-1]) {
		LBUF_MSG(("lb_alloc: size too big (%u); alloc failed;\n",
		       (lbufsz + size)));
		goto error;
	}
#else
	{
		int i;
		for (i = 0; i < ARRAYSIZE(lbsize); i++) {
			if ((lbufsz + ROUNDUP(size, sizeof(int))) <= lbsize[i]) {
				tot = lbsize[i];
				lbuf_hist[i]++;
				break;
			}
		}
	}
#endif /* BCMPKTPOOL */

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	if (!file)
		file = "unknown";

	if (tot == 0) {
		printf("lb_alloc: size too big (%u); alloc failed; file %s; line %d\n",
		       (lbufsz + size), file, line);
		goto error;
	}

	if ((lb = MALLOC_ALIGN(OSH_NULL, tot, 0)) == NULL) {
		printf("lb_alloc: size (%u); alloc failed; file %s; line %d\n", (lbufsz + size),
		       file, line);
		goto error;
	}
#else
	if (tot == 0) {
		LBUF_MSG(("lb_alloc: size too big (%u); alloc failed;\n",
		       (lbufsz + size)));
		goto error;
	}

	if ((lb = MALLOC_ALIGN(OSH_NULL, tot, 0)) == NULL) {
		LBUF_MSG(("lb_alloc: size (%u); alloc failed;\n", (lbufsz + size)));
		goto error;
	}
#endif /* BCMDBG_MEM || BCMDBG_MEMFAIL */

#ifdef HND_PT_GIANT
	success:
#endif
	ASSERT(ISALIGNED(lb, sizeof(int)));

	bzero(lb, lbufsz);

	head = (uchar*)((uchar*)lb + lbufsz);
	end_off = (tot - lbufsz);

	lb->data = (head + end_off) - ROUNDUP(size, sizeof(int));

#if defined(HNDLBUFCOMPACT)
	ASSERT((end_off > 0) && (end_off <= LB_BUFSIZE_MAX));
	ASSERT(((uintptr)head & LB_HEADHI_MASK) == ((uintptr)(lb->data) & LB_HEADHI_MASK));
	ASSERT(((uintptr)head & LB_HEADHI_MASK) == ((uintptr)(head + end_off) & LB_HEADHI_MASK));

	lb->head_off = ((uintptr)head) & LB_HEADLO_MASK;
	lb->end_off = (end_off - 1);
#else  /* ! HNDLBUFCOMPACT */
	lb->head = head;
	lb->end = head + end_off;
#endif /* ! HNDLBUFCOMPACT */

	lb->len = size;
	lb->mem.pktid = hnd_pktid_allocate(lb); /* BCMPKTIDMAP: pktid != 0 */
	ASSERT(lb->mem.pktid != PKT_INVALID_ID);
	lb->mem.refcnt = 1;
	lb->flags = flags;

	return (lb);

error:
	printf("lb_alloc: req_size: %d hnd_pktid_free_cnt: %d OSL_MEM_AVAIL: %d \n",
		size, hnd_pktid_free_cnt(), OSL_MEM_AVAIL());
	lbuf_allocfail++;
	return NULL;

}

static void
lb_free_one(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));
	ASSERT(hnd_pktid_sane(lb));

	if (!lb_isclone(lb)) {
		ASSERT(lb->mem.refcnt > 0);
		if (--lb->mem.refcnt == 0) {
			if (lb_pool(lb)) {
				pktpool_t *pool = lb_getpool(lb);

				ASSERT(pool);
				ASSERT(lb->mem.poolid == POOLID(pool));
				lb_resetpool(lb, pool->plen);
				pktpool_free(pool, lb); /* put back to pool */
			} else	if (!lb_isptblk(lb)) {
				lb->data = (uchar*) 0xdeadbeef;
				hnd_pktid_release(lb, lb->mem.pktid); /* BCMPKTIDMAP */
				MFREE(OSH_NULL, lb, LB_END(lb) - LB_HEAD(lb));
			} else {
#ifdef HND_PT_GIANT
				hnd_pktid_release(lb, lb->mem.pktid); /* BCMPKTIDMAP */
				hnd_free_pt(lb);
#else
				ASSERT(0);
#endif
			}
		}
	} else {
		struct lbuf *orig = ((struct lbuf_clone*)lb)->orig;

		ASSERT(!lb_pool(lb)); /* clone is not from pool */

		/* clones do not get refs, just originals */
		ASSERT(lb->mem.refcnt == 0);
		lb->data = (uchar*) 0xdeadbeef;
		hnd_pktid_release(lb, lb->mem.pktid); /* BCMPKTIDMAP */
		MFREE(OSH_NULL, lb, sizeof(struct lbuf_clone));

		lb_free_one(orig);
	}
}

#ifdef BCMPCIEDEV
static void
hnd_lbuf_free_cb(struct lbuf *lb)
{
	if (lbuf_free_cb != NULL)
		(lbuf_free_cb)(lb);
}
#endif

void
lb_free(struct lbuf *lb)
{
	struct lbuf *next;
	bool snarf;

	while (lb) {
		ASSERT(PKTLINK(lb) == NULL);

		next = PKTNEXT(OSH_NULL, lb);

		if (BCMLFRAG_ENAB()) {

			lbuf_freeinfo_t *lbuf_infoptr;

			lbuf_infoptr = lbuf_info_get();
			/* if a tx frag or rx frag , go to callback functions before free up */
			if (lbuf_infoptr->cb) {
				PKTSETNEXT(OSH_NULL, lb, NULL);
				snarf = lbuf_infoptr->cb(lbuf_infoptr->arg, lb);
				if (snarf) {
					lb = next;
					continue;
				}
			}
		}
#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB()) {
		hnd_lbuf_free_cb(lb);
	}
#endif
		lb_free_one(lb);
		lb = next;
	}
}

struct lbuf *
lb_dup(struct lbuf *lb)
{
	struct lbuf *lb_new;

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	if (!(lb_new = lb_alloc(lb->len, lbuf_basic, __FILE__, __LINE__)))
#else
	if (!(lb_new = lb_alloc(lb->len, lbuf_basic)))
#endif
		return (NULL);

	bcopy(lb->data, lb_new->data, lb->len);

	return (lb_new);
}


struct lbuf *
lb_shrink(struct lbuf *lb)
{
	ASSERT(PKTLINK(lb) == NULL);

	if (lb->len < LB_SHRINK_THRESHOLD) {

		struct lbuf *lb_new;
		uchar * head = lb_head(lb);

		/* original packet headroom */
		uint headroom = (uintptr)lb->data - (uintptr)head;

		/* preserve the alignment of the old data */
		headroom += (uintptr)lb->data & 0x3;

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
		lb_new = lb_alloc(lb->len + headroom, lbuf_basic, __FILE__, __LINE__);
#else
		lb_new = lb_alloc(lb->len + headroom, lbuf_basic);
#endif
		if (lb_new != NULL) {
			lb_pull(lb_new, headroom);
			bcopy(lb->data, lb_new->data, lb->len);
			lb_new->flags = lb->flags & ~(LBF_CLONE);
			lb_free(lb);
			lb = lb_new;
		}
	}
	return lb;
}


struct lbuf *
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
lb_clone(struct lbuf *lb, int offset, int len, const char *file, int line)
#else
lb_clone(struct lbuf *lb, int offset, int len)
#endif
{
	struct lbuf *clone;
	struct lbuf *orig;

	ASSERT(offset >= 0 && len >= 0);

	if (hnd_pktid_free_cnt() == 0) {
		LBUF_MSG(("lb_clone: hnd_pktid_free_cnt 0\n"));
		hnd_pktid_inc_fail_cnt();
		return (NULL);
	}

	if (offset < 0 || len < 0)
	        return (NULL);

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	if (!file)
		file = "unknown";

	clone = MALLOC_ALIGN(OSH_NULL, sizeof(struct lbuf_clone), 0);

	if (clone == NULL) {
		printf("lb_clone: size (%u); alloc failed; file %s; line %d\n",
		       LBUFSZ, file, line);
		return (NULL);
	}
#else
	if ((clone = MALLOC_ALIGN(OSH_NULL, sizeof(struct lbuf_clone), 0)) == NULL) {
		LBUF_MSG(("lb_clone: size (%u); alloc failed;\n", LBUFSZ));
		return (NULL);
	}
#endif /* BCMDBG_MEM || BCMDBG_MEMFAIL */

	/* find the actual non-clone original lbuf */
	if (lb_isclone(lb))
		orig = ((struct lbuf_clone*)lb)->orig;
	else
		orig = lb;

	ASSERT(orig->mem.refcnt < 255);
	orig->mem.refcnt++;

	ASSERT(ISALIGNED(clone, sizeof(int)));
	bzero(clone, sizeof(struct lbuf_clone));

	/* clone's data extent is a subset of the current data of lb */
	ASSERT(offset + len <= lb->len);

	clone->data = lb->data + offset;

#if defined(HNDLBUFCOMPACT)
	clone->head_off = ((uintptr)(clone->data) & LB_HEADLO_MASK);
	clone->end_off = len;
	ASSERT(((uintptr)(clone->data + len) & LB_HEADHI_MASK) ==
	       ((uintptr)(clone->data) & LB_HEADHI_MASK));
#else  /* ! HNDLBUFCOMPACT */
	clone->head = clone->data;
	clone->end = clone->head + len;
#endif /* ! HNDLBUFCOMPACT */

	clone->len = len;
	clone->flags |= LBF_CLONE;
	clone->mem.pktid = hnd_pktid_allocate(clone); /* BCMPKTIDMAP: id!=0 */
	ASSERT(clone->mem.pktid != PKT_INVALID_ID);

	((struct lbuf_clone*)clone)->orig = orig;

	return (clone);
}

/*
 * lb_sane() is invoked in various lb_xyz() functions. In the implementation of
 * lb_sane(), please do not include any calls to lb_xyz() to avoid stack
 * overflow from recursive calls.
 */
bool
lb_sane(struct lbuf *lb)
{
	int insane = 0;

#if defined(BCMPKTIDMAP)
	if ((lb->mem.pktid == PKT_NULL_ID) || (lb->mem.pktid > PKT_MAXIMUM_ID)) {
		insane |= 1;
	}
#endif /*   BCMPKTIDMAP */

	if (lb->mem.poolid != PKTPOOL_INVALID_ID) { /* From a pktpool */
		if (lb->mem.poolid > PKTPOOL_MAXIMUM_ID)
			insane |= 2;
		else
			insane |= (PKTPOOL_ID2PTR(lb->mem.poolid)->id != lb->mem.poolid) * 4;
	}

	insane |= (lb->data < LB_HEAD(lb)) * 8;
	insane |= (lb->data + lb->len > LB_END(lb)) * 16;

	if (insane) {
		LBUF_MSG(("lb_sane: insane 0x%08x\n"
		          "lbuf %p data %p head %p end %p len %d flags %d"
		          "pktid %u poolid %u\n", insane,
		          lb, lb->data, LB_HEAD(lb), LB_END(lb), lb->len, lb->flags,
		          lb->mem.pktid, lb->mem.poolid));
	}

	return (!insane);
}

void
lb_audit(struct lbuf * lb)
{
	ASSERT(lb != NULL);

	if (lb_is_frag(lb)) {
		do {
			uint16 fragix, flen = 0;

			fragix = LBFP(lb)->flist.finfo[LB_FRAG_CTX].ctx.fnum;
			ASSERT((fragix >= 1) && (fragix <= LB_FRAG_MAX));
			while (fragix > 0) {
				ASSERT(LBFP(lb)->flist.finfo[fragix].frag.data_lo != 0);
				ASSERT(LBFP(lb)->flist.flen[fragix] > 0);
				flen += LBFP(lb)->flist.flen[fragix];
				fragix--;
			}
			ASSERT(flen == LBFP(lb)->flist.flen[LB_FRAG_CTX]);
			lb = PKTNEXT(OSH_NULL, lb);
			if (lb)
				ASSERT((lb->data == NULL) && (lb->len == 0));	/* disallow? */
		} while (lb != NULL);
	} else 	{
		do {
			ASSERT(lb->data != NULL);
			ASSERT(lb_sane(lb) == 0);
			lb = PKTNEXT(OSH_NULL, lb);
		} while (lb != NULL);
	}
}

/*
 * reset lbuf before recycling to pool
 *      PRESERVE                        FIX-UP
 *      mem.pktid, mem.poolid           mem.refcnt, mem.poolstate
 *      offsets | head, end
 *                                      data
 *                                      len, flags
 *                                      reset all other fields
 */
void
lb_resetpool(struct lbuf *lb, uint16 len)
{
#ifdef BCMPCIEDEV
	uint16 rxcpl_id = 0;
	if (BCMPCIEDEV_ENAB()) {
		rxcpl_id = lb->rxcpl_id;
	}
#endif

	ASSERT(lb_sane(lb));
	ASSERT(lb_pool(lb));

	lb->mem.refcnt = 1;
#ifdef BCMDBG_POOL
	lb_setpoolstate(lb, POOL_IDLE); /* mem.poolstate */
#endif

	lb->data = lb_end(lb) - ROUNDUP(len, sizeof(int));
	lb->len = len;
	lb->flags = lb->flags & (LBF_TX_FRAG | LBF_PTBLK | LBF_RX_FRAG);

	bzero(&lb->reset, LBUFSZ - OFFSETOF(struct lbuf, reset));
#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB()) {
		lb->rxcpl_id = rxcpl_id;
	}
#endif
}

void
lbuf_free_cb_set(lbuf_free_global_cb_t cb)
{
#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB()) {
		lbuf_free_cb = cb;
	}
#endif
}

#ifdef BCMDBG
void
lb_dump(void)
{
	uint i;
	LBUF_MSG(("allocfail %d\n", lbuf_allocfail));
	for (i = 0; i < LBUF_BIN_SIZE; i++) {
		LBUF_MSG(("bin[%d] %d ", i, lbuf_hist[i]));
	}
	LBUF_MSG(("\n"));
}
#endif	/* BCMDBG */
