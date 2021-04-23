#ifndef CORE_SIMD_INFO_H
#define CORE_SIMD_INFO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


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

// Operating System Detection
#if defined(_WIN32)
#  define PYSIMD_OS_WINDOWS
#elif defined(__linux__) || defined(linux) || defined(__linux)
#  define PYSIMD_OS_LINUX
#elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#  define PYSIMD_OS_MAC
#else
#  define PYSIMD_OS_UNKNOWN
#endif

//Mobile OS detection (android is shared with linux)
#ifdef __ANDROID__
#  define PYSIMD_OSM_ANDROID
#endif // __ANDROID__

#if defined(PYSIMD_ARCH_X86_64)
#  if defined(PYSIMD_OS_WINDOWS)
#    define PYSIMD_X86_CPUID(info, x)    __cpuidex(info, x, 0)
#  elif defined(PYSIMD_CC_GCC)
#    include <cpuid.h>
#    define PYSIMD_X86_CPUID(info, x) __cpuid_count(x, 0, (info)[0], (info)[1], (info)[2], (info)[3])
#  endif
    // intrinsic headers
#   if defined(PYSIMD_CC_GCC) || defined(PYSIMD_CC_CLANG)
#      include <x86intrin.h>
#   else // defined(PYSIMD_CC_GCC) || defined(PYSIMD_CC_CLANG)
#      include <immintrin.h>
#   endif // !defined(PYSIMD_CC_GCC) || defined(PYSIMD_CC_CLANG)
#endif

// Exact width integer types
#if defined(PYSIMD_OS_WINDOWS) && defined(_MSC_VER)
    typedef __int8 int8_t;
    typedef __int16 int16_t;
    typedef __int32 int32_t;
    typedef __int64 int64_t;
    typedef unsigned __int8 uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int64 uint64_t;
#else
#     include <stdint.h>
#endif

enum pysimd_arch {
    PYSIMD_ARCH_TYPE_UNKNOWN,
    PYSIMD_ARCH_TYPE_X86,
    PYSIMD_ARCH_TYPE_ARM,
    PYSIMD_ARCH_TYPE_MIPS,
    PYSIMD_ARCH_TYPE_SPARC
};

static const char* pysimd_arch_stringify(enum pysimd_arch arch)
{
    static const char* pysimd_x86 = "x86";
    static const char* pysimd_arm = "arm";
    static const char* pysimd_mips = "mips";
    static const char* pysimd_sparc = "sparc";
    static const char* pysimd_unknown = "unknown";
    
    switch (arch) {
        case PYSIMD_ARCH_TYPE_X86: return pysimd_x86;
        case PYSIMD_ARCH_TYPE_ARM: return pysimd_arm;
        case PYSIMD_ARCH_TYPE_MIPS: return pysimd_mips;
        case PYSIMD_ARCH_TYPE_SPARC: return pysimd_sparc;
        case PYSIMD_ARCH_TYPE_UNKNOWN: return pysimd_unknown;
        default:
            return pysimd_unknown;
    }
}

enum pysimd_cc {
    PYSIMD_CC_TYPE_UNKNOWN,
    PYSIMD_CC_TYPE_GCC,
    PYSIMD_CC_TYPE_MSVC,
    PYSIMD_CC_TYPE_CLANG
};

static const char* pysimd_cc_stringify(enum pysimd_cc cc)
{
    static const char* pysimd_unknown = "unknown";
    static const char* pysimd_gcc = "gcc";
    static const char* pysimd_clang = "clang";
    static const char* pysimd_msvc = "msvc";

    switch (cc) {
        case PYSIMD_CC_TYPE_GCC: return pysimd_gcc;
        case PYSIMD_CC_TYPE_CLANG: return pysimd_clang;
        case PYSIMD_CC_TYPE_MSVC: return pysimd_msvc;
        default:
            return pysimd_unknown;
    }
}

#ifdef PYSIMD_ARCH_X86_64
struct pysimd_x86_features {
    int mmx;
    int popcnt;
    int sse;
    int sse2;
    int sse3;
    int ssse3;
    int sse41;
    int sse42;
    int sse4a;
    int avx;
    int avx2;
    int fma;
    int fma4;
    int xop;
    int bmi;
    int bmi2;
    int avx512f;
    int avx512vl;
    int avx512bw;
    int avx512dq;
    int avx512cd;
    int avx512pf;
    int avx512er;
    int avx512ifma;
    int avx512vbmi;
};

