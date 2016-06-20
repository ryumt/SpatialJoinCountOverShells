#ifndef RTREE_MULTIRANGECOUNTCONTEXT_HEAD__
#define RTREE_MULTIRANGECOUNTCONTEXT_HEAD__

#include <stdbool.h>
#include "types/ulong.h"

typedef struct MultiRangeCountContext {
	/* number of dimension */
        int ndims;
        /* number of radiuses */
        int nrads;
        /* central point float[ndims] */
        float *central;
        /* range float[nrads] */
        float *radiuses;
        /* count of each radius ulong[nrads] */
        ulong_t *counts;
} MultiRangeCountContext;

extern MultiRangeCountContext *constructMultiRangeCountContext(int nrads, float *central, bool alloc_rads, bool alloc_cnts);
extern void destructMultiRangeCountContext(MultiRangeCountContext *mrcc, bool free_rads, bool free_cnts);
extern int checkConstraintMultiRangeCountContext(MultiRangeCountContext *mrcc);
#endif //RTREE_MULTIRANGECOUNTCONTEXT_HEAD__
