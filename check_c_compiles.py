import distutils.ccompiler
import os
import random
import subprocess

"""
These classes allow a test to see if source code with the C compiler actually
compiles.
"""

DEFAULT_COMPILER = distutils.ccompiler.get_default_compiler()

C_EXTENSION = ".c"

def create_file_with_rand_name(source):
    cur_dir = os.getcwd()
    rand_file = os.path.join(cur_dir, "c_" + str(random.getrandbits(72)))
    while os.path.exists(rand_file):
        rand_file = os.path.join(cur_dir, "c_" + str(random.getrandbits(72)))
    with open(rand_file + C_EXTENSION, "w") as c_file:
        c_file.write(source)
    return rand_file

class CheckCCompiles(object):

    def __init__(self, name = "", source_code = ""):
        self.name = name
        self.source_code = source_code
        self.compiler = distutils.ccompiler.new_compiler()
        if DEFAULT_COMPILER == 'unix':
            # The idea here is that we want to have the compiler try and generate all the possible
            # simd instructions, then see by running it, if we get an illegal hardware instruction
            self.extra_args = ["-m" + self.name]
        elif DEFAULT_COMPILER == 'msvc':
            self.extra_args = ['/arch:AVX', '/arch:AVX2', '/arch:AVX512']
        else:
            self.extra_args = []
        self.works = False

    def try_run(self):
        try:
            self.run_result = subprocess.run(self.file_name, check=False)
            self.works = self.run_result.returncode == 0
        except Exception:
            self.works = False
        return self.works
        
    def __enter__(self):
        self.file_name = create_file_with_rand_name(self.source_code)
        self.c_name = self.file_name + C_EXTENSION
        try:
            self.obj_names = self.compiler.compile([self.c_name], extra_preargs=self.extra_args)
        except Exception as exc:
            print("FAILED " + self.name + " compile check: " + str(exc))
            return self
        self.compiles = True
        try:
            self.compiler.link_executable(self.obj_names, self.file_name)
        except Exception as exc:
            print("FAILED " + self.name + " link check: " + str(exc))
            return self
        self.links = True
        if self.try_run():
            print("PASSED " + self.name)
        else:
            print("FAILED " + self.name + " run check: " + str(self.run_result.stderr))
        return self
        
    def __exit__(self, exc_type, exc_val, exc_tb):
        try:
            os.remove(self.c_name)
            if os.name == 'nt':
                os.remove(self.file_name + ".exe")
            else:
                os.remove(self.file_name)
            for objfile in self.obj_names:
                os.remove(objfile)
        except Exception as exc:
            # Avoid noise for non existant files
            return