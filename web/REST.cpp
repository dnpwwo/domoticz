#include "stdafx.h"

#include "REST.h"
#include "../main/SQLHelper.h"
#include "../main/Logger.h"

//
//	REST API  - Dnpwwo, 2019
//

#pragma push_macro("DELETE")
#undef DELETE
namespace http {
	namespace server {

		CRESTBase::CRESTBase(WebEmSession& session, reply& rep) : m_Reply(rep), m_Session(session)
		{
		}

		void	CRESTBase::SetTable(std::string sTable, int iTableKey)
		{
			m_Table = sTable;
			m_TableKey = iTableKey;
		};

		void	CRESTBase::SetParent(std::string sParent, int iParentKey)
		{
			m_Parent = sParent;
			m_ParentKey = iParentKey;
		};

		void	CRESTBase::SetOptions(std::string sOrder, std::string sFilter)
		{
			m_Order = sOrder;
			m_Filter = sFilter;
		}

		bool	CRESTBase::UserHasAccess()
		{
			// Check user has access to the table for the requested HTTP verb
			// TODO: get user ID from actual AccessToken Session
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT Can%q, PUTFields, PATCHFields FROM TableAccess T, User U WHERE U.Username = '%q' AND U.RoleID = T.RoleID AND T.Name = '%q'", m_Verb.c_str(), "Admin", m_Table.c_str());
			if (!result.empty() && (result[0][0] == "1"))
			{
				m_PUTFields = result[0][1];
				m_PATCHFields = result[0][2];

				// look up structure of requested table, by default all fields are elidgable to be returned
				std::string		sSQL = "pragma table_info('" + m_Table + "');";
				m_GETFields = m_sql.safe_query(sSQL.c_str());
				if (!m_GETFields.empty())
				{
					return true;
				}
			}

			return false;
		}

		void	CRESTBase::ProcessRequest()
		{
			// Handle various HTTP verbs
			if (m_Verb == "POST")
			{
				POST();
			}
			else if (m_Verb == "GET")
			{
				GET();
			}
			else if (m_Verb == "PUT")
			{
				PUT();
			}
			else if (m_Verb == "PATCH")
			{
				PATCH();
			}
			else if (m_Verb == "DELETE")
			{
				DELETE();
			}
			else m_Reply = reply::stock_reply(reply::bad_request);

		}

		void	CRESTBase::POST()
		{
			Json::Value root;
		}

		void	CRESTBase::GET()
		{
			// Build SQL query
			std::string		sSQL = "SELECT ";
			for (int i = 0; i < m_GETFields.size(); i++)
			{
				sSQL += "T." + m_GETFields[i][1];
				if (i != m_GETFields.size() - 1) sSQL += ",";
			}
			sSQL += " FROM " + m_Table + " T ";

			if (m_TableKey)
			{	// Case 2
				sSQL += " WHERE T." + m_Table + "ID=" + std::to_string(m_TableKey);
			}
			if (m_ParentKey)
			{	// Case 3a - We can't be sure which way around the keys go
				std::string		testSQL = sSQL + ", " + m_Parent + " P WHERE P." + m_Parent + "ID=" + std::to_string(m_ParentKey) + " AND P." + m_Parent + "ID=T." + m_Parent + "ID";
				std::vector<std::vector<std::string> >	test = m_sql.safe_query(testSQL.c_str());
				if (test.empty())
				{	// Case 3b - the other way around
					sSQL += ", " + m_Parent + " P WHERE P." + m_Parent + "ID=" + std::to_string(m_ParentKey) + " AND P." + m_Table + "ID=T." + m_Table + "ID";
				}
				else
				{
					sSQL = testSQL;
				}
			}
			std::vector<std::vector<std::string> >	result = m_sql.safe_query(sSQL.c_str());

			// Handle any data we get back
			if (!result.empty())
			{
				m_Reply = reply::stock_reply(reply::ok);
				Json::Value root;

				root["Count"] = std::to_string(result.size());
				int		iIndex = 0;
				std::string	sOutputName = m_Table;
				if (!m_TableKey)
				{
					sOutputName += "s";		// If this is not a a request for a specific key then return '<TableName>s'
				}
				for (const auto& itt : result)
				{
					std::vector<std::string> sd = itt;
					for (int i = 0; i < sd.size(); i++)
					{
						if (m_GETFields[i][2] == "INTEGER")
						{
							root[sOutputName.c_str()][(Json::ArrayIndex)iIndex][m_GETFields[i][1]] = atoi(sd[i].c_str());
						}
						else
						{
							root[sOutputName.c_str()][(Json::ArrayIndex)iIndex][m_GETFields[i][1]] = sd[i];
						}
					}
					iIndex++;
				}

				// Add to the response
				Json::FastWriter	jWriter;
				m_Reply.content = jWriter.write(root);
			}
			else
			{
				m_Reply = reply::stock_reply(reply::no_content);
			}
		}

		void	CRESTBase::PUT()
		{
			Json::Value root;
		}

		void	CRESTBase::PATCH()
		{
			Json::Value root;
		}

