#include "stdafx.h"
#include "SQLHelper.h"
#include <iostream>     /* standard I/O functions                         */
#include <iomanip>
#include <boost/exception/diagnostic_information.hpp>
#include "localtime_r.h"
#include "Logger.h"
#include "mainworker.h"
#ifdef WITH_EXTERNAL_SQLITE
#include <sqlite3.h>
#else
#include "../sqlite/sqlite3.h"
#endif
#include "clx_unzip.h"
#ifdef ENABLE_PYTHON
#include "../hardware/plugins/Plugins.h"
#endif

#ifndef WIN32
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#else
#include "../msbuild/WindowsHelper.h"
#endif
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define DB_VERSION 137

extern std::string szWWWFolder;

const char* sqlCreatePerferences =
		"CREATE TABLE[Preferences] ("
			"[Key] VARCHAR(50) NOT NULL,"
			"[nValue] INTEGER DEFAULT 0,"
			"[sValue] VARCHAR(200))";

const char* sqlCreateInterface =
		"CREATE TABLE IF NOT EXISTS [Interface] ("
			"[InterfaceID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE DEFAULT Unknown, "
			"[Configuration] TEXT DEFAULT NULL, "
			"[Active] INTEGER DEFAULT 0);";

const char* sqlCreateInterfaceLog =
		"CREATE TABLE IF NOT EXISTS [InterfaceLog] ("
			"[InterfaceID] INTEGER, "
			"[Message] TEXT DEFAULT \"\","
			"[Timestamp] TEXT DEFAULT CURRENT_TIMESTAMP,"
		"FOREIGN KEY(InterfaceID) REFERENCES Interface(InterfaceID) ON DELETE CASCADE);";

const char* sqlCreateInterfaceDevice =
		"CREATE TABLE IF NOT EXISTS [InterfaceDevice] ("
			"[DeviceID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[InterfaceID] INTEGER, "
			"[Name] TEXT DEFAULT Unknown, "
			"[ExternalID] TEXT DEFAULT \"\", "
			"[Timestamp] TEXT DEFAULT CURRENT_TIMESTAMP,"
		"FOREIGN KEY(InterfaceID) REFERENCES Interface(InterfaceID) ON DELETE CASCADE);";

const char* sqlCreateValueUnit =
	"CREATE TABLE IF NOT EXISTS [ValueUnit] ("
			"[UnitID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE DEFAULT \"\", "
			"[Minimum] INTEGER DEFAULT 0, "
			"[Maximum] INTEGER DEFAULT 0, "
			"[IconList] TEXT DEFAULT \"\", "
			"[TextLabels] TEXT DEFAULT \"\");";

const char* sqlCreateDeviceValue =
	"CREATE TABLE IF NOT EXISTS [DeviceValue] ("
			"[ValueID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT DEFAULT \"\", "
			"[DeviceID] INTEGER, "
			"[UnitID] INTEGER, "
			"[Value] TEXT DEFAULT \"\", "
			"[RetensionDays] INTEGER DEFAULT -1, "
			"[RetensionInterval] INTEGER DEFAULT 900, "
			"[Timestamp] TEXT DEFAULT CURRENT_TIMESTAMP, "
		"FOREIGN KEY(UnitID) REFERENCES ValueUnit(UnitID), "
		"FOREIGN KEY(DeviceID) REFERENCES InterfaceDevice(DeviceID) ON DELETE CASCADE);";

const char* sqlCreateValueLog =
	"CREATE TABLE IF NOT EXISTS [ValueLog] ("
			"[ValueID] INTEGER, "
			"[Message] TEXT DEFAULT \"\","
			"[Timestamp] TEXT DEFAULT CURRENT_TIMESTAMP, "
		"FOREIGN KEY(ValueID) REFERENCES DeviceValue(ValueID) ON DELETE CASCADE);";

const char* sqlCreateValueHistory =
	"CREATE TABLE IF NOT EXISTS [ValueHistory] ("
			"[ValueID] INTEGER, "
			"[Value] TEXT DEFAULT \"\", "
			"[Timestamp] TEXT DEFAULT CURRENT_TIMESTAMP, "
		"FOREIGN KEY(ValueID) REFERENCES DeviceValue(ValueID) ON DELETE CASCADE);";

const char* sqlCreateValueScripts =
	"CREATE TABLE IF NOT EXISTS [ValueScripts] ("
			"[ValueID] INTEGER, "
			"[UpdateScript] TEXT DEFAULT NULL, "
			"[PeriodicScript] TEXT DEFAULT NULL, "
		"FOREIGN KEY(ValueID) REFERENCES DeviceValue(ValueID) ON DELETE CASCADE);";

const char* sqlCreateTimerPlan =
	"CREATE TABLE IF NOT EXISTS [TimerPlan] ("
			"[TimerPlanID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE DEFAULT Unknown, "
			"[Active] INTEGER DEFAULT 0);";

const char* sqlCreateValueTimer =
	"CREATE TABLE IF NOT EXISTS [ValueTimer] ("
			"[TimerPlanID] INTEGER, "
			"[ValueID] INTEGER, "
			"[DayMask] TEXT DEFAULT \"Mon,Tue,Wed,Thu,Fri,Sat,Sun\", "
			"[Time] TEXT DEFAULT \"00:00:00\", "
			"[Sunrise] INTEGER DEFAULT 0, "
			"[Sunset] INTEGER DEFAULT 0, "
			"[Random] INTEGER DEFAULT 0, "
		"FOREIGN KEY(TimerPlanID) REFERENCES TimerPlan(TimerPlanID), "
		"FOREIGN KEY(ValueID) REFERENCES DeviceValue(ValueID) ON DELETE CASCADE);";

const char* sqlCreateUserRole =
	"CREATE TABLE IF NOT EXISTS [UserRole] ("
			"[RoleID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE DEFAULT \"\");";

const char* sqlCreateUser =
	"CREATE TABLE IF NOT EXISTS [User] ("
			"[UserID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[UserName] TEXT UNIQUE DEFAULT Unknown, "
			"[Password] TEXT DEFAULT Unknown, "
			"[Name] TEXT DEFAULT Unknown, "
			"[RoleID] INTEGER, "
			"[Active] INTEGER DEFAULT 0, "
			"[ForceChange] INTEGER DEFAULT 1, "
			"[FailedAttempts] INTEGER DEFAULT 0, "
			"[Timestamp] TEXT DEFAULT 0, "
		"FOREIGN KEY(RoleID) REFERENCES UserRole(RoleID) ON DELETE CASCADE);";

const char* sqlCreateUserSession =
	"CREATE TABLE IF NOT EXISTS [UserSession] ("
			"[UserID] INTEGER, "
			"[RoleID] INTEGER, "
			"[AccessToken] TEXT DEFAULT \"\", "
			"[Expiry] TEXT DEFAULT 0, "
		"FOREIGN KEY(UserID) REFERENCES User(UserID) ON DELETE CASCADE, "
		"FOREIGN KEY(RoleID) REFERENCES UserRole(RoleID) ON DELETE CASCADE);";

const char* sqlCreateRESTPrivilege =
	"CREATE TABLE IF NOT EXISTS [RESTPrivilege] ("
			"[TableName] TEXT, "
			"[RoleID] INTEGER, "
			"[CanGET] TEXT DEFAULT 0, "
			"[CanPOST] TEXT DEFAULT 0, "
			"[CanPUT] TEXT DEFAULT 0, "
			"[CanPATCH] TEXT DEFAULT 0, "
			"[CanDELETE] TEXT DEFAULT 0, "
			"[PUTFields] TEXT DEFAULT \"*\", "
			"[PATCHFields] TEXT DEFAULT \"*\", "
		"FOREIGN KEY(RoleID) REFERENCES UserRole(RoleID) ON DELETE CASCADE);";

