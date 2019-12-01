#include "stdafx.h"

#include "DataAccess.h"
#include "../main/SQLHelper.h"
#include "../main/Logger.h"
#include "../sqlite/sqlite3.h"

//
//	Data Access API  - Dnpwwo, 2019
//

namespace http {
	namespace server {

		void	CDataAccess::setTable(std::string sTable, int iTableKey)
		{
			m_Table = sTable;
			m_TableKey = iTableKey;
		};

		void	CDataAccess::setParent(std::string sParent, int iParentKey)
		{
			m_Parent = sParent;
			m_ParentKey = iParentKey;
		};

		void	CDataAccess::setOptions(std::string sOrder, std::string sFilter)
		{
			m_Order = sOrder;
			m_Filter = sFilter;
		}

		bool	CDataAccess::UserHasAccess()
		{
			// Check user has access to the table for the requested HTTP verb
			// TODO: get user ID from actual AccessToken Session
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT Can%q, PUTFields, PATCHFields, DontGETFields FROM TableAccess T, User U WHERE U.Username = '%q' AND U.RoleID = T.RoleID AND T.Name = '%q'", m_Verb.c_str(), m_User.c_str(), m_Table.c_str());
			if (!result.empty() && (result[0][0] == "1"))
			{
				m_PUTFields = result[0][1];
				m_PATCHFields = result[0][2];
				m_DontGETFields = result[0][3];

				// look up structure of requested table, by default all fields are eligible to be returned
				std::string		sSQL = "pragma table_info('" + m_Table + "');";
				m_ValidFields = m_sql.safe_query(sSQL.c_str());
				if (!m_ValidFields.empty())
				{
					return true;
				}
			}

			return false;
		}

		bool	CDataAccess::hasTimestamp()
		{
			// Search for timestamp field
			for (size_t i = 0; i < m_ValidFields.size(); i++)
			{
				if (m_ValidFields[i][1] == "Timestamp")
				{
					return true;
				}
			}
			return false;
		}

		void	CDataAccess::StripForbiddenFields()
		{
			if (m_Verb == "PUT")
			{
				for (size_t i = 0; i < m_ValidFields.size(); i++)
				{
					if (m_PUTFields.find(m_ValidFields[i][1]) == std::string::npos)
					{
						m_ValidFields.erase(m_ValidFields.begin() + i);
						i = 0; // Vector has changed so force loop through from the start again
					}
				}
			}
			else if (m_Verb == "PATCH")
			{
				for (size_t i = 0; i < m_ValidFields.size(); i++)
				{
					if (m_PATCHFields.find(m_ValidFields[i][1]) == std::string::npos)
					{
						m_ValidFields.erase(m_ValidFields.begin() + i);
						i = 0; // Vector has changed so force loop through from the start again
					}
				}
			}
			else
			{
				for (size_t i = 0; i < m_ValidFields.size(); i++)
				{
					if (m_DontGETFields.find(m_ValidFields[i][1]) != std::string::npos)
					{
						m_ValidFields.erase(m_ValidFields.begin() + i);
						i = 0; // Vector has changed so force loop through from the start again
					}
				}
			}
		}

		void CDataAccess::GetValidFields(std::vector<std::string>& vValidFields)
		{
			vValidFields.clear();
			for (size_t i = 0; i < m_ValidFields.size(); i++)
			{
				vValidFields.push_back(m_ValidFields[i][1]);
			}
		}

	}
}
