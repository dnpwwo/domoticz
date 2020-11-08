#pragma once

#include "Plugins.h"
#include "DelayedLink.h"

namespace Plugins {

	class CPluginTransport;
	class CPluginProtocol;

	class CConnection {
		public:
			PyObject_HEAD
			PyObject*			Name;
			PyObject*			Target;
			PyObject*			Address;		// Python object to callback to (Interface or Device normally)
			PyObject*			Port;
			int					Baud;
			int					Timeout;
			PyObject*			LastSeen;
			CPlugin*			pPlugin;
			PyObject*			Transport;
			CPluginTransport*	pTransport;
			PyObject*			Protocol;
			CPluginProtocol*	pProtocol;
			CConnection*		Parent;			// Parent Connection, used when a listening connection spawns a Connection
	};

	void CConnection_dealloc(CConnection* self);
	PyObject* CConnection_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
	int CConnection_init(CConnection *self, PyObject *args, PyObject *kwds);
	PyObject* CConnection_connect(CConnection* self, PyObject* args, PyObject* kwds);
	PyObject* CConnection_listen(CConnection* self, PyObject* args, PyObject* kwds);
	PyObject* CConnection_send(CConnection *self, PyObject *args, PyObject *kwds);
	PyObject* CConnection_disconnect(CConnection* self);
	PyObject* CConnection_bytes(CConnection* self);
	PyObject* CConnection_isconnecting(CConnection* self);
	PyObject* CConnection_isconnected(CConnection* self);
	PyObject* CConnection_timestamp(CConnection* self);
	PyObject* CConnection_str(CConnection* self);

	static PyMemberDef CConnection_members[] = {
		{ "Name", T_OBJECT,	offsetof(CConnection, Name), READONLY, "Name" },
		{ "Address", T_OBJECT,	offsetof(CConnection, Address), READONLY, "Address" },
		{ "Port", T_OBJECT,	offsetof(CConnection, Port), READONLY, "Port" },
		{ "Baud", T_INT, offsetof(CConnection, Baud), READONLY, "Baud" },
		{ "Parent", T_OBJECT, offsetof(CConnection, Parent), READONLY, "Parent connection" },
		{ "Target", T_OBJECT, offsetof(CConnection, Target), 0, "Event Target" },
		{ "Timeout", T_INT, offsetof(CConnection, Timeout), 0, "Timeout" },
		{ NULL }  /* Sentinel */
	};

	static PyMethodDef CConnection_methods[] = {
		{ "Connect", (PyCFunction)CConnection_connect, METH_VARARGS | METH_KEYWORDS, "Connect to specified Address/Port)" },
		{ "Send", (PyCFunction)CConnection_send, METH_VARARGS | METH_KEYWORDS, "Send data to connection." },
		{ "Listen", (PyCFunction)CConnection_listen, METH_VARARGS | METH_KEYWORDS, "Start listening on specified Port." },
		{ "Disconnect", (PyCFunction)CConnection_disconnect, METH_NOARGS, "Disconnect connection or stop listening." },
		{ "BytesTransferred", (PyCFunction)CConnection_bytes, METH_NOARGS, "Bytes transferred since connection was opened." },
		{ "Connecting", (PyCFunction)CConnection_isconnecting, METH_NOARGS, "Connection in progress." },
		{ "Connected", (PyCFunction)CConnection_isconnected, METH_NOARGS, "Connection status." },
		{ "LastSeen", (PyCFunction)CConnection_timestamp, METH_NOARGS, "Last seen timestamp." },
		{ NULL }  /* Sentinel */
	};

	static PyTypeObject CConnectionType = {
		PyVarObject_HEAD_INIT(NULL, 0)
		"Domoticz.Connection",             /* tp_name */
		sizeof(CConnection),             /* tp_basicsize */
		0,                         /* tp_itemsize */
		(destructor)CConnection_dealloc, /* tp_dealloc */
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
		(reprfunc)CConnection_str,     /* tp_str */
		0,                         /* tp_getattro */
		0,                         /* tp_setattro */
		0,                         /* tp_as_buffer */
		Py_TPFLAGS_DEFAULT |
		Py_TPFLAGS_BASETYPE,   /* tp_flags */
		"Connection Object",           /* tp_doc */
		0,                         /* tp_traverse */
		0,                         /* tp_clear */
		0,                         /* tp_richcompare */
		0,                         /* tp_weaklistoffset */
		0,                         /* tp_iter */
		0,                         /* tp_iternext */
		CConnection_methods,             /* tp_methods */
		CConnection_members,             /* tp_members */
		0,                         /* tp_getset */
		0,                         /* tp_base */
		0,                         /* tp_dict */
		0,                         /* tp_descr_get */
		0,                         /* tp_descr_set */
		0,                         /* tp_dictoffset */
		(initproc)CConnection_init,      /* tp_init */
		0,                         /* tp_alloc */
		CConnection_new                 /* tp_new */
	};
}