extern std::string szUserDataFolder;

CSQLHelper::CSQLHelper(void)
{
	m_dbase = NULL;
	m_sensortimeoutcounter = 0;
	m_bAcceptNewHardware = true;
	m_bAllowWidgetOrdering = true;
	m_ActiveTimerPlan = 0;
	m_bAcceptHardwareTimerActive = false;
	m_iAcceptHardwareTimerCounter = 0;
	m_bPreviousAcceptNewHardware = false;

	SetDatabaseName("domoticz.db");
}

CSQLHelper::~CSQLHelper(void)
{
	StopThread();
	CloseDatabase();
}

bool CSQLHelper::OpenDatabase()
{
	//Open Database
	int rc = sqlite3_open(m_dbase_name.c_str(), &m_dbase);
	if (rc)
	{
		_log.Log(LOG_ERROR, "Error opening SQLite3 database: %s", sqlite3_errmsg(m_dbase));
		sqlite3_close(m_dbase);
		return false;
	}
#ifndef WIN32
	//test, this could improve performance
	sqlite3_exec(m_dbase, "PRAGMA synchronous = NORMAL", NULL, NULL, NULL);
	sqlite3_exec(m_dbase, "PRAGMA journal_mode = WAL", NULL, NULL, NULL);
#else
	sqlite3_exec(m_dbase, "PRAGMA journal_mode=DELETE", NULL, NULL, NULL);
#endif
	sqlite3_exec(m_dbase, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
	std::vector<std::vector<std::string> > result = query("SELECT name FROM sqlite_master WHERE type='table' AND name='Interface'");
	bool bNewInstall = (result.size() == 0);
	int dbversion = 0;
	if (!bNewInstall)
	{
		GetPreferencesVar("DB_Version", dbversion);
		if (dbversion > DB_VERSION)
		{
			//User is using a newer database on a old Domoticz version
			//This is very dangerous and should not be allowed
			_log.Log(LOG_ERROR, "Database incompatible with this Domoticz version. (You cannot downgrade to an old Domoticz version!)");
			sqlite3_close(m_dbase);
			m_dbase = NULL;
			return false;
		}
		//Pre-SQL Patches
	}

	//create database (if not exists)
	query(sqlCreatePerferences);
	query(sqlCreateInterface);
	query(sqlCreateInterfaceLog);
	query(sqlCreateInterfaceDevice);
	query(sqlCreateValueUnit);
	query(sqlCreateDeviceValue);
	query(sqlCreateValueHistory);
	query(sqlCreateValueLog);
	query(sqlCreateValueHistory);
	query(sqlCreateValueScripts);
	query(sqlCreateTimerPlan);
	query(sqlCreateValueTimer);
	query(sqlCreateUserRole);
	query(sqlCreateUser);
	query(sqlCreateUserSession);
	query(sqlCreateRESTPrivilege);

	//Add indexes to log tables

	if ((!bNewInstall) && (dbversion < DB_VERSION))
	{
		//Post-SQL Patches
		if (dbversion < 2)
		{
			//query("ALTER TABLE DeviceStatus ADD COLUMN [Order] INTEGER BIGINT(10) default 0");
		}
	} 
	else if (bNewInstall)
	{
		// User / Security related
		query("INSERT INTO UserRole (Name) VALUES ('Anonymous')");
		query("INSERT INTO UserRole (Name) VALUES ('User')");
		query("INSERT INTO UserRole (Name) VALUES ('Administrator')");

		// Add a couple of users
		query("INSERT INTO User (Username, Password, Name, RoleID) SELECT 'Anonymous','Anonymous','Anonymous User', RoleID FROM UserRole WHERE Name = 'Anonymous'");
		query("INSERT INTO User (Username, Password, Name, RoleID) SELECT 'Admin','Admin ','Administrative User', RoleID FROM UserRole WHERE Name = 'Administrator'");

		// Give access to tables
		query("INSERT INTO RESTPrivilege (TableName,RoleID,CanGET) SELECT A.name, B.RoleID, true FROM sqlite_master A, UserRole B WHERE (A.type='table' AND A.name <> 'sqlite_sequence' AND A.name NOT LIKE 'User%' AND A.name NOT LIKE 'REST%') AND (B.Name <> 'Administrator')");
		query("UPDATE RESTPrivilege SET CanPATCH=true, PATCHFields='Value' WHERE TableName='DeviceValue' AND RoleID IN (SELECT RoleID FROM UserRole WHERE Name <> 'Administrator')");
		query("INSERT INTO RESTPrivilege (TableName,RoleID,CanGET,CanPOST,CanPUT,CanPATCH,CanDELETE) SELECT A.name, B.RoleID, true, true, true, true, true FROM sqlite_master A, UserRole B WHERE(A.type = 'table' and A.name <> 'sqlite_sequence') AND(B.Name = 'Administrator')");

		// Units that Values can be associated with
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Light On/Off', 0, 1, 'Light48_Off.png,Light48_On.png', 'Off,On')");
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Fan On/Off', 0, 1, 'Fan48_Off.png,Fan48_On.png', 'Off,On')");
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Laptop On/Off', 0, 1, 'Computer48_Off.png,Computer48_On.png', 'Off,On')");
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Desktop On/Off', 0, 1, 'ComputerPC48_Off.png,ComputerPC48_On.png', 'Off,On')");
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Contact Open/Close', 0, 1, 'Contact48_Off.png,Contact48_On.png', 'Closed,Open')");
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Door Open/Close', 0, 1, 'Door48_Off.png,Door48_On.png', 'Closed,Open')");
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Dimmer', 0, 100, 'Dimmer48_Off.png,Dimmer48_On.png', 'Off,On')");
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Percentage', 0, 100, 'Percentage48.png', '%')");
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Celsius', -25, 100, 'temp48.png', '°C')");
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Temperature', 0, 6, 'temp-0-5.png,temp-5-10.png,temp-10-15.png,temp-15-20.png,temp-20-25.png,temp-25-30.png,temp-gt-30.png', 'Cold,Chilly,Cool,Mild,Warm,Hot,Baking')");
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Wind Direction', 0, 15, 'WindN.png,WindNNE.png,WindNE.png,WindENE.png,WindE.png,WindESE.png,WindSE.png,WindSSE.png,WindS.png,WindSSW.png,WindSW.png,WindWSW.png,WindW.png,WindWNW.png,WindNW.png,WindNNW.png', 'N,NNE,NE,ENE,E,ESE,SE,SSE,S,SSW,SW,WSW,W,WNW,NW,NNW')");
		query("INSERT INTO ValueUnit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Volume On/Muted', 0, 100, 'Speaker48_Off.png,Speaker48_On.png', 'Muted,On')");

		sqlite3_wal_checkpoint(m_dbase, NULL);
	}
	UpdatePreferencesVar("DB_Version", DB_VERSION);

	//Make sure we have some default preferences
	int nValue = 10;
	std::string sValue;
	if (!GetPreferencesVar("Title", sValue))
	{
		UpdatePreferencesVar("Title", "Domoticz");
	}

	if (!GetPreferencesVar("UseAutoUpdate", nValue))
	{
		UpdatePreferencesVar("UseAutoUpdate", 1);
	}

	if (!GetPreferencesVar("UseAutoBackup", nValue))
	{
		UpdatePreferencesVar("UseAutoBackup", 0);
	}

	if ((!GetPreferencesVar("Language", sValue)) || (sValue.empty()))
	{
		UpdatePreferencesVar("Language", "en");
	}
	if (!GetPreferencesVar("AuthenticationMethod", nValue))
	{
		UpdatePreferencesVar("AuthenticationMethod", 0);//AUTH_LOGIN=0, AUTH_BASIC=1
	}
	if (!GetPreferencesVar("ReleaseChannel", nValue))
	{
		UpdatePreferencesVar("ReleaseChannel", 0);//Stable=0, Beta=1
	}

	nValue = 1;
	if (!GetPreferencesVar("AcceptNewHardware", nValue))
	{
		UpdatePreferencesVar("AcceptNewHardware", 1);
		nValue = 1;
	}
	m_bAcceptNewHardware = (nValue == 1);
	if ((!GetPreferencesVar("ZWavePollInterval", nValue)) || (nValue == 0))
	{
		UpdatePreferencesVar("ZWavePollInterval", 60);
	}
	if (!GetPreferencesVar("ZWaveEnableDebug", nValue))
	{
		UpdatePreferencesVar("ZWaveEnableDebug", 0);
	}
	if ((!GetPreferencesVar("ZWaveNetworkKey", sValue)) || (sValue.empty()))
	{
		sValue = "0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10";
		UpdatePreferencesVar("ZWaveNetworkKey", sValue);
	}

	//Double check network_key
	std::vector<std::string> splitresults;
	StringSplit(sValue, ",", splitresults);
	if (splitresults.size() != 16)
	{
		sValue = "0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10";
		UpdatePreferencesVar("ZWaveNetworkKey", sValue);
	}

	if (!GetPreferencesVar("ZWaveEnableNightlyNetworkHeal", nValue))
	{
		UpdatePreferencesVar("ZWaveEnableNightlyNetworkHeal", 0);
	}
	if (!GetPreferencesVar("BatteryLowNotification", nValue))
	{
		UpdatePreferencesVar("BatteryLowNotification", 0); //default disabled
	}
	if ((!GetPreferencesVar("WebTheme", sValue)) || (sValue.empty()))
	{
		UpdatePreferencesVar("WebTheme", "default");
	}

	//Start background thread
	if (!StartThread())
		return false;
	return true;
}

void CSQLHelper::CloseDatabase()
{
	std::lock_guard<std::mutex> l(m_sqlQueryMutex);
	if (m_dbase != NULL)
	{
		OptimizeDatabase(m_dbase);
		sqlite3_close(m_dbase);
		m_dbase = NULL;
	}
}

void CSQLHelper::StopThread()
{
	if (m_thread)
	{
		RequestStop();
		m_thread->join();
		m_thread.reset();
	}
}

bool CSQLHelper::StartThread()
{
	RequestStart();
	m_thread = std::make_shared<std::thread>(&CSQLHelper::Do_Work, this);
	SetThreadName(m_thread->native_handle(), "SQLHelper");
	return (m_thread != NULL);
}

void CSQLHelper::Do_Work()
{
	while (!IsStopRequested(static_cast<const long>(1000.0f / timer_resolution_hz)))
	{
		if (m_bAcceptHardwareTimerActive)
		{
			m_iAcceptHardwareTimerCounter -= static_cast<float>(1. / timer_resolution_hz);
			if (m_iAcceptHardwareTimerCounter <= (1.0f / timer_resolution_hz / 2))
			{
				m_bAcceptHardwareTimerActive = false;
				m_bAcceptNewHardware = m_bPreviousAcceptNewHardware;
				UpdatePreferencesVar("AcceptNewHardware", (m_bAcceptNewHardware == true) ? 1 : 0);
				if (!m_bAcceptNewHardware)
				{
					_log.Log(LOG_STATUS, "Receiving of new sensors disabled!...");
				}
			}
		}
	}
}

void CSQLHelper::SetDatabaseName(const std::string &DBName)
{
	m_dbase_name = DBName;
}

bool CSQLHelper::DoesColumnExistsInTable(const std::string &columnname, const std::string &tablename)
{
	if (!m_dbase)
	{
		_log.Log(LOG_ERROR, "Database not open!!...Check your user rights!..");
		return false;
	}
	bool columnExists = false;

	sqlite3_stmt *statement;
	std::string szQuery = "SELECT " + columnname + " FROM " + tablename;
	if (sqlite3_prepare_v2(m_dbase, szQuery.c_str(), -1, &statement, NULL) == SQLITE_OK)
	{
		columnExists = true;
		sqlite3_finalize(statement);
	}
	return columnExists;
}

void CSQLHelper::safe_exec_no_return(const char *fmt, ...)
{
	if (!m_dbase)
		return;

	va_list args;
	va_start(args, fmt);
	char *zQuery = sqlite3_vmprintf(fmt, args);
	va_end(args);
	if (!zQuery)
		return;
	sqlite3_exec(m_dbase, zQuery, NULL, NULL, NULL);
	sqlite3_free(zQuery);
}

bool CSQLHelper::safe_UpdateBlobInTableWithID(const std::string &Table, const std::string &Column, const std::string &sID, const std::string &BlobData)
{
	if (!m_dbase)
		return false;
	sqlite3_stmt *stmt = NULL;
	char *zQuery = sqlite3_mprintf("UPDATE %q SET %q = ? WHERE ID=%q", Table.c_str(), Column.c_str(), sID.c_str());
	if (!zQuery)
	{
		_log.Log(LOG_ERROR, "SQL: Out of memory, or invalid printf!....");
		return false;
	}
	int rc = sqlite3_prepare_v2(m_dbase, zQuery, -1, &stmt, NULL);
	sqlite3_free(zQuery);
	if (rc != SQLITE_OK) {
		return false;
	}
	rc = sqlite3_bind_blob(stmt, 1, BlobData.c_str(), BlobData.size(), SQLITE_STATIC);
	if (rc != SQLITE_OK) {
		return false;
	}
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		return false;
	}
	sqlite3_finalize(stmt);
	return true;
}

