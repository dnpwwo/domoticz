#pragma once

#include "DelayedLink.h"
#include "Plugins.h"

#ifndef byte
typedef unsigned char byte;
#endif

namespace Plugins {

	extern std::mutex PythonMutex;			// controls access to Python

	class CPluginMessageBase
	{
	public:
		virtual ~CPluginMessageBase(void) {};

		CPlugin*	m_pPlugin;
		std::string	m_Name;
		int			m_InterfaceID;
		bool		m_Delay;
		time_t		m_When;

	protected:
		CPluginMessageBase(CPlugin* pPlugin) : m_pPlugin(pPlugin), m_InterfaceID(pPlugin->m_InterfaceID), m_Delay(false)
		{
			m_Name = __func__;
			m_When = time(0);
		};
		virtual void ProcessLocked() = 0;
	public:
		virtual const char* Name() { return m_Name.c_str(); };
		virtual const CPlugin*	Plugin() { return m_pPlugin; };
		virtual void Process()
		{
			std::lock_guard<std::mutex> l(PythonMutex);
			m_pPlugin->RestoreThread();
			ProcessLocked();
			m_pPlugin->ReleaseThread();
		};
	};

	// Handles lifecycle management of the Python Connection object
	class CHasConnection
	{
	public:
		CHasConnection(PyObject* Connection) : m_pConnection(Connection)
		{
			Py_XINCREF(m_pConnection);
		};
		~CHasConnection()
		{
			Py_XDECREF(m_pConnection);
		}
		PyObject*	m_pConnection;
	};

	class InitializeMessage : public CPluginMessageBase
	{
	public:
		InitializeMessage(CPlugin* pPlugin) : CPluginMessageBase(pPlugin) { m_Name = __func__; };
		virtual void Process()
		{
			std::lock_guard<std::mutex> l(PythonMutex);
			m_pPlugin->Initialise();
		};
		virtual void ProcessLocked() {};
	};

	// Base callback message class
	class CCallbackBase : public CPluginMessageBase
	{
	protected:
		PyObject* m_Target;
		std::string	m_Callback;
		virtual void ProcessLocked() = 0;
	public:
		CCallbackBase(CPlugin* pPlugin, const std::string& Callback) : CPluginMessageBase(pPlugin), m_Callback(Callback), m_Target(NULL) {};
		CCallbackBase(CPlugin* pPlugin, PyObject* pTarget, const std::string& Callback) : CPluginMessageBase(pPlugin), m_Callback(Callback), m_Target(pTarget) {};
		virtual void Callback(PyObject* pParams)
		{
			if (!m_Target) m_Target = (PyObject*)m_pPlugin->m_Interface;
			if (m_Callback.length()) m_pPlugin->Callback(m_Target, m_Callback, pParams);
		};
		virtual void Callback(PyObject* pTarget, PyObject* pParams)
		{
			if (m_Callback.length()) m_pPlugin->Callback(pTarget, m_Callback, pParams);
		};
		virtual const char* PythonName() { return m_Callback.c_str(); };
	};

	class onStartCallback : public CCallbackBase
	{
	public:
		onStartCallback(CPlugin* pPlugin) : CCallbackBase(pPlugin, "onStart") { m_Name = __func__; };
	protected:
		virtual void ProcessLocked()
		{
			m_pPlugin->Start();
			Callback(NULL);
		};
	};

	class onHeartbeatCallback : public CCallbackBase
	{
	public:
		onHeartbeatCallback(CPlugin* pPlugin) : CCallbackBase(pPlugin, "onHeartbeat") { m_Name = __func__; };
	protected:
		virtual void ProcessLocked()
		{
			Callback(NULL);
		};
	};

