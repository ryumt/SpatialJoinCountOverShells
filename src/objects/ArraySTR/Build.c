#define ARRAYSTR_FAMILY__
#define ARRAYSTR_BUILD_SOURCE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

#include "atomic.h"
#include "support_functions.h"
#include "objects/ArraySTR/Build.h"


#define prefetch32(srcp, level) _mm_prefetch(srcp, level)
#define prefetch64(srcp, level) do { prefetch32(srcp, level); prefetch32((char *)srcp + 32, level); } while (0)
#define stream_store32_cast(dstp, val) _mm_stream_si32((int *)dstp, *(int *)&val)
#define stream_store64_cast(dstp, val) _mm_stream_si64((long long *)dstp, *(long long *)&val)
#define stream_store32(dstp, val) _mm_stream_si32((int *)dstp, (int)val)
#define stream_store64(dstp, val) _mm_stream_si64((long long *)dstp, (long long)val)


static const int SHIFT_SIZE = sizeof(float) * CHAR_BIT - 1;
static const uint32_t SIGN = 0x80000000; /* (0x01 << SHIFT_SIZE); */


#undef __BEGIN_RADIXSORT_SETUP__
typedef struct LocalBuffer {
	/* 4 * 256 = 1 KB */
        int tails[NUM_OF_RADIX];
	/* 16 * 256 * B_BUFFER_SIZE = 4*B_BUFFER_SIZE KB */
	ulong_t key[NUM_OF_RADIX][B_BUFFER_SIZE];
	float val[NUM_OF_RADIX][B_BUFFER_SIZE];
} LocalBuffer;
/* + per thread bin: 8 * 256 = 2 KB */
/*   -> B_BUFFER_SIZE <= 63 (ex. L2:256KB -> opt. B_BUFFER_SIZE=32) */

/* used to manipulate bits of float value */
typedef union FloatUint {
	float fval;
	uint32_t uval;
	int32_t ival;
} FloatUint;


/* get p'th digit for radix sort */
static inline
int
GetDigit(float v, int p)
{
        FloatUint fu = {v};
        return (fu.uval >> p * D_RADIX_SIZE) & 0x000000ff;
}

/* create histgram: count of each digit, to use in radix sort */
static inline
void
CreateHistogram(ulong_t *hist, float *val, ulong_t width, int place)
{
        ulong_t idx;
	
	bzero(hist, sizeof(*hist) * NUM_OF_RADIX);
        for (idx = 0; idx < width; idx++)
		hist[GetDigit(val[idx], place)]++;
}

/* calculate offset of each digit */
static inline
void
PrefixSum(ulong_t *offset, ulong_t *hist)
{
        ulong_t woff;
        int i;

        woff = 0;
        for (i = 0; i < NUM_OF_RADIX; i++) {
                ulong_t tmp = hist[i];
                offset[i] = woff;
                woff += tmp;
        }
}

/* setup values to sort */
static inline 
void 
SetupValues(KeyValue *keyval, ArraySTRNode *nodes, int axis, ulong_t beg, ulong_t end)
{
	ulong_t idx;
	for (idx = beg; idx < end; idx++) {
		keyval->val[idx] = (nodes[keyval->key[idx]].mbr.low[axis]
				    + nodes[keyval->key[idx]].mbr.upp[axis]) / 2;
	}
}

/* setup sequenced index and values to sort */
static inline 
void 
SetupKeyValues(KeyValue *keyval, ArraySTRNode *nodes, int axis, ulong_t beg, ulong_t end)
{
	ulong_t idx;
	
	for (idx = beg; idx < end; idx++) {
		keyval->key[idx] = idx;
		keyval->val[idx] = (nodes[idx].mbr.low[axis]
				    + nodes[idx].mbr.upp[axis]) / 2;
	}
}

/* setup backup array for radix sort (radix sort needs copy of original data) */
static inline 
void 
SetupBackupArray(ArraySTRNode *backup, ArraySTRNode *nodes, ulong_t beg, ulong_t end)
{
	memcpy(backup, nodes, sizeof(*nodes) * (end - beg));
}

/* sort node array by sorted index and backup array */
static inline 
void 
ArrangeNodeArray(ArraySTRNode *nodes, ArraySTRNode *backup, KeyValue *keyval, ulong_t beg, ulong_t end)
{
	ulong_t idx;
	
	for (idx = beg; idx < end; idx++)
		nodes[idx] = backup[keyval->key[idx]];
}