std::vector<std::vector<std::string> > CSQLHelper::safe_query(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char *zQuery = sqlite3_vmprintf(fmt, args);
	va_end(args);
	if (!zQuery)
	{
		_log.Log(LOG_ERROR, "SQL: Out of memory, or invalid printf!....");
		std::vector<std::vector<std::string> > results;
		return results;
	}
	std::vector<std::vector<std::string> > results = query(zQuery);
	sqlite3_free(zQuery);
	return results;
}

std::vector<std::vector<std::string> > CSQLHelper::query(const std::string &szQuery)
{
	if (!m_dbase)
	{
		_log.Log(LOG_ERROR, "Database not open!!...Check your user rights!..");
		std::vector<std::vector<std::string> > results;
		return results;
	}
	std::lock_guard<std::mutex> l(m_sqlQueryMutex);

	sqlite3_stmt *statement;
	std::vector<std::vector<std::string> > results;

	if (sqlite3_prepare_v2(m_dbase, szQuery.c_str(), -1, &statement, 0) == SQLITE_OK)
	{
		int cols = sqlite3_column_count(statement);
		while (true)
		{
			int result = sqlite3_step(statement);
			if (result == SQLITE_ROW)
			{
				std::vector<std::string> values;
				for (int col = 0; col < cols; col++)
				{
					char* value = (char*)sqlite3_column_text(statement, col);
					if ((value == 0) && (col == 0))
						break;
					else if (value == 0)
						values.push_back(std::string("")); //insert empty string
					else
						values.push_back(value);
				}
				if (values.size() > 0)
					results.push_back(values);
			}
			else
			{
				break;
			}
		}
		sqlite3_finalize(statement);
	}

	std::string error = sqlite3_errmsg(m_dbase);
	if (error != "not an error")
		_log.Log(LOG_ERROR, "SQL Query(\"%s\") : %s", szQuery.c_str(), error.c_str());
	return results;
}

