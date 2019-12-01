#pragma once

#include "DataAccess.h"
#include "webserver/cWebem.h"
#include "webserver/request.hpp"
#include "webserver/session_store.hpp"
#include "../json/json.h"

//
//	REST API  - Dnpwwo, 2019
//

#pragma push_macro("DELETE")	// Why Windows !!??!  Why??
#undef DELETE

namespace http {
	namespace server {

		class CRESTRequest : public CDataAccess
		{
		private:
			const WebEmSession& m_Session;
			const request& m_Request;
			reply& m_Reply;
		protected:
			bool			getFieldsAndValues(std::vector<std::string>* pFields, std::vector<std::string>* pValues);
			virtual void	POST();
			virtual void	GET();
			virtual void	PUT();
			virtual void	PATCH();
			virtual void	DELETE();
		public:
			CRESTRequest(const WebEmSession& session, const request& req, reply& rep);
			virtual	void	ProcessRequest();

			static CRESTRequest* Create(const WebEmSession& session, const request& req, reply& rep);
		};

		class CRESTSession : CRESTRequest
		{
		public:
			CRESTSession(const WebEmSession& session, const request& req, reply& rep) : CRESTRequest(session, req, rep) {};
		protected:
			virtual void	GET();
		};
	}
}

#pragma pop_macro("DELETE")
