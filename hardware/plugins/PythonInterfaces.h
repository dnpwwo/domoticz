#pragma once

#include "Plugins.h"
#include "PythonDevices.h"
#include "DelayedLink.h"

namespace Plugins {

	class CInterface {
	public:
		PyObject_HEAD
		long		InterfaceID;
		PyObject*	Name;
		PyObject*	Settings;
		PyObject*	Configuration;
		bool		Debug;
		bool		Active;
		PyObject*	Devices;
		CPlugin*	pPlugin;
		// Interface functions NOT exposed to Python
		bool		sanityCheck;
		CInterface* Copy();
		CDevice*	AddDeviceToDict(long lDeviceID);
		CDevice*	FindDevice(long lDeviceID);
		operator PyObject* () { return &ob_base; }
		operator const PyObject* () { return (const PyObject*)&ob_base; }
	};

	void CInterface_dealloc(CInterface* self);
	PyObject* CInterface_new(PyTypeObject* type, PyObject* args, PyObject* kwds);
	int CInterface_init(CInterface* self, PyObject* args, PyObject* kwds);
	PyObject* CInterface_refresh(CInterface* self);
	PyObject* CInterface_update(CInterface* self);
	PyObject* CInterface_str(CInterface* self);

	PyObject* CInterface_debug(CInterface* self, PyObject* args, PyObject* kwds);
	PyObject* CInterface_log(CInterface* self, PyObject* args, PyObject* kwds);
	PyObject* CInterface_error(CInterface* self, PyObject* args, PyObject* kwds);
	PyObject* CInterface_timeout(CInterface* self, PyObject* args, PyObject* kwds);

	static PyMemberDef CInterface_members[] = {
		{ "InterfaceID", T_LONG, offsetof(CInterface, InterfaceID), READONLY, "Internal Interface Number" },
		{ "Name", T_OBJECT,	offsetof(CInterface, Name), READONLY, "Name" },
		{ "Configuration", T_OBJECT, offsetof(CInterface, Configuration), 0, "Configuration" },
		{ "Debugging", T_BOOL, offsetof(CInterface, Debug), READONLY, "Debug logging status" },
		{ "Active", T_BOOL, offsetof(CInterface, Active), 0, "Interface active status" },
		{ "Devices", T_OBJECT, offsetof(CInterface, Devices), READONLY, "Devices dictionary" },
		{ NULL }  /* Sentinel */
	};

	static PyMethodDef CInterface_methods[] = {
		{ "Refresh",	(PyCFunction)CInterface_refresh,	METH_NOARGS, "Refresh the Interface from the database" },
		{ "Update",		(PyCFunction)CInterface_update,		METH_NOARGS, "Update the Interface" },
		{ "Debug",		(PyCFunction)CInterface_debug,		METH_VARARGS | METH_KEYWORDS, "Write a debug message to the Interface's log." },
		{ "Log",		(PyCFunction)CInterface_log,		METH_VARARGS | METH_KEYWORDS, "Write a message to the Interface's log." },
		{ "Error",		(PyCFunction)CInterface_error,		METH_VARARGS | METH_KEYWORDS, "Write a error message to the Interface's log." },
		{ "Timeout",	(PyCFunction)CInterface_timeout,	METH_VARARGS | METH_KEYWORDS, "Enable/Disable restart if data not seen for an hour." },
		{ NULL }  /* Sentinel */
	};

	static PyTypeObject CInterfaceType = {
		PyVarObject_HEAD_INIT(NULL, 0)
		"domoticz.Interface",             /* tp_name */
		sizeof(CInterface),             /* tp_basicsize */
		0,                         /* tp_itemsize */
		(destructor)CInterface_dealloc, /* tp_dealloc */
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
		(reprfunc)CInterface_str,     /* tp_str */
		0,                         /* tp_getattro */
		0,                         /* tp_setattro */
		0,                         /* tp_as_buffer */
		Py_TPFLAGS_DEFAULT |
		Py_TPFLAGS_BASETYPE,   /* tp_flags */
		"Domoticz Interface",           /* tp_doc */
		0,                         /* tp_traverse */
		0,                         /* tp_clear */
		0,                         /* tp_richcompare */
		0,                         /* tp_weaklistoffset */
		0,                         /* tp_iter */
		0,                         /* tp_iternext */
		CInterface_methods,             /* tp_methods */
		CInterface_members,             /* tp_members */
		0,                         /* tp_getset */
		0,                         /* tp_base */
		0,                         /* tp_dict */
		0,                         /* tp_descr_get */
		0,                         /* tp_descr_set */
		0,                         /* tp_dictoffset */
		(initproc)CInterface_init,      /* tp_init */
		0,                         /* tp_alloc */
		CInterface_new                 /* tp_new */
	};
}
