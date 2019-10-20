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
#include "REST.h"

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
			std::string	sBlank = "";
			if (m_sql.GetPreferencesVar("WebRemoteProxyIPs", WebRemoteProxyIPs, sBlank) && WebRemoteProxyIPs.length())
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
			CRESTBase* pREST = CRESTBase::Create(session,  req, rep);
			// If a REST Object was not created then bail out
			if (!pREST)
			{
				rep = reply::stock_reply(reply::bad_request);
				return;
			}

			// Do some security
			if (!pREST->UserHasAccess())
			{
				rep = reply::stock_reply(reply::not_allowed);
				return;
			}

			// Action request
			pREST->ProcessRequest();

			// and tidy up
			delete pREST;

			return;
		}
	} //server
}//http
