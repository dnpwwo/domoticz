#include "stdafx.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016 - 2020
//
#ifdef ENABLE_PYTHON

#include <tinyxml.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "PluginManager.h"
#include "PluginMessages.h"
#include "PluginTransports.h"
#include "PythonInterfaces.h"
#include "PythonDevices.h"
#include "PythonValues.h"

#include "../main/Helper.h"
#include "../main/Logger.h"
#include "../main/SQLHelper.h"
#include "../main/mainworker.h"
#include "../main/localtime_r.h"
#ifdef WIN32
#	include <direct.h>
#else
#	include <sys/stat.h>
#endif
#include <boost/thread.hpp>
#include "DelayedLink.h"

#define MINIMUM_PYTHON_VERSION "3.4.0"

#define ATTRIBUTE_VALUE(pElement, Name, Value) \
		{	\
			Value = ""; \
			const char*	pAttributeValue = NULL;	\
			if (pElement) pAttributeValue = pElement->Attribute(Name);	\
			if (pAttributeValue) Value = pAttributeValue;	\
		}

#define ATTRIBUTE_NUMBER(pElement, Name, Value) \
		{	\
			Value = 0; \
			const char*	pAttributeValue = NULL;	\
			if (pElement) pAttributeValue = pElement->Attribute(Name);	\
			if (pAttributeValue) Value = atoi(pAttributeValue);	\
		}

extern std::string szUserDataFolder;
extern std::string szPyVersion;

#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))

namespace Plugins {

	extern struct PyModuleDef DomoticzModuleDef;

	PyMODINIT_FUNC PyInit_Domoticz(void);

	std::mutex PluginMutex;	// controls accessto the message queue and m_pPlugins map
	boost::asio::io_service ios;

	std::map<int, CDomoticzHardwareBase*>	CPluginSystem::m_pPlugins;
	void* CPluginSystem::m_InitialPythonThread;

	CPluginSystem::CPluginSystem()
	{
		m_bEnabled = false;
		m_bAllPluginsStarted = false;
		m_iPollInterval = 10;
		//m_InitialPythonThread = NULL;
	}

	CPluginSystem::~CPluginSystem(void)
	{
	}

	bool CPluginSystem::StartPluginSystem()
	{
		m_pPlugins.clear();

		if (!Py_LoadLibrary())
		{
			_log.Log(LOG_STATUS, "PluginSystem: Failed dynamic library load, install the latest libpython3.x library that is available for your platform.");
			return false;
		}

		m_thread = std::make_shared<std::thread>(&CPluginSystem::Do_Work, this);
		SetThreadName(m_thread->native_handle(), "Plugin Manager");

		szPyVersion = Py_GetVersion();

		try
		{
			// Make sure Python is not running
			if (Py_IsInitialized()) {
				Py_Finalize();
			}

			std::string sVersion = szPyVersion.substr(0, szPyVersion.find_first_of(' '));
			if (sVersion < MINIMUM_PYTHON_VERSION)
			{
				_log.Log(LOG_STATUS, "PluginSystem: Invalid Python version '%s' found, '%s' or above required.", sVersion.c_str(), MINIMUM_PYTHON_VERSION);
				return false;
			}

			// Set program name, this prevents it being set to 'python'
			Py_SetProgramName(Py_GetProgramFullPath());

			if (PyImport_AppendInittab("domoticz", PyInit_Domoticz) == -1)
			{
				_log.Log(LOG_ERROR, "PluginSystem: Failed to append 'domoticz' to the existing table of built-in modules.");
				return false;
			}

			Py_Initialize();

			// Initialise threads. Python 3.7+ does this inside Py_Initialize so done here for compatibility
			if (!PyEval_ThreadsInitialized())
			{
				PyEval_InitThreads();
			}

			m_InitialPythonThread = PyEval_SaveThread();

			// Start listening for changes in Interface/Device/Value records
			m_Subscriber = m_mainworker.m_UpdateManager.Publisher.connect(boost::bind(&CPluginSystem::DatabaseUpdate, this, _1));

			m_bEnabled = true;
			_log.Log(LOG_STATUS, "PluginSystem: Started, Python version '%s'.", sVersion.c_str());
		}
		catch (...) {
			_log.Log(LOG_ERROR, "PluginSystem: Failed to start, Python version '%s', Program '%S', Path '%S'.", szPyVersion.c_str(), Py_GetProgramFullPath(), Py_GetPath());
			return false;
		}

		return true;
	}

