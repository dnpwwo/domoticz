#include "stdafx.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016
//
#ifdef ENABLE_PYTHON

#include "../main/Logger.h"
#include "../main/SQLHelper.h"
#include "../main/localtime_r.h"
#include "../main/mainstructs.h"
#include "../main/mainworker.h"
#include "PythonObjects.h"
#include "PluginMessages.h"
#include "PluginProtocols.h"
#include "PluginTransports.h"
#include <datetime.h>

namespace Plugins {

	extern struct PyModuleDef DomoticzModuleDef;
	extern void LogPythonException(CPlugin* pPlugin, const std::string &sHandler);

	struct module_state {
		CPlugin*	pPlugin;
		PyObject*	error;
	};

	void CImage_dealloc(CImage* self)
	{
		Py_XDECREF(self->Base);
		Py_XDECREF(self->Name);
		Py_XDECREF(self->Description);
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyObject* CImage_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		CImage *self = (CImage *)type->tp_alloc(type, 0);

		try
		{
			if (self == NULL) {
				_log.Log(LOG_ERROR, "%s: Self is NULL.", __func__);
			}
			else {
				self->ImageID = -1;
				self->Base = PyUnicode_FromString("");
				if (self->Base == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Name = PyUnicode_FromString("");
				if (self->Name == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Description = PyUnicode_FromString("");
				if (self->Description == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Filename = PyUnicode_FromString("");
				if (self->Filename == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->pPlugin = NULL;
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

	int CImage_init(CImage *self, PyObject *args, PyObject *kwds)
	{
		char*		szFileName = NULL;
		static char *kwlist[] = { "Filename", NULL };

		try
		{
			PyObject*	pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CImage:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state*	pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CImage:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CImage:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}

			if (PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &szFileName))
			{
				std::string	sFileName = szFileName ? szFileName : "";

				if (sFileName.length())
				{
					self->pPlugin = pModState->pPlugin;
					sFileName = self->pPlugin->m_HomeFolder + sFileName;
					Py_DECREF(self->Filename);
					self->Filename = PyUnicode_FromString(sFileName.c_str());
				}
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				_log.Log(LOG_ERROR, "Expected: myVar = Domoticz.Image(Filename=\"MyImages.zip\")");
				LogPythonException(pPlugin, __func__);
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

	PyObject* CImage_insert(CImage* self, PyObject *args)
	{
		if (self->pPlugin)
		{
			std::string	sName = PyUnicode_AsUTF8(self->Name);
			std::string	sFilename = PyUnicode_AsUTF8(self->Filename);
			if (self->ImageID == -1)
			{
				if (sFilename.length())
				{
					if (self->pPlugin->m_bDebug & PDM_IMAGE)
					{
						_log.Log(LOG_NORM, "(%s) Creating images from file '%s'.", self->pPlugin->m_Name.c_str(), sFilename.c_str());
					}

					//
					//	Call code to do insert here
					//
					std::string ErrorMessage;
					//if (!m_sql.InsertCustomIconFromZipFile(sFilename, ErrorMessage))
					{
						_log.Log(LOG_ERROR, "(%s) Insert Custom Icon From Zip failed on file '%s' with error '%s'.", self->pPlugin->m_Name.c_str(), sFilename.c_str(), ErrorMessage.c_str());
					}
					//else
					{
						// load associated custom images to make them available to python
						std::vector<std::vector<std::string> > result = m_sql.safe_query("SELECT max(ID), Base, Name, Description FROM CustomImages");
						if (!result.empty())
						{
							PyType_Ready(&CImageType);
							// Add image objects into the image dictionary with ID as the key
							for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
							{
								std::vector<std::string> sd = *itt;
								CImage* pImage = (CImage*)CImage_new(&CImageType, (PyObject*)NULL, (PyObject*)NULL);

								PyObject*	pKey = PyUnicode_FromString(sd[1].c_str());
								if (PyDict_SetItem((PyObject*)self->pPlugin->m_ImageDict, pKey, (PyObject*)pImage) == -1)
								{
									_log.Log(LOG_ERROR, "(%s) failed to add ID '%s' to image dictionary.", self->pPlugin->m_PluginKey.c_str(), sd[0].c_str());
									break;
								}
								else
								{
									pImage->ImageID = atoi(sd[0].c_str()) + 100;
									pImage->Base = PyUnicode_FromString(sd[1].c_str());
									pImage->Name = PyUnicode_FromString(sd[2].c_str());
									pImage->Description = PyUnicode_FromString(sd[3].c_str());
									Py_DECREF(pImage);
								}
							}
						}
					}
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) No images loaded.", self->pPlugin->m_Name.c_str());
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Image creation failed, '%s' already exists in Domoticz with Image ID '%d'.", self->pPlugin->m_Name.c_str(), sName.c_str(), self->ImageID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Image creation failed, Image object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CImage_delete(CImage* self, PyObject *args)
	{
		if (self->pPlugin)
		{
			std::string	sName = PyUnicode_AsUTF8(self->Name);
			if (self->ImageID != -1)
			{
				if (self->pPlugin->m_bDebug & PDM_IMAGE)
				{
					_log.Log(LOG_NORM, "(%s) Deleting Image '%s'.", self->pPlugin->m_Name.c_str(), sName.c_str());
				}

				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT Name FROM CustomImages WHERE (ID==%d)", self->ImageID);
				if (!result.empty())
				{
					m_sql.safe_query("DELETE FROM CustomImages WHERE (ID==%d)", self->ImageID);

					PyObject*	pKey = PyLong_FromLong(self->ImageID);
					if (PyDict_DelItem((PyObject*)self->pPlugin->m_ImageDict, pKey) == -1)
					{
						_log.Log(LOG_ERROR, "(%s) failed to delete image '%d' from images dictionary.", self->pPlugin->m_Name.c_str(), self->ImageID);
						Py_INCREF(Py_None);
						return Py_None;
					}
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) Image deletion failed, Image %d not found in Domoticz.", self->pPlugin->m_Name.c_str(), self->ImageID);
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Image deletion failed, '%s' does not represent a Image in Domoticz.", self->pPlugin->m_Name.c_str(), sName.c_str());
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Image deletion failed, Image object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CImage_str(CImage* self)
	{
		PyObject*	pRetVal = PyUnicode_FromFormat("ID: %d, Base: '%U', Name: %U, Description: '%U'", self->ImageID, self->Base, self->Name, self->Description);
		return pRetVal;
	}

	void CConnection_dealloc(CConnection * self)
	{
		if (self->pPlugin && (self->pPlugin->m_bDebug & PDM_CONNECTION))
		{
			_log.Log(LOG_NORM, "(%s) Deallocating connection object '%s' (%s:%s).", self->pPlugin->m_Name.c_str(), PyUnicode_AsUTF8(self->Name), PyUnicode_AsUTF8(self->Address), PyUnicode_AsUTF8(self->Port));
		}

		Py_XDECREF(self->Address);
		Py_XDECREF(self->Port);
		Py_XDECREF(self->LastSeen);
		Py_XDECREF(self->Transport);
		Py_XDECREF(self->Protocol);
		Py_XDECREF(self->Parent);

		if (self->pTransport)
		{
			delete self->pTransport;
			self->pTransport = NULL;
		}
		if (self->pProtocol)
		{
			delete self->pProtocol;
			self->pProtocol = NULL;
		}

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
					Py_DECREF(self);
					return NULL;
				}
				self->Address = PyUnicode_FromString("");
				if (self->Address == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Port = PyUnicode_FromString("");
				if (self->Port == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->LastSeen = PyUnicode_FromString("");
				if (self->LastSeen == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Transport = PyUnicode_FromString("");
				if (self->Transport == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Protocol = PyUnicode_FromString("None");
				if (self->Protocol == NULL) {
					Py_DECREF(self);
					return NULL;
				}

				self->Parent = Py_None;
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
		static char *kwlist[] = { "Name", "Transport", "Protocol", "Address", "Port", "Baud", NULL };

		try
		{
			PyObject*	pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CPlugin:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state*	pModState = ((struct module_state*)PyModule_GetState(pModule));
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

			if (PyArg_ParseTupleAndKeywords(args, kwds, "ss|sssi", kwlist, &pName, &pTransport, &pProtocol, &pAddress, &pPort, &iBaud))
			{
				self->pPlugin = pModState->pPlugin;
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
					self->pPlugin->MessagePlugin(new ProtocolDirective(self->pPlugin, (PyObject*)self));
				}
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				_log.Log(LOG_ERROR, "Expected: myVar = Domoticz.Connection(Name=\"<Name>\", Transport=\"<Transport>\", Protocol=\"<Protocol>\", Address=\"<IP-Address>\", Port=\"<Port>\", Baud=0)");
				LogPythonException(pPlugin, __func__);
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

	PyObject * CConnection_connect(CConnection * self)
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
			_log.Log(LOG_NORM, "%s, connect request from '%s' ignored. Plugin is stopping.", __func__, self->pPlugin->m_Name.c_str());
			return Py_None;
		}

		if (self->pTransport && self->pTransport->IsConnecting())
		{
			_log.Log(LOG_ERROR, "%s, connect request from '%s' ignored. Transport is connecting.", __func__, self->pPlugin->m_Name.c_str());
			return Py_None;
		}

		if (self->pTransport && self->pTransport->IsConnected())
		{
			_log.Log(LOG_ERROR, "%s, connect request from '%s' ignored. Transport is connected.", __func__, self->pPlugin->m_Name.c_str());
			return Py_None;
		}

		self->pPlugin->MessagePlugin(new ConnectDirective(self->pPlugin, (PyObject*)self));

		return Py_None;
	}

	PyObject * CConnection_listen(CConnection * self)
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

		self->pPlugin->MessagePlugin(new ListenDirective(self->pPlugin, (PyObject*)self));

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
			_log.Log(LOG_NORM, "%s, send request from '%s' ignored. Plugin is stopping.", __func__, self->pPlugin->m_Name.c_str());
		}
		else
		{
			PyObject*	pData = NULL;
			int			iDelay = 0;
			static char *kwlist[] = { "Message", "Delay", NULL };
			if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &pData, &iDelay))
			{
				_log.Log(LOG_ERROR, "(%s) failed to parse parameters, Message or Message, Delay expected.", self->pPlugin->m_Name.c_str());
				LogPythonException(self->pPlugin, std::string(__func__));
			}
			else
			{
				//	Add start command to message queue
				self->pPlugin->MessagePlugin(new WriteDirective(self->pPlugin, (PyObject*)self, pData, iDelay));
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
				self->pPlugin->MessagePlugin(new DisconnectDirective(self->pPlugin, (PyObject*)self));
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
		if (self->Parent != Py_None)
		{
			sParent = PyUnicode_AsUTF8(((CConnection*)self->Parent)->Name);
		}

		if (self->pTransport)
		{
			time_t	tLastSeen = self->pTransport->LastSeen();
			struct tm ltime;
			localtime_r(&tLastSeen, &ltime);
			char date[32];
			strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &ltime);
			PyObject*	pRetVal = PyUnicode_FromFormat("Name: '%U', Transport: '%U', Protocol: '%U', Address: '%U', Port: '%U', Baud: %d, Bytes: %d, Connected: %s, Last Seen: %s, Parent: '%s'",
				self->Name, self->Transport, self->Protocol, self->Address, self->Port, self->Baud,
				(self->pTransport ? self->pTransport->TotalBytes() : -1),
				(self->pTransport ? (self->pTransport->IsConnected() ? "True" : "False") : "False"), date, sParent.c_str());
			return pRetVal;
		}
		else
		{
			PyObject*	pRetVal = PyUnicode_FromFormat("Name: '%U', Transport: '%U', Protocol: '%U', Address: '%U', Port: '%U', Baud: %d, Connected: False, Parent: '%s'",
				self->Name, self->Transport, self->Protocol, self->Address, self->Port, self->Baud, sParent.c_str());
			return pRetVal;
		}
	}

}
#endif