/* to radix sort float value (positive & negative), we should encode value (positive => negate, negative => invert) */
static inline
void
RadixSortEncodeValues(KeyValue *keyval, ulong_t beg, ulong_t end)
{
        ulong_t idx;
	
        for (idx = beg; idx < end; idx++) {
		FloatUint fu = {keyval->val[idx]};
		
		/* arithmetic shift */
		fu.uval ^= (fu.ival >> SHIFT_SIZE) | SIGN;
		keyval->val[idx] = fu.fval;
	}
}

/* restore encoded value (positive => invert, negative => negate) */
/*  but this process is not needed in STR-Tree construction */
/*  because sorted float value is just copy and will be descarded. */
static inline
void
RadixSortDecodeValues(KeyValue *keyval, ulong_t beg, ulong_t end)
{
	ulong_t idx;
	
        for (idx = beg; idx < end; idx++) {
		FloatUint fu = {keyval->val[idx]};
		
		fu.uval ^= (~fu.ival >> SHIFT_SIZE) | SIGN;
		keyval->val[idx] = fu.fval;
	}
}
#undef __END_RADIXSORT_SETUP__


/* single threaded version of LSD radix sort */
static inline 
void 
RadixSortWithKey(KeyValue *keyval, KeyValue *tmp_keyval, ulong_t offset, ulong_t ndata)
{
	ulong_t hist[NUM_OF_RADIX];
	int place;
	
	for (place = 0; place < sizeof(*keyval->val); place++) {
		LocalBuffer lb;
		ulong_t idx;
		int i;
		
		CreateHistogram(hist, keyval->val + offset, ndata, place);
		PrefixSum(hist, hist);
		
		bzero(lb.tails, sizeof(lb.tails));
		for (idx = offset; idx < offset + ndata; idx++) {
			int digit = GetDigit(keyval->val[idx], place);
			lb.val[digit][lb.tails[digit]] = keyval->val[idx];
			lb.key[digit][lb.tails[digit]] = keyval->key[idx];
			lb.tails[digit]++;
			if (lb.tails[digit] == B_BUFFER_SIZE) {
				memcpy(&tmp_keyval->val[offset + hist[digit]], lb.val[digit], sizeof(*tmp_keyval->val) * B_BUFFER_SIZE);
				memcpy(&tmp_keyval->key[offset + hist[digit]], lb.key[digit], sizeof(*tmp_keyval->key) * B_BUFFER_SIZE);
				lb.tails[digit] = 0;
				hist[digit] += B_BUFFER_SIZE;
			}
		}
		for (i = 0; i < NUM_OF_RADIX; i++) {
			if (lb.tails[i] > 0) {
				memcpy(&tmp_keyval->val[offset + hist[i]], lb.val[i], sizeof(*tmp_keyval->val) * lb.tails[i]);
				memcpy(&tmp_keyval->key[offset + hist[i]], lb.key[i], sizeof(*tmp_keyval->key) * lb.tails[i]);
			}
		}
		SWAP_MACRO(KeyValue *, &keyval, &tmp_keyval);
	}
}

static inline 
void 
PartialRadixSortWithKey(KeyValue *keyval, KeyValue *tmp_keyval,
			ulong_t ndata, ulong_t slice, ulong_t width)
{
	ulong_t bound;
	ulong_t offset;
	ulong_t fixwidth;
	
	fixwidth = width;
	bound = (slice > ndata) ? ndata : slice;
	for (offset = 0; offset < ndata - 1; offset += fixwidth) {
		while (offset >= bound) {
                        bound += slice;
                        if (bound > ndata)
				bound = ndata;
		}
                if (offset + fixwidth > bound) {
                        fixwidth = bound - offset;
                        bound += slice;
                        if (bound > ndata)
				bound = ndata;
                }
		RadixSortWithKey(keyval, tmp_keyval, offset, fixwidth);
	}
}

static inline 
void 
PackNodes(ArraySTRNode *nodes, ulong_t first, ulong_t last, ulong_t width, ulong_t *parentp)
{
	ulong_t ndata = last - first;
	ulong_t offset;
	
	for (offset = 0; offset < ndata; offset += width) {
		ulong_t parent;
		ulong_t fixwidth = width;
		
		if (offset + fixwidth > ndata)
			fixwidth = ndata - offset;
		parent = fetch_and_add(parentp, -1);
		nodes[parent].pos = first + offset;
		nodes[parent].len = fixwidth;
		adjustMbrArraySTRNode(nodes, parent);
	}
}


/*
 * multi-threaded LSD radix sort 
 */
