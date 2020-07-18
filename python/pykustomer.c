/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "kustomer.h"

#if PY_MAJOR_VERSION >= 3
#define PY3K
#else
#error This module is Python 3 only!
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

typedef struct {
	PyObject_HEAD
	PyObject *in_weakreflist;
	void *kpc_ptr;
} pykustomer_KopanoProductClaimsObject;

static void
pykustomer_KopanoProductClaimsObject_dealloc(pykustomer_KopanoProductClaimsObject* self)
{
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
pykustomer_KopanoProductClaimsObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	pykustomer_KopanoProductClaimsObject *self;

	self = (pykustomer_KopanoProductClaimsObject *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->kpc_ptr = NULL;
	}

	return (PyObject *)self;
}

static PyMethodDef pykustomer_KopanoProductClaimsObject_methods[] = {
	{NULL}  /* Sentinel */
};

static PyTypeObject pykustomer_KopanoProductClaimsType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"_pykustomer.KopanoProductClaimsType", /* tp_name */
	sizeof(pykustomer_KopanoProductClaimsObject),	/* tp_basicsize */
	0,						/* tp_itemsize */
	(destructor)pykustomer_KopanoProductClaimsObject_dealloc, /* tp_dealloc */
	0,						/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	0,						/* tp_reserved */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,						/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,						/* tp_hash  */
	0,						/* tp_call */
	0,						/* tp_str */
	0,						/* tp_getattro */
	0,						/* tp_setattro */
	0,						/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
	"KopanoProductClaimsType objects",		/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	0,						/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	pykustomer_KopanoProductClaimsObject_methods,	/* tp_methods */
	0,       					/* tp_members */
	0,						/* tp_getset */
	0,						/* tp_base */
	0,						/* tp_dict */
	0,						/* tp_descr_get */
	0,						/* tp_descr_set */
	0,						/* tp_dictoffset */
	0,						/* tp_init */
	0,						/* tp_alloc */
	pykustomer_KopanoProductClaimsObject_new,       /* tp_new */
};

static PyObject *
pykustomer_begin_ensure(PyObject *self, PyObject *args)
{
	struct kustomer_begin_ensure_return res;

	Py_BEGIN_ALLOW_THREADS;
	res = kustomer_begin_ensure();
	Py_END_ALLOW_THREADS;

	if (res.r0 != 0) {
		PyErr_SetObject(PyKustomerError, PyLong_FromLong(res.r0));
		return NULL;
	}

	pykustomer_KopanoProductClaimsObject *kpc;
	kpc = PyObject_New(pykustomer_KopanoProductClaimsObject, &pykustomer_KopanoProductClaimsType);
	kpc->kpc_ptr = res.r1;

	return (PyObject*)kpc;
}

static PyObject *
pykustomer_end_ensure(PyObject *self, PyObject *args)
{
	pykustomer_KopanoProductClaimsObject *kpc;

	if (!PyArg_ParseTuple(args, "O!", &pykustomer_KopanoProductClaimsType, &kpc))
		return NULL;

	int res;

	Py_BEGIN_ALLOW_THREADS;
	res = kustomer_end_ensure(kpc->kpc_ptr);
	Py_END_ALLOW_THREADS;

	kpc->kpc_ptr = NULL;

	if (res != 0) {
		PyErr_SetObject(PyKustomerError, PyLong_FromLong(res));
		return NULL;
	}

	return PyLong_FromLong(res);
}

static PyMethodDef MyMethods[] = {
	{"initialize", pykustomer_initialize, METH_VARARGS, "Initialize Kustomer."},
	{"wait_until_ready", pykustomer_wait_until_ready, METH_VARARGS, "Wait until Kustomer is ready or until timeout."},
	{"uninitialize",  pykustomer_uninitialize, METH_NOARGS, "Uninitialize Kustomer."},
	{"begin_ensure", pykustomer_begin_ensure, METH_NOARGS, "Begin ensure."},
	{"end_ensure", pykustomer_end_ensure, METH_VARARGS, "End ensure."},
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

	pykustomer_KopanoProductClaimsType.tp_new = PyType_GenericNew;
	if (PyType_Ready(&pykustomer_KopanoProductClaimsType) < 0)
		return NULL;

	m = PyModule_Create(&myModule);
	if (m == NULL)
		return NULL;

	PyKustomerError = PyErr_NewException("_pykustomer.Error", NULL, NULL);
	Py_INCREF(PyKustomerError);
	PyModule_AddObject(m, "Error", PyKustomerError);

	Py_INCREF(&pykustomer_KopanoProductClaimsType);
	PyModule_AddObject(m, "KopanoProductClaimsType", (PyObject *)&pykustomer_KopanoProductClaimsType);

	return m;
}
#endif // PY3K
