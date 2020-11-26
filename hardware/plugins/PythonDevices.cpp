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

	void DeviceLog(CDevice* self, const _eLogLevel level, const char* Message, ...)
	{
		va_list argList;
		char cbuffer[1024];
		va_start(argList, Message);
		vsnprintf(cbuffer, sizeof(cbuffer), Message, argList);
		va_end(argList);

		// Firstly do standard log behaviour
		_log.Log(level, "%s", cbuffer);

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
			PyObject* brModule = PyState_FindModule(&DomoticzModuleDef);
			if (!brModule)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(brModule));
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
				self->pPlugin->LogPythonException((PyObject*)self, __func__);
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

		Py_RETURN_NONE;
	}

	PyObject* CDevice_log(CDevice* self, PyObject* args, PyObject* kwds)
	{
		char* szMessage = NULL;
		static char* kwlist[] = { "Message", NULL };

		try
		{
			PyObject* brModule = PyState_FindModule(&DomoticzModuleDef);
			if (!brModule)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(brModule));
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
				self->pPlugin->LogPythonException((PyObject*)self, __func__);
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

		Py_RETURN_NONE;
	}

	PyObject* CDevice_error(CDevice* self, PyObject* args, PyObject* kwds)
	{
		char* szMessage = NULL;
		static char* kwlist[] = { "Message", NULL };

		try
		{
			PyObject* brModule = PyState_FindModule(&DomoticzModuleDef);
			if (!brModule)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(brModule));
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
				self->pPlugin->LogPythonException((PyObject*)self, __func__);
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

		Py_RETURN_NONE;
	}

	CDevice* CDevice::Copy()
	{
		CDevice* pRetVal = NULL;
		module_state* pModState;
		Py_ssize_t pos = 0;

		PyObject* brModule = PyState_FindModule(&DomoticzModuleDef);
		if (!brModule)
		{
			_log.Log(LOG_ERROR, "CDevice:%s, unable to find module for current interpreter.", __func__);
			goto Error;
		}

		pModState = ((struct module_state*)PyModule_GetState(brModule));
		if (!pModState)
		{
			_log.Log(LOG_ERROR, "CDevice:%s, unable to obtain module state.", __func__);
			goto Error;
		}

		pRetVal = (CDevice*)CDevice_new(pModState->pDeviceClass, NULL, NULL);
		if (!pRetVal)
		{
			_log.Log(LOG_ERROR, "CDevice:%s, failed to create Device object.", __func__);
			goto Error;
		}

		pRetVal->sanityCheck = false; // Reference counts will not be 1 during dealloc so suppress checking

		pRetVal->DeviceID = DeviceID;
		pRetVal->InterfaceID = InterfaceID;
		Py_INCREF(Name);
		Py_XDECREF(pRetVal->Name);
		pRetVal->Name = Name;
		Py_INCREF(InternalID);
		Py_XDECREF(pRetVal->InternalID);
		pRetVal->InternalID = InternalID;
		Py_INCREF(Address);
		Py_XDECREF(pRetVal->Address);
		pRetVal->Address = Address;
		pRetVal->Debug = Debug;
		pRetVal->Enabled = Enabled;
		pRetVal->Active = Active;
		Py_INCREF(Timestamp);
		Py_XDECREF(pRetVal->Timestamp);
		pRetVal->Timestamp = Timestamp;
		pRetVal->Parent = Parent;
		pRetVal->pPlugin = pPlugin;

		PyObject* key;
		CValue* pValue;
		while (PyDict_Next(Values, &pos, &key, (PyObject * *)& pValue))
		{
			// And insert it into the Device's Values dictionary
			if (PyDict_SetItem(pRetVal->Values, pValue->InternalID, (PyObject*)pValue) == -1)
			{
				DeviceLog(this, LOG_ERROR, "Failed to add value number '%d' to Values dictionary for Device %d.", pValue->ValueID, pRetVal->DeviceID);
				goto Error;
			}
		}

	Error:
		if (PyErr_Occurred())
		{
			pPlugin->LogPythonException((PyObject*)this, __func__);
		}

		return pRetVal;
	}

	CValue* CDevice::AddValueToDict(long lValueID)
	{
		CValue* pValue = NULL;

		try
		{
			PyObject* brModule = PyState_FindModule(&DomoticzModuleDef);
			if (!brModule)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(brModule));
			if (!pModState)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to obtain module state.", __func__);
				return 0;
			}

			PyObjPtr nrArgList = Py_BuildValue("(ssiiO)", "", "", -1, -1, PyUnicode_FromString(""));
			if (!nrArgList)
			{
				DeviceLog(this, LOG_ERROR, "Building Value argument list failed for Value %d.", lValueID);
				goto Error;
			}

			// Pass values in, needs to be a tuple and signal CValue_init to load from the database
			pModState->lObjectID = lValueID;
			pValue = (CValue*)PyObject_CallObject((PyObject*)pModState->pValueClass, nrArgList);
			if (!pValue)
			{
				DeviceLog(this, LOG_ERROR, "Value object creation failed for Value %d.", lValueID);
				goto Error;
			}
			pValue->Parent = this;			// Borrow a reference to the owning Interface

			// And insert it into the Device's Values dictionary
			if (PyDict_SetItem(Values, pValue->InternalID, (PyObject*)pValue) == -1)
			{
				DeviceLog(this, LOG_ERROR, "Failed to add value number '%d' to Values dictionary for Device %d.", lValueID, DeviceID);
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
			pPlugin->LogPythonException((PyObject*)this, __func__);
		}

		// If not NULL the caller will need to decref
		return pValue;
	}

	CValue* CDevice::FindValue(long lValueID)
	{
		CValue* pValue = NULL;

		try
		{
			// Dictionary is keyed by InternalID to make it more useable so do iterative search
			PyObject*	key;
			Py_ssize_t	pos = 0;
			while (PyDict_Next(Values, &pos, &key, (PyObject**)&pValue))
			{
				if (pValue->ValueID == lValueID)
				{
					Py_INCREF(pValue);
					break;
				}
				pValue = NULL;
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
		PyObject* brModule = PyState_FindModule(&DomoticzModuleDef);
		if (!brModule)
		{
			_log.Log(LOG_ERROR, "CDevice:%s, unable to find module for current interpreter.", __func__);
			return;
		}

		module_state* pModState = ((struct module_state*)PyModule_GetState(brModule));
		if (!pModState)
		{
			_log.Log(LOG_ERROR, "CDevice:%s, unable to obtain module state.", __func__);
			return;
		}

		PyObject* key;
		CValue* pValue;
		Py_ssize_t pos = 0;
		// Sanity check to make sure the reference counting is all good.
		while (self->sanityCheck && PyDict_Next(self->Values, &pos, &key, (PyObject**)&pValue))
		{
			// Dictionary should be full of Values but Python script can make this assumption false, log warning if this has happened
			int	isValue = PyObject_IsInstance((PyObject*)pValue, (PyObject*)pModState->pValueClass);
			if (isValue == -1)
			{
				self->pPlugin->LogPythonException((PyObject*)self, "Error determining type of Python object during dealloc");
			}
			else if (isValue == 0)
			{
				DeviceLog(self, LOG_NORM, "%s: Value dictionary contained non-Value entry.", __func__);
			}
			else
			{
				if (pValue->ob_base.ob_refcnt > 1)
				{
					std::string	sName = PyUnicode_AsUTF8(pValue->Name);
						DeviceLog(self, LOG_ERROR, "%s: Value '%s' Reference Count not one: %d.", __func__, sName.c_str(), pValue->ob_base.ob_refcnt);
				}
				else if (pValue->ob_base.ob_refcnt < 1)
				{
					DeviceLog(self, LOG_ERROR, "%s: Value with ID %d Reference Count not one: %d.", __func__, pValue->ValueID, pValue->ob_base.ob_refcnt);
				}
			}
		}

		Py_XDECREF(self->Name);
		Py_XDECREF(self->InternalID);
		Py_XDECREF(self->Timestamp);
		if (PyDict_Size(self->Values))
		{
			PyDict_Clear(self->Values);
		}
		Py_XDECREF(self->Values);
		// Py_XDECREF(self->Parent);	// Reference to owning connection is borrowed so don't release it.
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
					Py_XDECREF(self);
					return NULL;
				}
				self->InternalID = PyUnicode_FromString("");
				if (self->InternalID == NULL) {
					Py_XDECREF(self);
					return NULL;
				}
				self->Address = PyUnicode_FromString("");
				if (self->Address == NULL) {
					Py_XDECREF(self);
					return NULL;
				}
				self->Debug = false;
				self->Enabled = false;
				self->Active = false;
				self->Timestamp = PyUnicode_FromString("");
				if (self->Timestamp == NULL) {
					Py_XDECREF(self);
					return NULL;
				}
				self->Values = PyDict_New();
				self->pPlugin = NULL;

				self->sanityCheck = true;	// Check reference counts during deletee
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
		char* szInternalID = NULL;
		char* szAddress = NULL;
		int   bEnabled = 1;
		static char* kwlist[] = { "Name", "InternalID", "Address", "Enabled", NULL };

		try
		{
			PyObject* brModule = PyState_FindModule(&DomoticzModuleDef);
			if (!brModule)
			{
				_log.Log(LOG_ERROR, "CDevice:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(brModule));
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
						CValue* pValue = self->AddValueToDict(atoi(sd[0].c_str()));
						Py_XDECREF(pValue);
					}
				}
			}
			else
			{
				if (PyArg_ParseTupleAndKeywords(args, kwds, "sssp", kwlist, &szName, &szInternalID, &szAddress, &bEnabled))
				{
					self->InterfaceID = pModState->pPlugin->m_InterfaceID;
					std::string	sName = szName ? szName : "";
					if (sName.length())
					{
						Py_XDECREF(self->Name);
						self->Name = PyUnicode_FromString(sName.c_str());
					}
					std::string	sInternalID = szInternalID ? szInternalID : "";
					if (sInternalID.length())
					{
						Py_XDECREF(self->InternalID);
						self->InternalID = PyUnicode_FromString(sInternalID.c_str());
					}
					std::string	sAddress = szAddress ? szAddress : "";
					if (sAddress.length())
					{
						Py_XDECREF(self->Address);
						self->Address = PyUnicode_FromString(sAddress.c_str());
					}
					self->Enabled = bEnabled;
				}
				else
				{
					CPlugin* pPlugin = NULL;
					if (pModState) pPlugin = pModState->pPlugin;
					_log.Log(LOG_ERROR, "Expected: myDevice = domoticz.Device(Name=\"myDevice\", InternalID=\"xxxxxxx\", Address=\"192.168.0.xxx\", Enabled=True)");
					self->pPlugin->LogPythonException((PyObject*)self, __func__);
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
			self->pPlugin->LogPythonException((PyObject*)self, __func__);
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
				result = m_sql.safe_query("SELECT InterfaceID, Name, InternalID, Address, Debug, Enabled, Active, Timestamp FROM Device WHERE DeviceID = %d", self->DeviceID);

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

						pSafeAssign = self->InternalID;
						self->InternalID = PyUnicode_FromString(sd[2].c_str());

						pSafeAssign = self->Address;
						self->Address = PyUnicode_FromString(sd[3].c_str());

						self->Debug = atoi(sd[4].c_str()) ? true : false;
						self->Enabled = atoi(sd[5].c_str()) ? true : false;
						self->Active = atoi(sd[6].c_str()) ? true : false;

						pSafeAssign = self->Timestamp;
						self->Timestamp = PyUnicode_FromString(sd[7].c_str());
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

		Py_RETURN_NONE;
	}

	PyObject* CDevice_insert(CDevice* self)
	{
		if (self->pPlugin)
		{
			if (self->DeviceID <= 1)
			{
				std::string		sSQL = "INSERT INTO Device (InterfaceID, Name, InternalID, Address, Debug, Enabled, Active) VALUES (?,?,?,?,?,?,?);";
				std::vector<std::string> vValues;
				vValues.push_back(std::to_string(self->InterfaceID));
				std::string	sName = PyUnicode_AsUTF8(self->Name);
				vValues.push_back(sName);
				std::string	sInternalID = PyUnicode_AsUTF8(self->InternalID);
				vValues.push_back(sInternalID);
				std::string	sAddress = PyUnicode_AsUTF8(self->Address);
				vValues.push_back(sAddress);
				vValues.push_back(std::to_string(self->Debug));
				vValues.push_back(std::to_string(self->Enabled));
				vValues.push_back(std::to_string(self->Active));
				int		iRowCount = m_sql.execute_sql(sSQL, &vValues, true);

				// Handle any data we get back
				if (!iRowCount)
				{
					DeviceLog(self, LOG_ERROR, "Insert into 'Device' failed to create any records for ID %ld", self->DeviceID);
				}
				else
				{
					// Read DeviceID of new device and update the object
					std::vector<std::vector<std::string>> result = m_sql.safe_query("SELECT MAX(DeviceID) FROM Device WHERE InterfaceID = %d AND Name = '%q'", self->InterfaceID, sName.c_str());
					for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
					{
						std::vector<std::string> sd = *itt;
						self->DeviceID = atoi(sd[0].c_str());
						self->Parent = self->pPlugin->m_Interface;
					}

					if (self->pPlugin->m_bDebug && PDM_PUB_SUB)
						_log.Log(LOG_NORM, "Insert into 'Device' succeeded with ID %ld, %d record(s) created.", self->DeviceID, iRowCount);
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Invalid Device ID '%ld', must not be already set.", self->pPlugin->m_Name.c_str(), (long)self->DeviceID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Device creation failed, Device object is not associated with a plugin.");
		}

		Py_RETURN_NONE;
	}

	PyObject* CDevice_update(CDevice* self)
	{
		if (self->pPlugin)
		{
			if (self->DeviceID != -1)
			{
				std::string		sSQL = "UPDATE Device SET Address=?, Active=?, Timestamp=CURRENT_TIMESTAMP WHERE DeviceID=" + std::to_string(self->DeviceID) + ";";
				std::vector<std::string> vValues;
				std::string	sAddress = PyUnicode_AsUTF8(self->Address);
				vValues.push_back(sAddress);
				vValues.push_back(std::to_string(self->Active));
				int		iRowCount = m_sql.execute_sql(sSQL, &vValues, true);

				// Handle any data we get back
				if (!iRowCount)
				{
					DeviceLog(self, LOG_ERROR, "Update to 'Device' failed to update any records for ID %ld", self->DeviceID);
				}
				else
				{
					std::string	sName = PyUnicode_AsUTF8(self->Name);
					if (self->pPlugin->m_bDebug && PDM_PUB_SUB)
						_log.Log(LOG_NORM, "Update to Device '%s' succeeded, %d records updated.", sName.c_str(), iRowCount);
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Invalid Device ID '%ld', must already be set.", self->pPlugin->m_Name.c_str(), (long)self->DeviceID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Device creation failed, Device object is not associated with an interface.");
		}

		Py_RETURN_NONE;
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
					DeviceLog(self, LOG_ERROR, "Delete from 'Device' failed to delete any records for ID %ld", self->DeviceID);
				}
				else
				{
					if (self->pPlugin->m_bDebug && PDM_PUB_SUB)
						_log.Log(LOG_NORM, "Delete from 'Device' succeeded, %s records removed.", result[0][0].c_str());
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Device deletion failed, '%ld' does not represent a Device identifier.", self->pPlugin->m_Name.c_str(), self->DeviceID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Device deletion failed, Device object is not associated with a plugin.");
		}

		Py_RETURN_NONE;
	}

	PyObject* CDevice_str(CDevice* self)
	{
		PyObject* nrRetVal = PyUnicode_FromFormat("DeviceID: %d, Name: %U, InterfaceID: %d, InternalID: %U, Address: %U, Debug: '%s', Enabled: '%s', Active: '%s', Timestamp: %U",
			self->DeviceID, self->Name, self->InterfaceID, self->InternalID, self->Address, (self->Debug ? "True" : "False"), (self->Enabled ? "True" : "False"), (self->Active ? "True" : "False"), self->Timestamp);
		return nrRetVal;
	}
}