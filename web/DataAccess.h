#pragma once

namespace http {
	namespace server {
		class CDataAccess
		{
		protected:
			std::string		m_User;
			std::string		m_Verb;
			std::string		m_Table;
			int				m_TableKey = 0;
			std::string		m_Parent;
			int				m_ParentKey = 0;

			std::string		m_Order;
			std::string		m_Filter;

			std::string		m_DontGETFields;
			std::string		m_PUTFields;
			std::string		m_PATCHFields;

			std::vector<std::vector<std::string> >		m_ValidFields;

		public:
			CDataAccess() {};
			CDataAccess(std::string User, std::string Table, std::string	Verb) : m_User(User), m_Table(Table), m_Verb(Verb) {};
			~CDataAccess() {};

			virtual	void	setTable(std::string sTable, int iTableKey);
			virtual	void	setParent(std::string sParent, int iParentKey);
			virtual	void	setOptions(std::string sOrder, std::string sFilter);

			virtual bool	UserHasAccess();

			bool			hasTimestamp();
			void			StripForbiddenFields();
			void			GetValidFields(std::vector<std::string>& vValidFields);
		};
	}
}