#ifndef ARRAYSTR_TRAVERSE_HEAD__
#define ARRAYSTR_TRAVERSE_HEAD__

#include "objects/ArraySTR/ArraySTR.h"

/*** public ***/
extern void traverseArraySTR(ArraySTR *art);

/*** private ***/
#ifdef ARRAYSTR_TRAVERSE_SOURCE__
static void TraverseRecursive(ArraySTR *art, ulong_t inode, int level);
#endif

#endif //ARRAYSTR_TRAVERSE_HEAD__