#undef __BEGIN_PARALLEL_RADIXSORT__

#define STOP (0)
#define START (1)

typedef struct ParallelSetupKVContext {
	KeyValue *keyval;
	ArraySTRNode *nodes;
	int axis;
	ulong_t ndata;
	
	SameThreadPool *stp;
	ulong_t *nprocp;
	
	ArraySTRNode *backup;
} ParallelSetupKVContext;

typedef struct ParallelRadixSortWKContext {
	KeyValue *keyval;
	KeyValue *tmp_keyval;
	ulong_t **bins;
	ulong_t ndata;
	
	SameThreadPool *stp;
	int *idp;
	int *barp;
	int *flagp;
} ParallelRadixSortWKContext;


/* multi-threaded version of radix sort setup funcions */
static 
void 
ParallelSetupValues(void *arg)
{
	ParallelSetupKVContext *psc = arg;
	ulong_t width = UlongCeil(psc->ndata,  psc->stp->nworkers);
	ulong_t beg, end;
	
	for (;;) {
		beg = fetch_and_add(psc->nprocp, width);
		if (beg >= psc->ndata)
			break;
		end = beg + width;
		end = (end > psc->ndata) ? psc->ndata : end;
		SetupValues(psc->keyval, psc->nodes, psc->axis, beg, end);
	}
}

static 
void 
ParallelSetupKeyValues(void *arg)
{
	ParallelSetupKVContext *psc = arg;
	ulong_t width = UlongCeil(psc->ndata,  psc->stp->nworkers);
	ulong_t beg, end;
	
	for (;;) {
		beg = fetch_and_add(psc->nprocp, width);
		if (beg >= psc->ndata)
			break;
		end = beg + width;
		end = (end > psc->ndata) ? psc->ndata : end;
		SetupKeyValues(psc->keyval, psc->nodes, psc->axis, beg, end);
	}
}

static 
void 
ParallelSetupBackupArray(void *arg)
{
	ParallelSetupKVContext *psc = arg;
	ulong_t width = UlongCeil(psc->ndata,  psc->stp->nworkers);
	ulong_t beg, end;
	
	for (;;) {
		beg = fetch_and_add(psc->nprocp, width);
		if (beg >= psc->ndata)
			break;
		end = beg + width;
		end = (end > psc->ndata) ? psc->ndata : end;
		SetupBackupArray(psc->backup + beg, psc->nodes + beg, beg, end);
	}
}

static 
void 
ParallelArrangeNodeArray(void *arg)
{
	ParallelSetupKVContext *psc = arg;
	ulong_t width = UlongCeil(psc->ndata,  psc->stp->nworkers);
	ulong_t beg, end;
	
	for (;;) {
		beg = fetch_and_add(psc->nprocp, width);
		if (beg >= psc->ndata)
			break;
		end = beg + width;
		end = (end > psc->ndata) ? psc->ndata : end;
		ArrangeNodeArray(psc->nodes, psc->backup, psc->keyval, beg, end);
	}
}

static 
void 
ParallelEncodeValues(void *arg)
{
	ParallelSetupKVContext *psc = arg;
	ulong_t width = UlongCeil(psc->ndata,  psc->stp->nworkers);
	ulong_t beg, end;
	
	for (;;) {
		beg = fetch_and_add(psc->nprocp, width);
		if (beg >= psc->ndata)
			break;
		end = beg + width;
		end = (end > psc->ndata) ? psc->ndata : end;
		RadixSortEncodeValues(psc->keyval, beg, end);
	}
}

static 
void 
ParallelDecodeValues(void *arg)
{
	ParallelSetupKVContext *psc = arg;
	ulong_t width = UlongCeil(psc->ndata,  psc->stp->nworkers);
	ulong_t beg, end;
	
	for (;;) {
		beg = fetch_and_add(psc->nprocp, width);
		if (beg >= psc->ndata)
			break;
		end = beg + width;
		end = (end > psc->ndata) ? psc->ndata : end;
		RadixSortDecodeValues(psc->keyval, beg, end);
	}
}

static inline 
void 
ParallelRadixSortSetup(SameThreadPool *stp, void (*func)(void *), 
		       KeyValue *keyval, ArraySTRNode *nodes, int axis, ulong_t ndata, ArraySTRNode *backup)
{
	ulong_t nproc = 0;
	ParallelSetupKVContext psc = {
		keyval, nodes, axis, ndata, 
		stp, &nproc, backup
	};
	requestWorkSameThreadPool(stp, func, &psc);
	waitCompletionSameThreadPool(stp);
}

