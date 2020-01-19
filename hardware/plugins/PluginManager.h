#pragma once

#include "../../main/StoppableTask.h"
#include <boost/signals2.hpp>
#include <string>
#include "../main/UpdatePublisher.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016
//

class CDomoticzHardwareBase;

namespace Plugins {

	class CPluginSystem : public StoppableTask
	{
	private:
		bool	m_bEnabled;
		bool	m_bAllPluginsStarted;
		int		m_iPollInterval;

		void*	m_InitialPythonThread;

		static	std::map<int, CDomoticzHardwareBase*>	m_pPlugins;

		std::shared_ptr<std::thread> m_thread;
		std::mutex m_mutex;

		boost::signals2::connection m_Subscriber;

		void Do_Work();
	public:
		CPluginSystem();
		~CPluginSystem(void);

		bool StartPluginSystem();
		std::map<int, CDomoticzHardwareBase*>* GetHardware() { return &m_pPlugins; };
		CDomoticzHardwareBase* RegisterPlugin(const int HwdID, const std::string &Name);
		bool	 RestartPlugin(const int InterfaceID);
		void	 DeregisterPlugin(const int HwdID);
		bool	StopPluginSystem();
		void	AllPluginsStarted() { m_bAllPluginsStarted = true; };
		void*	PythonThread() { return m_InitialPythonThread; };
		virtual void DatabaseUpdate(CUpdateEntry*);
	};
};

