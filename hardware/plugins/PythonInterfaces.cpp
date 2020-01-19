#include "stdafx.h"

#include "PythonDevices.h"

#include "PythonInterfaces.h"
#include "PluginProtocols.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016 - 2020
//
#include "../main/Logger.h"
#include "../main/SQLHelper.h"

namespace Plugins {

	extern struct PyModuleDef DomoticzModuleDef;
	extern void LogPythonException(CPlugin* pPlugin, const std::string& sHandler);

	void InterfaceLog(CInterface* self, const _eLogLevel level, const char* Message, ...)
	{
		va_list argList;
		char cbuffer[1024];
		va_start(argList, Message);
		vsnprintf(cbuffer, sizeof(cbuffer), Message, argList);
		va_end(argList);

		// Firstly do standard log behaviour
		_log.Log(level, cbuffer);

		// Build SQL statement
		std::string sMessage = cbuffer;
		std::string		sSQL = "INSERT INTO InterfaceLog (InterfaceID, Message) VALUES(?,?);";
		std::vector<std::string> vValues;
		vValues.push_back(std::to_string(self->InterfaceID));
		if (level == LOG_DEBUG_INT)
		{
			sMessage = "DEBUG: " + sMessage;
			if (!self->Debug)
			{
				return;  // we aren't debugging so bail out
			}
		}
		else if (level == LOG_ERROR)
		{
			sMessage = "ERROR: " + sMessage;
		}
		vValues.push_back(sMessage);

		// Execute the statement
		int		iRowCount = m_sql.execute_sql(sSQL, &vValues, true);
		if (!iRowCount)
		{
			_log.Log(LOG_ERROR, "Insert into 'InterfaceLog' failed to create a record.");
		}
	}

