#include "stdafx.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016 - 2020
//
#ifdef ENABLE_PYTHON

#include <tinyxml.h>

#include "Plugins.h"
#include "PluginMessages.h"
#include "PluginProtocols.h"
#include "PluginTransports.h"
#include "PythonInterfaces.h"
#include "PythonDevices.h"
#include "PythonValues.h"
#include "PythonConnections.h"

#include "../main/Helper.h"
#include "../main/SQLHelper.h"
#include "../main/mainworker.h"
#include "../main/localtime_r.h"

#define ADD_STRING_TO_DICT(pDict, key, value) \
		{	\
			PyObject*	pObj = Py_BuildValue("s", value.c_str());	\
			if (PyDict_SetItemString(pDict, key, pObj) == -1)	\
				InterfaceLog(LOG_ERROR, "Failed to add key '%s', value '%s' to dictionary.", key, value.c_str());	\
			Py_DECREF(pObj); \
		}

extern std::string szWWWFolder;
extern std::string szStartupFolder;
extern std::string szUserDataFolder;
extern std::string szWebRoot;
extern std::string dbasefile;
extern std::string szAppVersion;
extern std::string szAppHash;
extern std::string szAppDate;
extern MainWorker m_mainworker;

namespace Plugins {

	extern std::mutex PluginMutex;	// controls access to the message queue
	extern std::deque<CPluginMessageBase*>	PluginMessageQueue;

	void LogPythonException(CPlugin* pPlugin, const std::string &sHandler)
	{
		PyTracebackObject	*pTraceback;
		PyObject			*pExcept, *pValue;
		PyTypeObject		*TypeName;
		PyBytesObject		*pErrBytes = NULL;
		const char*			pTypeText = NULL;
		std::string			Name = "Unknown";

		if (pPlugin)
			Name = pPlugin->m_Name;

		PyErr_Fetch(&pExcept, &pValue, (PyObject**)&pTraceback);

		if (pExcept)
		{
			TypeName = (PyTypeObject*)pExcept;
			pTypeText = TypeName->tp_name;
		}
		if (pValue)
		{
			pErrBytes = (PyBytesObject*)PyUnicode_AsASCIIString(pValue);
		}
		if (pTypeText && pErrBytes)
		{
			if (pPlugin)
				pPlugin->InterfaceLog(LOG_ERROR, "(%s) '%s' failed '%s':'%s'.", Name.c_str(), sHandler.c_str(), pTypeText, pErrBytes->ob_sval);
			else
				_log.Log(LOG_ERROR, "(%s) '%s' failed '%s':'%s'.", Name.c_str(), sHandler.c_str(), pTypeText, pErrBytes->ob_sval);
		}
		if (pTypeText && !pErrBytes)
		{
			if (pPlugin)
				pPlugin->InterfaceLog(LOG_ERROR, "(%s) '%s' failed '%s'.", Name.c_str(), sHandler.c_str(), pTypeText);
			else
				_log.Log(LOG_ERROR, "(%s) '%s' failed '%s'.", Name.c_str(), sHandler.c_str(), pTypeText);
		}
		if (!pTypeText && pErrBytes)
		{
			if (pPlugin)
				pPlugin->InterfaceLog(LOG_ERROR, "(%s) '%s' failed '%s'.", Name.c_str(), sHandler.c_str(), pErrBytes->ob_sval);
			else
				_log.Log(LOG_ERROR, "(%s) '%s' failed '%s'.", Name.c_str(), sHandler.c_str(), pErrBytes->ob_sval);
		}
		if (!pTypeText && !pErrBytes)
		{
			if (pPlugin)
				pPlugin->InterfaceLog(LOG_ERROR, "(%s) '%s' failed, unable to determine error.", Name.c_str(), sHandler.c_str());
			else
				_log.Log(LOG_ERROR, "(%s) '%s' failed, unable to determine error.", Name.c_str(), sHandler.c_str());
		}
		if (pErrBytes) Py_XDECREF(pErrBytes);

		// Log a stack trace if there is one
		while (pTraceback)
		{
			PyFrameObject *frame = pTraceback->tb_frame;
			if (frame)
			{
				int lineno = PyFrame_GetLineNumber(frame);
				PyCodeObject*	pCode = frame->f_code;
				PyBytesObject*	pFileBytes = (PyBytesObject*)PyUnicode_AsASCIIString(pCode->co_filename);
				PyBytesObject*	pFuncBytes = (PyBytesObject*)PyUnicode_AsASCIIString(pCode->co_name);
				if (pPlugin)
					pPlugin->InterfaceLog(LOG_ERROR, "(%s) ----> Line %d in %s, function %s", Name.c_str(), lineno, pFileBytes->ob_sval, pFuncBytes->ob_sval);
				else
					_log.Log(LOG_ERROR, "(%s) ----> Line %d in %s, function %s", Name.c_str(), lineno, pFileBytes->ob_sval, pFuncBytes->ob_sval);
				Py_XDECREF(pFileBytes);
				Py_XDECREF(pFuncBytes);
			}
			pTraceback = pTraceback->tb_next;
		}

		if (!pExcept && !pValue && !pTraceback)
		{
			if (pPlugin)
				pPlugin->InterfaceLog(LOG_ERROR, "(%s) Call to message handler '%s' failed, unable to decode exception.", Name.c_str(), sHandler.c_str());
			else
				_log.Log(LOG_ERROR, "(%s) Call to message handler '%s' failed, unable to decode exception.", Name.c_str(), sHandler.c_str());
		}

		if (pExcept) Py_XDECREF(pExcept);
		if (pValue) Py_XDECREF(pValue);
		if (pTraceback) Py_XDECREF(pTraceback);
	}

	static PyObject* PyDomoticz_Register(PyObject* self, PyObject* args, PyObject* kwds)
	{
		static char* kwlist[] = { "Interface", "Device", "Value", "Connection", NULL };
		module_state* pModState = ((struct module_state*)PyModule_GetState(self));
		if (!pModState)
		{
			_log.Log(LOG_ERROR, "CPlugin:PyDomoticz_Log, unable to obtain module state.");
		}
		else
		{
			PyTypeObject* pInterfaceClass = NULL;
			PyTypeObject* pDeviceClass = NULL;
			PyTypeObject* pValueClass = NULL;
			PyTypeObject* pConnectionClass = NULL;
			if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|OOO", kwlist, &pInterfaceClass, &pDeviceClass, &pValueClass, &pConnectionClass))
			{
				_log.Log(LOG_ERROR, "(%s) PyDomoticz_Log failed to parse parameters: string expected.", pModState->pPlugin->m_Name.c_str());
				LogPythonException(pModState->pPlugin, std::string(__func__));
			}
			else
			{
				if (pInterfaceClass)	pModState->pInterfaceClass = pInterfaceClass;
				if (pDeviceClass)		pModState->pDeviceClass = pDeviceClass;
				if (pValueClass)		pModState->pValueClass = pValueClass;
				if (pConnectionClass)	pModState->pConnectionClass = pConnectionClass;

				_log.Log((_eLogLevel)LOG_NORM, "Registered.");
			}
		}

		Py_INCREF(Py_None);
		return Py_None;
	}

	static PyMethodDef DomoticzMethods[] = {
		{ "Register", (PyCFunction)PyDomoticz_Register, METH_VARARGS | METH_KEYWORDS, "Register Interface,Device and Value override classes." },
		{ NULL, NULL, 0, NULL }
	};

#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))

