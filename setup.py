from distutils.core import setup, Extension
import os
from check_c_compiles import CheckCCompiles

pysimd_patch_version = 2
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


if os.name == 'nt':
  macro_defs.append(('_CRT_SECURE_NO_WARNINGS', '1'))

# A Python package may have multiple extensions, but this
# template has one.
module1 = Extension('simd',
                    define_macros = macro_defs,
                    include_dirs = ['include'],
                    sources = ['src/pymain.c'])

setup (name = 'simd',
       version = ".".join([str(elem) for elem in pysimd_version]),
       description = 'The SIMD Python Module',
       author = 'Joshua Weinstein',
       author_email = 'jweinst1@berkeley.edu',
       url = 'https://github.com/jweinst1/pysimd',
       license = 'MIT',
       keywords = keyword_list,
       classifiers = classifers_list,
       long_description_content_type="text/markdown",
       long_description = open('README.md').read(),
       ext_modules = [module1])