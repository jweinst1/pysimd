#define PY_SSIZE_T_CLEAN
#include <Python.h>

static const char* EXT_MOD_NAME = "simd";
static const char* VEC_TYPE = "Vec";

static PyObject* args_list = NULL;
static PyObject* kw_list = NULL;

int
main(int argc, char *argv[])
{
    PyObject* pModule, *pVecType, *pNewed;
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }
    Py_SetProgramName(program);  /* optional but recommended */
    Py_Initialize();
    PyObject * sys_path = PySys_GetObject("path");
    PyList_Append(sys_path, PyUnicode_FromString(TESTING_BIN_PATH));

    args_list = PyList_New(0);
    kw_list = PyDict_New();

    if (args_list == NULL || kw_list == NULL) {
        Py_FatalError("Cannot initialize empty args or kwlist, something is really wrong");
    }

    pModule = PyImport_ImportModule(EXT_MOD_NAME);

    if (pModule != NULL) {
        printf("Imported module: %s\n", EXT_MOD_NAME);
        pVecType = PyObject_GetAttrString(pModule, VEC_TYPE);
        if (pVecType != NULL) {
            printf("Found type '%s'\n", VEC_TYPE);
            pNewed = PyType_GenericNew((PyTypeObject*)pVecType, NULL, NULL);
            if (pNewed != NULL) {
                printf("Calling new worked on type: %s\n", VEC_TYPE);
                Py_DECREF(pNewed);
                Py_DECREF(pModule);
                Py_DECREF(pVecType);
            } else {
                PyErr_Print();
                fprintf(stderr, "Failed to call new on type: %s\n", VEC_TYPE);
                Py_DECREF(pModule);
                Py_DECREF(pVecType);
                PyMem_RawFree(program);
                return 1;
            }
        } else {
            PyErr_Print();
            fprintf(stderr, "Module %s did not contain type '%s'\n", EXT_MOD_NAME, VEC_TYPE);
            Py_DECREF(pModule);
            PyMem_RawFree(program);
            return 1;
        }
    } else {
        PyErr_Print();
        PyMem_RawFree(program);
        return 1;
    }

    if (Py_FinalizeEx() < 0) {
        exit(120);
    }

    PyMem_RawFree(program);
    return 0;
}