std::vector<std::vector<std::string> > CSQLHelper::safe_queryBlob(const char *fmt, ...)
{
	va_list args;
	std::vector<std::vector<std::string> > results;
	va_start(args, fmt);
	char *zQuery = sqlite3_vmprintf(fmt, args);
	va_end(args);
	if (!zQuery)
	{
		_log.Log(LOG_ERROR, "SQL: Out of memory, or invalid printf!....");
		std::vector<std::vector<std::string> > results;
		return results;
	}
	results = queryBlob(zQuery);
	sqlite3_free(zQuery);
	return results;
}

std::vector<std::vector<std::string> > CSQLHelper::queryBlob(const std::string &szQuery)
{
	if (!m_dbase)
	{
		_log.Log(LOG_ERROR, "Database not open!!...Check your user rights!..");
		std::vector<std::vector<std::string> > results;
		return results;
	}
	std::lock_guard<std::mutex> l(m_sqlQueryMutex);

	sqlite3_stmt *statement;
	std::vector<std::vector<std::string> > results;

	if (sqlite3_prepare_v2(m_dbase, szQuery.c_str(), -1, &statement, 0) == SQLITE_OK)
	{
		int cols = sqlite3_column_count(statement);
		while (true)
		{
			int result = sqlite3_step(statement);
			if (result == SQLITE_ROW)
			{
				std::vector<std::string> values;
				for (int col = 0; col < cols; col++)
				{
					int blobSize = sqlite3_column_bytes(statement, col);
					char* value = (char*)sqlite3_column_blob(statement, col);
					if ((blobSize == 0) && (col == 0))
						break;
					else if (value == 0)
						values.push_back(std::string("")); //insert empty string
					else
						values.push_back(std::string(value, value + blobSize));
				}
				if (values.size() > 0)
					results.push_back(values);
			}
			else
			{
				break;
			}
		}
		sqlite3_finalize(statement);
	}

	std::string error = sqlite3_errmsg(m_dbase);
	if (error != "not an error")
		_log.Log(LOG_ERROR, "SQL Query(\"%s\") : %s", szQuery.c_str(), error.c_str());
	return results;
}

bool CSQLHelper::DoesDeviceExist(const int HardwareID, const char* ID, const unsigned char unit, const unsigned char devType, const unsigned char subType) {
	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT ID,Name FROM DeviceStatus WHERE (HardwareID=%d AND DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d)", HardwareID, ID, unit, devType, subType);
	if (result.empty()) {
		return false;
	}
	else {
		return true;
	}
}

void CSQLHelper::UpdatePreferencesVar(const std::string &Key, const std::string &sValue)
{
	UpdatePreferencesVar(Key, 0, sValue);
}
void CSQLHelper::UpdatePreferencesVar(const std::string &Key, const double Value)
{
	std::string sValue = boost::to_string(Value);
	UpdatePreferencesVar(Key, 0, sValue);
}

void CSQLHelper::UpdatePreferencesVar(const std::string &Key, const int nValue)
{
	UpdatePreferencesVar(Key, nValue, "");
}

void CSQLHelper::UpdatePreferencesVar(const std::string &Key, const int nValue, const std::string &sValue)
{
	if (!m_dbase)
		return;

	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT ROWID FROM Preferences WHERE (Key='%q')",
		Key.c_str());
	if (result.empty())
	{
		//Insert
		result = safe_query("INSERT INTO Preferences (Key, nValue, sValue) VALUES ('%q', %d,'%q')",
			Key.c_str(), nValue, sValue.c_str());
	}
	else
	{
		//Update
		result = safe_query("UPDATE Preferences SET Key='%q', nValue=%d, sValue='%q' WHERE (ROWID = '%q')",
			Key.c_str(), nValue, sValue.c_str(), result[0][0].c_str());
	}
}

bool CSQLHelper::GetPreferencesVar(const std::string &Key, std::string &sValue)
{
	if (!m_dbase)
		return false;


	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT sValue FROM Preferences WHERE (Key='%q')",
		Key.c_str());
	if (result.empty())
		return false;
	std::vector<std::string> sd = result[0];
	sValue = sd[0];
	return true;
}

bool CSQLHelper::GetPreferencesVar(const std::string &Key, double &Value)
{

	std::string sValue;
	int nValue;
	Value = 0;
	bool res = GetPreferencesVar(Key, nValue, sValue);
	if (!res)
		return false;
	Value = atof(sValue.c_str());
	return true;
}
bool CSQLHelper::GetPreferencesVar(const std::string &Key, int &nValue, std::string &sValue)
{
	if (!m_dbase)
		return false;

	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT nValue, sValue FROM Preferences WHERE (Key='%q')",
		Key.c_str());
	if (result.empty())
		return false;
	std::vector<std::string> sd = result[0];
	nValue = atoi(sd[0].c_str());
	sValue = sd[1];
	return true;
}

bool CSQLHelper::GetPreferencesVar(const std::string &Key, int &nValue)
{
	std::string sValue;
	return GetPreferencesVar(Key, nValue, sValue);
}
void CSQLHelper::DeletePreferencesVar(const std::string &Key)
{
	std::string sValue;
	if (!m_dbase)
		return;

	//if found, delete
	if (GetPreferencesVar(Key, sValue) == true)
	{
		safe_query("DELETE FROM Preferences WHERE (Key='%q')", Key.c_str());
	}
}



int CSQLHelper::GetLastBackupNo(const char *Key, int &nValue)
{
	if (!m_dbase)
		return false;

	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT nValue FROM BackupLog WHERE (Key='%q')", Key);
	if (result.empty())
		return -1;
	std::vector<std::string> sd = result[0];
	nValue = atoi(sd[0].c_str());
	return nValue;
}

void CSQLHelper::SetLastBackupNo(const char *Key, const int nValue)
{
	if (!m_dbase)
		return;

	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT ROWID FROM BackupLog WHERE (Key='%q')", Key);
	if (result.empty())
	{
		//Insert
		safe_query(
			"INSERT INTO BackupLog (Key, nValue) "
			"VALUES ('%q','%d')",
			Key,
			nValue);
	}
	else
	{
		//Update
		uint64_t ID = std::strtoull(result[0][0].c_str(), nullptr, 10);

		safe_query(
			"UPDATE BackupLog SET Key='%q', nValue=%d "
			"WHERE (ROWID = %" PRIu64 ")",
			Key,
			nValue,
			ID);
	}
}

void CSQLHelper::ScheduleShortlog()
{
#ifdef _DEBUG
	//return;
#endif
	if (!m_dbase)
		return;

	try
	{
		//Force WAL flush
		sqlite3_wal_checkpoint(m_dbase, NULL);

		//Removing the line below could cause a very large database,
		//and slow(large) data transfer (specially when working remote!!)
		CleanupShortLog();
	}
	catch (boost::exception & e)
	{
		_log.Log(LOG_ERROR, "Domoticz: Error running the shortlog schedule script!");
#ifdef _DEBUG
		_log.Log(LOG_ERROR, "-----------------\n%s\n----------------", boost::diagnostic_information(e).c_str());
#else
		(void)e;
#endif
		return;
	}
}

void CSQLHelper::ScheduleDay()
{
	if (!m_dbase)
		return;

	try
	{
		//Force WAL flush
		sqlite3_wal_checkpoint(m_dbase, NULL);

		CleanupLightSceneLog();
	}
	catch (boost::exception & e)
	{
		_log.Log(LOG_ERROR, "Domoticz: Error running the daily schedule script!");
#ifdef _DEBUG
		_log.Log(LOG_ERROR, "-----------------\n%s\n----------------", boost::diagnostic_information(e).c_str());
#else
		(void)e;
#endif
		return;
	}
}

