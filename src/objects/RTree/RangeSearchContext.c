#include <stdio.h>
#include <stdlib.h>

#include "objects/RTree/RangeSearchContext.h"


inline 
RangeSearchContext *
constructRangeSearchContext(float radius, float *central)
{
	static const int MAX = 1024 * 128;
	RangeSearchContext *rsc;
	
	rsc = (void *)malloc(sizeof(*rsc) + sizeof(*rsc->ids) * MAX);
	rsc->radius = radius;
	rsc->central = central;
	rsc->nids = 0;
	rsc->len = MAX;
	
	return rsc;
}

inline 
void 
destructRangeSearchContext(RangeSearchContext *rsc)
{
	free(rsc);
}

inline 
bool 
needReallocRangeSearchContext(RangeSearchContext *rsc, ulong_t need)
{
	return (need > rsc->len);
}

inline 
RangeSearchContext *
reallocRangeSearchContext(RangeSearchContext *rsc, ulong_t need)
{
	void *ptr;
	
	if (needReallocRangeSearchContext(rsc, need)) {
		do {
                        rsc->len *= 2;
                } while (needReallocRangeSearchContext(rsc, need));
		
		ptr = (void *)realloc(rsc, sizeof(*rsc) + sizeof(*rsc->ids) * rsc->len);
		if (ptr == NULL) {
			fprintf(stderr, "%s(): memory shortage.\n", __func__);
			fprintf(stderr, "-- RangeSearchContext %p was freed.\n", rsc);
			destructRangeSearchContext(rsc);
			return NULL;
		}
		rsc = ptr;
	}
	
	return rsc;
}
