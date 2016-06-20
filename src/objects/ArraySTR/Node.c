#define ARRAYSTR_NODE_SOURCE__

#include <x86intrin.h>

#include "objects/ArraySTR/Node.h"

#define ENABLE_SSE_ADJUST 

/* Adjust MBR to fit all child MBRs */
inline 
void 
adjustMbrArraySTRNode(ArraySTRNode nodes[], ulong_t cur)
{
	ArraySTRNode *node, *child;
	ulong_t k;
	
	node = &nodes[cur];
	child = &nodes[node->pos];
	
	/* enlarge mbr to include all childlen's mbr */
#ifdef ENABLE_SSE_ADJUST 
	{
		__m128 v_nlow = _mm_load_ps(child[0].mbr.low);
		__m128 v_nupp = _mm_load_ps(child[0].mbr.upp);
		for (k = 1; k < node->len; k++) {
			v_nlow = _mm_min_ps(v_nlow, _mm_load_ps(child[k].mbr.low));
			v_nupp = _mm_max_ps(v_nupp, _mm_load_ps(child[k].mbr.upp));
		}
		_mm_store_ps(node->mbr.low, v_nlow);
		_mm_store_ps(node->mbr.upp, v_nupp);
	}
#else
#ifdef ENABLE_AVX_TEST1	
	{
		__m256 v_nmbr = _mm256_loadu_ps((float *)&child[0].mbr);
		for (k = 1; k < node->len; k++) {
			__m256 v_cmbr = _mm256_loadu_ps((float *)&child[k].mbr);
			__m256 v_min = _mm256_min_ps(v_nmbr, v_cmbr);
			__m256 v_max = _mm256_max_ps(v_nmbr, v_cmbr);
			v_nmbr = _mm256_permute2f128_ps(v_min, v_max, 0x12);
		}
		_mm256_storeu_ps((float *)&node->mbr, v_nmbr);
	}
#else
	/* copy first child's mbr */
	node->mbr = child[0].mbr;
	for (k = 1; k < node->len; k++) {
		int i;
		for (i = 0; i < NDIMS; i++) {
			if (node->mbr.low[i] > child[k].mbr.low[i])
				node->mbr.low[i] = child[k].mbr.low[i];
			if (node->mbr.upp[i] < child[k].mbr.upp[i])
				node->mbr.upp[i] = child[k].mbr.upp[i];
		}
	}
#endif
#endif
}