int pysimd_x86_features_init(struct pysimd_x86_features* feat)
{
#ifdef PYSIMD_CC_GCC
    #define FORMAT_CPU_FEATURE(name) feat->name = __builtin_cpu_supports(#name)
    FORMAT_CPU_FEATURE(mmx);
    FORMAT_CPU_FEATURE(sse);
    FORMAT_CPU_FEATURE(sse2);
    FORMAT_CPU_FEATURE(sse3);
    FORMAT_CPU_FEATURE(ssse3);
    feat->sse41 = __builtin_cpu_supports("sse4.1");
    feat->sse42 = __builtin_cpu_supports("sse4.2");
    FORMAT_CPU_FEATURE(sse4a);
    FORMAT_CPU_FEATURE(avx);
    FORMAT_CPU_FEATURE(avx2);
    FORMAT_CPU_FEATURE(fma);
    FORMAT_CPU_FEATURE(fma4);
    FORMAT_CPU_FEATURE(xop);
    FORMAT_CPU_FEATURE(bmi);
    FORMAT_CPU_FEATURE(bmi2);
    FORMAT_CPU_FEATURE(avx512f);
    FORMAT_CPU_FEATURE(avx512vl);
    FORMAT_CPU_FEATURE(avx512bw);
    FORMAT_CPU_FEATURE(avx512dq);
    FORMAT_CPU_FEATURE(avx512cd);
    FORMAT_CPU_FEATURE(avx512er);
    FORMAT_CPU_FEATURE(avx512pf);
    FORMAT_CPU_FEATURE(avx512ifma);
    FORMAT_CPU_FEATURE(avx512vbmi);
    #undef FORMAT_CPU_FEATURE
    return 1;
#elif defined(PYSIMD_CC_MSVC)
    int infos[4];
    PYSIMD_X86_CPUID(infos, 0);
    int nIds = infos[0];

    PYSIMD_X86_CPUID(infos, 0x80000000);
    unsigned nExIds = (unsigned)infos[0];

    //  Detect Features
    if (nIds >= 0x00000001){
        PYSIMD_X86_CPUID(infos,0x00000001);
        feat->mmx    = (infos[3] & ((int)1 << 23)) != 0;
        feat->sse    = (infos[3] & ((int)1 << 25)) != 0;
        feat->sse2   = (infos[3] & ((int)1 << 26)) != 0;
        feat->sse3   = (infos[2] & ((int)1 <<  0)) != 0;
        feat->ssse3  = (infos[2] & ((int)1 <<  9)) != 0;
        feat->sse41  = (infos[2] & ((int)1 << 19)) != 0;
        feat->sse42  = (infos[2] & ((int)1 << 20)) != 0;
        feat->popcnt = (infos[2] & ((int)1 << 23)) != 0;
        //info->HW_AES    = (infos[2] & ((int)1 << 25)) != 0;

        feat->avx    = (infos[2] & ((int)1 << 28)) != 0;
        feat->fma   = (infos[2] & ((int)1 << 12)) != 0;

        //info->HW_RDRAND = (infos[2] & ((int)1 << 30)) != 0;
    }
    if (nIds >= 0x00000007){
        PYSIMD_X86_CPUID(infos,0x00000007);
        feat->avx2   = (infos[1] & ((int)1 <<  5)) != 0;

        feat->bmi        = (infos[1] & ((int)1 <<  3)) != 0;
        feat->bmi2       = (infos[1] & ((int)1 <<  8)) != 0;
        //info->HW_PREFETCHWT1 = (infos[2] & ((int)1 <<  0)) != 0;

        feat->avx512f     = (infos[1] & ((int)1 << 16)) != 0;
        feat->avx512cd   = (infos[1] & ((int)1 << 28)) != 0;
        feat->avx512pf    = (infos[1] & ((int)1 << 26)) != 0;
        feat->avx512er    = (infos[1] & ((int)1 << 27)) != 0;
        feat->avx512vl    = (infos[1] & ((int)1 << 31)) != 0;
        feat->avx512bw   = (infos[1] & ((int)1 << 30)) != 0;
        feat->avx512dq    = (infos[1] & ((int)1 << 17)) != 0;
        feat->avx512ifma  = (infos[1] & ((int)1 << 21)) != 0;
        feat->avx512vbmi  = (infos[2] & ((int)1 <<  1)) != 0;
    }
    if (nExIds >= 0x80000001){
        PYSIMD_X86_CPUID(infos,0x80000001);
        //info->HW_ABM   = (infos[2] & ((int)1 <<  5)) != 0;
        feat->sse4a = (infos[2] & ((int)1 <<  6)) != 0;
        feat->fma4  = (infos[2] & ((int)1 << 16)) != 0;
        feat->xop   = (infos[2] & ((int)1 << 11)) != 0;
    } 
    return 1;
#else
    // Unsupported x86
    fprintf(stderr, "WARN %s\n", "Compiler does not support cpuid feature detection\n");
    return 0;
#endif
}
#endif // PYSIMD_ARCH_X86_64

struct pysimd_sys_info {
    enum pysimd_arch arch;
    enum pysimd_cc compiler;
#ifdef PYSIMD_ARCH_X86_64
    struct pysimd_x86_features features;
#endif // PYSIMD_ARCH_X86_64
};

static void pysimd_sys_info_init(struct pysimd_sys_info* sinfo)
{
    #if defined(PYSIMD_ARCH_X86_64)
        sinfo->arch = PYSIMD_ARCH_TYPE_X86;
        (void)pysimd_x86_features_init(&(sinfo->features));
    #elif defined(PYSIMD_ARCH_ARM_64)
        sinfo->arch = PYSIMD_ARCH_TYPE_ARM;
    #elif defined(PYSIMD_ARCH_MIPS)
       sinfo->arch = PYSIMD_ARCH_TYPE_MIPS;
    #elif defined(PYSIMD_ARCH_SPARC)
       sinfo->arch = PYSIMD_ARCH_TYPE_SPARC;
    #else
       sinfo->arch = PYSIMD_ARCH_TYPE_UNKNOWN;
    #endif
    
    #if defined(PYSIMD_CC_MSVC)
       sinfo->compiler = PYSIMD_CC_TYPE_MSVC;
    #elif defined(PYSIMD_CC_GCC)
       sinfo->compiler = PYSIMD_CC_TYPE_GCC;
    #elif defined(PYSIMD_CC_CLANG)
       sinfo->compiler = PYSIMD_CC_TYPE_CLANG;
    #else
        sinfo->compiler = PYSIMD_CC_TYPE_UNKNOWN;
    #endif
}

#endif // CORE_SIMD_INFO_H