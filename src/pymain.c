#include "core_simd_info.h"
#include <Python.h>

static PyObject* _system_info(PyObject* self, PyObject *args, PyObject *kwds)
{
    PyObject* info_dict = NULL;
    PyObject* arch_str = NULL;
    PyObject* cc_str = NULL;
    PyObject* features_dict = NULL;
    struct pysimd_sys_info sinfo;
    pysimd_sys_info_init(&sinfo);
    info_dict = PyDict_New();
    if (info_dict == NULL) {
        // Can't allocate for dict
        return NULL;
    }
    arch_str = PyUnicode_FromString(pysimd_arch_stringify(sinfo.arch));
    if (arch_str == NULL) {
        goto DICT_ERRCLEAN;
    }

    cc_str = PyUnicode_FromString(pysimd_cc_stringify(sinfo.compiler));
    if (cc_str == NULL) {
        goto DICT_ERRCLEAN;
    }

    if (0 != PyDict_SetItemString(info_dict, "arch", arch_str)) {
        goto DICT_ERRCLEAN;
    }
    Py_DECREF(arch_str);
    if (0 != PyDict_SetItemString(info_dict, "compiler", cc_str)) {
        goto DICT_ERRCLEAN;
    }
    Py_DECREF(cc_str);

    features_dict = PyDict_New();
    if (features_dict == NULL) {
        goto DICT_ERRCLEAN;
    }
#ifdef PYSIMD_ARCH_X86_64
    #define X86_PYDICT_SETTER(ftname) \
         if (0 != PyDict_SetItemString(features_dict, #ftname, PyBool_FromLong(sinfo.features.ftname))) { \
             goto DICT_ERRCLEAN; \
        }                 
    X86_PYDICT_SETTER(mmx)
    X86_PYDICT_SETTER(popcnt)
    X86_PYDICT_SETTER(sse)
    X86_PYDICT_SETTER(sse2)
    X86_PYDICT_SETTER(sse3)
    X86_PYDICT_SETTER(ssse3)
    X86_PYDICT_SETTER(sse41)
    X86_PYDICT_SETTER(sse42)
    X86_PYDICT_SETTER(sse4a)
    X86_PYDICT_SETTER(avx)
    X86_PYDICT_SETTER(avx2)
    X86_PYDICT_SETTER(fma)
    X86_PYDICT_SETTER(fma4)
    X86_PYDICT_SETTER(xop)
    X86_PYDICT_SETTER(bmi)
    X86_PYDICT_SETTER(bmi2)
    X86_PYDICT_SETTER(avx512f)
    X86_PYDICT_SETTER(avx512vl)
    X86_PYDICT_SETTER(avx512bw)
    X86_PYDICT_SETTER(avx512dq)
    X86_PYDICT_SETTER(avx512cd)
    X86_PYDICT_SETTER(avx512pf)
    X86_PYDICT_SETTER(avx512er)
    X86_PYDICT_SETTER(avx512ifma)
    X86_PYDICT_SETTER(avx512vbmi)
    #undef X86_PYDICT_SETTER
#endif // PYSIMD_ARCH_X86_64
    if (0 != PyDict_SetItemString(info_dict, "features", features_dict)) {
        goto DICT_ERRCLEAN;
    }
    Py_DECREF(features_dict);
    return info_dict;
DICT_ERRCLEAN:
    Py_XDECREF(info_dict);
    Py_XDECREF(arch_str);
    Py_XDECREF(cc_str);
    Py_XDECREF(features_dict);
    return NULL;
}

static PyMethodDef myMethods[] = {
    { "system_info", (PyCFunction)_system_info, METH_VARARGS | METH_KEYWORDS, 
      "Returns a dictionary containing information on the system architecture and features." 
    },
    { NULL, NULL, 0, NULL }
};

// Our Module Definition struct
static struct PyModuleDef myModule = {
    PyModuleDef_HEAD_INIT,
    "simd",
    "The Python SIMD Module",
    -1,
    myMethods
};

// Initializes our module using our above struct
PyMODINIT_FUNC PyInit_simd(void)
{
    return PyModule_Create(&myModule);
}