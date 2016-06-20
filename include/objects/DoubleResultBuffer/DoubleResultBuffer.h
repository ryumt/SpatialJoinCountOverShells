#ifndef DOUBLERESULTBUFFER_HEAD__
#define DOUBLERESULTBUFFER_HEAD__

#include "objects/ResultBuffer/ResultBuffer.h"

typedef struct DoubleResultBuffer {
        /* index of current result buffer */
        int idx;
        /* dummy, may contain additional data */
        int pad;
        ResultBuffer *rb[2];
} DoubleResultBuffer;

extern DoubleResultBuffer *constructDoubleResultBuffer(uint32_t size);
extern void destructDoubleResultBuffer(DoubleResultBuffer *drb);
extern ResultBuffer *getCurrentBufferDoubleResultBuffer(DoubleResultBuffer *drb);
extern ResultBuffer *getNextBufferDoubleResultBuffer(DoubleResultBuffer *drb);
extern void rewriteIndexDoubleResultBuffer(DoubleResultBuffer *drb, int next);
extern void switchIndexDoubleResultBuffer(DoubleResultBuffer *drb);

#endif //DOUBLERESULTBUFFER_HEAD__