/* parallel LSD radix sort routine */
static 
void 
ParallelRadixSortWithKeyLSD(void *arg)
{
	ParallelRadixSortWKContext *prsc = arg;
	int id = fetch_and_add(prsc->idp, 1);
	KeyValue *keyval = prsc->keyval;
	KeyValue *tmp_keyval = prsc->tmp_keyval;
	ulong_t *bin = prsc->bins[id];
	
	ulong_t hist[NUM_OF_RADIX];
	ulong_t width = UlongCeil(prsc->ndata, prsc->stp->nworkers);
	ulong_t beg, end;
	int place;
	
	/* get processing range */
	beg = width * id;
	end = beg + width;
	if (end > prsc->ndata)
		end = prsc->ndata;
	
	/* if processing range for this thread is out of range, no need to sort */
	/* if (beg >= prsc->ndata)  */
	/* return ; */
	
	/* LSD radix sort main */
	for (place = 0; place < sizeof(*keyval->val); place++) {
		LocalBuffer lb;
		ulong_t idx;
		int i, j;
		
		CreateHistogram(bin, keyval->val + beg, end - beg, place);
		
		/* barrier, wait until all threads create their histogram */
		fetch_and_add(prsc->barp, 1);
		wait_while(*prsc->flagp == START);
		
		/* calculate global memory write offset */
		bzero(hist, sizeof(*hist) * NUM_OF_RADIX);
		for (j = 1; j < NUM_OF_RADIX; j++) {
			hist[j] = hist[j - 1];
			for (i = 0; i < prsc->stp->nworkers; i++)
				hist[j] += prsc->bins[i][j - 1];
		}
		for (i = 0; i < id; i++) {
			for (j = 0; j < NUM_OF_RADIX; j++)
				hist[j] += prsc->bins[i][j];
		}
		
		/* sort data  */
		bzero(lb.tails, sizeof(lb.tails));
		for (idx = beg; idx < end; idx++) {
			int digit = GetDigit(keyval->val[idx], place);
			/* write to local buffer */
			lb.val[digit][lb.tails[digit]] = keyval->val[idx];
			lb.key[digit][lb.tails[digit]] = keyval->key[idx];
			lb.tails[digit]++;
			/* flush local buffer to memory */
			if (lb.tails[digit] == B_BUFFER_SIZE) {
				memcpy(&tmp_keyval->val[hist[digit]], lb.val[digit], sizeof(*tmp_keyval->val) * B_BUFFER_SIZE);
				memcpy(&tmp_keyval->key[hist[digit]], lb.key[digit], sizeof(*tmp_keyval->key) * B_BUFFER_SIZE);
				lb.tails[digit] = 0;
				hist[digit] += B_BUFFER_SIZE;
			}
		}
		/* flush in-buffer remaining entries */
		for (i = 0; i < NUM_OF_RADIX; i++) {
			if (lb.tails[i] > 0) {
				memcpy(&tmp_keyval->val[hist[i]], lb.val[i], sizeof(*tmp_keyval->val) * lb.tails[i]);
				memcpy(&tmp_keyval->key[hist[i]], lb.key[i], sizeof(*tmp_keyval->key) * lb.tails[i]);
			}
		}
		
		/* swap src, dst buffer */
		SWAP_MACRO(KeyValue *, &keyval, &tmp_keyval);
		
		/* barrier, wait until all threads end this iteration */
		fetch_and_add(prsc->barp, 1);
		wait_while(*prsc->flagp == STOP);
	}
}


static inline
void
ParallelRadixSortWithKey(SameThreadPool *stp, KeyValue *keyval, KeyValue *tmp_keyval, 
			 ulong_t *bins[], ulong_t ndata)
{
	int total_nworkers;
        ulong_t width;
	
	int id, bar, flag;
	int place;
	ParallelRadixSortWKContext prsc = {
		keyval, tmp_keyval, 
		bins, ndata, 
		stp, &id, &bar, &flag
	};
	
	/* initialize id ticket, barrier variable, sync flag */
	id = bar = 0;
	flag = START;
	
	/* backup total number of workers */
	total_nworkers = stp->nworkers;
	/* if number of workers exceeds number of data, decrease number of workers */
	width = UlongCeil(ndata, total_nworkers);
        if (width * total_nworkers > ndata)
		stp->nworkers = UlongCeil(ndata, width);
	
	/* sort data by LSD in parallel */
	requestWorkSameThreadPool(stp, ParallelRadixSortWithKeyLSD, &prsc);
	/* barrier */
	for (place = 0; place < sizeof(*keyval->val); place++) {
		/* wait creation of histgrams */
		wait_while(bar < stp->nworkers);
		bar = 0;
		fetch_and_add(&flag, -START);
		/* wait sorting place'th digit */
		wait_while(bar < stp->nworkers);
		bar = 0;
		fetch_and_add(&flag, START);
	}
	waitCompletionSameThreadPool(stp);
	/* restore total number of workers */
	stp->nworkers = total_nworkers;
}
#undef START
#undef STOP
#undef __END_PARALLEL_RADIXSORT__


