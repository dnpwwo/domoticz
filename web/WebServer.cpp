#include "stdafx.h"
#include "WebServer.h"
#include <boost/bind.hpp>
#include <iostream>
#include <fstream>
#include "../main/mainworker.h"
#include "../main/Helper.h"
#include "../main/SQLHelper.h"
#include "../main/Logger.h"
#include "../main/localtime_r.h"
#include "webserver/Base64.h"
#include "../json/json.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define round(a) ( int ) ( a + .5 )

extern std::string szUserDataFolder;
extern std::string szWWWFolder;

extern std::string szAppVersion;
extern std::string szAppHash;
extern std::string szAppDate;
extern std::string szPyVersion;

extern bool g_bUseUpdater;

extern time_t m_StartTime;

extern bool g_bDontCacheWWW;

namespace http {
	namespace server {

		CWebServer::CWebServer(void) : session_store()
		{
			m_pWebEm = NULL;
			m_bDoStop = false;
		}

		CWebServer::~CWebServer(void)
		{
			// RK, we call StopServer() instead of just deleting m_pWebEm. The Do_Work thread might still be accessing that object
			StopServer();
		}

		void CWebServer::Do_Work()
		{
			bool exception_thrown = false;
			while (!m_bDoStop)
			{
				exception_thrown = false;
				try {
					if (m_pWebEm) {
						m_pWebEm->Run();
					}
				}
				catch (std::exception& e) {
					_log.Log(LOG_ERROR, "WebServer(%s) exception occurred : '%s'", m_server_alias.c_str(), e.what());
					exception_thrown = true;
				}
				catch (...) {
					_log.Log(LOG_ERROR, "WebServer(%s) unknown exception occurred", m_server_alias.c_str());
					exception_thrown = true;
				}
				if (exception_thrown) {
					_log.Log(LOG_STATUS, "WebServer(%s) restart server in 5 seconds", m_server_alias.c_str());
					sleep_milliseconds(5000); // prevents from an exception flood
					continue;
				}
				break;
			}
			_log.Log(LOG_STATUS, "WebServer(%s) stopped", m_server_alias.c_str());
		}

		bool CWebServer::StartServer(server_settings & settings, const std::string & serverpath, const bool bIgnoreUsernamePassword)
		{
			m_server_alias = (settings.is_secure() == true) ? "SSL" : "HTTP";

			if (!settings.is_enabled())
				return true;

			int tries = 0;
			bool exception = false;

			do {
				try {
					exception = false;
					m_pWebEm = new http::server::cWebem(settings, serverpath.c_str());
				}
				catch (std::exception& e) {
					exception = true;
					switch (tries) {
					case 0:
						settings.listening_address = "::";
						break;
					case 1:
						settings.listening_address = "0.0.0.0";
						break;
					case 2:
						_log.Log(LOG_ERROR, "WebServer(%s) startup failed on address %s with port: %s: %s", m_server_alias.c_str(), settings.listening_address.c_str(), settings.listening_port.c_str(), e.what());
						if (atoi(settings.listening_port.c_str()) < 1024)
							_log.Log(LOG_ERROR, "WebServer(%s) check privileges for opening ports below 1024", m_server_alias.c_str());
						else
							_log.Log(LOG_ERROR, "WebServer(%s) check if no other application is using port: %s", m_server_alias.c_str(), settings.listening_port.c_str());
						return false;
					}
					tries++;
				}
			} while (exception);

			_log.Log(LOG_STATUS, "WebServer(%s) started on address: %s with port %s", m_server_alias.c_str(), settings.listening_address.c_str(), settings.listening_port.c_str());

			m_pWebEm->SetDigistRealm("Domoticz.com");
			m_pWebEm->SetSessionStore(this);

			std::string WebRemoteProxyIPs;
			if (m_sql.GetPreferencesVar("WebRemoteProxyIPs", WebRemoteProxyIPs, std::string("")) && WebRemoteProxyIPs.length())
			{
				std::vector<std::string> strarray;
				StringSplit(WebRemoteProxyIPs, ";", strarray);
				for (const auto & itt : strarray)
					m_pWebEm->AddRemoteProxyIPs(itt);
			}

			//register callbacks
			m_pWebEm->RegisterPageCode("/API", boost::bind(&CWebServer::HandleREST, this, _1, _2, _3));

			//Start normal worker thread
			m_bDoStop = false;
			m_thread = std::make_shared<std::thread>(&CWebServer::Do_Work, this);
			std::string server_name = "WebServer_" + settings.listening_port;
			SetThreadName(m_thread->native_handle(), server_name.c_str());
			return (m_thread != nullptr);
		}

