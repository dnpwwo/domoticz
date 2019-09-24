#pragma once
#include "WebServer.h"
#ifndef NOCLOUD
#include "../hardware/DomoticzTCP.h"
#endif
#include "../tcpserver/TCPServer.h"
#include "../webserver/proxyclient.h"

namespace http {
	namespace server {
		//class CProxyManager;
		class CWebServerHelper {
		public:
			CWebServerHelper();
			~CWebServerHelper();

			// called from mainworker():
#ifdef WWW_ENABLE_SSL
			bool StartServers(server_settings & web_settings, ssl_server_settings & secure_web_settings, const std::string &serverpath, const bool bIgnoreUsernamePassword, tcp::server::CTCPServer *sharedServer);
#else
			bool StartServers(server_settings & web_settings, const std::string &serverpath, const bool bIgnoreUsernamePassword, tcp::server::CTCPServer *sharedServer);
#endif
			void StopServers();
#ifndef NOCLOUD
			void RestartProxy();
			CProxyClient *GetProxyForMaster(DomoticzTCP *master);
			void RemoveMaster(DomoticzTCP *master);
#endif
			void SetWebCompressionMode(const _eWebCompressionMode gzmode);
			void SetAuthenticationMethod(const _eAuthenticationMethod amethod);
			void SetWebTheme(const std::string &themename);
			void SetWebRoot(const std::string &webRoot);
			void ClearUserPasswords();
			// called from CSQLHelper
			void ReloadCustomSwitchIcons();
			std::string our_listener_port;
		private:
			std::shared_ptr<CWebServer> plainServer_;
#ifdef WWW_ENABLE_SSL
			std::shared_ptr<CWebServer> secureServer_;
#endif
			tcp::server::CTCPServer *m_pDomServ;
			std::vector<std::shared_ptr<CWebServer> > serverCollection;

			std::string our_serverpath;

#ifndef NOCLOUD
			CProxyManager proxymanager;
#endif
};

	} // end namespace server
} // end namespace http
