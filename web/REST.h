#pragma once

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
		class CRESTBase
		{
		private:
			WebEmSession&	m_Session;
			reply&			m_Reply;

			std::string		m_Verb;
			std::string		m_Table;
			int				m_TableKey = 0;
			std::string		m_Parent;
			int				m_ParentKey = 0;

			std::string		m_Order;
			std::string		m_Filter;

			std::string		m_PUTFields;
			std::string		m_PATCHFields;

		protected:
			std::vector<std::vector<std::string> >		m_GETFields;

			virtual void	POST();
			virtual void	GET();
			virtual void	PUT();
			virtual void	PATCH();
			virtual void	DELETE();
		public:
			CRESTBase(WebEmSession& session, reply& rep);
			~CRESTBase() {};

			virtual	void	SetVerb(std::string sVerb) { m_Verb = sVerb; };
			virtual	void	SetTable(std::string sTable, int iTableKey);
			virtual	void	SetParent(std::string sParent, int iParentKey);
			virtual	void	SetOptions(std::string sOrder, std::string sFilter);

			virtual bool	UserHasAccess();
			virtual	void	ProcessRequest();

			static CRESTBase* Create(WebEmSession& session, const request& req, reply& rep);
		};

		class CRESTUser : CRESTBase
		{
		public:
			CRESTUser(WebEmSession& session, reply& rep) : CRESTBase(session, rep) {};
		protected:
			virtual void	GET();
		};

		class CRESTUserSession : CRESTBase
		{
		public:
			CRESTUserSession(WebEmSession& session, reply& rep) : CRESTBase(session, rep) {};
		protected:
			virtual void	GET();
		};
	}
}

#pragma pop_macro("DELETE")
