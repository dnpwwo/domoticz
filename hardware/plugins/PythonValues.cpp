#include "stdafx.h"

#include "PythonValues.h"
#include "PluginProtocols.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016 - 2020
//
#include "../main/Logger.h"
#include "../main/SQLHelper.h"

namespace Plugins {

	extern struct PyModuleDef DomoticzModuleDef;
	extern void LogPythonException(CPlugin* pPlugin, const std::string& sHandler);

	void ValueLog(CValue* self, const _eLogLevel level, const char* Message, ...)
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
		std::string		sSQL = "INSERT INTO ValueLog (ValueID, Message) VALUES(?,?);";
		std::vector<std::string> vValues;
		vValues.push_back(std::to_string(self->ValueID));
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
			_log.Log(LOG_ERROR, "Insert into 'ValueLog' failed to create a record.");
		}
	}

	PyObject* CValue_debug(CValue* self, PyObject* args, PyObject* kwds)
	{
		char* szMessage = NULL;
		static char* kwlist[] = { "Message", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CValue:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CValue:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CValue:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}

			if (PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &szMessage))
			{
				std::string	sMessage = szMessage ? szMessage : "No message specifed.";
				ValueLog(self, LOG_DEBUG_INT, sMessage.c_str());
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				ValueLog(self, LOG_ERROR, "Expected: Value.Log(Message=\"\")");
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

	PyObject* CValue_log(CValue* self, PyObject* args, PyObject* kwds)
	{
		char* szMessage = NULL;
		static char* kwlist[] = { "Message", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CValue:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CValue:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CValue:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}

			if (PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &szMessage))
			{
				std::string	sMessage = szMessage ? szMessage : "No message specifed.";
				ValueLog(self, LOG_NORM, sMessage.c_str());
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				ValueLog(self, LOG_ERROR, "Expected: Value.Log(Message=\"\")");
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

	PyObject* CValue_error(CValue* self, PyObject* args, PyObject* kwds)
	{
		char* szMessage = NULL;
		static char* kwlist[] = { "Message", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CValue:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CValue:%s, unable to obtain module state.", __func__);
				return 0;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CValue:%s, illegal operation, Plugin has not started yet.", __func__);
				return 0;
			}

			if (PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &szMessage))
			{
				std::string	sMessage = szMessage ? szMessage : "No message specifed.";
				ValueLog(self, LOG_ERROR, sMessage.c_str());
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				ValueLog(self, LOG_ERROR, "Expected: Value.Log(Message=\"\")");
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

	void CValue_dealloc(CValue* self)
	{
		Py_XDECREF(self->Name);
		Py_XDECREF(self->Value);
		Py_XDECREF(self->Timestamp);
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyObject* CValue_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
	{
		CValue* self = (CValue*)type->tp_alloc(type, 0);

		try
		{
			if (self == NULL) {
				_log.Log(LOG_ERROR, "%s: Self is NULL.", __func__);
			}
			else {
				self->ValueID = -1;
				self->DeviceID = -1;
				self->Name = PyUnicode_FromString("");
				if (self->Name == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->UnitID = -1;
				self->Value = PyUnicode_FromString("");
				if (self->Value == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Debug = false;
				self->Timestamp = PyUnicode_FromString("");
				if (self->Timestamp == NULL) {
					Py_DECREF(self);
					return NULL;
				}
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

	int CValue_init(CValue* self, PyObject* args, PyObject* kwds)
	{
		char*	szName = NULL;
		long	lDeviceID = -1;
		long	lUnitID = -1;
		PyObject* pValue = NULL;
		static char* kwlist[] = { "Name", "DeviceID", "UnitID",	"Value", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CValue:%s, unable to find module for current interpreter.", __func__);
				goto Error;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CValue:%s, unable to obtain module state.", __func__);
				goto Error;
			}

			if (!pModState->pPlugin)
			{
				_log.Log(LOG_ERROR, "CValue:%s, illegal operation, Plugin has not started yet.", __func__);
				goto Error;
			}
			self->pPlugin = pModState->pPlugin;

			// During startup plugin sets this to signal load from database
			if (pModState->lObjectID)
			{
				self->ValueID = pModState->lObjectID;
				CValue_refresh(self);
				pModState->lObjectID = 0;
			}
			else
			{
				if (PyArg_ParseTupleAndKeywords(args, kwds, "siiO", kwlist, &szName, &lDeviceID, &lUnitID, &pValue))
				{
					PyObject* pSafeAssign;
					std::string	sName = szName ? szName : "";
					if (sName.length())
					{
						pSafeAssign = self->Name;
						self->Name = PyUnicode_FromString(sName.c_str());
						Py_XDECREF(pSafeAssign);
					}
					self->DeviceID = lDeviceID;
					self->UnitID = lUnitID;
					if (pValue)
					{
						pSafeAssign = self->Value;
						PyObject* pStringObj = PyObject_Str(pValue);
						if (pStringObj)
						{
							self->Value = pStringObj;
							Py_DECREF(pSafeAssign);
						}
						else
						{
							_log.Log(LOG_ERROR, "(%s) Unable to derive string for Value", self->pPlugin->m_Name.c_str());
						}
					}
				}
				else
				{
					CPlugin* pPlugin = NULL;
					if (pModState) pPlugin = pModState->pPlugin;
					_log.Log(LOG_ERROR, "Expected: myValue = domoticz.Value(Name=\"myValue\", DeviceID=1, UnitID=1, Value=\"1.0\")");
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

		Error:
		if (PyErr_Occurred())
		{
			self->pPlugin->LogPythonException("CValue_init");
		}
		return 0;
	}

	PyObject* CValue_refresh(CValue* self)
	{
		if (self->pPlugin)
		{
			if (self->ValueID >= 1)
			{
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT DeviceID, Name, UnitID, Value, Debug, Timestamp FROM Value WHERE ValueID = %d", self->ValueID);

				// Handle any data we get back
				if (result.empty())
				{
					ValueLog(self, LOG_ERROR, "Refresh of 'Value' failed to read any records for ID %d", self->DeviceID);
				}
				else
				{
					ValueLog(self, LOG_DEBUG_INT, "Refresh of 'Value' succeeded for ID %d", self->DeviceID);
					for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
					{
						std::vector<std::string> sd = *itt;
						PyObject* pSafeAssign;

						self->DeviceID = atoi(sd[0].c_str());

						pSafeAssign = self->Name;
						self->Name = PyUnicode_FromString(sd[1].c_str());
						Py_XDECREF(pSafeAssign);

						self->UnitID = atoi(sd[2].c_str());

						pSafeAssign = self->Value;
						self->Value = PyUnicode_FromString(sd[3].c_str());
						Py_XDECREF(pSafeAssign);

						self->Debug = atoi(sd[4].c_str()) ? true : false;

						pSafeAssign = self->Timestamp;
						self->Timestamp = PyUnicode_FromString(sd[5].c_str());
						Py_XDECREF(pSafeAssign);
					}
				}
			}
			else
			{
				ValueLog(self, LOG_ERROR, "Invalid Value ID '%d', must be already set.", (long)self->DeviceID);
			}
		}
		else
		{
			ValueLog(self, LOG_ERROR, "Value refresh failed, Value object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CValue_insert(CValue* self)
	{
		if (self->pPlugin)
		{
			if (self->ValueID <= 1)
			{
				std::string		sSQL = "INSERT INTO Value (Name, DeviceID, UnitID, Value, RetentionDays, RetentionInterval, Debug) "
										"VALUES (?,?,?,?,"
												"(SELECT RetentionDays from Unit WHERE Unit.UnitID = "+ std::to_string(self->UnitID) +"), "
												"(SELECT RetentionInterval from Unit WHERE Unit.UnitID = "+ std::to_string(self->UnitID) +"), "
												"?);";
				std::vector<std::string> vValues;
				std::string	sName = PyUnicode_AsUTF8(self->Name);
				vValues.push_back(sName);
				vValues.push_back(std::to_string(self->DeviceID));
				vValues.push_back(std::to_string(self->UnitID));

				if (PyUnicode_Check(self->Value))
				{
					vValues.push_back(std::string(PyUnicode_AsUTF8(self->Value)));
				}
				else
				{
					PyObject* pStringObj = PyObject_Str(self->Value);
					if (pStringObj)
					{
						vValues.push_back(PyUnicode_AsUTF8(pStringObj));
						Py_DECREF(pStringObj);
					}
					else
					{
						_log.Log(LOG_ERROR, "(%s) Unable to derive string for Value", self->pPlugin->m_Name.c_str());
						vValues.push_back(std::string(""));
					}
				}
				
				vValues.push_back(std::to_string(self->Debug));
				int		iRowCount = m_sql.execute_sql(sSQL, &vValues, true);

				// Handle any data we get back
				if (!iRowCount)
				{
					ValueLog(self, LOG_ERROR, "Insert into 'Value' failed to create any records for ID %d", self->ValueID);
				}
				else
				{
					// Read ValueID of new value and update the object
					std::vector<std::vector<std::string>> result = m_sql.safe_query("SELECT ValueID FROM Value WHERE DeviceID = %d AND Name = '%q'", self->DeviceID, sName.c_str());
					for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
					{
						std::vector<std::string> sd = *itt;
						self->ValueID = atoi(sd[0].c_str());
					}

					_log.Log(LOG_NORM, "Insert into 'Value' succeeded with ID %d, %d record(s) created.", self->ValueID, iRowCount);
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Invalid Value ID '%d', must not be already set.", self->pPlugin->m_Name.c_str(), (long)self->ValueID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Value creation failed, Value object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CValue_update(CValue* self)
	{
		if (self->pPlugin)
		{
			if (self->ValueID != -1)
			{
				std::string		sSQL = "UPDATE Value SET Value=?, Timestamp=CURRENT_TIMESTAMP WHERE ValueID=" + std::to_string(self->ValueID) + ";";
				std::vector<std::string> vValues;
				if (PyUnicode_Check(self->Value))
				{
					vValues.push_back(std::string(PyUnicode_AsUTF8(self->Value)));
				}
				else
				{
					PyObject* pStringObj = PyObject_Str(self->Value);
					if (pStringObj)
					{
						vValues.push_back(PyUnicode_AsUTF8(pStringObj));
						Py_DECREF(pStringObj);
					}
					else
					{
						ValueLog(self, LOG_ERROR, "(%s) Unable to derive string for Value", self->pPlugin->m_Name.c_str());
						vValues.push_back(std::string(""));
					}
				}

				int		iRowCount = m_sql.execute_sql(sSQL, &vValues, true);

				// Handle any data we get back
				if (!iRowCount)
				{
					ValueLog(self, LOG_ERROR, "Update to 'Value' failed to update any records for ID %d", self->ValueID);
				}
				else
				{
					ValueLog(self, LOG_NORM, "Update to 'Value' succeeded, %d records updated.", iRowCount);
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Invalid Value ID '%d', must already be set.", self->pPlugin->m_Name.c_str(), (long)self->ValueID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Value update failed, Value object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CValue_delete(CValue* self)
	{
		if (self->pPlugin)
		{
			std::string	sName = PyUnicode_AsUTF8(self->Name);
			if (self->ValueID != -1)
			{
				// Build SQL statement
				std::string		sSQL = "DELETE FROM VALUE WHERE ValueID = " + std::to_string(self->ValueID) + ";";
				m_sql.safe_query(sSQL.c_str());

				// Handle any data we get back
				std::vector<std::vector<std::string> >	result = m_sql.safe_query("SELECT changes();");
				if (result.empty())
				{
					ValueLog(self, LOG_ERROR, "Delete from 'Value' failed to delete any records for ID %d", self->ValueID);
				}
				else
				{
					_log.Log(LOG_NORM, "Delete from 'Value' succeeded, %s records removed.", result[0][0].c_str());
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Value deletion failed, '%d' does not represent a Value identifier.", self->pPlugin->m_Name.c_str(), self->ValueID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Value deletion failed, Value object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CValue_str(CValue* self)
	{
		PyObject* pRetVal = PyUnicode_FromFormat("ID: %d, Name: %U, DeviceID: %d, UnitID: %d, Value: '%U', Timestamp: %U", 
									self->ValueID, self->Name, self->DeviceID, self->UnitID, self->Value, self->Timestamp);
		return pRetVal;
	}
}