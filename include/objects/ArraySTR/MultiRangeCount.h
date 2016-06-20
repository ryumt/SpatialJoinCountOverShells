#ifndef ARRAYSTR_MULTIRANGECOUNT_HEAD__
#define ARRAYSTR_MULTIRANGECOUNT_HEAD__

#include "types/ulong.h"

#include "objects/ArraySTR/ArraySTR.h"
#include "objects/RTree/MultiRangeCountContext.h"

/*** public ***/
extern void multiRangeCountArraySTR(ArraySTR *art, MultiRangeCountContext *mrcc);
extern void multiRangeCountWithPriodicBoundArraySTR(ArraySTR *art, MultiRangeCountContext *mrcc);

/*** private ***/
#ifdef ARRAYSTR_MULTIRANGECOUNT_SOURCE__
static void MultiRangeCountRecursive(ArraySTR *art, ulong_t inode, Mbr *qmbr, MultiRangeCountContext *mrcc);
#endif

#endif //ARRAYSTR_MULTIRANGECOUNT_HEAD__
