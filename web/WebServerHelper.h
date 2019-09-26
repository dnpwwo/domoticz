#pragma once
#include "WebServer.h"

namespace http {
	namespace server {
		class CWebServerHelper {
		public:
			CWebServerHelper();
			~CWebServerHelper();

			// called from mainworker():
			bool StartServers();
			void StopServers();
			void SetWebCompressionMode(const _eWebCompressionMode gzmode);
			void SetAuthenticationMethod(const _eAuthenticationMethod amethod);
			void SetWebTheme(const std::string &themename);
			void SetWebRoot(const std::string &webRoot);
		private:
			static std::shared_ptr<CWebServer> plainServer_;
			static std::shared_ptr<CWebServer> secureServer_;
			static std::vector<std::shared_ptr<CWebServer> > serverCollection;
			static std::string our_listener_port;
			static std::string our_serverpath;
		};
	} // end namespace server
} // end namespace http
