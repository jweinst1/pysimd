#include "core_simd_info.h"
#include "simd_vec.h"
#include "simd_vec_arith.h"
//#include "simd_vec_filter.h"
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
    static char *kwlist[] = {"size", "repeat_value", "repeat_size", NULL};
    Py_ssize_t param_size = 0;
    PyObject* param_rep_val = NULL;
    unsigned char param_rep_size = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|nOb", kwlist,
                                     &param_size, &param_rep_val, &param_rep_size))
        return -1;
    if (param_size > 0 && param_size % 16 != 0) {
        PyErr_Format(SimdError, "The size '%zu' cannot be aligned by at least 16 bytes", (size_t)param_size);
        return -1;
    }
    param_size = param_size == 0 ? /*default*/ 64 : param_size;
    pysimd_vec_init(&(self->vec), (size_t)param_size);
    if (param_rep_val != NULL && param_rep_size != 0) {
        if (PyLong_Check(param_rep_val)) {
            size_t rep_value = PyLong_AsSize_t(param_rep_val);
            if (!pysimd_vec_fill(&(self->vec), rep_value, param_rep_size)) {
                PyErr_Format(SimdError, "Invalid repeat parameters, value: %zu, size: %u", rep_value, param_rep_size);
                return -1;
            }
        } else if (PyFloat_Check(param_rep_val)) {
            double rep_value = PyFloat_AsDouble(param_rep_val);
            if (!pysimd_vec_fill_float(&(self->vec), rep_value, param_rep_size)) {
                PyErr_Format(SimdError, "Invalid repeat parameters, value: %f, size: %u", rep_value, param_rep_size);
                return -1;
            }
        } else {
            PyErr_Format(SimdError, "The type '%s' is not supported for 'repeat_value' option", param_rep_val->ob_type->tp_name);
            return -1;
        }
    }
    return 0;
}

static PyObject*
SimdObject_copy(SimdObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"start", "end", NULL};
    Py_ssize_t param_start = -1;
    Py_ssize_t param_end = -1;
    size_t actual_start = 0;
    size_t actual_end = self->vec.size;
    PyObject* copied = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|nn", kwlist,
                                     &param_start, &param_end)) {
        return NULL;
    }

    if (param_start > -1) {
        if (param_start >= self->vec.size) {
            PyErr_Format(SimdError, "'start' option: %ld is out of bounds", param_start);
            return NULL;
        }
        actual_start = (size_t)param_start;
    }

    if (param_end > -1) {
        if (param_end >= self->vec.size || param_end <= param_start) {
            PyErr_Format(SimdError, "'end' option: %ld is out of bounds", param_start);
            return NULL;
        }
        actual_end = (size_t)param_end;
    }

    // Alignment check
    if ((actual_end - actual_start) % 16 != 0) {
        PyErr_Format(SimdError, "requested copy size: %zu is not aligned on a 16 byte boundary", actual_end - actual_start);
        return NULL;
    }
    copied = SimdObjectType.tp_alloc(&SimdObjectType, 0);
    if (copied == NULL) {
        PyErr_Format(PyExc_SystemError, "Internal object failure line: %u", __LINE__);
        return NULL;
    }
    if (!pysimd_vec_copy( &((SimdObject*)copied)->vec, &self->vec, actual_start, actual_end)) {
        PyErr_SetString(SimdError, "Internal vector copy failure");
        SimdObject_dealloc((SimdObject*)copied);
        return NULL;
    }
    return copied;
}

static PyObject* SimdObject_repr(SimdObject* self)
{
    char* representation = pysimd_vec_repr(&(self->vec));
    PyObject* printed = NULL;
    printed = PyUnicode_FromString(representation);
    free(representation);
    RETURN_OR_SYS_ERROR(printed);
}

static PyObject *
SimdObject_size(SimdObject *self, PyObject *Py_UNUSED(ignored))
{
    PyObject* size_val = NULL;
    size_val = PyLong_FromSize_t(self->vec.size);
    RETURN_OR_SYS_ERROR(size_val);
}

static PyObject*
SimdObject_resize(SimdObject *self, PyObject *args, PyObject *kwargs)
{
    Py_ssize_t resize_to = 0;
    PyObject* size_val = NULL;
    if (!PyArg_ParseTuple(args, "n", &resize_to)) {
        return NULL;
    }
    if (resize_to == 0) {
        PyErr_SetString(SimdError, "vector cannot be resized to 0");
        return NULL;
    } else if (resize_to % 16 != 0) {
        PyErr_SetString(SimdError, "vector can only be resized to 16-byte aligned size");
        return NULL;
    }
    pysimd_vec_resize(&self->vec, (size_t)resize_to);
    size_val = PyLong_FromSize_t(self->vec.size);
    RETURN_OR_SYS_ERROR(size_val);
}

