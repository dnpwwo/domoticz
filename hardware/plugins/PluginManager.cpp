#include "stdafx.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016
//
#ifdef ENABLE_PYTHON

#include <tinyxml.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "PluginManager.h"
#include "Plugins.h"
#include "PluginMessages.h"
#include "PluginTransports.h"

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

	PyMODINIT_FUNC PyInit_Domoticz(void);

#ifdef ENABLE_PYTHON
    // Need forward decleration
    // PyMODINIT_FUNC PyInit_DomoticzEvents(void);
#endif // ENABLE_PYTHON

	std::mutex PluginMutex;	// controls accessto the message queue and m_pPlugins map
	std::queue<CPluginMessageBase*>	PluginMessageQueue;
	boost::asio::io_service ios;

	std::map<int, CDomoticzHardwareBase*>	CPluginSystem::m_pPlugins;
	std::map<std::string, std::string>		CPluginSystem::m_PluginXml;

	CPluginSystem::CPluginSystem()
	{
		m_bEnabled = false;
		m_bAllPluginsStarted = false;
		m_iPollInterval = 10;
		m_InitialPythonThread = NULL;
	}

	CPluginSystem::~CPluginSystem(void)
	{
	}

	bool CPluginSystem::StartPluginSystem()
	{
		// Flush the message queue (should already be empty)
		std::lock_guard<std::mutex> l(PluginMutex);
		while (!PluginMessageQueue.empty())
		{
			PluginMessageQueue.pop();
		}

		m_pPlugins.clear();

		if (!Py_LoadLibrary())
		{
			_log.Log(LOG_STATUS, "PluginSystem: Failed dynamic library load, install the latest libpython3.x library that is available for your platform.");
			return false;
		}

		// Pull UI elements from plugins and create manifest map in memory
		BuildManifest();

		m_thread = std::make_shared<std::thread>(&CPluginSystem::Do_Work, this);
		SetThreadName(m_thread->native_handle(), "PluginMgr");

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

			if (PyImport_AppendInittab("Domoticz", PyInit_Domoticz) == -1)
			{
				_log.Log(LOG_ERROR, "PluginSystem: Failed to append 'Domoticz' to the existing table of built-in modules.");
				return false;
			}

			Py_Initialize();

			// Initialise threads. Python 3.7+ does this inside Py_Initialize so done here for compatibility
			if (!PyEval_ThreadsInitialized())
			{
				PyEval_InitThreads();
			}

			m_InitialPythonThread = PyEval_SaveThread();

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

		if (m_thread)
		{
			RequestStop();
			m_thread->join();
			m_thread.reset();
		}

		// Hardware should already be stopped so just flush the queue (should already be empty)
		std::lock_guard<std::mutex> l(PluginMutex);
		while (!PluginMessageQueue.empty())
		{
			CPluginMessageBase* Message = PluginMessageQueue.front();
			const CPlugin* pPlugin = Message->Plugin();
			if (pPlugin)
			{
				_log.Log(LOG_NORM, "(" + pPlugin->m_Name + ") ' flushing " + std::string(Message->Name()) + "' queue entry");
			}
			PluginMessageQueue.pop();
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

	void CPluginSystem::LoadSettings()
	{
		//	Add command to message queue for every plugin
		for (std::map<int, CDomoticzHardwareBase*>::iterator itt = m_pPlugins.begin(); itt != m_pPlugins.end(); ++itt)
		{
			if (itt->second)
			{
				CPlugin*	pPlugin = reinterpret_cast<CPlugin*>(itt->second);
				pPlugin->MessagePlugin(new SettingsDirective(pPlugin));
			}
			else
			{
				_log.Log(LOG_ERROR, "%s: NULL entry found in Plugins map for Hardware %d.", __func__, itt->first);
			}
		}
	}

	void CPluginSystem::BuildManifest()
	{
		//
		//	Scan plugins folder and load XML plugin manifests
		//
		m_PluginXml.clear();
		std::string plugin_BaseDir;
#ifdef WIN32
		plugin_BaseDir = szUserDataFolder + "plugins\\";
#else
		plugin_BaseDir = szUserDataFolder + "plugins/";
#endif
		if (!createdir(plugin_BaseDir.c_str(), 0755))
		{
			_log.Log(LOG_NORM, "%s: Created directory %s", __func__, plugin_BaseDir.c_str());
		}

		std::vector<std::string> DirEntries, FileEntries;
		std::vector<std::string>::const_iterator itt_Dir, itt_File;
		std::string plugin_Dir, plugin_File;

		DirectoryListing(DirEntries, plugin_BaseDir, true, false);
		for (itt_Dir = DirEntries.begin(); itt_Dir != DirEntries.end(); ++itt_Dir)
		{
			if (*itt_Dir != "examples")
			{
#ifdef WIN32
				plugin_Dir = plugin_BaseDir + *itt_Dir + "\\";
#else
				plugin_Dir = plugin_BaseDir + *itt_Dir + "/";
#endif
				DirectoryListing(FileEntries, plugin_Dir, false, true);
				for (itt_File = FileEntries.begin(); itt_File != FileEntries.end(); ++itt_File)
				{
					if (*itt_File == "plugin.py")
					{
						try
						{
							std::string sXML;
							plugin_File = plugin_Dir + *itt_File;
							std::string line;
							std::ifstream readFile(plugin_File.c_str());
							bool bFound = false;
							while (getline(readFile, line)) {
								if (!bFound && (line.find("<plugin") != std::string::npos))
									bFound = true;
								if (bFound)
									sXML += line + '\n';
								if (line.find("</plugin>") != std::string::npos)
									break;
							}
							readFile.close();
							m_PluginXml.insert(std::pair<std::string, std::string>(plugin_Dir, sXML));
						}
						catch (...)
						{
							_log.Log(LOG_ERROR, "%s: Exception loading plugin file: '%s'", __func__, plugin_File.c_str());
						}
					}
				}
				FileEntries.clear();
			}
		}
	}

	CDomoticzHardwareBase* CPluginSystem::RegisterPlugin(const int HwdID, const std::string & Name, const std::string & PluginKey)
	{
		CPlugin*	pPlugin = NULL;
		if (m_bEnabled)
		{
			std::lock_guard<std::mutex> l(PluginMutex);
			pPlugin = new CPlugin(HwdID, Name, PluginKey);
			m_pPlugins.insert(std::pair<int, CPlugin*>(HwdID, pPlugin));
		}
		else
		{
			_log.Log(LOG_STATUS, "PluginSystem: '%s' Registration ignored, Plugins are not enabled.", Name.c_str());
		}
		return reinterpret_cast<CDomoticzHardwareBase*>(pPlugin);
	}

	void CPluginSystem::DeregisterPlugin(const int HwdID)
	{
		if (m_pPlugins.count(HwdID))
		{
			std::lock_guard<std::mutex> l(PluginMutex);
			m_pPlugins.erase(HwdID);
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

		while (!IsStopRequested(50))
		{
			time_t	Now = time(0);
			bool	bProcessed = true;
			while (bProcessed)
			{
				CPluginMessageBase* Message = NULL;
				bProcessed = false;

				// Cycle once through the queue looking for the 1st message that is ready to process
				{
					std::lock_guard<std::mutex> l(PluginMutex);
					for (size_t i = 0; i < PluginMessageQueue.size(); i++)
					{
						CPluginMessageBase* FrontMessage = PluginMessageQueue.front();
						PluginMessageQueue.pop();
						if (!FrontMessage->m_Delay || FrontMessage->m_When <= Now)
						{
							// Message is ready now or was already ready (this is the case for almost all messages)
							Message = FrontMessage;
							break;
						}
						// Message is for sometime in the future so requeue it (this happens when the 'Delay' parameter is used on a Send)
						PluginMessageQueue.push(FrontMessage);
					}
				}

				if (Message)
				{
					bProcessed = true;
					try
					{
						const CPlugin* pPlugin = Message->Plugin();
						if (pPlugin && (pPlugin->m_bDebug & PDM_QUEUE))
						{
							_log.Log(LOG_NORM, "(" + pPlugin->m_Name + ") Processing '" + std::string(Message->Name()) + "' message");
						}
						Message->Process();
					}
					catch(...)
					{
						_log.Log(LOG_ERROR, "PluginSystem: Exception processing message.");
					}
				}
				// Free the memory for the message
				if (Message)
				{
					std::lock_guard<std::mutex> l(PythonMutex); // Take mutex to guard access to CPluginTransport::m_pConnection inside the message
					CPlugin* pPlugin = (CPlugin*)Message->Plugin();
					pPlugin->RestoreThread();
					delete Message;
					pPlugin->ReleaseThread();
				}
			}
		}

		// Shutdown IO workers
		ios.stop();
		BoostThreads.join_all();

		_log.Log(LOG_STATUS, "PluginSystem: Exiting work loop.");
	}

	void CPluginSystem::DeviceModified(uint64_t ID)
	{
		std::vector<std::vector<std::string> > result;
		result = m_sql.safe_query("SELECT HardwareID,Unit FROM DeviceStatus WHERE (ID == %" PRIu64 ")", ID);
		if (!result.empty())
		{
			std::vector<std::string> sd = result[0];
			std::string sHwdID = sd[0];
			std::string Unit = sd[1];
			//CDomoticzHardwareBase *pHardware = m_mainworker.GetHardwareByIDType(sHwdID, HTYPE_PythonPlugin);
			CDomoticzHardwareBase *pHardware = NULL;
			if (pHardware != NULL)
			{
				//std::vector<std::string> sd = result[0];
				_log.Debug(DEBUG_NORM, "CPluginSystem::DeviceModified: Notifying plugin %u about modification of device %u", atoi(sHwdID.c_str()), atoi(Unit.c_str()));
				Plugins::CPlugin *pPlugin = (Plugins::CPlugin*)pHardware;
				pPlugin->DeviceModified(atoi(Unit.c_str()));
			}
		}
	}
}
#endif