	bool CPluginSystem::StopPluginSystem()
	{
		m_bAllPluginsStarted = false;

		// Stop subscription to updates
		m_Subscriber.disconnect();

		if (m_thread)
		{
			RequestStop();
			m_thread->join();
			m_thread.reset();
		}

		m_pPlugins.clear();

		if (Py_LoadLibrary() && m_InitialPythonThread)
		{
			if (Py_IsInitialized()) {
				PyEval_RestoreThread((PyThreadState*)m_InitialPythonThread);
				Py_Finalize();
			}
		}

		_log.Log(LOG_STATUS, "PluginSystem: Stopped.");
		return true;
	}

	CDomoticzHardwareBase* CPluginSystem::RegisterPlugin(const int InterfaceID, const std::string & Name)
	{
		CPlugin*	pPlugin = NULL;
		if (m_bEnabled)
		{
			std::lock_guard<std::mutex> l(PluginMutex);
			pPlugin = new CPlugin(InterfaceID, Name);
			m_pPlugins.insert(std::pair<int, CPlugin*>(InterfaceID, pPlugin));
		}
		else
		{
			_log.Log(LOG_STATUS, "PluginSystem: '%s' Registration ignored, Plugins are not enabled.", Name.c_str());
		}
		return reinterpret_cast<CDomoticzHardwareBase*>(pPlugin);
	}

	bool CPluginSystem::RestartPlugin(const int InterfaceID)
	{
		// Stop it if it is running
		if (m_pPlugins.count(InterfaceID))
		{
			CPlugin* pPlugin = (CPlugin*) m_pPlugins.find(InterfaceID)->second;
			if (pPlugin->IsRunning())
			{
				pPlugin->RequestRestart();
				pPlugin->StopHardware();
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "PluginSystem:%s Failed because Interface %d was not active.", __func__, InterfaceID);
			return false;
		}
		return true;
	}

	void CPluginSystem::DeregisterPlugin(const int InterfaceID)
	{
		if (m_pPlugins.count(InterfaceID))
		{
			std::lock_guard<std::mutex> l(PluginMutex);
			m_pPlugins.erase(InterfaceID);
		}
	}

	void BoostWorkers()
	{
		ios.run();
	}

	void CPluginSystem::Do_Work()
	{
		while (!m_bAllPluginsStarted)
		{
			sleep_milliseconds(500);
		}

		_log.Log(LOG_STATUS, "PluginSystem: Entering work loop.");

		// Create initial IO Service thread
		ios.restart();
		// Create some work to keep IO Service alive
		auto work = boost::asio::io_service::work(ios);
		boost::thread_group BoostThreads;
		for (int i = 0; i < 1; i++)
		{
			boost::thread*	bt = BoostThreads.create_thread(BoostWorkers);
			SetThreadName(bt->native_handle(), "Plugin_ASIO");
		}

		while (!IsStopRequested(500))
		{
		}

		// Shutdown IO workers
		ios.stop();
		BoostThreads.join_all();

		_log.Log(LOG_STATUS, "PluginSystem: Exiting work loop.");
	}

	std::string GetFieldValue(std::string fieldName, CUpdateEntry* pEntry)
	{
		for (size_t i = 0; i < pEntry->m_Columns.size(); i++)
		{
			if (pEntry->m_Columns[i] == fieldName)
			{
				return pEntry->m_Values[i];
			}
		}
		return "";
	}

