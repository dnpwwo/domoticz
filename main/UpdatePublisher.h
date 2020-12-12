#pragma once

#include "../main/StoppableTask.h"
#ifdef WITH_EXTERNAL_SQLITE
#include <sqlite3.h>
#else
#include "../sqlite/sqlite3.h"
#endif
#include <boost/signals2.hpp>
//#include <boost/bind.hpp>

//
//	Dnpwwo, 2019
//

class CUpdateEntry
{
public:
	std::string					m_Action;
	std::string					m_Table;
	sqlite3_int64				m_RowIdx;
	std::vector<std::string>	m_Columns;
	std::vector<std::string>	m_ColumnTypes;
	std::vector<std::string>	m_Values;
	time_t						m_Time;

	CUpdateEntry(std::string Action, std::string Table, sqlite3_int64 RowIndex);
};

typedef	boost::signals2::signal<void(CUpdateEntry*)>	UpdatePublisher;

class CUpdateManager : public StoppableTask
{
private:
	sqlite3*	m_DB;

	std::shared_ptr<std::thread> m_thread;
	std::mutex m_mutex;

	void Do_Work();
public:
	CUpdateManager();
	~CUpdateManager(void);

	bool	Start(sqlite3*);
	bool	Stop();

	static void	c_callback(void*, int, char const*, char const*, sqlite3_int64);
	void		callback(int iAction, char const* pDB, char const* pTable, sqlite3_int64 iRowIdx);

	UpdatePublisher	Publisher;
};


