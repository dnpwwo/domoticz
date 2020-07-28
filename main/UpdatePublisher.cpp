#include "stdafx.h"

//
//	Dnpwwo, 2019
//

#include "../main/Helper.h"
#include "../main/Logger.h"
#include "../main/SQLHelper.h"
#include "../main/mainworker.h"
#include "../main/localtime_r.h"
#ifdef WIN32
#	include <direct.h>
#else
#	include <sys/stat.h>
#endif
#include <boost/thread.hpp>


std::mutex QueueMutex;	// controls accessto the message queue and m_pPlugins map
std::queue<CUpdateEntry*>	UpdateEntryQueue;

CUpdateManager::CUpdateManager()
{
}

CUpdateManager::~CUpdateManager(void)
{
}

bool CUpdateManager::Start(sqlite3* pDB)
{
	// Flush the message queue (should already be empty)
	std::lock_guard<std::mutex> l(QueueMutex);
	while (!UpdateEntryQueue.empty())
	{
		UpdateEntryQueue.pop();
	}

	m_thread = std::make_shared<std::thread>(&CUpdateManager::Do_Work, this);
	SetThreadName(m_thread->native_handle(), "Publisher");

	// Register the update callback
	m_DB = pDB;
	sqlite3_update_hook(m_DB, CUpdateManager::c_callback, this);

	try
	{
		_log.Log(LOG_STATUS, "UpdatePublisher: Started.");
	}
	catch (...) {
		_log.Log(LOG_ERROR, "UpdatePublisher: Failed to start.");
		return false;
	}

	return true;
}

bool CUpdateManager::Stop()
{
	if (m_thread)
	{
		RequestStop();
		m_thread->join();
		m_thread.reset();
	}

	// Hardware should already be stopped so just flush the queue (should already be empty)
	std::lock_guard<std::mutex> l(QueueMutex);
	while (!UpdateEntryQueue.empty())
	{
		CUpdateEntry* Message = UpdateEntryQueue.front();
		if (Message)
		{
			_log.Log(LOG_NORM, "(" + Message->m_Table + ") ' flushing " + std::to_string(Message->m_RowIdx) + "' queue entry");
		}
		UpdateEntryQueue.pop();
	}

	_log.Log(LOG_STATUS, "UpdatePublisher: Stopped.");
	return true;
}

void CUpdateManager::Do_Work()
{
	_log.Log(LOG_STATUS, "UpdatePublisher: Entering work loop.");

	// Look for messages 20 times per second
	while (!IsStopRequested(50))
	{
		try
		{
			if (!UpdateEntryQueue.empty())
			{
				// Dequeue update
				CUpdateEntry* pMessage = NULL;
				{
					std::lock_guard<std::mutex> l(QueueMutex);
					pMessage = UpdateEntryQueue.front();
					UpdateEntryQueue.pop();
				}

//				_log.Log(LOG_NORM, "UpdatePublisher: Publishing %s message for table %s.", pMessage->m_Action.c_str(), pMessage->m_Table.c_str());

				// Inform any subscribers
				Publisher(pMessage);

				// Free the memory
				if (pMessage) delete pMessage;
			}
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "UpdatePublisher: Exception processing message.");
		}
	}

	_log.Log(LOG_STATUS, "UpdatePublisher: Exiting work loop.");
}

void CUpdateManager::c_callback(void*	instance, int iAction, char const* pDB, char const* pTable, sqlite3_int64 iRowIdx)
{
	CUpdateManager* update = reinterpret_cast<CUpdateManager*>(instance);
	update->callback(iAction, pDB, pTable, iRowIdx);
}

void CUpdateManager::callback(int iAction, char const* pDB, char const* pTable, sqlite3_int64 iRowIdx)
{
	// Avoid publishing messages during backup
	if (pDB == "main")
	{
		CUpdateEntry* pEntry = NULL;
		switch (iAction)
		{
		case SQLITE_INSERT:
			pEntry = new CUpdateEntry("Insert", pTable, iRowIdx);
			break;
		case SQLITE_UPDATE:
			pEntry = new CUpdateEntry("Update", pTable, iRowIdx);
			break;
		case SQLITE_DELETE:
			pEntry = new CUpdateEntry("Delete", pTable, iRowIdx);
			break;
		default:
			_log.Log(LOG_ERROR, "[%s]: Unknown Action, ignored. Table: %s, Action: %d, Row Index: %ld.", __func__, pTable, iAction, iRowIdx);
			return;
		}

		// Load data if the row wasn't deleted (in case it changes again)
		if (iAction != SQLITE_DELETE)
		{
			std::stringstream	ss;
			ss << "SELECT * FROM " << pTable << " WHERE (ROWID=" << iRowIdx << ")";
			sqlite3_stmt* statement;
			if (sqlite3_prepare_v2(m_DB, ss.str().c_str(), -1, &statement, 0) == SQLITE_OK)
			{
				int cols = sqlite3_column_count(statement);
				while (true)
				{
					int result = sqlite3_step(statement);
					if (result == SQLITE_ROW)
					{
						for (int col = 0; col < cols; col++)
						{
							char* value = (char*)sqlite3_column_name(statement, col);
							pEntry->m_Columns.push_back(value);

							if (sqlite3_column_type(statement, col) == SQLITE_INTEGER)
							{
								pEntry->m_ColumnTypes.push_back("INTEGER");
							}
							else pEntry->m_ColumnTypes.push_back("TEXT");

							value = (char*)sqlite3_column_text(statement, col);
							if ((value == 0) && (col == 0))
								break;
							else if (value == 0)
								pEntry->m_Values.push_back(std::string("")); //insert empty string
							else
								pEntry->m_Values.push_back(value);
						}
					}
					else
					{
						break;
					}
				}
				sqlite3_finalize(statement);
			}
		}

		// Put it in the queue to be published
		std::lock_guard<std::mutex> l(QueueMutex);
		UpdateEntryQueue.push(pEntry);
	}
}

CUpdateEntry::CUpdateEntry(std::string Action, std::string Table, sqlite3_int64 RowIndex) : m_Action(Action), m_Table(Table), m_RowIdx(RowIndex)
{
	m_Time = time(0);
}