	static int DomoticzTraverse(PyObject* m, visitproc visit, void* arg) {
		Py_VISIT(GETSTATE(m)->error);
		return 0;
	}

	static int DomoticzClear(PyObject* m) {
		Py_CLEAR(GETSTATE(m)->error);
		return 0;
	}

	struct PyModuleDef DomoticzModuleDef = {
		PyModuleDef_HEAD_INIT,
		"domoticz",
		NULL,
		sizeof(struct module_state),
		DomoticzMethods,
		NULL,
		DomoticzTraverse,
		DomoticzClear,
		NULL
	};

	PyMODINIT_FUNC PyInit_Domoticz(void)
	{

		// This is called during the import of the plugin module
		// triggered by the "import Domoticz" statement
		PyObject* pModule = PyModule_Create2(&DomoticzModuleDef, PYTHON_API_VERSION);

		if (PyType_Ready(&CInterfaceType) < 0)
		{
			_log.Log(LOG_ERROR, "%s, Interface Type not ready.", __func__);
			return pModule;
		}
		Py_INCREF((PyObject*)& CInterfaceType);
		PyModule_AddObject(pModule, "Interface", (PyObject*)&CInterfaceType);

		if (PyType_Ready(&CDeviceType) < 0)
		{
			_log.Log(LOG_ERROR, "%s, Device Type not ready.", __func__);
			return pModule;
		}
		Py_INCREF((PyObject*)& CDeviceType);
		PyModule_AddObject(pModule, "Device", (PyObject*)&CDeviceType);

		if (PyType_Ready(&CValueType) < 0)
		{
			_log.Log(LOG_ERROR, "%s, Value Type not ready.", __func__);
			return pModule;
		}
		Py_INCREF((PyObject*)& CValueType);
		PyModule_AddObject(pModule, "Value", (PyObject*)&CValueType);

		if (PyType_Ready(&CConnectionType) < 0)
		{
			_log.Log(LOG_ERROR, "%s, Connection Type not ready.", __func__);
			return pModule;
		}
		Py_INCREF((PyObject *)&CConnectionType);
		PyModule_AddObject(pModule, "Connection", (PyObject *)&CConnectionType);

		module_state* pModState = ((struct module_state*)PyModule_GetState(pModule));
		pModState->pInterfaceClass = &CInterfaceType;
		pModState->pDeviceClass = &CDeviceType;
		pModState->pValueClass = &CValueType;
		pModState->pConnectionClass = &CConnectionType;
		pModState->lObjectID = 0;

		return pModule;
	}

	std::mutex	AccessPython::PythonMutex;
	volatile bool	AccessPython::m_bHasThreadState = false;

	AccessPython::AccessPython(CPlugin* pPlugin) : m_Python(NULL)
	{
		m_Lock = new std::unique_lock<std::mutex>(PythonMutex, std::defer_lock);
		if (!m_Lock->try_lock())
		{
			_log.Log(LOG_NORM, "Python lock in use, will wait.");
			m_Lock->lock();
		}

		if (pPlugin)
		{
			m_Python = pPlugin->PythonInterpreter();
			if (m_Python)
			{
				PyEval_RestoreThread(m_Python);
				m_bHasThreadState = true;
			}
			else
			{
				_log.Log(LOG_ERROR, "Attempt to aquire the GIL with NULL Interpreter details.");
			}
		}
		else
		{
			_log.Log(LOG_ERROR, "Attempt to aquire the GIL with NULL Plugin details.");
		}

		if (PyErr_Occurred())
		{
			_log.Log(LOG_NORM, "Clearing Python error during Thread Restore.");
			LogPythonException(pPlugin, "Unknown");
			PyErr_Clear();
		}
	}

	AccessPython::~AccessPython()
	{
		if (m_Python)
		{
			m_bHasThreadState = false;
			if (!PyEval_SaveThread())
			{
				_log.Log(LOG_ERROR, "Python Save state returned NULL value.");
			}
		}
		if (m_Lock)
		{
			delete m_Lock;
		}
	}


	CPlugin::CPlugin(const int InterfaceID, const std::string &sName) :
		m_iPollInterval(10),
		m_bDebug(PDM_NONE),
		m_PyInterpreter(NULL),
		m_PyModule(NULL),
		m_Interface(NULL),
		m_SettingsDict(NULL)
	{
		m_InterfaceID = InterfaceID;
		m_Name = sName;
		m_bIsStarted = false;
		m_bIsStarting = false;
		m_bTracing = false;
	}

	CPlugin::~CPlugin(void)
	{
		m_bIsStarted = false;
	}

	void CPlugin::InterfaceLog(const _eLogLevel level, const char* Message, ...)
	{
		va_list argList;
		char cbuffer[1024];
		va_start(argList, Message);
		vsnprintf(cbuffer, sizeof(cbuffer), Message, argList);
		va_end(argList);

		// Firstly do standard log behaviour
		_log.Log(level, &cbuffer[0]);

		// Build SQL statement
		std::string sMessage = cbuffer;
		std::string		sSQL = "INSERT INTO InterfaceLog (InterfaceID, Message) VALUES(?,?);";
		std::vector<std::string> vValues;
		vValues.push_back(std::to_string(m_InterfaceID));
		if (level == LOG_DEBUG_INT)
		{
			sMessage = "DEBUG: " + sMessage;
			if (!m_bDebug)
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
			_log.Log(LOG_ERROR, "Insert into 'InterfaceLog' failed to create a record.");
		}
	}

	void CPlugin::LogPythonException()
	{
		PyTracebackObject	*pTraceback;
		PyObjPtr			pExcept;
		PyObjPtr			pValue;

		PyErr_Fetch(&pExcept, &pValue, (PyObject**)&pTraceback);
		PyErr_NormalizeException(&pExcept, &pValue, (PyObject**)&pTraceback);

		if (pExcept)
		{
			_log.Log(LOG_ERROR, "(%s) Module Import failed, exception: '%s'", m_Name.c_str(), ((PyTypeObject*)pExcept)->tp_name);
		}
		if (pValue)
		{
			std::string		sError;
			PyObjPtr		pErrBytes = PyUnicode_AsASCIIString(pValue);	// Won't normally return text for Import related errors
			if (!pErrBytes)
			{
				// ImportError has name and path attributes
				PyErr_Clear();
				if (PyObject_HasAttrString(pValue, "path"))
				{
					PyObjPtr		pString = PyObject_GetAttrString(pValue, "path");
					PyBytesObject*	pBytes = (PyBytesObject*)PyUnicode_AsASCIIString(pString);
					if (pBytes)
					{
						sError += "Path: ";
						sError += pBytes->ob_sval;
						Py_XDECREF(pBytes);
					}
				}
				PyErr_Clear();
				if (PyObject_HasAttrString(pValue, "name"))
				{
					PyObjPtr		pString = PyObject_GetAttrString(pValue, "name");
					PyBytesObject*	pBytes = (PyBytesObject*)PyUnicode_AsASCIIString(pString);
					if (pBytes)
					{
						sError += " Name: ";
						sError += pBytes->ob_sval;
						Py_XDECREF(pBytes);
					}
				}
				if (!sError.empty())
				{
					_log.Log(LOG_ERROR, "(%s) Module Import failed: '%s'", m_Name.c_str(), sError.c_str());
					sError = "";
				}

				// SyntaxError, IndentationError & TabError have filename, lineno, offset and text attributes
				PyErr_Clear();
				if (PyObject_HasAttrString(pValue, "filename"))
				{
					PyObjPtr		pString = PyObject_GetAttrString(pValue, "filename");
					PyBytesObject*	pBytes = (PyBytesObject*)PyUnicode_AsASCIIString(pString);
					sError += "File: ";
					sError += pBytes->ob_sval;
					Py_XDECREF(pBytes);
				}
				long long	lineno = -1;
				long long 	offset = -1;
				PyErr_Clear();
				if (PyObject_HasAttrString(pValue, "lineno"))
				{
					PyObjPtr		pString = PyObject_GetAttrString(pValue, "lineno");
					lineno = PyLong_AsLongLong(pString);
				}
				PyErr_Clear();
				if (PyObject_HasAttrString(pExcept, "offset"))
				{
					PyObjPtr		pString = PyObject_GetAttrString(pExcept, "offset");
					offset = PyLong_AsLongLong(pString);
				}

				if (!sError.empty())
				{
					if ((lineno > 0) && (lineno < 1000))
					{
						_log.Log(LOG_ERROR, "(%s) Import detail: %s, Line: %lld, offset: %lld", m_Name.c_str(), sError.c_str(), lineno, offset);
					}
					else
					{
						_log.Log(LOG_ERROR, "(%s) Import detail: %s, Line: %lld", m_Name.c_str(), sError.c_str(), offset);
					}
					sError = "";
				}

				PyErr_Clear();
				if (PyObject_HasAttrString(pValue, "text"))
				{
					PyObjPtr		pString = PyObject_GetAttrString(pValue, "text");
					std::string		sUTF = PyUnicode_AsUTF8(pString);
					sUTF.erase(std::remove(sUTF.begin(), sUTF.end(), '\n'), sUTF.end());
					_log.Log(LOG_ERROR, "(%s) Error Line '%s'", m_Name.c_str(), sUTF.c_str());
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) Error Line details not available.", m_Name.c_str());
				}

				if (!sError.empty())
				{
					_log.Log(LOG_ERROR, "(%s) Import detail: %s", m_Name.c_str(), sError.c_str());
				}
			}
			else _log.Log(LOG_ERROR, "(%s) Module Import failed '%s'", m_Name.c_str(), ((PyBytesObject*)pErrBytes)->ob_sval);
		}

