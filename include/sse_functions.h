#ifndef SSE_FUNCTIONS_HEAD__
#define SSE_FUNCTIONS_HEAD__

#include <stdbool.h>
#include <stdint.h>
#include <x86intrin.h>

static inline 
bool 
mm128TestAllZeros(__m128i v)
{
	return (bool)_mm_cvtss_f32(_mm_dp_ps((__m128)v, (__m128)v, 0xFF));
}

static inline 
__m128i 
mm128Invert(__m128i v)
{
	static const uint32_t mask[] __attribute__((__aligned__(16))) = {
		~0x00, ~0x00, ~0x00, ~0x00
	};
	return _mm_xor_si128(v, (__m128i)_mm_load_ps((float *)mask));
}

static inline 
__m256
mm256UpperNegate(__m256 v)
{
	static const uint32_t mask[] __attribute__((__aligned__(32))) = {
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 
		0x80000000, 0x80000000, 0x80000000, 0x80000000
	};
	return _mm256_xor_ps(v, _mm256_load_ps((float *)mask));
}

#endif //SSE_FUNCTIONS_HEAD__
