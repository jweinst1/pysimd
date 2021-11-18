#ifndef SIMD_VEC_ARITH_H
#define SIMD_VEC_ARITH_H

#include "simd_vec_type.h"

int simd_vec_add_(struct pysimd_vec_t* v1, const struct pysimd_vec_t* v2) {
#if defined(PYSIMD_X86_SSE2)
	// foo
#else
	//foo
#endif
}

#endif // SIMD_VEC_ARITH_H
