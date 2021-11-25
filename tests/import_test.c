#define PY_SSIZE_T_CLEAN
#include <Python.h>

static const char* EXT_MOD_NAME = "simd";

int
main(int argc, char *argv[])
{
    PyObject* pModule;
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }
    Py_SetProgramName(program);  /* optional but recommended */
    Py_Initialize();
    PyObject * sys_path = PySys_GetObject("path");
    PyList_Append(sys_path, PyUnicode_FromString(TESTING_BIN_PATH));

    pModule = PyImport_ImportModule(EXT_MOD_NAME);

    if (pModule != NULL) {
        printf("Imported module: %s\n", EXT_MOD_NAME);
        Py_DECREF(pModule);
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
