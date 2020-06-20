#pragma once

#include "../DomoticzHardware.h"
#include "DelayedLink.h"
#include "../main/Logger.h"

#ifndef byte
typedef unsigned char byte;
#endif

namespace Plugins {

	class CDirectiveBase;
	class CEventBase;
	class CPluginMessageBase;
	class CPluginNotifier;
	class CPluginTransport;
	class CInterface;

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

		PyThreadState*	m_PyInterpreter;
		PyObject*		m_PyModule;

		std::mutex	m_TransportsMutex;
		std::vector<CPluginTransport*>	m_Transports;

		std::shared_ptr<std::thread> m_thread;

		bool StartHardware() override;
		void Do_Work();
		bool StopHardware() override;
		void ClearMessageQueue();

		void LogPythonException();

	public:
		CPlugin(const int InterfaceID, const std::string &Name);
		~CPlugin(void);

		int		PollInterval(int Interval = -1);

		PyObject* PythonModule() { return m_PyModule; };
		PyThreadState* PythonInterpreter() { return m_PyInterpreter; };

		void	AddConnection(CPluginTransport*);
		void	RemoveConnection(CPluginTransport*);

		bool	Initialise();
		bool	Finalise();
		bool	LoadSettings();
		bool	Start();
		void	ConnectionProtocol(CDirectiveBase*);
		void	ConnectionConnect(CDirectiveBase*);
		void	ConnectionListen(CDirectiveBase*);
		void	ConnectionRead(CPluginMessageBase*);
		void	ConnectionWrite(CDirectiveBase*);
		void	ConnectionDisconnect(CDirectiveBase*);
		void	DisconnectEvent(CEventBase*);
		void	Callback(PyObject*, std::string, void*);
		void	Stop();

		void	WriteToTargetLog(PyObject* pTarget, const char* sLevel, std::string& sMessage);
		void	WriteToTargetLog(PyObject* pTarget, const char* sLevel, const char* Message, ...);

		void	LogPythonException(PyObject*, const std::string&);
		void	LogPythonException(PyObject*);
		void	InterfaceLog(const _eLogLevel level, const char* Message, ...);

		void	WriteDebugBuffer(const std::vector<byte>& Buffer, bool Incoming);

		bool	WriteToHardware(const char *pdata, const unsigned char length) override;
		//void	SendCommand(const int Unit, const std::string &command, const int level, const _tColor color);
		//void	SendCommand(const int Unit, const std::string &command, const float level);

		void	MessagePlugin(CPluginMessageBase *pMessage);

		CInterface*			m_Interface;

		PyObject*			m_SettingsDict;
		PluginDebugMask		m_bDebug;
		bool				m_bIsStarting;
		bool				m_bTracing;
	};

	//
	//	Holds per plugin state details, specifically plugin object, read using PyModule_GetState(PyObject *module)
	//
	struct module_state {
		CPlugin*		pPlugin;
		PyObject*		error;
		PyTypeObject*	pInterfaceClass;
		PyTypeObject*	pDeviceClass;
		PyTypeObject*	pValueClass;
		PyTypeObject*	pConnectionClass;
		long			lObjectID;
	};

	//
	//	Controls access to Python (single threads it)
	//
	class AccessPython
	{
	private:
		static	std::mutex	PythonMutex;
		static  volatile bool		m_bHasThreadState;
		std::unique_lock<std::mutex>*	m_Lock;
		PyThreadState* m_Python;

	public:
		AccessPython(CPlugin* pPlugin);
		~AccessPython();
	};

	//
	//	Controls lifetime of Python Objects to ensure they always release
	//
	class PyObjPtr
	{
	private:
		PyObject* m_pObject;
	public:
		PyObjPtr() : m_pObject(NULL) { };
		PyObjPtr(PyObject* pObject) : m_pObject(pObject) { };
		operator PyObject* () const { return m_pObject; }
		operator PyTypeObject* () const { return (PyTypeObject*)m_pObject; }
		operator PyBytesObject* () const { return (PyBytesObject*)m_pObject; }
		operator bool () const { return (m_pObject != NULL); }
		PyObject** operator &() { return &m_pObject; };
		void operator =(PyObject* pObject)
		{
			if (m_pObject)
			{
				Py_XDECREF(m_pObject);
			}
			m_pObject = pObject;
		}
		~PyObjPtr()
		{
			if (m_pObject)
			{
				if (m_pObject->ob_refcnt <= 0)
				{
					_log.Log(LOG_ERROR, "Reference count error on decref, ref count is '%d'.", m_pObject->ob_refcnt);
				}
				else
				{
					Py_XDECREF(m_pObject);
				}
			}
		};
	};
}
