#ifndef CORE_SIMD_H
#define CORE_SIMD_H

#ifdef __GNUC__
#  define PYSIMD_CC_GCC
#endif // __GNUC__

#ifdef _MSC_VER
#  define PYSIMD_CC_MSVC
#endif // _MSC_VER

#ifdef __clang__
#  define PYSIMD_CC_CLANG
#endif // __clang__

// Architecture detection
#if (defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64) || defined(_AMD64_))
#   define PYSIMD_ARCH_X86_64
#elif defined(__arm__) && defined(__aarch64__)
#   define PYSIMD_ARCH_ARM_64
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__)
#   define PYSIMD_ARCH_MIPS
#elif defined(__sparc) || defined(__sparc__)
#   define PYSIMD_ARCH_SPARC
#else
#   define PYSIMD_ARCH_UNKNOWN
#endif

enum pysimd_arch {
    PYSIMD_ARCH_TYPE_UNKNOWN,
    PYSIMD_ARCH_TYPE_X86,
    PYSIMD_ARCH_TYPE_ARM,
    PYSIMD_ARCH_TYPE_MIPS,
    PYSIMD_ARCH_TYPE_SPARC
};

struct pysimd_cpu_info {
    enum pysimd_arch arch;
};

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#endif // CORE_SIMD_H