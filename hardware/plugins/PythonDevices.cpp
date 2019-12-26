#include "stdafx.h"

#include "PythonDevices.h"
#include "PluginProtocols.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016 - 2020
//
#include "../main/Logger.h"
#include "../main/SQLHelper.h"

namespace Plugins {

	extern struct PyModuleDef DomoticzModuleDef;
	extern void LogPythonException(CPlugin* pPlugin, const std::string& sHandler);

	struct module_state {
		CPlugin* pPlugin;
		PyObject* error;
		PyObject* pInterfaceClass;
		PyObject* pDeviceClass;
		PyObject* pValueClass;
		PyObject* pConnectionClass;
		long		lObjectID;
	};

	void DeviceLog(CDevice* self, const _eLogLevel level, const char* Message, ...)
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
		std::string		sSQL = "INSERT INTO DeviceLog (DeviceID, Message) VALUES(?,?);";
		std::vector<std::string> vValues;
		vValues.push_back(std::to_string(self->DeviceID));
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
			_log.Log(LOG_ERROR, "Insert into 'DeviceLog' failed to create a record.");
		}
	}

	PyObject* CDevice_debug(CDevice* self, PyObject* args, PyObject* kwds)
	{
		char* szMessage = NULL;
		static char* kwlist[] = { "Message", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}

			if (PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &szMessage))
			{
				std::string	sMessage = szMessage ? szMessage : "No message specifed.";
				DeviceLog(self, LOG_DEBUG_INT, sMessage.c_str());
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				DeviceLog(self, LOG_ERROR, "Expected: Device.Log(Message=\"\")");
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

	PyObject* CDevice_log(CDevice* self, PyObject* args, PyObject* kwds)
	{
		char* szMessage = NULL;
		static char* kwlist[] = { "Message", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}

			if (PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &szMessage))
			{
				std::string	sMessage = szMessage ? szMessage : "No message specifed.";
				DeviceLog(self, LOG_NORM, sMessage.c_str());
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				DeviceLog(self, LOG_ERROR, "Expected: Device.Log(Message=\"\")");
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

	PyObject* CDevice_error(CDevice* self, PyObject* args, PyObject* kwds)
	{
		char* szMessage = NULL;
		static char* kwlist[] = { "Message", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}

			if (PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &szMessage))
			{
				std::string	sMessage = szMessage ? szMessage : "No message specifed.";
				DeviceLog(self, LOG_ERROR, sMessage.c_str());
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				DeviceLog(self, LOG_ERROR, "Expected: Device.Log(Message=\"\")");
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

	void CDevice_dealloc(CDevice* self)
	{
		Py_XDECREF(self->Name);
		Py_XDECREF(self->ExternalID);
		Py_XDECREF(self->Timestamp);
		Py_XDECREF(self->Values);
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyObject* CDevice_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
	{
		CDevice* self = (CDevice*)type->tp_alloc(type, 0);

		try
		{
			if (self == NULL) {
				_log.Log(LOG_ERROR, "%s: Self is NULL.", __func__);
			}
			else {
				self->DeviceID = -1;
				self->InterfaceID = -1;
				self->Name = PyUnicode_FromString("");
				if (self->Name == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->ExternalID = PyUnicode_FromString("");
				if (self->ExternalID == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Debug = false;
				self->Active = false;
				self->Timestamp = PyUnicode_FromString("");
				if (self->Timestamp == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Values = PyDict_New();
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

	int CDevice_init(CDevice* self, PyObject* args, PyObject* kwds)
	{
		char* szName = NULL;
		char* szExternalID = NULL;
		static char* kwlist[] = { "Name", "ExternalID", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}
			self->pPlugin = pModState->pPlugin;

			// During startup plugin sets this to signal load from database 
			if (pModState->lObjectID)
			{
				self->DeviceID = pModState->lObjectID;
				CDevice_refresh(self);
				pModState->lObjectID = 0;
			}
			else
			{
				if (PyArg_ParseTupleAndKeywords(args, kwds, "ss", kwlist, &szName, &szExternalID))
				{
					self->InterfaceID = pModState->pPlugin->m_InterfaceID;
					std::string	sName = szName ? szName : "";
					if (sName.length())
					{
						Py_DECREF(self->Name);
						self->Name = PyUnicode_FromString(sName.c_str());
					}
					std::string	sExternalID = szExternalID ? szExternalID : "";
					if (sExternalID.length())
					{
						Py_DECREF(self->ExternalID);
						self->ExternalID = PyUnicode_FromString(sExternalID.c_str());
					}
				}
				else
				{
					CPlugin* pPlugin = NULL;
					if (pModState) pPlugin = pModState->pPlugin;
					_log.Log(LOG_ERROR, "Expected: myDevice = domoticz.Device(Name=\"myDevice\", DeviceID=1, UnitID=1, Device=\"1.0\")");
					LogPythonException(pPlugin, __func__);
				}
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

		return 0;
	}

	PyObject* CDevice_refresh(CDevice* self)
	{
		if (self->pPlugin)
		{
			if (self->DeviceID >= 1)
			{
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT InterfaceID, Name, ExternalID, Debug, Active, Timestamp FROM Device WHERE DeviceID = %d", self->DeviceID);

				// Handle any data we get back
				if (result.empty())
				{
					DeviceLog(self, LOG_ERROR, "Refresh of 'Device' failed to read any records for ID %d", self->DeviceID);
				}
				else
				{
					DeviceLog(self, LOG_DEBUG_INT, "Refresh of 'Device' succeeded for ID %d", self->DeviceID);
					for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
					{
						std::vector<std::string> sd = *itt;
						self->InterfaceID = atoi(sd[0].c_str());
						Py_XDECREF(self->Name);
						self->Name = PyUnicode_FromString(sd[1].c_str());
						Py_XDECREF(self->ExternalID);
						self->ExternalID = PyUnicode_FromString(sd[2].c_str());
						self->Debug = atoi(sd[3].c_str()) ? true : false;
						self->Active = atoi(sd[4].c_str()) ? true : false;
						Py_XDECREF(self->Timestamp);
						self->Timestamp = PyUnicode_FromString(sd[5].c_str());
					}
				}
			}
			else
			{
				DeviceLog(self, LOG_ERROR, "Invalid Device ID '%d', must not be already set.", self->DeviceID);
			}
		}
		else
		{
			DeviceLog(self, LOG_ERROR, "Device refresh failed, Device object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CDevice_insert(CDevice* self)
	{
		if (self->pPlugin)
		{
			if (self->DeviceID <= 1)
			{
				std::string		sSQL = "INSERT INTO Device (InterfaceID, Name, ExternalID, Debug, Active) VALUES (?,?,?,?,?);";
				std::vector<std::string> vValues;
				vValues.push_back(std::to_string(self->InterfaceID));
				std::string	sName = PyUnicode_AsUTF8(self->Name);
				vValues.push_back(sName);
				std::string	sExternalID = PyUnicode_AsUTF8(self->ExternalID);
				vValues.push_back(sExternalID);
				vValues.push_back(std::to_string(self->Debug));
				vValues.push_back(std::to_string(self->Active));
				int		iRowCount = m_sql.execute_sql(sSQL, &vValues, true);

				// Handle any data we get back
				if (!iRowCount)
				{
					DeviceLog(self, LOG_ERROR, "Insert into 'Device' failed to create any records for ID %d", self->DeviceID);
				}
				else
				{
					_log.Log(LOG_NORM, "Insert into 'Device' succeeded, %d records created.", iRowCount);
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Invalid Device ID '%d', must not be already set.", self->pPlugin->m_Name.c_str(), self->DeviceID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Device creation failed, Device object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CDevice_update(CDevice* self)
	{
		if (self->pPlugin)
		{
			if (self->DeviceID <= 1)
			{
				std::string		sSQL = "UPDATE Device SET Active=?, Timestamp=CURRENT_TIMESTAMP WHERE DeviceID=" + std::to_string(self->DeviceID) + ";";
				std::vector<std::string> vValues;
				vValues.push_back(std::to_string(self->Active));
				int		iRowCount = m_sql.execute_sql(sSQL, &vValues, true);

				// Handle any data we get back
				if (!iRowCount)
				{
					DeviceLog(self, LOG_ERROR, "Update to 'Device' failed to update any records for ID %d", self->DeviceID);
				}
				else
				{
					_log.Log(LOG_NORM, "Update to 'Device' succeeded, %d records updated.", iRowCount);
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Invalid Device ID '%d', must not be already set.", self->pPlugin->m_Name.c_str(), self->DeviceID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Device creation failed, Device object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CDevice_delete(CDevice* self)
	{
		if (self->pPlugin)
		{
			std::string	sName = PyUnicode_AsUTF8(self->Name);
			if (self->DeviceID != -1)
			{
				// Build SQL statement
				std::string		sSQL = "DELETE FROM Device WHERE DeviceID = " + std::to_string(self->DeviceID) + ";";
				m_sql.safe_query(sSQL.c_str());

				// Handle any data we get back
				std::vector<std::vector<std::string> >	result = m_sql.safe_query("SELECT changes();");
				if (result.empty())
				{
					DeviceLog(self, LOG_ERROR, "Delete from 'Device' failed to delete any records for ID %d", self->DeviceID);
				}
				else
				{
					_log.Log(LOG_NORM, "Delete from 'Device' succeeded, %s records removed.", result[0][0].c_str());
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Device deletion failed, '%d' does not represent a Device identifier.", self->pPlugin->m_Name.c_str(), self->DeviceID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Device deletion failed, Device object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CDevice_str(CDevice* self)
	{
		PyObject* pRetVal = PyUnicode_FromFormat("ID: %d, Name: %U, InterfaceID: %d, ExternalID: %U, Active: '%s', Timestamp: %U",
			self->DeviceID, self->Name, self->InterfaceID, self->ExternalID, (self->Active?"True":"False"), self->Timestamp);
		return pRetVal;
	}
}