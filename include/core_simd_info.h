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

//Mobile OS detection (android is shared wiht linux)
#ifdef __ANDROID__
#  define PYSIMD_OSM_ANDROID
#endif // __ANDROID__

#if defined(PYSIMD_ARCH_X86_64)
#  if defined(PYSIMD_OS_WINDOWS)
#    define PYSIMD_X86_CPUID(info, x)    __cpuidex(info, x, 0)
#  elif defined(PYSIMD_CC_GCC)
#    include <cpuid.h>
#    define PYSIMD_X86_CPUID(info, x) __cpuid_count(x, 0, info[0], info[1], info[2], info[3])
#  endif
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

struct pysimd_sys_info {
    enum pysimd_arch arch;
    enum pysimd_cc compiler;
};

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static void pysimd_sys_info_init(struct pysimd_sys_info* sinfo)
{
    #if defined(PYSIMD_ARCH_X86_64)
        sinfo->arch = PYSIMD_ARCH_TYPE_X86;
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

#endif // CORE_SIMD_H