	CPlugin* CPluginSystem::PluginFromDeviceID(long	DeviceID)
	{
		for (std::map<int, CDomoticzHardwareBase*>::iterator it = GetHardware()->begin(); it != GetHardware()->end(); it++)
		{
			CPlugin*	pPlugin = (CPlugin*)it->second;
			//AccessPython	Guard(pPlugin, "PluginFromDeviceID");

			if (pPlugin->m_Interface)
			{
				PyObjPtr	pDevice = (PyObject*)pPlugin->m_Interface->FindDevice(DeviceID);
				if (pDevice)
				{
					return pPlugin;
				}
			}
		}
	
		return NULL;
	}

	CPlugin* CPluginSystem::PluginFromValueID(long	ValueID)
	{
		// Only detail available is the ValueID so cycle through plugins and find it
		for (std::map<int, CDomoticzHardwareBase*>::iterator it = GetHardware()->begin(); it != GetHardware()->end(); it++)
		{
			CPlugin* pPlugin = (CPlugin*)it->second;

			// May not be required but when an error is set an interpreter needs to be 'current' to clear it
			AccessPython	Guard(pPlugin, "PluginFromValueID");

			if (PyErr_Occurred())
			{
				PyErr_Clear();
			}
			if (PyObject_HasAttrString((PyObject*)pPlugin->m_Interface, "Devices"))
			{
				PyObjPtr pDevicesDict = PyObject_GetAttrString((PyObject*)pPlugin->m_Interface, "Devices");
				if (!pDevicesDict || !PyDict_Check(pDevicesDict))
				{
					continue; // Should never happen
				}

				PyObject* key, * pDevice;
				Py_ssize_t pos = 0;
				// we don't know the 'owning' DeviceID so search all
				while (PyDict_Next(pDevicesDict, &pos, &key, &pDevice))
				{
					// And locate it into the Device's Values dictionary
					if (PyErr_Occurred())
					{
						PyErr_Clear();
					}
					if (PyObject_HasAttrString(pDevice, "Values"))
					{
						PyObjPtr pValuesDict = PyObject_GetAttrString(pDevice, "Values");
						if (!pValuesDict || !PyDict_Check(pValuesDict))
						{
							continue; // Should never happen
						}

						PyObject* key, * pValue;
						Py_ssize_t pos = 0;
						// For delete we don't know the 'owning' DeviceID so search all
						while (PyDict_Next(pValuesDict, &pos, &key, &pValue))
						{
							if (((CValue*)pValue)->ValueID == ValueID)
							{
								return pPlugin;
							}
						}
					}
				}
			}
		}

		return NULL;
	};

