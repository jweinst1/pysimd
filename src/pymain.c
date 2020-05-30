#include "core_simd.h"
#include <Python.h>

static PyObject* _system_info(PyObject* self, PyObject *args, PyObject *kwds)
{
    PyObject* info_dict;
    PyObject* arch_str;
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

    if (0 != PyDict_SetItemString(info_dict, "arch", arch_str)) {
        goto DICT_ERRCLEAN;
    }
    return info_dict;
DICT_ERRCLEAN:
    Py_XDECREF(info_dict);
    Py_XDECREF(arch_str);
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