#undef __BEGIN_PARALLEL_PARTIAL_RADIXSORT__
typedef struct ParallelPartialRadixSortContext {
	KeyValue *keyval;
	KeyValue *tmp_keyval;
	ulong_t ndata;
	
	SameThreadPool *stp;
	int *idp;
	ulong_t *nprocp;
	
	ulong_t slice;
	ulong_t width;
	
	pthread_mutex_t *nplockp;
} ParallelPartialRadixSortContext;

/* each thread sorts vertical slice by radix sort. */
static 
void 
ParallelPartialRadixSortWithKeyWorker(void *arg)
{
	ParallelPartialRadixSortContext *pprsc = arg;
	// int id = fetch_and_add(pprsc->idp, 1);
	ulong_t bound;
	// int ncoll;
	
	bound = (pprsc->slice > pprsc->ndata) ? pprsc->ndata : pprsc->slice;
	// ncoll = 0;
	for (;;) {
		ulong_t width = pprsc->width;
		ulong_t offset = *pprsc->nprocp;
		// int i;
		
		pthread_mutex_lock(pprsc->nplockp);
                offset = *pprsc->nprocp;
                if (offset >= pprsc->ndata - 1) {
                        pthread_mutex_unlock(pprsc->nplockp);
                        break;
                }
                while (offset >= bound) {
                        bound += pprsc->slice;
                        if (bound > pprsc->ndata)
				bound = pprsc->ndata;
		}
                if (offset + width > bound) {
                        width = bound - offset;
                        bound += pprsc->slice;
                        if (bound > pprsc->ndata)
				bound = pprsc->ndata;
                }
                *pprsc->nprocp += width;
		pthread_mutex_unlock(pprsc->nplockp);
		RadixSortWithKey(pprsc->keyval, pprsc->tmp_keyval, offset, width);
		
/* 		if (offset >= pprsc->ndata - 1) */
/* 			break; */
		
/* 		while (offset >= bound) { */
/* 			bound += pprsc->slice; */
/* 			if (bound > pprsc->ndata) */
/* 				bound = pprsc->ndata; */
/* 		} */
/* 		if (offset + width > bound) { */
/* 			width = bound - offset; */
/* 			bound += pprsc->slice; */
/* 			if (bound > pprsc->ndata) */
/* 				bound = pprsc->ndata; */
/* 		} */
		
/* 		if (bool_old_compare_and_swap(pprsc->nprocp, offset, offset + width)) { */
/* 			RadixSortWithKey(pprsc->keyval, pprsc->tmp_keyval, offset, width); */
/* 			ncoll = 0; */
/* 			continue; */
/* 		} */
		
/* 		/\* back-off *\/ */
/* 		ncoll++; */
/* 		for (i = (int)exp2(ncoll); i >= 0; i--) */
/* 			; */
	}
}

static inline 
void 
ParallelPartialRadixSortWithKey(SameThreadPool *stp, KeyValue *keyval, KeyValue *tmp_keyval,
				ulong_t ndata, ulong_t slice, ulong_t width)
{
	int id;
	ulong_t nproc;
	pthread_mutex_t nplock;
	ParallelPartialRadixSortContext pprsc = {
		keyval, tmp_keyval, ndata, 
		stp, &id, &nproc, 
		slice, width, &nplock
	};
	
	id = nproc = 0;
	pthread_mutex_init(&nplock, NULL);
	requestWorkSameThreadPool(stp, ParallelPartialRadixSortWithKeyWorker, &pprsc);
	waitCompletionSameThreadPool(stp);
	pthread_mutex_destroy(&nplock);
}
#undef __END_PARALLEL_PARTIAL_RADIXSORT__



#undef __BEGIN_PARALLEL_PACK__
typedef struct ParallelPackContext {
	ArraySTRNode *nodes;
	ulong_t first;
	ulong_t last;
	ulong_t width;
	
	SameThreadPool *stp;
	ulong_t *parentp;
	ulong_t *nprocp;
} ParallelPackContext;

