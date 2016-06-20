#ifndef RTREE_RANGESEARCHCONTEXT_HEAD__
#define RTREE_RANGESEARCHCONTEXT_HEAD__

#include <stdbool.h>
#include "types/ulong.h"

typedef struct RangeSearchContext {
	/* central point */
	float *central;
	/* range */
	float radius;
	/* current number of entries in ids[] */
	ulong_t nids;
	/* max number of entries ids[] can hold */
	ulong_t len;
	/* result id array */
	ulong_t ids[0];
} RangeSearchContext;

extern RangeSearchContext *constructRangeSearchContext(float radius, float *central);
extern void destructRangeSearchContext(RangeSearchContext *rsc);
extern bool needReallocRangeSearchContext(RangeSearchContext *rsc, ulong_t need);
extern RangeSearchContext *reallocRangeSearchContext(RangeSearchContext *rsc, ulong_t need);

#endif //RTREE_RANGESEARCHCONTEXT_HEAD__