	PyObject* CInterface_debug(CInterface* self, PyObject* args, PyObject* kwds)
	{
		char* szMessage = NULL;
		static char* kwlist[] = { "Message", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CInterface:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CInterface:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CInterface:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}

			if (PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &szMessage))
			{
				std::string	sMessage = szMessage ? szMessage : "No message specifed.";
				InterfaceLog(self, LOG_DEBUG_INT, sMessage.c_str());
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				InterfaceLog(self, LOG_ERROR, "Expected: Interface.Log(Message=\"\")");
				LogPythonException(pPlugin, __func__);
			}
		}
		catch (std::exception* e)
		{
			_log.Log(LOG_ERROR, "%s: Execption thrown: %s", __func__, e->what());
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "%s: Unknown execption thrown", __func__);
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CInterface_log(CInterface* self, PyObject* args, PyObject* kwds)
	{
		char* szMessage = NULL;
		static char* kwlist[] = { "Message", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CInterface:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CInterface:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CInterface:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}

			if (PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &szMessage))
			{
				std::string	sMessage = szMessage ? szMessage : "No message specifed.";
				InterfaceLog(self, LOG_NORM, sMessage.c_str());
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				InterfaceLog(self, LOG_ERROR, "Expected: Interface.Log(Message=\"\")");
				LogPythonException(pPlugin, __func__);
			}
		}
		catch (std::exception* e)
		{
			_log.Log(LOG_ERROR, "%s: Execption thrown: %s", __func__, e->what());
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "%s: Unknown execption thrown", __func__);
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CInterface_error(CInterface* self, PyObject* args, PyObject* kwds)
	{
		char* szMessage = NULL;
		static char* kwlist[] = { "Message", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CInterface:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CInterface:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CInterface:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}

			if (PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &szMessage))
			{
				std::string	sMessage = szMessage ? szMessage : "No message specifed.";
				InterfaceLog(self, LOG_ERROR, sMessage.c_str());
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				InterfaceLog(self, LOG_ERROR, "Expected: Interface.Log(Message=\"\")");
				LogPythonException(pPlugin, __func__);
			}
		}
		catch (std::exception* e)
		{
			_log.Log(LOG_ERROR, "%s: Execption thrown: %s", __func__, e->what());
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "%s: Unknown execption thrown", __func__);
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	void CInterface_dealloc(CInterface* self)
	{
		Py_XDECREF(self->Name);
		Py_XDECREF(self->Configuration);
		if (PyDict_Size(self->Devices))
		{
			PyDict_Clear(self->Devices);
		}
		Py_XDECREF(self->Devices);
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyObject* CInterface_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
	{
		CInterface* self = (CInterface*)type->tp_alloc(type, 0);

		try
		{
			if (self == NULL) {
				_log.Log(LOG_ERROR, "%s: Self is NULL.", __func__);
			}
			else {
				self->InterfaceID = -1;
				self->Name = PyUnicode_FromString("");
				if (self->Name == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Configuration = PyDict_New();
				if (self->Configuration == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Debug = false;
				self->Active = false;
				self->Devices = PyDict_New();
				self->pPlugin = NULL;
			}
		}
		catch (std::exception* e)
		{
			_log.Log(LOG_ERROR, "%s: Execption thrown: %s", __func__, e->what());
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "%s: Unknown execption thrown", __func__);
		}

		return (PyObject*)self;
	}

	int CInterface_init(CInterface* self, PyObject* args, PyObject* kwds)
	{
		char* szName = NULL;
		char* szConfiguration = NULL;
		static char* kwlist[] = { "Name", "Configuration", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				InterfaceLog(self, LOG_ERROR, "CInterface:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				InterfaceLog(self, LOG_ERROR, "CInterface:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				InterfaceLog(self, LOG_ERROR, "CInterface:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}
			self->pPlugin = pModState->pPlugin;

			// During startup plugin sets this to signal load from database 
			if (pModState->lObjectID)
			{
				self->InterfaceID = pModState->lObjectID;
				CInterface_refresh(self);
				pModState->lObjectID = 0;

				// load associated devices to make them available to python
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT DeviceID FROM Device WHERE (InterfaceID==%d) ORDER BY DeviceID ASC", self->InterfaceID);
				if (!result.empty())
				{
					PyType_Ready(&CDeviceType);
					// Add device objects into the device dictionary with DeviceID as the key
					for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
					{
						std::vector<std::string> sd = *itt;
						long lDeviceID = atoi(sd[0].c_str());

						// Pass values in, needs to be a tuple and signal CDevice_init to load from the database
						pModState->lObjectID = lDeviceID;
						PyObject* argList = Py_BuildValue("(ss)", "", "");
						if (!argList)
						{
							InterfaceLog(self, LOG_ERROR, "Building Device argument list failed for Device %d.", lDeviceID);
							goto Error;
						}

						// Call the class object, this will call new followed by init
						PyObject* pDevice = PyObject_CallObject((PyObject*)pModState->pDeviceClass, argList);
						Py_DECREF(argList);
						if (!pDevice)
						{
							InterfaceLog(self, LOG_ERROR, "Device object creation failed for Device %d.", lDeviceID);
							goto Error;
						}

						// And insert it into the Interface's Devices dictionary
						PyObject* pKey = PyLong_FromLong(lDeviceID);
						if (PyDict_SetItem(self->Devices, pKey, pDevice) == -1)
						{
							InterfaceLog(self, LOG_ERROR, "Failed to add device number '%d' to device dictionary.", lDeviceID);
							goto Error;
						}
						Py_DECREF(pKey);
						Py_DECREF(pDevice);
					}
				}
			}
			else
			{
				InterfaceLog(self, LOG_ERROR, "CInterface:%s, illegal operation, Interface objects can not be created programmatically.", __func__);
				return 0;
			}
		}
		catch (std::exception* e)
		{
			InterfaceLog(self, LOG_ERROR, "%s: Execption thrown: %s", __func__, e->what());
		}
		catch (...)
		{
			InterfaceLog(self, LOG_ERROR, "%s: Unknown execption thrown", __func__);
		}

	Error:
		if (PyErr_Occurred())
		{
			self->pPlugin->LogPythonException("Start");
		}
		return 0;
	}

	PyObject* CInterface_refresh(CInterface* self)
	{
		if (self->pPlugin)
		{
			if (self->InterfaceID >= 1)
			{
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT InterfaceID, Name, Configuration, Debug, Active FROM Interface WHERE InterfaceID = %d", self->InterfaceID);

				// Handle any data we get back
				if (result.empty())
				{
					InterfaceLog(self, LOG_ERROR, "Refresh of 'Interface' failed to read any records for ID %d", self->InterfaceID);
				}
				else
				{
					InterfaceLog(self, LOG_DEBUG_INT, "Refresh of 'Interface' succeeded for ID %d", self->InterfaceID);
					for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
					{
						std::vector<std::string> sd = *itt;
						PyObject* pSafeAssign;

						self->InterfaceID = atoi(sd[0].c_str());
						pSafeAssign = self->Name;
						self->Name = PyUnicode_FromString(sd[1].c_str());
						Py_XDECREF(pSafeAssign);

						pSafeAssign = self->Configuration;
						CPluginProtocolJSON	pJSON;
						self->Configuration = pJSON.JSONtoPython(sd[2]);
						Py_ssize_t iSize = PyDict_Size(self->Configuration);
						if (!iSize)
						{
							InterfaceLog(self, LOG_DEBUG_INT, "No configuration loaded or parse error.");
						}
						Py_XDECREF(pSafeAssign);

						self->Debug = atoi(sd[3].c_str()) ? true : false;
						self->Active = atoi(sd[4].c_str()) ? true : false;
					}
				}
			}
			else
			{
				InterfaceLog(self, LOG_ERROR, "Invalid Interface ID '%d', must be already set.", (long)self->InterfaceID);
			}
		}
		else
		{
			InterfaceLog(self, LOG_ERROR, "Interface refresh failed, Interface object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CInterface_update(CInterface* self)
	{
		if (self->pPlugin)
		{
			if (self->InterfaceID <= 1)
			{
				std::string		sSQL = "UPDATE Interface SET Active=? WHERE InterfaceID=" + std::to_string(self->InterfaceID) + ";";
				std::vector<std::string> vValues;
				vValues.push_back(std::to_string(self->Active));
				int		iRowCount = m_sql.execute_sql(sSQL, &vValues, true);

				// Handle any data we get back
				if (!iRowCount)
				{
					InterfaceLog(self, LOG_ERROR, "Update to 'Interface' failed to update any records for ID %d", self->InterfaceID);
				}
				else
				{
					_log.Log(LOG_NORM, "Update to 'Interface' succeeded, %d records updated.", iRowCount);
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Invalid Interface ID '%d', must not be already set.", self->pPlugin->m_Name.c_str(), (long)self->InterfaceID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Interface creation failed, Interface object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CInterface_str(CInterface* self)
	{
		PyObject* pRetVal = PyUnicode_FromFormat("ID: %d, Name: %U, InterfaceID: %d, Configuration: %U, Active: '%s'",
			self->InterfaceID, self->Name, self->InterfaceID, self->Configuration, (self->Active?"True":"False"));
		return pRetVal;
	}
}