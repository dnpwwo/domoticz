#pragma once

#include <string>
#include "webserver/cWebem.h"
#include "webserver/request.hpp"
#include "webserver/session_store.hpp"

namespace Json
{
	class Value;
};

namespace http {
	namespace server {
		class cWebem;

		class CWebServer : public session_store, public std::enable_shared_from_this<CWebServer>
		{
			typedef boost::function< void(WebEmSession& session, const request& req, Json::Value& root) > webserver_response_function;
		public:
			CWebServer(void);
			~CWebServer(void);
			bool StartServer(server_settings& settings, const std::string& serverpath, const bool bIgnoreUsernamePassword);
			void StopServer();

			cWebem* m_pWebEm;

			void SetWebCompressionMode(const _eWebCompressionMode gzmode);
			void SetAuthenticationMethod(const _eAuthenticationMethod amethod);
			void SetWebTheme(const std::string& themename);
			void SetWebRoot(const std::string& webRoot);

			// SessionStore interface
			const WebEmStoredSession GetSession(const std::string& sessionId) override;
			void StoreSession(const WebEmStoredSession& session) override;
			void RemoveSession(const std::string& sessionId) override;
			void CleanSessions() override;
			void RemoveUsersSessions(const std::string& username, const WebEmSession& exceptSession);

			void HandleREST(const WebEmSession& session, const request& req, reply& rep);

		private:
			std::shared_ptr<std::thread> m_thread;

			void Do_Work();
			bool m_bDoStop;
			std::string m_server_alias;
		};
	}; //server
};//http
