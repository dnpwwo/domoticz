#pragma once

#include "Plugins.h"
#include "DelayedLink.h"

namespace Plugins {

	typedef struct {
		PyObject_HEAD
		long		ValueID;
		long		DeviceID;
		PyObject*	Name;
		long		UnitID;
		PyObject*	Value;
		bool		Debug;
		PyObject*	Timestamp;
		CPlugin*	pPlugin;
	} CValue;

	void CValue_dealloc(CValue* self);
	PyObject* CValue_new(PyTypeObject* type, PyObject* args, PyObject* kwds);
	int CValue_init(CValue* self, PyObject* args, PyObject* kwds);
	PyObject* CValue_refresh(CValue* self);
	PyObject* CValue_insert(CValue* self);
	PyObject* CValue_update(CValue* self);
	PyObject* CValue_delete(CValue* self);
	PyObject* CValue_str(CValue* self);

	PyObject* CValue_getValueID(CValue* self, void* closure);
	int		  CValue_setValueID(CValue* self, PyObject* value, void* closure);

	PyObject* CValue_debug(CValue* self, PyObject* args, PyObject* kwds);
	PyObject* CValue_log(CValue* self, PyObject* args, PyObject* kwds);
	PyObject* CValue_error(CValue* self, PyObject* args, PyObject* kwds);

	static PyMemberDef CValue_members[] = {
		{ "ValueID",	T_LONG, offsetof(CValue, ValueID), READONLY, "Internal Value Number" },
		{ "Name", T_OBJECT,	offsetof(CValue, Name), READONLY, "Name" },
		{ "DeviceID", T_LONG, offsetof(CValue, DeviceID), READONLY, "Device this Value relates to" },
		{ "UnitID",	T_LONG, offsetof(CValue, UnitID), READONLY, "Unit ID this Value is constrained by" },
		{ "Value", T_OBJECT, offsetof(CValue, Value), 0, "Value" },
		{ "Debug", T_BOOL, offsetof(CValue, Debug), READONLY, "Debug logging status" },
		{ "Timestamp", T_OBJECT, offsetof(CValue, Timestamp), READONLY, "Last update timestamp" },
		{ NULL }  /* Sentinel */
	};

	static PyMethodDef CValue_methods[] = {
		{ "Refresh",(PyCFunction)CValue_refresh, METH_NOARGS, "Refresh the Value from the database" },
		{ "Insert", (PyCFunction)CValue_insert, METH_NOARGS, "Insert the Value" },
		{ "Update", (PyCFunction)CValue_update, METH_NOARGS, "Update the Value" },
		{ "Delete", (PyCFunction)CValue_delete, METH_NOARGS, "Delete the Value" },
		{ "Debug",	(PyCFunction)CValue_debug,	METH_VARARGS | METH_KEYWORDS, "Write a debug message to the Value's log." },
		{ "Log",	(PyCFunction)CValue_log,	METH_VARARGS | METH_KEYWORDS, "Write a message to the Value's log." },
		{ "Error",	(PyCFunction)CValue_error,	METH_VARARGS | METH_KEYWORDS, "Write a error message to the Value's log." },
		{ NULL }  /* Sentinel */
	};

	static PyTypeObject CValueType = {
		PyVarObject_HEAD_INIT(NULL, 0)
		"domoticz.Value",          /* tp_name */
		sizeof(CValue),            /* tp_basicsize */
		0,                         /* tp_itemsize */
		(destructor)CValue_dealloc, /* tp_dealloc */
		0,                         /* tp_print */
		0,                         /* tp_getattr */
		0,                         /* tp_setattr */
		0,                         /* tp_reserved */
		0,                         /* tp_repr */
		0,                         /* tp_as_number */
		0,                         /* tp_as_sequence */
		0,                         /* tp_as_mapping */
		0,                         /* tp_hash  */
		0,                         /* tp_call */
		(reprfunc)CValue_str,      /* tp_str */
		0,                         /* tp_getattro */
		0,                         /* tp_setattro */
		0,                         /* tp_as_buffer */
		Py_TPFLAGS_DEFAULT |
		Py_TPFLAGS_BASETYPE,       /* tp_flags */
		"Domoticz Value",          /* tp_doc */
		0,                         /* tp_traverse */
		0,                         /* tp_clear */
		0,                         /* tp_richcompare */
		0,                         /* tp_weaklistoffset */
		0,                         /* tp_iter */
		0,                         /* tp_iternext */
		CValue_methods,            /* tp_methods */
		CValue_members,            /* tp_members */
		0,                         /* tp_getset */
		0,                         /* tp_base */
		0,                         /* tp_dict */
		0,                         /* tp_descr_get */
		0,                         /* tp_descr_set */
		0,                         /* tp_dictoffset */
		(initproc)CValue_init,     /* tp_init */
		0,                         /* tp_alloc */
		CValue_new                 /* tp_new */
	};
}
