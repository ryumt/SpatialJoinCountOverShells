#define RTREE_MBR_SOURCE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <x86intrin.h>

#include "sse_functions.h"
#include "objects/RTree/Mbr.h"


// #define ENABLE_SSE_CALC_MARGIN /* for RStarTree */
#define ENABLE_SSE_CALC_AREA /* for RTree */
// #define ENABLE_SSE_CALC_CENTERDIST /* for RStarTree */
#define ENABLE_SSE_ENLARGE /* for RTree */
#define ENABLE_SSE_CHECKOVL /* for Search */
#define ENABLE_SSE_SETQMBR /* for Search */
#define ENABLE_SSE_CEHCKOOB /* for Search */


inline
float 
calculateMarginMbr(Mbr *mbr)
{
#ifdef ENABLE_SSE_CALC_MARGIN
	__m128 v_low = _mm_load_ps(mbr->low);
	__m128 v_upp = _mm_load_ps(mbr->upp);
	__m128 v_length;
	
	v_length = _mm_sub_ps(v_upp, v_low);
	/* dot product to do horizontal add (will gain length^2 but it doesn't matter in cost calc) */
	v_length = _mm_dp_ps(v_length, v_length, 0x77);
	return _mm_cvtss_f32(v_length);
#else 
	float length = 0.0;
	int i;
	
	for (i = 0; i < NDIMS; i++) {
		float diff = mbr->upp[i] - mbr->low[i];
		length += diff;
	}
	return length;
#endif 
}

inline 
float 
calculateAreaMbr(Mbr *mbr)
{
	float area = 1.0;

#ifdef ENABLE_SSE_CALC_AREA	
	__m128 v_low = _mm_load_ps(mbr->low);
	__m128 v_upp = _mm_load_ps(mbr->upp);
	__m128 v_diff = _mm_sub_ps(v_upp, v_low);
	
	area *= _mm_cvtss_f32(v_diff);
	area *= _mm_cvtss_f32(_mm_shuffle_ps(v_diff, v_diff, _MM_SHUFFLE(0, 2, 3, 1)));
	area *= _mm_cvtss_f32(_mm_shuffle_ps(v_diff, v_diff, _MM_SHUFFLE(1, 3, 0, 2)));
#else
	int i;
	for (i = 0; i < NDIMS; i++) {
		float diff = mbr->upp[i] - mbr->low[i];
		area *= diff;
	}
#endif
	return area;
}

inline
float 
calculateOverlapAreaMbr(Mbr *mbr1, Mbr *mbr2)
{
	float area;
	int i;
	
	area = 1.0;
	for (i = 0; i < NDIMS; i++) {
                float diff;
		
                if (mbr1->upp[i] > mbr2->low[i] && mbr2->upp[i] > mbr1->upp[i]) {
                        if (mbr1->low[i] > mbr2->low[i])
                                diff = mbr1->upp[i] - mbr1->low[i];
                        else
                                diff = mbr1->upp[i] - mbr2->low[i];
                }
                else if (mbr2->upp[i] > mbr1->low[i] && mbr1->upp[i] > mbr2->upp[i]) {
                        if (mbr2->low[i] > mbr1->low[i])
                                diff = mbr2->upp[i] - mbr2->low[i];
                        else
                                diff = mbr2->upp[i] - mbr1->low[i];
                }
                else {
                        area = 0.0;
                        break;
                }
                area *= diff;
	}
	return area;
}

inline 
float 
calculateCenterDistanceMbr(Mbr *mbr1, Mbr *mbr2)
{
#ifdef ENABLE_SSE_CALC_CENTERDIST
	static float TWO[] = {
		2.0, 2.0, 2.0, 2.0
	};
	__m128 v_two = _mm_load_ss(TWO);
	__m128 v_cnt1 = _mm_div_ps(_mm_add_ps(_mm_load_ps(mbr1->low), _mm_load_ps(mbr1->upp)), v_two);
	__m128 v_cnt2 = _mm_div_ps(_mm_add_ps(_mm_load_ps(mbr2->low), _mm_load_ps(mbr2->upp)), v_two);
	__m128 v_diff = _mm_sub_ps(v_cnt1, v_cnt2);
	__m128 v_dist = _mm_dp_ps(v_diff, v_diff, 0x77);
	return _mm_cvtss_f32(v_dist);	
#else
	float dist;
	int i;
	
	dist = 0;
	for (i = 0; i < NDIMS; i++) {
		float central1 = (mbr1->upp[i] + mbr1->low[i]) / 2;
		float central2 = (mbr2->upp[i] + mbr2->low[i]) / 2;
		float diff = central1 - central2;
		dist += diff * diff;
	}
	return dist;
#endif
}


