#ifndef SIMD_VEC_H
#define SIMD_VEC_H

#include "simd_vec_type.h"

static inline void pysimd_vec_clear(struct pysimd_vec_t* vec) {
	vec->len = 0;
	vec->cap = 0;
	vec->data = NULL;
}

void pysimd_vec_init(struct pysimd_vec_t* buf, size_t capacity)
{
	buf->len = 0;
	buf->cap = capacity;
	buf->data = calloc(1, capacity);
}

static inline size_t pysimd_vec_grow(struct pysimd_vec_t* buf)
{
	buf->cap *= (buf->cap / 2);
	buf->data = realloc(buf->data, buf->cap);
	return buf->cap;
}

static int pysimd_vec_resize(struct pysimd_vec_t* buf, size_t new_size) {
	if (new_size == 0)
		return 0;
	buf->cap = new_size;
	buf->len = buf->len > buf-> cap ? buf->cap : buf->len;
	buf->data = realloc(buf->data, new_size);
	return 1;
}

static inline void pysimd_vec_push(struct pysimd_vec_t* buf, void* items, size_t size)
{
	while (size > (buf->cap - buf->len)) {
		pysimd_vec_grow(buf);
	}
	memcpy(buf->data + buf->len, items, size);
	buf->len += size;
}

void pysimd_vec_deinit(struct pysimd_vec_t* buf)
{
	buf->cap = 0;
	free(buf->data);
}

#endif // SIMD_DATA_OBJECT_H