		// Log a stack trace if there is one
		PyTracebackObject* pTraceFrame = pTraceback;
		while (pTraceFrame)
		{
			PyFrameObject* frame = pTraceFrame->tb_frame;
			if (frame)
			{
				int lineno = PyFrame_GetLineNumber(frame);
				PyCodeObject* pCode = frame->f_code;
				std::string		FileName = "";
				if (pCode->co_filename)
				{
					PyObjPtr	pFileBytes = PyUnicode_AsASCIIString(pCode->co_filename);
					FileName = ((PyBytesObject*)pFileBytes)->ob_sval;
				}
				std::string		FuncName = "Unknown";
				if (pCode->co_name)
				{
					PyObjPtr	pFuncBytes = PyUnicode_AsASCIIString(pCode->co_name);
					FuncName = ((PyBytesObject*)pFuncBytes)->ob_sval;
				}
				if (!FileName.empty())
					_log.Log(LOG_ERROR, "(%s) ----> Line %d in '%s', function %s", m_Name.c_str(), lineno, FileName.c_str(), FuncName.c_str());
				else
					_log.Log(LOG_ERROR, "(%s) ----> Line %d in '%s'", m_Name.c_str(), lineno, FuncName.c_str());
			}
			pTraceFrame = pTraceFrame->tb_next;
		}

		if (!pExcept && !pValue && !pTraceback)
		{
			_log.Log(LOG_ERROR, "(%s) Call to import module failed, unable to decode exception.", m_Name.c_str());
		}

