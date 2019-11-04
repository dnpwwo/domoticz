#include "stdafx.h"

#include "REST.h"
#include "../main/SQLHelper.h"
#include "../main/Logger.h"
#include "../sqlite/sqlite3.h"

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split

//
//	REST API  - Dnpwwo, 2019
//

#pragma push_macro("DELETE")
#undef DELETE
namespace http {
	namespace server {

		CRESTBase::CRESTBase(const WebEmSession& session, const request& req, reply& rep) : m_Session(session), m_Request(req), m_Reply(rep)
		{
		}

		void	CRESTBase::setTable(std::string sTable, int iTableKey)
		{
			m_Table = sTable;
			m_TableKey = iTableKey;
		};

		void	CRESTBase::setParent(std::string sParent, int iParentKey)
		{
			m_Parent = sParent;
			m_ParentKey = iParentKey;
		};

		void	CRESTBase::setOptions(std::string sOrder, std::string sFilter)
		{
			m_Order = sOrder;
			m_Filter = sFilter;
		}

		bool	CRESTBase::UserHasAccess()
		{
			// Check user has access to the table for the requested HTTP verb
			// TODO: get user ID from actual AccessToken Session
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT Can%q, PUTFields, PATCHFields FROM TableAccess T, User U WHERE U.Username = '%q' AND U.RoleID = T.RoleID AND T.Name = '%q'", this->getVerb().c_str(), "Admin", m_Table.c_str());
			if (!result.empty() && (result[0][0] == "1"))
			{
				m_PUTFields = result[0][1];
				m_PATCHFields = result[0][2];

				// look up structure of requested table, by default all fields are eligible to be returned
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
			if (getVerb() == "POST")
			{
				POST();
			}
			else if (getVerb() == "GET")
			{
				GET();
			}
			else if (getVerb() == "PUT")
			{
				PUT();
			}
			else if (getVerb() == "PATCH")
			{
				PATCH();
			}
			else if (getVerb() == "DELETE")
			{
				DELETE();
			}
			else m_Reply = reply::stock_reply(reply::not_allowed);

		}

		bool	CRESTBase::getFieldsAndValues(std::vector<std::string>* pFields, std::vector<std::string>* pValues)
		{
			try
			{
				Json::Value root;
				Json::Reader reader;
				// Convert string back to JSON
				if (!reader.parse(m_Request.content, root))
				{
					_log.Log(LOG_ERROR, "%s: Failed to parse JSON content for '%s'", __func__, getVerb().c_str());
					m_Reply = reply::stock_reply(reply::bad_request);
					return false;
				}

				// Iterate through supplied data and strings containing field names and values
				for (Json::Value::const_iterator it = root.begin(); it != root.end(); it++)
				{
					// Skip primary keys and timestamps for insert, these will be set by database
					if ((it.name() != m_Table + "ID") && (it.name() != "Timestamp"))
					{
						// Check field name is valid
						bool	bValid = false;
						for (const auto& iField : m_GETFields)
						{
							if (iField[1] == it.name())
							{
								bValid = true;
								break;
							}
						}

						// Skip if field is not expected/allowed (probably filtered by PUTFields/PATCHFields)
						if (!bValid)
						{
							continue;
						}

						// Handle various data types and convert them to string
						pFields->push_back(it.name());
						if (it->isString())
						{
							pValues->push_back(it->asCString());
						}
						else if (it->isInt())
						{
							pValues->push_back(std::to_string(it->asInt()));
						}
						else if (it->isBool())
						{
							pValues->push_back(std::to_string(it->asBool()));
						}
						else if (it->isDouble())
						{
							pValues->push_back(std::to_string(it->asDouble()));
						}
						else
						{
							_log.Log(LOG_ERROR, "%s: Unhandled type for field '%s'", __func__, it.name().c_str());
						}
					}
				}

				// Sanity check returned vectors
				if (pFields->size() != pValues->size())
				{
					_log.Log(LOG_ERROR, "%s: Fields (%d) vs Values (%d) mismatch.", __func__, pFields->size(), pValues->size());
					m_Reply = reply::stock_reply(reply::bad_request);
					return false;
				}

				// If there are no updatable fields then bail
				if (pFields->empty())
				{
					m_Reply = reply::stock_reply(reply::not_allowed);
					return false;
				}

				return true;
			}
			catch (std::exception& e)
			{
				_log.Log(LOG_ERROR, "%s: Execption thrown: %s", __func__, e.what());
			}

			m_Reply = reply::stock_reply(reply::bad_request);
			return false;
		}

		bool	CRESTBase::hasTimestamp()
		{
			// Search for timestamp field
			for (int i = 0; i < m_GETFields.size(); i++)
			{
				if (m_GETFields[i][1] == "Timestamp")
				{
					return true;
				}
			}
			return false;
		}

		bool	CRESTBase::stripForbiddenFields(std::string	ValidFields)
		{
			for (int i = 0; i < m_GETFields.size(); i++)
			{
				if (ValidFields.find(m_GETFields[i][1]) == std::string::npos)
				{
					m_GETFields.erase(m_GETFields.begin()+i);
					return true;
				}
			}
			return false;
		}

		void	CRESTBase::POST()
		{
			// Sanity check data
			if (!m_Request.content_length)
			{
				return;
			}

			// Unpack request and match to database schema
			std::vector<std::string> vFields;
			std::vector<std::string> vValues;
			if (!getFieldsAndValues(&vFields, &vValues))
			{
				return;
			}

			// Build SQL statement
			std::string		sSQL = "INSERT INTO " + m_Table + " (";
			for (int i = 0; i < vFields.size(); i++)
			{
				sSQL += vFields[i];
				if (i < vFields.size() - 1) sSQL += ", ";
			}
			sSQL += ") VALUES (";
			for (int i = 0; i < vValues.size(); i++)
			{
				sSQL += "?";
				if (i < vValues.size() - 1) sSQL += ", ";
			}
			sSQL += ");";

			// Execute the statement
			int		iRowCount = m_sql.execute_sql(sSQL, &vValues, true);
			if (!iRowCount)
			{
				m_Reply = reply::stock_reply(reply::bad_request);
				_log.Log(LOG_ERROR, "Insert into '%s' failed to create a record.", m_Table.c_str());
			}
			else
			{
				m_Reply = reply::stock_reply(reply::created);
				_log.Log(LOG_NORM, "Insert into '%s' succeeded, %d records created.", m_Table.c_str(), iRowCount);
			}

			// Add to the response
			Json::Value			root;
			Json::FastWriter	jWriter;
			root["Count"] = iRowCount;
			m_Reply.content = jWriter.write(root);
		}

		void	CRESTBase::GET()
		{
			// Sanity check data
			if (m_Request.content_length)
			{
				_log.Log(LOG_ERROR, "%s: Unexpected Non-Zero content length (%d) for '%s'", __func__, m_Request.content_length, getVerb().c_str());
				m_Reply = reply::stock_reply(reply::bad_request);
				return;
			}

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
			}
			else
			{
				m_Reply = reply::stock_reply(reply::no_content);
			}

			Json::Value root;
			root["Count"] = std::to_string(result.size());
			int		iIndex = 0;
			std::string	sOutputName = m_Table;
			if (!m_TableKey)
			{
				sOutputName += "s";		// If this is not a request for a specific key then return '<TableName>s'
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

		void	CRESTBase::PUT()
		{
			// Sanity check data
			if (!m_Request.content_length)
			{
				return;
			}

			// Remember if the table has a timestamp (for later)
			bool	bHasTimestamp = hasTimestamp();

			// Respect the 'PUT Fields' configured for the table
			if (m_PUTFields != "*")
			{
				while (stripForbiddenFields(m_PUTFields));
			}

			// Unpack request and match to database schema
			std::vector<std::string> vFields;
			std::vector<std::string> vValues;
			if (!getFieldsAndValues(&vFields, &vValues))
			{
				return;
			}

			// Build SQL statement
			std::string		sSQL = "UPDATE " + m_Table + " SET ";
			for (int i = 0; i < vFields.size(); i++)
			{
				sSQL += vFields[i] + "=?";
				if (i < vFields.size() - 1) sSQL += ", ";
			}

			// if the table has a timestamp field then update it to 'now'
			if (bHasTimestamp)
			{
				sSQL += ", Timestamp=CURRENT_TIMESTAMP";
			}

			// Add the where clause to update the correct row (use REST API URL number)
			sSQL += " WHERE " + m_Table + "ID=" + std::to_string(m_TableKey) + "; ";

			// Execute the statement
			int		iRowCount = m_sql.execute_sql(sSQL, &vValues, true);
			if (!iRowCount)
			{
				m_Reply = reply::stock_reply(reply::bad_request);
				_log.Log(LOG_ERROR, "Update of '%s' failed to modify a record.", m_Table.c_str());
			}
			else
			{
				m_Reply = reply::stock_reply(reply::created);
				_log.Log(LOG_NORM, "Update of '%s' succeeded, %d records modified.", m_Table.c_str(), iRowCount);
			}

			// Add to the response
			Json::Value			root;
			Json::FastWriter	jWriter;
			root["Count"] = iRowCount;
			m_Reply.content = jWriter.write(root);
		}

		void	CRESTBase::PATCH()
		{
			// Sanity check data
			if (!m_Request.content_length)
			{
				return;
			}

			// Remember if the table has a timestamp (for later)
			bool	bHasTimestamp = hasTimestamp();

			// Respect the 'PUT Fields' configured for the table
			if (m_PUTFields != "*")
			{
				while (stripForbiddenFields(m_PATCHFields));
			}

			// Unpack request and match to database schema
			std::vector<std::string> vFields;
			std::vector<std::string> vValues;
			if (!getFieldsAndValues(&vFields, &vValues))
			{
				return;
			}

			// Build SQL statement
			std::string		sSQL = "UPDATE " + m_Table + " SET ";
			for (int i = 0; i < vFields.size(); i++)
			{
				sSQL += vFields[i] + "=?";
				if (i < vFields.size() - 1) sSQL += ", ";
			}

			// if the table has a timestamp field then update it to 'now'
			if (bHasTimestamp)
			{
				sSQL += ", Timestamp=CURRENT_TIMESTAMP";
			}

			// Add the where clause to update the correct row (use REST API URL number)
			sSQL += " WHERE " + m_Table + "ID=" + std::to_string(m_TableKey) + "; ";

			// Execute the statement
			int		iRowCount = m_sql.execute_sql(sSQL, &vValues, true);
			if (!iRowCount)
			{
				m_Reply = reply::stock_reply(reply::bad_request);
				_log.Log(LOG_ERROR, "Patch of '%s' failed to update a record.", m_Table.c_str());
			}
			else
			{
				m_Reply = reply::stock_reply(reply::created);
				_log.Log(LOG_NORM, "Patch of '%s' succeeded, %d records updated.", m_Table.c_str(), iRowCount);
			}

			// Add to the response
			Json::Value			root;
			Json::FastWriter	jWriter;
			root["Count"] = iRowCount;
			m_Reply.content = jWriter.write(root);
		}

		void	CRESTBase::DELETE()
		{
			// Sanity check data
			if (m_Request.content_length)
			{
				_log.Log(LOG_ERROR, "%s: Unexpected Non-Zero content length (%d) for '%s'", __func__, m_Request.content_length, getVerb().c_str());
				m_Reply = reply::stock_reply(reply::bad_request);
				return;
			}

			// Build SQL statement
			std::string		sSQL = "DELETE  FROM " + m_Table + " WHERE " + m_Table + "ID = " + std::to_string(m_TableKey) + ";";
			m_sql.safe_query(sSQL.c_str());
			std::vector<std::vector<std::string> >	result = m_sql.safe_query("SELECT changes();");

			// Handle any data we get back
			Json::Value root;
			if (result.empty())
			{
				m_Reply = reply::stock_reply(reply::not_found);
				root["Count"] = 0;
				_log.Log(LOG_ERROR, "Delete from '%s' failed to delete any records for ID %d", m_Table.c_str(), m_TableKey);
			}
			else
			{
				m_Reply = reply::stock_reply(reply::no_content);
				root["Count"] = atoi(result[0][0].c_str());
				_log.Log(LOG_NORM, "Delete from '%s' succeeded, %s records removed.", m_Table.c_str(), result[0][0].c_str());
			}

			// Add to the response
			Json::FastWriter	jWriter;
			m_Reply.content = jWriter.write(root);
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

		void	CRESTSession::GET()
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

		CRESTBase* CRESTBase::Create(const WebEmSession& session, const request& req, reply& rep)
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
						pREST = (CRESTBase*) new CRESTUser(session, req, rep);
					}
					else if (sTable == "Session")
					{
						pREST = (CRESTBase*) new CRESTSession(session, req, rep);
					}
					else
					{
						pREST = new CRESTBase(session, req, rep);
					}

					// Initialise object's data
					pREST->setTable(sTable, iTableKey);
					pREST->setParent(sParent, iParentKey);
					pREST->setOptions(sOrder, sFilter);
				}
			}

			return pREST;
		}

	}
}
#pragma pop_macro("DELETE")
