/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#include <Python.h>
#include "kustomer.h"

#if PY_MAJOR_VERSION >= 3
#define PY3K
#endif


static PyObject *PyKustomerError;

static PyObject *
pykustomer_initialize(PyObject *self, PyObject *args)
{
	char *productName_s;
	int res;

	if (!PyArg_ParseTuple(args, "z", &productName_s))
		return NULL;

	Py_BEGIN_ALLOW_THREADS;
	res = kustomer_initialize(productName_s);
	Py_END_ALLOW_THREADS;

	if (res != 0) {
		PyErr_SetObject(PyKustomerError, PyLong_FromLong(res));
		return NULL;
	}

	return PyLong_FromLong(res);
}

static PyObject *
pykustomer_uninitialize(PyObject *self, PyObject *args)
{
	int res;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	Py_BEGIN_ALLOW_THREADS;
	res = kustomer_uninitialize();
	Py_END_ALLOW_THREADS;

	if (res != 0) {
		PyErr_SetObject(PyKustomerError, PyLong_FromLong(res));
		return NULL;
	}

	return PyLong_FromLong(res);
}

static PyObject *
pykustomer_wait_until_ready(PyObject *self, PyObject *args)
{
	unsigned long long timeout;
	int res;

	if (!PyArg_ParseTuple(args, "K", &timeout))
		return NULL;

	Py_BEGIN_ALLOW_THREADS;
	res = kustomer_wait_until_ready(timeout);
	Py_END_ALLOW_THREADS;

	if (res != 0) {
		PyErr_SetObject(PyKustomerError, PyLong_FromLong(res));
		return NULL;
	}

	return PyLong_FromLong(res);
}

static PyMethodDef MyMethods[] = {
	{"initialize", pykustomer_initialize, METH_VARARGS, "Initialize Kustomer."},
	{"wait_until_ready", pykustomer_wait_until_ready, METH_VARARGS, "Wait until Kustomer is ready or until timeout."},
	{"uninitialize",  pykustomer_uninitialize, METH_VARARGS, "Uninitialize Kustomer."},
	{NULL, NULL, 0, NULL} /* Sentinel */
};

#ifdef PY3K
static struct PyModuleDef myModule = {
	PyModuleDef_HEAD_INIT,
	"_pykustomer",
	NULL,
	-1,
	MyMethods
};
PyMODINIT_FUNC
PyInit__pykustomer(void)
{
	PyObject *m;

	m = PyModule_Create(&myModule);
	if (m == NULL)
		return NULL;

	PyKustomerError = PyErr_NewException("_pykustomer.Error", NULL, NULL);
	Py_INCREF(PyKustomerError);
	PyModule_AddObject(m, "Error", PyKustomerError);

	return m;
}
#else // PY3K
void init_pykustomer(void)
{
	PyObject *m;

	m = Py_InitModule3("_pykustomer", MyMethods, NULL);
	if (m == NULL)
		return;

	PyKustomerError = PyErr_NewException("_pykustomer.Error", NULL, NULL);
	Py_INCREF(PyKustomerError);
	PyModule_AddObject(m, "Error", PyKustomerError);

	return;
}
#endif // PY3K