	class onStopCallback : public CCallbackBase
	{
	public:
		onStopCallback(CPlugin* pPlugin) : CCallbackBase(pPlugin, "onStop") { m_Name = __func__; };
	protected:
		virtual void ProcessLocked()
		{
			Callback(NULL);
			m_pPlugin->Stop();
		};
	};

#ifdef _WIN32
static std::wstring string_to_wstring(const std::string &str, int codepage)
{
	if (str.empty()) return std::wstring();
	int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
	std::wstring res(sz, 0);
	MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
	return res;
}

static std::string wstring_to_string(const std::wstring &wstr, int codepage)
{
	if (wstr.empty()) return std::string();
	int sz = WideCharToMultiByte(codepage, 0, &wstr[0], (int)wstr.size(), 0, 0, 0, 0);
	std::string res(sz, 0);
	WideCharToMultiByte(codepage, 0, &wstr[0], (int)wstr.size(), &res[0], sz, 0, 0);
	return res;
}

static std::string get_utf8_from_ansi(const std::string &utf8, int codepage)
{
	std::wstring utf16 = string_to_wstring(utf8, codepage);
	std::string ansi = wstring_to_string(utf16, CP_UTF8);
	return ansi;
}
#endif


	// onCreate, onUpdate, onDelete can apply to Interface, Device or Values objects
	class onCreateCallback : public CCallbackBase
	{
	public:
		onCreateCallback(CPlugin* pPlugin, PyObject* pTarget) : CCallbackBase(pPlugin, pTarget, "onCreate") { };
	protected:
		virtual void ProcessLocked()
		{
			Callback(NULL);
			Py_DECREF(m_Target);
		};
	};

	class onUpdateCallback : public CCallbackBase
	{
	public:
		onUpdateCallback(CPlugin* pPlugin, PyObject* pTarget) : CCallbackBase(pPlugin, pTarget, "onUpdate") { };
	protected:
		virtual void ProcessLocked()
		{
			Callback(NULL);
			Py_DECREF(m_Target);
		};
	};

	class onDeleteCallback : public CCallbackBase
	{
	public:
		onDeleteCallback(CPlugin* pPlugin, PyObject* pTarget) : CCallbackBase(pPlugin, pTarget, "onDelete") { };
	protected:
		virtual void ProcessLocked()
		{
			Callback(NULL);
			Py_DECREF(m_Target);
		};
	};


	class onConnectCallback : public CCallbackBase, public CHasConnection
		{
		public:
			onConnectCallback(CPlugin* pPlugin, PyObject* Connection) : CCallbackBase(pPlugin, "onConnect"), CHasConnection(Connection) { m_Name = __func__; };
			onConnectCallback(CPlugin* pPlugin, PyObject* Connection, const int Code, const std::string &Text) : CCallbackBase(pPlugin, "onConnect"), CHasConnection(Connection), m_Status(Code), m_Text(Text) { m_Name = __func__; };
			int						m_Status;
			std::string				m_Text;
		protected:
			virtual void ProcessLocked()
			{
	#ifdef _WIN32
				std::string textUTF8 = get_utf8_from_ansi(m_Text, GetACP());
	#else
				std::string textUTF8 = m_Text; // TODO: Is it safe to assume non-Windows will always be UTF-8?
	#endif
				Callback(m_pConnection, Py_BuildValue("is", m_Status, textUTF8.c_str()));  // 0 is success else socket failure code
			};
		};

	class onMessageCallback : public CCallbackBase, public CHasConnection
	{
	public:
		onMessageCallback(CPlugin* pPlugin, PyObject* Connection, const std::string& Buffer) : CCallbackBase(pPlugin, "onMessage"), CHasConnection(Connection), m_Data(NULL)
		{
			m_Name = __func__;
			m_Buffer.reserve(Buffer.length());
			m_Buffer.assign((const byte*)Buffer.c_str(), (const byte*)Buffer.c_str() + Buffer.length());
		};
		onMessageCallback(CPlugin* pPlugin, PyObject* Connection, const std::vector<byte>& Buffer) : CCallbackBase(pPlugin, "onMessage"), CHasConnection(Connection), m_Data(NULL)
		{
			m_Name = __func__;
			m_Buffer = Buffer;
		};
		onMessageCallback(CPlugin* pPlugin, PyObject* Connection, PyObject* pData) : CCallbackBase(pPlugin, "onMessage"), CHasConnection(Connection)
		{
			m_Name = __func__;
			m_Data = pData;
		};
		std::vector<byte>		m_Buffer;
		PyObject* m_Data;

	protected:
		virtual void ProcessLocked()
		{
			PyObject* pParams = NULL;

			// Data is stored in a single vector of bytes
			if (m_Buffer.size())
			{
				pParams = Py_BuildValue("y#", &m_Buffer[0], m_Buffer.size());
				Callback(m_pConnection, pParams);
			}

			// Data is in a dictionary
			if (m_Data)
			{
				pParams = Py_BuildValue("O", m_Data);
				Callback(m_pConnection, pParams);
				Py_XDECREF(m_Data);
			}
		}
	};