void CSQLHelper::CleanupShortLog()
{
	int n5MinuteHistoryDays = 1;
	if (GetPreferencesVar("5MinuteHistoryDays", n5MinuteHistoryDays))
	{
		// If the history days is zero then all data in the short logs is deleted!
		if (n5MinuteHistoryDays == 0)
		{
			_log.Log(LOG_ERROR, "CleanupShortLog(): MinuteHistoryDays is zero!");
			return;
		}
		std::string szQueryFilter = "strftime('%s',datetime('now','localtime')) - strftime('%s',Date) > (SELECT p.nValue * 86400 From Preferences AS p WHERE p.Key='5MinuteHistoryDays')";
	}
}

void CSQLHelper::ClearShortLog()
{
	VacuumDatabase();
}

void CSQLHelper::VacuumDatabase()
{
	query("VACUUM");
}

void CSQLHelper::OptimizeDatabase(sqlite3 *dbase)
{
	if (dbase == NULL)
		return;
	sqlite3_exec(dbase, "PRAGMA optimize;", NULL, NULL, NULL);
}

void CSQLHelper::CleanupLightSceneLog()
{
	//cleanup the lighting log
	int nMaxDays = 30;
	GetPreferencesVar("LightHistoryDays", nMaxDays);

	char szDateEnd[40];
	time_t now = mytime(NULL);
	struct tm tm1;
	localtime_r(&now, &tm1);

	time_t daybefore;
	struct tm tm2;
	constructTime(daybefore, tm2, tm1.tm_year + 1900, tm1.tm_mon + 1, tm1.tm_mday - nMaxDays, tm1.tm_hour, tm1.tm_min, 0, tm1.tm_isdst);
	sprintf(szDateEnd, "%04d-%02d-%02d %02d:%02d:00", tm2.tm_year + 1900, tm2.tm_mon + 1, tm2.tm_mday, tm2.tm_hour, tm2.tm_min);


	safe_query("DELETE FROM LightingLog WHERE (Date<'%q')", szDateEnd);
	safe_query("DELETE FROM SceneLog WHERE (Date<'%q')", szDateEnd);
}

void CSQLHelper::DeleteDataPoint(const char *ID, const std::string &Date)
{
	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT Type,SubType FROM DeviceStatus WHERE (ID==%q)", ID);
	if (result.empty())
		return;

	if (Date.find(':') != std::string::npos)
	{
		char szDateEnd[100];

		time_t now = mytime(NULL);
		struct tm tLastUpdate;
		localtime_r(&now, &tLastUpdate);

		time_t cEndTime;
		ParseSQLdatetime(cEndTime, tLastUpdate, Date, tLastUpdate.tm_isdst);
		tLastUpdate.tm_min += 2;
		sprintf(szDateEnd, "%04d-%02d-%02d %02d:%02d:%02d", tLastUpdate.tm_year + 1900, tLastUpdate.tm_mon + 1, tLastUpdate.tm_mday, tLastUpdate.tm_hour, tLastUpdate.tm_min, tLastUpdate.tm_sec);

		//Short log
		safe_query("DELETE FROM Rain WHERE (DeviceRowID=='%q') AND (Date>='%q') AND (Date<='%q')", ID, Date.c_str(), szDateEnd);
		safe_query("DELETE FROM Wind WHERE (DeviceRowID=='%q') AND (Date>='%q') AND (Date<='%q')", ID, Date.c_str(), szDateEnd);
		safe_query("DELETE FROM UV WHERE (DeviceRowID=='%q') AND (Date>='%q') AND (Date<='%q')", ID, Date.c_str(), szDateEnd);
		safe_query("DELETE FROM Temperature WHERE (DeviceRowID=='%q') AND (Date>='%q') AND (Date<='%q')", ID, Date.c_str(), szDateEnd);
		safe_query("DELETE FROM Meter WHERE (DeviceRowID=='%q') AND (Date>='%q') AND (Date<='%q')", ID, Date.c_str(), szDateEnd);
		safe_query("DELETE FROM MultiMeter WHERE (DeviceRowID=='%q') AND (Date>='%q') AND (Date<='%q')", ID, Date.c_str(), szDateEnd);
		safe_query("DELETE FROM Percentage WHERE (DeviceRowID=='%q') AND (Date>='%q') AND (Date<='%q')", ID, Date.c_str(), szDateEnd);
		safe_query("DELETE FROM Fan WHERE (DeviceRowID=='%q') AND (Date>='%q') AND (Date<='%q')", ID, Date.c_str(), szDateEnd);
	}
	else
	{
		//Day/Month/Year
		safe_query("DELETE FROM Rain_Calendar WHERE (DeviceRowID=='%q') AND (Date=='%q')", ID, Date.c_str());
		safe_query("DELETE FROM Wind_Calendar WHERE (DeviceRowID=='%q') AND (Date=='%q')", ID, Date.c_str());
		safe_query("DELETE FROM UV_Calendar WHERE (DeviceRowID=='%q') AND (Date=='%q')", ID, Date.c_str());
		safe_query("DELETE FROM Temperature_Calendar WHERE (DeviceRowID=='%q') AND (Date=='%q')", ID, Date.c_str());
		safe_query("DELETE FROM Meter_Calendar WHERE (DeviceRowID=='%q') AND (Date=='%q')", ID, Date.c_str());
		safe_query("DELETE FROM MultiMeter_Calendar WHERE (DeviceRowID=='%q') AND (Date=='%q')", ID, Date.c_str());
		safe_query("DELETE FROM Percentage_Calendar WHERE (DeviceRowID=='%q') AND (Date=='%q')", ID, Date.c_str());
		safe_query("DELETE FROM Fan_Calendar WHERE (DeviceRowID=='%q') AND (Date=='%q')", ID, Date.c_str());
	}
}

bool CSQLHelper::RestoreDatabase(const std::string &dbase)
{
	_log.Log(LOG_STATUS, "Restore Database: Starting...");
	//write file to disk
	std::string fpath;
#ifdef WIN32
	size_t bpos = m_dbase_name.rfind('\\');
#else
	size_t bpos = m_dbase_name.rfind('/');
#endif
	if (bpos != std::string::npos)
		fpath = m_dbase_name.substr(0, bpos + 1);
#ifdef WIN32
	std::string outputfile = fpath + "restore.db";
#else
	std::string outputfile = "/tmp/restore.db";
#endif
	std::ofstream outfile;
	outfile.open(outputfile.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!outfile.is_open())
	{
		_log.Log(LOG_ERROR, "Restore Database: Could not open backup file for writing!");
		return false;
	}
	outfile << dbase;
	outfile.flush();
	outfile.close();
	//check if we can open the database (check if valid)
	sqlite3 *dbase_restore = NULL;
	int rc = sqlite3_open(outputfile.c_str(), &dbase_restore);
	if (rc)
	{
		_log.Log(LOG_ERROR, "Restore Database: Could not open SQLite3 database: %s", sqlite3_errmsg(dbase_restore));
		sqlite3_close(dbase_restore);
		return false;
	}
	if (dbase_restore == NULL)
		return false;
	//could still be not valid
	std::stringstream ss;
	ss << "SELECT sValue FROM Preferences WHERE (Key='DB_Version')";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(dbase_restore, ss.str().c_str(), -1, &statement, 0) != SQLITE_OK)
	{
		_log.Log(LOG_ERROR, "Restore Database: Seems this is not our database, or it is corrupted!");
		sqlite3_close(dbase_restore);
		return false;
	}
	OptimizeDatabase(dbase_restore);
	sqlite3_close(dbase_restore);
	//we have a valid database!
	std::remove(outputfile.c_str());

	StopThread();

	//stop database
	sqlite3_close(m_dbase);
	m_dbase = NULL;
	std::ofstream outfile2;
	outfile2.open(m_dbase_name.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!outfile2.is_open())
	{
		_log.Log(LOG_ERROR, "Restore Database: Could not open backup file for writing!");
		return false;
	}
	outfile2 << dbase;
	outfile2.flush();
	outfile2.close();
	//change ownership
