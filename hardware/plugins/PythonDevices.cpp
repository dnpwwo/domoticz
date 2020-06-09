#include "stdafx.h"

#include "PythonDevices.h"
#include "PluginProtocols.h"
#include "PythonValues.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016 - 2020
//
#include "../main/Logger.h"
#include "../main/SQLHelper.h"

namespace Plugins {

	extern struct PyModuleDef DomoticzModuleDef;
	extern void LogPythonException(CPlugin* pPlugin, const std::string& sHandler);

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

	PyObject* CDevice_AddValueToDict(CDevice* self, long lValueID)
	{
		PyObject* pValue = NULL;

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

			PyObjPtr argList = Py_BuildValue("(siiO)", "", -1, -1, PyUnicode_FromString(""));
			if (!argList)
			{
				DeviceLog(self, LOG_ERROR, "Building Value argument list failed for Value %d.", lValueID);
				goto Error;
			}

			// Pass values in, needs to be a tuple and signal CValue_init to load from the database
			pModState->lObjectID = lValueID;
			pValue = PyObject_CallObject((PyObject*)pModState->pValueClass, argList);
			if (!pValue)
			{
				DeviceLog(self, LOG_ERROR, "Value object creation failed for Value %d.", lValueID);
				goto Error;
			}

			// And insert it into the Device's Values dictionary
			PyObjPtr pKey = PyLong_FromLong(lValueID);
			if (PyDict_SetItem(self->Values, pKey, pValue) == -1)
			{
				DeviceLog(self, LOG_ERROR, "Failed to add value number '%d' to Values dictionary for Device %d.", lValueID, self->DeviceID);
				goto Error;
			}
		}
		catch (std::exception* e)
		{
			_log.Log(LOG_ERROR, "CDevice:%s, Execption thrown: %s", __func__, e->what());
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "CDevice:%s, Unknown execption thrown", __func__);
		}

	Error:
		if (PyErr_Occurred())
		{
			self->pPlugin->LogPythonException("CDevice_AddValueToDict");
		}

		// If not NULL the caller will need to decref
		return pValue;
	}

	PyObject* CDevice_FindValue(CDevice* self, long lValueID)
	{
		PyObject* pValue = NULL;

		try
		{
			// If the key is in the dictionary then return a pointer to it
			PyObjPtr pKey = PyLong_FromLong(lValueID);
			pValue = PyDict_GetItem(self->Values, pKey);
			if (pValue)
			{
				Py_INCREF(pValue);
			}
		}
		catch (std::exception* e)
		{
			_log.Log(LOG_ERROR, "CDevice:%s, Execption thrown: %s", __func__, e->what());
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "CDevice:%s, Unknown execption thrown", __func__);
		}

		// If not NULL the caller will need to decref
		return pValue;
	}

	void CDevice_dealloc(CDevice* self)
	{
		Py_XDECREF(self->Name);
		Py_XDECREF(self->ExternalID);
		Py_XDECREF(self->Timestamp);
		PyObject* key, * value;
		Py_ssize_t pos = 0;
		// Sanity check to make sure the reference counbting is all good.
		while (PyDict_Next(self->Values, &pos, &key, &value))
		{
			if (value->ob_refcnt != 1)
			{
				std::string	sName = PyUnicode_AsUTF8(((CValue*)value)->Name);
				_log.Log(LOG_ERROR, "%s: Value '%s' Reference Count not one: %d.", __func__, sName.c_str(), value->ob_refcnt);
			}
		}

		if (PyDict_Size(self->Values))
		{
			PyDict_Clear(self->Values);
		}
		Py_XDECREF(self->Values);
		// Py_XDECREF(self->Parent);	// Reference to owning interface is borrowed so don't release it.
										// Interface will always exist longer than a device, plus they can't point at each other (they could never be deleted)
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
				self->Parent = pModState->pPlugin->m_Interface;			// Borrow a reference to the owning Interface
				CDevice_refresh(self);
				pModState->lObjectID = 0;

				// Set Device as inactive during startup
				if (self->Active)
				{
					self->Active = false;
					CDevice_update(self);
				}

				// Load Values associated with the device
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT ValueID FROM Value WHERE (DeviceID==%d) ORDER BY ValueID ASC", self->DeviceID);
				if (!result.empty())
				{
					PyType_Ready(&CValueType);
					// Create Value objects and add o the Values dictionary with ValueID as the key
					for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
					{
						std::vector<std::string> sd = *itt;
						PyObject* pValue = CDevice_AddValueToDict(self, atoi(sd[0].c_str()));
						Py_XDECREF(pValue);
					}
				}
			}
			else
			{
				if (PyArg_ParseTupleAndKeywords(args, kwds, "ss", kwlist, &szName, &szExternalID))
				{
					self->InterfaceID = pModState->pPlugin->m_InterfaceID;
					self->Parent = pModState->pPlugin->m_Interface;			// Borrow a reference to the owning Interface
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

		if (PyErr_Occurred())
		{
			self->pPlugin->LogPythonException("CDevice_init");
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
						PyObjPtr pSafeAssign;	// Used to make sure fields always have a valid value during update

						self->InterfaceID = atoi(sd[0].c_str());

						pSafeAssign = self->Name;
						self->Name = PyUnicode_FromString(sd[1].c_str());

						pSafeAssign = self->ExternalID;
						self->ExternalID = PyUnicode_FromString(sd[2].c_str());

						self->Debug = atoi(sd[3].c_str()) ? true : false;
						self->Active = atoi(sd[4].c_str()) ? true : false;

						pSafeAssign = self->Timestamp;
						self->Timestamp = PyUnicode_FromString(sd[5].c_str());
					}
				}
			}
			else
			{
				DeviceLog(self, LOG_ERROR, "Invalid Device ID '%d', must be already set.", (long)self->DeviceID);
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
					// Read DeviceID of new device and update the object
					std::vector<std::vector<std::string>> result = m_sql.safe_query("SELECT MAX(DeviceID) FROM Device WHERE InterfaceID = %d AND Name = '%q'", self->InterfaceID, sName.c_str());
					for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
					{
						std::vector<std::string> sd = *itt;
						self->DeviceID = atoi(sd[0].c_str());
					}

					_log.Log(LOG_NORM, "Insert into 'Device' succeeded with ID %d, %d record(s) created.", self->DeviceID, iRowCount);
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Invalid Device ID '%d', must not be already set.", self->pPlugin->m_Name.c_str(), (long)self->DeviceID);
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
			if (self->DeviceID != -1)
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
					std::string	sName = PyUnicode_AsUTF8(self->Name);
					_log.Log(LOG_NORM, "Update to Device '%s' succeeded, %d records updated.", sName.c_str(), iRowCount);
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Invalid Device ID '%d', must already be set.", self->pPlugin->m_Name.c_str(), (long)self->DeviceID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Device creation failed, Device object is not associated with an interface.");
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
		PyObject* pRetVal = PyUnicode_FromFormat("DeviceID: %d, Name: %U, InterfaceID: %d, ExternalID: %U, Active: '%s', Timestamp: %U",
			self->DeviceID, self->Name, self->InterfaceID, self->ExternalID, (self->Active?"True":"False"), self->Timestamp);
		return pRetVal;
	}
}