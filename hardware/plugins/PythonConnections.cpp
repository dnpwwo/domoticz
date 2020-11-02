#include "stdafx.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016
//

#include "../main/Logger.h"
#include "../main/SQLHelper.h"
#include "../main/localtime_r.h"
#include "../main/mainstructs.h"
#include "../main/mainworker.h"
#include "PythonConnections.h"
#include "PluginMessages.h"
#include "PluginProtocols.h"
#include "PluginTransports.h"
#include <datetime.h>

namespace Plugins {

	extern struct PyModuleDef DomoticzModuleDef;
	extern void LogPythonException(CPlugin* pPlugin, const std::string &sHandler);

	void CConnection_dealloc(CConnection * self)
	{
		if (self->pPlugin && (self->pPlugin->m_bDebug & PDM_CONNECTION))
		{
			_log.Log(LOG_NORM, "(%s) Deallocating connection object '%s' (%s:%s).", self->pPlugin->m_Name.c_str(), PyUnicode_AsUTF8(self->Name), PyUnicode_AsUTF8(self->Address), PyUnicode_AsUTF8(self->Port));
		}

		if (self->pTransport)
		{
			// if the transport is connected then disconnect it
			if (self->pTransport->IsConnected() || self->pTransport->IsConnecting())
			{
				self->pTransport->Clear();	// Suppress disconnect event being queued
				if (self->pPlugin)
				{
					self->pPlugin->InterfaceLog(LOG_ERROR, "%s:, Connection being deallocated while still connected.", __func__);
					std::string		sName = "";
					PyObjPtr		pStr = CConnection_str(self);
					sName = PyUnicode_AsUTF8(pStr);
					self->pPlugin->InterfaceLog(LOG_ERROR, "Connection details: %s", sName.c_str());
				}
				else
				{
					_log.Log(LOG_ERROR, "%s:, Connection being deallocated while still connected.", __func__);
				}
				self->pTransport->handleDisconnect();
			}
			delete self->pTransport;
			self->pTransport = NULL;
		}
		if (self->pProtocol)
		{
			delete self->pProtocol;
			self->pProtocol = NULL;
		}

		Py_XDECREF(self->Name);
		Py_XDECREF(self->Target);
		Py_XDECREF(self->Address);
		Py_XDECREF(self->Port);
		Py_XDECREF(self->LastSeen);
		Py_XDECREF(self->Transport);
		Py_XDECREF(self->Protocol);
		Py_XDECREF(self->Parent);

		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyObject * CConnection_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
	{
		CConnection *self = NULL;
		if ((CConnection *)type->tp_alloc)
		{
			self = (CConnection *)type->tp_alloc(type, 0);
		}
		else
		{
			//!Giz: self = NULL here!!
			//_log.Log(LOG_ERROR, "(%s) CConnection Type is not ready.", self->pPlugin->m_Name.c_str());
			_log.Log(LOG_ERROR, "(Python plugin) CConnection Type is not ready!");
		}

		try
		{
			if (self == NULL) {
				_log.Log(LOG_ERROR, "%s: Self is NULL.", __func__);
			}
			else {
				self->Name = PyUnicode_FromString("");
				if (self->Name == NULL) {
					Py_XDECREF(self);
					return NULL;
				}

				self->Target = Py_None;
				Py_INCREF(Py_None);

				self->Address = PyUnicode_FromString("");
				if (self->Address == NULL) {
					Py_XDECREF(self);
					return NULL;
				}

				self->Port = PyUnicode_FromString("");
				if (self->Port == NULL) {
					Py_XDECREF(self);
					return NULL;
				}

				self->LastSeen = PyUnicode_FromString("");
				if (self->LastSeen == NULL) {
					Py_XDECREF(self);
					return NULL;
				}

				self->Transport = PyUnicode_FromString("");
				if (self->Transport == NULL) {
					Py_XDECREF(self);
					return NULL;
				}

				self->Protocol = PyUnicode_FromString("None");
				if (self->Protocol == NULL) {
					Py_XDECREF(self);
					return NULL;
				}

				self->Parent = (CConnection*)Py_None;
				Py_INCREF(Py_None);

				self->pPlugin = NULL;
				self->pTransport = NULL;
				self->pProtocol = NULL;
			}
		}
		catch (std::exception *e)
		{
			_log.Log(LOG_ERROR, "%s: Execption thrown: %s", __func__, e->what());
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "%s: Unknown execption thrown", __func__);
		}

		return (PyObject *)self;
	}