		if (pTraceback) Py_XDECREF(pTraceback);
	}

	void CPlugin::LogPythonException(const std::string &sHandler)
	{
		PyTracebackObject	*pTraceback;
		PyObjPtr			pExcept;
		PyObjPtr			pValue;
		std::string			sTypeText;
		std::string			sErrorText;

		PyErr_Fetch(&pExcept, &pValue, (PyObject**)&pTraceback);
		PyErr_NormalizeException(&pExcept, &pValue, (PyObject * *)& pTraceback);

		if (pExcept)
		{
			sTypeText = ((PyTypeObject*)pExcept)->tp_name;
		}
		if (pValue)
		{
			PyObjPtr pErrBytes = PyUnicode_AsASCIIString(pValue);
			if (pErrBytes)
			{
				sErrorText = ((PyBytesObject*)pErrBytes)->ob_sval;
			}
		}
		if (sTypeText.length() && sErrorText.length())
		{
			InterfaceLog(LOG_ERROR, "(%s) '%s' failed '%s':'%s'.", m_Name.c_str(), sHandler.c_str(), sTypeText.c_str(), sErrorText.c_str());
		}
		if (sTypeText.length() && !sErrorText.length())
		{
			InterfaceLog(LOG_ERROR, "(%s) '%s' failed '%s'.", m_Name.c_str(), sHandler.c_str(), sTypeText.c_str());
		}
		if (!sTypeText.length() && sErrorText.length())
		{
			InterfaceLog(LOG_ERROR, "(%s) '%s' failed '%s'.", m_Name.c_str(), sHandler.c_str(), sErrorText.c_str());
		}
		if (!sTypeText.length() && !sErrorText.length())
		{
			InterfaceLog(LOG_ERROR, "(%s) '%s' failed, unable to determine error.", m_Name.c_str(), sHandler.c_str());
		}

		// Log a stack trace if there is one
		PyTracebackObject* pTraceFrame = pTraceback;
		while (pTraceFrame)
			{
			PyFrameObject *frame = pTraceFrame->tb_frame;
			if (frame)
			{
				int lineno = PyFrame_GetLineNumber(frame);
				PyCodeObject*	pCode = frame->f_code;
				std::string		FileName = "";
				if (pCode->co_filename)
				{
					//	Current design has no file and Python seems to supply invalid data in that case
					PyObjPtr	pFileBytes = PyUnicode_AsASCIIString(pCode->co_filename);
					FileName = ((PyBytesObject*)pFileBytes)->ob_sval;
				}
				std::string		FuncName = "Unknown";
				if (pCode->co_name)
				{
					PyObjPtr	pFuncBytes = PyUnicode_AsASCIIString(pCode->co_name);
					FuncName = ((PyBytesObject*)pFuncBytes)->ob_sval;
				}
				if (!FileName.empty())
					InterfaceLog(LOG_ERROR, "(%s) ----> Line %d in '%s', function %s", m_Name.c_str(), lineno, FileName.c_str(), FuncName.c_str());
				else
					InterfaceLog(LOG_ERROR, "(%s) ----> Line %d in '%s'", m_Name.c_str(), lineno, FuncName.c_str());
			}
			pTraceFrame = pTraceFrame->tb_next;
		}

		if (!pExcept && !pValue && !pTraceback)
		{
			InterfaceLog(LOG_ERROR, "(%s) Call to message handler '%s' failed, unable to decode exception.", m_Name.c_str(), sHandler.c_str());
		}

		if (pTraceback) Py_XDECREF(pTraceback);
	}

	int CPlugin::PollInterval(int Interval)
	{
		if (Interval > 0)
			m_iPollInterval = Interval;
		if (m_bDebug & PDM_PLUGIN) _log.Log(LOG_NORM, "(%s) Heartbeat interval set to: %d.", m_Name.c_str(), m_iPollInterval);
		return m_iPollInterval;
	}

	void CPlugin::AddConnection(CPluginTransport *pTransport)
	{
		std::lock_guard<std::mutex> l(m_TransportsMutex);
		m_Transports.push_back(pTransport);
	}

	void CPlugin::RemoveConnection(CPluginTransport *pTransport)
	{
		std::lock_guard<std::mutex> l(m_TransportsMutex);
		for (std::vector<CPluginTransport*>::iterator itt = m_Transports.begin(); itt != m_Transports.end(); itt++)
		{
			CPluginTransport*	pPluginTransport = *itt;
			if (pTransport == pPluginTransport)
			{
				m_Transports.erase(itt);
				break;
			}
		}
	}
	
	bool CPlugin::StartHardware()
	{
		if (m_bIsStarted) StopHardware();

		RequestStart();

		//	Add start command to message queue
		m_bIsStarting = true;
		MessagePlugin(new InitializeMessage(this));

		InterfaceLog(LOG_STATUS, "%s interface started.", m_Name.c_str());

		return true;
	}

	void CPlugin::ClearMessageQueue()
	{
		// Copy the event queue to a temporary one, then copy back the events for other plugins
		std::lock_guard<std::mutex> l(PluginMutex);
		std::queue<CPluginMessageBase*>	TempMessageQueue(PluginMessageQueue);
		while (!PluginMessageQueue.empty())
			PluginMessageQueue.pop_front();

		while (!TempMessageQueue.empty())
		{
			CPluginMessageBase* FrontMessage = TempMessageQueue.front();
			TempMessageQueue.pop();
			if (FrontMessage->m_pPlugin == this)
			{
				// log events that will not be processed
				CCallbackBase* pCallback = dynamic_cast<CCallbackBase*>(FrontMessage);
				if (pCallback)
					InterfaceLog(LOG_ERROR, "(%s) Callback event '%s' (Python call '%s') discarded.", m_Name.c_str(), FrontMessage->Name(), pCallback->PythonName());
				else
					InterfaceLog(LOG_ERROR, "(%s) Non-callback event '%s' discarded.", m_Name.c_str(), FrontMessage->Name());
			}
			else
			{
				// Message is for a different plugin so requeue it
				_log.Log(LOG_NORM, "(%s) requeuing '%s' message for '%s'", m_Name.c_str(), FrontMessage->Name(), FrontMessage->Plugin()->m_Name.c_str());
				PluginMessageQueue.push_back(FrontMessage);
			}
		}
	}

	bool CPlugin::StopHardware()
	{
		try
		{
			InterfaceLog(LOG_DEBUG_INT, "%s Stop directive received.", m_Name.c_str());

			// loop on plugin to finish startup
			while (m_bIsStarting)
			{
				sleep_milliseconds(100);
			}

			RequestStop();

			if (m_bIsStarted)
			{
				// If we have connections queue disconnects
				if (m_Transports.size())
				{
					std::lock_guard<std::mutex> lTransports(m_TransportsMutex);
					for (std::vector<CPluginTransport*>::iterator itt = m_Transports.begin(); itt != m_Transports.end(); itt++)
					{
						CPluginTransport*	pPluginTransport = *itt;
						// Tell transport to disconnect if required
						if (pPluginTransport)
						{
							//std::lock_guard<std::mutex> l(PythonMutex); // Take mutex to guard access to CPluginTransport::m_pConnection
							MessagePlugin(new DisconnectDirective(this, pPluginTransport->Connection()));
						}
					}
				}
				else
				{
					// otherwise just signal stop
					MessagePlugin(new onStopCallback(this));
				}
			}
		}
		catch (...)
		{
			//Don't throw from a Stop command
		}

		return true;
	}

	void CPlugin::Do_Work()
	{
		InterfaceLog(LOG_STATUS, "(%s) Entering work loop.", m_Name.c_str());
		int scounter = m_iPollInterval * 2;
		while (!IsStopRequested(500))
		{
			if (!--scounter)
			{
				//	Add heartbeat to message queue
				MessagePlugin(new onHeartbeatCallback(this));
				scounter = m_iPollInterval * 2;
				m_LastHeartbeat = mytime(NULL);
			}

			// Check all connections are still valid, vector could be affected by a disconnect on another thread
			try
			{
				std::lock_guard<std::mutex> lTransports(m_TransportsMutex);
				if (m_Transports.size())
				{
					for (std::vector<CPluginTransport*>::iterator itt = m_Transports.begin(); itt != m_Transports.end(); itt++)
					{
						//std::lock_guard<std::mutex> l(PythonMutex); // Take mutex to guard access to CPluginTransport::m_pConnection
						CPluginTransport*	pPluginTransport = *itt;
						pPluginTransport->VerifyConnection();
					}
				}
			}
			catch (...)
			{
				InterfaceLog(LOG_NORM, "(%s) Transport vector changed during %s loop, continuing.", m_Name.c_str(), __func__);
			}
		}

		InterfaceLog(LOG_STATUS, "(%s) Exiting work loop.", m_Name.c_str());
	}

	bool CPlugin::Initialise()
	{

		m_bIsStarted = false;

		// Load details from database
		std::vector<std::vector<std::string> > result;
		result = m_sql.safe_query("SELECT Script, Configuration, Debug, Notifiable FROM Interface WHERE Active = 1 AND InterfaceID = %d", m_InterfaceID);
		if (!result.empty())
		{
			bool bNDebug = (result[0][2] == "1") ? true : false;
			bool bNotifiable = (result[0][3] == "1") ? true : false;
		}
		else
		{
			_log.Log(LOG_ERROR, "(%s) Active Interface not found in database.", m_Name.c_str());
			goto Error;
		}

		try
		{
			PyEval_RestoreThread((PyThreadState*)m_mainworker.m_pluginsystem.PythonThread());
			m_PyInterpreter = Py_NewInterpreter();
			if (!m_PyInterpreter)
			{
				InterfaceLog(LOG_ERROR, "Failed to create python interpreter.");
				goto Error;
			}

			// Prepend plugin api directory to path so that python will search it early when importing
			std::wstringstream ssPath;
#ifdef WIN32
			std::wstring	sSeparator = L";";
			ssPath << "hardware" << sSeparator << "hardware\\api";
#else
			std::wstring	sSeparator = L":";
			ssPath << "hardware" << sSeparator << "hardware/api";
#endif
			std::wstring	sPath = ssPath.str() + sSeparator;
			sPath += Py_GetPath();

			try
			{
				//
				//	Python loads the 'site' module automatically and adds extra search directories for module loading
				//	This code makes the plugin framework function the same way
				//
				void* pSiteModule = PyImport_ImportModule("site");
				if (!pSiteModule)
				{
					InterfaceLog(LOG_ERROR, "Failed to load 'site' module, continuing.");
				}
				else
				{
					PyObject* pFunc = PyObject_GetAttrString((PyObject*)pSiteModule, "getsitepackages");
					if (pFunc && PyCallable_Check(pFunc))
					{
						PyObject* pSites = PyObject_CallObject(pFunc, NULL);
						if (!pSites)
						{
							LogPythonException("getsitepackages");
						}
						else
							for (Py_ssize_t i = 0; i < PyList_Size(pSites); i++)
							{
								PyObject* pSite = PyList_GetItem(pSites, i);
								if (pSite && PyUnicode_Check(pSite))
								{
									std::wstringstream ssPath;
									ssPath << PyUnicode_AsUTF8(pSite);
									sPath += sSeparator + ssPath.str();
								}
							}
						Py_XDECREF(pSites);
					}
				}
			}
			catch (...)
			{
				InterfaceLog(LOG_ERROR, "Exception loading 'site' module, continuing.");
				PyErr_Clear();
			}

			// Update the path itself
			PySys_SetPath((wchar_t*)sPath.c_str());

			try
			{
				//
				//	Load the 'faulthandler' module to get a python stackdump during a segfault
				//
				void* pFaultModule = PyImport_ImportModule("faulthandler");
				if (!pFaultModule)
				{
					InterfaceLog(LOG_ERROR, "Failed to load 'faulthandler' module, continuing.");
				}
				else
				{
					PyObject* pFunc = PyObject_GetAttrString((PyObject*)pFaultModule, "enable");
					if (pFunc && PyCallable_Check(pFunc))
					{
						PyObject_CallObject(pFunc, NULL);
					}
				}
			}
			catch (...)
			{
				InterfaceLog(LOG_ERROR, "Exception loading 'faulthandler' module, continuing.");
				PyErr_Clear();
			}

			try
			{
				// Just importing something seems to set the intpreter up properly
				m_PyModule = PyImport_ImportModule("api");
				if (!m_PyModule)
				{
					InterfaceLog(LOG_ERROR, "Failed to load API library, Python Path used was '%S'.", sPath.c_str());
					goto Error;
				}
			}
			catch (...)
			{
				InterfaceLog(LOG_ERROR, "Exception loading API library, Python Path used was '%S'.", sPath.c_str());
				PyErr_Clear();
			}

			try
			{
				// Create a module and add code to it
				m_PyModule = PyModule_New(m_Name.c_str());
				if (!m_PyModule)
				{
					InterfaceLog(LOG_ERROR, "Failed to create module to host plugin.");
					goto Error;
				}

				if (PyModule_AddStringConstant((PyObject*)m_PyModule, "__file__", ""))
				{
					InterfaceLog(LOG_ERROR, "Failed to set empty filename for plugin module.");
					goto Error;
				}

				PyObject* localDict = PyModule_GetDict((PyObject*)m_PyModule);   // Returns a borrowed reference: no need to Py_DECREF() it once we are done
				if (!localDict)
				{
					InterfaceLog(LOG_ERROR, "Local dictionary not returned, module intialisation failed.");
					goto Error;
				}

				PyObject* builtins = PyEval_GetBuiltins();  // Returns a borrowed reference: no need to Py_DECREF() it once we are done
				if (!builtins)
				{
					InterfaceLog(LOG_ERROR, "Builtins not returned, module intialisation failed.");
					goto Error;
				}

				if (PyDict_SetItemString(localDict, "__builtins__", builtins))
				{
					InterfaceLog(LOG_ERROR, "Failed to add builtins to local dictionary for plugin module.");
					goto Error;
				}

				// Domoticz callbacks need state so they know which plugin to act on
				PyObject* pMod = PyState_FindModule(&DomoticzModuleDef);
				if (!pMod)
				{
					InterfaceLog(LOG_ERROR, "Start up failed, module not found in interpreter.");
					goto Error;
				}
				module_state* pModState = ((struct module_state*)PyModule_GetState(pMod));
				pModState->pPlugin = this;

				// Define code in the newly created module
				PyObject* pyValue = PyRun_String(result[0][0].c_str(), Py_file_input, localDict, localDict);
				if (!pyValue)
				{
					InterfaceLog(LOG_ERROR, "Module load failed.");
					goto Error;
				}
			}
			catch (...)
			{
				InterfaceLog(LOG_ERROR, "Exception loading interface module, Python Path used was '%S'.", sPath.c_str());
				PyErr_Clear();
			}

			//Start worker thread
			m_thread = std::make_shared<std::thread>(&CPlugin::Do_Work, this);
			std::string plugin_name = "Plugin_" + std::to_string(m_InterfaceID);
			SetThreadName(m_thread->native_handle(), plugin_name.c_str());

			if (PyErr_Occurred())
			{
				InterfaceLog(LOG_ERROR, "Uncaught error during initialisation.");
				goto Error;
			}

			if (!m_thread)
			{
				InterfaceLog(LOG_ERROR, "Failed start worker thread.");
				goto Error;
			}

			//	Add start command to message queue
			MessagePlugin(new onStartCallback(this));

			InterfaceLog(LOG_STATUS, "%s interface initialized.", m_Name.c_str());

			PyEval_SaveThread();
			m_bIsStarting = false;
			return true;
		}
		catch (...)
		{
			InterfaceLog(LOG_ERROR, "Exception caught in '%s'.", m_Name.c_str(), __func__);
		}

	Error:
		if (PyErr_Occurred())
		{
			LogPythonException();
		}
		PyEval_SaveThread();
		m_bIsStarting = false;
		return false;
	}

	bool CPlugin::Finalise()
	{
		InterfaceLog(LOG_DEBUG_INT, "Stopping threads.", m_Name.c_str());

		if (m_thread)
		{
			m_thread->join();
			m_thread.reset();
		}

		InterfaceLog(LOG_STATUS, "%s interface stopped.", m_Name.c_str());

		m_bIsStarting = false;
		return false;
	}

	bool CPlugin::Start()
	{
		try
		{
			// Reset heartbeats in case plugin is restarting
			m_LastHeartbeat = mytime(NULL);
			m_LastHeartbeatReceive = mytime(NULL);

			AccessPython	Guard(this);

			if (PyErr_Occurred())
			{
				_log.Log(LOG_ERROR, "Error already set at beggining of Start.");
				goto Error;
			}

			PyObject* pMod = PyState_FindModule(&DomoticzModuleDef);
			if (!pMod)
			{
				_log.Log(LOG_ERROR, "CPlugin:%s, unable to find module for current interpreter.", __func__);
				goto Error;
			}

			module_state* pModState = ((struct module_state*)PyModule_GetState(pMod));

			PyObject* pModuleDict = PyModule_GetDict((PyObject*)m_PyModule);  // returns a borrowed referece to the __dict__ object for the module
			if (!pModuleDict)
			{
				_log.Log(LOG_ERROR, "Module dictionary not returned.");
				goto Error;
			}

			// load associated devices to make them available to python
			if (m_InterfaceID)
			{
				PyType_Ready(&CInterfaceType);

				pModState->lObjectID = m_InterfaceID;

				// Call the class object, this will call new followed by init
				m_Interface = (CInterface*)PyObject_CallObject((PyObject*)pModState->pInterfaceClass, NULL);
				if (!m_Interface)
				{
					_log.Log(LOG_ERROR, "Interface object creation failed for Interface %d.", m_InterfaceID);
					goto Error;
				}
			}
			else
			{
				_log.Log(LOG_ERROR, "No interface ID set for plugin.");
				goto Error;
			}

			LoadSettings();

			m_bIsStarted = true;
			m_bIsStarting = false;
			return true;
		}
		catch (...)
		{
			InterfaceLog(LOG_ERROR, "Exception caught in '%s'.", __func__);
		}

Error:
		if (PyErr_Occurred())
		{
			LogPythonException("Start");
		}
		m_bIsStarting = false;
		return false;
	}

	void CPlugin::ConnectionProtocol(CDirectiveBase * pMess)
	{
		ProtocolDirective*	pMessage = (ProtocolDirective*)pMess;
		CConnection*		pConnection = (CConnection*)pMessage->m_pConnection;
		std::string	sProtocol = PyUnicode_AsUTF8(pConnection->Protocol);
		pConnection->pProtocol = CPluginProtocol::Create(sProtocol);
		if (m_bDebug & PDM_CONNECTION) _log.Log(LOG_NORM, "(%s) Protocol set to: '%s'.", m_Name.c_str(), sProtocol.c_str());
	}

	void CPlugin::ConnectionConnect(CDirectiveBase * pMess)
	{
		ConnectDirective*	pMessage = (ConnectDirective*)pMess;
		CConnection*		pConnection = (CConnection*)pMessage->m_pConnection;

		m_LastHeartbeatReceive = mytime(NULL);

		if (pConnection->pTransport && pConnection->pTransport->IsConnected())
		{
			_log.Log(LOG_ERROR, "(%s) Current transport is still connected, directive ignored.", m_Name.c_str());
			return;
		}

		if (!pConnection->pProtocol)
		{
			if (m_bDebug & PDM_CONNECTION)
			{
				std::string	sConnection = PyUnicode_AsUTF8(pConnection->Name);
				_log.Log(LOG_NORM, "(%s) Protocol for '%s' not specified, 'None' assumed.", m_Name.c_str(), sConnection.c_str());
			}
			pConnection->pProtocol = new CPluginProtocol();
		}

		std::string	sTransport = PyUnicode_AsUTF8(pConnection->Transport);
		std::string	sAddress = PyUnicode_AsUTF8(pConnection->Address);
		if ((sTransport == "TCP/IP") || (sTransport == "TLS/IP"))
		{
			std::string	sPort = PyUnicode_AsUTF8(pConnection->Port);
			if (m_bDebug & PDM_CONNECTION) _log.Log(LOG_NORM, "(%s) Transport set to: '%s', %s:%s.", m_Name.c_str(), sTransport.c_str(), sAddress.c_str(), sPort.c_str());
			if (sPort.empty())
			{
				_log.Log(LOG_ERROR, "(%s) No port number specified for %s connection to: '%s'.", m_Name.c_str(), sTransport.c_str(), sAddress.c_str());
				return;
			}
			if ((sTransport == "TLS/IP") || pConnection->pProtocol->Secure())
				pConnection->pTransport = (CPluginTransport*) new CPluginTransportTCPSecure(m_InterfaceID, (PyObject*)pConnection, sAddress, sPort);
			else
				pConnection->pTransport = (CPluginTransport*) new CPluginTransportTCP(m_InterfaceID, (PyObject*)pConnection, sAddress, sPort);
		}
		else if (sTransport == "Serial")
		{
			if (pConnection->pProtocol->Secure())  _log.Log(LOG_ERROR, "(%s) Transport '%s' does not support secure connections.", m_Name.c_str(), sTransport.c_str());
			if (m_bDebug & PDM_CONNECTION) _log.Log(LOG_NORM, "(%s) Transport set to: '%s', '%s', %d.", m_Name.c_str(), sTransport.c_str(), sAddress.c_str(), pConnection->Baud);
			pConnection->pTransport = (CPluginTransport*) new CPluginTransportSerial(m_InterfaceID, (PyObject*)pConnection, sAddress, pConnection->Baud);
		}
		else
		{
			_log.Log(LOG_ERROR, "(%s) Invalid transport type for connecting specified: '%s', valid types are TCP/IP and Serial.", m_Name.c_str(), sTransport.c_str());
			return;
		}
		if (pConnection->pTransport)
		{
			AddConnection(pConnection->pTransport);
		}
		if (pConnection->pTransport->handleConnect())
		{
			if (m_bDebug & PDM_CONNECTION) _log.Log(LOG_NORM, "(%s) Connect directive received, action initiated successfully.", m_Name.c_str());
		}
		else
		{
			_log.Log(LOG_NORM, "(%s) Connect directive received, action initiation failed.", m_Name.c_str());
			RemoveConnection(pConnection->pTransport);
		}
	}

	void CPlugin::ConnectionListen(CDirectiveBase * pMess)
	{
		ListenDirective*	pMessage = (ListenDirective*)pMess;
		CConnection*		pConnection = (CConnection*)pMessage->m_pConnection;

		if (pConnection->pTransport && pConnection->pTransport->IsConnected())
		{
			_log.Log(LOG_ERROR, "(%s) Current transport is still connected, directive ignored.", m_Name.c_str());
			return;
		}

		if (!pConnection->pProtocol)
		{
			if (m_bDebug & PDM_CONNECTION)
			{
				std::string	sConnection = PyUnicode_AsUTF8(pConnection->Name);
				_log.Log(LOG_NORM, "(%s) Protocol for '%s' not specified, 'None' assumed.", m_Name.c_str(), sConnection.c_str());
			}
			pConnection->pProtocol = new CPluginProtocol();
		}

		std::string	sTransport = PyUnicode_AsUTF8(pConnection->Transport);
		std::string	sAddress = PyUnicode_AsUTF8(pConnection->Address);
		if (sTransport == "TCP/IP")
		{
			std::string	sPort = PyUnicode_AsUTF8(pConnection->Port);
			if (m_bDebug & PDM_CONNECTION) _log.Log(LOG_NORM, "(%s) Transport set to: '%s', %s:%s.", m_Name.c_str(), sTransport.c_str(), sAddress.c_str(), sPort.c_str());
			if (!pConnection->pProtocol->Secure())
				pConnection->pTransport = (CPluginTransport*) new CPluginTransportTCP(m_InterfaceID, (PyObject*)pConnection, "", sPort);
			else
				pConnection->pTransport = (CPluginTransport*) new CPluginTransportTCPSecure(m_InterfaceID, (PyObject*)pConnection, "", sPort);
		}
		else if (sTransport == "UDP/IP")
		{
			std::string	sPort = PyUnicode_AsUTF8(pConnection->Port);
			if (pConnection->pProtocol->Secure())  _log.Log(LOG_ERROR, "(%s) Transport '%s' does not support secure connections.", m_Name.c_str(), sTransport.c_str());
			if (m_bDebug & PDM_CONNECTION) _log.Log(LOG_NORM, "(%s) Transport set to: '%s', %s:%s.", m_Name.c_str(), sTransport.c_str(), sAddress.c_str(), sPort.c_str());
			pConnection->pTransport = (CPluginTransport*) new CPluginTransportUDP(m_InterfaceID, (PyObject*)pConnection, sAddress.c_str(), sPort);
		}
		else if (sTransport == "ICMP/IP")
		{
			std::string	sPort = PyUnicode_AsUTF8(pConnection->Port);
			if (pConnection->pProtocol->Secure())  _log.Log(LOG_ERROR, "(%s) Transport '%s' does not support secure connections.", m_Name.c_str(), sTransport.c_str());
			if (m_bDebug & PDM_CONNECTION) _log.Log(LOG_NORM, "(%s) Transport set to: '%s', %s.", m_Name.c_str(), sTransport.c_str(), sAddress.c_str());
			pConnection->pTransport = (CPluginTransport*) new CPluginTransportICMP(m_InterfaceID, (PyObject*)pConnection, sAddress.c_str(), sPort);
		}
		else
		{
			_log.Log(LOG_ERROR, "(%s) Invalid transport type for listening specified: '%s', valid types are TCP/IP, UDP/IP and ICMP/IP.", m_Name.c_str(), sTransport.c_str());
			return;
		}
		if (pConnection->pTransport)
		{
			AddConnection(pConnection->pTransport);
		}
		if (pConnection->pTransport->handleListen())
		{
			if (m_bDebug & PDM_CONNECTION) _log.Log(LOG_NORM, "(%s) Listen directive received, action initiated successfully.", m_Name.c_str());
		}
		else
		{
			_log.Log(LOG_NORM, "(%s) Listen directive received, action initiation failed.", m_Name.c_str());
			RemoveConnection(pConnection->pTransport);
		}
	}

	void CPlugin::ConnectionRead(CPluginMessageBase * pMess)
	{
		ReadEvent*	pMessage = (ReadEvent*)pMess;
		CConnection*	pConnection = (CConnection*)pMessage->m_pConnection;
		m_LastHeartbeatReceive = mytime(NULL);
		pConnection->pProtocol->ProcessInbound(pMessage);
	}

	void CPlugin::ConnectionWrite(CDirectiveBase* pMess)
	{
		WriteDirective*	pMessage = (WriteDirective*)pMess;
		CConnection*	pConnection = (CConnection*)pMessage->m_pConnection;
		std::string	sTransport = PyUnicode_AsUTF8(pConnection->Transport);
		std::string	sConnection = PyUnicode_AsUTF8(pConnection->Name);
		if (pConnection->pTransport)
		{
			if (sTransport == "UDP/IP")
			{
				_log.Log(LOG_ERROR, "(%s) Connectionless Transport is listening, write directive to '%s' ignored.", m_Name.c_str(), sConnection.c_str());
				return;
			}

			if ((sTransport != "ICMP/IP") && (!pConnection->pTransport->IsConnected()))
			{
				_log.Log(LOG_ERROR, "(%s) Transport is not connected, write directive to '%s' ignored.", m_Name.c_str(), sConnection.c_str());
				return;
			}
		}

		if (!pConnection->pTransport)
		{
			// UDP is connectionless so create a temporary transport and write to it
			if (sTransport == "UDP/IP")
			{
				std::string	sAddress = PyUnicode_AsUTF8(pConnection->Address);
				std::string	sPort = PyUnicode_AsUTF8(pConnection->Port);
				if (m_bDebug & PDM_CONNECTION)
				{
					if (!sPort.empty())
						_log.Log(LOG_NORM, "(%s) Transport set to: '%s', %s:%s for '%s'.", m_Name.c_str(), sTransport.c_str(), sAddress.c_str(), sPort.c_str(), sConnection.c_str());
					else
						_log.Log(LOG_NORM, "(%s) Transport set to: '%s', %s for '%s'.", m_Name.c_str(), sTransport.c_str(), sAddress.c_str(), sConnection.c_str());
				}
				pConnection->pTransport = (CPluginTransport*) new CPluginTransportUDP(m_InterfaceID, (PyObject*)pConnection, sAddress, sPort);
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) No transport, write directive to '%s' ignored.", m_Name.c_str(), sConnection.c_str());
				return;
			}
		}

		// Make sure there is a protocol to encode the data
		if (!pConnection->pProtocol)
		{
			pConnection->pProtocol = new CPluginProtocol();
		}

		std::vector<byte>	vWriteData = pConnection->pProtocol->ProcessOutbound(pMessage);
		WriteDebugBuffer(vWriteData, false);

		pConnection->pTransport->handleWrite(vWriteData);

		// UDP is connectionless so remove the transport after write
		if (pConnection->pTransport && (sTransport == "UDP/IP"))
		{
			delete pConnection->pTransport;
			pConnection->pTransport = NULL;
		}
	}

	void CPlugin::ConnectionDisconnect(CDirectiveBase * pMess)
	{
		DisconnectDirective*	pMessage = (DisconnectDirective*)pMess;
		CConnection*	pConnection = (CConnection*)pMessage->m_pConnection;

		// Return any partial data to plugin
		if (pConnection->pProtocol)
		{
			pConnection->pProtocol->Flush(pMessage->m_pPlugin, (PyObject*)pConnection);
		}

		if (pConnection->pTransport)
		{
			if (m_bDebug & PDM_CONNECTION)
			{
				std::string	sTransport = PyUnicode_AsUTF8(pConnection->Transport);
				std::string	sAddress = PyUnicode_AsUTF8(pConnection->Address);
				std::string	sPort = PyUnicode_AsUTF8(pConnection->Port);
				if ((sTransport == "Serial") || (sPort.empty()))
					_log.Log(LOG_NORM, "(%s) Disconnect directive received for '%s'.", m_Name.c_str(), sAddress.c_str());
				else
					_log.Log(LOG_NORM, "(%s) Disconnect directive received for '%s:%s'.", m_Name.c_str(), sAddress.c_str(), sPort.c_str());
			}

			// If transport is not going to disconnect asynchronously tidy it up here
			if (!pConnection->pTransport->AsyncDisconnect())
			{
				pConnection->pTransport->handleDisconnect();
				RemoveConnection(pConnection->pTransport);
				delete pConnection->pTransport;
				pConnection->pTransport = NULL;

				// Plugin exiting and all connections have disconnect messages queued
				if (IsStopRequested(0) && !m_Transports.size())
				{
					MessagePlugin(new onStopCallback(this));
				}
			}
			else
			{
				pConnection->pTransport->handleDisconnect();
			}
		}
	}

	void CPlugin::MessagePlugin(CPluginMessageBase *pMessage)
	{
		if (m_bDebug & PDM_QUEUE)
		{
			_log.Log(LOG_NORM, "(" + m_Name + ") Pushing '" + std::string(pMessage->Name()) + "' on to queue");
		}

		// Add message to queue
		std::lock_guard<std::mutex> l(PluginMutex);
		PluginMessageQueue.push_back(pMessage);
	}

	void CPlugin::DisconnectEvent(CEventBase * pMess)
	{
		DisconnectedEvent*	pMessage = (DisconnectedEvent*)pMess;
		CConnection*	pConnection = (CConnection*)pMessage->m_pConnection;

		// Return any partial data to plugin
		if (pConnection->pProtocol)
		{
			pConnection->pProtocol->Flush(pMessage->m_pPlugin, (PyObject*)pConnection);
		}

		if (pConnection->pTransport)
		{
			if (m_bDebug & PDM_CONNECTION)
			{
				std::string	sTransport = PyUnicode_AsUTF8(pConnection->Transport);
				std::string	sAddress = PyUnicode_AsUTF8(pConnection->Address);
				std::string	sPort = PyUnicode_AsUTF8(pConnection->Port);
				if ((sTransport == "Serial") || (sPort.empty()))
					_log.Log(LOG_NORM, "(%s) Disconnect event received for '%s'.", m_Name.c_str(), sAddress.c_str());
				else
					_log.Log(LOG_NORM, "(%s) Disconnect event received for '%s:%s'.", m_Name.c_str(), sAddress.c_str(), sPort.c_str());
			}

			RemoveConnection(pConnection->pTransport);
			delete pConnection->pTransport;
			pConnection->pTransport = NULL;

			// inform the plugin if transport is connection based
			if (pMessage->bNotifyPlugin)
			{
				MessagePlugin(new onDisconnectCallback(this, (PyObject*)pConnection));
			}
			else if (pConnection->Target)
			{
				// Remove reference to event target because the disconnect callback won't do it
				Py_XDECREF(pConnection->Target);
				pConnection->Target = NULL;
			}

			// Plugin exiting and all connections have disconnect messages queued
			if (IsStopRequested(0) && !m_Transports.size())
			{
				MessagePlugin(new onStopCallback(this));
			}
		}
	}

	void CPlugin::Callback(PyObject* pTarget, std::string sHandler, void * pParams)
	{
		try
		{
			AccessPython	Guard(this);

			// Signal that plugin is still servicing events
			m_LastHeartbeat = mytime(NULL);

			// Should never happen but default to module if no target object specified
			if (!pTarget)
			{
				_log.Log(LOG_ERROR, "%s No target specified for '%s'.", __func__, sHandler.c_str());
				return;
			}

			// Callbacks MUST already have taken the PythonMutex lock otherwise bad things will happen
			if (m_PyModule && !sHandler.empty())
			{
				PyObjPtr	pFunc = PyObject_GetAttrString(pTarget, sHandler.c_str());
				if (pFunc && PyCallable_Check(pFunc))
				{
					// if object has debugging set then log this callback
					PyObjPtr pDebugging = PyObject_GetAttrString(pTarget, "Debugging");
					if (pDebugging)
					{
						if (PyObject_IsTrue(pDebugging))
						{
							PyObjPtr pDebug = PyObject_GetAttrString(pTarget, "Debug");
							if (pDebug && PyCallable_Check(pDebug))
							{
								PyObjPtr argList = Py_BuildValue("(s)", std::string("Calling message handler '"+sHandler+"'.").c_str());
								if (!argList)
								{
									_log.Log(LOG_ERROR, "Building Device argument list failed for Debug call.");
									return;
								}
								PyErr_Clear();
								PyObjPtr pReturnValue = PyObject_CallObject(pDebug, argList);
							}
						}
					}

					if (PyErr_Occurred())
					{
						_log.Log(LOG_ERROR, "Python exception set prior to callback '%s'.", sHandler.c_str());
						PyErr_Clear();
					}
					PyObjPtr	pReturnValue = PyObject_CallObject(pFunc, (PyObject*)pParams);
					if (!pReturnValue || PyErr_Occurred())
					{
						LogPythonException(sHandler);
						if (PyErr_Occurred())
						{
							PyErr_Clear();
						}
					}
				}
				else
				{
					if (m_bDebug & PDM_QUEUE) _log.Log(LOG_NORM, "(%s) Message handler '%s' not callable, ignored.", m_Name.c_str(), sHandler.c_str());
					if (PyErr_Occurred())
					{
						PyErr_Clear();
					}
				}
			}

			if (pParams) Py_XDECREF(pParams);
		}
		catch (std::exception *e)
		{
			_log.Log(LOG_ERROR, "%s: Execption thrown: %s", __func__, e->what());
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "%s: Unknown execption thrown", __func__);
		}
	}

	void CPlugin::Stop()
	{
		try
		{
			if (PyErr_Occurred())
			{
				PyErr_Clear();
			}

			PyEval_RestoreThread(m_PyInterpreter);

			// Delete any connection records before releasing the Interface itself
			std::lock_guard<std::mutex> lTransports(m_TransportsMutex);
			for (std::vector<CPluginTransport*>::iterator itt = m_Transports.begin(); itt != m_Transports.end(); itt++)
			{
				CPluginTransport* pPluginTransport = *itt;
				// Tell transport to disconnect if required
				if (pPluginTransport)
				{
					Py_XDECREF(pPluginTransport);
				}
			}

			// Stop Python
			if (m_SettingsDict && PyDict_Size(m_SettingsDict))
			{
				PyDict_Clear(m_SettingsDict);
			}
			if (m_Interface)
			{
				if (m_Interface->ob_base.ob_refcnt != 1)
					_log.Log(LOG_ERROR, "%s: Interface '%d' Reference Count not one: %d.", __func__, m_InterfaceID, m_Interface->ob_base.ob_refcnt);
				Py_XDECREF(m_Interface);
				m_Interface = NULL;
			}
			if (m_PyModule)
			{
				Py_XDECREF(m_PyModule);
				m_PyModule = NULL;
			}
			if (m_PyInterpreter)
			{
				Py_EndInterpreter((PyThreadState*)m_PyInterpreter);
				m_PyInterpreter = NULL;
			}
			PyEval_ReleaseLock();
		}
		catch (std::exception *e)
		{
			_log.Log(LOG_ERROR, "%s: Execption thrown releasing Interpreter: %s", __func__, e->what());
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "%s: Unknown execption thrown releasing Interpreter", __func__);
		}

		ClearMessageQueue();
		m_bIsStarted = false;
	}

	bool CPlugin::LoadSettings()
	{
		PyObject* pModuleDict = PyModule_GetDict(m_PyModule);  // returns a borrowed referece to the __dict__ object for the module
		if (m_SettingsDict) Py_XDECREF(m_SettingsDict);
		m_SettingsDict = PyDict_New();
		if (PyDict_SetItemString(pModuleDict, "Settings", m_SettingsDict) == -1)
		{
			InterfaceLog(LOG_ERROR, "Failed to add Settings dictionary.");
			return false;
		}
		Py_DECREF(m_SettingsDict);

		// load associated settings to make them available to python
		std::vector<std::vector<std::string> > result;
		result = m_sql.safe_query("SELECT Name, Value FROM Preference");
		if (!result.empty())
		{
			// Add settings strings into the settings dictionary with Unit as the key
			for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
			{
				std::vector<std::string> sd = *itt;

				PyObject*	pKey = PyUnicode_FromString(sd[0].c_str());
				PyObject*	pValue = PyUnicode_FromString(sd[1].c_str());
				if (PyDict_SetItem(m_SettingsDict, pKey, pValue))
				{
					InterfaceLog(LOG_ERROR, "Failed to add setting '%s' to settings dictionary.", sd[0].c_str());
					return false;
				}
				Py_XDECREF(pValue);
				Py_XDECREF(pKey);
			}
		}

		return true;
	}

