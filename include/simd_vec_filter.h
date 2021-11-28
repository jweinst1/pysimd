#ifndef PYSIMD_VEC_FILTER_H
#define PYSIMD_VEC_FILTER_H

#include "simd_vec_type.h"
#include "vec_macros.h"


static int pysimd_vec_filter_32(struct pysimd_vec_t* vec, int* gt,
	                                                      int* lt,
	                                                      int* eq) {
#if defined(PYSIMD_X86_SSE2)
	// pass
	unsigned char* ptr = vec->data;
	const unsigned char* ptr_end = ptr + vec->size;
	while (ptr < ptr_end) {
		__m128i loaded = _mm_load_si128((__m128i const*)ptr);
		__m128i mask = _mm_set1_epi8(0xff);
		if (gt != NULL) {
			__m128i gtnum = _mm_set1_epi32(*gt);
			__m128i gtres = _mm_cmpgt_epi32 (loaded, gtnum);
			mask = _mm_and_si128(mask, gtres);
		}
		if (lt != NULL) {
			__m128i ltnum = _mm_set1_epi32(*lt);
			__m128i ltres = _mm_cmplt_epi32 (loaded, ltnum);
			mask = _mm_and_si128(mask, ltres);
		}
		if (eq != NULL) {
			__m128i eqnum = _mm_set1_epi32(*eq);
			__m128i eqres = _mm_cmpeq_epi32 (loaded, eqnum);
			mask = _mm_and_si128(mask, eqres);
		}
		__m128i final_result = _mm_and_si128(mask, loaded);
		int mask_result = _mm_movemask_epi8 (final_result);
		size_t to_advance = 0;
		switch (mask_result) {
			case 0xFFFF:
			case 0x0FFF:
			case 0x00FF:
			case 0x000F:
			case 0x0:
			      // no filtering needed
			      to_advance = 16;
			      break;
			case 0xFF0F:
			      final_result = _mm_shuffle_epi32(final_result, 0x78);
			      to_advance = 12;
			      break;
			case 0xF00F:
			      // shuffle, reverse order of 0b10101100
			      final_result = _mm_shuffle_epi32(final_result, 0xac);
			      to_advance = 8;
			      break;
			case 0x0FF0:
			      // shuffle, reverse order of 0b11001001
			      final_result = _mm_shuffle_epi32(final_result, 0xc9);
			      to_advance = 8;
			      break;
			case 0xF0F0:
			      // shuffle, reverse order of 0b10001101
			      final_result = _mm_shuffle_epi32(final_result, 0x8d);
			      to_advance = 8;
			      break;
			case 0x0F0F:
			      // shuffle, reverse order of 0b11011000
			      final_result = _mm_shuffle_epi32(final_result, 0xd8);
			      to_advance = 8;
			      break;
			case 0xF0FF:
			      final_result = _mm_shuffle_epi32(final_result, 0xb4);
			      to_advance = 12;
			      break;
			case 0xFFF0:
			      // shuffle, reversed order of 00011011
			      final_result = _mm_shuffle_epi32(final_result, 0x1b);
			      to_advance = 12;
			      break;
			case 0xFF00:
			      // shuffle, reversed order of 00001110
			      final_result = _mm_shuffle_epi32(final_result, 0xe);
			      to_advance = 8;
			      break;
			case 0xF000:
			      // shuffle, reversed order of 00000011
			      final_result = _mm_shuffle_epi32(final_result, 0x3);
			      to_advance = 4;
			      break;
			case 0x0F00:
			      // shuffle, reversed order of 0b00000010
			      final_result = _mm_shuffle_epi32(final_result, 0x2);
			      to_advance = 4;
			      break;
			case 0x00F0:
			      // shuffle, reversed order of 0b00000001
			      final_result = _mm_shuffle_epi32(final_result, 0x1);
			      to_advance = 4;
			      break;
			default:
			      fprintf(stderr, "Got impossible mask value: 0x%x, aborting ...\n", mask_result);
			      abort();
		}
		_mm_store_si128 ((__m128i*)ptr, final_result);
		ptr += to_advance;
	}
#else
	const unsigned char* reader = vec->data;
	const unsigned char* read_end = reader + vec->size;
	void* new_buf = calloc(1, vec->size);
	unsigned char* writer = new_buf;
	while (reader < read_end) {
		if (*reader) {
			*writer++ = *reader;
		}
		++reader;
	}
	free(vec->data);
	vec->data = new_buf;
#endif
	return 1;
}

