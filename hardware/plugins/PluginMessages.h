#pragma once

#include "DelayedLink.h"
#include "PluginManager.h"
#include "Plugins.h"
#include "PythonConnections.h"
#include "PythonInterfaces.h"
#include "PythonDevices.h"
#include "PythonValues.h"
#include "PluginTransports.h"
#include <deque>

#ifndef byte
typedef unsigned char byte;
#endif

namespace Plugins {

	class CPluginMessageBase
	{
	public:
		virtual ~CPluginMessageBase(void) {};

		CPlugin*	m_pPlugin;
		std::string	m_Name;
		bool		m_Delay;
		time_t		m_When;

	protected:
		CPluginMessageBase(CPlugin* pPlugin) : m_pPlugin(pPlugin), m_Delay(false)
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
			ProcessLocked();
		};
	};

	extern std::deque<CPluginMessageBase*>	PluginMessageQueue;

	// Handles lifecycle management of the Python Connection object
	class CHasConnection
	{
	public:
		CHasConnection(CConnection* Connection) : m_pConnection(Connection)
		{
			Py_XINCREF(m_pConnection);
		};
		~CHasConnection()
		{
			Py_XDECREF(m_pConnection);
		}
		CConnection*	m_pConnection;
	};

	class InitializeMessage : public CPluginMessageBase
	{
	public:
		InitializeMessage(CPlugin* pPlugin) : CPluginMessageBase(pPlugin) { m_Name = __func__; };
		virtual void Process()
		{
			m_pPlugin->Initialise();
		};
		virtual void ProcessLocked() {};
	};

	// Base callback message class, things the framework tells the plugin
	class CCallbackBase : public CPluginMessageBase
	{
	protected:
		PyObject* m_Target;
		std::string	m_Callback;
		virtual void ProcessLocked()
		{
			_log.Log(LOG_ERROR, "(%s) CCallbackBase::ProcessLocked called in error.", m_Name.c_str());
		};
	public:
		CCallbackBase(CPlugin* pPlugin, const std::string& Callback) : CPluginMessageBase(pPlugin), m_Callback(Callback), m_Target(NULL) {};
		CCallbackBase(CPlugin* pPlugin, PyObject* pTarget, const std::string& Callback) : CPluginMessageBase(pPlugin), m_Callback(Callback), m_Target(pTarget) {};
		virtual void Callback(PyObject* pParams)
		{
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
			m_Target = (PyObject*)m_pPlugin->m_Interface;
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
			m_Target = (PyObject*)m_pPlugin->m_Interface;
			Callback(NULL);
		};
	};

	class onStopCallback : public CCallbackBase
	{
	public:
		onStopCallback(CPlugin* pPlugin) : CCallbackBase(pPlugin, "onStop")
		{
			m_Name = __func__;
			m_Delay = true;
			m_When += 1;  // Delay stop to given other events time to filter through
		};
	protected:
		virtual void ProcessLocked()
		{
			// Make sure there are no other messages for the plugin in the queue, if there are, then put the stop message to the back of the queue for later processing
			for (CPluginMessageBase* pMessage : PluginMessageQueue)
			{
				if (pMessage->m_pPlugin == m_pPlugin)
				{
					m_When += 1;  // Wait longer before stopping
					PluginMessageQueue.push_back(this);
					return;
				}
			}

			m_Target = (PyObject*)m_pPlugin->m_Interface;
			Callback(NULL);
			// Interface will be released in Stop so don't do it here
			m_pPlugin->Stop();
			CPluginSystem	PluginSystem;
			PluginSystem.DeregisterPlugin(m_pPlugin->m_InterfaceID);
			m_pPlugin->Finalise();
			m_pPlugin = NULL;
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


	// onCreate, onUpdate, onDelete can apply to Device or Values objects
	class onUpdateInterfaceCallback : public CCallbackBase
{
public:
	onUpdateInterfaceCallback(CPlugin* pPlugin) : CCallbackBase(pPlugin, "onUpdate") { m_Target = (PyObject*)pPlugin->m_Interface; };
protected:
	virtual void Process()
	{
		CInterface* pCurrent = NULL;
		{
			AccessPython	Guard(m_pPlugin, "onUpdateInterfaceCallback 1");
			if (m_Target)
			{
				pCurrent = ((CInterface*)m_Target)->Copy();
				CInterface_refresh((CInterface*)m_Target);
			}
		}

		if (m_Target)
		{
			Callback(Py_BuildValue("(O)", pCurrent));
			AccessPython	Guard(m_pPlugin, "onUpdateInterfaceCallback 2");
			Py_DECREF(pCurrent);
		}
	};
};

	class onCreateDeviceCallback : public CCallbackBase
	{
	private:
		long		m_DeviceID;
	public:
		onCreateDeviceCallback(CPlugin* pPlugin, long lDeviceID) : m_DeviceID(lDeviceID), CCallbackBase(pPlugin, "onCreate") { };
	protected:
		virtual void Process()
		{
			CDevice* pDevice = NULL;

			{
				AccessPython	Guard(m_pPlugin, "onCreateDeviceCallback 1");

				// Check the device has not been manually added by plugin
				pDevice = ((CInterface*)m_pPlugin->m_Interface)->FindDevice(m_DeviceID);
				if (!pDevice)
				{
					// if not then add it to the dictionary
					pDevice = ((CInterface*)m_pPlugin->m_Interface)->AddDeviceToDict(m_DeviceID);
				}
			}

			if (pDevice)
			{
				m_Target = (PyObject*)pDevice;
				Callback(NULL);
				AccessPython	Guard(m_pPlugin, "onCreateDeviceCallback 2");
				Py_DECREF(m_Target);
			}
		};
	};

	class onUpdateDeviceCallback : public CCallbackBase
	{
	protected:
		long		m_DeviceID;
	public:
		onUpdateDeviceCallback(CPlugin* pPlugin, long lDeviceID) : m_DeviceID(lDeviceID), CCallbackBase(pPlugin, "onUpdate") { };
	protected:
		virtual void Process()
		{
			CDevice* pCurrent = NULL;
			{
				AccessPython	Guard(m_pPlugin, "onUpdateDeviceCallback 1");
				m_Target = (PyObject*)m_pPlugin->m_Interface->FindDevice(m_DeviceID);
				if (m_Target)
				{
					pCurrent = ((CDevice*)m_Target)->Copy();
					CDevice_refresh((CDevice*)m_Target);
				}
			}

			if (m_Target)
			{
				Callback(Py_BuildValue("(O)", pCurrent));
				AccessPython	Guard(m_pPlugin, "onUpdateDeviceCallback 2");
				Py_DECREF(m_Target);
				Py_DECREF(pCurrent);
			}
		};
	};

	class onDeleteDeviceCallback : public CCallbackBase
	{
	protected:
		long		m_DeviceID;
	public:
		onDeleteDeviceCallback(long lDeviceID) : m_DeviceID(lDeviceID), CCallbackBase(NULL, "onDelete") { };
	protected:
		virtual void Process()
		{
			// Only detail available is the DeviceID so cycle through plugins and find it
			CPluginSystem	PluginSystem;
			for (std::map<int, CDomoticzHardwareBase*>::iterator it = PluginSystem.GetHardware()->begin(); it != PluginSystem.GetHardware()->end(); it++)
			{
				m_pPlugin = (CPlugin*)it->second;
				AccessPython	Guard(m_pPlugin, "onDeleteDeviceCallback 1");

				CInterface* pInterface = (CInterface*)m_pPlugin->m_Interface;
				m_Target = (PyObject*)pInterface->FindDevice(m_DeviceID);
				if (m_Target)
				{
					PyObjPtr pKey = PyLong_FromLong(m_DeviceID);
					PyDict_DelItem(pInterface->Devices, pKey);
					break; // This one
				}
			}

			if (m_Target)
			{
				Callback(NULL);
				AccessPython	Guard(m_pPlugin, "onDeleteDeviceCallback 2");
				Py_DECREF(m_Target);
			}
		};
	};


	class onCreateValueCallback : public CCallbackBase
	{
	private:
		long		m_DeviceID;
		long		m_ValueID;
	public:
		onCreateValueCallback(long lDeviceID, long lValueID) : m_DeviceID(lDeviceID), m_ValueID(lValueID), CCallbackBase(NULL, "onCreate") { };
	protected:
		virtual void Process()
		{
			{
				// Iterate through all Plugins (Interfaces)
				CPluginSystem	PluginSystem;
				for (std::map<int, CDomoticzHardwareBase*>::iterator it = PluginSystem.GetHardware()->begin(); it != PluginSystem.GetHardware()->end(); it++)
				{
					m_pPlugin = (CPlugin*)it->second;
					AccessPython	Guard(m_pPlugin, "onCreateValueCallback 1");

					CInterface* pInterface = (CInterface*)m_pPlugin->m_Interface;
					CDevice* pDevice = pInterface->FindDevice(m_DeviceID);
					if (pDevice) // This the Device the Value has been added to
					{
						// Check the value has not been manually added by plugin
						m_Target = (PyObject*)pDevice->FindValue(m_ValueID);
						Py_DECREF(pDevice);
						if (!m_Target)
						{
							m_Target = (PyObject*)pDevice->AddValueToDict(m_ValueID);
						}
						break;
					}
				}
			}

			if (m_Target)
			{
				Callback(NULL);
				AccessPython	Guard(m_pPlugin, "onCreateValueCallback 2");
				Py_DECREF(m_Target);
			}
		};
	};

	class onUpdateValueCallback : public CCallbackBase
	{
	private:
		long	m_DeviceID;
		long	m_ValueID;
	public:
		onUpdateValueCallback(long lDeviceID, long lValueID) : m_DeviceID(lDeviceID), m_ValueID(lValueID), CCallbackBase(NULL, "onUpdate") { };
	protected:
		virtual void Process()
		{	
			CValue* pCurrent = NULL;

			// Iterate through all Plugins (Interfaces)
			CPluginSystem	PluginSystem;
			for (std::map<int, CDomoticzHardwareBase*>::iterator it = PluginSystem.GetHardware()->begin(); it != PluginSystem.GetHardware()->end(); it++)
			{
				m_pPlugin = (CPlugin*)it->second;
				AccessPython	Guard(m_pPlugin, "onUpdateValueCallback 1");

				CInterface* pInterface = (CInterface*)m_pPlugin->m_Interface;
				CDevice*	pDevice = pInterface->FindDevice(m_DeviceID);
				if (pDevice) // This the Device the Value belongs to
				{
					m_Target = (PyObject*)pDevice->FindValue(m_ValueID);
					Py_DECREF(pDevice);
					if (m_Target)
					{
						pCurrent = ((CValue*)m_Target)->Copy();
						CValue_refresh((CValue*)m_Target);
					}
					break;
				}
			}

			if (m_Target)
			{
				Callback(Py_BuildValue("(O)", pCurrent));
				AccessPython	Guard(m_pPlugin, "onUpdateValueCallback 2");
				Py_DECREF(m_Target);
				Py_DECREF(pCurrent);
			}
		};
	};

	class onDeleteValueCallback : public CCallbackBase
	{
	protected:
		long	m_ValueID;
	public:
		onDeleteValueCallback(long lValueID) : m_ValueID(lValueID), CCallbackBase(NULL, "onDelete") { };
	protected:
		virtual void Process()
		{
			// Only detail available is the DeviceID so cycle through plugins and find it
			CPluginSystem	PluginSystem;
			for (std::map<int, CDomoticzHardwareBase*>::iterator it = PluginSystem.GetHardware()->begin(); !m_Target && (it != PluginSystem.GetHardware()->end()); it++)
			{
				m_pPlugin = (CPlugin*)it->second;
				AccessPython	Guard(m_pPlugin, "onDeleteValueCallback 1");

				PyObject* pDevicesDict = PyObject_GetAttrString((PyObject*)m_pPlugin->m_Interface, "Devices");
				if (!pDevicesDict || !PyDict_Check(pDevicesDict))
				{
					continue; // Should never happen
				}

				PyObject* key, * pDevice;
				Py_ssize_t pos = 0;
				// For delete we don't know the 'owning' DeviceID so search all
				while (PyDict_Next(pDevicesDict, &pos, &key, &pDevice))
				{
					// And locate it into the Device's Values dictionary
					PyObject* pValuesDict = PyObject_GetAttrString(pDevice, "Values");
					if (!pValuesDict || !PyDict_Check(pValuesDict))
					{
						continue; // Should never happen
					}

					PyObject* pKey = PyLong_FromLong(m_ValueID);
					m_Target = PyDict_GetItem(pValuesDict, pKey);
					if (m_Target)
					{
						// Delete it from the Device's Values dictionary
						Py_INCREF(m_Target);
						PyDict_DelItem(pValuesDict, pKey);
						Py_DECREF(pKey);
						Py_DECREF(pValuesDict);
						break;
					}

					Py_DECREF(pValuesDict);
				}
			}

			if (m_Target)
			{
				Callback(NULL);
				AccessPython	Guard(m_pPlugin, "onDeleteValueCallback 2");
				Py_DECREF(m_Target);
			}
		};
	};


	class onConnectCallback : public CCallbackBase, public CHasConnection
		{
		public:
			onConnectCallback(CPlugin* pPlugin, CConnection* Connection) : CCallbackBase(pPlugin, "onConnect"), CHasConnection(Connection) { m_Name = __func__; };
			onConnectCallback(CPlugin* pPlugin, CConnection* Connection, const int Code, const std::string &Text) : CCallbackBase(pPlugin, "onConnect"), CHasConnection(Connection), m_Status(Code), m_Text(Text)
			{
				m_Name = __func__;
				m_Target = ((CConnection*)m_pConnection)->Target;
			};
			int						m_Status;
			std::string				m_Text;
		protected:
			virtual void Process()
			{
	#ifdef _WIN32
				std::string textUTF8 = get_utf8_from_ansi(m_Text, GetACP());
	#else
				std::string textUTF8 = m_Text; // TODO: Is it safe to assume non-Windows will always be UTF-8?
	#endif
				if (m_Target)
				{
					Callback(m_Target, Py_BuildValue("Ois", m_pConnection, m_Status, textUTF8.c_str()));  // 0 is success else socket failure code
					if (!((CConnection*)m_pConnection)->pTransport->IsConnected())
					{
						// Non-connection based transports only call this on error and connection based protocol will be connected if it wasn't an error so
						// if not connected this is the last event for the connection before another 'Connect' (or 'Listen') so release reference to the target
						AccessPython	Guard(m_pPlugin, "onConnectCallback");
						Py_DECREF(m_Target);
						((CConnection*)m_pConnection)->Target = NULL;
					}
				}
	};
		};

	class onMessageCallback : public CCallbackBase, public CHasConnection
	{
	public:
		onMessageCallback(CPlugin* pPlugin, CConnection* Connection, const std::string& Buffer) : CCallbackBase(pPlugin, "onMessage"), CHasConnection(Connection), m_Data(NULL)
		{
			m_Name = __func__;
			m_Buffer.reserve(Buffer.length());
			m_Buffer.assign((const byte*)Buffer.c_str(), (const byte*)Buffer.c_str() + Buffer.length());
		};
		onMessageCallback(CPlugin* pPlugin, CConnection* Connection, const std::vector<byte>& Buffer) : CCallbackBase(pPlugin, "onMessage"), CHasConnection(Connection), m_Data(NULL)
		{
			m_Name = __func__;
			m_Buffer = Buffer;
		};
		onMessageCallback(CPlugin* pPlugin, CConnection* Connection, PyObject* pData) : CCallbackBase(pPlugin, "onMessage"), CHasConnection(Connection)
		{
			m_Name = __func__;
			m_Data = pData;
		};
		std::vector<byte>		m_Buffer;
		PyObject* m_Data;

	protected:
		virtual void Process()
		{
			PyObject* pParams = NULL;

			// Data is stored in a single vector of bytes
			if (m_Buffer.size())
			{
				Callback(((CConnection*)m_pConnection)->Target, Py_BuildValue("Oy#", m_pConnection, &m_Buffer[0], m_Buffer.size()));
			}

			// Data is in a dictionary
			if (m_Data)
			{
				Callback(((CConnection*)m_pConnection)->Target, Py_BuildValue("OO", m_pConnection, m_Data));
				AccessPython	Guard(m_pPlugin, "onMessageCallback");
				Py_XDECREF(m_Data);
			}
		}
	};

	class onDisconnectCallback : public CCallbackBase, public CHasConnection
	{
	public:
		onDisconnectCallback(CPlugin* pPlugin, CConnection* Connection) : CCallbackBase(pPlugin, "onDisconnect"), CHasConnection(Connection)
		{
			m_Name = __func__;
			m_Target = m_pConnection->Target;
		};
	protected:
		virtual void Process()
		{
			if (m_Target)
			{
				Callback(m_Target, Py_BuildValue("(O)", m_pConnection));  // 0 is success else socket failure code
				AccessPython	Guard(m_pPlugin, "onDisconnectCallback");
				// This is the last event for the connection before another 'Connect' so release reference to the target
				Py_DECREF(m_Target);
				m_pConnection->Target = NULL;
			}
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
		virtual void Process()
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
		virtual void Process()
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
		virtual void Process()
		{
			PyObject*	pParams = Py_BuildValue("ssssiss", m_SuppliedName.c_str(), m_Subject.c_str(), m_Text.c_str(), m_Status.c_str(), m_Priority, m_Sound.c_str(), m_ImageFile.c_str());
			Callback(pParams);
		};
	};

	// Base directive message class, things the plugin directs the framework to do
	class CDirectiveBase : public CPluginMessageBase
	{
	protected:
		virtual void ProcessLocked() = 0;
	public:
		CDirectiveBase(CPlugin* pPlugin) : CPluginMessageBase(pPlugin) {};
		virtual void Process() {
			AccessPython	Guard(m_pPlugin, "CDirectiveBase");
			ProcessLocked();
		};
	};

	class ProtocolDirective : public CDirectiveBase, public CHasConnection
	{
	public:
		ProtocolDirective(CPlugin* pPlugin, CConnection* Connection) : CDirectiveBase(pPlugin), CHasConnection(Connection) { m_Name = __func__; };
		virtual void ProcessLocked() { m_pPlugin->ConnectionProtocol(this); };
	};

	class ConnectDirective : public CDirectiveBase, public CHasConnection
	{
	public:
		ConnectDirective(CPlugin* pPlugin, CConnection* Connection) : CDirectiveBase(pPlugin), CHasConnection(Connection) { m_Name = __func__; };
		virtual void ProcessLocked() { m_pPlugin->ConnectionConnect(this); };
	};

	class ListenDirective : public CDirectiveBase, public CHasConnection
	{
	public:
		ListenDirective(CPlugin* pPlugin, CConnection* Connection) : CDirectiveBase(pPlugin), CHasConnection(Connection) { m_Name = __func__; };
		virtual void ProcessLocked() { m_pPlugin->ConnectionListen(this); };
	};

	class DisconnectDirective : public CDirectiveBase, public CHasConnection
	{
	public:
		DisconnectDirective(CPlugin* pPlugin, CConnection* Connection) : CDirectiveBase(pPlugin), CHasConnection(Connection) { m_Name = __func__; };
		virtual void ProcessLocked() { m_pPlugin->ConnectionDisconnect(this); };
	};

	class WriteDirective : public CDirectiveBase, public CHasConnection
	{
	public:
		PyObject*		m_Object;
		WriteDirective(CPlugin* pPlugin, CConnection* Connection, PyObject* pData, const int Delay) : CDirectiveBase(pPlugin), CHasConnection(Connection)
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

	// Base event message class, external things that happend that the plugin needs to be informed of
	class CEventBase : public CPluginMessageBase
	{
	protected:
		virtual void ProcessLocked() = 0;
	public:
		CEventBase(CPlugin* pPlugin) : CPluginMessageBase(pPlugin) {};
		virtual void Process() {
			AccessPython	Guard(m_pPlugin, "CEventBase");
			ProcessLocked();
		};
	};

	class ReadEvent : public CEventBase, public CHasConnection
	{
	public:
		ReadEvent(CPlugin* pPlugin, CConnection* Connection, const int ByteCount, const unsigned char* Data, const int ElapsedMs = -1) : CEventBase(pPlugin), CHasConnection(Connection)
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
		DisconnectedEvent(CPlugin* pPlugin, CConnection* Connection) : CEventBase(pPlugin), CHasConnection(Connection), bNotifyPlugin(true) { m_Name = __func__; };
		DisconnectedEvent(CPlugin* pPlugin, CConnection* Connection, bool NotifyPlugin) : CEventBase(pPlugin), CHasConnection(Connection), bNotifyPlugin(NotifyPlugin) { m_Name = __func__; };
		virtual void ProcessLocked() { m_pPlugin->DisconnectEvent(this); };
		bool	bNotifyPlugin;
	};
}
