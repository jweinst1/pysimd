#ifndef SIMD_VEC_H
#define SIMD_VEC_H

#include "simd_vec_type.h"

static inline void pysimd_vec_clear(struct pysimd_vec_t* vec) {
	vec->size = 0;
	vec->data = NULL;
}

static inline void pysimd_vec_clear_data(struct pysimd_vec_t* vec) {
	unsigned char* cleaner = vec->data;
	const unsigned char* end = cleaner + vec->size;
	while (cleaner != end) {
		*cleaner++ = 0;
	}
}

static void pysimd_vec_init(struct pysimd_vec_t* buf, size_t capacity)
{
	buf->size = capacity;
	buf->data = calloc(1, capacity);
}

static int pysimd_vec_resize(struct pysimd_vec_t* buf, size_t new_size) {
	if (new_size == 0)
		return 0;
	size_t old_size = buf->size;
	buf->size = new_size;
	buf->data = realloc(buf->data, new_size);
	while (old_size < new_size) {
		// Must make sure upstream is zeroed
		buf->data[old_size++] = 0;
	}
	return 1;
}

static void pysimd_vec_deinit(struct pysimd_vec_t* buf)
{
	buf->size = 0;
	free(buf->data);
}

static char* pysimd_vec_repr(const struct pysimd_vec_t* buf)
{
	char* repr_str = calloc(1, buf->size * 4 + 2);
	char* writer = repr_str;
	*writer++ = '[';
	size_t i = 0;
	for (; i < buf->size; ++i)
	{
		writer += sprintf(writer, "%x", buf->data[i]);
		if (i != (buf->size - 1))
			*writer++ = ',';
	}
	*writer++ = ']';
	return repr_str;
}

static int pysimd_vec_copy(struct pysimd_vec_t* dst, 
	                        const struct pysimd_vec_t* src,
	                        size_t start,
	                        size_t end)
{
	size_t diff = end - start;
	if (diff % 16 != 0)
		return 0;
	pysimd_vec_init(dst, diff);
	const unsigned char* reader = src->data + start;
	const unsigned char* read_end = src->data + end;
	unsigned char* writer = dst->data;
	while (reader < read_end) {
#if defined(PYSIMD_X86_SSE2)
		_mm_store_si128((__m128i*)writer, _mm_load_si128((__m128i const*)reader));
		reader += 16;
		writer += 16;
#else
		*(long long*)writer = *(long long*)reader;
		writer += sizeof(long long);
		reader += sizeof(long long);
#endif
	}
	return 1;
}

static int pysimd_vec_fill(struct pysimd_vec_t* buf, size_t val, unsigned char sizer)
{
	unsigned char* data_ptr = buf->data;
	const unsigned char* data_end = buf->data + buf->size;
#if defined(PYSIMD_X86_SSE2)
	__m128i filler;
	switch (sizer) {
		case 1:
		    filler = _mm_set1_epi8((char)val);
		    break;
		case 2:
		    filler = _mm_set1_epi16((short)val);
		    break;
		case 4:
		    filler = _mm_set1_epi32((int)val);
		    break;
		case 8:
		    filler = _mm_set1_epi64x(val);
		    break;
		default:
		    return 0;
	}
	while (data_ptr < data_end) {
		_mm_store_si128((__m128i*)data_ptr, filler);
		data_ptr += 16;
	}
#else
	char filler[16] = {0};
	switch (sizer) {
		case 1:
		    filler[0] = (char)val; filler[1] = (char)val; filler[2] = (char)val; filler[3] = (char)val;
		    filler[4] = (char)val; filler[5] = (char)val; filler[6] = (char)val; filler[7] = (char)val;
		    filler[8] = (char)val; filler[9] = (char)val; filler[10] = (char)val; filler[11] = (char)val;
		    filler[12] = (char)val; filler[13] = (char)val; filler[14] = (char)val; filler[15] = (char)val;
		    break;
		case 2:
		    *(short*)filler = (short)val; *(short*)(filler + 2) = (short)val;
		    *(short*)(filler + 4) = (short)val; *(short*)(filler + 6) = (short)val;
		    *(short*)(filler + 8) = (short)val; *(short*)(filler + 10) = (short)val;
		    *(short*)(filler + 12) = (short)val; *(short*)(filler + 14) = (short)val;
		    break;
		case 4:
		    *(int*)filler = (int)val; *(int*)(filler + 4) = (int)val;
		    *(int*)(filler + 8) = (int)val; *(int*)(filler + 12) = (int)val;
		    break;
		case 8:
		    *(long long*)filler = (long long)val;
		    *(long long*)(filler + 8) = (long long)val;
		    break;
		default:
		    return 0;
	}
	while(data_ptr < data_end) {
		*(long long*)data_ptr = *(long long*)filler;
		data_ptr += 8;
	}
#endif
	return 1;
}

static int pysimd_vec_fill_float(struct pysimd_vec_t* buf, double val, unsigned char sizer) {
	unsigned char* data_ptr = buf->data;
	const unsigned char* data_end = buf->data + buf->size;
#if defined(PYSIMD_X86_SSE2)
	switch (sizer) {
		case 4:
		    {
		    	__m128 filler = _mm_set1_ps((float)val);
				while (data_ptr < data_end) {
					_mm_store_ps((float*)data_ptr, filler);
					data_ptr += 16;
				}
		    }
		    break;
		case 8:
		    {
		    	__m128d filler = _mm_set1_pd(val);
				while (data_ptr < data_end) {
					_mm_store_pd((double*)data_ptr, filler);
					data_ptr += 16;
				}
		    }
		    break;
		default:
		    return 0;
	}
#else
	switch (sizer) {
		case 4:
		    {
		    	float filler = (float)val;
		    	while (data_ptr < data_end) {
					*(float*)(data_ptr) = filler;
					*(float*)(data_ptr + 4) = filler;
					*(float*)(data_ptr + 8) = filler;
					*(float*)(data_ptr + 12) = filler;
					data_ptr += 16;
				}
		    }
		    break;
		case 8:
		    {
		    	double filler = val;
		    	while (data_ptr < data_end) {
					*(double*)(data_ptr) = filler;
					*(double*)(data_ptr + 8) = filler;
					data_ptr += 16;
				}
		    }
		    break;
		default:
		    return 0;
	}
#endif
	return 1;
}

#endif // SIMD_DATA_OBJECT_H