inline
void 
enlargeMbr(Mbr *mbr, Mbr *expand)
{
#ifdef ENABLE_SSE_ENLARGE
	__m128 v_nlow = _mm_load_ps(mbr->low);
	__m128 v_nupp = _mm_load_ps(mbr->upp);
	__m128 v_clow = _mm_load_ps(expand->low);
	__m128 v_cupp = _mm_load_ps(expand->upp);
	_mm_store_ps(mbr->low, _mm_min_ps(v_nlow, v_clow));
	_mm_store_ps(mbr->upp, _mm_max_ps(v_nupp, v_cupp));
#else
#ifdef ENABLE_AVX_TEST1
	__m256 v_nmbr = _mm256_loadu_ps((float *)&mbr);
	__m256 v_cmbr = _mm256_loadu_ps((float *)&expand);
	__m256 v_min = _mm256_min_ps(v_nmbr, v_cmbr);
	__m256 v_max = _mm256_max_ps(v_nmbr, v_cmbr);
	__m256 v_tmp;
	v_tmp = _mm256_permute2f128_ps(v_min, v_max, 0x12);
	_mm256_storeu_ps((float *)&mbr, v_tmp);
#else
	int i;
	for (i = 0; i < NDIMS; i++) {
		if (mbr->upp[i] < expand->upp[i])
			mbr->upp[i] = expand->upp[i];
		if (mbr->low[i] > expand->low[i])
			mbr->low[i] = expand->low[i];
	}
#endif
#endif
}


inline 
bool 
checkOverlapMbr(Mbr *mbr1, Mbr *mbr2)
{
#ifdef ENABLE_SSE_CHECKOVL
	__m128 v_m1low = _mm_load_ps(mbr1->low);
	__m128 v_m1upp = _mm_load_ps(mbr1->upp);
	__m128 v_m2low = _mm_load_ps(mbr2->low);
	__m128 v_m2upp = _mm_load_ps(mbr2->upp);
	
	if (!_mm_test_all_ones((__m128i)_mm_cmpnlt_ps(v_m1upp, v_m2low)))
		return false;
	if (!_mm_test_all_ones((__m128i)_mm_cmpnlt_ps(v_m2upp, v_m1low)))
		return false;
	
	return true;
#else
#ifdef ENABLE_AVX_TEST0
	__m256 v_mbr1 = _mm256_loadu_ps((float *)mbr1);
	__m256 v_mbr2 = _mm256_loadu_ps((float *)mbr2);
	__m256 v_upp = _mm256_permute2f128_ps(v_mbr1, v_mbr2, 0x20);
	__m256 v_low = _mm256_permute2f128_ps(v_mbr1, v_mbr2, 0x13);
	__m256 v_tmp;
	
	v_tmp = _mm256_cmp_ps(v_upp, v_low, _CMP_NLT_UQ);
	if (!_mm256_testc_ps(v_tmp, _mm256_cmp_ps(v_tmp, v_tmp, _CMP_EQ_UQ)))
		return false;
	return true;	
#else
	int i;
	
	for (i = 0; i < NDIMS; i++) {
		if (mbr1->upp[i] < mbr2->low[i] || mbr2->upp[i] < mbr1->low[i])
			return false;
	}
	return true;
#endif //ENABLE_AVX_TEST0
#endif //ENABLE_SSE_CHECKOVL
}

