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
			// Messages that don't need locking should create their own Process(), otherwise this will call the objects ProcessLocked()
			AccessPython	Guard(m_pPlugin, m_Name.c_str());
			ProcessLocked();
		};
	};

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
		PyObject*	m_Target;
		std::string	m_Callback;
		virtual void ProcessLocked()
		{
			_log.Log(LOG_ERROR, "(%s) CCallbackBase::ProcessLocked called in error.", m_Name.c_str());
		};
	public:
		CCallbackBase(CPlugin* pPlugin, const std::string& Callback) : CPluginMessageBase(pPlugin), m_Callback(Callback), m_Target(NULL) {}
		CCallbackBase(CPlugin* pPlugin, PyObject* pTarget, const std::string& Callback) : CPluginMessageBase(pPlugin), m_Callback(Callback), m_Target(pTarget) {}
		virtual void SetTarget(PyObject* pTarget)
		{
			m_Target = pTarget;
		};
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
			SetTarget((PyObject*)m_pPlugin->m_Interface);
			Callback(NULL);
		};
	};

	class onHeartbeatCallback : public CCallbackBase
	{
	public:
		onHeartbeatCallback(CPlugin* pPlugin) : CCallbackBase(pPlugin, (PyObject*)pPlugin->m_Interface, "onHeartbeat") { m_Name = __func__; };
	protected:
		virtual void ProcessLocked()
		{
			Callback(NULL);
		};
	};

	class onStopCallback : public CCallbackBase
	{
	public:
		onStopCallback(CPlugin* pPlugin) : CCallbackBase(pPlugin, (PyObject*)pPlugin->m_Interface, "onStop")
		{
			m_Name = __func__;
			m_Delay = true;
			m_When = time(0) + 2;  // Delay stop to given other events time to filter through
		};
	protected:
		virtual void Process()
		{
			{
				AccessPython	Guard(m_pPlugin, m_Name.c_str());
				Callback(NULL);
				m_pPlugin->SetStopping();
			}
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
	onUpdateInterfaceCallback(CPlugin* pPlugin) : CCallbackBase(pPlugin, (PyObject*)pPlugin->m_Interface, "onUpdate") { m_Name = __func__; };
protected:
	virtual void ProcessLocked()
	{
		if (m_Target)
		{
			CInterface* pCurrent = ((CInterface*)m_Target)->Copy();
			PyObjPtr	nrParam = Py_BuildValue("(O)", pCurrent);
			Py_XDECREF(pCurrent);

			CInterface_refresh((CInterface*)m_Target);

			Callback(nrParam);
		}
	};
};

	class onCreateDeviceCallback : public CCallbackBase
	{
	private:
		long		m_DeviceID;
	public:
		onCreateDeviceCallback(CPlugin* pPlugin, long lDeviceID) : m_DeviceID(lDeviceID), CCallbackBase(pPlugin, "onCreate") { m_Name = __func__; };
	protected:
		virtual void ProcessLocked()
		{
			CDevice* pDevice = NULL;
			{
				// Check the device has not been manually added by plugin
				pDevice = m_pPlugin->m_Interface->FindDevice(m_DeviceID);
				if (!pDevice)
				{
					// if not then add it to the dictionary
					pDevice = m_pPlugin->m_Interface->AddDeviceToDict(m_DeviceID);
				}
			}

			if (pDevice)
			{
				m_Target = (PyObject*)pDevice;
				Callback(NULL);
				Py_XDECREF(m_Target);
			}
		};
	};

	class onUpdateDeviceCallback : public CCallbackBase
	{
	protected:
		long		m_DeviceID;
	public:
		onUpdateDeviceCallback(CPlugin* pPlugin, long lDeviceID) : m_DeviceID(lDeviceID), CCallbackBase(pPlugin, "onUpdate") { m_Name = __func__; };
	protected:
		virtual void ProcessLocked()
		{
			m_Target = (PyObject*)m_pPlugin->m_Interface->FindDevice(m_DeviceID);
			if (m_Target)
			{
				CDevice* pCurrent = ((CDevice*)m_Target)->Copy();
				CDevice_refresh((CDevice*)m_Target);
				PyObjPtr	nrParam = Py_BuildValue("(O)", pCurrent);
				Py_XDECREF(pCurrent);
				Callback(nrParam);
				Py_XDECREF(m_Target);
			}
		};
	};

	class onDeleteDeviceCallback : public CCallbackBase
	{
	protected:
		long		m_DeviceID;
	public:
		onDeleteDeviceCallback(CPlugin* pPlugin, long lDeviceID) : m_DeviceID(lDeviceID), CCallbackBase(pPlugin, "onDelete") { m_Name = __func__; };
	protected:
		virtual void ProcessLocked()
		{
			CInterface* pInterface = m_pPlugin->m_Interface;
			m_Target = (PyObject*)pInterface->FindDevice(m_DeviceID);
			if (m_Target)
			{
				PyDict_DelItem(pInterface->Devices, ((CDevice*)m_Target)->InternalID);
				Callback(NULL);
				Py_XDECREF(m_Target);
			}
		};
	};


	class onCreateValueCallback : public CCallbackBase
	{
	private:
		long		m_DeviceID;
		long		m_ValueID;
	public:
		onCreateValueCallback(CPlugin* pPlugin, long lDeviceID, long lValueID) : m_DeviceID(lDeviceID), m_ValueID(lValueID), CCallbackBase(pPlugin, "onCreate") { m_Name = __func__; };
	protected:
		virtual void ProcessLocked()
		{
			CDevice* pDevice = m_pPlugin->m_Interface->FindDevice(m_DeviceID);
			if (pDevice) // This the Device the Value has been added to
			{
				// Check the value has not been manually added by plugin
				m_Target = (PyObject*)pDevice->FindValue(m_ValueID);
				Py_XDECREF(pDevice);
				if (!m_Target)
				{
					m_Target = (PyObject*)pDevice->AddValueToDict(m_ValueID);
				}
			}

			if (m_Target)
			{
				Callback(NULL);
				Py_XDECREF(m_Target);
			}
		};
	};

	class onUpdateValueCallback : public CCallbackBase
	{
	private:
		long	m_DeviceID;
		long	m_ValueID;
	public:
		onUpdateValueCallback(CPlugin* pPlugin, long lDeviceID, long lValueID) : m_DeviceID(lDeviceID), m_ValueID(lValueID), CCallbackBase(pPlugin, "onUpdate") { m_Name = __func__; };
	protected:
		virtual void ProcessLocked()
		{	
			CDevice* pDevice = m_pPlugin->m_Interface->FindDevice(m_DeviceID);
			if (pDevice) // This the Device the Value belongs to
			{
				m_Target = (PyObject*)pDevice->FindValue(m_ValueID);
				Py_XDECREF(pDevice);
				if (m_Target)
				{
					CValue*		pCurrent = ((CValue*)m_Target)->Copy();
					PyObjPtr	nrParam = Py_BuildValue("(O)", pCurrent);
					Py_XDECREF(pCurrent);

					CValue_refresh((CValue*)m_Target);

					Callback(nrParam);
					Py_XDECREF(m_Target);
				}
			}
		};
	};

	class onDeleteValueCallback : public CCallbackBase
	{
	protected:
		long	m_ValueID;
	public:
		onDeleteValueCallback(CPlugin* pPlugin, long lValueID) : m_ValueID(lValueID), CCallbackBase(pPlugin, "onDelete") { m_Name = __func__; };
	protected:
		virtual void ProcessLocked()
		{
			// Only detail available is the DeviceID so find it
			PyObjPtr pDevicesDict = PyObject_GetAttrString((PyObject*)m_pPlugin->m_Interface, "Devices");
			if (!pDevicesDict || !PyDict_Check(pDevicesDict))
			{
				return; // Should never happen
			}

			PyObject* key, * pDevice;
			Py_ssize_t pos = 0;
			// For delete we don't know the 'owning' DeviceID so search all
			while (PyDict_Next(pDevicesDict, &pos, &key, &pDevice))
			{
				// And locate it into the Device's Values dictionary
				PyObjPtr pValuesDict = PyObject_GetAttrString(pDevice, "Values");
				if (!pValuesDict || !PyDict_Check(pValuesDict))
				{
					return; // Should never happen
				}

				PyObject* key, * pValue;
				Py_ssize_t pos = 0;
				// For delete we don't know the 'owning' DeviceID so search all
				while (PyDict_Next(pValuesDict, &pos, &key, &pValue))
				{
					if (((CValue*)pValue)->ValueID == m_ValueID)
					{
						m_Target = PyDict_GetItem(pValuesDict, ((CValue*)pValue)->InternalID);
						if (m_Target)
						{
							// Delete it from the Device's Values dictionary
							Py_INCREF(m_Target);
							PyDict_DelItem(pValuesDict, ((CValue*)pValue)->InternalID);
							Callback(NULL);
							Py_XDECREF(m_Target);
							return;
						}
					}
				}
			}
		};
	};


	class onConnectCallback : public CCallbackBase, public CHasConnection
		{
		public:
			onConnectCallback(CPlugin* pPlugin, CConnection* Connection) : CCallbackBase(pPlugin, Connection->Target, "onConnect"), CHasConnection(Connection) { m_Name = __func__; };
			onConnectCallback(CPlugin* pPlugin, CConnection* Connection, const int Code, const std::string &Text) : CCallbackBase(pPlugin, Connection->Target, "onConnect"), CHasConnection(Connection), m_Status(Code), m_Text(Text)
			{
				m_Name = __func__;
			};
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
				if (m_Target)
				{
					PyObjPtr	nrParams = Py_BuildValue("Ois", m_pConnection, m_Status, textUTF8.c_str());
					Callback(m_Target, nrParams);  // 0 is success else socket failure code
					if (!((CConnection*)m_pConnection)->pTransport->IsConnected())
					{
						// Non-connection based transports only call this on error and connection based protocol will be connected if it wasn't an error so
						// if not connected this is the last event for the connection before another 'Connect' (or 'Listen') so release reference to the target
						Py_XDECREF(m_Target);
						((CConnection*)m_pConnection)->Target = NULL;
					}
				}
			};
		};

	class onMessageCallback : public CCallbackBase, public CHasConnection
	{
	public:
		onMessageCallback(CPlugin* pPlugin, CConnection* Connection, const std::string& Buffer) : CCallbackBase(pPlugin, Connection->Target, "onMessage"), CHasConnection(Connection), m_Data(NULL)
		{
			m_Name = __func__;
			m_Buffer.reserve(Buffer.length());
			m_Buffer.assign((const byte*)Buffer.c_str(), (const byte*)Buffer.c_str() + Buffer.length());
		};
		onMessageCallback(CPlugin* pPlugin, CConnection* Connection, const std::vector<byte>& Buffer) : CCallbackBase(pPlugin, Connection->Target, "onMessage"), CHasConnection(Connection), m_Data(NULL)
		{
			m_Name = __func__;
			m_Buffer = Buffer;
		};
		onMessageCallback(CPlugin* pPlugin, CConnection* Connection, PyObject* pData) : CCallbackBase(pPlugin, Connection->Target, "onMessage"), CHasConnection(Connection)
		{
			m_Name = __func__;
			m_Data = pData;
		};
		std::vector<byte>		m_Buffer;
		PyObject* m_Data;

	protected:
		virtual void ProcessLocked()
		{
			PyObjPtr nrParams = NULL;

			if (m_Target)
			{
				// Data is stored in a single vector of bytes
				if (m_Buffer.size())
				{
					nrParams = Py_BuildValue("Oy#", m_pConnection, &m_Buffer[0], m_Buffer.size());
				}
				// Data is in a dictionary
				else if (m_Data)
				{
					nrParams = Py_BuildValue("OO", m_pConnection, m_Data);
					Py_XDECREF(m_Data);
				}
				else
				{
					nrParams = Py_BuildValue("Os", m_pConnection, "");
				}

				// Callback will decrement the parameter reference count
				Callback(m_Target, nrParams);
			}
		}
	};

	class onTimeoutCallback : public CCallbackBase, public CHasConnection
	{
	public:
		onTimeoutCallback(CPlugin* pPlugin, CConnection* Connection) : CCallbackBase(pPlugin, Connection->Target, "onTimeout"), CHasConnection(Connection)
		{
			m_Name = __func__;
		};
	protected:
		virtual void ProcessLocked()
		{
			if (m_Target)
			{
				PyObjPtr nrParams = Py_BuildValue("(O)", m_pConnection);

				Callback(m_Target, nrParams);
			}
		};
	};

	class onDisconnectCallback : public CCallbackBase, public CHasConnection
	{
	public:
		onDisconnectCallback(CPlugin* pPlugin, CConnection* Connection) : CCallbackBase(pPlugin, Connection->Target, "onDisconnect"), CHasConnection(Connection)
		{
			m_Name = __func__;
		};
	protected:
		virtual void ProcessLocked()
		{
			if (m_Target)
			{
				PyObjPtr nrParams = Py_BuildValue("(O)", m_pConnection);

				Callback(m_Target, nrParams);  // 0 is success else socket failure code
				Py_XDECREF(m_Target);
				// This is the last event for the connection before another 'Connect' so release reference to the target
				m_pConnection->Target = NULL;
			}
		};
	};

	class onCommandCallback : public CCallbackBase
	{
	public:
		onCommandCallback(CPlugin* pPlugin, const std::string& Command, const int level, const std::string &color) : CCallbackBase(pPlugin, (PyObject*)pPlugin->m_Interface, "onCommand")
		{
			m_Name = __func__;
			m_fLevel = -273.15f;
			m_Command = Command;
			m_iLevel = level;
			m_iColor = color;
		};
		onCommandCallback(CPlugin* pPlugin, const std::string& Command, const float level) : CCallbackBase(pPlugin, (PyObject*)pPlugin->m_Interface, "onCommand")
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
			PyObject*	nrParams;
			if (m_fLevel != -273.15f)
			{
				nrParams = Py_BuildValue("sfs",  m_Command.c_str(), m_fLevel, "");
			}
			else
			{
				nrParams = Py_BuildValue("sis", m_Command.c_str(), m_iLevel, m_iColor.c_str());
			}
			Callback(nrParams);
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
			PyObjPtr	nrParams = Py_BuildValue("is", m_iLevel, m_Description.c_str());
			Callback(nrParams);
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
							const std::string& ImageFile) : CCallbackBase(pPlugin, (PyObject*)pPlugin->m_Interface, "onNotification")
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
			PyObjPtr	nrParams = Py_BuildValue("ssssiss", m_SuppliedName.c_str(), m_Subject.c_str(), m_Text.c_str(), m_Status.c_str(), m_Priority, m_Sound.c_str(), m_ImageFile.c_str());
			Callback(nrParams);
		};
	};

	// Base directive message class, things the plugin directs the framework to do
	class CDirectiveBase : public CPluginMessageBase
	{
	protected:
		virtual void ProcessLocked() = 0;
	public:
		CDirectiveBase(CPlugin* pPlugin) : CPluginMessageBase(pPlugin) {};
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
				Py_XDECREF(m_Object);
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
			AccessPython	Guard(m_pPlugin, m_Name.c_str());
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
