#include "core_simd_info.h"
#include "simd_vec.h"
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#define RETURN_OR_SYS_ERROR(variable) \
    if (variable == NULL) { \
        return PyErr_Format(PyExc_SystemError, "Internal object failure line: %u", __LINE__); \
    }  \
    return variable

typedef struct {
    PyObject_HEAD
    struct pysimd_vec_t vec;
} SimdObject;

extern PyTypeObject SimdObjectType;
static PyObject *SimdError;

static void SimdObject_dealloc(SimdObject* self)
{
    pysimd_vec_deinit(&(self->vec));
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject*
SimdObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SimdObject *self;
    self = (SimdObject*) type->tp_alloc(type, 0);
    if (self != NULL) {
        pysimd_vec_clear(&(self->vec));
    }
    return (PyObject *) self;
}

static int SimdObject_init(SimdObject* self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"capacity", NULL};
    Py_ssize_t param_capacity = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|n", kwlist,
                                     &param_capacity))
        return -1;
    if (param_capacity > 0 && param_capacity % 16 != 0) {
        PyErr_Format(SimdError, "The capacity '%zu' cannot be aligned by at least 16 bytes", param_capacity);
        return -1;
    }
    param_capacity = param_capacity == 0 ? /*default*/ 64 : param_capacity;
    pysimd_vec_init(&(self->vec), (size_t)param_capacity);
    return 0;
}

static PyObject* SimdObject_repr(SimdObject* self)
{
    PyObject* printed = NULL;
    printed = PyUnicode_FromFormat("{\"size\": %zu, \"capacity\": %zu}", self->vec.len, self->vec.cap);
    RETURN_OR_SYS_ERROR(printed);
}

static PyObject *
SimdObject_size(SimdObject *self, PyObject *Py_UNUSED(ignored))
{
    PyObject* size_val = NULL;
    size_val = PyLong_FromSize_t(self->vec.len);
    RETURN_OR_SYS_ERROR(size_val);
}

static PyObject *
SimdObject_capacity(SimdObject *self, PyObject *Py_UNUSED(ignored))
{
    PyObject* cap_val = NULL;
    cap_val = PyLong_FromSize_t(self->vec.cap);
    RETURN_OR_SYS_ERROR(cap_val);
}

static PyObject*
SimdObject_append(SimdObject *self, PyObject *args, PyObject *kwargs)
{
    size_t width_to_use = 0;
    if (kwargs != NULL) {
        PyObject* width_arg = PyDict_GetItemString(kwargs, "width");
        if (width_arg != NULL) {
            if (!PyLong_Check(width_arg)) {
                PyErr_Format(SimdError, "The 'width' option expected an integer, got type '%s'", width_arg->ob_type->tp_name);
                return NULL;
            }
            width_to_use = PyLong_AsSize_t(width_arg);
            if (width_to_use == ((size_t)-1)) {
                // overflow
                return NULL;
            }
            else if (width_to_use < 1 || width_to_use > 8) {
                PyErr_Format(SimdError, "The 'width' option must be between 1 and 8, got '%zu'", width_to_use);
                return NULL;
            }
        }
    }
    // default to full size_t
    width_to_use = width_to_use == 0 ? sizeof(size_t) : width_to_use;
    Py_ssize_t i = 0;
    Py_ssize_t end = PyTuple_Size(args);
    for (;i < end; ++i) {
        PyObject* current = PyTuple_GET_ITEM(args, i);
        if (PyLong_Check(current)) {
            size_t value = PyLong_AsSize_t(current);
            pysimd_vec_push(&(self->vec), &value, width_to_use);
        } else {
            // err handle
            PyErr_Format(SimdError, "The type '%s' is not supported for append", current->ob_type->tp_name);
            return NULL;
        }
    }
    //pysimd_vec_push(self->vec, &input_value, used_width);
    Py_INCREF(Py_None);
    return Py_None;
}


static PyMethodDef SimdObject_methods[] = {
    {"size", (PyCFunction) SimdObject_size, METH_NOARGS,
     "Returns the current size of the vector"
    },
    {"capacity", (PyCFunction) SimdObject_capacity, METH_NOARGS,
     "Returns the current capacity of the vector"
    },
    {"append", (PyCFunction) SimdObject_append, METH_VARARGS | METH_KEYWORDS,
    "Adds data to the end of the vector"
    },
    {NULL}  /* Sentinel */
};

PyTypeObject SimdObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "simd.Vec",
    .tp_doc = "A vector containing simd data",
    .tp_basicsize = sizeof(SimdObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = SimdObject_new,
    .tp_init = (initproc) SimdObject_init,
    .tp_dealloc = (destructor) SimdObject_dealloc,
    .tp_repr = (reprfunc) SimdObject_repr,
    .tp_methods = SimdObject_methods,
};


static PyObject* _system_info(PyObject* self, PyObject *Py_UNUSED(ignored))
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

static PyObject* _simd_verion(PyObject* self, PyObject *Py_UNUSED(ignored))
{
    return Py_BuildValue("III", 0, 0, 1);
}

static PyMethodDef myMethods[] = {
    { "system_info", (PyCFunction)_system_info, METH_NOARGS, 
      "Returns a dictionary containing information on the system architecture and features." 
    },
    { "version", (PyCFunction)_simd_verion, METH_NOARGS, 
      "Returns the version of pysimd." 
    },
    { NULL, NULL, 0, NULL }
};

// Our Module Definition struct
static struct PyModuleDef simdModule = {
    PyModuleDef_HEAD_INIT,
    "simd",
    "The Python SIMD Module",
    -1,
    myMethods
};

// Initializes our module using our above struct
PyMODINIT_FUNC PyInit_simd(void)
{
    PyObject *m;
    if (PyType_Ready(&SimdObjectType) < 0)
        return NULL;

    m = PyModule_Create(&simdModule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&SimdObjectType);
    if (PyModule_AddObject(m, "Vec", (PyObject *) &SimdObjectType) < 0) {
        Py_DECREF(&SimdObjectType);
        Py_DECREF(m);
        return NULL;
    }

    SimdError = PyErr_NewException("simd.SimdError", NULL, NULL);
    Py_XINCREF(SimdError);
    if (PyModule_AddObject(m, "error", SimdError) < 0) {
        Py_XDECREF(SimdError);
        Py_CLEAR(SimdError);
        Py_DECREF(&SimdObjectType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}