import sys
import os
import shutil
import subprocess
import unittest
import distutils.ccompiler
import sysconfig
from distutils.spawn import find_executable

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
BUILT_TEST_DIR = os.path.join(CURRENT_DIR, 'bin')
PROJECT_DIR = os.path.dirname(CURRENT_DIR)
BUILD_DIR = os.path.join(PROJECT_DIR, 'build')
BINPY_DIR = os.path.dirname(sys.executable)

os.makedirs(BUILT_TEST_DIR, exist_ok=True)

if not os.path.isdir(BUILD_DIR):
	try:
		import simd
		print("Testing extension already importable in " + CURRENT_DIR)
	except Exception as exc:
		raise Exception("Cannot find directory with built C extension, nor is it importable: " + str(exc))
else:
	lib_paths = [path for path in os.listdir(BUILD_DIR) if path.startswith('lib.')]
	extend_dir = os.path.join(BUILD_DIR, lib_paths[0])
	extensions = [path for path in os.listdir(extend_dir) if path.startswith('simd')]
	if len(extensions) > 0:
		testing_ext = os.path.join(BUILT_TEST_DIR, extensions[0])
		shutil.copy(os.path.join(extend_dir, extensions[0]), testing_ext)
		print("Testing extension at setup path " + testing_ext)
	else:
		raise Exception("Found no extensions in " + extend_dir)

# Try to import extension
try:
	import simd
except Exception as exc:
	raise("Could not import extension, reason: " + str(exc))

compiler_includes_and_libs = sysconfig.get_config_vars('INCLUDEPY', 'LIBPL', 'LIBRARY')
if len(compiler_includes_and_libs) != 3:
	raise Exception("Missing or unexpected system compiler configuration: " + str(compiler_includes_and_libs))
(cc_includes, cc_lib_dirs, cc_libs) = compiler_includes_and_libs

if cc_libs.startswith("lib"):
	cc_libs = cc_libs[3:]
if cc_libs.endswith(".a"):
	cc_libs = cc_libs[:-2]

py3_embed_cc = distutils.ccompiler.new_compiler()

current_dir_cfiles = [os.path.join(CURRENT_DIR, path) for path in os.listdir(CURRENT_DIR) if path.endswith(".c")]
built_tests = []

for cfile in current_dir_cfiles:
	cfile_dir = os.path.dirname(cfile)
	built_name = os.path.join(cfile_dir, 'bin', os.path.basename(cfile)[:-2])
	print("Building: " + cfile)
	obj_file = py3_embed_cc.compile([cfile], include_dirs=[cc_includes], macros=[('TESTING_BIN_PATH', "\"" + cfile_dir + "\"")])
	py3_embed_cc.link_executable(obj_file, library_dirs=[cc_lib_dirs], libraries=[cc_libs], 
		                         output_progname=built_name)
	built_tests.append(built_name)

# Run the tests
tests_passed = []
tests_failed = []

for cmdtest in built_tests:
	try:
		result = subprocess.run(cmdtest, check=False, timeout=600)
		if result.returncode == 0:
			tests_passed.append(cmdtest)
		else:
			print("Test {} FAILED with return_code {}".format(cmdtest, str(result.returncode)))
			tests_failed.append(cmdtest)
	except subprocess.TimeoutExpired as exc:
		print("Test {} FAILED due to timeout error: {}".format(cmdtest, str(exc)))
		tests_failed.append(cmdtest)
	except Exception as exc:
		print("Test {} FAILED due to error: {}".format(cmdtest, str(exc)))
		tests_failed.append(cmdtest)

print("------------------------------------------------------")
print("{} tests failed out of {} total tests".format(len(tests_failed), len(built_tests)))
print("------------------------------------------------------")