#ifndef WIN32
	struct stat info;
	if (stat(m_dbase_name.c_str(), &info) == 0)
	{
		struct passwd *pw = getpwuid(info.st_uid);
		int ret = chown(m_dbase_name.c_str(), pw->pw_uid, pw->pw_gid);
		if (ret != 0)
		{
			_log.Log(LOG_ERROR, "Restore Database: Could not set database ownership (chown returned an error!)");
		}
	}
#endif
	if (!OpenDatabase())
	{
		_log.Log(LOG_ERROR, "Restore Database: Error opening new database!");
		return false;
	}
	//Cleanup the database
	VacuumDatabase();
	_log.Log(LOG_STATUS, "Restore Database: Succeeded!");
	return true;
}

bool CSQLHelper::BackupDatabase(const std::string &OutputFile)
{
	if (!m_dbase)
		return false; //database not open!

	//First cleanup the database
	OptimizeDatabase(m_dbase);
	VacuumDatabase();

	std::lock_guard<std::mutex> l(m_sqlQueryMutex);

	int rc;                     // Function return code
	sqlite3 *pFile;             // Database connection opened on zFilename
	sqlite3_backup *pBackup;    // Backup handle used to copy data

	// Open the database file identified by zFilename.
	rc = sqlite3_open(OutputFile.c_str(), &pFile);
	if (rc != SQLITE_OK)
		return false;

	// Open the sqlite3_backup object used to accomplish the transfer
	pBackup = sqlite3_backup_init(pFile, "main", m_dbase, "main");
	if (pBackup)
	{
		// Each iteration of this loop copies 5 database pages from database
		// pDb to the backup database.
		do {
			rc = sqlite3_backup_step(pBackup, 5);
			//xProgress(  sqlite3_backup_remaining(pBackup), sqlite3_backup_pagecount(pBackup) );
			//if( rc==SQLITE_OK || rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
			  //sqlite3_sleep(250);
			//}
		} while (rc == SQLITE_OK || rc == SQLITE_BUSY || rc == SQLITE_LOCKED);

		/* Release resources allocated by backup_init(). */
		sqlite3_backup_finish(pBackup);
	}
	rc = sqlite3_errcode(pFile);
	// Close the database connection opened on database file zFilename
	// and return the result of this function.
	sqlite3_close(pFile);
	return (rc == SQLITE_OK);
}

void CSQLHelper::FixDaylightSavingTableSimple(const std::string &TableName)
{
	std::vector<std::vector<std::string> > result;

	result = safe_query("SELECT t.RowID, u.RowID, t.Date FROM %s as t, %s as u WHERE (t.[Date] == u.[Date]) AND (t.[DeviceRowID] == u.[DeviceRowID]) AND (t.[RowID] != u.[RowID]) ORDER BY t.[RowID]",
		TableName.c_str(),
		TableName.c_str());
	if (!result.empty())
	{
		std::stringstream sstr;
		unsigned long ID1;
		unsigned long ID2;
		for (const auto & itt : result)
		{
			std::vector<std::string> sd = itt;
			sstr.clear();
			sstr.str("");
			sstr << sd[0];
			sstr >> ID1;
			sstr.clear();
			sstr.str("");
			sstr << sd[1];
			sstr >> ID2;
			if (ID2 > ID1)
			{
				std::string szDate = sd[2];
				std::vector<std::vector<std::string> > result2;
				result2 = safe_query("SELECT date('%q','+1 day')",
					szDate.c_str());

				std::string szDateNew = result2[0][0];

				//Check if Date+1 exists, if yes, remove current double value
				result2 = safe_query("SELECT RowID FROM %s WHERE (Date='%q') AND (RowID=='%q')",
					TableName.c_str(), szDateNew.c_str(), sd[1].c_str());
				if (!result2.empty())
				{
					//Delete row
					safe_query("DELETE FROM %s WHERE (RowID=='%q')", TableName.c_str(), sd[1].c_str());
				}
				else
				{
					//Update date
					safe_query("UPDATE %s SET Date='%q' WHERE (RowID=='%q')", TableName.c_str(), szDateNew.c_str(), sd[1].c_str());
				}
			}
		}
	}
}

