#ifndef ARRAYSTR_HEAD__
#define ARRAYSTR_HEAD__

#include <stdbool.h>
#include "types/ulong.h"

#include "objects/SameThreadPool/SameThreadPool.h"

/*** friend public ***/
#ifdef ARRAYSTR_FAMILY__

//#define ENABLE_SSE_TEST__
//#define ENABLE_AVX_TEST__
//#define ENABLE_SSE_TEST0
//#define ENABLE_SSE_TEST1
//#define ENABLE_SSE_TEST2
//#define ENABLE_SSE_TEST3
//#define ENABLE_SSE_TEST5

//#define ENABLE_AVX_TEST0
//#define ENABLE_AVX_TEST1
//#define ENABLE_AVX_TEST2

#include "objects/ArraySTR/Node.h"

typedef struct NumsAndIdx {
	/* number of non-leaf/leaf */
	ulong_t nums;
	/* first index of non-leaf/leaf */
	ulong_t fidx;
} NumsAndIdx;

typedef struct ArraySTR {
	/* working index */
	ulong_t head;
	/* index of root */
	ulong_t root;
	
	/* NumsAndIdx structure for node/leaf */
	NumsAndIdx ni4node;
	NumsAndIdx ni4leaf;
	
	/* node array */
	ArraySTRNode nodes[0];
} ArraySTR;
/*** public ***/
#else
typedef struct ArraySTR ArraySTR;
#endif //ARRAYSTR_FAMILY__

/*** public ***/
extern ArraySTR *constructArraySTR(ulong_t ndata);
extern void destructArraySTR(ArraySTR *art);
extern void insertDataArraySTR(ArraySTR *art, ulong_t id, float p[]);
extern void makeTreeArraySTR(ArraySTR *art, int np);
extern void makeTreeWithSameThreadPoolArraySTR(ArraySTR *art, SameThreadPool *stp);

extern ulong_t getNumberOfDataArraySTR(ArraySTR *art);

/*** private ***/
#ifdef ARRAYSTR_SOURCE__
static ulong_t CalcNumberOfNodes(ulong_t nl, ulong_t ne);
#endif //ARRAYSTR_SOURCE__

#endif //ARRAYSTR_HEAD__
