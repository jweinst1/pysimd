#define PY_SSIZE_T_CLEAN
#include <Python.h>

static const char* EXT_MOD_NAME = "simd";
static const char* VEC_TYPE = "Vec";
static const char* SIZE_FUNC = "size";

static inline void print_and_dec(PyObject* obj, const char* label)
{
	printf("REF '%s' = %ld\n", label, Py_REFCNT(obj));
	Py_DECREF(obj);
}

int
main(int argc, char *argv[])
{
	PyObject* pModule, *pVecType, *pNewed, *pSized, *pFName;
	Py_Initialize();
    PyObject * sys_path = PySys_GetObject("path");
    PyList_Append(sys_path, PyUnicode_FromString(TESTING_BIN_PATH));

    pModule = PyImport_ImportModule(EXT_MOD_NAME);
    if (pModule != NULL) {
    	pVecType = PyObject_GetAttrString(pModule, VEC_TYPE);
    	if (pVecType != NULL) {
    		pNewed = PyType_GenericNew((PyTypeObject*)pVecType, NULL, NULL);
    		if (pNewed != NULL) {
    			pFName = PyUnicode_FromString(SIZE_FUNC);
    			if (pFName != NULL) {
    				pSized = PyObject_CallMethodObjArgs(pNewed, pFName, NULL);
    				if (pSized != NULL && PyLong_Check(pSized)) {
    					size_t oSize = PyLong_AsSize_t(pSized);
    					if (oSize != (size_t)-1) {
    						printf("Succesfully determined size of vec is %zu\n", oSize);
    						print_and_dec(pSized, "size of vec");
    						print_and_dec(pFName, "size func name");
    						print_and_dec(pNewed, "Vec obj");
    						print_and_dec(pVecType, "Vec type");
    						print_and_dec(pModule, "simd mod");
    					} else {
    						PyErr_Print();
    						print_and_dec(pSized, "size of vec");
    						print_and_dec(pFName, "size func name");
    						print_and_dec(pNewed, "Vec obj");
    						print_and_dec(pVecType, "Vec type");
    						print_and_dec(pModule, "simd mod");
    					}
    				} else {
    					PyErr_Print();
						print_and_dec(pFName, "size func name");
						print_and_dec(pNewed, "Vec obj");
						print_and_dec(pVecType, "Vec type");
						print_and_dec(pModule, "simd mod");
    				}
    			} else {
    				PyErr_Print();
					print_and_dec(pNewed, "Vec obj");
					print_and_dec(pVecType, "Vec type");
					print_and_dec(pModule, "simd mod");
    			} 
    		} else {
    			PyErr_Print();
				print_and_dec(pVecType, "Vec type");
				print_and_dec(pModule, "simd mod");
    		}
    	} else {
    		PyErr_Print();
			print_and_dec(pModule, "simd mod");
    	}
    } else {
    	PyErr_Print();
    }

    if (Py_FinalizeEx() < 0) {
        exit(120);
    }
	return 0;
}