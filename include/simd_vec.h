#ifndef SIMD_VEC_H
#define SIMD_VEC_H

#include "core_simd_info.h"

struct pysimd_vec_t {
	size_t len;
	size_t cap;
	uint8_t* data;
};

static inline void pysimd_vec_clear(struct pysimd_vec_t* vec) {
	vec->len = 0;
	vec->cap = 0;
	vec->data = NULL;
}

void pysimd_vec_init(struct pysimd_vec_t* buf, size_t capacity)
{
	buf->len = 0;
	buf->cap = capacity;
	buf->data = malloc(capacity);
}

static inline size_t pysimd_vec_grow(struct pysimd_vec_t* buf)
{
	buf->cap *= buf->cap;
	buf->data = realloc(buf->data, buf->cap);
	return buf->cap;
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