void CSQLHelper::FixDaylightSaving()
{
	//First the easy tables
	FixDaylightSavingTableSimple("Fan_Calendar");
	FixDaylightSavingTableSimple("Percentage_Calendar");
	FixDaylightSavingTableSimple("Rain_Calendar");
	FixDaylightSavingTableSimple("Temperature_Calendar");
	FixDaylightSavingTableSimple("UV_Calendar");
	FixDaylightSavingTableSimple("Wind_Calendar");

	//Meter_Calendar
	std::vector<std::vector<std::string> > result;

	result = safe_query("SELECT t.RowID, u.RowID, t.Value, u.Value, t.Date from Meter_Calendar as t, Meter_Calendar as u WHERE (t.[Date] == u.[Date]) AND (t.[DeviceRowID] == u.[DeviceRowID]) AND (t.[RowID] != u.[RowID]) ORDER BY t.[RowID]");
	if (!result.empty())
	{
		std::stringstream sstr;
		unsigned long ID1;
		unsigned long ID2;
		unsigned long long Value1;
		unsigned long long Value2;
		unsigned long long ValueDest;
		for (const auto & itt : result)
		{
			std::vector<std::string> sd1 = itt;

			sstr.clear();
			sstr.str("");
			sstr << sd1[0];
			sstr >> ID1;
			sstr.clear();
			sstr.str("");
			sstr << sd1[1];
			sstr >> ID2;
			sstr.clear();
			sstr.str("");
			sstr << sd1[2];
			sstr >> Value1;
			sstr.clear();
			sstr.str("");
			sstr << sd1[3];
			sstr >> Value2;
			if (ID2 > ID1)
			{
				if (Value2 > Value1)
					ValueDest = Value2 - Value1;
				else
					ValueDest = Value2;

				std::string szDate = sd1[4];
				std::vector<std::vector<std::string> > result2;
				result2 = safe_query("SELECT date('%q','+1 day')", szDate.c_str());

				std::string szDateNew = result2[0][0];

				//Check if Date+1 exists, if yes, remove current double value
				result2 = safe_query("SELECT RowID FROM Meter_Calendar WHERE (Date='%q') AND (RowID=='%q')", szDateNew.c_str(), sd1[1].c_str());
				if (!result2.empty())
				{
					//Delete Row
					safe_query("DELETE FROM Meter_Calendar WHERE (RowID=='%q')", sd1[1].c_str());
				}
				else
				{
					//Update row with new Date
					safe_query("UPDATE Meter_Calendar SET Date='%q', Value=%llu WHERE (RowID=='%q')", szDateNew.c_str(), ValueDest, sd1[1].c_str());
				}
			}
		}
	}

	//Last (but not least) MultiMeter_Calendar
	result = safe_query("SELECT t.RowID, u.RowID, t.Value1, t.Value2, t.Value3, t.Value4, t.Value5, t.Value6, u.Value1, u.Value2, u.Value3, u.Value4, u.Value5, u.Value6, t.Date from MultiMeter_Calendar as t, MultiMeter_Calendar as u WHERE (t.[Date] == u.[Date]) AND (t.[DeviceRowID] == u.[DeviceRowID]) AND (t.[RowID] != u.[RowID]) ORDER BY t.[RowID]");
	if (!result.empty())
	{
		std::stringstream sstr;
		unsigned long ID1;
		unsigned long ID2;
		unsigned long long tValue1;
		unsigned long long tValue2;
		unsigned long long tValue3;
		unsigned long long tValue4;
		unsigned long long tValue5;
		unsigned long long tValue6;

		unsigned long long uValue1;
		unsigned long long uValue2;
		unsigned long long uValue3;
		unsigned long long uValue4;
		unsigned long long uValue5;
		unsigned long long uValue6;

		unsigned long long ValueDest1;
		unsigned long long ValueDest2;
		unsigned long long ValueDest3;
		unsigned long long ValueDest4;
		unsigned long long ValueDest5;
		unsigned long long ValueDest6;
		for (const auto & itt : result)
		{
			std::vector<std::string> sd1 = itt;

			sstr.clear();
			sstr.str("");
			sstr << sd1[0];
			sstr >> ID1;
			sstr.clear();
			sstr.str("");
			sstr << sd1[1];
			sstr >> ID2;

			sstr.clear();
			sstr.str("");
			sstr << sd1[2];
			sstr >> tValue1;
			sstr.clear();
			sstr.str("");
			sstr << sd1[3];
			sstr >> tValue2;
			sstr.clear();
			sstr.str("");
			sstr << sd1[4];
			sstr >> tValue3;
			sstr.clear();
			sstr.str("");
			sstr << sd1[5];
			sstr >> tValue4;
			sstr.clear();
			sstr.str("");
			sstr << sd1[6];
			sstr >> tValue5;
			sstr.clear();
			sstr.str("");
			sstr << sd1[7];
			sstr >> tValue6;

			sstr.clear();
			sstr.str("");
			sstr << sd1[8];
			sstr >> uValue1;
			sstr.clear();
			sstr.str("");
			sstr << sd1[9];
			sstr >> uValue2;
			sstr.clear();
			sstr.str("");
			sstr << sd1[10];
			sstr >> uValue3;
			sstr.clear();
			sstr.str("");
			sstr << sd1[11];
			sstr >> uValue4;
			sstr.clear();
			sstr.str("");
			sstr << sd1[12];
			sstr >> uValue5;
			sstr.clear();
			sstr.str("");
			sstr << sd1[13];
			sstr >> uValue6;

			if (ID2 > ID1)
			{
				if (uValue1 > tValue1)
					ValueDest1 = uValue1 - tValue1;
				else
					ValueDest1 = uValue1;
				if (uValue2 > tValue2)
					ValueDest2 = uValue2 - tValue2;
				else
					ValueDest2 = uValue2;
				if (uValue3 > tValue3)
					ValueDest3 = uValue3 - tValue3;
				else
					ValueDest3 = uValue3;
				if (uValue4 > tValue4)
					ValueDest4 = uValue4 - tValue4;
				else
					ValueDest4 = uValue4;
				if (uValue5 > tValue5)
					ValueDest5 = uValue5 - tValue5;
				else
					ValueDest5 = uValue5;
				if (uValue6 > tValue6)
					ValueDest6 = uValue6 - tValue6;
				else
					ValueDest6 = uValue6;

				std::string szDate = sd1[14];
				std::vector<std::vector<std::string> > result2;
				result2 = safe_query("SELECT date('%q','+1 day')", szDate.c_str());

				std::string szDateNew = result2[0][0];

				//Check if Date+1 exists, if yes, remove current double value
				result2 = safe_query("SELECT RowID FROM MultiMeter_Calendar WHERE (Date='%q') AND (RowID=='%q')", szDateNew.c_str(), sd1[1].c_str());
				if (!result2.empty())
				{
					//Delete Row
					safe_query("DELETE FROM MultiMeter_Calendar WHERE (RowID=='%q')", sd1[1].c_str());
				}
				else
				{
					//Update row with new Date
					safe_query("UPDATE MultiMeter_Calendar SET Date='%q', Value1=%llu, Value2=%llu, Value3=%llu, Value4=%llu, Value5=%llu, Value6=%llu WHERE (RowID=='%q')",
						szDateNew.c_str(), ValueDest1, ValueDest2, ValueDest3, ValueDest4, ValueDest5, ValueDest6, sd1[1].c_str());
				}
			}
		}
	}

}

bool CSQLHelper::CheckDate(const std::string &sDate, int& d, int& m, int& y)
{
	std::istringstream is(sDate);
	char delimiter;
	if (is >> d >> delimiter >> m >> delimiter >> y) {
		struct tm t = { 0 };
		t.tm_mday = d;
		t.tm_mon = m - 1;
		t.tm_year = y - 1900;
		t.tm_isdst = -1;

		time_t when = mktime(&t);
		struct tm norm;
		localtime_r(&when, &norm);

		return (norm.tm_mday == d &&
			norm.tm_mon == m - 1 &&
			norm.tm_year == y - 1900);
	}
	return false;
}

bool CSQLHelper::CheckDateSQL(const std::string &sDate)
{
	if (sDate.size() != 10) {
		return false;
	}

	std::istringstream is(sDate);
	int d, m, y;
	char delimiter1, delimiter2;

	if (is >> y >> delimiter1 >> m >> delimiter2 >> d) {
		if (
			(delimiter1 != '-')
			|| (delimiter2 != '-')
			) {
			return false;
		}
		struct tm t = { 0 };
		t.tm_mday = d;
		t.tm_mon = m - 1;
		t.tm_year = y - 1900;
		t.tm_isdst = -1;

		time_t when = mktime(&t);
		struct tm norm;
		localtime_r(&when, &norm);

		return (norm.tm_mday == d &&
			norm.tm_mon == m - 1 &&
			norm.tm_year == y - 1900);
	}
	return false;
}

bool CSQLHelper::CheckDateTimeSQL(const std::string &sDateTime)
{
	if (sDateTime.size() != 19) {
		return false;
	}

	struct tm t;
	time_t when;
	bool result = ParseSQLdatetime(when, t, sDateTime);

	if (result) {
		struct tm norm;
		localtime_r(&when, &norm);

		return (
			norm.tm_mday == t.tm_mday
			&& norm.tm_mon == t.tm_mon
			&& norm.tm_year == t.tm_year
			&& norm.tm_hour == t.tm_hour
			&& norm.tm_min == t.tm_min
			&& norm.tm_mday == t.tm_mday
			&& norm.tm_sec == t.tm_sec
			);
	}
	return false;
}

bool CSQLHelper::CheckTime(const std::string &sTime)
{

	int iSemiColon = sTime.find(':');
	if ((iSemiColon == std::string::npos) || (iSemiColon < 1) || (iSemiColon > 2) || (iSemiColon == sTime.length() - 1)) return false;
	if ((sTime.length() < 3) || (sTime.length() > 5)) return false;
	if (atoi(sTime.substr(0, iSemiColon).c_str()) >= 24) return false;
	if (atoi(sTime.substr(iSemiColon + 1).c_str()) >= 60) return false;
	return true;
}

void CSQLHelper::AllowNewHardwareTimer(const int iTotMinutes)
{
	m_iAcceptHardwareTimerCounter = iTotMinutes * 60.0f;
	if (m_bAcceptHardwareTimerActive == false)
	{
		m_bPreviousAcceptNewHardware = m_bAcceptNewHardware;
	}
	m_bAcceptNewHardware = true;
	m_bAcceptHardwareTimerActive = true;
	_log.Log(LOG_STATUS, "New sensors allowed for %d minutes...", iTotMinutes);
}