	class onDisconnectCallback : public CCallbackBase, public CHasConnection
	{
	public:
		onDisconnectCallback(CPlugin* pPlugin, PyObject* Connection) : CCallbackBase(pPlugin, "onDisconnect"), CHasConnection(Connection) { m_Name = __func__; };
	protected:
		virtual void ProcessLocked()
		{
			Callback(m_pConnection, NULL);  // 0 is success else socket failure code
		};
	};


	class onCommandCallback : public CCallbackBase
	{
	public:
		onCommandCallback(CPlugin* pPlugin, const std::string& Command, const int level, const std::string &color) : CCallbackBase(pPlugin, "onCommand")
		{
			m_Name = __func__;
			m_fLevel = -273.15f;
			m_Command = Command;
			m_iLevel = level;
			m_iColor = color;
		};
		onCommandCallback(CPlugin* pPlugin, const std::string& Command, const float level) : CCallbackBase(pPlugin, "onCommand")
		{
			m_Name = __func__;
			m_fLevel = level;
			m_Command = Command;
			m_iLevel = -1;
			m_iColor = "";
		};
		std::string				m_Command;
		std::string				m_iColor;
		int						m_iLevel;
		float					m_fLevel;

	protected:
		virtual void ProcessLocked()
		{
			PyObject*	pParams;
			if (m_fLevel != -273.15f)
			{
				pParams = Py_BuildValue("sfs",  m_Command.c_str(), m_fLevel, "");
			}
			else
			{
				pParams = Py_BuildValue("sis", m_Command.c_str(), m_iLevel, m_iColor.c_str());
			}
			Callback(pParams);
		};
	};

	class onSecurityEventCallback : public CCallbackBase
	{
	public:
		onSecurityEventCallback(CPlugin* pPlugin, const int level, const std::string& Description) : CCallbackBase(pPlugin, "onSecurityEvent")
		{
			m_Name = __func__;
			m_iLevel = level;
			m_Description = Description;
		};
		int				m_iLevel;
		std::string		m_Description;

	protected:
		virtual void ProcessLocked()
		{
			PyObject*	pParams = Py_BuildValue("is", m_iLevel, m_Description.c_str());
			Callback(pParams);
		};
	};

	class onNotificationCallback : public CCallbackBase
	{
	public:
		onNotificationCallback(CPlugin* pPlugin,
							const std::string& Subject,
							const std::string& Text,
							const std::string& Name,
							const std::string& Status,
							int Priority,
							const std::string& Sound,
							const std::string& ImageFile) : CCallbackBase(pPlugin, "onNotification")
		{
			m_Name = __func__;
			m_Subject = Subject;
			m_Text = Text;
			m_SuppliedName = Name;
			m_Status = Status;
			m_Priority = Priority;
			m_Sound = Sound;
			m_ImageFile = ImageFile;
		};

		std::string				m_Subject;
		std::string				m_Text;
		std::string				m_SuppliedName;
		std::string				m_Status;
		int						m_Priority;
		std::string				m_Sound;
		std::string				m_ImageFile;

	protected:
		virtual void ProcessLocked()
		{
			PyObject*	pParams = Py_BuildValue("ssssiss", m_SuppliedName.c_str(), m_Subject.c_str(), m_Text.c_str(), m_Status.c_str(), m_Priority, m_Sound.c_str(), m_ImageFile.c_str());
			Callback(pParams);
		};
	};

	// Base directive message class
	class CDirectiveBase : public CPluginMessageBase
	{
	protected:
		virtual void ProcessLocked() = 0;
	public:
		CDirectiveBase(CPlugin* pPlugin) : CPluginMessageBase(pPlugin) {};
		virtual void Process() {
			std::lock_guard<std::mutex> l(PythonMutex);
			m_pPlugin->RestoreThread();
			ProcessLocked();
			m_pPlugin->ReleaseThread();
		};
	};

	class ProtocolDirective : public CDirectiveBase, public CHasConnection
	{
	public:
		ProtocolDirective(CPlugin* pPlugin, PyObject* Connection) : CDirectiveBase(pPlugin), CHasConnection(Connection) { m_Name = __func__; };
		virtual void ProcessLocked() { m_pPlugin->ConnectionProtocol(this); };
	};