/* node packing routine. pack MAX_ENT nodes and create their parent node. */
static 
void 
ParallelPackNodesWorker(void *arg)
{
	ParallelPackContext *ppc = arg;
	ulong_t ndata = ppc->last - ppc->first;
	
	for (;;) {
		ulong_t offset;
		ulong_t parent;
		ulong_t width;
		
		offset = fetch_and_add(ppc->nprocp, ppc->width);
		if (offset >= ndata)
			break;
		
		width = ppc->width;
		if (offset + width > ndata)
			width = ndata - offset;
		parent = fetch_and_add(ppc->parentp, -1);
		ppc->nodes[parent].pos = ppc->first + offset;
		ppc->nodes[parent].len = width;
		adjustMbrArraySTRNode(ppc->nodes, parent);
	}
}

static inline 
void 
ParallelPackNodes(SameThreadPool *stp, ArraySTRNode *nodes, ulong_t first, ulong_t last, ulong_t width, ulong_t *parentp)
{
	ulong_t nproc;
	ParallelPackContext ppc = {
		nodes, first, last, width, 
		stp, parentp, &nproc	    
	};
	
	nproc = 0;
	requestWorkSameThreadPool(stp, ParallelPackNodesWorker, &ppc);
	waitCompletionSameThreadPool(stp);
}
#undef __END_PARALLEL_PACK__


inline
void 
InsertDataArraySTR(ArraySTR *art, ulong_t id, float p[])
{
	ulong_t inode = --art->head;
	ArraySTRNode *node = &art->nodes[inode];
	
	node->id = id;
	/* create mbr for this leaf node */
	bzero(&node->mbr, sizeof(node->mbr));
	memcpy(node->mbr.low, p, sizeof(*p) * NDIMS);
	memcpy(node->mbr.upp, p, sizeof(*p) * NDIMS);
}