		void CWebServer::StopServer()
		{
			m_bDoStop = true;
			try
			{
				if (m_pWebEm == NULL)
					return;
				m_pWebEm->Stop();
				if (m_thread) {
					m_thread->join();
					m_thread.reset();
				}
				delete m_pWebEm;
				m_pWebEm = NULL;
			}
			catch (...)
			{

			}
		}

		void CWebServer::SetWebCompressionMode(const _eWebCompressionMode gzmode)
		{
			if (m_pWebEm == NULL)
				return;
			m_pWebEm->SetWebCompressionMode(gzmode);
		}

		void CWebServer::SetAuthenticationMethod(const _eAuthenticationMethod amethod)
		{
			if (m_pWebEm == NULL)
				return;
			m_pWebEm->SetAuthenticationMethod(amethod);
		}

		void CWebServer::SetWebTheme(const std::string &themename)
		{
			if (m_pWebEm == NULL)
				return;
			m_pWebEm->SetWebTheme(themename);
		}

		void CWebServer::SetWebRoot(const std::string &webRoot)
		{
			if (m_pWebEm == NULL)
				return;
			m_pWebEm->SetWebRoot(webRoot);
		}

		/**
		 * Retrieve user session from store, without remote host.
		 */
		const WebEmStoredSession CWebServer::GetSession(const std::string & sessionId) {
			//_log.Log(LOG_STATUS, "SessionStore : get...");
			WebEmStoredSession session;

			if (sessionId.empty()) {
				_log.Log(LOG_ERROR, "SessionStore : cannot get session without id.");
			}
			else {
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT SessionID, UserID, RoleID, AuthToken, Expiry FROM UserSession WHERE SessionID = '%q'",
					sessionId.c_str());
				if (!result.empty()) {
					session.id = result[0][0].c_str();
					session.username = base64_decode(result[0][1]);
					session.auth_token = result[0][2].c_str();

					std::string sExpirationDate = result[0][3];
					time_t now = mytime(NULL);
					struct tm tExpirationDate;
					ParseSQLdatetime(session.expires, tExpirationDate, sExpirationDate);
					// RemoteHost is not used to restore the session
					// LastUpdate is not used to restore the session
				}
			}

			return session;
		}

		/**
		 * Save user session.
		 */
		void CWebServer::StoreSession(const WebEmStoredSession & session) {
			//_log.Log(LOG_STATUS, "SessionStore : store...");
			if (session.id.empty()) {
				_log.Log(LOG_ERROR, "SessionStore : cannot store session without id.");
				return;
			}

			char szExpires[30];
			struct tm ltime;
			localtime_r(&session.expires, &ltime);
			strftime(szExpires, sizeof(szExpires), "%Y-%m-%d %H:%M:%S", &ltime);

			std::string remote_host = (session.remote_host.size() <= 50) ? // IPv4 : 15, IPv6 : (39|45)
				session.remote_host : session.remote_host.substr(0, 50);

			WebEmStoredSession storedSession = GetSession(session.id);
			if (storedSession.id.empty()) {
				m_sql.safe_query(
					"INSERT INTO UserSession (SessionID, UserID, RoleID, AuthToken, Expiry) VALUES ('%q', '%q', '%q', '%q', '%q')",
					session.id.c_str(),
					session.username,
					session.auth_token.c_str(),
					szExpires);
			}
			else {
				m_sql.safe_query(
					"UPDATE UserSession set AuthToken = '%q', Expiry = '%q') WHERE SessionID = '%q'", session.auth_token.c_str(), szExpires, session.id.c_str());
			}
		}

		/**
		 * Remove user session and expired sessions.
		 */
		void CWebServer::RemoveSession(const std::string & sessionId) {
			//_log.Log(LOG_STATUS, "SessionStore : remove...");
			if (sessionId.empty()) {
				return;
			}
			m_sql.safe_query(
				"DELETE FROM UserSession WHERE SessionID = '%q'",
				sessionId.c_str());
		}

		/**
		 * Remove all expired user sessions.
		 */
		void CWebServer::CleanSessions() {
			//_log.Log(LOG_STATUS, "SessionStore : clean...");
			m_sql.safe_query(
				"DELETE FROM UserSession WHERE Expiry < datetime('now', 'localtime')");
		}