	class ConnectDirective : public CDirectiveBase, public CHasConnection
	{
	public:
		ConnectDirective(CPlugin* pPlugin, PyObject* Connection) : CDirectiveBase(pPlugin), CHasConnection(Connection) { m_Name = __func__; };
		virtual void ProcessLocked() { m_pPlugin->ConnectionConnect(this); };
	};

	class ListenDirective : public CDirectiveBase, public CHasConnection
	{
	public:
		ListenDirective(CPlugin* pPlugin, PyObject* Connection) : CDirectiveBase(pPlugin), CHasConnection(Connection) { m_Name = __func__; };
		virtual void ProcessLocked() { m_pPlugin->ConnectionListen(this); };
	};

	class DisconnectDirective : public CDirectiveBase, public CHasConnection
	{
	public:
		DisconnectDirective(CPlugin* pPlugin, PyObject* Connection) : CDirectiveBase(pPlugin), CHasConnection(Connection) { m_Name = __func__; };
		virtual void ProcessLocked() { m_pPlugin->ConnectionDisconnect(this); };
	};

	class WriteDirective : public CDirectiveBase, public CHasConnection
	{
	public:
		PyObject*		m_Object;
		WriteDirective(CPlugin* pPlugin, PyObject* Connection, PyObject* pData, const int Delay) : CDirectiveBase(pPlugin), CHasConnection(Connection)
		{
			m_Name = __func__;
			m_Object = pData;
			if (m_Object)
				Py_INCREF(m_Object);
			if (Delay)
			{
				m_When += Delay;
				m_Delay=true;
			}
		};
		~WriteDirective()
		{
			if (m_Object)
				Py_DECREF(m_Object);
		}

		virtual void ProcessLocked() { m_pPlugin->ConnectionWrite(this); };
	};

	class SettingsDirective : public CDirectiveBase
	{
	public:
		SettingsDirective(CPlugin* pPlugin) : CDirectiveBase(pPlugin) { m_Name = __func__; };
		virtual void ProcessLocked() { m_pPlugin->LoadSettings(); };
	};

	class PollIntervalDirective : public CDirectiveBase
	{
	public:
		PollIntervalDirective(CPlugin* pPlugin, const int PollInterval) : CDirectiveBase(pPlugin), m_Interval(PollInterval) { m_Name = __func__; };
		int						m_Interval;
		virtual void ProcessLocked() {m_pPlugin->PollInterval(m_Interval); };
	};

	// Base event message class
	class CEventBase : public CPluginMessageBase
	{
	protected:
		virtual void ProcessLocked() = 0;
	public:
		CEventBase(CPlugin* pPlugin) : CPluginMessageBase(pPlugin) {};
	};

	class ReadEvent : public CEventBase, public CHasConnection
	{
	public:
		ReadEvent(CPlugin* pPlugin, PyObject* Connection, const int ByteCount, const unsigned char* Data, const int ElapsedMs = -1) : CEventBase(pPlugin), CHasConnection(Connection)
		{
			m_Name = __func__;
			m_ElapsedMs = ElapsedMs;
			m_Buffer.reserve(ByteCount);
			m_Buffer.assign(Data, Data + ByteCount);
		};
		std::vector<byte>		m_Buffer;
		int						m_ElapsedMs;
		virtual void ProcessLocked()
		{
			m_pPlugin->WriteDebugBuffer(m_Buffer, true);
			m_pPlugin->ConnectionRead(this);
		};
	};

	class DisconnectedEvent : public CEventBase, public CHasConnection
	{
	public:
		DisconnectedEvent(CPlugin* pPlugin, PyObject* Connection) : CEventBase(pPlugin), CHasConnection(Connection), bNotifyPlugin(true) { m_Name = __func__; };
		DisconnectedEvent(CPlugin* pPlugin, PyObject* Connection, bool NotifyPlugin) : CEventBase(pPlugin), CHasConnection(Connection), bNotifyPlugin(NotifyPlugin) { m_Name = __func__; };
		virtual void ProcessLocked() { m_pPlugin->DisconnectEvent(this); };
		bool	bNotifyPlugin;
	};
}
