#include <stdio.h>
#include <stdlib.h>

#include "atomic.h"
#include "objects/DoubleResultBuffer/DoubleResultBuffer.h"

inline
DoubleResultBuffer *
constructDoubleResultBuffer(uint32_t size)
{
	DoubleResultBuffer *drb;
	
        drb = (void *)malloc(sizeof(*drb));	
	drb->rb[0] = constructResultBuffer(size);
	drb->rb[1] = constructResultBuffer(size);
	drb->idx = 0;
	
        return drb;
}

inline
void
destructDoubleResultBuffer(DoubleResultBuffer *drb)
{
	destructResultBuffer(drb->rb[0]);
	destructResultBuffer(drb->rb[1]);
        free(drb);
}

inline 
ResultBuffer *
getCurrentBufferDoubleResultBuffer(DoubleResultBuffer *drb)
{
	return drb->rb[drb->idx];
}
inline 
ResultBuffer *
getNextBufferDoubleResultBuffer(DoubleResultBuffer *drb)
{
	return drb->rb[(drb->idx + 1) % 2];
}

inline
void
rewriteIndexDoubleResultBuffer(DoubleResultBuffer *drb, int next)
{
        compare_and_swap(&drb->idx, next);
}

inline
void
switchIndexDoubleResultBuffer(DoubleResultBuffer *drb)
{
        compare_and_swap(&drb->idx, (drb->idx + 1) % 2);
}