		void	CRESTBase::DELETE()
		{
			Json::Value root;
		}

		void	CRESTUser::GET()
		{
			// Remove the Password field
			for (int i = 0; i < m_GETFields.size(); i++)
			{
				if (m_GETFields[i][1] == "Password")
				{
					m_GETFields.erase(m_GETFields.begin() + i);
					break;
				}
			}

			// Now invoke base GET
			CRESTBase::GET();
		}

		void	CRESTUserSession::GET()
		{
			// Remove the Password field
			for (int i = 0; i < m_GETFields.size(); i++)
			{
				if (m_GETFields[i][1] == "AuthToken")
				{
					m_GETFields.erase(m_GETFields.begin() + i);
					break;
				}
			}

			// Now invoke base GET
			CRESTBase::GET();
		}

		CRESTBase* CRESTBase::Create(WebEmSession& session, const request& req, reply& rep)
		{
			CRESTBase* pREST = NULL;
			std::string	sURI = req.uri;
			std::string	sOrder;
			std::string	sFilter;

			// Handle query string - TODO: Order - Field or direction, Filter Field name or value???
			if (sURI.find('&'))
			{
				std::string::size_type pos = sURI.find("&order=");
				if (pos != std::string::npos)
				{
					sOrder = sURI.substr(pos+7, sURI.find('&', pos+1) - (pos+7));
				}

				pos = sURI.find("&filter=");
				if (pos != std::string::npos)
				{
					sFilter = sURI.substr(pos+8, sURI.find('&', pos+1) - (pos+8));
				}

				// Strip query string out of URI
				sURI = sURI.substr(0, sURI.find('&'));
			}

			// Break URI into parts
			std::vector<std::string> vURIElements;
			if (sURI.find("/", 5))
			{
				std::string::size_type prev_pos = 5, pos = 5; // step over '/API/'
				std::string substring;
				while ((pos = sURI.find("/", pos)) != std::string::npos)
				{
					substring = sURI.substr(prev_pos, pos - prev_pos);
					if (substring.length()) vURIElements.push_back(substring);
					prev_pos = ++pos;
				}
				substring = sURI.substr(prev_pos, pos - prev_pos);
				if (substring.length()) vURIElements.push_back(substring);
			}

			// Does this look like a REST request?
			if (!vURIElements.empty())
			{
				// Confirm which part of the URI we are operating on
				// Need one of three cases:
				//	1.	/API/Interfaces					== Request for summary of a whole table (No Keys)
				//	2.	/API/Interfaces/123				== Request for details of specific instance from a table (TableKey only)
				//	3.	/API/Interfaces/123/Devices		== Request for summary of table filterd for a specific parent instance (ParentKey)
				//	4.	/API/Interfaces/123/Devices/1	== THIS SHOULD BE HANDLED TO MAKE SURE DATA RETURNED IS RELEVANT TO THE PARENT
				std::string	sTable;
				int			iTableKey = 0;
				std::string	sParent;
				int			iParentKey = 0;
				int			iTable = vURIElements.size();
				// Target table is the rightmost non-numeric URI element 
				while (iTable)
				{
					char* p = NULL;
					int		iNumber = strtol(vURIElements[--iTable].c_str(), &p, 10);
					if (*p)
					{
						sTable = p;
						break;
					}
				}
				// Trim off trailing 's' of table name
				if (sTable.length())
				{
					sTable = sTable.substr(0, sTable.length() - 1);
				}

				// if there is a URI element to the right look for the TableKey (Case 2)
				if ((iTable + 1) < vURIElements.size())
				{
					char* p = NULL;
					iTableKey = strtol(vURIElements[iTable + 1].c_str(), &p, 10);
				}

				// if there are URI elements to the left look for the ParentKey and Table (Case 3)
				if ((iTable > 1) && !iTableKey)
				{
					char* p = NULL;
					iParentKey = strtol(vURIElements[iTable - 1].c_str(), &p, 10);
					sParent = vURIElements[iTable - 2];
					if (sParent.length())
					{
						sParent = sParent.substr(0, sParent.length() - 1);
					}
				}

				// Does this look like a valid request?
				if ((!iTable && sTable.length() && !iTableKey && !iParentKey) ||			// Case 1
					(sTable.length() && iTableKey && !iParentKey) ||						// Case 2
					(sTable.length() && !iTableKey && iParentKey && sParent.length())) 		// Case 3
				{
					// Handle special cases, otherwise return base class
					if (sTable == "User")
					{
						pREST = (CRESTBase*) new CRESTUser(session, rep);
					}
					else if (sTable == "UserSession")
					{
						pREST = (CRESTBase*) new CRESTUserSession(session, rep);
					}
					else
					{
						pREST = new CRESTBase(session, rep);
					}

					// Initialise object's data
					pREST->SetVerb(req.method);
					pREST->SetTable(sTable, iTableKey);
					pREST->SetParent(sParent, iParentKey);
					pREST->SetOptions(sOrder, sFilter);
				}
			}

			return pREST;
		}

	}
}
#pragma pop_macro("DELETE")
