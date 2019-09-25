#pragma once

#include <string>
#include "Helper.h"
#include "StoppableTask.h"

#define timer_resolution_hz 25

struct sqlite3;

enum _eWindUnit
{
	WINDUNIT_MS=0,
	WINDUNIT_KMH,
	WINDUNIT_MPH,
	WINDUNIT_Knots,
	WINDUNIT_Beaufort,
};

enum _eTempUnit
{
	TEMPUNIT_C=0,
	TEMPUNIT_F,
};

enum _eWeightUnit
{
    WEIGHTUNIT_KG,
    WEIGHTUNIT_LB,
};

//row result for an sql query : string Vector
typedef   std::vector<std::string> TSqlRowQuery;

// result for an sql query : Vector of TSqlRowQuery
typedef   std::vector<TSqlRowQuery> TSqlQueryResult;

class CSQLHelper : public StoppableTask
{
public:
	CSQLHelper(void);
	~CSQLHelper(void);

	void SetDatabaseName(const std::string &DBName);

	bool OpenDatabase();
	void CloseDatabase();

	bool BackupDatabase(const std::string &OutputFile);
	bool RestoreDatabase(const std::string &dbase);

	//Returns DeviceRowID
	bool DoesDeviceExist(const int HardwareID, const char* ID, const unsigned char unit, const unsigned char devType, const unsigned char subType);

	void DeleteDataPoint(const char *ID, const std::string &Date);

	void UpdatePreferencesVar(const std::string &Key, const std::string &sValue);
	void UpdatePreferencesVar(const std::string &Key, const int nValue);
	void UpdatePreferencesVar(const std::string &Key, const int nValue, const std::string &sValue);
	bool GetPreferencesVar(const std::string &Key, int &nValue, std::string &sValue);
	bool GetPreferencesVar(const std::string &Key, int &nValue);
	bool GetPreferencesVar(const std::string &Key, std::string &sValue);

	int GetLastBackupNo(const char *Key, int &nValue);
	void SetLastBackupNo(const char *Key, const int nValue);

	void ScheduleShortlog();
	void ScheduleDay();

	void ClearShortLog();
	void VacuumDatabase();
	void OptimizeDatabase(sqlite3 *dbase);

	std::vector<std::vector<std::string> > safe_query(const char *fmt, ...);
	std::vector<std::vector<std::string> > safe_queryBlob(const char *fmt, ...);
	void safe_exec_no_return(const char *fmt, ...);
	bool safe_UpdateBlobInTableWithID(const std::string &Table, const std::string &Column, const std::string &sID, const std::string &BlobData);
	bool DoesColumnExistsInTable(const std::string &columnname, const std::string &tablename);

	bool GetPreferencesVar(const std::string &Key, double &Value);
	void UpdatePreferencesVar(const std::string &Key, const double Value);
	void DeletePreferencesVar(const std::string &Key);
	void AllowNewHardwareTimer(const int iTotMinutes);

	bool InsertCustomIconFromZip(const std::string &szZip, std::string &ErrorMessage);
	bool InsertCustomIconFromZipFile(const std::string & szZipFile, std::string & ErrorMessage);
public:
	bool		m_bAcceptNewHardware;
	bool		m_bAllowWidgetOrdering;
	int			m_ActiveTimerPlan;
private:
	std::mutex		m_sqlQueryMutex;
	sqlite3			*m_dbase;
	std::string		m_dbase_name;
	unsigned char	m_sensortimeoutcounter;
	std::map<uint64_t, int> m_timeoutlastsend;
	std::map<uint64_t, int> m_batterylowlastsend;
	bool			m_bAcceptHardwareTimerActive;
	float			m_iAcceptHardwareTimerCounter;
	bool			m_bPreviousAcceptNewHardware;

	std::shared_ptr<std::thread> m_thread;
	std::mutex m_background_task_mutex;
	bool StartThread();
	void StopThread();
	void Do_Work();

	void FixDaylightSavingTableSimple(const std::string &TableName);
	void FixDaylightSaving();

	void CleanupLightSceneLog();

	void CleanupShortLog();
	bool CheckDate(const std::string &sDate, int &d, int &m, int &y);
	bool CheckDateSQL(const std::string &sDate);
	bool CheckDateTimeSQL(const std::string &sDateTime);
	bool CheckTime(const std::string &sTime);

	std::vector<std::vector<std::string> > query(const std::string &szQuery);
	std::vector<std::vector<std::string> > queryBlob(const std::string &szQuery);
};

extern CSQLHelper m_sql;
