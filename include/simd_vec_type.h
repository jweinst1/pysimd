#ifndef SIMD_VEC_TYPE_H
#define SIMD_VEC_TYPE_H

#include "core_simd_info.h"

struct pysimd_vec_t {
	size_t len;
	size_t cap;
	uint8_t* data;
};

#endif // SIMD_VEC_TYPE_H