from distutils.core import setup, Extension
import os

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
    'Operating System :: OS Independent'
    'Programming Language :: C',
    'Programming Language :: Python',
    'Topic :: Scientific/Engineering',
    'Topic :: System :: Hardware'
]

macro_defs = []

if os.name == 'nt':
  macro_defs.append(('_CRT_SECURE_NO_WARNINGS', '1'))

# A Python package may have multiple extensions, but this
# template has one.
module1 = Extension('simd',
                    define_macros = macro_defs,
                    include_dirs = ['include'],
                    sources = ['src/pymain.c'])

setup (name = 'DemoPackage',
       version = '0.0.1',
       description = 'The SIMD Python Module',
       author = 'Joshua Weinstein',
       author_email = 'jweinst1@berkeley.edu',
       url = 'https://github.com/jweinst1/pysimd',
       keywords = keyword_list,
       classifiers = classifers_list,
       long_description = open('README.md').read(),
       ext_modules = [module1])