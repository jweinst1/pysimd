from distutils.core import setup, Extension
from distutils.ccompiler import get_default_compiler
import os
from check_c_compiles import CheckCCompiles

DEFAULT_COMPILER = get_default_compiler()

pysimd_patch_version = 4
pysimd_minor_version = 0
pysimd_major_version = 0

pysimd_version = [pysimd_major_version, 
                  pysimd_minor_version, 
                  pysimd_patch_version]

keyword_list = [
    'simd',
    'x86',
    'arm',
    'sse2',
    'avx',
    'avx512',
    'performance',
    'big data',
    'data science'
]

classifers_list = [
    'Development Status :: 2 - Pre-Alpha',
    'Operating System :: OS Independent',
    'Programming Language :: C',
    'Programming Language :: Python',
    'Topic :: Scientific/Engineering',
    'Topic :: System :: Hardware'
]

macro_defs = [
    ('SIMDPY_VERSION_MAJOR', str(pysimd_major_version)),
    ('SIMDPY_VERSION_MINOR', str(pysimd_minor_version)),
    ('SIMDPY_VERSION_PATCH', str(pysimd_patch_version))
]

# extra_compile_args
compiler_flags = []

x86_header_string = """
   #ifdef _WIN32
   #  include <immintrin.h>
   #else
   #  include <x86intrin.h>
   #endif
"""

with CheckCCompiles("sse2", x86_header_string + """
   int main(void) {
    __m128i foo = _mm_set1_epi8(8);
    __m128i new_vec = _mm_add_epi8(foo, _mm_setzero_si128());
    (void)new_vec;
    return 0;
}

""") as sse2_test:
  if sse2_test:
    macro_defs.append(('PYSIMD_X86_SSE2', '1'))

with CheckCCompiles("sse3", x86_header_string + """
   int main(void) {
    float lst[4] = {1.0, 2.0, 3.0, 4.0};
    __m128 lstv = _mm_load_ps((float const*)lst);
    __m128 hadded = _mm_hadd_ps(lstv, lstv);
    (void)hadded;
    return 0;
}
""") as sse3_test:
  if sse3_test:
    macro_defs.append(('PYSIMD_X86_SSE3', '1'))
    if DEFAULT_COMPILER == 'unix':
      compiler_flags.append('-msse3')

with CheckCCompiles("ssse3", x86_header_string + """

   #include <assert.h>

   int main(void) {
    unsigned char nums[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    __m128i loaded = _mm_load_si128((__m128i const*)nums);
    __m128i direction = _mm_set1_epi8(1);
    __m128i shuffled = _mm_shuffle_epi8(loaded, direction);
    _mm_store_si128((__m128i*)nums, shuffled);
    assert(nums[0] == 2);
    return 0;
} 
""") as ssse3_test:
  if ssse3_test:
    macro_defs.append(('PYSIMD_X86_SSSE3', '1'))
    if DEFAULT_COMPILER == 'unix':
      compiler_flags.append('-mssse3')

if os.name == 'nt':
  macro_defs.append(('_CRT_SECURE_NO_WARNINGS', '1'))

if DEFAULT_COMPILER == 'unix':
  # shut off not so useful warnings
  compiler_flags.append('-Wno-sign-compare')

# A Python package may have multiple extensions, but this
# template has one.
module1 = Extension('simd',
                    define_macros = macro_defs,
                    include_dirs = ['include'],
                    sources = ['src/pymain.c'],
                    extra_compile_args=compiler_flags)

setup (name = 'simd',
       version = ".".join([str(elem) for elem in pysimd_version]),
       description = 'The SIMD Python Module',
       author = 'Joshua Weinstein',
       author_email = 'jweinst1@berkeley.edu',
       url = 'https://github.com/jweinst1/pysimd',
       license = 'MIT',
       keywords = keyword_list,
       classifiers = classifers_list,
       long_description = open('README.rst').read(),
       ext_modules = [module1])