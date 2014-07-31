#include <Python.h>

#include "bcrypt.h"

void nudd_hash(const char* input, char* output)
{
    uint32_t hash[8];
    std::string const hash_data = bcrypt_iterated(
        std::string(
                reinterpret_cast<char const*>(input),80
        )
    );

    memcpy(output, hash_data.data(), 32);

}


static PyObject *nudd_getpowhash(PyObject *self, PyObject *args)
{
    char *output;
    PyObject *value;
#if PY_MAJOR_VERSION >= 3
    PyBytesObject *input;
#else
    PyStringObject *input;
#endif
    if (!PyArg_ParseTuple(args, "S", &input))
        return NULL;
    Py_INCREF(input);
    output = (char*)PyMem_Malloc(32);

#if PY_MAJOR_VERSION >= 3
    nudd_hash((char *)PyBytes_AsString((PyObject*) input), output);
#else
    nudd_hash((char *)PyString_AsString((PyObject*) input), output);
#endif
    Py_DECREF(input);
#if PY_MAJOR_VERSION >= 3
    value = Py_BuildValue("y#", output, 32);
#else
    value = Py_BuildValue("s#", output, 32);
#endif
    PyMem_Free(output);
    return value;
}

static PyMethodDef NuddMethods[] = {
    { "getPoWHash", nudd_getpowhash, METH_VARARGS, "Returns the proof of work hash using nudd hash" },
    { NULL, NULL, 0, NULL }
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef NuddModule = {
    PyModuleDef_HEAD_INIT,
    "nudd_hash",
    "...",
    -1,
    NuddMethods
};

PyMODINIT_FUNC PyInit_nudd_hash(void) {
    return PyModule_Create(&NuddModule);
}

#else

PyMODINIT_FUNC initnudd_hash(void) {
    (void) Py_InitModule("nudd_hash", NuddMethods);
}
#endif
