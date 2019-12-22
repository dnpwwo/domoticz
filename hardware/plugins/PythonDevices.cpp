#include "stdafx.h"

#include "PythonDevices.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016 - 2020
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
	extern void LogPythonException(CPlugin* pPlugin, const std::string& sHandler);

	struct module_state {
		CPlugin* pPlugin;
		PyObject* error;
	};

	void CDevice_dealloc(CDevice* self)
	{
		Py_XDECREF(self->Name);
		Py_XDECREF(self->Description);
		Py_XDECREF(self->sValue);
		PyDict_Clear(self->Options);
		Py_XDECREF(self->Options);
		Py_XDECREF(self->Color);
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
				self->PluginKey = PyUnicode_FromString("");
				if (self->PluginKey == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->InterfaceID = -1;
				self->DeviceID = PyUnicode_FromString("");
				if (self->DeviceID == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Unit = -1;
				self->Type = 0;
				self->SubType = 0;
				self->SwitchType = 0;
				self->ID = -1;
				self->LastLevel = 0;
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
				self->nValue = 0;
				self->sValue = PyUnicode_FromString("");
				if (self->sValue == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Options = PyDict_New();
				if (self->Options == NULL) {
					Py_DECREF(self);
					return NULL;
				}
				self->Image = 0;
				self->Used = 0;
				self->SignalLevel = 100;
				self->BatteryLevel = 255;
				self->TimedOut = 0;
				self->Color = PyUnicode_FromString("");
				if (self->Color == NULL) {
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

	static void maptypename(const std::string& sTypeName, int& Type, int& SubType, int& SwitchType, std::string& sValue, PyObject* OptionsIn, PyObject* OptionsOut)
	{
		/*
		Type = pTypeGeneral;

		if (sTypeName == "Pressure")					SubType = sTypePressure;
		else if (sTypeName == "Percentage")				SubType = sTypePercentage;
		else if (sTypeName == "Gas")
		{
			Type = pTypeP1Gas;
			SubType = sTypeP1Gas;
		}
		else if (sTypeName == "Voltage")				SubType = sTypeVoltage;
		else if (sTypeName == "Text")					SubType = sTypeTextStatus;
		else if (sTypeName == "Switch")
		{
			Type = pTypeGeneralSwitch;
			SubType = sSwitchGeneralSwitch;
		}
		else if (sTypeName == "Alert")
		{
			sValue = "No Alert!";
			SubType = sTypeAlert;
		}
		else if (sTypeName == "Current/Ampere")
		{
			sValue = "0.0;0.0;0.0";
			Type = pTypeCURRENT;
			SubType = sTypeELEC1;
		}
		else if (sTypeName == "Sound Level")			SubType = sTypeSoundLevel;
		else if (sTypeName == "Barometer")
		{
			sValue = "1021.34;0";
			SubType = sTypeBaro;
		}
		else if (sTypeName == "Visibility")				SubType = sTypeVisibility;
		else if (sTypeName == "Distance")				SubType = sTypeDistance;
		else if (sTypeName == "Counter Incremental")	SubType = sTypeCounterIncremental;
		else if (sTypeName == "Soil Moisture")			SubType = sTypeSoilMoisture;
		else if (sTypeName == "Leaf Wetness")			SubType = sTypeLeafWetness;
		else if (sTypeName == "kWh")
		{
			sValue = "0; 0.0";
			SubType = sTypeKwh;
		}
		else if (sTypeName == "Current (Single)")		SubType = sTypeCurrent;
		else if (sTypeName == "Solar Radiation")		SubType = sTypeSolarRadiation;
		else if (sTypeName == "Temperature")
		{
			Type = pTypeTEMP;
			SubType = sTypeTEMP5;
		}
		else if (sTypeName == "Humidity")
		{
			Type = pTypeHUM;
			SubType = sTypeHUM1;
		}
		else if (sTypeName == "Temp+Hum")
		{
			sValue = "0.0;50;1";
			Type = pTypeTEMP_HUM;
			SubType = sTypeTH1;
		}
		else if (sTypeName == "Temp+Hum+Baro")
		{
			sValue = "0.0;50;1;1010;1";
			Type = pTypeTEMP_HUM_BARO;
			SubType = sTypeTHB1;
		}
		else if (sTypeName == "Wind")
		{
			sValue = "0;N;0;0;0;0";
			Type = pTypeWIND;
			SubType = sTypeWIND1;
		}
		else if (sTypeName == "Rain")
		{
			sValue = "0;0";
			Type = pTypeRAIN;
			SubType = sTypeRAIN3;
		}
		else if (sTypeName == "UV")
		{
			sValue = "0;0";
			Type = pTypeUV;
			SubType = sTypeUV1;
		}
		else if (sTypeName == "Air Quality")
		{
			Type = pTypeAirQuality;
			SubType = sTypeVoltcraft;
		}
		else if (sTypeName == "Usage")
		{
			Type = pTypeUsage;
			SubType = sTypeElectric;
		}
		else if (sTypeName == "Illumination")
		{
			Type = pTypeLux;
			SubType = sTypeLux;
		}
		else if (sTypeName == "Waterflow")				SubType = sTypeWaterflow;
		else if (sTypeName == "Wind+Temp+Chill")
		{
			sValue = "0;N;0;0;0;0";
			Type = pTypeWIND;
			SubType = sTypeWIND4;
		}
		else if (sTypeName == "Selector Switch")
		{
			if (!OptionsIn || !PyDict_Check(OptionsIn)) {
				PyDict_Clear(OptionsOut);
				PyDict_SetItemString(OptionsOut, "LevelActions", PyUnicode_FromString("|||"));
				PyDict_SetItemString(OptionsOut, "LevelNames", PyUnicode_FromString("Off|Level1|Level2|Level3"));
				PyDict_SetItemString(OptionsOut, "LevelOffHidden", PyUnicode_FromString("false"));
				PyDict_SetItemString(OptionsOut, "SelectorStyle", PyUnicode_FromString("0"));
			}
			Type = pTypeGeneralSwitch;
			SubType = sSwitchTypeSelector;
			SwitchType = STYPE_Selector;
		}
		else if (sTypeName == "Push On")
		{
			Type = pTypeGeneralSwitch;
			SubType = sSwitchGeneralSwitch;
			SwitchType = STYPE_PushOn;
		}
		else if (sTypeName == "Push Off")
		{
			Type = pTypeGeneralSwitch;
			SubType = sSwitchGeneralSwitch;
			SwitchType = STYPE_PushOff;
		}
		else if (sTypeName == "Contact")
		{
			Type = pTypeGeneralSwitch;
			SubType = sSwitchGeneralSwitch;
			SwitchType = STYPE_Contact;
		}
		else if (sTypeName == "Dimmer")
		{
			Type = pTypeGeneralSwitch;
			SubType = sSwitchGeneralSwitch;
			SwitchType = STYPE_Dimmer;
		}
		else if (sTypeName == "Motion")
		{
			Type = pTypeGeneralSwitch;
			SubType = sSwitchGeneralSwitch;
			SwitchType = STYPE_Motion;
		}
		else if (sTypeName == "Custom")
		{
			SubType = sTypeCustom;
			if (!OptionsIn || !PyDict_Check(OptionsIn)) {
				PyDict_Clear(OptionsOut);
				PyDict_SetItemString(OptionsOut, "Custom", PyUnicode_FromString("1"));
			}
		}
		else if (sTypeName == "Security Panel")
		{
			Type = pTypeSecurity1;
			SubType = sTypeDomoticzSecurity;
		}
		*/
	}

	int CDevice_init(CDevice* self, PyObject* args, PyObject* kwds)
	{
		char* Name = NULL;
		char* DeviceID = NULL;
		int			Unit = -1;
		char* TypeName = NULL;
		int			Type = -1;
		int			SubType = -1;
		int			SwitchType = -1;
		int			Image = -1;
		PyObject* Options = NULL;
		int			Used = -1;
		char* Description = NULL;
		static char* kwlist[] = { "Name", "Unit", "TypeName", "Type", "Subtype", "Switchtype", "Image", "Options", "Used", "DeviceID", "Description", NULL };

		try
		{
			PyObject* pModule = PyState_FindModule(&DomoticzModuleDef);
			if (!pModule)
			{
				_log.Log(LOG_ERROR, "CPlugin:%s, unable to find module for current interpreter.", __func__);
				return 0;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
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

			if (PyArg_ParseTupleAndKeywords(args, kwds, "si|siiiiOiss", kwlist, &Name, &Unit, &TypeName, &Type, &SubType, &SwitchType, &Image, &Options, &Used, &DeviceID, &Description))
			{
				self->pPlugin = pModState->pPlugin;
				self->PluginKey = PyUnicode_FromString(pModState->pPlugin->m_PluginKey.c_str());
				self->InterfaceID = pModState->pPlugin->m_InterfaceID;
				if (Name) {
					Py_DECREF(self->Name);
					self->Name = PyUnicode_FromString(Name);
				}
				if (Description) {
					Py_DECREF(self->Description);
					self->Description = PyUnicode_FromString(Description);
				}
				if ((Unit > 0) && (Unit < 256))
				{
					self->Unit = Unit;
				}
				else
				{
					_log.Log(LOG_ERROR, "CPlugin:%s, illegal Unit number (%d), valid values range from 1 to 255.", __func__, Unit);
					return 0;
				}
				Py_DECREF(self->DeviceID);
				if (DeviceID) {
					self->DeviceID = PyUnicode_FromString(DeviceID);
				}
				else
				{
					char szID[40];		// Generate a Device ID if one was not supplied
					sprintf(szID, "%04X%04X", self->InterfaceID, self->Unit);
					self->DeviceID = PyUnicode_FromString(szID);
				}
				if (TypeName) {
					std::string sValue = "";
					maptypename(std::string(TypeName), self->Type, self->SubType, self->SwitchType, sValue, Options, self->Options);
					Py_DECREF(self->sValue);
					self->sValue = PyUnicode_FromString(sValue.c_str());
				}
				if ((Type != -1) && Type) self->Type = Type;
				if ((SubType != -1) && SubType) self->SubType = SubType;
				if (SwitchType != -1) self->SwitchType = SwitchType;
				if (Image != -1) self->Image = Image;
				if (Used == 1) self->Used = Used;
				if (Options && PyDict_Check(Options) && PyDict_Size(Options) > 0) {
					PyObject* pKey, * pValue;
					Py_ssize_t pos = 0;
					PyDict_Clear(self->Options);
					while (PyDict_Next(Options, &pos, &pKey, &pValue))
					{
						if (PyUnicode_Check(pValue))
						{
							PyObject* pKeyDict = PyUnicode_FromKindAndData(PyUnicode_KIND(pKey), PyUnicode_DATA(pKey), PyUnicode_GET_LENGTH(pKey));
							PyObject* pValueDict = PyUnicode_FromKindAndData(PyUnicode_KIND(pValue), PyUnicode_DATA(pValue), PyUnicode_GET_LENGTH(pValue));
							if (PyDict_SetItem(self->Options, pKeyDict, pValueDict) == -1)
							{
								_log.Log(LOG_ERROR, "(%s) Failed to initialize Options dictionary for Hardware/Unit combination (%d:%d).", self->pPlugin->m_Name.c_str(), self->InterfaceID, self->Unit);
								Py_XDECREF(pKeyDict);
								Py_XDECREF(pValueDict);
								break;
							}
							Py_XDECREF(pKeyDict);
							Py_XDECREF(pValueDict);
						}
						else
						{
							_log.Log(LOG_ERROR, "(%s) Failed to initialize Options dictionary for Hardware/Unit combination (%d:%d): Only \"string\" type dictionary entries supported, but entry has type \"%s\"", self->pPlugin->m_Name.c_str(), self->InterfaceID, self->Unit, pValue->ob_type->tp_name);
						}
					}
				}
			}
			else
			{
				CPlugin* pPlugin = NULL;
				if (pModState) pPlugin = pModState->pPlugin;
				_log.Log(LOG_ERROR, "Expected: myVar = Domoticz.Device(Name=\"myDevice\", Unit=0, TypeName=\"\", Type=0, Subtype=0, Switchtype=0, Image=0, Options={}, Used=1)");
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

		return 0;
	}

	PyObject* CDevice_refresh(CDevice* self)
	{
		if ((self->pPlugin) && (self->InterfaceID != -1) && (self->Unit != -1))
		{
			// load associated devices to make them available to python
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT Unit, ID, Name, nValue, sValue, DeviceID, Type, SubType, SwitchType, LastLevel, CustomImage, SignalLevel, BatteryLevel, LastUpdate, Options, Description, Color FROM DeviceStatus WHERE (HardwareID==%d) AND (Unit==%d) ORDER BY Unit ASC", self->InterfaceID, self->Unit);
			if (!result.empty())
			{
				for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
				{
					std::vector<std::string> sd = *itt;
					self->Unit = atoi(sd[0].c_str());
					self->ID = atoi(sd[1].c_str());
					Py_XDECREF(self->Name);
					self->Name = PyUnicode_FromString(sd[2].c_str());
					self->nValue = atoi(sd[3].c_str());
					Py_XDECREF(self->sValue);
					self->sValue = PyUnicode_FromString(sd[4].c_str());
					Py_XDECREF(self->DeviceID);
					self->DeviceID = PyUnicode_FromString(sd[5].c_str());
					self->Type = atoi(sd[6].c_str());
					self->SubType = atoi(sd[7].c_str());
					self->SwitchType = atoi(sd[8].c_str());
					self->LastLevel = atoi(sd[9].c_str());
					self->Image = atoi(sd[10].c_str());
					self->SignalLevel = atoi(sd[11].c_str());
					self->BatteryLevel = atoi(sd[12].c_str());
					Py_XDECREF(self->LastUpdate);
					self->LastUpdate = PyUnicode_FromString(sd[13].c_str());
					PyDict_Clear(self->Options);
					/*
					if (!sd[14].empty())
					{
						if (self->SubType == sTypeCustom)
						{
							PyDict_SetItemString(self->Options, "Custom", PyUnicode_FromString(sd[14].c_str()));
						}
						else
						{
							std::map<std::string, std::string> mpOptions = m_sql.BuildDeviceOptions(sd[14], true);
							for (std::map<std::string, std::string>::const_iterator ittOpt = mpOptions.begin(); ittOpt != mpOptions.end(); ++ittOpt)
							{
								PyObject *pKeyDict = PyUnicode_FromString(ittOpt->first.c_str());
								PyObject *pValueDict =  PyUnicode_FromString(ittOpt->second.c_str());
								if (PyDict_SetItem(self->Options, pKeyDict, pValueDict) == -1)
								{
									_log.Log(LOG_ERROR, "(%s) Failed to refresh Options dictionary for Hardware/Unit combination (%d:%d).", self->pPlugin->m_Name.c_str(), self->InterfaceID, self->Unit);
									Py_DECREF(pKeyDict);
									Py_DECREF(pValueDict);
									break;
								}
								Py_DECREF(pKeyDict);
								Py_DECREF(pValueDict);
							}
						}
					}
					*/
					Py_XDECREF(self->Description);
					self->Description = PyUnicode_FromString(sd[15].c_str());
					Py_XDECREF(self->Color);
					self->Color = PyUnicode_FromString(""); //Parse the color to detect incorrectly formatted color data
				}
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Device refresh failed, Device object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CDevice_insert(CDevice* self)
	{
		if (self->pPlugin)
		{
			std::string	sName = PyUnicode_AsUTF8(self->Name);
			std::string	sDeviceID = PyUnicode_AsUTF8(self->DeviceID);
			if (self->ID == -1)
			{
				if (self->pPlugin->m_bDebug & PDM_DEVICE)
				{
					_log.Log(LOG_NORM, "(%s) Creating device '%s'.", self->pPlugin->m_Name.c_str(), sName.c_str());
				}

				if (!m_sql.m_bAcceptNewHardware)
				{
					_log.Log(LOG_ERROR, "(%s) Device creation failed, Domoticz settings prevent accepting new devices.", self->pPlugin->m_Name.c_str());
				}
				else
				{
					std::vector<std::vector<std::string> > result;
					result = m_sql.safe_query("SELECT Name FROM DeviceStatus WHERE (HardwareID==%d) AND (Unit==%d)", self->InterfaceID, self->Unit);
					if (result.empty())
					{
						std::string	sValue = PyUnicode_AsUTF8(self->sValue);
						std::string	sColor = "";
						std::string	sLongName = self->pPlugin->m_Name + " - " + sName;
						std::string	sDescription = PyUnicode_AsUTF8(self->Description);
						//if ((self->SubType == sTypeCustom) && (PyDict_Size(self->Options) > 0))
						{
							PyObject* pValueDict = PyDict_GetItemString(self->Options, "Custom");
							std::string sOptionValue;
							if (pValueDict == NULL)
								sOptionValue = "";
							else
								sOptionValue = PyUnicode_AsUTF8(pValueDict);

							m_sql.safe_query(
								"INSERT INTO DeviceStatus (HardwareID, DeviceID, Unit, Type, SubType, SwitchType, Used, SignalLevel, BatteryLevel, Name, nValue, sValue, CustomImage, Description, Color, Options) "
								"VALUES (%d, '%q', %d, %d, %d, %d, %d, 12, 255, '%q', 0, '%q', %d, '%q', '%q', '%q')",
								self->InterfaceID, sDeviceID.c_str(), self->Unit, self->Type, self->SubType, self->SwitchType, self->Used, sLongName.c_str(), sValue.c_str(), self->Image, sDescription.c_str(), sColor.c_str(), sOptionValue.c_str());
						}
						//else
						{
							m_sql.safe_query(
								"INSERT INTO DeviceStatus (HardwareID, DeviceID, Unit, Type, SubType, SwitchType, Used, SignalLevel, BatteryLevel, Name, nValue, sValue, CustomImage, Description, Color) "
								"VALUES (%d, '%q', %d, %d, %d, %d, %d, 12, 255, '%q', 0, '%q', %d, '%q', '%q')",
								self->InterfaceID, sDeviceID.c_str(), self->Unit, self->Type, self->SubType, self->SwitchType, self->Used, sLongName.c_str(), sValue.c_str(), self->Image, sDescription.c_str(), sColor.c_str());
						}

						result = m_sql.safe_query("SELECT ID FROM DeviceStatus WHERE (HardwareID==%d) AND (Unit==%d)", self->InterfaceID, self->Unit);
						if (result.size())
						{
							self->ID = atoi(result[0][0].c_str());

							PyObject* pKey = PyLong_FromLong(self->Unit);
							if (PyDict_SetItem((PyObject*)self->pPlugin->m_DeviceDict, pKey, (PyObject*)self) == -1)
							{
								_log.Log(LOG_ERROR, "(%s) failed to add unit '%d' to device dictionary.", self->pPlugin->m_Name.c_str(), self->Unit);
								Py_INCREF(Py_None);
								return Py_None;
							}

							// Device successfully created, now set the options when supplied
							//if ((self->SubType != sTypeCustom) && (PyDict_Size(self->Options) > 0))
							{
								PyObject* pKeyDict, * pValueDict;
								Py_ssize_t pos = 0;
								std::map<std::string, std::string> mpOptions;
								while (PyDict_Next(self->Options, &pos, &pKeyDict, &pValueDict)) {
									std::string sOptionName = PyUnicode_AsUTF8(pKeyDict);
									PyObject* pStr = PyObject_Str(pValueDict);
									std::string sOptionValue = PyUnicode_AsUTF8(pStr);
									Py_XDECREF(pStr);
									mpOptions.insert(std::pair<std::string, std::string>(sOptionName, sOptionValue));
								}
								//m_sql.SetDeviceOptions(self->ID, mpOptions);
							}

							// Refresh device data to ensure it is usable straight away
							PyObject* pRetVal = CDevice_refresh(self);
							Py_DECREF(pRetVal);
						}
						else
						{
							_log.Log(LOG_ERROR, "(%s) Device creation failed, Hardware/Unit combination (%d:%d) not found in Domoticz.", self->pPlugin->m_Name.c_str(), self->InterfaceID, self->Unit);
						}
					}
					else
					{
						_log.Log(LOG_ERROR, "(%s) Device creation failed, Hardware/Unit combination (%d:%d) already exists in Domoticz.", self->pPlugin->m_Name.c_str(), self->InterfaceID, self->Unit);
					}
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Device creation failed, '%s' already exists in Domoticz with Device ID '%d'.", self->pPlugin->m_Name.c_str(), sName.c_str(), self->ID);
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Device creation failed, Device object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CDevice_update(CDevice* self, PyObject* args, PyObject* kwds)
	{
		if (self->pPlugin)
		{
			int			nValue = self->nValue;
			char* sValue = NULL;
			int			iSignalLevel = self->SignalLevel;
			int			iBatteryLevel = self->BatteryLevel;
			int			iImage = self->Image;
			int			iTimedOut = self->TimedOut;
			PyObject* pOptionsDict = NULL;

			char* Name = NULL;
			char* TypeName = NULL;
			int			iType = self->Type;
			int			iSubType = self->SubType;
			int			iSwitchType = self->SwitchType;
			int			iUsed = self->Used;
			char* Description = NULL;
			char* Color = NULL;
			int			SuppressTriggers = false;

			std::string	sName = PyUnicode_AsUTF8(self->Name);
			std::string	sDeviceID = PyUnicode_AsUTF8(self->DeviceID);
			std::string	sDescription = PyUnicode_AsUTF8(self->Description);
			static char* kwlist[] = { "nValue", "sValue", "Image", "SignalLevel", "BatteryLevel", "Options", "TimedOut", "Name", "TypeName", "Type", "Subtype", "Switchtype", "Used", "Description", "Color", "SuppressTriggers", NULL };

			// Try to extract parameters needed to update device settings
			if (!PyArg_ParseTupleAndKeywords(args, kwds, "is|iiiOissiiiissp", kwlist, &nValue, &sValue, &iImage, &iSignalLevel, &iBatteryLevel, &pOptionsDict, &iTimedOut, &Name, &TypeName, &iType, &iSubType, &iSwitchType, &iUsed, &Description, &Color, &SuppressTriggers))
			{
				_log.Log(LOG_ERROR, "(%s) %s: Failed to parse parameters: 'nValue', 'sValue', 'Image', 'SignalLevel', 'BatteryLevel', 'Options', 'TimedOut', 'Name', 'TypeName', 'Type', 'Subtype', 'Switchtype', 'Used', 'Description', 'Color' or 'SuppressTriggers' expected.", __func__, sName.c_str());
				LogPythonException(self->pPlugin, __func__);
				Py_INCREF(Py_None);
				return Py_None;
			}

			std::string sID = std::to_string(self->ID);

			// Name change
			if (Name)
			{
				sName = Name;
				//m_sql.UpdateDeviceValue("Name", sName, sID);
			}

			// Description change
			if (Description)
			{
				std::string sDescription = Description;
				//m_sql.UpdateDeviceValue("Description", sDescription, sID);
			}

			// TypeName change - actually derives new Type, SubType and SwitchType values
			if (TypeName) {
				std::string stdsValue = "";
				maptypename(std::string(TypeName), iType, iSubType, iSwitchType, stdsValue, pOptionsDict, pOptionsDict);

				// Reset nValue and sValue when changing device types
				//m_sql.UpdateDeviceValue("nValue", 0, sID);
				//m_sql.UpdateDeviceValue("sValue", stdsValue, sID);
			}

			// Type change
			if (iType != self->Type)
			{
				//m_sql.UpdateDeviceValue("Type", iType, sID);
			}

			// SubType change
			if (iSubType != self->SubType)
			{
				//m_sql.UpdateDeviceValue("SubType", iSubType, sID);
			}

			// SwitchType change
			if (iSwitchType != self->SwitchType)
			{
				//m_sql.UpdateDeviceValue("SwitchType", iSwitchType, sID);
			}

			// Image change
			if (iImage != self->Image)
			{
				//m_sql.UpdateDeviceValue("CustomImage", iImage, sID);
			}

			// Used change
			if (iUsed != self->Used)
			{
				//m_sql.UpdateDeviceValue("Used", iUsed, sID);
			}

			// Color change
			if (Color)
			{
				std::string	sColor = "";
				//m_sql.UpdateDeviceValue("Color", sColor, sID);
			}

			// Options provided, assume change
			if (pOptionsDict && PyDict_Check(pOptionsDict))
			{
				//if (self->SubType != sTypeCustom)
				{
					PyObject* pKeyDict, * pValueDict;
					Py_ssize_t pos = 0;
					std::map<std::string, std::string> mpOptions;
					while (PyDict_Next(pOptionsDict, &pos, &pKeyDict, &pValueDict))
					{
						std::string sOptionName = PyUnicode_AsUTF8(pKeyDict);
						PyObject* pStr = PyObject_Str(pValueDict);
						std::string sOptionValue = PyUnicode_AsUTF8(pStr);
						Py_XDECREF(pStr);
						mpOptions.insert(std::pair<std::string, std::string>(sOptionName, sOptionValue));
					}
					//m_sql.SetDeviceOptions(self->ID, mpOptions);
				}
				//else
				{
					std::string sOptionValue = "";
					PyObject* pValue = PyDict_GetItemString(pOptionsDict, "Custom");
					if (pValue)
					{
						sOptionValue = PyUnicode_AsUTF8(pValue);
					}

					time_t now = time(0);
					struct tm ltime;
					localtime_r(&now, &ltime);
					//m_sql.UpdateDeviceValue("Options", iUsed, sID);
					m_sql.safe_query("UPDATE DeviceStatus SET Options='%q', LastUpdate='%04d-%02d-%02d %02d:%02d:%02d' WHERE (HardwareID==%d) and (Unit==%d)",
						sOptionValue.c_str(), ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday, ltime.tm_hour, ltime.tm_min, ltime.tm_sec, self->InterfaceID, self->Unit);
				}
			}

			// TimedOut change (not stored in database, webserver calls back directly to check)
			if (iTimedOut != self->TimedOut)
			{
				self->TimedOut = iTimedOut;
			}

			// Suppress Triggers updates non-key fields only (specifically NOT nValue or sValue)
			if (!SuppressTriggers)
			{
				if (self->pPlugin->m_bDebug & PDM_DEVICE)
				{
					_log.Log(LOG_NORM, "(%s) Updating device from %d:'%s' to have values %d:'%s'.", sName.c_str(), self->nValue, PyUnicode_AsUTF8(self->sValue), nValue, sValue);
				}

				//DevRowIdx = m_sql.UpdateValue(self->InterfaceID, sDeviceID.c_str(), (const unsigned char)self->Unit, (const unsigned char)iType, (const unsigned char)iSubType, iSignalLevel, iBatteryLevel, nValue, sValue, sName, true);

				// Notify MQTT and various push mechanisms and notifications
				m_mainworker.sOnDeviceReceived(self->pPlugin->m_InterfaceID, self->ID, self->pPlugin->m_Name, NULL);
				//m_notifications.CheckAndHandleNotification(DevRowIdx, self->InterfaceID, sDeviceID, sName, self->Unit, iType, iSubType, nValue, sValue);

				// Trigger any associated scene / groups
				//m_mainworker.CheckSceneCode(DevRowIdx, (const unsigned char)self->Type, (const unsigned char)self->SubType, nValue, sValue);
			}

			CDevice_refresh(self);
		}
		else
		{
			_log.Log(LOG_ERROR, "Device update failed, Device object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CDevice_delete(CDevice* self)
	{
		if (self->pPlugin)
		{
			std::string	sName = PyUnicode_AsUTF8(self->Name);
			if (self->ID != -1)
			{
				if (self->pPlugin->m_bDebug & PDM_DEVICE)
				{
					_log.Log(LOG_NORM, "(%s) Deleting device '%s'.", self->pPlugin->m_Name.c_str(), sName.c_str());
				}

				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT Name FROM DeviceStatus WHERE (HardwareID==%d) AND (Unit==%d)", self->InterfaceID, self->Unit);
				if (!result.empty())
				{
					m_sql.safe_query("DELETE FROM DeviceStatus WHERE (HardwareID==%d) AND (Unit==%d)", self->InterfaceID, self->Unit);

					PyObject* pKey = PyLong_FromLong(self->Unit);
					if (PyDict_DelItem((PyObject*)self->pPlugin->m_DeviceDict, pKey) == -1)
					{
						_log.Log(LOG_ERROR, "(%s) failed to delete unit '%d' from device dictionary.", self->pPlugin->m_Name.c_str(), self->Unit);
						Py_INCREF(Py_None);
						return Py_None;
					}
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) Device deletion failed, Hardware/Unit combination (%d:%d) not found in Domoticz.", self->pPlugin->m_Name.c_str(), self->InterfaceID, self->Unit);
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) Device deletion failed, '%s' does not represent a device in Domoticz.", self->pPlugin->m_Name.c_str(), sName.c_str());
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Device deletion failed, Device object is not associated with a plugin.");
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CDevice_touch(CDevice* self)
	{
		if ((self->pPlugin) && (self->InterfaceID != -1) && (self->Unit != -1))
		{
			std::string sID = std::to_string(self->ID);
			m_sql.safe_query("UPDATE DeviceStatus SET LastUpdate='%s' WHERE (ID == %s )", TimeToString(NULL, TF_DateTime).c_str(), sID.c_str());
		}
		else
		{
			_log.Log(LOG_ERROR, "Device touch failed, Device object is not associated with a plugin.");
		}

		return CDevice_refresh(self);
	}

	PyObject* CDevice_str(CDevice* self)
	{
		PyObject* pRetVal = PyUnicode_FromFormat("ID: %d, Name: '%U', nValue: %d, sValue: '%U'", self->ID, self->Name, self->nValue, self->sValue);
		return pRetVal;
	}
}
#endif
