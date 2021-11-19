#ifndef PYSIMD_VEC_MACROS_H
#define PYSIMD_VEC_MACROS_H


#define PYSIMD_MIN_VEC_SIZE(v1, v2) (((v1)->size) < ((v2)->size)) ? ((v1)->size) : ((v2)->size)


#endif // PYSIMD_VEC_MACROS_H