inline 
void 
MakeTreeArraySTR(ArraySTR *art, int np)
{
	ulong_t ndata = art->ni4leaf.nums;
	ulong_t first = art->head--;
	ulong_t last = first + ndata;
		
	/* for radix sort */
	KeyValue keyval;
	KeyValue tmp_keyval;
	ArraySTRNode *backup;
	ulong_t **bins;
	SameThreadPool *stp;
	int i;
	
	if (ndata < 1) {
		fprintf(stderr, "%s(): No data loaded. Cannot make tree.\n", __func__);
		return ;
	}
	
	/* values to sort */
	keyval.val = (void *)malloc(sizeof(*keyval.val) * ndata);
	/* index, will be sorted togather with values */
	keyval.key = (void *)malloc(sizeof(*keyval.key) * ndata);
	tmp_keyval.val = (void *)malloc(sizeof(*keyval.val) * ndata);
	tmp_keyval.key = (void *)malloc(sizeof(*keyval.key) * ndata);
	/* to radix-sort 'structure', we have to reserve copy of original data. sort them by using sorted index later. */
	backup = (void *)malloc(sizeof(*backup) * ndata);
	/* bins need to be shared by parallel radix-sort routine, so alloc them here. */
	bins = (void *)malloc(sizeof(*bins) * np);
	for (i = 0; i < np; i++)
		bins[i] = (void *)malloc(sizeof(**bins) * NUM_OF_RADIX);
	
	/* construct thread pool */
	stp = (np > 1) ? constructSameThreadPool(np) : NULL;
	
	/* tree construction loop */
	art->ni4leaf.fidx = first;
	for (;;) {
		ulong_t next_last = first;
		ulong_t nruns = UlongCeil(ndata, MAX_ENT);
		ulong_t width[3];
		
		/* create root node */
		if (ndata <= MAX_ENT) {
			ulong_t iroot;
			ArraySTRNode *root;
			
			iroot = art->root = art->head;
			// printf("ArraySTR: index of the root=%zu\n", iroot);
			
			root = &art->nodes[iroot];
			root->pos = first;
			root->len = last - first;
			adjustMbrArraySTRNode(art->nodes, iroot);
			break;
		}
		
		width[0] = ndata;
		width[1] = (ulong_t)(ceil(pow(nruns, 2./3.))) * MAX_ENT;
		width[2] = (ulong_t)(ceil(pow(nruns, 1./2.))) * MAX_ENT;
		
		if (ndata < np || np == 1) {
			/* create backup of node array */
			SetupBackupArray(backup, &art->nodes[first], 0, ndata);
			/* sort on first-axis */
			SetupKeyValues(&keyval, backup, FIRST_AXIS, 0, ndata);
			RadixSortEncodeValues(&keyval, 0, ndata);
			RadixSortWithKey(&keyval, &tmp_keyval, 0, ndata);
			/* sort on second-axis */
			SetupValues(&keyval, backup, SECOND_AXIS, 0, ndata);
			RadixSortEncodeValues(&keyval, 0, ndata);
			PartialRadixSortWithKey(&keyval, &tmp_keyval, ndata, width[0], width[1]);
			/* sort on third-axis */
			SetupValues(&keyval, backup, THIRD_AXIS, 0, ndata);
			RadixSortEncodeValues(&keyval, 0, ndata);
			PartialRadixSortWithKey(&keyval, &tmp_keyval, ndata, width[1], width[2]);
			/* sort node array */
			ArrangeNodeArray(&art->nodes[first], backup, &keyval, 0, ndata);
			PackNodes(art->nodes, first, last, MAX_ENT, &art->head);
		}
		else {
			/* create backup of node array */
			ParallelRadixSortSetup(stp, ParallelSetupBackupArray, &keyval, &art->nodes[first], FIRST_AXIS, ndata, backup);
				
			/* sort on first-axis */
			ParallelRadixSortSetup(stp, ParallelSetupKeyValues, &keyval, backup, FIRST_AXIS, ndata, backup);
			ParallelRadixSortSetup(stp, ParallelEncodeValues, &keyval, backup, FIRST_AXIS, ndata, backup);
			ParallelRadixSortWithKey(stp, &keyval, &tmp_keyval, bins, ndata);
			/* float value decode is not needed */
			/* ParallelRadixSortSetup(stp, ParallelDecodeValues, &keyval, backup, FIRST_AXIS, ndata, backup); */
			
			/* sort on second-axis */
			ParallelRadixSortSetup(stp, ParallelSetupValues, &keyval, backup, SECOND_AXIS, ndata, backup);
			ParallelRadixSortSetup(stp, ParallelEncodeValues, &keyval, backup, SECOND_AXIS, ndata, backup);
			ParallelPartialRadixSortWithKey(stp, &keyval, &tmp_keyval, ndata, width[0], width[1]);
			
			/* sort on third-axis */
			ParallelRadixSortSetup(stp, ParallelSetupValues, &keyval, backup, THIRD_AXIS, ndata, backup);
			ParallelRadixSortSetup(stp, ParallelEncodeValues, &keyval, backup, THIRD_AXIS, ndata, backup);
			ParallelPartialRadixSortWithKey(stp, &keyval, &tmp_keyval, ndata, width[1], width[2]);
			
			/* sort node array */
			ParallelRadixSortSetup(stp, ParallelArrangeNodeArray, &keyval, &art->nodes[first], FIRST_AXIS, ndata, backup);
			/* multi-threaded packing */
			ParallelPackNodes(stp, art->nodes, first, last, MAX_ENT, &art->head);
		}
				
		first = art->head + 1;
		last = next_last;
		ndata = nruns;
	}
	free(keyval.val);
	free(keyval.key);
	free(tmp_keyval.val);
	free(tmp_keyval.key);
	free(backup);
	for (i = 0; i < np; i++)
		free(bins[i]);
	free(bins);
	
	if (stp != NULL)
		destructSameThreadPool(stp);
}



