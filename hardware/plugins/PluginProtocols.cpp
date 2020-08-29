#include "stdafx.h"

//
//	Domoticz Plugin System - Dnpwwo, 2016
//
#ifdef ENABLE_PYTHON

#include "PluginMessages.h"
#include "PluginProtocols.h"
#include "../main/Helper.h"
#include "../main/Logger.h"
#include "Base64.h"
#include "icmp_header.hpp"
#include "ipv4_header.hpp"
#include "../json/json.h"

namespace Plugins {

	CPluginProtocol * CPluginProtocol::Create(std::string sProtocol)
	{
		if (sProtocol == "Line") return (CPluginProtocol*) new CPluginProtocolLine();
		else if (sProtocol == "XML") return (CPluginProtocol*) new CPluginProtocolXML();
		else if (sProtocol == "JSON") return (CPluginProtocol*) new CPluginProtocolJSON();
		else if ((sProtocol == "HTTP") || (sProtocol == "HTTPS"))
		{
			CPluginProtocolHTTP*	pProtocol = new CPluginProtocolHTTP(sProtocol == "HTTPS");
			return (CPluginProtocol*)pProtocol;
		}
		else if (sProtocol == "ICMP") return (CPluginProtocol*) new CPluginProtocolICMP();
		else if ((sProtocol == "MQTT") || (sProtocol == "MQTTS"))
		{
			CPluginProtocolMQTT*	pProtocol = new CPluginProtocolMQTT(sProtocol == "MQTTS");
			return (CPluginProtocol*)pProtocol;
		}
		else if ((sProtocol == "WS") || (sProtocol == "WSS"))
		{
			CPluginProtocolWS*	pProtocol = new CPluginProtocolWS(sProtocol == "WSS");
			return (CPluginProtocol*)pProtocol;
		}
		else return new CPluginProtocol();
	}