#define DZ_BYTES_PER_LINE 20
	void CPlugin::WriteDebugBuffer(const std::vector<byte>& Buffer, bool Incoming)
	{
		if (m_bDebug & (PDM_CONNECTION | PDM_MESSAGE))
		{
			if (Incoming)
				_log.Log(LOG_NORM, "(%s) Received %d bytes of data", m_Name.c_str(), (int)Buffer.size());
			else
				_log.Log(LOG_NORM, "(%s) Sending %d bytes of data", m_Name.c_str(), (int)Buffer.size());
		}

		if (m_bDebug & PDM_MESSAGE)
		{
			for (int i = 0; i < (int)Buffer.size(); i = i + DZ_BYTES_PER_LINE)
			{
				std::stringstream ssHex;
				std::string sChars;
				for (int j = 0; j < DZ_BYTES_PER_LINE; j++)
				{
					if (i + j < (int)Buffer.size())
					{
						if (Buffer[i + j] < 16)
							ssHex << '0' << std::hex << (int)Buffer[i + j] << " ";
						else
							ssHex << std::hex << (int)Buffer[i + j] << " ";
						if ((int)Buffer[i + j] > 32) sChars += Buffer[i + j];
						else sChars += ".";
					}
					else ssHex << ".. ";
				}
				_log.Log(LOG_NORM, "(%s)     %s    %s", m_Name.c_str(), ssHex.str().c_str(), sChars.c_str());
			}
		}
	}

	bool CPlugin::WriteToHardware(const char *pdata, const unsigned char length)
	{
		return true;
	}
/*
	void CPlugin::SendCommand(const int Unit, const std::string &command, const int level, const _tColor color)
	{
		//	Add command to message queue
		std::string JSONColor = color.toJSONString();
		MessagePlugin(new onCommandCallback(this, Unit, command, level, JSONColor));
	}

	void CPlugin::SendCommand(const int Unit, const std::string & command, const float level)
	{
		//	Add command to message queue
		MessagePlugin(new onCommandCallback(this, Unit, command, level));
	}
*/
}
#endif