static int pysimd_vec_filter_64(struct pysimd_vec_t* vec, long long* gt,
	                                                      long long* lt,
	                                                      long long* eq) {
#if defined(PYSIMD_X86_SSE2)
	// pass
	unsigned char* ptr = vec->data;
	const unsigned char* ptr_end = ptr + vec->size;
	while (ptr < ptr_end) {
		__m128i loaded = _mm_load_si128((__m128i const*)ptr);
		__m128i mask = _mm_set1_epi8(0xff);
		if (gt != NULL) {
			__m128i gtnum = _mm_set1_epi64(*gt);
			__m128i gtres = _mm_cmpgt_epi64(loaded, gtnum);
			mask = _mm_and_si128(mask, gtres);
		}
		if (lt != NULL) {
			__m128i ltnum = _mm_set1_epi64(*lt);
			__m128i ltres = _mm_cmplt_epi64(loaded, ltnum);
			mask = _mm_and_si128(mask, ltres);
		}
		if (eq != NULL) {
			__m128i eqnum = _mm_set1_epi64(*eq);
			__m128i eqres = _mm_cmpeq_epi64(loaded, eqnum);
			mask = _mm_and_si128(mask, eqres);
		}
		__m128i final_result = _mm_and_si128(mask, loaded);
		int mask_result = _mm_movemask_epi8 (final_result);
		size_t to_advance = 0;
		switch (mask_result) {
			case 0xFFFF:
			case 0x0FFF:
			case 0x00FF:
			case 0x000F:
			case 0x0:
			      // no filtering needed
			      to_advance = 16;
			      break;
			case 0xFF0F:
			      final_result = _mm_shuffle_epi32(final_result, 0x78);
			      to_advance = 12;
			      break;
			case 0xF00F:
			      // shuffle, reverse order of 0b10101100
			      final_result = _mm_shuffle_epi32(final_result, 0xac);
			      to_advance = 8;
			      break;
			case 0x0FF0:
			      // shuffle, reverse order of 0b11001001
			      final_result = _mm_shuffle_epi32(final_result, 0xc9);
			      to_advance = 8;
			      break;
			case 0xF0F0:
			      // shuffle, reverse order of 0b10001101
			      final_result = _mm_shuffle_epi32(final_result, 0x8d);
			      to_advance = 8;
			      break;
			case 0x0F0F:
			      // shuffle, reverse order of 0b11011000
			      final_result = _mm_shuffle_epi32(final_result, 0xd8);
			      to_advance = 8;
			      break;
			case 0xF0FF:
			      final_result = _mm_shuffle_epi32(final_result, 0xb4);
			      to_advance = 12;
			      break;
			case 0xFFF0:
			      // shuffle, reversed order of 00011011
			      final_result = _mm_shuffle_epi32(final_result, 0x1b);
			      to_advance = 12;
			      break;
			case 0xFF00:
			      // shuffle, reversed order of 00001110
			      final_result = _mm_shuffle_epi32(final_result, 0xe);
			      to_advance = 8;
			      break;
			case 0xF000:
			      // shuffle, reversed order of 00000011
			      final_result = _mm_shuffle_epi32(final_result, 0x3);
			      to_advance = 4;
			      break;
			case 0x0F00:
			      // shuffle, reversed order of 0b00000010
			      final_result = _mm_shuffle_epi32(final_result, 0x2);
			      to_advance = 4;
			      break;
			case 0x00F0:
			      // shuffle, reversed order of 0b00000001
			      final_result = _mm_shuffle_epi32(final_result, 0x1);
			      to_advance = 4;
			      break;
			default:
			      fprintf(stderr, "Got impossible mask value: 0x%x, aborting ...\n", mask_result);
			      abort();
		}
		_mm_store_si128 ((__m128i*)ptr, final_result);
		ptr += to_advance;
	}
#else
	const unsigned char* reader = vec->data;
	const unsigned char* read_end = reader + vec->size;
	void* new_buf = calloc(1, vec->size);
	unsigned char* writer = new_buf;
	while (reader < read_end) {
		if (*reader) {
			*writer++ = *reader;
		}
		++reader;
	}
	free(vec->data);
	vec->data = new_buf;
#endif
	return 1;
}

#endif // PYSIMD_VEC_FILTER_H