	int CConnection_init(CConnection * self, PyObject * args, PyObject * kwds)
	{
		char*		pName = NULL;
		char*		pTransport = NULL;
		char*		pProtocol = NULL;
		char*		pAddress = NULL;
		char*		pPort = NULL;
		int			iBaud = -1;
		static char* kwlist[] = { "Name", "Transport", "Protocol", "Address", "Port", "Baud", NULL };

		try
		{
			PyObject*	brModule = PyState_FindModule(&DomoticzModuleDef);
			if (!brModule)
			{
				_log.Log(LOG_ERROR, "CPlugin:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state*	pModState = ((struct module_state*)PyModule_GetState(brModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CPlugin:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CPlugin:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}
			self->pPlugin = pModState->pPlugin;

			if (PyArg_ParseTupleAndKeywords(args, kwds, "ss|sssi", kwlist, &pName, &pTransport, &pProtocol, &pAddress, &pPort, &iBaud))
				{
				if (pName) {
					Py_XDECREF(self->Name);
					self->Name = PyUnicode_FromString(pName);
				}
				if (pAddress) {
					Py_XDECREF(self->Address);
					self->Address = PyUnicode_FromString(pAddress);
				}
				if (pPort) {
					Py_XDECREF(self->Port);
					self->Port = PyUnicode_FromString(pPort);
				}
				if (iBaud) self->Baud = iBaud;
				if (pTransport)
				{
					Py_XDECREF(self->Transport);
					self->Transport = PyUnicode_FromString(pTransport);
				}
				if (pProtocol)
				{
					Py_XDECREF(self->Protocol);
					self->Protocol = PyUnicode_FromString(pProtocol);
					self->pPlugin->MessagePlugin(new ProtocolDirective(self->pPlugin, self));
				}
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				self->pPlugin->WriteToTargetLog((PyObject*)self->pPlugin->m_Interface, "Error", "Expected: myVar = domoticz.Connection(Name=\"<Name>\", Transport=\"<Transport>\", Protocol=\"<Protocol>\", Address=\"<IP-Address>\", Port=\"<Port>\", Baud=0)");
				pPlugin->LogPythonException((PyObject*)pPlugin->m_Interface, __func__);
			}
		}
		catch (std::exception *e)
		{
			_log.Log(LOG_ERROR, "%s: Execption thrown: %s", __func__, e->what());
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "%s: Unknown execption thrown", __func__);
		}

		return 0;
	}

	PyObject * CConnection_connect(CConnection* self, PyObject* args, PyObject* kwds)
	{
		Py_INCREF(Py_None);

		if (!self->pPlugin)
		{
			_log.Log(LOG_ERROR, "%s:, illegal operation, Plugin has not started yet.", __func__);
			return Py_None;
		}

		//	Add connect command to message queue unless already connected
		if (self->pPlugin->IsStopRequested(0))
		{
			self->pPlugin->WriteToTargetLog((PyObject*)self->pPlugin->m_Interface, "Log", "%s, connect request from '%s' ignored. Plugin is stopping.", __func__, self->pPlugin->m_Name.c_str());
			return Py_None;
		}

		if (self->pTransport && self->pTransport->IsConnecting())
		{
			self->pPlugin->WriteToTargetLog((PyObject*)self->pPlugin->m_Interface, "Error", "%s, connect request from '%s' ignored. Transport is connecting.", __func__, self->pPlugin->m_Name.c_str());
			return Py_None;
		}

		if (self->pTransport && self->pTransport->IsConnected())
		{
			self->pPlugin->WriteToTargetLog((PyObject*)self->pPlugin->m_Interface, "Error", "%s, connect request from '%s' ignored. Transport is connected.", __func__, self->pPlugin->m_Name.c_str());
			return Py_None;
		}

		PyObject*	pTarget = NULL;
		int			iTimeout = 0;
		static char* kwlist[] = { "Target", "Timeout", NULL };
		if (PyArg_ParseTupleAndKeywords(args, kwds, "O|I", kwlist, &pTarget, &iTimeout))
		{
			self->Timeout = iTimeout;
			if (pTarget) {
				Py_INCREF(pTarget);
				Py_XDECREF(self->Target);
				self->Target = pTarget;
				self->pPlugin->MessagePlugin(new ConnectDirective(self->pPlugin, self));
			}
			else
			{
				self->pPlugin->WriteToTargetLog((PyObject*)self->pPlugin->m_Interface, "Error", "Connect request not completed, no Event Target specified.");
				self->pPlugin->LogPythonException((PyObject*)self->pPlugin->m_Interface, __func__);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Expected: myVar = Connection.Connect(Target=\"<Object>\")");
			LogPythonException(self->pPlugin, __func__);
		}

		return Py_None;
	}

	PyObject * CConnection_listen(CConnection * self, PyObject* args, PyObject* kwds)
	{
		Py_INCREF(Py_None);

		if (!self->pPlugin)
		{
			_log.Log(LOG_ERROR, "%s:, illegal operation, Plugin has not started yet.", __func__);
			return Py_None;
		}

		//	Add connect command to message queue unless already connected
		if (self->pPlugin->IsStopRequested(0))
		{
			_log.Log(LOG_NORM, "%s, listen request from '%s' ignored. Plugin is stopping.", __func__, self->pPlugin->m_Name.c_str());
			return Py_None;
		}

		if (self->pTransport && self->pTransport->IsConnecting())
		{
			_log.Log(LOG_ERROR, "%s, listen request from '%s' ignored. Transport is connecting.", __func__, self->pPlugin->m_Name.c_str());
			return Py_None;
		}

		if (self->pTransport && self->pTransport->IsConnected())
		{
			_log.Log(LOG_ERROR, "%s, listen request from '%s' ignored. Transport is connected.", __func__, self->pPlugin->m_Name.c_str());
			return Py_None;
		}

		PyObject* pTarget = NULL;
		static char* kwlist[] = { "Target", NULL };
		if (PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &pTarget))
		{
			if (pTarget) {
				Py_INCREF(pTarget);
				Py_XDECREF(self->Target);
				self->Target = pTarget;
				self->pPlugin->MessagePlugin(new ListenDirective(self->pPlugin, self));
			}
			else
			{
				_log.Log(LOG_ERROR, "Listen request not completed, no Event Target specified.");
				LogPythonException(self->pPlugin, __func__);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Expected: myVar = Connection.Listen(Target=\"<Object>\")");
			LogPythonException(self->pPlugin, __func__);
		}

		return Py_None;
	}

	PyObject * CConnection_send(CConnection * self, PyObject * args, PyObject * kwds)
	{
		if (!self->pPlugin)
		{
			_log.Log(LOG_ERROR, "%s:, illegal operation, Plugin has not started yet.", __func__);
		}
		else if (self->pPlugin->IsStopRequested(0))
		{
			self->pPlugin->WriteToTargetLog(self->Target, "Log", "%s, send request from '%s' ignored. Plugin is stopping.", __func__, self->pPlugin->m_Name.c_str());
		}
		else
		{
			PyObject*	pData = NULL;
			int			iDelay = 0;
			static char *kwlist[] = { "Message", "Delay", NULL };
			if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &pData, &iDelay))
			{
				self->pPlugin->WriteToTargetLog(self->Target, "Error", "(%s) failed to parse parameters, Message or Message, Delay expected.", self->pPlugin->m_Name.c_str());
				self->pPlugin->LogPythonException(self->Target, std::string(__func__));
			}
			else
			{
				//	Add start command to message queue
				self->pPlugin->MessagePlugin(new WriteDirective(self->pPlugin, self, pData, iDelay));
			}
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject * CConnection_disconnect(CConnection * self)
	{
		if (self->pTransport)
		{
			if (self->pTransport->IsConnecting() || self->pTransport->IsConnected())
			{
				self->pPlugin->MessagePlugin(new DisconnectDirective(self->pPlugin, self));
			}
			else
				_log.Log(LOG_ERROR, "%s, disconnection request from '%s' ignored. Transport is not connecting or connected.", __func__, self->pPlugin->m_Name.c_str());
		}
		else
			_log.Log(LOG_ERROR, "%s, disconnection request from '%s' ignored. Transport does not exist.", __func__, self->pPlugin->m_Name.c_str());

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject * CConnection_bytes(CConnection * self)
	{
		if (self->pTransport)
		{
			return PyLong_FromLong(self->pTransport->TotalBytes());
		}

		return PyBool_FromLong(0);
	}

	PyObject * CConnection_isconnecting(CConnection * self)
	{
		if (self->pTransport)
		{
			return PyBool_FromLong(self->pTransport->IsConnecting());
		}

		return PyBool_FromLong(0);
	}

	PyObject * CConnection_isconnected(CConnection * self)
	{
		if (self->pTransport)
		{
			return PyBool_FromLong(self->pTransport->IsConnected());
		}

		return PyBool_FromLong(0);
	}

	PyObject * CConnection_timestamp(CConnection * self)
	{
		if (self->pTransport)
		{
			time_t	tLastSeen = self->pTransport->LastSeen();
			struct tm ltime;
			localtime_r(&tLastSeen, &ltime);
			char date[32];
			strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &ltime);
			PyObject* pLastSeen = PyUnicode_FromString(date);
			return pLastSeen;
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject * CConnection_str(CConnection * self)
	{
		std::string		sParent = "None";
		if (((PyObject*)self->Parent) != Py_None)
		{
			sParent = PyUnicode_AsUTF8(self->Parent->Name);
		}

		if (self->pTransport)
		{
			time_t	tLastSeen = self->pTransport->LastSeen();
			struct tm ltime;
			localtime_r(&tLastSeen, &ltime);
			char date[32];
			strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &ltime);
			PyObject*	nrRetVal = PyUnicode_FromFormat("Name: '%U', Transport: '%U', Protocol: '%U', Address: '%U', Port: '%U', Baud: %d, Bytes: %d, Connected: %s, Last Seen: %s, Parent: '%s'",
				self->Name, self->Transport, self->Protocol, self->Address, self->Port, self->Baud,
				(self->pTransport ? self->pTransport->TotalBytes() : -1),
				(self->pTransport ? (self->pTransport->IsConnected() ? "True" : "False") : "False"), date, sParent.c_str());
			return nrRetVal;
		}
		else
		{
			PyObject*	nrRetVal = PyUnicode_FromFormat("Name: '%U', Transport: '%U', Protocol: '%U', Address: '%U', Port: '%U', Baud: %d, Connected: False, Parent: '%s'",
				self->Name, self->Transport, self->Protocol, self->Address, self->Port, self->Baud, sParent.c_str());
			return nrRetVal;
		}
	}

}