inline 
void 
setQueryMbr(Mbr *qmbr, float *central, float radius)
{
#ifdef ENABLE_SSE_SETQMBR
	__m128 v_cnt = _mm_loadu_ps(central);
	__m128 v_rad = _mm_broadcast_ss(&radius);
	__m128 v_tmp;
	v_tmp = _mm_sub_ps(v_cnt, v_rad);
	v_tmp = (__m128)_mm_insert_epi32((__m128i)v_tmp, 0x00, 3);
	_mm_store_ps(qmbr->low, v_tmp);
	v_tmp = _mm_add_ps(v_cnt, v_rad);
	v_tmp = (__m128)_mm_insert_epi32((__m128i)v_tmp, 0x00, 3);
	_mm_store_ps(qmbr->upp, v_tmp);
#else
#ifdef ENABLE_AVX_TEST2
	__m256 v_cnt = _mm256_loadu_ps(central);
	__m256 v_rad = _mm256_broadcast_ss(&radius);
	__m256 v_tmp;
	v_cnt = _mm256_permute2f128_ps(v_cnt, v_cnt, 0x00);
	v_tmp = _mm256_add_ps(v_cnt, mm256UpperNegate(v_rad));
	v_tmp = (__m256)_mm256_insert_epi32((__m256i)v_tmp, 0x00, 3);
	v_tmp = (__m256)_mm256_insert_epi32((__m256i)v_tmp, 0x00, 7);
	_mm256_store_ps((float *)qmbr, v_tmp);
#else
	int i;
	for (i = 0; i < NDIMS; i++) {
		qmbr->low[i] = central[i] - radius;
		qmbr->upp[i] = central[i] + radius;
	}
#endif
#endif
}

inline
void 
checkMinimumOutOfBoundMbr(Mbr *mbr, uint32_t *oob_flags)
{
#ifdef ENABLE_SSE_CEHCKOOB
	static const float LBOUND[] __attribute__((__aligned__(16))) = {
		LOW_BOUND, LOW_BOUND, LOW_BOUND, 0.0f
	};
	__m128 v_low = _mm_load_ps(mbr->low);
	__m128 v_lbound = _mm_load_ps(LBOUND);
	__m128 v_oob = _mm_cmplt_ps(v_low, v_lbound);
	_mm_store_ps((float *)oob_flags, v_oob);
#else 
	int i;
	for (i = 0; i < NDIMS; i++) {
                if (mbr->low[i] < LOW_BOUND)
			oob_flags[i] = true;
	}
#endif //ENABLE_SSE_CEHCKOOB	
}

inline
void 
checkMaximumOutOfBoundMbr(Mbr *mbr, uint32_t *oob_flags)
{
#ifdef ENABLE_SSE_CEHCKOOB
	static const float UBOUND[] __attribute__((__aligned__(16))) = {
		UPP_BOUND, UPP_BOUND, UPP_BOUND, 0.0f
	};
	__m128 v_upp = _mm_load_ps(mbr->upp);
	__m128 v_ubound = _mm_load_ps(UBOUND);
	__m128 v_oob = _mm_cmpgt_ps(v_upp, v_ubound);
	_mm_store_ps((float *)oob_flags, v_oob);
#else 
	int i;
	for (i = 0; i < NDIMS; i++) {
		if (mbr->upp[i] > UPP_BOUND)
			oob_flags[i] = true;
        }
#endif //ENABLE_SSE_CEHCKOOB	
}


inline
void 
checkMinMaxOutOfBoundMbr(Mbr *mbr, uint32_t *min_oob, uint32_t *max_oob)
{
#ifdef ENABLE_SSE_CEHCKOOB
	static const float LBOUND[] __attribute__((__aligned__(16))) = {
		LOW_BOUND, LOW_BOUND, LOW_BOUND, 0.0f
	};
	static const float UBOUND[] __attribute__((__aligned__(16))) = {
		UPP_BOUND, UPP_BOUND, UPP_BOUND, 0.0f
	};
	
	__m128 v_low = _mm_load_ps(mbr->low);
	__m128 v_upp = _mm_load_ps(mbr->upp);
	__m128 v_lbound = _mm_load_ps(LBOUND);
	__m128 v_ubound = _mm_load_ps(UBOUND);
	__m128 v_minoob = _mm_cmplt_ps(v_low, v_lbound);
	__m128 v_maxoob = _mm_cmpgt_ps(v_upp, v_ubound);
	_mm_store_ps((float *)min_oob, v_minoob);
	_mm_store_ps((float *)max_oob, v_maxoob);
#else 
	int i;
	for (i = 0; i < NDIMS; i++) {
                if (mbr->low[i] < LOW_BOUND)
			min_oob[i] = true;
		else if (mbr->upp[i] > UPP_BOUND)
			max_oob[i] = true;
        }
#endif //ENABLE_SSE_CEHCKOOB	
}