/*
std::string CSQLHelper::GetDeviceValue(const char * FieldName, const char *Idx)
{
	TSqlQueryResult result = safe_query("SELECT %s from DeviceStatus WHERE (ID == %s )", FieldName, Idx);
	if (!result.empty())
		return  result[0][0];
	else
		return  "";
}
*/

bool CSQLHelper::InsertCustomIconFromZip(const std::string &szZip, std::string &ErrorMessage)
{
	//write file to disk
#ifdef WIN32
	std::string outputfile = "custom_icons.zip";
#else
	std::string outputfile = "/tmp/custom_icons.zip";
#endif
	std::ofstream outfile;
	outfile.open(outputfile.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!outfile.is_open())
	{
		ErrorMessage = "Error writing zip to disk";
		return false;
	}
	outfile << szZip;
	outfile.flush();
	outfile.close();

	return InsertCustomIconFromZipFile(outputfile, ErrorMessage);
}

bool CSQLHelper::InsertCustomIconFromZipFile(const std::string &szZipFile, std::string &ErrorMessage)
{
	clx::basic_unzip<char> in(szZipFile);
	if (!in.is_open())
	{
		ErrorMessage = "Error opening zip file";
		return false;
	}

	int iTotalAdded = 0;

	for (clx::unzip::iterator pos = in.begin(); pos != in.end(); ++pos) {
		//_log.Log(LOG_STATUS, "unzip: %s", pos->path().c_str());
		std::string fpath = pos->path();

		//Skip strange folders
		if (fpath.find("__MACOSX") != std::string::npos)
			continue;

		int ipos = fpath.find("icons.txt");
		if (ipos != std::string::npos)
		{
			std::string rpath;
			if (ipos > 0)
				rpath = fpath.substr(0, ipos);

			uLong fsize;
			unsigned char *pFBuf = (unsigned char *)(pos).Extract(fsize, 1);
			if (pFBuf == NULL)
			{
				ErrorMessage = "Could not extract icons.txt";
				return false;
			}
			pFBuf[fsize] = 0; //null terminate

			std::string _defFile = std::string(pFBuf, pFBuf + fsize);
			free(pFBuf);

			std::vector<std::string> _Lines;
			StringSplit(_defFile, "\n", _Lines);
			for (const auto & itt : _Lines)
			{
				std::string sLine = itt;
				sLine.erase(std::remove(sLine.begin(), sLine.end(), '\r'), sLine.end());
				std::vector<std::string> splitresult;
				StringSplit(sLine, ";", splitresult);
				if (splitresult.size() == 3)
				{
					std::string IconBase = splitresult[0];
					std::string IconName = splitresult[1];
					std::string IconDesc = splitresult[2];

					//Check if this Icon(Name) does not exist in the database already
					std::vector<std::vector<std::string> > result;
					result = safe_query("SELECT ID FROM CustomImages WHERE Base='%q'", IconBase.c_str());
					bool bIsDuplicate = (result.size() > 0);
					int RowID = 0;
					if (bIsDuplicate)
					{
						RowID = atoi(result[0][0].c_str());
					}

					//Locate the files in the zip, if not present back out
					std::string IconFile16 = IconBase + ".png";
					std::string IconFile48On = IconBase + "48_On.png";
					std::string IconFile48Off = IconBase + "48_Off.png";

					std::map<std::string, std::string> _dbImageFiles;
					_dbImageFiles["IconSmall"] = IconFile16;
					_dbImageFiles["IconOn"] = IconFile48On;
					_dbImageFiles["IconOff"] = IconFile48Off;

					//Check if all icons are there
					for (const auto & iItt : _dbImageFiles)
					{
						//std::string TableField = iItt.first;
						std::string IconFile = rpath + iItt.second;
						if (in.find(IconFile) == in.end())
						{
							ErrorMessage = "Icon File: " + IconFile + " is not present";
							if (iTotalAdded > 0)
							{
								//m_webservers.ReloadCustomSwitchIcons();
							}
							return false;
						}
					}

					//All good, now lets add it to the database
					if (!bIsDuplicate)
					{
						safe_query("INSERT INTO CustomImages (Base,Name, Description) VALUES ('%q', '%q', '%q')",
							IconBase.c_str(), IconName.c_str(), IconDesc.c_str());

						//Get our Database ROWID
						result = safe_query("SELECT ID FROM CustomImages WHERE Base='%q'", IconBase.c_str());
						if (result.empty())
						{
							ErrorMessage = "Error adding new row to database!";
							if (iTotalAdded > 0)
							{
								//m_webservers.ReloadCustomSwitchIcons();
							}
							return false;
						}
						RowID = atoi(result[0][0].c_str());
					}
					else
					{
						//Update
						safe_query("UPDATE CustomImages SET Name='%q', Description='%q' WHERE ID=%d",
							IconName.c_str(), IconDesc.c_str(), RowID);

						//Delete from disk, so it will be updated when we exit this function
						std::string IconFile16 = szWWWFolder + "/images/" + IconBase + ".png";
						std::string IconFile48On = szWWWFolder + "/images/" + IconBase + "48_On.png";
						std::string IconFile48Off = szWWWFolder + "/images/" + IconBase + "48_Off.png";
						std::remove(IconFile16.c_str());
						std::remove(IconFile48On.c_str());
						std::remove(IconFile48Off.c_str());
					}

					//Insert the Icons

					for (const auto & iItt : _dbImageFiles)
					{
						std::string TableField = iItt.first;
						std::string IconFile = rpath + iItt.second;

						sqlite3_stmt *stmt = NULL;
						char *zQuery = sqlite3_mprintf("UPDATE CustomImages SET %s = ? WHERE ID=%d", TableField.c_str(), RowID);
						if (!zQuery)
						{
							_log.Log(LOG_ERROR, "SQL: Out of memory, or invalid printf!....");
							return false;
						}
						int rc = sqlite3_prepare_v2(m_dbase, zQuery, -1, &stmt, NULL);
						sqlite3_free(zQuery);
						if (rc != SQLITE_OK) {
							ErrorMessage = "Problem inserting icon into database! " + std::string(sqlite3_errmsg(m_dbase));
							if (iTotalAdded > 0)
							{
								//m_webservers.ReloadCustomSwitchIcons();
							}
							return false;
						}
						// SQLITE_STATIC because the statement is finalized
						// before the buffer is freed:
						pFBuf = (unsigned char *)in.find(IconFile).Extract(fsize);
						if (pFBuf == NULL)
						{
							ErrorMessage = "Could not extract File: " + IconFile16;
							if (iTotalAdded > 0)
							{
								//m_webservers.ReloadCustomSwitchIcons();
							}
							return false;
						}
						rc = sqlite3_bind_blob(stmt, 1, pFBuf, fsize, SQLITE_STATIC);
						if (rc != SQLITE_OK) {
							ErrorMessage = "Problem inserting icon into database! " + std::string(sqlite3_errmsg(m_dbase));
							free(pFBuf);
							if (iTotalAdded > 0)
							{
								//m_webservers.ReloadCustomSwitchIcons();
							}
							return false;
						}
						else {
							rc = sqlite3_step(stmt);
							if (rc != SQLITE_DONE)
							{
								free(pFBuf);
								ErrorMessage = "Problem inserting icon into database! " + std::string(sqlite3_errmsg(m_dbase));
								if (iTotalAdded > 0)
								{
									//m_webservers.ReloadCustomSwitchIcons();
								}
								return false;
							}
						}
						sqlite3_finalize(stmt);
						free(pFBuf);
						iTotalAdded++;
					}
				}
			}

		}
	}

	if (iTotalAdded == 0)
	{
		//definition file not found
		ErrorMessage = "No Icon definition file not found";
		return false;
	}

	//m_webservers.ReloadCustomSwitchIcons();
	return true;
}
