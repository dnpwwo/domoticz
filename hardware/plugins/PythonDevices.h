#pragma once

#include "Plugins.h"
#include "DelayedLink.h"

namespace Plugins {

	typedef struct {
		PyObject_HEAD
			PyObject* PluginKey;
		int			InterfaceID;
		PyObject* DeviceID;
		int			Unit;
		int			Type;
		int			SubType;
		int			SwitchType;
		int			ID;
		int			LastLevel;
		PyObject* Name;
		PyObject* LastUpdate;
		int			nValue;
		int			SignalLevel;
		int			BatteryLevel;
		PyObject* sValue;
		int			Image;
		PyObject* Options;
		int			Used;
		int			TimedOut;
		PyObject* Description;
		PyObject* Color;
		CPlugin* pPlugin;
	} CDevice;

	void CDevice_dealloc(CDevice* self);
	PyObject* CDevice_new(PyTypeObject* type, PyObject* args, PyObject* kwds);
	int CDevice_init(CDevice* self, PyObject* args, PyObject* kwds);
	PyObject* CDevice_refresh(CDevice* self);
	PyObject* CDevice_insert(CDevice* self);
	PyObject* CDevice_update(CDevice* self, PyObject* args, PyObject* kwds);
	PyObject* CDevice_delete(CDevice* self);
	PyObject* CDevice_touch(CDevice* self);
	PyObject* CDevice_str(CDevice* self);

	static PyMemberDef CDevice_members[] = {
		{ "ID",	T_INT, offsetof(CDevice, ID), READONLY, "Domoticz internal ID" },
		{ "Name", T_OBJECT,	offsetof(CDevice, Name), READONLY, "Name" },
		{ "DeviceID", T_OBJECT,	offsetof(CDevice, DeviceID), READONLY, "External device ID" },
		{ "Unit",	T_INT, offsetof(CDevice, Unit), READONLY, "Numeric Unit number" },
		{ "nValue", T_INT, offsetof(CDevice, nValue), READONLY, "Numeric device value" },
		{ "sValue", T_OBJECT, offsetof(CDevice, sValue), READONLY, "String device value" },
		{ "SignalLevel", T_INT, offsetof(CDevice, SignalLevel), READONLY, "Numeric signal level" },
		{ "BatteryLevel", T_INT, offsetof(CDevice, BatteryLevel), READONLY, "Numeric battery level" },
		{ "Image", T_INT, offsetof(CDevice, Image), READONLY, "Numeric image number" },
		{ "Type", T_INT, offsetof(CDevice, Type), READONLY, "Numeric device type" },
		{ "SubType", T_INT, offsetof(CDevice, SubType), READONLY, "Numeric device subtype" },
		{ "SwitchType", T_INT, offsetof(CDevice, SwitchType), READONLY, "Numeric device switchtype" },
		{ "LastLevel", T_INT, offsetof(CDevice, LastLevel), READONLY, "Previous device level" },
		{ "LastUpdate", T_OBJECT, offsetof(CDevice, LastUpdate), READONLY, "Last update timestamp" },
		{ "Options", T_OBJECT, offsetof(CDevice, Options), READONLY, "Device options" },
		{ "Used", T_INT, offsetof(CDevice, Used), READONLY, "Numeric device Used flag" },
		{ "TimedOut", T_INT, offsetof(CDevice, TimedOut), READONLY, "Is the device marked as timed out" },
		{ "Description", T_OBJECT, offsetof(CDevice, Description), READONLY, "Description" },
		{ "Color", T_OBJECT, offsetof(CDevice, Color), READONLY, "Color JSON dictionary" },
		{ NULL }  /* Sentinel */
	};

	static PyMethodDef CDevice_methods[] = {
		{ "Refresh", (PyCFunction)CDevice_refresh, METH_NOARGS, "Refresh device details" },
		{ "Create", (PyCFunction)CDevice_insert, METH_NOARGS, "Create the device in Domoticz." },
		{ "Update", (PyCFunction)CDevice_update, METH_VARARGS | METH_KEYWORDS, "Update the device values in Domoticz." },
		{ "Delete", (PyCFunction)CDevice_delete, METH_NOARGS, "Delete the device in Domoticz." },
		{ "Touch", (PyCFunction)CDevice_touch, METH_NOARGS, "Notify Domoticz that device has been seen." },
		{ NULL }  /* Sentinel */
	};

	static PyTypeObject CDeviceType = {
		PyVarObject_HEAD_INIT(NULL, 0)
		"Domoticz.Device",             /* tp_name */
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