/*************          **************/
inline 
void 
MakeTreeWithSameThreadPoolArraySTR(ArraySTR *art, SameThreadPool *stp)
{
	ulong_t ndata = art->ni4leaf.nums;
	ulong_t first = art->head--;
	ulong_t last = first + ndata;
		
	/* for radix sort */
	KeyValue keyval;
	KeyValue tmp_keyval;
	ArraySTRNode *backup;
	ulong_t **bins;
	int i;
	
	if (ndata < 1) {
		fprintf(stderr, "%s(): No data loaded. Cannot make tree.\n", __func__);
		return ;
	}
	
	/* values to sort */
	keyval.val = (void *)malloc(sizeof(*keyval.val) * ndata);
	/* index, will be sorted togather with values */
	keyval.key = (void *)malloc(sizeof(*keyval.key) * ndata);
	tmp_keyval.val = (void *)malloc(sizeof(*keyval.val) * ndata);
	tmp_keyval.key = (void *)malloc(sizeof(*keyval.key) * ndata);
	/* to radix-sort 'structure', we have to reserve copy of original data. sort them by using sorted index later. */
	backup = (void *)malloc(sizeof(*backup) * ndata);
	/* bins need to be shared by parallel radix-sort routine, so alloc them here. */
	bins = (void *)malloc(sizeof(*bins) * stp->nworkers);
	for (i = 0; i < stp->nworkers; i++)
		bins[i] = (void *)malloc(sizeof(**bins) * NUM_OF_RADIX);
		
	/* tree construction loop */
	art->ni4leaf.fidx = first;
	for (;;) {
		ulong_t next_last = first;
		ulong_t nruns = UlongCeil(ndata, MAX_ENT);
		ulong_t width[3];
		
		/* create root node */
		if (ndata <= MAX_ENT) {
			ulong_t iroot;
			ArraySTRNode *root;
			
			iroot = art->root = art->head;
			root = &art->nodes[iroot];
			root->pos = first;
			root->len = last - first;
			adjustMbrArraySTRNode(art->nodes, iroot);
			break;
		}
		
		width[0] = ndata;
		width[1] = (ulong_t)(ceil(pow(nruns, 2./3.))) * MAX_ENT;
		width[2] = (ulong_t)(ceil(pow(nruns, 1./2.))) * MAX_ENT;
		
		if (ndata < stp->nworkers) {
			/* create backup of node array */
			SetupBackupArray(backup, &art->nodes[first], 0, ndata);
			
			/* sort on first-axis */
			SetupKeyValues(&keyval, backup, FIRST_AXIS, 0, ndata);
			RadixSortEncodeValues(&keyval, 0, ndata);
			RadixSortWithKey(&keyval, &tmp_keyval, 0, ndata);
			
			/* sort on second-axis */
			SetupValues(&keyval, backup, SECOND_AXIS, 0, ndata);
			RadixSortEncodeValues(&keyval, 0, ndata);
			ParallelPartialRadixSortWithKey(stp, &keyval, &tmp_keyval, ndata, width[0], width[1]);
			
			/* sort on third-axis */
			SetupValues(&keyval, backup, THIRD_AXIS, 0, ndata);
			RadixSortEncodeValues(&keyval, 0, ndata);
			ParallelPartialRadixSortWithKey(stp, &keyval, &tmp_keyval, ndata, width[1], width[2]);
			
			/* sort node array */
			ArrangeNodeArray(&art->nodes[first], backup, &keyval, 0, ndata);
		}
		else {
			/* create backup of node array */
			ParallelRadixSortSetup(stp, ParallelSetupBackupArray, &keyval, &art->nodes[first], FIRST_AXIS, ndata, backup);
				
			/* sort on first-axis */
			ParallelRadixSortSetup(stp, ParallelSetupKeyValues, &keyval, backup, FIRST_AXIS, ndata, backup);
			ParallelRadixSortSetup(stp, ParallelEncodeValues, &keyval, backup, FIRST_AXIS, ndata, backup);
			ParallelRadixSortWithKey(stp, &keyval, &tmp_keyval, bins, ndata);
						
			/* sort on second-axis */
			ParallelRadixSortSetup(stp, ParallelSetupValues, &keyval, backup, SECOND_AXIS, ndata, backup);
			ParallelRadixSortSetup(stp, ParallelEncodeValues, &keyval, backup, SECOND_AXIS, ndata, backup);
			ParallelPartialRadixSortWithKey(stp, &keyval, &tmp_keyval, ndata, width[0], width[1]);
			
			/* sort on third-axis */
			ParallelRadixSortSetup(stp, ParallelSetupValues, &keyval, backup, THIRD_AXIS, ndata, backup);
			ParallelRadixSortSetup(stp, ParallelEncodeValues, &keyval, backup, THIRD_AXIS, ndata, backup);
			ParallelPartialRadixSortWithKey(stp, &keyval, &tmp_keyval, ndata, width[1], width[2]);
			
			/* sort node array */
			ParallelRadixSortSetup(stp, ParallelArrangeNodeArray, &keyval, &art->nodes[first], FIRST_AXIS, ndata, backup);
		}
		
		/* multi-threaded packing */
		ParallelPackNodes(stp, art->nodes, first, last, MAX_ENT, &art->head);
		
		first = art->head + 1;
		last = next_last;
		ndata = nruns;
	}
		
	free(keyval.val);
	free(keyval.key);
	free(tmp_keyval.val);
	free(tmp_keyval.key);
	free(backup);
	for (i = 0; i < stp->nworkers; i++)
		free(bins[i]);
	free(bins);
}
/*************          **************/

