#pragma once

#include "Plugins.h"
#include "DelayedLink.h"

namespace Plugins {

	typedef struct {
		PyObject_HEAD
		long		DeviceID;
		long		InterfaceID;
		PyObject*	Name;
		PyObject*	ExternalID;
		bool		Debug;
		bool		Active;
		PyObject*	Timestamp;
		PyObject*	Values;
		CPlugin*	pPlugin;
	} CDevice;

	void CDevice_dealloc(CDevice* self);
	PyObject* CDevice_new(PyTypeObject* type, PyObject* args, PyObject* kwds);
	int CDevice_init(CDevice* self, PyObject* args, PyObject* kwds);
	PyObject* CDevice_refresh(CDevice* self);
	PyObject* CDevice_insert(CDevice* self);
	PyObject* CDevice_update(CDevice* self);
	PyObject* CDevice_delete(CDevice* self);
	PyObject* CDevice_str(CDevice* self);

	PyObject* CDevice_debug(CDevice* self, PyObject* args, PyObject* kwds);
	PyObject* CDevice_log(CDevice* self, PyObject* args, PyObject* kwds);
	PyObject* CDevice_error(CDevice* self, PyObject* args, PyObject* kwds);

	static PyMemberDef CDevice_members[] = {
		{ "DeviceID",	T_LONG, offsetof(CDevice, DeviceID), READONLY, "Internal Device Number" },
		{ "InterfaceID", T_LONG, offsetof(CDevice, InterfaceID), READONLY, "Interface this Device relates to" },
		{ "Name", T_OBJECT,	offsetof(CDevice, Name), READONLY, "Name" },
		{ "ExternalID", T_OBJECT,	offsetof(CDevice, ExternalID), READONLY, "ExternalID" },
		{ "Debugging", T_BOOL, offsetof(CDevice, Debug), READONLY, "Debug logging status" },
		{ "Active", T_BOOL, offsetof(CDevice, Active), 0, "Device active status" },
		{ "Timestamp", T_OBJECT, offsetof(CDevice, Timestamp), READONLY, "Last update timestamp" },
		{ "Values", T_OBJECT, offsetof(CDevice, Values), READONLY, "Values dictionary" },
		{ NULL }  /* Sentinel */
	};

	static PyMethodDef CDevice_methods[] = {
		{ "Refresh",(PyCFunction)CDevice_refresh, METH_NOARGS, "Refresh the Device from the database" },
		{ "Insert", (PyCFunction)CDevice_insert, METH_NOARGS, "Insert the Device" },
		{ "Update", (PyCFunction)CDevice_update, METH_NOARGS, "Update the Device" },
		{ "Delete", (PyCFunction)CDevice_delete, METH_NOARGS, "Delete the Device" },
		{ "Debug",	(PyCFunction)CDevice_debug,	METH_VARARGS | METH_KEYWORDS, "Write a debug message to the Device's log." },
		{ "Log",	(PyCFunction)CDevice_log,	METH_VARARGS | METH_KEYWORDS, "Write a message to the Device's log." },
		{ "Error",	(PyCFunction)CDevice_error,	METH_VARARGS | METH_KEYWORDS, "Write a error message to the Device's log." },
		{ NULL }  /* Sentinel */
	};

	static PyTypeObject CDeviceType = {
		PyVarObject_HEAD_INIT(NULL, 0)
		"domoticz.Device",             /* tp_name */
		sizeof(CDevice),             /* tp_basicsize */
		0,                         /* tp_itemsize */
		(destructor)CDevice_dealloc, /* tp_dealloc */
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
		(reprfunc)CDevice_str,     /* tp_str */
		0,                         /* tp_getattro */
		0,                         /* tp_setattro */
		0,                         /* tp_as_buffer */
		Py_TPFLAGS_DEFAULT |
		Py_TPFLAGS_BASETYPE,   /* tp_flags */
		"Domoticz Device",           /* tp_doc */
		0,                         /* tp_traverse */
		0,                         /* tp_clear */
		0,                         /* tp_richcompare */
		0,                         /* tp_weaklistoffset */
		0,                         /* tp_iter */
		0,                         /* tp_iternext */
		CDevice_methods,             /* tp_methods */
		CDevice_members,             /* tp_members */
		0,                         /* tp_getset */
		0,                         /* tp_base */
		0,                         /* tp_dict */
		0,                         /* tp_descr_get */
		0,                         /* tp_descr_set */
		0,                         /* tp_dictoffset */
		(initproc)CDevice_init,      /* tp_init */
		0,                         /* tp_alloc */
		CDevice_new                 /* tp_new */
	};
}
