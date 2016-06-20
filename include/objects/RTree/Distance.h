#include <x86intrin.h>

#define ENABLE_SSE_CALC_DIST

static inline
float
CalcDistance(float p1[], float p2[])
{
#ifdef ENABLE_SSE_CALC_DIST
	/* make sure that p[NDIMS] = 0.0 & p is aligned 16 */
	__m128 v_diff = _mm_sub_ps(_mm_load_ps(p1), _mm_loadu_ps(p2));
	__m128 v_dist = _mm_dp_ps(v_diff, v_diff, 0x77);
	return _mm_cvtss_f32(v_dist);
#else
	float dist;
	int i;
	
	dist = 0;
	for (i = 0; i < NDIMS; i++) {
		float diff = p1[i] - p2[i];
		dist += diff * diff;
	}
        return dist;
#endif
}

static inline
bool
RangeSearchValidation(float p1[], float p2[], float radius)
{
#ifdef ENABLE_SSE_CALC_DIST
        __m128 v_rad = _mm_load_ss(&radius);
        __m128 v_diff = _mm_sub_ps(_mm_load_ps(p1), _mm_loadu_ps(p2));
        __m128 v_dist = _mm_dp_ps(v_diff, v_diff, 0x77);
        return _mm_comilt_ss(v_dist, v_rad);
#else
        float dist;
        int i;

        dist = 0;
        for (i = 0; i < NDIMS; i++) {
                float diff = p1[i] - p2[i];
                dist += diff * diff;
        }
        return (dist < radius);
#endif
}
