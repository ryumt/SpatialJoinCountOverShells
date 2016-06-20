#ifndef ARRAYSTR_RANGESEARCH_HEAD__
#define ARRAYSTR_RANGESEARCH_HEAD__

#include <stdbool.h>

#include "types/ulong.h"
#include "objects/ArraySTR/ArraySTR.h"
#include "objects/RTree/RangeSearchContext.h"

/*** public ***/
extern RangeSearchContext *rangeSearchArraySTR(ArraySTR *art, RangeSearchContext *rsc);

/*** private ***/
#ifdef ARRAYSTR_RANGESEARCH_SOURCE__
static RangeSearchContext *RangeSearchDataIdRecursive(ArraySTR *art, ulong_t inode, Mbr *qmbr, RangeSearchContext *rsc);
#endif //ARRAYSTR_RANGESEARCH_SOURCE__

#endif //ARRAYSTR_RANGESEARCH_HEAD__
