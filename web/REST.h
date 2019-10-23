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
			const WebEmSession&	m_Session;
			const request&		m_Request;
			reply&				m_Reply;

			std::string		m_Table;
			int				m_TableKey = 0;
			std::string		m_Parent;
			int				m_ParentKey = 0;

			std::string		m_Order;
			std::string		m_Filter;

			std::string		m_PUTFields;
			std::string		m_PATCHFields;

			std::string		getVerb() { return m_Request.method; };
			bool			getFieldsAndValues(std::string*	pFields, std::string*	pValues);
			bool			hasTimestamp();
			bool			stripForbiddenFields(std::string	ValidFields);

		protected:
			std::vector<std::vector<std::string> >		m_GETFields;

			virtual void	POST();
			virtual void	GET();
			virtual void	PUT();
			virtual void	PATCH();
			virtual void	DELETE();
		public:
			CRESTBase(const WebEmSession& session, const request& req, reply& rep);
			~CRESTBase() {};

			virtual	void	setTable(std::string sTable, int iTableKey);
			virtual	void	setParent(std::string sParent, int iParentKey);
			virtual	void	setOptions(std::string sOrder, std::string sFilter);

			virtual bool	UserHasAccess();
			virtual	void	ProcessRequest();

			static CRESTBase* Create(const WebEmSession& session, const request& req, reply& rep);
		};

		class CRESTUser : CRESTBase
		{
		public:
			CRESTUser(const WebEmSession& session, const request& req, reply& rep) : CRESTBase(session, req, rep) {};
		protected:
			virtual void	GET();
		};

		class CRESTUserSession : CRESTBase
		{
		public:
			CRESTUserSession(const WebEmSession& session, const request& req, reply& rep) : CRESTBase(session, req, rep) {};
		protected:
			virtual void	GET();
		};
	}
}

#pragma pop_macro("DELETE")