		/**
		 * Delete all user's session, except the session used to modify the username or password.
		 * username must have been hashed
		 *
		 * Note : on the WebUserName modification, this method will not delete the session, but the session will be deleted anyway
		 * because the username will be unknown (see cWebemRequestHandler::checkAuthToken).
		 */
		void CWebServer::RemoveUsersSessions(const std::string& username, const WebEmSession & exceptSession) {
			m_sql.safe_query("DELETE FROM UserSession US, User U WHERE (U.Username =='%q') AND (U.UserID = US.UserID) AND (SessionID!='%q')", username.c_str(), exceptSession.id.c_str());
		}

		void CWebServer::HandleREST(WebEmSession& session, const request& req, reply& rep)
		{
			//API/Interfaces
			//API/Interfaces/1
			//API/Interfaces/1/InterfaceLogs
			//API/Interfaces/1/Devices
			//API/Interfaces/1/Devices/1
			//API/Interfaces/1/Devices/1/Values
			//API/Interfaces/1/Devices/1/Values/1
			//API/Interfaces/1/Devices/1/Values/1/ValueLogs
			//API/Interfaces/1/Devices/1/Values/1/ValueHistorys
			//API/Interfaces/1/Devices/1/Values/1/ValueNotifications

			//API/Interfaces/1/Devices/1/Values/1/Units		?????

			//API/Units
			//API/Units/1
			//API/Units/1/Values
			//API/Units/1/Values/1/ValueLogs
			//API/Units/1/Values/1/ValueHistorys
			//API/Units/1/Values/1/ValueNotifications
			//API/Units/1/Values/1/ValueNotifications/?/Interfaces
			//API/Units/1/Values/1/Devices					????

			//API/Users
			//API/Users/1
			//API/Users/1/UserSessions
			//API/Users/1/Role
			//API/Users/1/Role/1
			//API/Users/1/Role/1/RESTPrivileges

			//API/Roles
			//API/Roles/1
			//API/Roles/1/Users
			//API/Roles/1/Users/1
			//API/Roles/1/Users/1/UserSessions
			//API/Roles/1/RESTPrivieges


			// Break URI into parts
			std::vector<std::string> vURIElements;
			if (req.uri.find("/", 5))
			{
				std::string::size_type prev_pos = 5, pos = 5; // step over '/API/'
				std::string substring;
				while ((pos = req.uri.find("/", pos)) != std::string::npos)
				{
					substring = req.uri.substr(prev_pos, pos - prev_pos);
					if (substring.length()) vURIElements.push_back(substring);
					prev_pos = ++pos;
				}
				substring = req.uri.substr(prev_pos, pos - prev_pos);
				if (substring.length()) vURIElements.push_back(substring);
			}

			// Doesn't look like a REST request
			if (vURIElements.empty())
			{
				rep = reply::stock_reply(reply::bad_request);
				return;
			}

			// Confirm which part of the URI we are operating on
			// Need one of three cases:
			//	1.	/API/Interfaces					== Request for summary of a whole table (No Keys)
			//	2.	/API/Interfaces/123				== Request for details of specific instance from a table (TableKey only)
			//	3.	/API/Interfaces/123/Devices		== Request for summary of table filterd for a specific parent instance (ParentKey)
			//	4.	/API/Interfaces/123/Devices/1	== THIS SHOULD BE HANDLED TO MAKE SURE DATE RETURNED IS RELEVANT TO THE PARENT
			std::string	sTable;
			int			iTableKey = 0;
			std::string	sParent;
			int			iParentKey = 0;
			int			iTable = vURIElements.size();
			// Target table is the rightmost non-numeric URI element 
			while (iTable)
			{
				char*	p = NULL;
				int		iNumber = strtol(vURIElements[--iTable].c_str(), &p, 10);
				if (*p)
				{
					sTable = p;
					break;
				}
			}
			// Trim off trailing 's' of table name
			if (sTable.length())
			{
				sTable = sTable.substr(0, sTable.length()-1);
			}

			// if there is a URI element to the right look for the TableKey (Case 2)
			if ((iTable+1) < vURIElements.size())
			{
				char* p = NULL;
				iTableKey = strtol(vURIElements[iTable + 1].c_str(), &p, 10);
			}

			// if there are URI elements to the left look for the ParentKey and Table (Case 3)
			if ((iTable > 1) && !iTableKey)
			{
				char* p = NULL;
				iParentKey = strtol(vURIElements[iTable - 1].c_str(), &p, 10);
				sParent = vURIElements[iTable - 2];
				if (sParent.length())
				{
					sParent = sParent.substr(0, sParent.length() - 1);
				}
			}

			// Does this look like a valid request?
			if (
				!(!iTable && sTable.length() && !iTableKey && !iParentKey)	&&			// Case 1
				!(sTable.length() &&  iTableKey && !iParentKey)	&&						// Case 2
				!(sTable.length() && !iTableKey &&  iParentKey && sParent.length()) 	// Case 3
				)
			{
				rep = reply::stock_reply(reply::bad_request);
				return;
			}

			// Check user has access to the table for the requested HTTP verb
			// TODO: get user ID from actual AccessToken Session
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT Can%q, PUTFields, PATCHFields FROM TableAccess T, User U WHERE U.Username = '%q' AND U.RoleID = T.RoleID AND T.Name = '%q'", req.method.c_str(), "Admin", sTable.c_str());
			if (result.empty() || (result[0][0] != "1"))
			{
				rep = reply::stock_reply(reply::unauthorized);
				return;
			}

			// Handle various HTTP verbs
			Json::Value root;
			reply::status_type	status = reply::ok;
			if (req.method == "GET")
			{
				status = RESTfulGET(sTable, iTableKey, sParent, iParentKey, root);
			}

			if (status != reply::ok)
			{
				rep = reply::stock_reply(status);
				return;
			}

			// Add to the response
			Json::StyledWriter	jWriter;
			rep.content = jWriter.write(root);


			// req.headers (http::server::header  name,value)
			// req.content_length (appears not set for 0 length)
			// req.content (string)

			return;
		}

