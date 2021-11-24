import sys
import os
import shutil
import subprocess

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(CURRENT_DIR)
BUILD_DIR = os.path.join(PROJECT_DIR, 'build')

if not os.path.isdir(BUILD_DIR):
	try:
		import simd
	except Exception as exc:
		raise Exception("Cannot find directory with built C extension, nor is it importable")
else:
	lib_paths = [path for path in os.listdir(BUILD_DIR) if path.startswith('lib.')]
	extend_dir = os.path.join(BUILD_DIR, lib_paths[0])
	extensions = [path for path in os.listdir(extend_dir) if path.startswith('simd')]
	if len(extensions) > 0:
		testing_ext = os.path.join(CURRENT_DIR, extensions[0])
		shutil.copy(os.path.join(extend_dir, extensions[0]), testing_ext)
		print("Testing extension at setup path " + testing_ext)
	else:
		raise Exception("Found no extensions in " + extend_dir)

print("Running tests in " + CURRENT_DIR)