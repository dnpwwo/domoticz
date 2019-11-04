#include "stdafx.h"
#include "WebsocketHandler.h"
#include "../main/localtime_r.h"
#include "../main/mainworker.h"
#include "../main/Helper.h"
#include "../json/json.h"
#include "webserver/cWebem.h"
#include "../main/Logger.h"

#define WEBSOCKET_SESSION_TIMEOUT 86400 // 1 day

namespace http {
	namespace server {

		CWebsocketHandler::CWebsocketHandler(cWebem *pWebem, boost::function<void(const std::string &packet_data)> _MyWrite) : 
			sessionid(""),
//			MyWrite(_MyWrite),
			myWebem(pWebem)
		{
			
		}

		CWebsocketHandler::~CWebsocketHandler()
		{
		}

		boost::tribool CWebsocketHandler::Handle(const std::string &packet_data, bool outbound)
		{
			Json::Value jsonValue;
			Json::StyledWriter writer;
/*
			try
			{
				// WebSockets only do security during set up so keep pushing the expiry out to stop it being cleaned up
				WebEmSession session;
				std::map<std::string, WebEmSession>::iterator itt = myWebem->m_sessions.find(sessionid);
				if (itt != myWebem->m_sessions.end())
				{
					session = itt->second;
				}
				else
					// for outbound messages create a temporary session if required
					// todo: Add the username and rights from the original connection
					if (outbound)
					{
							time_t	nowAnd1Day = ((time_t)mytime(NULL)) + WEBSOCKET_SESSION_TIMEOUT;
							session.timeout = nowAnd1Day;
							session.expires = nowAnd1Day;
							session.isnew = false;
							session.forcelogin = false;
							session.rememberme = false;
							session.reply_status = 200;
					}


				Json::Reader reader;
				Json::Value value;
				if (!reader.parse(packet_data, value)) {
					return true;
				}
				std::string szEvent = value["event"].asString();
				if (szEvent.find("request") == std::string::npos)
					return true;

				request req;
				req.method = "GET";
				std::string querystring = value["query"].asString();
				req.uri = "/json.htm?" + querystring;
				req.http_version_major = 1;
				req.http_version_minor = 1;
				req.headers.resize(0); // todo: do we need any headers?
				req.content.clear();
				reply rep;
				if (myWebem->CheckForPageOverride(session, req, rep)) {
					if (rep.status == reply::ok) {
						jsonValue["request"] = szEvent;
						jsonValue["event"] = "response";
						Json::Value::Int64 reqID = value["requestid"].asInt64();
						jsonValue["requestid"] = reqID;
						jsonValue["data"] = rep.content;
						std::string response = writer.write(jsonValue);
						MyWrite(response);
						return true;
					}
				}
			}
			catch (std::exception& e)
			{
				_log.Log(LOG_ERROR, "WebsocketHandler::%s Exception: %s", __func__, e.what());
			}

			jsonValue["error"] = "Internal Server Error!!";
			std::string response = writer.write(jsonValue);
			MyWrite(response);
*/			return true;
		}

		void CWebsocketHandler::Start()
		{
			RequestStart();

			m_Subscriber = m_mainworker.m_UpdateManager.Publisher.connect(boost::bind(&CWebsocketHandler::DatabaseUpdate, this, _1));

			//Start worker thread
			m_thread = std::make_shared<std::thread>(&CWebsocketHandler::Do_Work, this);
		}

		void CWebsocketHandler::Stop()
		{
			m_Subscriber.disconnect();

			if (m_thread)
			{
				RequestStop();
				m_thread->join();
				m_thread.reset();
			}
		}

		void CWebsocketHandler::Do_Work()
		{
			while (!IsStopRequested(1000))
			{
				time_t atime = mytime(NULL);
				if (atime % 10 == 0)
				{
					//Send Date/Time every 10 seconds
				}
			}
		}

		void CWebsocketHandler::DatabaseUpdate(CUpdateEntry* pEntry)
		{
			try
			{
				Json::Value root;
				root["Count"] = 1;
				for (int i = 0; i < pEntry->m_Columns.size(); i++)
				{
					if (pEntry->m_ColumnTypes[i] == "INTEGER")
					{
						root[pEntry->m_Table.c_str()][0][pEntry->m_Columns[i]] = atoi(pEntry->m_Values[i].c_str());
					}
					else
					{
						root[pEntry->m_Table.c_str()][0][pEntry->m_Columns[i]] = pEntry->m_Values[i];
					}
				}

				// Add to the response
				Json::FastWriter	jWriter;
				std::string response = jWriter.write(root);
				_log.Log(LOG_NORM, "WebsocketHandler::%s Called, Message: %s", __func__, response.c_str());
//				MyWrite(response);
			}
			catch (std::exception& e)
			{
				_log.Log(LOG_ERROR, "WebsocketHandler::%s Exception: %s", __func__, e.what());
			}
		}
	}
}