		reply::status_type CWebServer::RESTfulGET(std::string& sTable, int iTableKey, std::string& sParent, int iParentKey, Json::Value& root)
		{
			// look up structure of requested table
			std::string		sSQL = "pragma table_info('" + sTable + "');";
			std::vector<std::vector<std::string> > vTableStruct = m_sql.safe_query(sSQL.c_str());
			if (vTableStruct.empty())
			{
				return reply::not_found;
			}

			// look up and encode requested table
			sSQL = "SELECT ";
			for (int i = 0; i < vTableStruct.size(); i++)
			{
				sSQL += "T."+vTableStruct[i][1];
				if (i != vTableStruct.size() - 1) sSQL += ", ";
			}
			sSQL += " FROM " + sTable+ " T ";	// Case 1
			if (iTableKey)
			{	// Case 2
				sSQL += " WHERE T." + sTable + "ID=" + std::to_string(iTableKey);
			}
			if (iParentKey)
			{	// Case 3a - We can't be sure which way around the keys go
				std::string		testSQL = sSQL + ", "+sParent+" P WHERE P." + sParent + "ID=" + std::to_string(iParentKey) + " AND P."+ sParent +"ID=T."+ sParent +"ID";
				std::vector<std::vector<std::string> >	test = m_sql.safe_query(testSQL.c_str());
				if (test.empty())
				{	// Case 3b - the other way around
					sSQL += ", " + sParent + " P WHERE P." + sParent + "ID=" + std::to_string(iParentKey) + " AND P." + sTable + "ID=T." + sTable + "ID";
				}
				else
				{
					sSQL = testSQL;
				}
			}
			std::vector<std::vector<std::string> >	result = m_sql.safe_query(sSQL.c_str());
			if (result.empty())
			{
				return reply::not_found;
			}

			root["Count"] = std::to_string(result.size());
			int		iIndex = 0;
			for (const auto& itt : result)
			{
				std::vector<std::string> sd = itt;
				for (int i = 0; i < sd.size(); i++)
				{
					if (vTableStruct[i][2] == "INTEGER")
					{
						root[sTable.c_str()][(Json::ArrayIndex)iIndex][vTableStruct[i][1]] = atoi(sd[i].c_str());
					}
					else
					{
						root[sTable.c_str()][(Json::ArrayIndex)iIndex][vTableStruct[i][1]] = sd[i];
					}
				}
				iIndex++;
			}

			// Report Inward References
			result = m_sql.safe_query(std::string("PRAGMA foreign_key_list('" + sTable + "');").c_str());
			iIndex = 0;
			for (const auto& itt : result)
			{
				root["InwardReferences"][(Json::ArrayIndex)iIndex++] = itt[2].c_str();
			}

			// Report Outward References
			result = m_sql.safe_query(std::string("SELECT name FROM sqlite_master WHERE sql like '%%" + sTable + "ID%%' and name <> '" + sTable + "'").c_str());
			iIndex = 0;
			for (const auto& itt : result)
			{
				root["OutwardReferences"][(Json::ArrayIndex)iIndex++] = itt[0].c_str();
			}

			return reply::ok;
		}
	} //server
}//http
