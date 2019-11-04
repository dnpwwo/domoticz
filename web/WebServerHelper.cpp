#include "stdafx.h"
#include "../main/UpdatePublisher.h"
#include "WebServerHelper.h"
#include "../main/Logger.h"
#include "../main/SQLHelper.h"

namespace http {
	namespace server {

		typedef std::vector<std::shared_ptr<CWebServer> >::iterator server_iterator;

		std::shared_ptr<CWebServer> CWebServerHelper::plainServer_ = NULL;
		std::shared_ptr<CWebServer> CWebServerHelper::secureServer_ = NULL;
		std::vector<std::shared_ptr<CWebServer> > CWebServerHelper::serverCollection;
		std::string CWebServerHelper::our_listener_port = "8080";
		std::string CWebServerHelper::our_serverpath = "web/www/domoticz/src";

		CWebServerHelper::CWebServerHelper()
		{
		}

		CWebServerHelper::~CWebServerHelper()
		{
		}
		bool CWebServerHelper::StartServers(UpdatePublisher*	Publisher)
		{
			server_settings		web_settings;
			ssl_server_settings	secure_web_settings;
			bool bIgnoreUsernamePassword = false;
			bool bRet = false;

			// start plain webserver first
			std::string	sParam = "::";
			m_sql.GetPreferencesVar("WebserverListeningAddress", web_settings.listening_address, sParam);
			sParam = "8080";
			m_sql.GetPreferencesVar("WebserverNormalPort", web_settings.listening_port, sParam);
			sParam = "web/www";
			m_sql.GetPreferencesVar("WebserverWebRoot", our_serverpath, sParam);
			plainServer_.reset(new CWebServer());
			serverCollection.push_back(plainServer_);
			bRet |= plainServer_->StartServer(web_settings, our_serverpath, bIgnoreUsernamePassword);
			our_listener_port = web_settings.listening_port;

			// start secure webserver if the required preferences are set
			secure_web_settings.listening_address = web_settings.listening_address;
			sParam = "443";
			m_sql.GetPreferencesVar("WebserverSecurePort", secure_web_settings.listening_port, sParam);
			secure_web_settings.ssl_method = "sslv23";
			sParam = "./server_cert.pem";
			m_sql.GetPreferencesVar("WebserverSecureCertificate", secure_web_settings.certificate_chain_file_path, sParam);
			secure_web_settings.ca_cert_file_path = secure_web_settings.certificate_chain_file_path; // not used
			secure_web_settings.cert_file_path = secure_web_settings.certificate_chain_file_path;
			secure_web_settings.private_key_file_path = secure_web_settings.certificate_chain_file_path;
			secure_web_settings.private_key_pass_phrase = "";
			secure_web_settings.ssl_options = "default_workarounds,no_sslv2,no_sslv3,no_tlsv1,no_tlsv1_1,single_dh_use";
			secure_web_settings.tmp_dh_file_path = secure_web_settings.certificate_chain_file_path;
			secure_web_settings.verify_peer = false;
			secure_web_settings.verify_fail_if_no_peer_cert = false;
			secure_web_settings.verify_file_path = "";

			if (secure_web_settings.is_enabled()) {
#if (OPENSSL_VERSION_NUMBER < 0x10100000L) || defined(LIBRESSL_VERSION_NUMBER)
				SSL_library_init();
#endif
				secureServer_.reset(new CWebServer());
				bRet |= secureServer_->StartServer(secure_web_settings, our_serverpath, bIgnoreUsernamePassword);
				serverCollection.push_back(secureServer_);
			}

			return bRet;
		}

		void CWebServerHelper::StopServers()
		{
			for (server_iterator it = serverCollection.begin(); it != serverCollection.end(); ++it) {
				(*it)->StopServer();
			}
			serverCollection.clear();
			plainServer_.reset();
			secureServer_.reset();
		}

		void CWebServerHelper::SetWebCompressionMode(const _eWebCompressionMode gzmode)
		{
			for (server_iterator it = serverCollection.begin(); it != serverCollection.end(); ++it) {
				(*it)->SetWebCompressionMode(gzmode);
			}
		}

		void CWebServerHelper::SetAuthenticationMethod(const _eAuthenticationMethod amethod)
		{
			for (server_iterator it = serverCollection.begin(); it != serverCollection.end(); ++it) {
				(*it)->SetAuthenticationMethod(amethod);
			}
		}

		void CWebServerHelper::SetWebTheme(const std::string& themename)
		{
			for (server_iterator it = serverCollection.begin(); it != serverCollection.end(); ++it) {
				(*it)->SetWebTheme(themename);
			}
		}

		void CWebServerHelper::SetWebRoot(const std::string& webRoot)
		{
			for (server_iterator it = serverCollection.begin(); it != serverCollection.end(); ++it) {
				(*it)->SetWebRoot(webRoot);
			}
		}
	}
}
