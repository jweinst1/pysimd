#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define TARGET_SIZE 128

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
	PyObject *pModule, *pVecType, *pNewed, *pSized, *pFName, *pArgs;
	PyObject *pInitArg;
	Py_Initialize();
    PyObject * sys_path = PySys_GetObject("path");
    PyList_Append(sys_path, PyUnicode_FromString(TESTING_BIN_PATH));

    pModule = PyImport_ImportModule(EXT_MOD_NAME);
    if (pModule != NULL) {
    	pVecType = PyObject_GetAttrString(pModule, VEC_TYPE);
    	if (pVecType != NULL) {
    		pArgs = PyTuple_New(1);
    		pInitArg = PyLong_FromSize_t(TARGET_SIZE);
    		if (pArgs == NULL || pInitArg == NULL) {
    			Py_FatalError("Cannot initialize single argument list, something is really wrong");
    		}
    		PyTuple_SetItem(pArgs, 0, pInitArg);
    		pNewed = PyObject_CallObject(pVecType, pArgs);
    		Py_DECREF(pArgs);
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
    						if (oSize != TARGET_SIZE) {
    							fprintf(stderr, "Expected size to be %zu\n", (size_t)TARGET_SIZE);
    							return 1;
    						}
    					} else {
    						PyErr_Print();
    						print_and_dec(pSized, "size of vec");
    						print_and_dec(pFName, "size func name");
    						print_and_dec(pNewed, "Vec obj");
    						print_and_dec(pVecType, "Vec type");
    						print_and_dec(pModule, "simd mod");
    						return 1;
    					}
    				} else {
    					PyErr_Print();
						print_and_dec(pFName, "size func name");
						print_and_dec(pNewed, "Vec obj");
						print_and_dec(pVecType, "Vec type");
						print_and_dec(pModule, "simd mod");
						return 1;
    				}
    			} else {
    				PyErr_Print();
					print_and_dec(pNewed, "Vec obj");
					print_and_dec(pVecType, "Vec type");
					print_and_dec(pModule, "simd mod");
					return 1;
    			} 
    		} else {
    			PyErr_Print();
				print_and_dec(pVecType, "Vec type");
				print_and_dec(pModule, "simd mod");
				return 1;
    		}
    	} else {
    		PyErr_Print();
			print_and_dec(pModule, "simd mod");
			return 1;
    	}
    } else {
    	PyErr_Print();
    	return 1;
    }

    if (Py_FinalizeEx() < 0) {
        exit(120);
    }
	return 0;
}

#undef TARGET_SIZE