static PyObject*
SimdObject_add(SimdObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"other", "width", NULL};
    Py_ssize_t param_width = 0;
    PyObject* param_other = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "On", kwlist,
                                     &param_other, &param_width)) {
        return NULL;
    }

    if (param_other->ob_type != &SimdObjectType) {
        PyErr_Format(SimdError, "Expected vector, got type '%s'", param_other->ob_type->tp_name);
        return NULL;
    }

    switch (param_width) {
        case 1:
            simd_vec_add_i8(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        case 2:
            simd_vec_add_i16(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        case 4:
            simd_vec_add_i32(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        case 8:
            simd_vec_add_i64(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        default:
            PyErr_Format(SimdError, "Unrecognized width: %zu for add operation", (size_t)param_width);
            return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject*
SimdObject_fadd(SimdObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"other", "width", NULL};
    Py_ssize_t param_width = 0;
    PyObject* param_other = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "On", kwlist,
                                     &param_other, &param_width)) {
        return NULL;
    }

    if (param_other->ob_type != &SimdObjectType) {
        PyErr_Format(SimdError, "Expected vector, got type '%s'", param_other->ob_type->tp_name);
        return NULL;
    }

    switch (param_width) {
        case 4:
            simd_vec_add_f32(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        case 8:
            simd_vec_add_f64(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        default:
            PyErr_Format(SimdError, "Unrecognized width: %zu for fadd operation", (size_t)param_width);
            return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject*
SimdObject_sub(SimdObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"other", "width", NULL};
    Py_ssize_t param_width = 0;
    PyObject* param_other = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "On", kwlist,
                                     &param_other, &param_width)) {
        return NULL;
    }

    if (param_other->ob_type != &SimdObjectType) {
        PyErr_Format(SimdError, "Expected vector, got type '%s'", param_other->ob_type->tp_name);
        return NULL;
    }

    switch (param_width) {
        case 1:
            simd_vec_sub_i8(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        case 2:
            simd_vec_sub_i16(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        case 4:
            simd_vec_sub_i32(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        case 8:
            simd_vec_sub_i64(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        default:
            PyErr_Format(SimdError, "Unrecognized width: %zu for sub operation", (size_t)param_width);
            return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject*
SimdObject_fsub(SimdObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"other", "width", NULL};
    Py_ssize_t param_width = 0;
    PyObject* param_other = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "On", kwlist,
                                     &param_other, &param_width)) {
        return NULL;
    }

    if (param_other->ob_type != &SimdObjectType) {
        PyErr_Format(SimdError, "Expected vector, got type '%s'", param_other->ob_type->tp_name);
        return NULL;
    }

    switch (param_width) {
        case 4:
            simd_vec_sub_f32(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        case 8:
            simd_vec_sub_f64(&(self->vec), &(((SimdObject*)param_other)->vec));
            break;
        default:
            PyErr_Format(SimdError, "Unrecognized width: %zu for fsub operation", (size_t)param_width);
            return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject*
SimdObject_as_bytes(SimdObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"start", "end", NULL};
    Py_ssize_t param_start = 0;
    Py_ssize_t param_end = 0;
    size_t actual_start = 0;
    size_t actual_end = self->vec.size;
    PyObject* bytes_made = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|nn", kwlist,
                                     &param_start, &param_end)) {
        return NULL;
    }

    if (param_start != 0) {
        if (param_start > self->vec.size || param_start < 0) {
            PyErr_Format(SimdError, "start: '%ld', is out of bounds for vector of size %zu", param_start, self->vec.size);
            return NULL;
        }
        actual_start = (size_t)param_start;
    }

    if (param_end != 0) {
        if (param_end > self->vec.size || param_end < 0) {
            PyErr_Format(SimdError, "end: '%ld', is out of bounds for vector of size %zu", param_end, self->vec.size);
            return NULL;
        }
        actual_end = (size_t)param_end;
    }

    bytes_made = PyBytes_FromStringAndSize((const char*)self->vec.data +  actual_start, actual_end - actual_start);
    RETURN_OR_SYS_ERROR(bytes_made);

}

static PyObject*
SimdObject_as_tuple(SimdObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"type", "width", NULL};
    PyObject* tuple_to_give = NULL;
    PyObject* param_type = NULL;
    Py_ssize_t param_width = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "On", kwlist,
                                     &param_type, &param_width)) {
        return NULL;
    }

    size_t actual_width = (size_t)param_width;
    if (actual_width != 1 && actual_width != 2 && actual_width != 4 && actual_width != 8) {
        PyErr_Format(SimdError, "The width '%zu' is not supported for method 'as_tuple'", actual_width);
        return NULL;
    }
    size_t n_members = self->vec.size / actual_width;
    tuple_to_give = PyTuple_New(n_members);

    if ((PyTypeObject*)param_type == &PyLong_Type) {
        if (actual_width == 1) {
            char* reader = (char*)(self->vec.data);
            for (size_t i = 0; i < n_members; ++i) {
                PyObject* to_put = PyLong_FromLong(reader[i]);
                if (to_put == NULL) {
                    Py_DECREF(tuple_to_give);
                    PyErr_Format(PyExc_SystemError, "Internal object failure line: %u", __LINE__);
                    return NULL;
                }
                PyTuple_SET_ITEM(tuple_to_give, i, to_put);
            }
        } else if (actual_width == 2) {
            short* reader = (short*)(self->vec.data);
            for (size_t i = 0; i < n_members; ++i) {
                PyObject* to_put = PyLong_FromLong(reader[i]);
                if (to_put == NULL) {
                    Py_DECREF(tuple_to_give);
                    PyErr_Format(PyExc_SystemError, "Internal object failure line: %u", __LINE__);
                    return NULL;
                }
                PyTuple_SET_ITEM(tuple_to_give, i, to_put);
            }
        } else if (actual_width == 4) {
            int* reader = (int*)(self->vec.data);
            for (size_t i = 0; i < n_members; ++i) {
                PyObject* to_put = PyLong_FromLong(reader[i]);
                if (to_put == NULL) {
                    Py_DECREF(tuple_to_give);
                    PyErr_Format(PyExc_SystemError, "Internal object failure line: %u", __LINE__);
                    return NULL;
                }
                PyTuple_SET_ITEM(tuple_to_give, i, to_put);
            }
        } else if (actual_width == 8) {
            long long* reader = (long long*)(self->vec.data);
            for (size_t i = 0; i < n_members; ++i) {
                PyObject* to_put = PyLong_FromLongLong(reader[i]);
                if (to_put == NULL) {
                    Py_DECREF(tuple_to_give);
                    PyErr_Format(PyExc_SystemError, "Internal object failure line: %u", __LINE__);
                    return NULL;
                }
                PyTuple_SET_ITEM(tuple_to_give, i, to_put);
            }
        } else {
            Py_FatalError("Should not reach this point in 'as_tuple', width error");
        }
    } else if ((PyTypeObject*)param_type == &PyFloat_Type) {
        if (actual_width == 4) {
            float* reader = (float*)(self->vec.data);
            for (size_t i = 0; i < n_members; ++i) {
                PyObject* to_put = PyFloat_FromDouble((double)reader[i]);
                if (to_put == NULL) {
                    Py_DECREF(tuple_to_give);
                    PyErr_Format(PyExc_SystemError, "Internal object failure line: %u", __LINE__);
                    return NULL;
                }
                PyTuple_SET_ITEM(tuple_to_give, i, to_put);
            }
        } else if (actual_width == 8) {
            double* reader = (double*)(self->vec.data);
            for (size_t i = 0; i < n_members; ++i) {
                PyObject* to_put = PyFloat_FromDouble(reader[i]);
                if (to_put == NULL) {
                    Py_DECREF(tuple_to_give);
                    PyErr_Format(PyExc_SystemError, "Internal object failure line: %u", __LINE__);
                    return NULL;
                }
                PyTuple_SET_ITEM(tuple_to_give, i, to_put);
            }
        } else {
            if (actual_width == 1 || actual_width == 2) {
                PyErr_Format(SimdError, "The width '%zu' is not supported for floats for 'as_tuple'", actual_width);
                Py_DECREF(tuple_to_give);
                return NULL;
            } else {
                Py_FatalError("Should not reach invalid state for float in 'as_tuple");
            }
        }
    } else {
        Py_DECREF(tuple_to_give);
        PyErr_Format(SimdError, "The type '%s' is not supported for method 'as_tuple'", param_type->ob_type->tp_name);
        return NULL;
    }
    return tuple_to_give;
}

static PyObject *
SimdObject_clear(SimdObject *self, PyObject *Py_UNUSED(ignored))
{
    pysimd_vec_clear_data(&(self->vec));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef SimdObject_methods[] = {
    {"clear", (PyCFunction) SimdObject_clear, METH_NOARGS,
     "Sets all bytes in the vector to 0"
    },
    {"size", (PyCFunction) SimdObject_size, METH_NOARGS,
     "Returns the current size of the vector"
    },
    {"resize", (PyCFunction) SimdObject_resize, METH_VARARGS | METH_KEYWORDS,
    "Resizes the vector to the desired capacity"
    },
    {"add", (PyCFunction) SimdObject_add, METH_VARARGS | METH_KEYWORDS,
    "Adds a vector into another vector, without creating a new vector"
    },
    {"fadd", (PyCFunction) SimdObject_fadd, METH_VARARGS | METH_KEYWORDS,
    "Adds a vector into another vector as floating point numbers"
    },
    {"sub", (PyCFunction) SimdObject_sub, METH_VARARGS | METH_KEYWORDS,
    "Subtracts a vector from another vector, without creating a new vector"
    },
    {"fsub", (PyCFunction) SimdObject_fsub, METH_VARARGS | METH_KEYWORDS,
    "Subtracts a vector from another vector as floating point numbers"
    },
    {"as_bytes", (PyCFunction) SimdObject_as_bytes, METH_VARARGS | METH_KEYWORDS,
    "Returns a bytes object representing the internal bytes of the vector"
    },
    {"as_tuple", (PyCFunction) SimdObject_as_tuple, METH_VARARGS | METH_KEYWORDS,
    "Returns a tuple populated with members of the vector, defaults to 32 bit integers"
    },
    {"copy", (PyCFunction) SimdObject_copy, METH_VARARGS | METH_KEYWORDS,
    "Returns a copy of the vector"
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