	void CPluginSystem::DatabaseUpdate(CUpdateEntry*	pEntry)
	{
		if (m_bEnabled) // Ignore events during start up
		{
			std::string		sTable = pEntry->m_Table + "s";
			if (sTable == "Interfaces") {
				int InterfaceID = pEntry->m_RowIdx;
				bool bActive = (GetFieldValue("Active", pEntry) == "1") ? true : false;

				if (pEntry->m_Action == "Insert") {
					if (bActive)
					{
						CDomoticzHardwareBase* pPlugin = RegisterPlugin(InterfaceID, pEntry->m_Values[1]);
						if (pPlugin) pPlugin->Start();
					}
				}
				else if (pEntry->m_Action == "Update") {
					// If update makes interface active but it isn't currently then register it
					if (bActive && !m_pPlugins.count(InterfaceID))
					{
						CDomoticzHardwareBase* pPlugin = RegisterPlugin(InterfaceID, pEntry->m_Values[1]);
						if (pPlugin) pPlugin->Start();
					}
					// If update makes interface inactive but it isn't currently then deregister it
					else if (!bActive && m_pPlugins.count(InterfaceID))
					{
						m_pPlugins.find(InterfaceID)->second->Stop();
					}
					else
					{
						if (m_pPlugins.count(InterfaceID))
						{
							CPlugin* pPlugin = (CPlugin*)m_pPlugins.find(InterfaceID)->second;
							if (pPlugin)
							{
								pPlugin->MessagePlugin(new onUpdateInterfaceCallback(pPlugin));
							}
						}
					}
				}
				else if (pEntry->m_Action == "Delete") {
					// Stop and de-register interface
					if (!bActive && m_pPlugins.count(InterfaceID))
					{
						m_pPlugins.find(InterfaceID)->second->Stop();
					}
				}
				else
				{
					_log.Log(LOG_STATUS, "PluginSystem: Interface change event on '%s', ignored.", pEntry->m_Values[1].c_str());
				}
			}
			else if (sTable == "Devices")
			{
				CPlugin* pPlugin = NULL;
				PyObject* pDevice = NULL;
				long lDeviceID = pEntry->m_RowIdx;
				long lInterfaceID = atoi(GetFieldValue("InterfaceID", pEntry).c_str());

				try
				{
					// locate the Plugin related to the Device
					if (lInterfaceID)
					{
						std::map<int, CDomoticzHardwareBase*>::const_iterator it = m_pPlugins.find(lInterfaceID);
						if (it != m_pPlugins.end())
						{
							pPlugin = (CPlugin*)it->second;
						}
						// Sanity check
						if (!pPlugin)
						{
							// Just continue if the plugin is not running
							return;
						}
					}

					if (pEntry->m_Action == "Insert")
					{
						pPlugin->MessagePlugin(new onCreateDeviceCallback(pPlugin, lDeviceID));
					}
					else if (pEntry->m_Action == "Update") 
					{
						pPlugin->MessagePlugin(new onUpdateDeviceCallback(pPlugin, lDeviceID));
					}
					else if (pEntry->m_Action == "Delete") 
					{
						CPlugin* pPlugin = PluginFromDeviceID(lDeviceID);
						if (pPlugin)
						{
							pPlugin->MessagePlugin(new onDeleteDeviceCallback(pPlugin, lDeviceID));
						}
					}
					else
					{
						_log.Log(LOG_STATUS, "PluginSystem: Device change event on '%s', ignored.", pEntry->m_Values[1].c_str());
					}
				}
				catch (...)
				{
					_log.Log(LOG_ERROR, "Exception caught in '%s'.", __func__);
				}
			}
			else if (sTable == "Values") 
			{
				long lDeviceID = atoi(GetFieldValue("DeviceID", pEntry).c_str());
				long lValueID = pEntry->m_RowIdx;
				CPlugin* pPlugin = NULL;
				PyObject* pDevice = NULL; // Used for insert

				if (pEntry->m_Action == "Insert")
				{
					CPlugin* pPlugin = PluginFromDeviceID(lDeviceID);
					if (pPlugin)
					{
						pPlugin->MessagePlugin(new onCreateValueCallback(pPlugin, lDeviceID, lValueID));
					}
				}
				else if (pEntry->m_Action == "Update")
				{
					CPlugin* pPlugin = PluginFromDeviceID(lDeviceID);
					if (pPlugin)
					{
						pPlugin->MessagePlugin(new onUpdateValueCallback(pPlugin, lDeviceID, lValueID));
					}
				}
				else if (pEntry->m_Action == "Delete") 
				{
					CPlugin* pPlugin = PluginFromValueID(lValueID);
					if (pPlugin)
					{
						pPlugin->MessagePlugin(new onDeleteValueCallback(pPlugin, lValueID));
					}
				}
				else
				{
					_log.Log(LOG_STATUS, "PluginSystem: Value change event on '%s', ignored.", pEntry->m_Values[1].c_str());
				}
			}
			else if (sTable == "Preferences") {
				_log.Log(LOG_STATUS, "PluginSystem: Preference change event on '%s', ignored.", pEntry->m_Values[1].c_str());
				/*
				for (std::map<int, CDomoticzHardwareBase*>::iterator itt = m_pPlugins.begin(); itt != m_pPlugins.end(); ++itt)
				{
					if (itt->second)
					{
						CPlugin* pPlugin = reinterpret_cast<CPlugin*>(itt->second);
						pPlugin->MessagePlugin(new SettingsDirective(pPlugin));
					}
					else
					{
						_log.Log(LOG_ERROR, "%s: NULL entry found in Plugins map for Hardware %d.", __func__, itt->first);
					}
				} */
			}
		}
	}
}
#endif
