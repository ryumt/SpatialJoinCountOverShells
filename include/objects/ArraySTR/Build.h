#ifndef ARRAYSTR_BUILD_HEAD__
#define ARRAYSTR_BUILD_HEAD__

#include <stdio.h>
#include "types/ulong.h"
#include "objects/ArraySTR/ArraySTR.h"
#include "objects/SameThreadPool/SameThreadPool.h"


/*** private ***/
#ifdef ARRAYSTR_BUILD_SOURCE__
#define B_BUFFER_SIZE (32)
#define D_RADIX_SIZE (8)
#define NUM_OF_RADIX (0x01 << D_RADIX_SIZE)

/* 8 + 4 + (4) = 16 byte */
typedef struct KeyValue {
	ulong_t *key;
	float *val;
} KeyValue;

/* get p-th radix */
static int GetDigit(float v, int p);
static void CreateHistogram(ulong_t *hist, float *val, ulong_t width, int place);
static void PrefixSum(ulong_t *offset, ulong_t *hist);


#define __BEGIN_RADIXSORT_SETUP__ /* this definition is just a delimiter */
/* gather data to sort */ 
static void SetupValues(KeyValue *keyval, ArraySTRNode *nodes, int axis, ulong_t beg, ulong_t end);
static void SetupKeyValues(KeyValue *keyval, ArraySTRNode *nodes, int axis, ulong_t beg, ulong_t end);
static void SetupBackupArray(ArraySTRNode *backup, ArraySTRNode *nodes, ulong_t beg, ulong_t end);
/* scatter sorted data */
static void ArrangeNodeArray(ArraySTRNode *nodes, ArraySTRNode *backup, KeyValue *keyval, ulong_t beg, ulong_t end);
/* float radix sort support functions */
static void RadixSortEncodeValues(KeyValue *keyval, ulong_t beg, ulong_t end);
static void RadixSortDecodeValues(KeyValue *keyval, ulong_t beg, ulong_t end);
#define __END_RADIXSORT_SETUP__

/* single thread construction routines */
static void RadixSortWithKey(KeyValue *keyval, KeyValue *tmp_keyval, ulong_t offset, ulong_t ndata);
static void PartialRadixSortWithKey(KeyValue *keyval, KeyValue *tmp_keyval, ulong_t ndata, ulong_t slice, ulong_t width);
static void PackNodes(ArraySTRNode *nodes, ulong_t first, ulong_t last, ulong_t width, ulong_t *parentp);


#define __BEGIN_PARALLEL_RADIXSORT__
/* multi thread worker routines */
static void ParallelSetupValues(void *arg);
static void ParallelSetupKeyValues(void *arg);
static void ParallelArrangeNodeArray(void *arg);
static void ParallelSetupBackupArray(void *arg);
static void ParallelEncodeValues(void *arg);
static void ParallelDecodeValues(void *arg);
static void ParallelRadixSortWithKeyLSD(void *arg);
/* interface for calling worker functions */
static void ParallelRadixSortSetup(SameThreadPool *stp, void (*func)(void *),
				   KeyValue *keyval, ArraySTRNode *nodes, int axis, ulong_t ndata, ArraySTRNode *backup);
static void ParallelRadixSortWithKey(SameThreadPool *stp, KeyValue *keyval, KeyValue *tmp_keyval,
				     ulong_t *bins[], ulong_t ndata);
#define __END_PARALLEL_RADIXSORT__

#define __BEGIN_PARALLEL_PARTIAL_RADIXSORT__
/* multi thread worker routines */
static void ParallelPartialRadixSortWithKeyWorker(void *arg);
/* interface for calling worker functions */
static void ParallelPartialRadixSortWithKey(SameThreadPool *stp, KeyValue *keyval, KeyValue *tmp_keyval,
					    ulong_t ndata, ulong_t slice, ulong_t width);
#define __END_PARALLEL_PARTIAL_RADIXSORT__

#define __BEGIN_PARALLEL_PACK__
/* multi thread worker routines */
static void ParallelPackNodesWorker(void *arg);
/* interface for calling worker functions */
static void ParallelPackNodes(SameThreadPool *stp, ArraySTRNode *nodes, ulong_t first, ulong_t last, ulong_t width, ulong_t *parentp);
#define __END_PARALLEL_PACK__

#endif //ARRAYSTR_BUILD_SOURCE__


/*** public ***/
extern void InsertDataArraySTR(ArraySTR *art, ulong_t id, float p[]);
extern void MakeTreeArraySTR(ArraySTR *art, int np);
extern void MakeTreeWithSameThreadPoolArraySTR(ArraySTR *art, SameThreadPool *stp);

#endif//ARRAYSTR_BUILD_HEAD__
