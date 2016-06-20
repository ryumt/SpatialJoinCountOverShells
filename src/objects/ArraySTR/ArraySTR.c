#define ARRAYSTR_FAMILY__
#define ARRAYSTR_SOURCE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <pthread.h>

#include "support_functions.h"
#include "objects/ArraySTR/ArraySTR.h"
#include "objects/ArraySTR/Build.h"


inline 
ArraySTR *
constructArraySTR(ulong_t ndata)
{
	ArraySTR *art;
	ulong_t nn;
	
	nn = CalcNumberOfNodes(ndata, MAX_ENT);
	
	art = (void *)malloc(sizeof(*art) + sizeof(ArraySTRNode) * (nn + ndata));
	bzero(art, sizeof(ArraySTR));
	
	art->ni4node.nums = nn;
	art->ni4leaf.nums = ndata;
		
	art->head = nn + ndata;
		
	return art;
}

inline 
void 
destructArraySTR(ArraySTR *art)
{
	free(art);
}

	
/* Bulk Load */
inline
void 
insertDataArraySTR(ArraySTR *art, ulong_t id, float p[])
{
	InsertDataArraySTR(art, id, p);
}

/* Parallel Construct */
inline 
void 
makeTreeArraySTR(ArraySTR *art, int np)
{
	MakeTreeArraySTR(art, np);
}

inline 
void 
makeTreeWithSameThreadPoolArraySTR(ArraySTR *art, SameThreadPool *stp)
{
	MakeTreeWithSameThreadPoolArraySTR(art, stp);
}

inline 
ulong_t 
getNumberOfDataArraySTR(ArraySTR *art)
{
	return art->ni4leaf.nums;	
}

/* nl: number of leaves, ne: max number of entries in a node */
static inline 
ulong_t 
CalcNumberOfNodes(ulong_t nl, ulong_t ne)
{
	/* nn: number of nodes */
	ulong_t nn = 0;
	/* nd: number of data */
	ulong_t nd;
	
	nd = nl;
	do {
		nd = UlongCeil(nd, ne);
		nn += nd;
	} while (nd > 1);
       
	return nn;
}

