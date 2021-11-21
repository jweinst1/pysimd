import distutils.ccompiler
import os
import random

"""
These classes allow a test to see if source code with the C compiler actually
compiles.
"""

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
        self.works = False
        
    def __enter__(self):
        self.file_name = create_file_with_rand_name(self.source_code)
        self.c_name = self.file_name + C_EXTENSION
        try:
            self.obj_names = self.compiler.compile([self.c_name])
        except Exception as exc:
            print("FAILED " + self.name + " compile check: " + str(exc))
            return self.works
        try:
            self.compiler.link_executable(self.obj_names, self.file_name)
        except Exception as exc:
            print("FAILED " + self.name + " link check: " + str(exc))
            return self.works
        self.works = True
        print("PASSED " + self.name)
        return self.works
        
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