	void CPluginProtocol::ProcessInbound(const ReadEvent* Message)
	{
		// Raw protocol is to just always dispatch data to plugin without interpretation
		Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, Message->m_Buffer));
	}

	std::vector<byte> CPluginProtocol::ProcessOutbound(const WriteDirective* WriteMessage)
	{
		std::vector<byte>	retVal;

		// Handle Bytes objects
		if ((((PyObject*)WriteMessage->m_Object)->ob_type->tp_flags & (Py_TPFLAGS_BYTES_SUBCLASS)) != 0)
		{
			const char*	pData = PyBytes_AsString(WriteMessage->m_Object);
			int			iSize = PyBytes_Size(WriteMessage->m_Object);
			retVal.reserve((size_t)iSize);
			retVal.assign(pData, pData + iSize);
		}
		// Handle ByteArray objects
		else if ((((PyObject*)WriteMessage->m_Object)->ob_type->tp_name == std::string("bytearray")))
		{
			size_t	len = PyByteArray_Size(WriteMessage->m_Object);
			char*	data = PyByteArray_AsString(WriteMessage->m_Object);
			retVal.reserve(len);
			retVal.assign((const byte*)data, (const byte*)data + len);
		}
		// Handle String objects
		else if ((((PyObject*)WriteMessage->m_Object)->ob_type->tp_flags & (Py_TPFLAGS_UNICODE_SUBCLASS)) != 0)
		{
			std::string	sData = PyUnicode_AsUTF8(WriteMessage->m_Object);
			retVal.reserve((size_t)sData.length());
			retVal.assign((const byte*)sData.c_str(), (const byte*)sData.c_str() + sData.length());
		}
		else
			_log.Log(LOG_ERROR, "(%s) Send request Python object parameter was not of type Unicode or Byte, ignored.", __func__);

		return retVal;
	}

	void CPluginProtocol::Flush(CPlugin* pPlugin, CConnection* pConnection)
	{
		if (m_sRetainedData.size())
		{
			// Forced buffer clear, make sure the plugin gets a look at the data in case it wants it
			pPlugin->MessagePlugin(new onMessageCallback(pPlugin, pConnection, m_sRetainedData));
			m_sRetainedData.clear();
		}
	}

	void CPluginProtocolLine::ProcessInbound(const ReadEvent* Message)
	{
		//
		//	Handles the cases where a read contains a partial message or multiple messages
		//
		std::vector<byte>	vData = m_sRetainedData;										// if there was some data left over from last time add it back in
		vData.insert(vData.end(), Message->m_Buffer.begin(), Message->m_Buffer.end());		// add the new data

		std::string		sData(vData.begin(), vData.end());
		int iPos = sData.find_first_of('\r');		//  Look for message terminator
		while (iPos != std::string::npos)
		{
			Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, std::vector<byte>(&sData[0], &sData[iPos])));

			if (sData[iPos + 1] == '\n') iPos++;				//  Handle \r\n
			sData = sData.substr(iPos + 1);
			iPos = sData.find_first_of('\r');
		}

		m_sRetainedData.assign(sData.c_str(), sData.c_str() + sData.length()); // retain any residual for next time
	}

	static void AddBytesToDict(PyObject* pDict, const char* key, const std::string &value)
	{
		PyObjPtr	nrKey = PyUnicode_FromString(key);
		PyObjPtr	nrObj = Py_BuildValue("y#", value.c_str(), value.length());
		if (PyDict_SetItem(pDict, nrKey, nrObj) == -1)
			_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", __func__, key, value.c_str());
	}

	static void AddStringToDict(PyObject* pDict, const char* key, const std::string &value)
	{
		PyObjPtr	nrKey = PyUnicode_FromString(key);
		PyObjPtr	nrObj = Py_BuildValue("s#", value.c_str(), value.length());
		if (PyDict_SetItem(pDict, nrKey, nrObj) == -1)
			_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", __func__, key, value.c_str());
	}

	static void AddIntToDict(PyObject* pDict, const char* key, const int value)
	{
		PyObjPtr	nrKey = PyUnicode_FromString(key);
		PyObjPtr	nrObj = Py_BuildValue("i", value);
		if (PyDict_SetItem(pDict, nrKey, nrObj) == -1)
			_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%d' to dictionary.", __func__, key, value);
	}

	PyObject*	CPluginProtocolJSON::JSONtoPython(Json::Value*	pJSON)
	{
		PyObject*	pRetVal = NULL;

		if (pJSON->isArray())
		{
			pRetVal = PyList_New(pJSON->size());
			Py_ssize_t	Index = 0;
			for (Json::ValueIterator it = pJSON->begin(); it != pJSON->end(); ++it)
			{
				Json::ValueIterator::reference	pRef = *it;
				// NOTE: PyList_SetItem does NOT increment the reference count
				if (it->isArray() || it->isObject())
				{
					PyObject*	nrObj = JSONtoPython(&pRef);
					if (!nrObj || (PyList_SetItem(pRetVal, Index++, nrObj) == -1))
						_log.Log(LOG_ERROR, "(%s) failed to add item '%zd', to list for object.", __func__, Index - 1);
				}
				else if (it->isUInt())
				{
					PyObject*	nrObj = Py_BuildValue("I", it->asUInt());
					if (!nrObj || (PyList_SetItem(pRetVal, Index++, nrObj) == -1))
						_log.Log(LOG_ERROR, "(%s) failed to add item '%zd', to list for unsigned integer.", __func__, Index - 1);
				}
				else if (it->isInt())
				{
					PyObject*	nrObj = Py_BuildValue("i", it->asInt());
					if (!nrObj || (PyList_SetItem(pRetVal, Index++, nrObj) == -1))
						_log.Log(LOG_ERROR, "(%s) failed to add item '%zd', to list for integer.", __func__, Index - 1);
				}
				else if (it->isDouble())
				{
					PyObject*	nrObj = Py_BuildValue("d", it->asDouble());
					if (!nrObj || (PyList_SetItem(pRetVal, Index++, nrObj) == -1))
						_log.Log(LOG_ERROR, "(%s) failed to add item '%zd', to list for double.", __func__, Index - 1);
				}
				else if (it->isConvertibleTo(Json::stringValue))
				{
					std::string	sString = it->asString();
					PyObject*	nrObj = Py_BuildValue("s#", sString.c_str(), sString.length());
					if (!nrObj || (PyList_SetItem(pRetVal, Index++, nrObj) == -1))
						_log.Log(LOG_ERROR, "(%s) failed to add item '%zd', to list for string.", __func__, Index - 1);
				}
				else
					_log.Log(LOG_ERROR, "(%s) failed to process entry.", __func__);
			}
		}
		else if (pJSON->isObject())
		{
			pRetVal = PyDict_New();
			for (Json::ValueIterator it = pJSON->begin(); it != pJSON->end(); ++it)
			{
				Json::ValueIterator::reference	pRef = *it;
				std::string	KeyName = it.name();
				PyObjPtr	nrKey = PyUnicode_FromString(KeyName.c_str());

				if (it->isArray() || it->isObject())
				{
					PyObjPtr	pObj = JSONtoPython(&pRef);
					if (!pObj || (PyDict_SetItem(pRetVal, nrKey, pObj) == -1))
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s', to dictionary for object.", __func__, KeyName.c_str());
				}
				else if (it->isUInt())
				{
					PyObjPtr	nrObj = Py_BuildValue("I", it->asUInt());
					if (PyDict_SetItem(pRetVal, nrKey, nrObj) == -1)
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s', unsigned integer value '%d' to dictionary.", __func__, KeyName.c_str(), it->asUInt());
				}
				else if (it->isInt())
				{
					PyObjPtr	nrObj = Py_BuildValue("i", it->asInt());
					if (PyDict_SetItem(pRetVal, nrKey, nrObj) == -1)
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s', integer value '%d' to dictionary.", __func__, KeyName.c_str(), it->asInt());
				}
				else if (it->isBool())
				{
					//PyObject* pObj = Py_BuildValue("N", PyBool_FromLong(it->asBool()));	// "N" does not increment the ref count
					PyObjPtr nrObj = Py_BuildValue("O", PyBool_FromLong(it->asBool()));
					if (PyDict_SetItem(pRetVal, nrKey, nrObj) == -1)
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s', boolean value '%d' to dictionary.", __func__, KeyName.c_str(), it->asBool());
				}
				else if (it->isDouble())
				{
					PyObjPtr	nrObj = Py_BuildValue("d", it->asDouble());
					if (PyDict_SetItem(pRetVal, nrKey, nrObj) == -1)
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s', double value '%lf' to dictionary.", __func__, KeyName.c_str(), it->asDouble());
				}
				else if (it->isConvertibleTo(Json::stringValue))
				{
					AddStringToDict(pRetVal, KeyName.c_str(), it->asString());
				}
				else _log.Log(LOG_ERROR, "(%s) failed to process entry for '%s'.", __func__, KeyName.c_str());
			}
		}
		return pRetVal;
	}

	PyObject* CPluginProtocolJSON::JSONtoPython(std::string	sData)
	{
		Json::Reader	jReader;
		Json::Value		root;
		PyObject*		pRetVal = NULL;

		bool bRet = jReader.parse(sData, root);
		if ((!bRet) || (!root.isObject()))
		{
			_log.Log(LOG_ERROR, "JSON Protocol: Parse Error on '%s'", sData.c_str());
			pRetVal = PyDict_New();
		}
		else
		{
			pRetVal = JSONtoPython(&root);
		}

		return pRetVal;
	}

	std::string CPluginProtocolJSON::PythontoJSON(PyObject* pObject)
	{
		std::string	sJson;

		if (PyUnicode_Check(pObject))
		{
			sJson += '"' + std::string(PyUnicode_AsUTF8(pObject)) + '"';
		}
		else if (pObject->ob_type->tp_name == std::string("bool"))
		{
			sJson += (PyObject_IsTrue(pObject) ? "true" : "false");
		}
		else if (PyLong_Check(pObject))
		{
			sJson += std::to_string(PyLong_AsLong(pObject));
		}
		else if (PyBytes_Check(pObject))
		{
			sJson += '"' + std::string(PyBytes_AsString(pObject)) + '"';
		}
		else if (pObject->ob_type->tp_name == std::string("bytearray"))
		{
			sJson += '"' + std::string(PyByteArray_AsString(pObject)) + '"';
		}
		else if (pObject->ob_type->tp_name == std::string("float"))
		{
			sJson += std::to_string(PyFloat_AsDouble(pObject));
		}
		else if (PyDict_Check(pObject))
		{
			sJson += "{ ";
			PyObject* key, * value;
			Py_ssize_t pos = 0;
			while (PyDict_Next(pObject, &pos, &key, &value))
			{
				PyObjPtr nrKeyString = PyObject_Str(key);
				sJson += '"' + std::string(PyUnicode_AsUTF8(nrKeyString)) + "\" :" + PythontoJSON(value) + ',';
			}
			sJson[sJson.length()-1] = '}';
		}
		else if (PyList_Check(pObject))
		{
			sJson += "[ ";
			for (Py_ssize_t i = 0; i < PyList_Size(pObject); i++)
			{
				sJson += PythontoJSON(PyList_GetItem(pObject, i)) + ',';
			}
			sJson[sJson.length()-1] = ']';
		}
		else if (PyTuple_Check(pObject))
		{
			sJson += "[ ";
			for (Py_ssize_t i = 0; i < PyTuple_Size(pObject); i++)
			{
				sJson += PythontoJSON(PyTuple_GetItem(pObject, i)) + ',';
			}
			sJson[sJson.length() - 1] = ']';
		}

		return sJson;
	}

	void CPluginProtocolJSON::ProcessInbound(const ReadEvent* Message)
	{
		//
		//	Handles the cases where a read contains a partial message or multiple messages
		//
		std::vector<byte>	vData = m_sRetainedData;										// if there was some data left over from last time add it back in
		vData.insert(vData.end(), Message->m_Buffer.begin(), Message->m_Buffer.end());		// add the new data

		std::string		sData(vData.begin(), vData.end());
		int iPos = 1;
		while (iPos) {
			Json::Reader	jReader;
			Json::Value		root;
			iPos = sData.find("}{", 0) + 1;		//  Look for message separater in case there is more than one
			if (!iPos) // no, just one or part of one
			{
				if ((sData.substr(sData.length() - 1, 1) == "}") &&
					(std::count(sData.begin(), sData.end(), '{') == std::count(sData.begin(), sData.end(), '}'))) // whole message so queue the whole buffer
				{
					bool bRet = jReader.parse(sData, root);
					if ((!bRet) || (!root.isObject()))
					{
						_log.Log(LOG_ERROR, "JSON Protocol: Parse Error on '%s'", sData.c_str());
						Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, sData));
					}
					else
					{
						PyObject*	pMessage = JSONtoPython(&root);
						Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, pMessage));
					}
					sData.clear();
				}
			}
			else  // more than one message so queue the first one
			{
				std::string sMessage = sData.substr(0, iPos);
				sData = sData.substr(iPos);
				bool bRet = jReader.parse(sMessage, root);
				if ((!bRet) || (!root.isObject()))
				{
					_log.Log(LOG_ERROR, "JSON Protocol: Parse Error on '%s'", sData.c_str());
					Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, sMessage));
				}
				else
				{
					PyObject*	pMessage = JSONtoPython(&root);
					Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, pMessage));
				}
			}
		}

		m_sRetainedData.assign(sData.c_str(), sData.c_str() + sData.length()); // retain any residual for next time
	}

	void CPluginProtocolXML::ProcessInbound(const ReadEvent* Message)
	{
		//
		//	Only returns whole XML messages. Does not handle <tag /> as the top level tag
		//	Handles the cases where a read contains a partial message or multiple messages
		//
		std::vector<byte>	vData = m_sRetainedData;										// if there was some data left over from last time add it back in
		vData.insert(vData.end(), Message->m_Buffer.begin(), Message->m_Buffer.end());		// add the new data

		std::string		sData(vData.begin(), vData.end());
		try
		{
			while (true)
			{
				//
				//	Find the top level tag name if it is not set
				//
				if (!m_Tag.length())
				{
					if (sData.find("<?xml") != std::string::npos)	// step over '<?xml version="1.0" encoding="utf-8"?>' if present
					{
						int iEnd = sData.find("?>");
						sData = sData.substr(iEnd + 2);
					}

					int iStart = sData.find_first_of('<');
					if (iStart == std::string::npos)
					{
						// start of a tag not found so discard
						m_sRetainedData.clear();
						break;
					}
					if (iStart) sData = sData.substr(iStart);		// remove any leading data
					int iEnd = sData.find_first_of(" >");
					if (iEnd != std::string::npos)
					{
						m_Tag = sData.substr(1, (iEnd - 1));
					}
				}

				int	iPos = sData.find("</" + m_Tag + ">");
				if (iPos != std::string::npos)
				{
					int iEnd = iPos + m_Tag.length() + 3;
					Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, sData.substr(0, iEnd)));

					if (iEnd == sData.length())
					{
						sData.clear();
					}
					else
					{
						sData = sData.substr(++iEnd);
					}
					m_Tag = "";
				}
				else
					break;
			}
		}
		catch (std::exception const &exc)
		{
			_log.Log(LOG_ERROR, "(CPluginProtocolXML::ProcessInbound) Unexpected exception thrown '%s', Data '%s'.", exc.what(), sData.c_str());
		}

		m_sRetainedData.assign(sData.c_str(), sData.c_str() + sData.length()); // retain any residual for next time
	}

	void CPluginProtocolHTTP::ExtractHeaders(std::string * pData)
	{
		// Remove headers
		if (m_Headers)
		{
			Py_XDECREF(m_Headers);
		}
		m_Headers = (PyObject*)PyDict_New();

		*pData = pData->substr(pData->find_first_of('\n') + 1);
		while (pData->length() && ((*pData)[0] != '\r'))
		{
			std::string		sHeaderLine = pData->substr(0, pData->find_first_of('\r'));
			std::string		sHeaderName = pData->substr(0, sHeaderLine.find_first_of(':'));
			std::string		uHeaderName = sHeaderName;
			stdupper(uHeaderName);
			std::string		sHeaderText = sHeaderLine.substr(sHeaderName.length() + 2);
			if (uHeaderName == "CONTENT-LENGTH")
			{
				m_ContentLength = atoi(sHeaderText.c_str());
			}
			else if (uHeaderName == "TRANSFER-ENCODING")
			{
				std::string		uHeaderText = sHeaderText;
				stdupper(uHeaderText);
				if (uHeaderText == "CHUNKED")
					m_Chunked = true;
			}
			else if ((uHeaderName == "CONTENT-TYPE") && (sHeaderText.find("application/json") != std::string::npos))
			{
				m_JSON = true;
			}
			PyObjPtr	nrObj = Py_BuildValue("s", sHeaderText.c_str());
			PyObject*	brPrevObj = PyDict_GetItemString((PyObject*)m_Headers, sHeaderName.c_str());
			// Encode multi headers in a list
			if (brPrevObj != NULL) {
				PyObject* pListObj = brPrevObj;
				// First duplicate? Create a list and add previous value
				if (!PyList_Check(pListObj))
				{
					pListObj = PyList_New(1);
					if (!pListObj)
					{
						_log.Log(LOG_ERROR, "(%s) failed to create list to handle duplicate header. Name '%s'.", __func__, sHeaderName.c_str());
						return;
					}
					PyList_SetItem(pListObj, 0, brPrevObj);
					Py_INCREF(brPrevObj);
					PyDict_SetItemString((PyObject*)m_Headers, sHeaderName.c_str(), pListObj);
					Py_XDECREF(pListObj);
				}
				// Append new value to the list
				if (PyList_Append(pListObj, nrObj) == -1) {
					_log.Log(LOG_ERROR, "(%s) failed to append to list key '%s', value '%s' to headers.", __func__, sHeaderName.c_str(), sHeaderText.c_str());
				}
			}
			else if (PyDict_SetItemString((PyObject*)m_Headers, sHeaderName.c_str(), nrObj) == -1) {
				_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to headers.", __func__, sHeaderName.c_str(), sHeaderText.c_str());
			}
			*pData = pData->substr(pData->find_first_of('\n') + 1);
		}
	}

	void CPluginProtocolHTTP::Flush(CPlugin* pPlugin, CConnection* pConnection)
	{
		if (m_sRetainedData.size())
		{
			// Forced buffer clear, make sure the plugin gets a look at the data in case it wants it
			ProcessInbound(new ReadEvent(pPlugin, pConnection, 0, NULL));
			m_sRetainedData.clear();
		}
	}

	void CPluginProtocolHTTP::ProcessInbound(const ReadEvent* Message)
	{
		// There won't be a buffer if the connection closed
		if (Message->m_Buffer.size())
		{
			m_sRetainedData.insert(m_sRetainedData.end(), Message->m_Buffer.begin(), Message->m_Buffer.end());
		}

		// HTML is non binary so use strings
		std::string		sData(m_sRetainedData.begin(), m_sRetainedData.end());

		m_ContentLength = -1;
		m_Chunked = false;
		m_RemainingChunk = 0;

		//
		//	Process server responses
		//
		if (sData.substr(0, 4) == "HTTP")
		{
			// HTTP/1.0 404 Not Found
			// Content-Type: text/html; charset=UTF-8
			// Content-Length: 1570
			// Date: Thu, 05 Jan 2017 05:50:33 GMT
			//
			// <!DOCTYPE html>
			// <html lang=en>
			//   <meta charset=utf-8>
			//   <meta name=viewport...

			// HTTP/1.1 200 OK
			// Content-Type: text/html; charset=UTF-8
			// Transfer-Encoding: chunked
			// Date: Thu, 05 Jan 2017 05:50:33 GMT
			//
			// 40d
			// <!DOCTYPE html>
			// <html lang=en>
			//   <meta charset=utf-8>
			// ...
			// </html>
			// 0

			// Process response header (HTTP/1.1 200 OK)
			std::string		sFirstLine = sData.substr(0, sData.find_first_of('\r'));
			sFirstLine = sFirstLine.substr(sFirstLine.find_first_of(' ') + 1);
			m_Status = sFirstLine.substr(0, sFirstLine.find_first_of(' '));

			ExtractHeaders(&sData);

			// not enough data arrived to complete header processing
			if (!sData.length())
			{
				return;
			}

			sData = sData.substr(sData.find_first_of('\n') + 1);		// skip over 2nd new line char

			// Process the message body
			if (m_Status.length())
			{
				if (!m_Chunked)
				{
					// If full message then return it
					if ((m_ContentLength == sData.length()) || (!Message->m_Buffer.size()))
					{
						PyDictObject*	pDataDict = (PyDictObject*)PyDict_New();
						PyObjPtr		nrObj = Py_BuildValue("s", m_Status.c_str());
						if (PyDict_SetItemString((PyObject*)pDataDict, "Status", nrObj) == -1)
							_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", "HTTP", "Status", m_Status.c_str());

						if (m_Headers)
						{
							if (PyDict_SetItemString((PyObject*)pDataDict, "Headers", (PyObject*)m_Headers) == -1)
								_log.Log(LOG_ERROR, "(%s) failed to add key '%s' to dictionary.", "HTTP", "Headers");
							Py_XDECREF((PyObject*)m_Headers);
							m_Headers = NULL;
						}

						if (sData.length())
						{
							// If the payload is JSON then translate it
							if (m_JSON)
							{
								CPluginProtocolJSON	oJSON;
								PyDictObject* pJsonDict = (PyDictObject*)oJSON.JSONtoPython(sData);
								if (PyDict_SetItemString((PyObject*)pDataDict, "Data", (PyObject*)pJsonDict) == -1)
									_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", "HTTP", "Data", sData.c_str());
								Py_XDECREF(pJsonDict);
							}
							else
							{
								PyObjPtr nrBytes = Py_BuildValue("y#", sData.c_str(), sData.length());
								if (PyDict_SetItemString((PyObject*)pDataDict, "Data", nrBytes) == -1)
									_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", "HTTP", "Data", sData.c_str());
							}
						}

						Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, (PyObject*)pDataDict));
						m_sRetainedData.clear();
					}
				}
				else
				{
					// Process available chunks
					std::string		sPayload;
					while (sData.length() && (sData != "\r\n"))
					{
						if (!m_RemainingChunk)	// Not processing a chunk so we should be at the start of one
						{
							if (sData[0] == '\r')
							{
								sData = sData.substr(sData.find_first_of('\n') + 1);
							}
							std::string		sChunkLine = sData.substr(0, sData.find_first_of('\r'));
							m_RemainingChunk = strtol(sChunkLine.c_str(), NULL, 16);
							sData = sData.substr(sData.find_first_of('\n') + 1);
							if (!m_RemainingChunk)	// last chunk is zero length
							{
								PyObject*	pDataDict = PyDict_New();
								PyObjPtr	nrObj = Py_BuildValue("s", m_Status.c_str());
								if (PyDict_SetItemString(pDataDict, "Status", nrObj) == -1)
									_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", "HTTP", "Status", m_Status.c_str());

								if (m_Headers)
								{
									if (PyDict_SetItemString(pDataDict, "Headers", (PyObject*)m_Headers) == -1)
										_log.Log(LOG_ERROR, "(%s) failed to add key '%s' to dictionary.", "HTTP", "Headers");
									Py_XDECREF((PyObject*)m_Headers);
									m_Headers = NULL;
								}

								if (sPayload.length())
								{
									nrObj = Py_BuildValue("y#", sPayload.c_str(), sPayload.length());
									if (PyDict_SetItemString(pDataDict, "Data", nrObj) == -1)
										_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", "HTTP", "Data", sPayload.c_str());
								}

								Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, pDataDict));
								m_sRetainedData.clear();
								break;
							}
						}

						if (sData.length() <= m_RemainingChunk)		// Read data is just part of a chunk
						{
							break;
						}

						sPayload += sData.substr(0, m_RemainingChunk);
						sData = sData.substr(m_RemainingChunk);
						m_RemainingChunk = 0;
					}
				}
			}
		}

		//
		//	Process client requests
		//
		else
		{
			// GET / HTTP / 1.1\r\n
			// Host: 127.0.0.1 : 9090\r\n
			// User - Agent: Mozilla / 5.0 (Windows NT 10.0; WOW64; rv:53.0) Gecko / 20100101 Firefox / 53.0\r\n
			// Accept: text / html, application / xhtml + xml, application / xml; q = 0.9, */*;q=0.8\r\n
			std::string		sFirstLine = sData.substr(0, sData.find_first_of('\r'));
			sFirstLine = sFirstLine.substr(0, sFirstLine.find_last_of(' '));

			ExtractHeaders(&sData);
			if (sData.substr(0,2) == "\r\n")
			{
				std::string		sPayload = sData.substr(2);
				// No payload || we have the payload || the connection has closed
				if ((m_ContentLength == -1) || (m_ContentLength == sPayload.length()) || !Message->m_Buffer.size())
				{
					PyObject* DataDict = PyDict_New();
					std::string		sVerb = sFirstLine.substr(0, sFirstLine.find_first_of(' '));
					PyObject*	nrObj = Py_BuildValue("s", sVerb.c_str());
					if (PyDict_SetItemString(DataDict, "Verb", nrObj) == -1)
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", "HTTP", "Verb", sVerb.c_str());
					Py_XDECREF(nrObj);

					std::string		sURL = sFirstLine.substr(sVerb.length() + 1, sFirstLine.find_first_of(' ', sVerb.length() + 1));
					nrObj = Py_BuildValue("s", sURL.c_str());
					if (PyDict_SetItemString(DataDict, "URL", nrObj) == -1)
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", "HTTP", "URL", sURL.c_str());
					Py_XDECREF(nrObj);

					if (m_Headers)
					{
						if (PyDict_SetItemString(DataDict, "Headers", (PyObject*)m_Headers) == -1)
							_log.Log(LOG_ERROR, "(%s) failed to add key '%s' to dictionary.", "HTTP", "Headers");
						Py_XDECREF((PyObject*)m_Headers);
						m_Headers = NULL;
					}

					if (sPayload.length())
					{
						nrObj = Py_BuildValue("y#", sPayload.c_str(), sPayload.length());
						if (PyDict_SetItemString(DataDict, "Data", nrObj) == -1)
							_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", "HTTP", "Data", sPayload.c_str());
						Py_XDECREF(nrObj);
					}

					Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, DataDict));
					m_sRetainedData.clear();
				}
			}
		}
	}

	std::vector<byte>	CPluginProtocolHTTP::ProcessOutbound(const WriteDirective* WriteMessage)
	{
		std::vector<byte>	retVal;
		std::string	sHttp;

		// Sanity check input
		if (!WriteMessage->m_Object || !PyDict_Check(WriteMessage->m_Object))
		{
			_log.Log(LOG_ERROR, "(%s) HTTP Send parameter was not a dictionary, ignored. See Python Plugin wiki page for help.", __func__);
			return retVal;
		}

		// Extract potential values.  Failures return NULL, success returns borrowed reference
		PyObject *brVerb = PyDict_GetItemString(WriteMessage->m_Object, "Verb");
		PyObject *brStatus = PyDict_GetItemString(WriteMessage->m_Object, "Status");
		PyObject *brChunk = PyDict_GetItemString(WriteMessage->m_Object, "Chunk");
		PyObject *brHeaders = PyDict_GetItemString(WriteMessage->m_Object, "Headers");
		PyObject *brData = PyDict_GetItemString(WriteMessage->m_Object, "Data");

		if (PyErr_Occurred())
		{
			_log.Log(LOG_NORM, "Python error");
			PyErr_Clear();
		}

		//
		//	Assume Request if 'Verb' specified
		//
		if (brVerb)
		{
			// GET /path/file.html HTTP/1.1
			// Connection: "keep-alive"
			// Accept: "text/html"
			//

			// POST /path/test.cgi HTTP/1.1
			// From: info@domoticz.com
			// User-Agent: Domoticz/1.0
			// Content-Type : application/x-www-form-urlencoded
			// Content-Length : 32
			//
			// param1=value&param2=other+value

			if (!PyUnicode_Check(brVerb))
			{
				_log.Log(LOG_ERROR, "(%s) HTTP 'Verb' dictionary entry not a string, ignored. See Python Plugin wiki page for help.", __func__);
				return retVal;
			}
			sHttp = PyUnicode_AsUTF8(brVerb);
			stdupper(sHttp);
			sHttp += " ";

			PyObject *brURL = PyDict_GetItemString(WriteMessage->m_Object, "URL");
			std::string	sHttpURL = "/";
			if (brURL && PyUnicode_Check(brURL))
			{
				sHttpURL = PyUnicode_AsUTF8(brURL);
			}
			sHttp += sHttpURL;
			sHttp += " HTTP/1.1\r\n";

			// If username &/or password specified then add a basic auth header (if one was not supplied)
			PyObject *brHead = NULL;
			if (brHeaders) brHead = PyDict_GetItemString(brHeaders, "Authorization:Basic");
			if (!brHead)
			{
				std::string		User;
				std::string		Pass;
				PyObject *		pModule = (PyObject*)WriteMessage->m_pPlugin->PythonModule();
				if (PyObject_HasAttrString(pModule, "Parameters"))
				{
					// TODO: The username password needs to come from the plugin->m_Interface->Configuration
					PyObjPtr		pDict = PyObject_GetAttrString(pModule, "Parameters");
					if (pDict)
					{
						PyObject* brUser = PyDict_GetItemString(pDict, "Username");
						if (brUser) User = PyUnicode_AsUTF8(brUser);
						PyObject* brPass = PyDict_GetItemString(pDict, "Password");
						if (brPass) Pass = PyUnicode_AsUTF8(brPass);
						Py_XDECREF(pDict);
					}
				}
				if (User.length() > 0 || Pass.length() > 0)
				{
					std::string auth;
					if (User.length() > 0)
					{
						auth += User;
					}
					auth += ":";
					if (Pass.length() > 0)
					{
						auth += Pass;
					}
					std::string encodedAuth = base64_encode(auth);
					sHttp += "Authorization:Basic " + encodedAuth + "\r\n";
				}
			}

			// Add Server header if it is not supplied
			if (brHeaders) brHead = PyDict_GetItemString(brHeaders, "User-Agent");
			if (!brHead)
			{
				sHttp += "User-Agent: Domoticz/1.0\r\n";
			}
		}
		//
		//	Assume Response if 'Status' specified
		//
		else if (brStatus)
		{
			//	HTTP/1.1 200 OK
			//	Date: Mon, 27 Jul 2009 12:28:53 GMT
			//	Server: Apache/2.2.14 (Win32)
			//	Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
			//	Content-Length: 88
			//	Content-Type: text/html
			//	Connection: Closed
			//
			//	<html>
			//	<body>
			//	<h1>Hello, World!</h1>
			//	</body>
			//	</html>

			if (!PyUnicode_Check(brStatus))
			{
				_log.Log(LOG_ERROR, "(%s) HTTP 'Status' dictionary entry was not a string, ignored. See Python Plugin wiki page for help.", __func__);
				return retVal;
			}

			sHttp = "HTTP/1.1 ";
			sHttp += PyUnicode_AsUTF8(brStatus);
			sHttp += "\r\n";

			// Add Date header if it is not supplied
			PyObject *brHead = NULL;
			if (brHeaders) brHead = PyDict_GetItemString(brHeaders, "Date");
			if (!brHead)
			{
				char szDate[100];
				time_t rawtime;
				struct tm *info;
				time(&rawtime);
				info = gmtime(&rawtime);
				if (0 < strftime(szDate, sizeof(szDate), "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", info))	sHttp += szDate;
			}

			// Add Server header if it is not supplied
			brHead = NULL;
			if (brHeaders) brHead = PyDict_GetItemString(brHeaders, "Server");
			if (!brHead)
			{
				sHttp += "Server: Domoticz/1.0\r\n";
			}
		}
		//
		//	Only other valid options is a chunk response
		//
		else if (!brChunk)
		{
			_log.Log(LOG_ERROR, "(%s) HTTP unable to determine send type. 'Verb', 'Status' or 'Chunk' dictionary entries not found, ignored. See Python Plugin wiki page for help.", __func__);
			return retVal;
		}

		// Handle headers for normal Requests & Responses
		if (brVerb || brStatus)
		{
			// Did we get headers to send?
			if (brHeaders)
			{
				if (PyDict_Check(brHeaders))
				{
					PyObject *key, *value;
					Py_ssize_t pos = 0;
					while (PyDict_Next(brHeaders, &pos, &key, &value))
					{
						std::string	sKey = PyUnicode_AsUTF8(key);
						if (PyUnicode_Check(value))
						{
							std::string	sValue = PyUnicode_AsUTF8(value);
							sHttp += sKey + ": " + sValue + "\r\n";
						}
						else if (PyBytes_Check(value))
						{
							const char* pBytes = PyBytes_AsString(value);
							sHttp += sKey + ": " + pBytes + "\r\n";
						}
						else if (value->ob_type->tp_name == std::string("bytearray"))
						{
							const char* pByteArray = PyByteArray_AsString(value);
							sHttp += sKey + ": " + pByteArray + "\r\n";
						}
						else if (PyList_Check(value))
						{
							PyObject* iterator = PyObject_GetIter(value);
							PyObject* item;
							while (item = PyIter_Next(iterator)) {
								if (PyUnicode_Check(item))
								{
									std::string	sValue = PyUnicode_AsUTF8(item);
									sHttp += sKey + ": " + sValue + "\r\n";
								}
								else if (PyBytes_Check(item))
								{
									const char* pBytes = PyBytes_AsString(item);
									sHttp += sKey + ": " + pBytes + "\r\n";
								}
								else if (item->ob_type->tp_name == std::string("bytearray"))
								{
									const char* pByteArray = PyByteArray_AsString(item);
									sHttp += sKey + ": " + pByteArray + "\r\n";
								}
								Py_XDECREF(item);
							}

							Py_XDECREF(iterator);
						}
					}
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) HTTP 'Headers' parameter was not a dictionary, ignored.", __func__);
				}
			}

			// Add Content-Length header if it is required but not supplied
			PyObject *brLength = NULL;
			if (brHeaders)
				brLength = PyDict_GetItemString(brHeaders, "Content-Length");
			if (!brLength && brData && !brChunk)
			{
				Py_ssize_t iLength = 0;
				if (PyUnicode_Check(brData))
					iLength = PyUnicode_GetLength(brData);
				else if (brData->ob_type->tp_name == std::string("bytearray"))
					iLength = PyByteArray_Size(brData);
				else if (PyBytes_Check(brData))
					iLength = PyBytes_Size(brData);
				sHttp += "Content-Length: " + std::to_string(iLength) + "\r\n";
			}

			// Add Transfer-Encoding header if required but not supplied
			if (brChunk)
			{
				PyObject *brHead = NULL;
				if (brHeaders) brHead = PyDict_GetItemString(brHeaders, "Transfer-Encoding");
				if (!brHead)
				{
					sHttp += "Transfer-Encoding: chunked\r\n";
				}
			}

			// Terminate preamble
			sHttp += "\r\n";
		}
		
		// Chunks require hex encoded chunk length instead of normal response
		if (brChunk)
		{
			long	lChunkLength = 0;
			if (brData)
			{
				if (PyUnicode_Check(brData))
					lChunkLength = PyUnicode_GetLength(brData);
				else if (brData->ob_type->tp_name == std::string("bytearray"))
					lChunkLength = PyByteArray_Size(brData);
				else if (PyBytes_Check(brData))
					lChunkLength = PyBytes_Size(brData);
			}
			std::stringstream stream;
			stream << std::hex << lChunkLength;
			sHttp += std::string(stream.str());
			sHttp += "\r\n";
		}

		// Append data if supplied (for POST) or Response
		if (brData && PyUnicode_Check(brData))
		{
			sHttp += PyUnicode_AsUTF8(brData);
			retVal.reserve(sHttp.length()+2);
			retVal.assign(sHttp.c_str(), sHttp.c_str() + sHttp.length());
		}
		else if (brData && (brData->ob_type->tp_name == std::string("bytearray")))
		{
			retVal.reserve(sHttp.length() + PyByteArray_Size(brData)+2);
			retVal.assign(sHttp.c_str(), sHttp.c_str() + sHttp.length());
			const char* pByteArray = PyByteArray_AsString(brData);
			int iStop = PyByteArray_Size(brData);
			for (int i = 0; i < iStop; i++)
			{
				retVal.push_back(pByteArray[i]);
			}
		}
		else if (brData && PyBytes_Check(brData))
		{
			retVal.reserve(sHttp.length() + PyBytes_Size(brData)+2);
			retVal.assign(sHttp.c_str(), sHttp.c_str() + sHttp.length());
			const char* pBytes = PyBytes_AsString(brData);
			int iStop = PyBytes_Size(brData);
			for (int i = 0; i < iStop; i++)
			{
				retVal.push_back(pBytes[i]);
			}
		}
		else
		{
			retVal.reserve(sHttp.length() + 2);
			retVal.assign(sHttp.c_str(), sHttp.c_str() + sHttp.length());
		}

		// Chunks require additional CRLF (hence '+2' on all vector reserves to make sure there is room)
		if (brChunk)
		{
			retVal.push_back('\r');
			retVal.push_back('\n');
		}

		return retVal;
	}

	void CPluginProtocolICMP::ProcessInbound(const ReadEvent * Message)
	{
		PyObject*	nrObj = NULL;
		PyObject*	pDataDict = PyDict_New();
		int			iTotalData = 0;
		int			iDataOffset = 0;

		// Handle response
		if (Message->m_Buffer.size())
		{
			PyObject*	pIPv4Dict = PyDict_New();
			if (pDataDict && pIPv4Dict)
			{
				if (PyDict_SetItemString(pDataDict, "IPv4", (PyObject*)pIPv4Dict) == -1)
					_log.Log(LOG_ERROR, "(%s) failed to add key '%s' to dictionary.", "ICMP", "IPv4");
				else
				{
					Py_XDECREF((PyObject*)pIPv4Dict);

					ipv4_header*	pIPv4 = (ipv4_header*)(&Message->m_Buffer[0]);

					nrObj = Py_BuildValue("s", pIPv4->source_address().to_string().c_str());
					PyDict_SetItemString(pIPv4Dict, "Source", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("s", pIPv4->destination_address().to_string().c_str());
					PyDict_SetItemString(pIPv4Dict, "Destination", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("b", pIPv4->version());
					PyDict_SetItemString(pIPv4Dict, "Version", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("b", pIPv4->protocol());
					PyDict_SetItemString(pIPv4Dict, "Protocol", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("b", pIPv4->type_of_service());
					PyDict_SetItemString(pIPv4Dict, "TypeOfService", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("h", pIPv4->header_length());
					PyDict_SetItemString(pIPv4Dict, "HeaderLength", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("h", pIPv4->total_length());
					PyDict_SetItemString(pIPv4Dict, "TotalLength", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("h", pIPv4->identification());
					PyDict_SetItemString(pIPv4Dict, "Identification", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("h", pIPv4->header_checksum());
					PyDict_SetItemString(pIPv4Dict, "HeaderChecksum", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("i", pIPv4->time_to_live());
					PyDict_SetItemString(pIPv4Dict, "TimeToLive", nrObj);
					Py_XDECREF(nrObj);

					iTotalData = pIPv4->total_length();
					iDataOffset = pIPv4->header_length();
				}
				pIPv4Dict = NULL;
			}

			PyObject*	pIcmpDict = PyDict_New();
			if (pDataDict && pIcmpDict)
			{
				if (PyDict_SetItemString(pDataDict, "ICMP", (PyObject*)pIcmpDict) == -1)
					_log.Log(LOG_ERROR, "(%s) failed to add key '%s' to dictionary.", "ICMP", "ICMP");
				else
				{
					Py_XDECREF((PyObject*)pIcmpDict);

					icmp_header*	pICMP = (icmp_header*)(&Message->m_Buffer[0] + 20);
					if ((pICMP->type() == icmp_header::echo_reply) && (Message->m_ElapsedMs >= 0))
					{
						nrObj = Py_BuildValue("I", Message->m_ElapsedMs);
						PyDict_SetItemString(pDataDict, "ElapsedMs", nrObj);
						Py_XDECREF(nrObj);
					}

					nrObj = Py_BuildValue("b", pICMP->type());
					PyDict_SetItemString(pIcmpDict, "Type", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("b", pICMP->type());
					PyDict_SetItemString(pDataDict, "Status", nrObj);
					Py_XDECREF(nrObj);

					switch (pICMP->type())
					{
					case icmp_header::echo_reply:
						nrObj = Py_BuildValue("s", "echo_reply");
						break;
					case icmp_header::destination_unreachable:
						nrObj = Py_BuildValue("s", "destination_unreachable");
						break;
					case icmp_header::time_exceeded:
						nrObj = Py_BuildValue("s", "time_exceeded");
						break;
					default:
						nrObj = Py_BuildValue("s", "unknown");
					}

					PyDict_SetItemString(pDataDict, "Description", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("b", pICMP->code());
					PyDict_SetItemString(pIcmpDict, "Code", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("h", pICMP->checksum());
					PyDict_SetItemString(pIcmpDict, "Checksum", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("h", pICMP->identifier());
					PyDict_SetItemString(pIcmpDict, "Identifier", nrObj);
					Py_XDECREF(nrObj);

					nrObj = Py_BuildValue("h", pICMP->sequence_number());
					PyDict_SetItemString(pIcmpDict, "SequenceNumber", nrObj);
					Py_XDECREF(nrObj);

					iDataOffset += sizeof(icmp_header);
					if (pICMP->type() == icmp_header::destination_unreachable)
					{
						ipv4_header*	pIPv4 = (ipv4_header*)(pICMP + 1);
						iDataOffset += pIPv4->header_length() + sizeof(icmp_header);
					}
				}
				pIcmpDict = NULL;
			}
		}
		else
		{
			nrObj = Py_BuildValue("b", icmp_header::time_exceeded);
			PyDict_SetItemString(pDataDict, "Status", nrObj);
			Py_XDECREF(nrObj);

			nrObj = Py_BuildValue("s", "time_exceeded");
			PyDict_SetItemString(pDataDict, "Description", nrObj);
			Py_XDECREF(nrObj);
		}

		std::string		sData(Message->m_Buffer.begin(), Message->m_Buffer.end());
		sData = sData.substr(iDataOffset, iTotalData - iDataOffset);
		nrObj = Py_BuildValue("y#", sData.c_str(), sData.length());
		if (PyDict_SetItemString(pDataDict, "Data", nrObj) == -1)
			_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", "ICMP", "Data", sData.c_str());
		Py_XDECREF(nrObj);

		if (pDataDict)
		{
			Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, pDataDict));
		}
	}

#define MQTT_CONNECT       1<<4
#define MQTT_CONNACK       2<<4
#define MQTT_PUBLISH       3<<4
#define MQTT_PUBACK        4<<4
#define MQTT_PUBREC        5<<4
#define MQTT_PUBREL        6<<4
#define MQTT_PUBCOMP       7<<4
#define MQTT_SUBSCRIBE     8<<4
#define MQTT_SUBACK        9<<4
#define MQTT_UNSUBSCRIBE  10<<4
#define MQTT_UNSUBACK     11<<4
#define MQTT_PINGREQ      12<<4
#define MQTT_PINGRESP     13<<4
#define MQTT_DISCONNECT   14<<4

#define MQTT_PROTOCOL	  4

	static void MQTTPushBackNumber(int iNumber, std::vector<byte> &vVector)
	{
		vVector.push_back(iNumber / 256);
		vVector.push_back(iNumber % 256);
	}

	static void MQTTPushBackString(const std::string &sString, std::vector<byte> &vVector)
	{
		vVector.insert(vVector.end(), sString.begin(), sString.end());
	}

	static void MQTTPushBackStringWLen(const std::string &sString, std::vector<byte> &vVector)
	{
		MQTTPushBackNumber(sString.length(), vVector);
		vVector.insert(vVector.end(), sString.begin(), sString.end());
	}

	void CPluginProtocolMQTT::ProcessInbound(const ReadEvent * Message)
	{
		if (m_bErrored)
		{
			_log.Log(LOG_ERROR, "(%s) MQTT protocol errored, discarding additional data.", __func__);
			return;
		}

		byte loop = 0;
		m_sRetainedData.insert(m_sRetainedData.end(), Message->m_Buffer.begin(), Message->m_Buffer.end());

		do {
			std::vector<byte>::iterator it = m_sRetainedData.begin();

			byte		header = *it++;
			byte		bResponseType = header & 0xF0;
			byte		flags = header & 0x0F;
			PyObject*	pMqttDict = PyDict_New();
			PyObject*	pObj = NULL;
			uint16_t	iPacketIdentifier = 0;
			long		iRemainingLength = 0;
			long		multiplier = 1;
			byte 		encodedByte;

			do
			{
				encodedByte = *it++;
				iRemainingLength += (encodedByte & 127) * multiplier;
				multiplier *= 128;
				if (multiplier > 128*128*128)
				{
					_log.Log(LOG_ERROR, "(%s) Malformed Remaining Length.", __func__);
					return;
				}
			} while ((encodedByte & 128) != 0);

			if (iRemainingLength > std::distance(it, m_sRetainedData.end()))
			{
				// Full packet has not arrived, wait for more data
				_log.Debug(DEBUG_NORM, "(%s) Not enough data received (got %ld, expected %ld).", __func__, (long)std::distance(it, m_sRetainedData.end()), iRemainingLength);
				return;
			}

			std::vector<byte>::iterator pktend = it+iRemainingLength;

			switch (bResponseType)
			{
			case MQTT_CONNACK:
			{
				AddStringToDict(pMqttDict, "Verb", std::string("CONNACK"));
				if (flags != 0)
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message flags %u for packet type '%u'", __func__, flags, bResponseType>>4);
					m_bErrored = true;
					break;
				}
				if (iRemainingLength == 2) // check length is correct
				{
					switch (*it++)
					{
					case 0:
						AddStringToDict(pMqttDict, "Description", std::string("Connection Accepted"));
						break;
					case 1:
						AddStringToDict(pMqttDict, "Description", std::string("Connection Refused, unacceptable protocol version"));
						break;
					case 2:
						AddStringToDict(pMqttDict, "Description", std::string("Connection Refused, identifier rejected"));
						break;
					case 3:
						AddStringToDict(pMqttDict, "Description", std::string("Connection Refused, Server unavailable"));
						break;
					case 4:
						AddStringToDict(pMqttDict, "Description", std::string("Connection Refused, bad user name or password"));
						break;
					case 5:
						AddStringToDict(pMqttDict, "Description", std::string("Connection Refused, not authorized"));
						break;
					default:
						AddStringToDict(pMqttDict, "Description", std::string("Unknown status returned"));
						break;
					}
					AddIntToDict(pMqttDict, "Status", *it++);
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message length %ld for packet type '%u'", __func__, iRemainingLength, bResponseType>>4);
					m_bErrored = true;
				}
				break;
			}
			case MQTT_SUBACK:
			{
				AddStringToDict(pMqttDict, "Verb", std::string("SUBACK"));
				iPacketIdentifier = (*it++ << 8) + *it++;
				AddIntToDict(pMqttDict, "PacketIdentifier", iPacketIdentifier);

				if (flags != 0)
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message flags %u for packet type '%u'", __func__, flags, bResponseType>>4);
					m_bErrored = true;
					break;
				}
				if (iRemainingLength >= 3) // check length is acceptable
				{
					PyObject* pResponsesList = PyList_New(0);
					if (PyDict_SetItemString(pMqttDict, "Topics", pResponsesList) == -1)
					{
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s' to dictionary.", __func__, "Topics");
						break;
					}
					Py_XDECREF(pResponsesList);

					while (it != pktend)
					{
						PyObject*	pResponseDict = PyDict_New();
						byte Status = *it++;
						AddIntToDict(pResponseDict, "Status", Status);
						switch (Status)
						{
						case 0x00:
							AddStringToDict(pResponseDict, "Description", std::string("Success - Maximum QoS 0"));
							break;
						case 0x01:
							AddStringToDict(pResponseDict, "Description", std::string("Success - Maximum QoS 1"));
							break;
						case 0x02:
							AddStringToDict(pResponseDict, "Description", std::string("Success - Maximum QoS 2"));
							break;
						case 0x80:
							AddStringToDict(pResponseDict, "Description", std::string("Failure"));
							break;
						default:
							AddStringToDict(pResponseDict, "Description", std::string("Unknown status returned"));
							break;
						}
						PyList_Append(pResponsesList, pResponseDict);
						Py_XDECREF(pResponseDict);
					}
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message length %ld for packet type '%u'", __func__, iRemainingLength, bResponseType>>4);
					m_bErrored = true;
				}
				break;
			}
			case MQTT_PUBACK:
				AddStringToDict(pMqttDict, "Verb", std::string("PUBACK"));
				if (flags != 0)
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message flags %u for packet type '%u'", __func__, flags, bResponseType>>4);
					m_bErrored = true;
					break;
				}
				if (iRemainingLength == 2) // check length is correct
				{
					iPacketIdentifier = (*it++ << 8) + *it++;
					AddIntToDict(pMqttDict, "PacketIdentifier", iPacketIdentifier);
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message length %ld for packet type '%u'", __func__, iRemainingLength, bResponseType>>4);
					m_bErrored = true;
				}
				break;
			case MQTT_PUBREC:
				AddStringToDict(pMqttDict, "Verb", std::string("PUBREC"));
				if (flags != 0)
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message flags %u for packet type '%u'", __func__, flags, bResponseType>>4);
					m_bErrored = true;
					break;
				}
				if (iRemainingLength == 2) // check length is correct
				{
					iPacketIdentifier = (*it++ << 8) + *it++;
					AddIntToDict(pMqttDict, "PacketIdentifier", iPacketIdentifier);
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message length %ld for packet type '%u'", __func__, iRemainingLength, bResponseType>>4);
					m_bErrored = true;
				}
				break;
			case MQTT_PUBCOMP:
				AddStringToDict(pMqttDict, "Verb", std::string("PUBCOMP"));
				if (flags != 0)
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message flags %u for packet type '%u'", __func__, flags, bResponseType>>4);
					m_bErrored = true;
					break;
				}
				if (iRemainingLength == 2) // check length is correct
				{
					iPacketIdentifier = (*it++ << 8) + *it++;
					AddIntToDict(pMqttDict, "PacketIdentifier", iPacketIdentifier);
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message length %ld for packet type '%u'", __func__, iRemainingLength, bResponseType>>4);
					m_bErrored = true;
				}
				break;
			case MQTT_PUBLISH:
			{
				// Fixed Header
				AddStringToDict(pMqttDict, "Verb", std::string("PUBLISH"));
				AddIntToDict(pMqttDict, "DUP", ((flags & 0x08) >> 3));
				long	iQoS = (flags & 0x06) >> 1;
				AddIntToDict(pMqttDict, "QoS", (int) iQoS);
				PyDict_SetItemString(pMqttDict, "Retain", PyBool_FromLong(flags & 0x01));
				// Variable Header
				int		topicLen = (*it++ << 8) + *it++;
				if (topicLen+2+(iQoS?2:0) > iRemainingLength)
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message length %ld for packet type '%u' (iQoS:%ld, topicLen:%d)", __func__, iRemainingLength, bResponseType>>4, iQoS, topicLen);
					m_bErrored = true;
					break;
				}
				std::string	sTopic((char const*)&*it, topicLen);
				AddStringToDict(pMqttDict, "Topic", sTopic);
				it += topicLen;
				if (iQoS)
				{
					iPacketIdentifier = (*it++ << 8) + *it++;
					AddIntToDict(pMqttDict, "PacketIdentifier", iPacketIdentifier);
				}
				// Payload
				const char*	pPayload = (it==pktend)?0:(const char*)&*it;
				std::string	sPayload(pPayload, std::distance(it, pktend));
				AddBytesToDict(pMqttDict, "Payload", sPayload);
				break;
			}
			case MQTT_UNSUBACK:
				AddStringToDict(pMqttDict, "Verb", std::string("UNSUBACK"));
				if (flags != 0)
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message flags %u for packet type '%u'", __func__, flags, bResponseType>>4);
					m_bErrored = true;
					break;
				}
				if (iRemainingLength == 2) // check length is correct
				{
					iPacketIdentifier = (*it++ << 8) + *it++;
					AddIntToDict(pMqttDict, "PacketIdentifier", iPacketIdentifier);
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message length %ld for packet type '%u'", __func__, iRemainingLength, bResponseType>>4);
					m_bErrored = true;
				}
				break;
			case MQTT_PINGRESP:
				AddStringToDict(pMqttDict, "Verb", std::string("PINGRESP"));
				if (flags != 0)
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message flags %u for packet type '%u'", __func__, flags, bResponseType>>4);
					m_bErrored = true;
					break;
				}
				if (iRemainingLength != 0) // check length is correct
				{
					_log.Log(LOG_ERROR, "(%s) MQTT protocol violation: Invalid message length %ld for packet type '%u'", __func__, iRemainingLength, bResponseType>>4);
					m_bErrored = true;
				}
				break;
			default:
				_log.Log(LOG_ERROR, "(%s) MQTT data invalid: packet type '%d' is unknown.", __func__, bResponseType);
				m_bErrored = true;
			}

			if (!m_bErrored) Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, pMqttDict));

			m_sRetainedData.erase(m_sRetainedData.begin(),pktend);
		} while (!m_bErrored && m_sRetainedData.size() > 0);

		if (m_bErrored)
		{
			_log.Log(LOG_ERROR, "(%s) MQTT protocol violation, sending DisconnectedEvent to Connection.", __func__);
			Message->m_pPlugin->MessagePlugin(new DisconnectedEvent(Message->m_pPlugin, Message->m_pConnection));
		}
	}

	std::vector<byte> CPluginProtocolMQTT::ProcessOutbound(const WriteDirective * WriteMessage)
	{
		std::vector<byte>	vVariableHeader;
		std::vector<byte>	vPayload;

		std::vector<byte>	retVal;

		// Sanity check input
		if (!WriteMessage->m_Object || !PyDict_Check(WriteMessage->m_Object))
		{
			_log.Log(LOG_ERROR, "(%s) MQTT Send parameter was not a dictionary, ignored. See Python Plugin wiki page for help.", __func__);
			return retVal;
		}

		// Extract potential values.  Failures return NULL, success returns borrowed reference
		PyObject *brVerb = PyDict_GetItemString(WriteMessage->m_Object, "Verb");
		if (brVerb)
		{
			if (!PyUnicode_Check(brVerb))
			{
				_log.Log(LOG_ERROR, "(%s) MQTT 'Verb' dictionary entry not a string, ignored. See Python Plugin wiki page for help.", __func__);
				return retVal;
			}
			std::string sVerb = PyUnicode_AsUTF8(brVerb);

			if (sVerb == "CONNECT")
			{
				MQTTPushBackStringWLen("MQTT", vVariableHeader);
				vVariableHeader.push_back(MQTT_PROTOCOL);

				byte	bControlFlags = 0;

				// Client Identifier
				PyObject *brID = PyDict_GetItemString(WriteMessage->m_Object, "ID");
				if (brID && PyUnicode_Check(brID))
				{
					MQTTPushBackStringWLen(std::string(PyUnicode_AsUTF8(brID)), vPayload);
				}
				else
					MQTTPushBackStringWLen("Domoticz", vPayload); // TODO: default ID should be more unique, for example "Domoticz_<plugin_name>_<HwID>"

				byte	bCleanSession = 1;
				PyObject*	brCleanSession = PyDict_GetItemString(WriteMessage->m_Object, "CleanSession");
				if (brCleanSession && PyLong_Check(brCleanSession))
				{
					bCleanSession = (byte)PyLong_AsLong(brCleanSession);
				}
				bControlFlags |= (bCleanSession&1)<<1;

				// Will topic
				PyObject*	brTopic = PyDict_GetItemString(WriteMessage->m_Object, "WillTopic");
				if (brTopic && PyUnicode_Check(brTopic))
				{
					MQTTPushBackStringWLen(std::string(PyUnicode_AsUTF8(brTopic)), vPayload);
					bControlFlags |= 4;
				}

				// Will QoS, Retain and Message
				if (bControlFlags & 4)
				{
					PyObject *brQoS = PyDict_GetItemString(WriteMessage->m_Object, "WillQoS");
					if (brQoS && PyLong_Check(brQoS))
					{
						byte bQoS = (byte) PyLong_AsLong(brQoS);
						bControlFlags |= (bQoS&3)<<3; // Set QoS flag
					}

					PyObject *brRetain = PyDict_GetItemString(WriteMessage->m_Object, "WillRetain");
					if (brRetain && PyLong_Check(brRetain))
					{
						byte bRetain = (byte)PyLong_AsLong(brRetain);
						bControlFlags |= (bRetain&1)<<5; // Set retain flag
					}

					std::string sPayload = "";
					PyObject*	brPayload = PyDict_GetItemString(WriteMessage->m_Object, "WillPayload");
					// Support both string and bytes
					//if (pPayload && PyByteArray_Check(pPayload)) // Gives linker error, why?
					if (brPayload && brPayload->ob_type->tp_name == std::string("bytearray"))
					{
						sPayload = std::string(PyByteArray_AsString(brPayload), PyByteArray_Size(brPayload));
					}
					else if (brPayload && PyUnicode_Check(brPayload))
					{
						sPayload = std::string(PyUnicode_AsUTF8(brPayload));
					}
					MQTTPushBackStringWLen(sPayload, vPayload);
				}

				// Username / Password
				std::string		User;
				std::string		Pass;
				PyObject *pModule = (PyObject*)WriteMessage->m_pPlugin->PythonModule();
				PyObject *pDict = PyObject_GetAttrString(pModule, "Parameters");
				if (pDict)
				{
					PyObject *brUser = PyDict_GetItemString(pDict, "Username");
					if (brUser) User = PyUnicode_AsUTF8(brUser);
					PyObject *brPass = PyDict_GetItemString(pDict, "Password");
					if (brPass) Pass = PyUnicode_AsUTF8(brPass);
					Py_XDECREF(pDict);
				}
				if (User.length())
				{
					MQTTPushBackStringWLen(User, vPayload);
					bControlFlags |= 128;
				}

				if (Pass.length())
				{
					MQTTPushBackStringWLen(Pass, vPayload);
					bControlFlags |= 64;
				}

				// Control Flags
				vVariableHeader.push_back(bControlFlags);

				// Keep Alive
				vVariableHeader.push_back(0);
				vVariableHeader.push_back(60);

				retVal.push_back(MQTT_CONNECT);
			}
			else if (sVerb == "PING")
			{
				retVal.push_back(MQTT_PINGREQ);
			}
			else if (sVerb == "SUBSCRIBE")
			{
				// Variable header - Packet Identifier.
				// If supplied then use it otherwise create one
				PyObject *brID = PyDict_GetItemString(WriteMessage->m_Object, "PacketIdentifier");
				long	iPacketIdentifier = 0;
				if (brID && PyLong_Check(brID))
				{
					iPacketIdentifier = PyLong_AsLong(brID);
				}
				else iPacketIdentifier = m_PacketID++;
				MQTTPushBackNumber((int)iPacketIdentifier, vVariableHeader);

				// Payload is list of topics and QoS numbers
				PyObject *brTopicList = PyDict_GetItemString(WriteMessage->m_Object, "Topics");
				if (!brTopicList || !PyList_Check(brTopicList))
				{
					_log.Log(LOG_ERROR, "(%s) MQTT Subscribe: No 'Topics' list present, nothing to subscribe to. See Python Plugin wiki page for help.", __func__);
					return retVal;
				}
				for (Py_ssize_t i = 0; i < PyList_Size(brTopicList); i++)
				{
					PyObject*	pTopicDict = PyList_GetItem(brTopicList, i);
					if (!pTopicDict || !PyDict_Check(pTopicDict))
					{
						_log.Log(LOG_ERROR, "(%s) MQTT Subscribe: Topics list entry is not a dictionary (Topic, QoS), nothing to subscribe to. See Python Plugin wiki page for help.", __func__);
						return retVal;
					}
					PyObject*	brTopic = PyDict_GetItemString(pTopicDict, "Topic");
					if (brTopic && PyUnicode_Check(brTopic))
					{
						MQTTPushBackStringWLen(std::string(PyUnicode_AsUTF8(brTopic)), vPayload);
						PyObject*	brQoS = PyDict_GetItemString(pTopicDict, "QoS");
						if (brQoS && PyLong_Check(brQoS))
						{
							vPayload.push_back((byte)PyLong_AsLong(brQoS));
						}
						else vPayload.push_back(0);
					}
					else
					{
						_log.Log(LOG_ERROR, "(%s) MQTT Subscribe: 'Topic' not in dictionary (Topic, QoS), nothing to subscribe to, skipping. See Python Plugin wiki page for help.", __func__);
					}
				}
				retVal.push_back(MQTT_SUBSCRIBE | 0x02);	// Add mandatory reserved flags
			}
			else if (sVerb == "UNSUBSCRIBE")
			{
				// Variable Header
				PyObject *brID = PyDict_GetItemString(WriteMessage->m_Object, "PacketIdentifier");
				long	iPacketIdentifier = 0;
				if (brID && PyLong_Check(brID))
				{
					iPacketIdentifier = PyLong_AsLong(brID);
				}
				else iPacketIdentifier = m_PacketID++;
				MQTTPushBackNumber((int)iPacketIdentifier, vVariableHeader);

				// Payload is a Python list of topics
				PyObject *brTopicList = PyDict_GetItemString(WriteMessage->m_Object, "Topics");
				if (!brTopicList || !PyList_Check(brTopicList))
				{
					_log.Log(LOG_ERROR, "(%s) MQTT Subscribe: No 'Topics' list present, nothing to unsubscribe from. See Python Plugin wiki page for help.", __func__);
					return retVal;
				}
				for (Py_ssize_t i = 0; i < PyList_Size(brTopicList); i++)
				{
					PyObject*	pTopic = PyList_GetItem(brTopicList, i);
					if (pTopic && PyUnicode_Check(pTopic))
					{
						MQTTPushBackStringWLen(std::string(PyUnicode_AsUTF8(pTopic)), vPayload);
					}
				}

				retVal.push_back(MQTT_UNSUBSCRIBE | 0x02);
			}
			else if (sVerb == "PUBLISH")
			{
				byte	bByte0 = MQTT_PUBLISH;

				// Fixed Header
				PyObject *brDUP = PyDict_GetItemString(WriteMessage->m_Object, "Duplicate");
				if (brDUP && PyLong_Check(brDUP))
				{
					long	bDUP = PyLong_AsLong(brDUP);
					if (bDUP) bByte0 |= 0x08; // Set duplicate flag
				}

				PyObject *brQoS = PyDict_GetItemString(WriteMessage->m_Object, "QoS");
				long	iQoS = 0;
				if (brQoS && PyLong_Check(brQoS))
				{
					iQoS = PyLong_AsLong(brQoS);
					bByte0 |= ((iQoS & 3) << 1); // Set QoS flag
				}

				PyObject *brRetain = PyDict_GetItemString(WriteMessage->m_Object, "Retain");
				if (brRetain && PyLong_Check(brRetain))
				{
					long	bRetain = PyLong_AsLong(brRetain);
					bByte0 |= (bRetain & 1); // Set retain flag
				}

				// Variable Header
				PyObject*	brTopic = PyDict_GetItemString(WriteMessage->m_Object, "Topic");
				if (brTopic && PyUnicode_Check(brTopic))
				{
					MQTTPushBackStringWLen(std::string(PyUnicode_AsUTF8(brTopic)), vVariableHeader);
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) MQTT Publish: No 'Topic' specified, nothing to publish. See Python Plugin wiki page for help.", __func__);
					return retVal;
				}

				PyObject *brID = PyDict_GetItemString(WriteMessage->m_Object, "PacketIdentifier");
				if (iQoS)
				{
					long	iPacketIdentifier = 0;
					if (brID && PyLong_Check(brID))
					{
						iPacketIdentifier = PyLong_AsLong(brID);
					}
					else iPacketIdentifier = m_PacketID++;
					MQTTPushBackNumber((int)iPacketIdentifier, vVariableHeader);
				}
				else if (brID)
				{
					_log.Log(LOG_ERROR, "(%s) MQTT Publish: PacketIdentifier ignored, QoS not specified. See Python Plugin wiki page for help.", __func__);
				}

				// Payload
				std::string sPayload = "";
				PyObject*	brPayload = PyDict_GetItemString(WriteMessage->m_Object, "Payload");
				// Support both string and bytes
				//if (pPayload && PyByteArray_Check(pPayload)) // Gives linker error, why?
				if (brPayload) {
					_log.Debug(DEBUG_NORM, "(%s) MQTT Publish: payload %p (%s)", __func__, brPayload, brPayload->ob_type->tp_name);
				}
				if (brPayload && brPayload->ob_type->tp_name == std::string("bytearray"))
				{
					sPayload = std::string(PyByteArray_AsString(brPayload), PyByteArray_Size(brPayload));
				}
				else if (brPayload && PyUnicode_Check(brPayload))
				{
					sPayload = std::string(PyUnicode_AsUTF8(brPayload));
				}
				MQTTPushBackString(sPayload, vPayload);

				retVal.push_back(bByte0);
			}
			else if (sVerb == "PUBREL")
			{
				// Variable Header
				PyObject *brID = PyDict_GetItemString(WriteMessage->m_Object, "PacketIdentifier");
				long	iPacketIdentifier = 0;
				if (brID && PyLong_Check(brID))
				{
					iPacketIdentifier = PyLong_AsLong(brID);
					MQTTPushBackNumber((int)iPacketIdentifier, vVariableHeader);
				}
				else
				{
					_log.Log(LOG_ERROR, "(%s) MQTT Publish: No valid PacketIdentifier specified. See Python Plugin wiki page for help.", __func__);
					return retVal;
				}

				retVal.push_back(MQTT_PUBREL & 0x02);
			}
			else if (sVerb == "DISCONNECT")
			{
				retVal.push_back(MQTT_DISCONNECT);
				retVal.push_back(0);
			}
			else
			{
				_log.Log(LOG_ERROR, "(%s) MQTT 'Verb' invalid '%s' is unknown.", __func__, sVerb.c_str());
				return retVal;
			}
		}

		// Build final message
		unsigned long	iRemainingLength = vVariableHeader.size() + vPayload.size();
		do
		{
			byte	encodedByte = iRemainingLength % 128;
			iRemainingLength = iRemainingLength / 128;

			// if there are more data to encode, set the top bit of this byte
			if (iRemainingLength > 0)
				encodedByte |= 128;
			retVal.push_back(encodedByte);

		} while (iRemainingLength > 0);

		retVal.insert(retVal.end(), vVariableHeader.begin(), vVariableHeader.end());
		retVal.insert(retVal.end(), vPayload.begin(), vPayload.end());

		return retVal;
	}

	/*

	See: https://tools.ietf.org/html/rfc6455#section-5.2

	  0                   1                   2                   3
	  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 +-+-+-+-+-------+-+-------------+-------------------------------+
	 |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	 |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	 |N|V|V|V|       |S|             |   (if payload len==126/127)   |
	 | |1|2|3|       |K|             |                               |
	 +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	 |     Extended payload length continued, if payload len == 127  |
	 + - - - - - - - - - - - - - - - +-------------------------------+
	 |                               |Masking-key, if MASK set to 1  |
	 +-------------------------------+-------------------------------+
	 | Masking-key (continued)       |          Payload Data         |
	 +-------------------------------- - - - - - - - - - - - - - - - +
	 :                     Payload Data continued ...                :
	 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	 |                     Payload Data continued ...                |
	 +---------------------------------------------------------------+
	
	*/
	
	bool CPluginProtocolWS::ProcessWholeMessage(std::vector<byte> &vMessage, const ReadEvent * Message)
	{
		while (vMessage.size())
		{
			// Look for a complete message
			std::vector<byte>	vPayload;
			int			iOffset = 0;
			int			iOpCode = 0;
			long		lMaskingKey = 0;
			bool		bFinish = false;

			bFinish = (vMessage[iOffset] & 0x80);				// Indicates that this is the final fragment in a message if true
			if (vMessage[iOffset] & 0x0F)
			{
				iOpCode = (vMessage[iOffset] & 0x0F);			// %x0 denotes a continuation frame
			}
			// %x1 denotes a text frame
			// %x2 denotes a binary frame
			// %x8 denotes a connection close
			// %x9 denotes a ping
			// %xA denotes a pong
			iOffset++;
			bool	bMasked = (vMessage[iOffset] & 0x80);			// Is the payload masked?
			long	lPayloadLength = (vMessage[iOffset] & 0x7F);	// if < 126 then this is the length
			if (lPayloadLength == 126)
			{
				if (vMessage.size() < (size_t)(iOffset + 2))
					return false;
				lPayloadLength = (vMessage[iOffset + 1] << 8) + vMessage[iOffset + 2];
				iOffset += 2;
			}
			else if (lPayloadLength == 127)							// 64 bit lengths not supported
			{
				_log.Log(LOG_ERROR, "(%s) 64 bit WebSocket messages lengths not supported.", __func__);
				vMessage.clear();
				iOffset += 5;
				return false;
			}
			iOffset++;

			byte*	pbMask = NULL;
			if (bMasked)
			{
				if (vMessage.size() < (size_t)iOffset)
					return false;
				lMaskingKey = (long)vMessage[iOffset];
				pbMask = &vMessage[iOffset];
				iOffset += 4;
			}

			if (vMessage.size() < (size_t)(iOffset + lPayloadLength))
				return false;

			// Append the payload to the existing (maybe) payload
			if (lPayloadLength)
			{
				vPayload.reserve(vPayload.size() + lPayloadLength);
				for (int i = iOffset; i < iOffset + lPayloadLength; i++)
				{
					vPayload.push_back(vMessage[i]);
				}
				iOffset += lPayloadLength;
			}

			PyObject*	pDataDict = (PyObject*)PyDict_New();
			PyObject*	nrPayload = NULL;

			// Handle full message
			PyObject *nrObj = Py_BuildValue("N", PyBool_FromLong(bFinish));
			if (PyDict_SetItemString(pDataDict, "Finish", nrObj) == -1)
				_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", __func__, "Finish", bFinish ? "True" : "False");
			Py_XDECREF(nrObj);

			// Masked data?
			if (lMaskingKey)
			{
				// Unmask data
				for (int i = 0; i < lPayloadLength; i++)
				{
					vPayload[i] ^= pbMask[i % 4];
				}
				PyObject*	nrObj = Py_BuildValue("i", lMaskingKey);
				if (PyDict_SetItemString(pDataDict, "Mask", nrObj) == -1)
					_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%ld' to dictionary.", __func__, "Mask", lMaskingKey);
				Py_XDECREF(nrObj);
			}

			switch (iOpCode)
			{
			case 0x01:	// Text message
			{
				std::string		sPayload(vPayload.begin(), vPayload.end());
				nrPayload = Py_BuildValue("s", sPayload.c_str());
				break;
			}
			case 0x02:	// Binary message
				break;
			case 0x08:	// Connection Close
			{
				PyObject*	nrObj = Py_BuildValue("s", "Close");
				if (PyDict_SetItemString(pDataDict, "Operation", nrObj) == -1)
					_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", __func__, "Operation", "Close");
				Py_XDECREF(nrObj);
				if (vPayload.size() == 2)
				{
					int		iReasonCode = (vPayload[0] << 8) + vPayload[1];
					nrPayload = Py_BuildValue("i", iReasonCode);
				}
				break;
			}
			case 0x09:	// Ping
			{
				pDataDict = (PyObject*)PyDict_New();
				PyObject*	nrObj = Py_BuildValue("s", "Ping");
				if (PyDict_SetItemString(pDataDict, "Operation", nrObj) == -1)
					_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", __func__, "Operation", "Ping");
				Py_XDECREF(nrObj);
				break;
			}
			case 0x0A:	// Pong
			{
				pDataDict = (PyObject*)PyDict_New();
				PyObject*	nrObj = Py_BuildValue("s", "Pong");
				if (PyDict_SetItemString(pDataDict, "Operation", nrObj) == -1)
					_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", __func__, "Operation", "Pong");
				Py_XDECREF(nrObj);
				break;
			}
			default:
				_log.Log(LOG_ERROR, "(%s) Unknown Operation Code (%d) encountered.", __func__, iOpCode);
			}

			// If there is a payload but not handled then map it as binary
			if (vPayload.size() && !nrPayload)
			{
				nrPayload = Py_BuildValue("y#", &vPayload[0], vPayload.size());
			}

			// If there is a payload then add it
			if (nrPayload)
			{
				if (PyDict_SetItemString(pDataDict, "Payload", nrPayload) == -1)
					_log.Log(LOG_ERROR, "(%s) failed to add key '%s' to dictionary.", __func__, "Payload");
				Py_XDECREF(nrPayload);
			}

			Message->m_pPlugin->MessagePlugin(new onMessageCallback(Message->m_pPlugin, Message->m_pConnection, pDataDict)); 
			
			// Remove the processed message from retained data
			vMessage.erase(vMessage.begin(), vMessage.begin() + iOffset);

			return true;
		}

		return false;
	}

	void CPluginProtocolWS::ProcessInbound(const ReadEvent * Message)
	{
		//	Although messages can be fragmented, control messages can be inserted in between fragments
		//	so try to process just the message first, then retained data and the message
		std::vector<byte>	Buffer = Message->m_Buffer;
		if (ProcessWholeMessage(Buffer, Message))
		{
			return;		// Message processed
		}

		// Add new message to retained data, process all messages if this one is the finish of a message
		m_sRetainedData.insert(m_sRetainedData.end(), Message->m_Buffer.begin(), Message->m_Buffer.end());

		// Always process the whole buffer because we can't know if we have whole, multiple or even complete messages unless we work through from the start
		if (ProcessWholeMessage(m_sRetainedData, Message))
		{
			return;		// Message processed
		}

	}

	std::vector<byte> CPluginProtocolWS::ProcessOutbound(const WriteDirective * WriteMessage)
	{
		std::vector<byte>	retVal;

		//
		//	Parameters need to be in a dictionary.
		//	if a 'URL' key is found message is assumed to be HTTP otherwise WebSocket is assumed
		//
		if (!WriteMessage->m_Object || !PyDict_Check(WriteMessage->m_Object))
		{
			_log.Log(LOG_ERROR, "(%s) Dictionary parameter expected.", __func__);
		}
		else
		{
			PyObject *brURL = PyDict_GetItemString(WriteMessage->m_Object, "URL");
			if (brURL)
			{
				// Is a verb specified?
				PyObject *brVerb = PyDict_GetItemString(WriteMessage->m_Object, "Verb");
				if (!brVerb)
				{
					PyObject*	nrObj = Py_BuildValue("s", "GET");
					if (PyDict_SetItemString(WriteMessage->m_Object, "Verb", nrObj) == -1)
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", __func__, "Verb", "GET");
					Py_XDECREF(nrObj);
				}

				// Required headers specified?
				PyObject *pHeaders = PyDict_GetItemString(WriteMessage->m_Object, "Headers");
				if (!pHeaders)
				{
					pHeaders = (PyObject*)PyDict_New();
					if (PyDict_SetItemString(WriteMessage->m_Object, "Headers", (PyObject*)pHeaders) == -1)
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s' to dictionary.", "WS", "Headers");
					Py_XDECREF(pHeaders);
				}
				PyObject *brConnection = PyDict_GetItemString(pHeaders, "Connection");
				if (!brConnection)
				{
					PyObject*	nrObj = Py_BuildValue("s", "keep-alive, Upgrade");
					if (PyDict_SetItemString(pHeaders, "Connection", nrObj) == -1)
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", __func__, "Connection", "Upgrade");
					Py_XDECREF(nrObj);
				}
				PyObject *brUpgrade = PyDict_GetItemString(pHeaders, "Upgrade");
				if (!brUpgrade)
				{
					PyObject*	nrObj = Py_BuildValue("s", "websocket");
					if (PyDict_SetItemString(pHeaders, "Upgrade", nrObj) == -1)
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", __func__, "Upgrade", "websocket");
					Py_XDECREF(nrObj);
				}
				PyObject *brUserAgent = PyDict_GetItemString(pHeaders, "User-Agent");
				if (!brUserAgent)
				{
					PyObject*	nrObj = Py_BuildValue("s", "Domoticz/1.0");
					if (PyDict_SetItemString(pHeaders, "User-Agent", nrObj) == -1)
						_log.Log(LOG_ERROR, "(%s) failed to add key '%s', value '%s' to dictionary.", __func__, "User-Agent", "Domoticz/1.0");
					Py_XDECREF(nrObj);
				}

				// Use parent HTTP protocol object to do the actual formatting
				return CPluginProtocolHTTP::ProcessOutbound(WriteMessage);
			}
			else
			{
				int			iOpCode = 0;
				long		lMaskingKey = 0;
				long		lPayloadLength = 0;
				byte		bMaskBit = 0x00;

				PyObject *brOperation = PyDict_GetItemString(WriteMessage->m_Object, "Operation");
				PyObject *brPayload = PyDict_GetItemString(WriteMessage->m_Object, "Payload");
				PyObject *brMask = PyDict_GetItemString(WriteMessage->m_Object, "Mask");

				if (brOperation)
				{
					if (!PyUnicode_Check(brOperation))
					{
						_log.Log(LOG_ERROR, "(%s) Expected dictionary 'Operation' key to have a string value.", __func__);
						return retVal;
					}

					std::string	sOperation = PyUnicode_AsUTF8(brOperation);
					if (sOperation == "Ping")
					{
						iOpCode = 0x09;
					}
					else if (sOperation == "Pong")
					{
						iOpCode = 0x0A;
					}
					else if (sOperation == "Close")
					{
						iOpCode = 0x08;
					}
				}

				// If there is no specific OpCode then set it from the payload datatype
				if (brPayload)
				{
					if (PyUnicode_Check(brPayload))
					{
						lPayloadLength = PyUnicode_GetLength(brPayload);
						if (!iOpCode) iOpCode = 0x01;				// Text message
					}
					else if (PyBytes_Check(brPayload))
					{
						lPayloadLength = PyBytes_Size(brPayload);
						if (!iOpCode) iOpCode = 0x02;				// Binary message
					}
					else if (brPayload->ob_type->tp_name == std::string("bytearray"))
					{
						lPayloadLength = PyByteArray_Size(brPayload);
						if (!iOpCode) iOpCode = 0x02;				// Binary message
					}
				}

				if (brMask)
				{
					if (PyLong_Check(brMask))
					{
						lMaskingKey = PyLong_AsLong(brMask);
						bMaskBit = 0x80;							// Set mask bit in header
					}
					else if (PyUnicode_Check(brMask))
					{
						std::string	sMask = PyUnicode_AsUTF8(brMask);
						lMaskingKey = atoi(sMask.c_str());
						bMaskBit = 0x80;							// Set mask bit in header
					}
					else
					{
						_log.Log(LOG_ERROR, "(%s) Invalid mask, expected number (integer or string).", __func__);
						return retVal;
					}
				}

				// Assemble the actual message
				retVal.reserve(lPayloadLength+16);		// Masking relies on vector not reallocating during message assembly
				retVal.push_back(0x80 | iOpCode);
				if (lPayloadLength < 126)
				{
					retVal.push_back(bMaskBit | lPayloadLength);	// Short length
				}
				else
				{
					retVal.push_back(bMaskBit | 126);
					retVal.push_back(lPayloadLength >> 24);
					retVal.push_back(lPayloadLength >> 16);
					retVal.push_back(lPayloadLength >> 8);
					retVal.push_back(lPayloadLength);				// Longer length
				}

				byte*	pbMask = NULL;
				if (bMaskBit)
				{
					retVal.push_back(lMaskingKey >> 24);
					pbMask = &retVal[retVal.size()-1];
					retVal.push_back(lMaskingKey >> 16);
					retVal.push_back(lMaskingKey >> 8);
					retVal.push_back(lMaskingKey);					// Encode mask
				}

				if (brPayload)
				{
					if (PyUnicode_Check(brPayload))
					{
						std::string	sPayload = PyUnicode_AsUTF8(brPayload);
						for (int i = 0; i < lPayloadLength; i++)
						{
							retVal.push_back(sPayload[i] ^ pbMask[i%4]);
						}
					}
					else if (PyBytes_Check(brPayload))
					{
						byte*	pByte = (byte*)PyBytes_AsString(brPayload);
						for (int i = 0; i < lPayloadLength; i++)
						{
							retVal.push_back(pByte[i] ^ pbMask[i % 4]);
						}
					}
					else if (brPayload->ob_type->tp_name == std::string("bytearray"))
					{
						byte*	pByte = (byte*)PyByteArray_AsString(brPayload);
						for (int i = 0; i < lPayloadLength; i++)
						{
							retVal.push_back(pByte[i] ^ pbMask[i % 4]);
						}
					}
				}

			}
		}

		return retVal;
	}
}
#endif
