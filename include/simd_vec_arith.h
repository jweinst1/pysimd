#ifndef SIMD_VEC_ARITH_H
#define SIMD_VEC_ARITH_H

#include "simd_vec_type.h"
#include "vec_macros.h"

static void simd_vec_add_i8(struct pysimd_vec_t* v1, const struct pysimd_vec_t* v2) {
	const size_t oper_region = PYSIMD_MIN_VEC_SIZE(v1, v2);
	size_t i = 0;
	while (i < oper_region) {
#if defined(PYSIMD_X86_SSE2)
	__m128i v1seg = _mm_load_si128((__m128i const*)(v1->data + i));
	__m128i v2seg = _mm_load_si128((__m128i const*)(v2->data + i));
	__m128i added = _mm_add_epi8 (v1seg, v2seg);
	_mm_storeu_si128 ((__m128i*)(v1->data + i), added);
	i += 16;
#else
	v1->data[i] = v1->data[i] + v2->data[i];
	++i;
#endif
	}
}

static void simd_vec_add_i16(struct pysimd_vec_t* v1, const struct pysimd_vec_t* v2) {
	const size_t oper_region = PYSIMD_MIN_VEC_SIZE(v1, v2);
	size_t i = 0;
	while (i < oper_region) {
#if defined(PYSIMD_X86_SSE2)
	__m128i v1seg = _mm_load_si128((__m128i const*)(v1->data + i));
	__m128i v2seg = _mm_load_si128((__m128i const*)(v2->data + i));
	__m128i added = _mm_add_epi16(v1seg, v2seg);
	_mm_storeu_si128 ((__m128i*)(v1->data + i), added);
	i += 16;
#else
	*(short*)(v1->data + i) = (*(short*)(v1->data + i)) + (*(short*)(v2->data + i));
	i += 2;
#endif
	}
}

static void simd_vec_add_i32(struct pysimd_vec_t* v1, const struct pysimd_vec_t* v2) {
	const size_t oper_region = PYSIMD_MIN_VEC_SIZE(v1, v2);
	size_t i = 0;
	while (i < oper_region) {
#if defined(PYSIMD_X86_SSE2)
	__m128i v1seg = _mm_load_si128((__m128i const*)(v1->data + i));
	__m128i v2seg = _mm_load_si128((__m128i const*)(v2->data + i));
	__m128i added = _mm_add_epi32(v1seg, v2seg);
	_mm_storeu_si128 ((__m128i*)(v1->data + i), added);
	i += 16;
#else
	*(int*)(v1->data + i) = (*(int*)(v1->data + i)) + (*(int*)(v2->data + i));
	i += 4;
#endif
	}
}

static void simd_vec_add_i64(struct pysimd_vec_t* v1, const struct pysimd_vec_t* v2) {
	const size_t oper_region = PYSIMD_MIN_VEC_SIZE(v1, v2);
	size_t i = 0;
	while (i < oper_region) {
#if defined(PYSIMD_X86_SSE2)
	__m128i v1seg = _mm_load_si128((__m128i const*)(v1->data + i));
	__m128i v2seg = _mm_load_si128((__m128i const*)(v2->data + i));
	__m128i added = _mm_add_epi64(v1seg, v2seg);
	_mm_storeu_si128 ((__m128i*)(v1->data + i), added);
	i += 16;
#else
	*(long long*)(v1->data + i) = (*(long long*)(v1->data + i)) + (*(long long*)(v2->data + i));
	i += 8;
#endif
	}
}

#endif // SIMD_VEC_ARITH_H
