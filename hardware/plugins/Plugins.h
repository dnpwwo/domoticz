#pragma once

#include "../DomoticzHardware.h"

#ifndef byte
typedef unsigned char byte;
#endif

namespace Plugins {

	class CDirectiveBase;
	class CEventBase;
	class CPluginMessageBase;
	class CPluginNotifier;
	class CPluginTransport;

	enum PluginDebugMask
	{
		PDM_NONE = 0,
		// 1 is mapped to PDM_ALL in code for backwards compatibility
		PDM_PYTHON = 2,
		PDM_PLUGIN = 4,
		PDM_DEVICE = 8,
		PDM_CONNECTION = 16,
		PDM_IMAGE = 32,
		PDM_MESSAGE = 64,
		PDM_QUEUE = 128,
		PDM_ALL = 65535
	};

	class CPlugin : public CDomoticzHardwareBase
	{
	private:
		int				m_iPollInterval;

		void*			m_PyInterpreter;
		void*			m_PyModule;

		CPluginNotifier*	m_Notifier;

		std::mutex	m_TransportsMutex;
		std::vector<CPluginTransport*>	m_Transports;

		std::shared_ptr<std::thread> m_thread;

		bool StartHardware() override;
		void Do_Work();
		bool StopHardware() override;
		void ClearMessageQueue();

		void InterfaceLog(const _eLogLevel level, const char* Message, ...);
		void LogPythonException();
		void LogPythonException(const std::string &);

	public:
		CPlugin(const int InterfaceID, const std::string &Name);
		~CPlugin(void);

		int		PollInterval(int Interval = -1);
		void*	PythonModule() { return m_PyModule; };
		void	Notifier(std::string Notifier = "");
		void	AddConnection(CPluginTransport*);
		void	RemoveConnection(CPluginTransport*);

		bool	Initialise();
		bool	LoadSettings();
		bool	Start();
		void	ConnectionProtocol(CDirectiveBase*);
		void	ConnectionConnect(CDirectiveBase*);
		void	ConnectionListen(CDirectiveBase*);
		void	ConnectionRead(CPluginMessageBase*);
		void	ConnectionWrite(CDirectiveBase*);
		void	ConnectionDisconnect(CDirectiveBase*);
		void	DisconnectEvent(CEventBase*);
		void	Callback(std::string sHandler, void* pParams);
		void	RestoreThread();
		void	ReleaseThread();
		void	Stop();

		void	WriteDebugBuffer(const std::vector<byte>& Buffer, bool Incoming);

		bool	WriteToHardware(const char *pdata, const unsigned char length) override;
		//void	SendCommand(const int Unit, const std::string &command, const int level, const _tColor color);
		//void	SendCommand(const int Unit, const std::string &command, const float level);

		void	MessagePlugin(CPluginMessageBase *pMessage);

		void*				m_DeviceDict;
		void*				m_SettingsDict;
		bool				m_bDebug;
		bool				m_bIsStarting;
		bool				m_bTracing;
	};

	class CNotificationBase {
	};

	class CPluginNotifier : public CNotificationBase
	{
	private:
		CPlugin*	m_pPlugin;
	public:
		CPluginNotifier(CPlugin* pPlugin, const std::string & );
		~CPluginNotifier();
		virtual bool IsConfigured();
	protected:
		virtual bool SendMessageImplementation(
			const uint64_t Idx,
			const std::string &Name,
			const std::string &Subject,
			const std::string &Text,
			const std::string &ExtraData,
			const int Priority,
			const std::string &Sound,
			const bool bFromNotification);
	};

}
