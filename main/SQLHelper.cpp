#include "stdafx.h"
#include "SQLHelper.h"
#include <iostream>     /* standard I/O functions                         */
#include <iomanip>
#include "RFXtrx.h"
#include "RFXNames.h"
#include "localtime_r.h"
#include "Logger.h"
#include "mainworker.h"
#ifdef WITH_EXTERNAL_SQLITE
#include <sqlite3.h>
#else
#include "../sqlite/sqlite3.h"
#endif
#include "../hardware/hardwaretypes.h"
#include "WebServerHelper.h"
#include "../webserver/Base64.h"
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

extern http::server::CWebServerHelper m_webservers;
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
		query("INSERT INTO RESTPrivilege (TableName,RoleID,CanGET,CanPOST,CanPUT,CanPATCH,CanDELETE) SELECT A.name, B.RoleID, true, true, true, true, true FROM sqlite_master A, UserRole B WHERE(A.type = 'table' and A.name <> 'sqlite_sequence') AND(B.Name = 'Administrator')");
		query("INSERT INTO RESTPrivilege (TableName,RoleID,CanGET) SELECT A.name, B.RoleID, true FROM sqlite_master A, UserRole B WHERE (A.type='table' AND A.name <> 'sqlite_sequence' AND A.name NOT LIKE 'User%' AND A.name NOT LIKE 'REST%') AND (B.Name <> 'Administrator')");
		query("UPDATE RESTPrivilege SET CanPATCH=true, PATCHFields='Value' WHERE TableName='DeviceValue' AND RoleID IN (SELECT RoleID FROM UserRole WHERE Name <> 'Administrator')");

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

bool CSQLHelper::SwitchLightFromTasker(const std::string &idx, const std::string &switchcmd, const std::string &level, const std::string &color)
{
	_tColor ocolor(color);
	return SwitchLightFromTasker(std::stoull(idx), switchcmd, atoi(level.c_str()), ocolor);
}

bool CSQLHelper::SwitchLightFromTasker(uint64_t idx, const std::string &switchcmd, int level, _tColor color)
{
	//Get Device details
	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT HardwareID, DeviceID,Unit,Type,SubType,SwitchType,AddjValue2,nValue,sValue,Name,Options FROM DeviceStatus WHERE (ID == %" PRIu64 ")", idx);
	if (result.empty())
		return false;

	std::vector<std::string> sd = result[0];
	return m_mainworker.SwitchLightInt(sd, switchcmd, level, color, false);
}

void CSQLHelper::Do_Work()
{
	std::vector<_tTaskItem> _items2do;

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

		{ // additional scope for lock (accessing size should be within lock too)
			std::lock_guard<std::mutex> l(m_background_task_mutex);
			if (m_background_task_queue.size() > 0)
			{
				_items2do.clear();

				std::vector<_tTaskItem>::iterator itt = m_background_task_queue.begin();
				while (itt != m_background_task_queue.end())
				{
					if (itt->_DelayTime)
					{
						struct timeval tvDiff, DelayTimeEnd;
						getclock(&DelayTimeEnd);
						if (timeval_subtract(&tvDiff, &DelayTimeEnd, &itt->_DelayTimeBegin)) {
							tvDiff.tv_sec = 0;
							tvDiff.tv_usec = 0;
						}
						float diff = ((tvDiff.tv_usec / 1000000.0f) + tvDiff.tv_sec);
						if ((itt->_DelayTime) <= diff)
						{
							_items2do.push_back(*itt);
							itt = m_background_task_queue.erase(itt);
						}
						else
							++itt;
					}
					else
					{
						_items2do.push_back(*itt);
						itt = m_background_task_queue.erase(itt);
					}
				}
			}
		}

		if (_items2do.size() < 1) {
			continue;
		}

		std::vector<_tTaskItem>::iterator itt = _items2do.begin();
		while (itt != _items2do.end())
		{
			_log.Debug(DEBUG_NORM, "SQLH: Do Task ItemType:%d Cmd:%s Value:%s ", itt->_ItemType, itt->_command.c_str(), itt->_sValue.c_str());

			if (itt->_ItemType == TITEM_SWITCHCMD)
			{
				if (itt->_switchtype == STYPE_Motion)
				{
					std::string devname = "";
					switch (itt->_devType)
					{
					case pTypeLighting1:
					case pTypeLighting2:
					case pTypeLighting3:
					case pTypeLighting5:
					case pTypeLighting6:
					case pTypeColorSwitch:
					case pTypeGeneralSwitch:
					case pTypeHomeConfort:
					case pTypeFS20:
						SwitchLightFromTasker(itt->_idx, "Off", 0, NoColor);
						break;
					case pTypeSecurity1:
						switch (itt->_subType)
						{
						case sTypeSecX10M:
							SwitchLightFromTasker(itt->_idx, "No Motion", 0, NoColor);
							break;
						default:
							//just update internally
							//UpdateValueInt(itt->_HardwareID, itt->_ID.c_str(), itt->_unit, itt->_devType, itt->_subType, itt->_signallevel, itt->_batterylevel, itt->_nValue, itt->_sValue.c_str(), devname, true);
							break;
						}
						break;
					case pTypeLighting4:
						//only update internally
						//UpdateValueInt(itt->_HardwareID, itt->_ID.c_str(), itt->_unit, itt->_devType, itt->_subType, itt->_signallevel, itt->_batterylevel, itt->_nValue, itt->_sValue.c_str(), devname, true);
						break;
					default:
						//unknown hardware type, sensor will only be updated internally
						//UpdateValueInt(itt->_HardwareID, itt->_ID.c_str(), itt->_unit, itt->_devType, itt->_subType, itt->_signallevel, itt->_batterylevel, itt->_nValue, itt->_sValue.c_str(), devname, true);
						break;
					}
				}
				else
				{
					if (itt->_devType == pTypeLighting4)
					{
						//only update internally
						std::string devname = "";
						//UpdateValueInt(itt->_HardwareID, itt->_ID.c_str(), itt->_unit, itt->_devType, itt->_subType, itt->_signallevel, itt->_batterylevel, itt->_nValue, itt->_sValue.c_str(), devname, true);
					}
					else
						SwitchLightFromTasker(itt->_idx, "Off", 0, NoColor);
				}
			}
			else if (itt->_ItemType == TITEM_EXECUTE_SCRIPT)
			{
				//start script
				_log.Log(LOG_STATUS, "Executing script: %s", itt->_ID.c_str());
#ifdef WIN32
				ShellExecute(NULL, "open", itt->_ID.c_str(), itt->_sValue.c_str(), NULL, SW_SHOWNORMAL);
#else
				std::string lscript = itt->_ID + " " + itt->_sValue;
				int ret = system(lscript.c_str());
				if (ret != 0)
				{
					_log.Log(LOG_ERROR, "Error executing script command (%s). returned: %d", itt->_ID.c_str(), ret);
				}
#endif
			}
			else if (itt->_ItemType == TITEM_GETURL)
			{
				std::string response;
				std::vector<std::string> headerData, extraHeaders;
				std::string postData = itt->_command;
				std::string callback = itt->_ID;

				if (!itt->_relatedEvent.empty())
					StringSplit(itt->_relatedEvent, "!#", extraHeaders);

				HTTPClient::_eHTTPmethod method = static_cast<HTTPClient::_eHTTPmethod>(itt->_switchtype);

				bool ret;
				if (method == HTTPClient::HTTP_METHOD_GET)
				{
					ret = HTTPClient::GET(itt->_sValue, extraHeaders, response, headerData);
				}
				else if (method == HTTPClient::HTTP_METHOD_POST)
				{
					ret = HTTPClient::POST(itt->_sValue, postData, extraHeaders, response, headerData, true, true);
				}

				m_mainworker.m_eventsystem.TriggerURL(response, headerData, callback);

				if (!ret)
				{
					_log.Log(LOG_ERROR, "Error opening url: %s", itt->_sValue.c_str());
				}
			}
			else if (itt->_ItemType == TITEM_SWITCHCMD_EVENT)
			{
				SwitchLightFromTasker(itt->_idx, itt->_command.c_str(), itt->_level, itt->_Color);
			}

			else if (itt->_ItemType == TITEM_SWITCHCMD_SCENE)
			{
				m_mainworker.SwitchScene(itt->_idx, itt->_command.c_str());
			}
			else if (itt->_ItemType == TITEM_UPDATEDEVICE)
			{
				m_mainworker.UpdateDevice(static_cast<int>(itt->_idx), itt->_nValue, itt->_sValue, 12, 255, (itt->_switchtype ? true : false));
				//m_mainworker.m_eventsystem.UpdateDevice(itt->_idx, itt->_nValue, itt->_sValue, itt->_HardwareID, (itt->_switchtype ? true : false));
			}
			else if (itt->_ItemType == TITEM_CUSTOM_COMMAND)
			{
				m_mainworker.m_eventsystem.CustomCommand(itt->_idx, itt->_command);
			}

			++itt;
		}
		_items2do.clear();
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

bool CSQLHelper::GetLastValue(const int HardwareID, const char* DeviceID, const unsigned char unit, const unsigned char devType, const unsigned char subType, int &nValue, std::string &sValue, struct tm &LastUpdateTime)
{
	bool result = false;
	std::vector<std::vector<std::string> > sqlresult;
	std::string sLastUpdate;
	//std::string sValue;
	//struct tm LastUpdateTime;
	sqlresult = safe_query(
		"SELECT nValue,sValue,LastUpdate FROM DeviceStatus WHERE (HardwareID=%d AND DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d) order by LastUpdate desc limit 1",
		HardwareID, DeviceID, unit, devType, subType);

	if (sqlresult.size() != 0)
	{
		nValue = (int)atoi(sqlresult[0][0].c_str());
		sValue = sqlresult[0][1];
		sLastUpdate = sqlresult[0][2];

		time_t lutime;
		ParseSQLdatetime(lutime, LastUpdateTime, sLastUpdate);

		result = true;
	}

	return result;
}


void CSQLHelper::GetMeterType(const int HardwareID, const char* ID, const unsigned char unit, const unsigned char devType, const unsigned char subType, int &meterType)
{
	meterType = 0;
	std::vector<std::vector<std::string> > result;
	result = safe_query(
		"SELECT SwitchType FROM DeviceStatus WHERE (HardwareID=%d AND DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d)",
		HardwareID, ID, unit, devType, subType);
	if (!result.empty())
	{
		meterType = atoi(result[0][0].c_str());
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

void CSQLHelper::UpdateRFXCOMHardwareDetails(const int HardwareID, const int msg1, const int msg2, const int msg3, const int msg4, const int msg5, const int msg6)
{
	safe_query("UPDATE Hardware SET Mode1=%d, Mode2=%d, Mode3=%d, Mode4=%d, Mode5=%d, Mode6=%d WHERE (ID == %d)",
		msg1, msg2, msg3, msg4, msg5, msg6, HardwareID);
}

bool CSQLHelper::HasTimers(const uint64_t Idx)
{
	if (!m_dbase)
		return false;

	std::vector<std::vector<std::string> > result;

	result = safe_query("SELECT COUNT(*) FROM Timers WHERE (DeviceRowID==%" PRIu64 ") AND (TimerPlan==%d)", Idx, m_ActiveTimerPlan);
	if (!result.empty())
	{
		std::vector<std::string> sd = result[0];
		int totaltimers = atoi(sd[0].c_str());
		if (totaltimers > 0)
			return true;
	}
	result = safe_query("SELECT COUNT(*) FROM SetpointTimers WHERE (DeviceRowID==%" PRIu64 ") AND (TimerPlan==%d)", Idx, m_ActiveTimerPlan);
	if (!result.empty())
	{
		std::vector<std::string> sd = result[0];
		int totaltimers = atoi(sd[0].c_str());
		return (totaltimers > 0);
	}
	return false;
}

bool CSQLHelper::HasTimers(const std::string &Idx)
{
	uint64_t idxll = std::strtoull(Idx.c_str(), nullptr, 10);
	return HasTimers(idxll);
}

bool CSQLHelper::HasSceneTimers(const uint64_t Idx)
{
	if (!m_dbase)
		return false;

	std::vector<std::vector<std::string> > result;

	result = safe_query("SELECT COUNT(*) FROM SceneTimers WHERE (SceneRowID==%" PRIu64 ") AND (TimerPlan==%d)", Idx, m_ActiveTimerPlan);
	if (result.empty())
		return false;
	std::vector<std::string> sd = result[0];
	int totaltimers = atoi(sd[0].c_str());
	return (totaltimers > 0);
}

bool CSQLHelper::HasSceneTimers(const std::string &Idx)
{
	uint64_t idxll = std::strtoull(Idx.c_str(), nullptr, 10);
	return HasSceneTimers(idxll);
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

		UpdateMeter();
		UpdateMultiMeter();
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

		AddCalendarTemperature();
		AddCalendarUpdateRain();
		AddCalendarUpdateUV();
		AddCalendarUpdateWind();
		AddCalendarUpdateMeter();
		AddCalendarUpdateMultiMeter();
		AddCalendarUpdatePercentage();
		AddCalendarUpdateFan();
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

bool CSQLHelper::UpdateCalendarMeter(
	const int HardwareID,
	const char* DeviceID,
	const unsigned char unit,
	const unsigned char devType,
	const unsigned char subType,
	const bool shortLog,
	const long long MeterValue,
	const long long MeterUsage,
	const char* date)
{
	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT ID, Name, SwitchType FROM DeviceStatus WHERE (HardwareID=%d AND DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d)", HardwareID, DeviceID, unit, devType, subType);
	if (result.empty()) {
		return false;
	}

	std::vector<std::string> sd = result[0];
	uint64_t DeviceRowID = std::strtoull(sd[0].c_str(), nullptr, 10);
	//std::string devname = sd[1];
	//_eSwitchType switchtype = (_eSwitchType)atoi(sd[2].c_str());

	if (shortLog)
	{
		if (!CheckDateTimeSQL(date)) {
			_log.Log(LOG_ERROR, "UpdateCalendarMeter(): incorrect date time format received, YYYY-MM-DD HH:mm:ss expected!");
			return false;
		}

		//insert or replace record
		result = safe_query(
			"SELECT DeviceRowID FROM Meter "
			"WHERE ((DeviceRowID=='%" PRIu64 "') AND (Date=='%q'))",
			DeviceRowID, date
		);
		if (result.empty())
		{
			safe_query(
				"INSERT INTO Meter (DeviceRowID, Value, Usage, Date) "
				"VALUES ('%" PRIu64 "','%lld','%lld','%q')",
				DeviceRowID, MeterValue, MeterUsage, date
			);
		}
		else
		{
			safe_query(
				"UPDATE Meter SET DeviceRowID='%" PRIu64 "', Value='%lld', Usage='%lld', Date='%q' "
				"WHERE ((DeviceRowID=='%" PRIu64 "') AND (Date=='%q'))",
				DeviceRowID, (MeterValue < 0) ? 0 : MeterValue, (MeterUsage < 0) ? 0 : MeterUsage, date,
				DeviceRowID, date
			);
		}
	}
	else
	{
		if (!CheckDateSQL(date)) {
			_log.Log(LOG_ERROR, "UpdateCalendarMeter(): incorrect date format received, YYYY-MM-DD expected!");
			return false;
		}
		result = safe_query(
			"SELECT DeviceRowID FROM Meter_Calendar "
			"WHERE (DeviceRowID=='%" PRIu64 "') AND (Date=='%q')",
			DeviceRowID, date
		);
		if (result.empty())
		{
			safe_query(
				"INSERT INTO Meter_Calendar (DeviceRowID, Counter, Value, Date) "
				"VALUES ('%" PRIu64 "', '%lld', '%lld', '%q')",
				DeviceRowID, (MeterValue < 0) ? 0 : MeterValue, (MeterUsage < 0) ? 0 : MeterUsage, date
			);
		}
		else
		{
			safe_query(
				"UPDATE Meter_Calendar SET DeviceRowID='%" PRIu64 "', Counter='%lld', Value='%lld', Date='%q' "
				"WHERE (DeviceRowID=='%" PRIu64 "') AND (Date=='%q')",
				DeviceRowID, (MeterValue < 0) ? 0 : MeterValue, (MeterUsage < 0) ? 0 : MeterUsage, date,
				DeviceRowID, date
			);
		}
	}
	return true;
}

void CSQLHelper::UpdateMeter()
{
	time_t now = mytime(NULL);
	if (now == 0)
		return;
	struct tm tm1;
	localtime_r(&now, &tm1);

	int SensorTimeOut = 60;
	GetPreferencesVar("SensorTimeout", SensorTimeOut);

	std::vector<std::vector<std::string> > result;
	std::vector<std::vector<std::string> > result2;

	result = safe_query(
		"SELECT ID,Name,HardwareID,DeviceID,Unit,Type,SubType,nValue,sValue,LastUpdate FROM DeviceStatus WHERE ("
		"Type=%d OR " //pTypeRFXMeter
		"Type=%d OR " //pTypeP1Gas
		"Type=%d OR " //pTypeYouLess
		"Type=%d OR " //pTypeENERGY
		"Type=%d OR " //pTypePOWER
		"Type=%d OR " //pTypeAirQuality
		"Type=%d OR " //pTypeUsage
		"Type=%d OR " //pTypeLux
		"Type=%d OR " //pTypeWEIGHT
		"(Type=%d AND SubType=%d) OR " //pTypeRego6XXValue,sTypeRego6XXCounter
		"(Type=%d AND SubType=%d) OR " //pTypeGeneral,sTypeVisibility
		"(Type=%d AND SubType=%d) OR " //pTypeGeneral,sTypeSolarRadiation
		"(Type=%d AND SubType=%d) OR " //pTypeGeneral,sTypeSoilMoisture
		"(Type=%d AND SubType=%d) OR " //pTypeGeneral,sTypeLeafWetness
		"(Type=%d AND SubType=%d) OR " //pTypeRFXSensor,sTypeRFXSensorAD
		"(Type=%d AND SubType=%d) OR" //pTypeRFXSensor,sTypeRFXSensorVolt
		"(Type=%d AND SubType=%d) OR"  //pTypeGeneral,sTypeVoltage
		"(Type=%d AND SubType=%d) OR"  //pTypeGeneral,sTypeCurrent
		"(Type=%d AND SubType=%d) OR"  //pTypeGeneral,sTypeSoundLevel
		"(Type=%d AND SubType=%d) OR " //pTypeGeneral,sTypeDistance
		"(Type=%d AND SubType=%d) OR " //pTypeGeneral,sTypePressure
		"(Type=%d AND SubType=%d) OR " //pTypeGeneral,sTypeCounterIncremental
		"(Type=%d AND SubType=%d)"     //pTypeGeneral,sTypeKwh
		")",
		pTypeRFXMeter,
		pTypeP1Gas,
		pTypeYouLess,
		pTypeENERGY,
		pTypePOWER,
		pTypeAirQuality,
		pTypeUsage,
		pTypeLux,
		pTypeWEIGHT,
		pTypeRego6XXValue, sTypeRego6XXCounter,
		pTypeGeneral, sTypeVisibility,
		pTypeGeneral, sTypeSolarRadiation,
		pTypeGeneral, sTypeSoilMoisture,
		pTypeGeneral, sTypeLeafWetness,
		pTypeRFXSensor, sTypeRFXSensorAD,
		pTypeRFXSensor, sTypeRFXSensorVolt,
		pTypeGeneral, sTypeVoltage,
		pTypeGeneral, sTypeCurrent,
		pTypeGeneral, sTypeSoundLevel,
		pTypeGeneral, sTypeDistance,
		pTypeGeneral, sTypePressure,
		pTypeGeneral, sTypeCounterIncremental,
		pTypeGeneral, sTypeKwh
	);
	if (!result.empty())
	{
		for (const auto & itt : result)
		{
			char szTmp[200];
			std::vector<std::string> sd = itt;

			uint64_t ID = std::strtoull(sd[0].c_str(), nullptr, 10);
			std::string devname = sd[1];
			int hardwareID = atoi(sd[2].c_str());
			std::string DeviceID = sd[3];
			unsigned char Unit = atoi(sd[4].c_str());

			unsigned char dType = atoi(sd[5].c_str());
			unsigned char dSubType = atoi(sd[6].c_str());
			int nValue = atoi(sd[7].c_str());
			std::string sValue = sd[8];
			std::string sLastUpdate = sd[9];

			std::string susage = "0";

			//do not include sensors that have no reading within an hour
			struct tm ntime;
			time_t checktime;
			ParseSQLdatetime(checktime, ntime, sLastUpdate, tm1.tm_isdst);


			//Check for timeout, if timeout then dont add value
			if (dType != pTypeP1Gas)
			{
				if (difftime(now, checktime) >= SensorTimeOut * 60)
					continue;
			}
			else
			{
				//P1 Gas meter transmits results every 1 a 2 hours
				if (difftime(now, checktime) >= 3 * 3600)
					continue;
			}

			if (dType == pTypeYouLess)
			{
				std::vector<std::string> splitresults;
				StringSplit(sValue, ";", splitresults);
				if (splitresults.size() < 2)
					continue;
				sValue = splitresults[0];
				susage = splitresults[1];
			}
			else if (dType == pTypeENERGY)
			{
				std::vector<std::string> splitresults;
				StringSplit(sValue, ";", splitresults);
				if (splitresults.size() < 2)
					continue;
				susage = splitresults[0];
				double fValue = atof(splitresults[1].c_str()) * 100;
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if (dType == pTypePOWER)
			{
				std::vector<std::string> splitresults;
				StringSplit(sValue, ";", splitresults);
				if (splitresults.size() < 2)
					continue;
				susage = splitresults[0];
				double fValue = atof(splitresults[1].c_str()) * 100;
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if (dType == pTypeAirQuality)
			{
				sprintf(szTmp, "%d", nValue);
				sValue = szTmp;
			}
			else if ((dType == pTypeGeneral) && ((dSubType == sTypeSoilMoisture) || (dSubType == sTypeLeafWetness)))
			{
				sprintf(szTmp, "%d", nValue);
				sValue = szTmp;
			}
			else if ((dType == pTypeGeneral) && (dSubType == sTypeVisibility))
			{
				double fValue = atof(sValue.c_str())*10.0f;
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if ((dType == pTypeGeneral) && (dSubType == sTypeDistance))
			{
				double fValue = atof(sValue.c_str())*10.0f;
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if ((dType == pTypeGeneral) && (dSubType == sTypeSolarRadiation))
			{
				double fValue = atof(sValue.c_str())*10.0f;
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if ((dType == pTypeGeneral) && (dSubType == sTypeSoundLevel))
			{
				double fValue = atof(sValue.c_str())*10.0f;
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if ((dType == pTypeGeneral) && (dSubType == sTypeKwh))
			{
				std::vector<std::string> splitresults;
				StringSplit(sValue, ";", splitresults);
				if (splitresults.size() < 2)
					continue;

				double fValue = atof(splitresults[0].c_str())*10.0f;
				sprintf(szTmp, "%.0f", fValue);
				susage = szTmp;

				fValue = atof(splitresults[1].c_str());
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if (dType == pTypeLux)
			{
				double fValue = atof(sValue.c_str());
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if (dType == pTypeWEIGHT)
			{
				double fValue = atof(sValue.c_str())*10.0f;
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if (dType == pTypeRFXSensor)
			{
				double fValue = atof(sValue.c_str());
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if ((dType == pTypeGeneral) && (dSubType == sTypeCounterIncremental))
			{
				double fValue = atof(sValue.c_str());
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if ((dType == pTypeGeneral) && (dSubType == sTypeVoltage))
			{
				double fValue = atof(sValue.c_str())*1000.0f;
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if ((dType == pTypeGeneral) && (dSubType == sTypeCurrent))
			{
				double fValue = atof(sValue.c_str())*1000.0f;
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if ((dType == pTypeGeneral) && (dSubType == sTypePressure))
			{
				double fValue = atof(sValue.c_str())*10.0f;
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}
			else if (dType == pTypeUsage)
			{
				double fValue = atof(sValue.c_str())*10.0f;
				sprintf(szTmp, "%.0f", fValue);
				sValue = szTmp;
			}

			long long MeterValue = std::strtoll(sValue.c_str(), nullptr, 10);
			long long MeterUsage = std::strtoll(susage.c_str(), nullptr, 10);

			//insert record
			safe_query(
				"INSERT INTO Meter (DeviceRowID, Value, [Usage]) "
				"VALUES ('%" PRIu64 "', '%lld', '%lld')",
				ID,
				MeterValue,
				MeterUsage
			);
		}
	}
}

void CSQLHelper::UpdateMultiMeter()
{
	time_t now = mytime(NULL);
	if (now == 0)
		return;
	struct tm tm1;
	localtime_r(&now, &tm1);

	int SensorTimeOut = 60;
	GetPreferencesVar("SensorTimeout", SensorTimeOut);

	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT ID,Type,SubType,nValue,sValue,LastUpdate FROM DeviceStatus WHERE (Type=%d OR Type=%d OR Type=%d)",
		pTypeP1Power,
		pTypeCURRENT,
		pTypeCURRENTENERGY
	);
	if (!result.empty())
	{
		for (const auto & itt : result)
		{
			std::vector<std::string> sd = itt;

			uint64_t ID = std::strtoull(sd[0].c_str(), nullptr, 10);
			unsigned char dType = atoi(sd[1].c_str());
			unsigned char dSubType = atoi(sd[2].c_str());
			//int nValue=atoi(sd[3].c_str());
			std::string sValue = sd[4];

			//do not include sensors that have no reading within an hour
			std::string sLastUpdate = sd[5];
			struct tm ntime;
			time_t checktime;
			ParseSQLdatetime(checktime, ntime, sLastUpdate, tm1.tm_isdst);

			if (difftime(now, checktime) >= SensorTimeOut * 60)
				continue;
			std::vector<std::string> splitresults;
			StringSplit(sValue, ";", splitresults);

			unsigned long long value1 = 0;
			unsigned long long value2 = 0;
			unsigned long long value3 = 0;
			unsigned long long value4 = 0;
			unsigned long long value5 = 0;
			unsigned long long value6 = 0;

			if (dType == pTypeP1Power)
			{
				if (splitresults.size() != 6)
					continue; //impossible
				unsigned long long powerusage1 = std::strtoull(splitresults[0].c_str(), nullptr, 10);
				unsigned long long powerusage2 = std::strtoull(splitresults[1].c_str(), nullptr, 10);
				unsigned long long powerdeliv1 = std::strtoull(splitresults[2].c_str(), nullptr, 10);
				unsigned long long powerdeliv2 = std::strtoull(splitresults[3].c_str(), nullptr, 10);
				unsigned long long usagecurrent = std::strtoull(splitresults[4].c_str(), nullptr, 10);
				unsigned long long delivcurrent = std::strtoull(splitresults[5].c_str(), nullptr, 10);

				value1 = powerusage1;
				value2 = powerdeliv1;
				value5 = powerusage2;
				value6 = powerdeliv2;
				value3 = usagecurrent;
				value4 = delivcurrent;
			}
			else if ((dType == pTypeCURRENT) && (dSubType == sTypeELEC1))
			{
				if (splitresults.size() != 3)
					continue; //impossible

				value1 = (unsigned long)(atof(splitresults[0].c_str())*10.0f);
				value2 = (unsigned long)(atof(splitresults[1].c_str())*10.0f);
				value3 = (unsigned long)(atof(splitresults[2].c_str())*10.0f);
			}
			else if ((dType == pTypeCURRENTENERGY) && (dSubType == sTypeELEC4))
			{
				if (splitresults.size() != 4)
					continue; //impossible

				value1 = (unsigned long)(atof(splitresults[0].c_str())*10.0f);
				value2 = (unsigned long)(atof(splitresults[1].c_str())*10.0f);
				value3 = (unsigned long)(atof(splitresults[2].c_str())*10.0f);
				value4 = (unsigned long long)(atof(splitresults[3].c_str())*1000.0f);
			}
			else
				continue;//don't know you (yet)

			//insert record
			safe_query(
				"INSERT INTO MultiMeter (DeviceRowID, Value1, Value2, Value3, Value4, Value5, Value6) "
				"VALUES ('%" PRIu64 "', '%llu', '%llu', '%llu', '%llu', '%llu', '%llu')",
				ID,
				value1,
				value2,
				value3,
				value4,
				value5,
				value6
			);
		}
	}
}


void CSQLHelper::AddCalendarTemperature()
{
	//Get All temperature devices in the Temperature Table
	std::vector<std::vector<std::string> > resultdevices;
	resultdevices = safe_query("SELECT DISTINCT(DeviceRowID) FROM Temperature ORDER BY DeviceRowID");
	if (resultdevices.size() < 1)
		return; //nothing to do

	char szDateStart[40];
	char szDateEnd[40];

	time_t now = mytime(NULL);
	struct tm ltime;
	localtime_r(&now, &ltime);
	sprintf(szDateEnd, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

	time_t yesterday;
	struct tm tm2;
	getNoon(yesterday, tm2, ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday - 1); // we only want the date
	sprintf(szDateStart, "%04d-%02d-%02d", tm2.tm_year + 1900, tm2.tm_mon + 1, tm2.tm_mday);

	std::vector<std::vector<std::string> > result;

	for (const auto & itt : resultdevices)
	{
		std::vector<std::string> sddev = itt;
		uint64_t ID = std::strtoull(sddev[0].c_str(), nullptr, 10);

		result = safe_query("SELECT MIN(Temperature), MAX(Temperature), AVG(Temperature), MIN(Chill), MAX(Chill), AVG(Humidity), AVG(Barometer), MIN(DewPoint), MIN(SetPoint), MAX(SetPoint), AVG(SetPoint) FROM Temperature WHERE (DeviceRowID='%" PRIu64 "' AND Date>='%q' AND Date<'%q')",
			ID,
			szDateStart,
			szDateEnd
		);
		if (!result.empty())
		{
			std::vector<std::string> sd = result[0];

			float temp_min = static_cast<float>(atof(sd[0].c_str()));
			float temp_max = static_cast<float>(atof(sd[1].c_str()));
			float temp_avg = static_cast<float>(atof(sd[2].c_str()));
			float chill_min = static_cast<float>(atof(sd[3].c_str()));
			float chill_max = static_cast<float>(atof(sd[4].c_str()));
			int humidity = atoi(sd[5].c_str());
			int barometer = atoi(sd[6].c_str());
			float dewpoint = static_cast<float>(atof(sd[7].c_str()));
			float setpoint_min = static_cast<float>(atof(sd[8].c_str()));
			float setpoint_max = static_cast<float>(atof(sd[9].c_str()));
			float setpoint_avg = static_cast<float>(atof(sd[10].c_str()));
			result = safe_query(
				"INSERT INTO Temperature_Calendar (DeviceRowID, Temp_Min, Temp_Max, Temp_Avg, Chill_Min, Chill_Max, Humidity, Barometer, DewPoint, SetPoint_Min, SetPoint_Max, SetPoint_Avg, Date) "
				"VALUES ('%" PRIu64 "', '%.2f', '%.2f', '%.2f', '%.2f', '%.2f', '%d', '%d', '%.2f', '%.2f', '%.2f', '%.2f', '%q')",
				ID,
				temp_min,
				temp_max,
				temp_avg,
				chill_min,
				chill_max,
				humidity,
				barometer,
				dewpoint,
				setpoint_min,
				setpoint_max,
				setpoint_avg,
				szDateStart
			);
		}
	}
}

void CSQLHelper::AddCalendarUpdateRain()
{
	//Get All UV devices
	std::vector<std::vector<std::string> > resultdevices;
	resultdevices = safe_query("SELECT DISTINCT(DeviceRowID) FROM Rain ORDER BY DeviceRowID");
	if (resultdevices.size() < 1)
		return; //nothing to do

	char szDateStart[40];
	char szDateEnd[40];

	time_t now = mytime(NULL);
	struct tm ltime;
	localtime_r(&now, &ltime);
	sprintf(szDateEnd, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

	time_t yesterday;
	struct tm tm2;
	getNoon(yesterday, tm2, ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday - 1); // we only want the date
	sprintf(szDateStart, "%04d-%02d-%02d", tm2.tm_year + 1900, tm2.tm_mon + 1, tm2.tm_mday);

	std::vector<std::vector<std::string> > result;

	for (const auto & itt : resultdevices)
	{
		std::vector<std::string> sddev = itt;
		uint64_t ID = std::strtoull(sddev[0].c_str(), nullptr, 10);

		//Get Device Information
		result = safe_query("SELECT SubType FROM DeviceStatus WHERE (ID='%" PRIu64 "')", ID);
		if (result.empty())
			continue;
		std::vector<std::string> sd = result[0];

		unsigned char subType = atoi(sd[0].c_str());

		if (subType == sTypeRAINWU || subType == sTypeRAINByRate)
		{
			result = safe_query("SELECT Total, Total, Rate FROM Rain WHERE (DeviceRowID='%" PRIu64 "' AND Date>='%q' AND Date<'%q') ORDER BY ROWID DESC LIMIT 1",
				ID,
				szDateStart,
				szDateEnd
			);
		}
		else
		{
			result = safe_query("SELECT MIN(Total), MAX(Total), MAX(Rate) FROM Rain WHERE (DeviceRowID='%" PRIu64 "' AND Date>='%q' AND Date<'%q')",
				ID,
				szDateStart,
				szDateEnd
			);
		}

		if (!result.empty())
		{
			std::vector<std::string> sd = result[0];

			float total_min = static_cast<float>(atof(sd[0].c_str()));
			float total_max = static_cast<float>(atof(sd[1].c_str()));
			int rate = atoi(sd[2].c_str());

			float total_real = 0;
			if (subType == sTypeRAINWU || subType == sTypeRAINByRate)
			{
				total_real = total_max;
			}
			else
			{
				total_real = total_max - total_min;
			}


			if (total_real < 1000)
			{
				result = safe_query(
					"INSERT INTO Rain_Calendar (DeviceRowID, Total, Rate, Date) "
					"VALUES ('%" PRIu64 "', '%.2f', '%d', '%q')",
					ID,
					total_real,
					rate,
					szDateStart
				);
			}
		}
	}
}

void CSQLHelper::AddCalendarUpdateMeter()
{
	float EnergyDivider = 1000.0f;
	float GasDivider = 100.0f;
	float WaterDivider = 100.0f;
	float musage = 0;
	int tValue;
	if (GetPreferencesVar("MeterDividerEnergy", tValue))
	{
		EnergyDivider = float(tValue);
	}
	if (GetPreferencesVar("MeterDividerGas", tValue))
	{
		GasDivider = float(tValue);
	}
	if (GetPreferencesVar("MeterDividerWater", tValue))
	{
		WaterDivider = float(tValue);
	}

	//Get All Meter devices
	std::vector<std::vector<std::string> > resultdevices;
	resultdevices = safe_query("SELECT DISTINCT(DeviceRowID) FROM Meter ORDER BY DeviceRowID");
	if (resultdevices.size() < 1)
		return; //nothing to do

	char szDateStart[40];
	char szDateEnd[40];

	time_t now = mytime(NULL);
	struct tm ltime;
	localtime_r(&now, &ltime);
	sprintf(szDateEnd, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

	time_t yesterday;
	struct tm tm2;
	getNoon(yesterday, tm2, ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday - 1); // we only want the date
	sprintf(szDateStart, "%04d-%02d-%02d", tm2.tm_year + 1900, tm2.tm_mon + 1, tm2.tm_mday);

	std::vector<std::vector<std::string> > result;

	for (const auto & itt : resultdevices)
	{
		std::vector<std::string> sddev = itt;
		uint64_t ID = std::strtoull(sddev[0].c_str(), nullptr, 10);

		//Get Device Information
		result = safe_query("SELECT Name, HardwareID, DeviceID, Unit, Type, SubType, SwitchType FROM DeviceStatus WHERE (ID='%" PRIu64 "')", ID);
		if (result.empty())
			continue;
		std::vector<std::string> sd = result[0];
		std::string devname = sd[0];
		//int hardwareID= atoi(sd[1].c_str());
		//std::string DeviceID=sd[2];
		//unsigned char Unit = atoi(sd[3].c_str());
		unsigned char devType = atoi(sd[4].c_str());
		unsigned char subType = atoi(sd[5].c_str());
		_eSwitchType switchtype = (_eSwitchType)atoi(sd[6].c_str());
		_eMeterType metertype = (_eMeterType)switchtype;

		float tGasDivider = GasDivider;

		if (devType == pTypeP1Power)
		{
			metertype = MTYPE_ENERGY;
		}
		else if (devType == pTypeP1Gas)
		{
			metertype = MTYPE_GAS;
			tGasDivider = 1000.0f;
		}
		else if ((devType == pTypeRego6XXValue) && (subType == sTypeRego6XXCounter))
		{
			metertype = MTYPE_COUNTER;
		}


		result = safe_query("SELECT MIN(Value), MAX(Value), AVG(Value) FROM Meter WHERE (DeviceRowID='%" PRIu64 "' AND Date>='%q' AND Date<'%q')",
			ID,
			szDateStart,
			szDateEnd
		);
		if (!result.empty())
		{
			std::vector<std::string> sd = result[0];

			double total_min = (double)atof(sd[0].c_str());
			double total_max = (double)atof(sd[1].c_str());
			double avg_value = (double)atof(sd[2].c_str());

			if (
				(devType != pTypeAirQuality) &&
				(devType != pTypeRFXSensor) &&
				(!((devType == pTypeGeneral) && (subType == sTypeVisibility))) &&
				(!((devType == pTypeGeneral) && (subType == sTypeDistance))) &&
				(!((devType == pTypeGeneral) && (subType == sTypeSolarRadiation))) &&
				(!((devType == pTypeGeneral) && (subType == sTypeSoilMoisture))) &&
				(!((devType == pTypeGeneral) && (subType == sTypeLeafWetness))) &&
				(!((devType == pTypeGeneral) && (subType == sTypeVoltage))) &&
				(!((devType == pTypeGeneral) && (subType == sTypeCurrent))) &&
				(!((devType == pTypeGeneral) && (subType == sTypePressure))) &&
				(!((devType == pTypeGeneral) && (subType == sTypeSoundLevel))) &&
				(devType != pTypeLux) &&
				(devType != pTypeWEIGHT) &&
				(devType != pTypeUsage)
				)
			{
				double total_real = total_max - total_min;
				double counter = total_max;

				result = safe_query(
					"INSERT INTO Meter_Calendar (DeviceRowID, Value, Counter, Date) "
					"VALUES ('%" PRIu64 "', '%.2f', '%.2f', '%q')",
					ID,
					total_real,
					counter,
					szDateStart
				);

				//Check for Notification
				musage = 0;
			}
			else
			{
				//AirQuality/Usage Meter/Moisture/RFXSensor/Voltage/Lux/SoundLevel insert into MultiMeter_Calendar table
				result = safe_query(
					"INSERT INTO MultiMeter_Calendar (DeviceRowID, Value1,Value2,Value3,Value4,Value5,Value6, Date) "
					"VALUES ('%" PRIu64 "', '%.2f','%.2f','%.2f','%.2f','%.2f','%.2f', '%q')",
					ID,
					total_min, total_max, avg_value, 0.0f, 0.0f, 0.0f,
					szDateStart
				);
			}
			if (
				(devType != pTypeAirQuality) &&
				(devType != pTypeRFXSensor) &&
				((devType != pTypeGeneral) && (subType != sTypeVisibility)) &&
				((devType != pTypeGeneral) && (subType != sTypeDistance)) &&
				((devType != pTypeGeneral) && (subType != sTypeSolarRadiation)) &&
				((devType != pTypeGeneral) && (subType != sTypeVoltage)) &&
				((devType != pTypeGeneral) && (subType != sTypeCurrent)) &&
				((devType != pTypeGeneral) && (subType != sTypePressure)) &&
				((devType != pTypeGeneral) && (subType != sTypeSoilMoisture)) &&
				((devType != pTypeGeneral) && (subType != sTypeLeafWetness)) &&
				((devType != pTypeGeneral) && (subType != sTypeSoundLevel)) &&
				(devType != pTypeLux) &&
				(devType != pTypeWEIGHT)
				)
			{
				result = safe_query("SELECT Value FROM Meter WHERE (DeviceRowID='%" PRIu64 "') ORDER BY ROWID DESC LIMIT 1", ID);
				if (!result.empty())
				{
					std::vector<std::string> sd = result[0];
					//Insert the last (max) counter value into the meter table to get the "today" value correct.
					result = safe_query(
						"INSERT INTO Meter (DeviceRowID, Value, Date) "
						"VALUES ('%" PRIu64 "', '%q', '%q')",
						ID,
						sd[0].c_str(),
						szDateEnd
					);
				}
			}
		}
		else
		{
			//no new meter result received in last day
			result = safe_query(
				"INSERT INTO Meter_Calendar (DeviceRowID, Value, Date) "
				"VALUES ('%" PRIu64 "', '%.2f', '%q')",
				ID,
				0.0f,
				szDateStart
			);
		}
	}
}

void CSQLHelper::AddCalendarUpdateMultiMeter()
{
	float EnergyDivider = 1000.0f;
	int tValue;
	if (GetPreferencesVar("MeterDividerEnergy", tValue))
	{
		EnergyDivider = float(tValue);
	}

	//Get All meter devices
	std::vector<std::vector<std::string> > resultdevices;
	resultdevices = safe_query("SELECT DISTINCT(DeviceRowID) FROM MultiMeter ORDER BY DeviceRowID");
	if (resultdevices.size() < 1)
		return; //nothing to do

	char szDateStart[40];
	char szDateEnd[40];

	time_t now = mytime(NULL);
	struct tm ltime;
	localtime_r(&now, &ltime);
	sprintf(szDateEnd, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

	time_t yesterday;
	struct tm tm2;
	getNoon(yesterday, tm2, ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday - 1); // we only want the date
	sprintf(szDateStart, "%04d-%02d-%02d", tm2.tm_year + 1900, tm2.tm_mon + 1, tm2.tm_mday);

	std::vector<std::vector<std::string> > result;

	for (const auto & itt : resultdevices)
	{
		std::vector<std::string> sddev = itt;
		uint64_t ID = std::strtoull(sddev[0].c_str(), nullptr, 10);

		//Get Device Information
		result = safe_query("SELECT Name, HardwareID, DeviceID, Unit, Type, SubType, SwitchType FROM DeviceStatus WHERE (ID='%" PRIu64 "')", ID);
		if (result.empty())
			continue;
		std::vector<std::string> sd = result[0];

		std::string devname = sd[0];
		//int hardwareID= atoi(sd[1].c_str());
		//std::string DeviceID=sd[2];
		//unsigned char Unit = atoi(sd[3].c_str());
		unsigned char devType = atoi(sd[4].c_str());
		unsigned char subType = atoi(sd[5].c_str());
		//_eSwitchType switchtype=(_eSwitchType) atoi(sd[6].c_str());
		//_eMeterType metertype=(_eMeterType)switchtype;

		result = safe_query(
			"SELECT MIN(Value1), MAX(Value1), MIN(Value2), MAX(Value2), MIN(Value3), MAX(Value3), MIN(Value4), MAX(Value4), MIN(Value5), MAX(Value5), MIN(Value6), MAX(Value6) FROM MultiMeter WHERE (DeviceRowID='%" PRIu64 "' AND Date>='%q' AND Date<'%q')",
			ID,
			szDateStart,
			szDateEnd
		);
		if (!result.empty())
		{
			std::vector<std::string> sd = result[0];

			float total_real[6];
			float counter1 = 0;
			float counter2 = 0;
			float counter3 = 0;
			float counter4 = 0;

			if (devType == pTypeP1Power)
			{
				for (int ii = 0; ii < 6; ii++)
				{
					float total_min = static_cast<float>(atof(sd[(ii * 2) + 0].c_str()));
					float total_max = static_cast<float>(atof(sd[(ii * 2) + 1].c_str()));
					total_real[ii] = total_max - total_min;
				}
				counter1 = static_cast<float>(atof(sd[1].c_str()));
				counter2 = static_cast<float>(atof(sd[3].c_str()));
				counter3 = static_cast<float>(atof(sd[9].c_str()));
				counter4 = static_cast<float>(atof(sd[11].c_str()));
			}
			else
			{
				for (int ii = 0; ii < 6; ii++)
				{
					float fvalue = static_cast<float>(atof(sd[ii].c_str()));
					total_real[ii] = fvalue;
				}
			}

			result = safe_query(
				"INSERT INTO MultiMeter_Calendar (DeviceRowID, Value1, Value2, Value3, Value4, Value5, Value6, Counter1, Counter2, Counter3, Counter4, Date) "
				"VALUES ('%" PRIu64 "', '%.2f', '%.2f', '%.2f', '%.2f', '%.2f', '%.2f', '%.2f', '%.2f', '%.2f', '%.2f', '%q')",
				ID,
				total_real[0],
				total_real[1],
				total_real[2],
				total_real[3],
				total_real[4],
				total_real[5],
				counter1,
				counter2,
				counter3,
				counter4,
				szDateStart
			);

			//Check for Notification
			if (devType == pTypeP1Power)
			{
				float musage = (total_real[0] + total_real[4]) / EnergyDivider;
			}
			/*
			//Insert the last (max) counter values into the table to get the "today" value correct.
			sprintf(szTmp,
			"INSERT INTO MultiMeter (DeviceRowID, Value1, Value2, Value3, Value4, Value5, Value6, Date) "
			"VALUES (%" PRIu64 ", %s, %s, %s, %s, %s, %s, '%s')",
			ID,
			sd[0].c_str(),
			sd[1].c_str(),
			sd[2].c_str(),
			sd[3].c_str(),
			sd[4].c_str(),
			sd[5].c_str(),
			szDateEnd
			);
			result=query(szTmp);
			*/
		}
	}
}

void CSQLHelper::AddCalendarUpdateWind()
{
	//Get All Wind devices
	std::vector<std::vector<std::string> > resultdevices;
	resultdevices = safe_query("SELECT DISTINCT(DeviceRowID) FROM Wind ORDER BY DeviceRowID");
	if (resultdevices.size() < 1)
		return; //nothing to do

	char szDateStart[40];
	char szDateEnd[40];

	time_t now = mytime(NULL);
	struct tm ltime;
	localtime_r(&now, &ltime);
	sprintf(szDateEnd, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

	time_t yesterday;
	struct tm tm2;
	getNoon(yesterday, tm2, ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday - 1); // we only want the date
	sprintf(szDateStart, "%04d-%02d-%02d", tm2.tm_year + 1900, tm2.tm_mon + 1, tm2.tm_mday);

	std::vector<std::vector<std::string> > result;

	for (const auto & itt : resultdevices)
	{
		std::vector<std::string> sddev = itt;
		uint64_t ID = std::strtoull(sddev[0].c_str(), nullptr, 10);

		result = safe_query("SELECT AVG(Direction), MIN(Speed), MAX(Speed), MIN(Gust), MAX(Gust) FROM Wind WHERE (DeviceRowID='%" PRIu64 "' AND Date>='%q' AND Date<'%q')",
			ID,
			szDateStart,
			szDateEnd
		);
		if (!result.empty())
		{
			std::vector<std::string> sd = result[0];

			float Direction = static_cast<float>(atof(sd[0].c_str()));
			int speed_min = atoi(sd[1].c_str());
			int speed_max = atoi(sd[2].c_str());
			int gust_min = atoi(sd[3].c_str());
			int gust_max = atoi(sd[4].c_str());

			result = safe_query(
				"INSERT INTO Wind_Calendar (DeviceRowID, Direction, Speed_Min, Speed_Max, Gust_Min, Gust_Max, Date) "
				"VALUES ('%" PRIu64 "', '%.2f', '%d', '%d', '%d', '%d', '%q')",
				ID,
				Direction,
				speed_min,
				speed_max,
				gust_min,
				gust_max,
				szDateStart
			);
		}
	}
}

void CSQLHelper::AddCalendarUpdateUV()
{
	//Get All UV devices
	std::vector<std::vector<std::string> > resultdevices;
	resultdevices = safe_query("SELECT DISTINCT(DeviceRowID) FROM UV ORDER BY DeviceRowID");
	if (resultdevices.size() < 1)
		return; //nothing to do

	char szDateStart[40];
	char szDateEnd[40];

	time_t now = mytime(NULL);
	struct tm ltime;
	localtime_r(&now, &ltime);
	sprintf(szDateEnd, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

	time_t yesterday;
	struct tm tm2;
	getNoon(yesterday, tm2, ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday - 1); // we only want the date
	sprintf(szDateStart, "%04d-%02d-%02d", tm2.tm_year + 1900, tm2.tm_mon + 1, tm2.tm_mday);

	std::vector<std::vector<std::string> > result;

	for (const auto & itt : resultdevices)
	{
		std::vector<std::string> sddev = itt;
		uint64_t ID = std::strtoull(sddev[0].c_str(), nullptr, 10);

		result = safe_query("SELECT MAX(Level) FROM UV WHERE (DeviceRowID='%" PRIu64 "' AND Date>='%q' AND Date<'%q')",
			ID,
			szDateStart,
			szDateEnd
		);
		if (!result.empty())
		{
			std::vector<std::string> sd = result[0];

			float level = static_cast<float>(atof(sd[0].c_str()));

			result = safe_query(
				"INSERT INTO UV_Calendar (DeviceRowID, Level, Date) "
				"VALUES ('%" PRIu64 "', '%g', '%q')",
				ID,
				level,
				szDateStart
			);
		}
	}
}

void CSQLHelper::AddCalendarUpdatePercentage()
{
	//Get All Percentage devices in the Percentage Table
	std::vector<std::vector<std::string> > resultdevices;
	resultdevices = safe_query("SELECT DISTINCT(DeviceRowID) FROM Percentage ORDER BY DeviceRowID");
	if (resultdevices.size() < 1)
		return; //nothing to do

	char szDateStart[40];
	char szDateEnd[40];

	time_t now = mytime(NULL);
	struct tm ltime;
	localtime_r(&now, &ltime);
	sprintf(szDateEnd, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

	time_t yesterday;
	struct tm tm2;
	getNoon(yesterday, tm2, ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday - 1); // we only want the date
	sprintf(szDateStart, "%04d-%02d-%02d", tm2.tm_year + 1900, tm2.tm_mon + 1, tm2.tm_mday);

	std::vector<std::vector<std::string> > result;

	for (const auto & itt : resultdevices)
	{
		std::vector<std::string> sddev = itt;
		uint64_t ID = std::strtoull(sddev[0].c_str(), nullptr, 10);

		result = safe_query("SELECT MIN(Percentage), MAX(Percentage), AVG(Percentage) FROM Percentage WHERE (DeviceRowID='%" PRIu64 "' AND Date>='%q' AND Date<'%q')",
			ID,
			szDateStart,
			szDateEnd
		);
		if (!result.empty())
		{
			std::vector<std::string> sd = result[0];

			float percentage_min = static_cast<float>(atof(sd[0].c_str()));
			float percentage_max = static_cast<float>(atof(sd[1].c_str()));
			float percentage_avg = static_cast<float>(atof(sd[2].c_str()));
			result = safe_query(
				"INSERT INTO Percentage_Calendar (DeviceRowID, Percentage_Min, Percentage_Max, Percentage_Avg, Date) "
				"VALUES ('%" PRIu64 "', '%g', '%g', '%g','%q')",
				ID,
				percentage_min,
				percentage_max,
				percentage_avg,
				szDateStart
			);
		}
	}
}


void CSQLHelper::AddCalendarUpdateFan()
{
	//Get All FAN devices in the Fan Table
	std::vector<std::vector<std::string> > resultdevices;
	resultdevices = safe_query("SELECT DISTINCT(DeviceRowID) FROM Fan ORDER BY DeviceRowID");
	if (resultdevices.size() < 1)
		return; //nothing to do

	char szDateStart[40];
	char szDateEnd[40];

	time_t now = mytime(NULL);
	struct tm ltime;
	localtime_r(&now, &ltime);
	sprintf(szDateEnd, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

	time_t yesterday;
	struct tm tm2;
	getNoon(yesterday, tm2, ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday - 1); // we only want the date
	sprintf(szDateStart, "%04d-%02d-%02d", tm2.tm_year + 1900, tm2.tm_mon + 1, tm2.tm_mday);

	std::vector<std::vector<std::string> > result;

	for (const auto & itt : resultdevices)
	{
		std::vector<std::string> sddev = itt;
		uint64_t ID = std::strtoull(sddev[0].c_str(), nullptr, 10);

		result = safe_query("SELECT MIN(Speed), MAX(Speed), AVG(Speed) FROM Fan WHERE (DeviceRowID='%" PRIu64 "' AND Date>='%q' AND Date<'%q')",
			ID,
			szDateStart,
			szDateEnd
		);
		if (!result.empty())
		{
			std::vector<std::string> sd = result[0];

			int speed_min = (int)atoi(sd[0].c_str());
			int speed_max = (int)atoi(sd[1].c_str());
			int speed_avg = (int)atoi(sd[2].c_str());
			result = safe_query(
				"INSERT INTO Fan_Calendar (DeviceRowID, Speed_Min, Speed_Max, Speed_Avg, Date) "
				"VALUES ('%" PRIu64 "', '%d', '%d', '%d','%q')",
				ID,
				speed_min,
				speed_max,
				speed_avg,
				szDateStart
			);
		}
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
#if 0
		char szDateStr[40];
		time_t clear_time = mytime(NULL) - (n5MinuteHistoryDays * 24 * 3600);
		struct tm ltime;
		localtime_r(&clear_time, &ltime);
		sprintf(szDateStr, "%04d-%02d-%02d %02d:%02d:%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday, ltime.tm_hour, ltime.tm_min, ltime.tm_sec);
		_log.Log(LOG_STATUS, "Cleaning up shortlog older than %s", szDateStr);
#endif

		char szQuery[250];
		std::string szQueryFilter = "strftime('%s',datetime('now','localtime')) - strftime('%s',Date) > (SELECT p.nValue * 86400 From Preferences AS p WHERE p.Key='5MinuteHistoryDays')";

		sprintf(szQuery, "DELETE FROM Temperature WHERE %s", szQueryFilter.c_str());
		query(szQuery);

		sprintf(szQuery, "DELETE FROM Rain WHERE %s", szQueryFilter.c_str());
		query(szQuery);

		sprintf(szQuery, "DELETE FROM Wind WHERE %s", szQueryFilter.c_str());
		query(szQuery);

		sprintf(szQuery, "DELETE FROM UV WHERE %s", szQueryFilter.c_str());
		query(szQuery);

		sprintf(szQuery, "DELETE FROM Meter WHERE %s", szQueryFilter.c_str());
		query(szQuery);

		sprintf(szQuery, "DELETE FROM MultiMeter WHERE %s", szQueryFilter.c_str());
		query(szQuery);

		sprintf(szQuery, "DELETE FROM Percentage WHERE %s", szQueryFilter.c_str());
		query(szQuery);

		sprintf(szQuery, "DELETE FROM Fan WHERE %s", szQueryFilter.c_str());
		query(szQuery);
	}
}

void CSQLHelper::ClearShortLog()
{
	query("DELETE FROM Temperature");
	query("DELETE FROM Rain");
	query("DELETE FROM Wind");
	query("DELETE FROM UV");
	query("DELETE FROM Meter");
	query("DELETE FROM MultiMeter");
	query("DELETE FROM Percentage");
	query("DELETE FROM Fan");
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

void CSQLHelper::DeleteHardware(const std::string &idx)
{
	safe_query("DELETE FROM Hardware WHERE (ID == '%q')", idx.c_str());

	//and now delete all records in the DeviceStatus table itself
	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT ID FROM DeviceStatus WHERE (HardwareID == '%q')", idx.c_str());
	if (!result.empty())
	{
		std::string devs2delete = "";
		for (const auto & itt : result)
		{
			std::vector<std::string> sd = itt;
			if (!devs2delete.empty())
				devs2delete += ";";
			devs2delete += sd[0];
		}
		DeleteDevices(devs2delete);
	}
	//also delete all records in other tables
	safe_query("DELETE FROM ZWaveNodes WHERE (HardwareID== '%q')", idx.c_str());
	safe_query("DELETE FROM EnoceanSensors WHERE (HardwareID== '%q')", idx.c_str());
	safe_query("DELETE FROM MySensors WHERE (HardwareID== '%q')", idx.c_str());
	safe_query("DELETE FROM WOLNodes WHERE (HardwareID == '%q')", idx.c_str());
}

void CSQLHelper::DeleteCamera(const std::string &idx)
{
	safe_query("DELETE FROM Cameras WHERE (ID == '%q')", idx.c_str());
	safe_query("DELETE FROM CamerasActiveDevices WHERE (CameraRowID == '%q')", idx.c_str());
}

void CSQLHelper::DeletePlan(const std::string &idx)
{
	safe_query("DELETE FROM Plans WHERE (ID == '%q')", idx.c_str());
}

void CSQLHelper::DeleteEvent(const std::string &idx)
{
	safe_query("DELETE FROM EventRules WHERE (EMID == '%q')", idx.c_str());
	safe_query("DELETE FROM EventMaster WHERE (ID == '%q')", idx.c_str());
}

//Argument, one or multiple devices separated by a semicolumn (;)
void CSQLHelper::DeleteDevices(const std::string &idx)
{
	std::vector<std::string> _idx;
	StringSplit(idx, ";", _idx);
	if (!_idx.empty())
	{
		std::set<std::pair<std::string, std::string> > removeddevices;
#ifdef ENABLE_PYTHON
		for (const auto & itt : _idx)
		{
			_log.Debug(DEBUG_NORM, "CSQLHelper::DeleteDevices: ID: %s", itt.c_str());
			std::vector<std::vector<std::string> > result;
			result = safe_query("SELECT HardwareID, Unit FROM DeviceStatus WHERE (ID == '%q')", itt.c_str());
			if (!result.empty())
			{
				std::vector<std::string> sd = result[0];
				std::string HwID = sd[0];
				std::string Unit = sd[1];
				CDomoticzHardwareBase *pHardware = m_mainworker.GetHardwareByIDType(HwID, HTYPE_PythonPlugin);
				if (pHardware != NULL)
				{
					removeddevices.insert(std::make_pair(HwID, Unit));
				}
			}
		}
#endif
		{
			//Avoid mutex deadlock here
			std::lock_guard<std::mutex> l(m_sqlQueryMutex);

			char* errorMessage;
			sqlite3_exec(m_dbase, "BEGIN TRANSACTION", NULL, NULL, &errorMessage);

			for (const auto & itt : _idx)
			{
				safe_exec_no_return("DELETE FROM LightingLog WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM LightSubDevices WHERE (ParentID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM LightSubDevices WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Notifications WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Rain WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Rain_Calendar WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Temperature WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Temperature_Calendar WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Timers WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM SetpointTimers WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM UV WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM UV_Calendar WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Wind WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Wind_Calendar WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Meter WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Meter_Calendar WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM MultiMeter WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM MultiMeter_Calendar WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Percentage WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Percentage_Calendar WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Fan WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM Fan_Calendar WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM SceneDevices WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM DeviceToPlansMap WHERE (DeviceRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM CamerasActiveDevices WHERE (DevSceneType==0) AND (DevSceneRowID == '%q')", itt.c_str());
				safe_exec_no_return("DELETE FROM SharedDevices WHERE (DeviceRowID== '%q')", itt.c_str());
				//notify eventsystem device is no longer present
				uint64_t ullidx = std::strtoull(itt.c_str(), nullptr, 10);
				m_mainworker.m_eventsystem.RemoveSingleState(ullidx, m_mainworker.m_eventsystem.REASON_DEVICE);
				//and now delete all records in the DeviceStatus table itself
				safe_exec_no_return("DELETE FROM DeviceStatus WHERE (ID == '%q')", itt.c_str());
			}
			sqlite3_exec(m_dbase, "COMMIT TRANSACTION", NULL, NULL, &errorMessage);
		}
#ifdef ENABLE_PYTHON
		for (const auto & it : removeddevices)
		{
			int HwID = atoi(it.first.c_str());
			int Unit = atoi(it.second.c_str());
			// Notify plugin to sync plugins' device list
			CDomoticzHardwareBase *pHardware = m_mainworker.GetHardware(HwID);
			if (pHardware != NULL && pHardware->HwdType == HTYPE_PythonPlugin)
			{
				_log.Debug(DEBUG_NORM, "CSQLHelper::DeleteDevices: Notifying plugin %u about deletion of device %u", HwID, Unit);
				Plugins::CPlugin *pPlugin = (Plugins::CPlugin*)pHardware;
				pPlugin->DeviceRemoved(Unit);
			}

		}
#endif
	}
	else
		return;

}

void CSQLHelper::TransferDevice(const std::string &idx, const std::string &newidx)
{
	std::vector<std::vector<std::string> > result;

	safe_query("UPDATE LightingLog SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());
	safe_query("UPDATE LightSubDevices SET ParentID='%q' WHERE (ParentID == '%q')", newidx.c_str(), idx.c_str());
	safe_query("UPDATE LightSubDevices SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());
	safe_query("UPDATE Notifications SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());
	safe_query("UPDATE DeviceToPlansMap SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());
	safe_query("UPDATE SharedDevices SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	//Rain
	result = safe_query("SELECT Date FROM Rain WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Rain SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Rain SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	result = safe_query("SELECT Date FROM Rain_Calendar WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Rain_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Rain_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	//Temperature
	result = safe_query("SELECT Date FROM Temperature WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Temperature SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Temperature SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	result = safe_query("SELECT Date FROM Temperature_Calendar WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Temperature_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Temperature_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	safe_query("UPDATE Timers SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	//UV
	result = safe_query("SELECT Date FROM UV WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE UV SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE UV SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	result = safe_query("SELECT Date FROM UV_Calendar WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE UV_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE UV_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	//Wind
	result = safe_query("SELECT Date FROM Wind WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Wind SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Wind SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	result = safe_query("SELECT Date FROM Wind_Calendar WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Wind_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Wind_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	//Meter
	result = safe_query("SELECT Date FROM Meter WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Meter SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Meter SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	result = safe_query("SELECT Date FROM Meter_Calendar WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Meter_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Meter_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	//Multimeter
	result = safe_query("SELECT Date FROM MultiMeter WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE MultiMeter SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE MultiMeter SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	result = safe_query("SELECT Date FROM MultiMeter_Calendar WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE MultiMeter_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE MultiMeter_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	//Fan
	result = safe_query("SELECT Date FROM Fan WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Fan SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Fan SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	result = safe_query("SELECT Date FROM Fan_Calendar WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Fan_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Fan_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());


	//Percentage
	result = safe_query("SELECT Date FROM Percentage WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Percentage SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Percentage SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());

	result = safe_query("SELECT Date FROM Percentage_Calendar WHERE (DeviceRowID == '%q') ORDER BY Date ASC LIMIT 1", newidx.c_str());
	if (!result.empty())
		safe_query("UPDATE Percentage_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q') AND (Date<'%q')", newidx.c_str(), idx.c_str(), result[0][0].c_str());
	else
		safe_query("UPDATE Percentage_Calendar SET DeviceRowID='%q' WHERE (DeviceRowID == '%q')", newidx.c_str(), idx.c_str());
}

void CSQLHelper::CheckAndUpdateDeviceOrder()
{
	std::vector<std::vector<std::string> > result;

	//Get All ID's where Order=0
	result = safe_query("SELECT ROWID FROM DeviceStatus WHERE ([Order]==0)");
	if (!result.empty())
	{
		for (const auto & itt : result)
		{
			std::vector<std::string> sd = itt;
			safe_query("UPDATE DeviceStatus SET [Order] = (SELECT MAX([Order]) FROM DeviceStatus)+1 WHERE (ROWID == '%q')", sd[0].c_str());
		}
	}
}

void CSQLHelper::CheckAndUpdateSceneDeviceOrder()
{
	std::vector<std::vector<std::string> > result;

	//Get All ID's where Order=0
	result = safe_query("SELECT ROWID FROM SceneDevices WHERE ([Order]==0)");
	if (!result.empty())
	{
		for (const auto & itt : result)
		{
			std::vector<std::string> sd = itt;
			safe_query("UPDATE SceneDevices SET [Order] = (SELECT MAX([Order]) FROM SceneDevices)+1 WHERE (ROWID == '%q')", sd[0].c_str());
		}
	}
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

bool CSQLHelper::DoesSceneByNameExits(const std::string &SceneName)
{
	std::vector<std::vector<std::string> > result;

	//Get All ID's where Order=0
	result = safe_query("SELECT ID FROM Scenes WHERE (Name=='%q')", SceneName.c_str());
	return (result.size() > 0);
}

void CSQLHelper::CheckSceneStatusWithDevice(const std::string &DevIdx)
{
	std::stringstream s_str(DevIdx);
	uint64_t idxll;
	s_str >> idxll;
	return CheckSceneStatusWithDevice(idxll);
}

void CSQLHelper::CheckSceneStatusWithDevice(const uint64_t DevIdx)
{
	std::vector<std::vector<std::string> > result;

	result = safe_query("SELECT SceneRowID FROM SceneDevices WHERE (DeviceRowID == %" PRIu64 ")", DevIdx);
	for (const auto & itt : result)
	{
		std::vector<std::string> sd = itt;
		CheckSceneStatus(sd[0]);
	}
}

void CSQLHelper::CheckSceneStatus(const std::string &Idx)
{
	uint64_t idxll = std::strtoull(Idx.c_str(), nullptr, 10);
	return CheckSceneStatus(idxll);
}

void CSQLHelper::CheckSceneStatus(const uint64_t Idx)
{
	std::vector<std::vector<std::string> > result;

	result = safe_query("SELECT nValue FROM Scenes WHERE (ID == %" PRIu64 ")", Idx);
	if (result.empty())
		return; //not found

	unsigned char orgValue = (unsigned char)atoi(result[0][0].c_str());
	unsigned char newValue = orgValue;

	result = safe_query("SELECT a.ID, a.DeviceID, a.Unit, a.Type, a.SubType, a.SwitchType, a.nValue, a.sValue FROM DeviceStatus AS a, SceneDevices as b WHERE (a.ID == b.DeviceRowID) AND (b.SceneRowID == %" PRIu64 ")",
		Idx);
	if (result.empty())
		return; //no devices in scene

	std::vector<bool> _DeviceStatusResults;

	for (const auto & itt : result)
	{
		std::vector<std::string> sd = itt;
		int nValue = atoi(sd[6].c_str());
		std::string sValue = sd[7];
		//unsigned char Unit=atoi(sd[2].c_str());
		unsigned char dType = atoi(sd[3].c_str());
		unsigned char dSubType = atoi(sd[4].c_str());
		_eSwitchType switchtype = (_eSwitchType)atoi(sd[5].c_str());

		std::string lstatus = "";
		int llevel = 0;
		bool bHaveDimmer = false;
		bool bHaveGroupCmd = false;
		int maxDimLevel = 0;

		GetLightStatus(dType, dSubType, switchtype, nValue, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);
		_DeviceStatusResults.push_back(IsLightSwitchOn(lstatus));
	}

	//Check if all on/off
	int totOn = 0;
	int totOff = 0;

	for (const auto & itt2 : _DeviceStatusResults)
	{
		if (itt2 == true)
			totOn++;
		else
			totOff++;
	}
	if (totOn == _DeviceStatusResults.size())
	{
		//All are on
		newValue = 1;
	}
	else if (totOff == _DeviceStatusResults.size())
	{
		//All are Off
		newValue = 0;
	}
	else
	{
		//Some are on, some are off
		newValue = 2;
	}
	if (newValue != orgValue)
	{
		//Set new Scene status
		safe_query("UPDATE Scenes SET nValue=%d WHERE (ID == %" PRIu64 ")",
			int(newValue), Idx);
		m_mainworker.m_eventsystem.GetCurrentScenesGroups(); 
	}
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

void CSQLHelper::AddTaskItem(const _tTaskItem &tItem, const bool cancelItem)
{
	std::lock_guard<std::mutex> l(m_background_task_mutex);

	// Check if an event for the same device is already in queue, and if so, replace it
	_log.Debug(DEBUG_NORM, "SQLH AddTask: Request to add task: idx=%" PRIu64 ", DelayTime=%f, Command='%s', Level=%d, Color='%s', RelatedEvent='%s'", tItem._idx, tItem._DelayTime, tItem._command.c_str(), tItem._level, tItem._Color.toString().c_str(), tItem._relatedEvent.c_str());
	// Remove any previous task linked to the same device

	if (
		(tItem._ItemType == TITEM_SWITCHCMD_EVENT) ||
		(tItem._ItemType == TITEM_SWITCHCMD_SCENE) ||
		(tItem._ItemType == TITEM_UPDATEDEVICE) ||
		(tItem._ItemType == TITEM_SET_VARIABLE)
		)
	{
		std::vector<_tTaskItem>::iterator itt = m_background_task_queue.begin();
		while (itt != m_background_task_queue.end())
		{
			_log.Debug(DEBUG_NORM, "SQLH AddTask: Comparing with item in queue: idx=%" PRId64 ", DelayTime=%f, Command='%s', Level=%d, Color='%s', RelatedEvent='%s'", itt->_idx, itt->_DelayTime, itt->_command.c_str(), itt->_level, itt->_Color.toString().c_str(), itt->_relatedEvent.c_str());
			if (itt->_idx == tItem._idx && itt->_ItemType == tItem._ItemType)
			{
				float iDelayDiff = tItem._DelayTime - itt->_DelayTime;
				if (iDelayDiff < (1. / timer_resolution_hz / 2))
				{
					_log.Debug(DEBUG_NORM, "SQLH AddTask: => Already present. Cancelling previous task item");
					itt = m_background_task_queue.erase(itt);
				}
				else
					++itt;
			}
			else
				++itt;
		}
	}
	// _log.Log(LOG_NORM, "=> Adding new task item");
	if (!cancelItem)
		m_background_task_queue.push_back(tItem);
}

void CSQLHelper::EventsGetTaskItems(std::vector<_tTaskItem> &currentTasks)
{
	std::lock_guard<std::mutex> l(m_background_task_mutex);

	currentTasks.clear();

	for (const auto & it : m_background_task_queue)
		currentTasks.push_back(it);
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

bool CSQLHelper::HandleOnOffAction(const bool bIsOn, const std::string &OnAction, const std::string &OffAction)
{
	if (bIsOn)
		_log.Debug(DEBUG_NORM, "SQLH HandleOnOffAction: OnAction:%s", OnAction.c_str());
	else
		_log.Debug(DEBUG_NORM, "SQLH HandleOnOffAction: OffAction:%s", OffAction.c_str());

	if (bIsOn)
	{
		if (OnAction.empty())
			return true;

		if ((OnAction.find("http://") == 0) || (OnAction.find("https://") == 0))
		{
			AddTaskItem(_tTaskItem::GetHTTPPage(0.2f, OnAction, "SwitchActionOn"));
		}
		else if (OnAction.find("script://") == 0)
		{
			//Execute possible script
			if (OnAction.find("../") != std::string::npos)
			{
				_log.Log(LOG_ERROR, "SQLHelper: Invalid script location! '%s'", OnAction.c_str());
				return false;
			}

			std::string scriptname = OnAction.substr(9);
#if !defined WIN32
			if (scriptname.find("/") != 0)
				scriptname = szUserDataFolder + "scripts/" + scriptname;
#endif
			std::string scriptparams = "";
			//Add parameters
			int pindex = scriptname.find(' ');
			if (pindex != std::string::npos)
			{
				scriptparams = scriptname.substr(pindex + 1);
				scriptname = scriptname.substr(0, pindex);
			}
			if (file_exist(scriptname.c_str()))
			{
				AddTaskItem(_tTaskItem::ExecuteScript(0.2f, scriptname, scriptparams));
			}
			else
				_log.Log(LOG_ERROR, "SQLHelper: Error script not found '%s'", scriptname.c_str());
		}
		return true;
	}

	//Off action
	if (OffAction.empty())
		return true;

	if ((OffAction.find("http://") == 0) || (OffAction.find("https://") == 0))
	{
		AddTaskItem(_tTaskItem::GetHTTPPage(0.2f, OffAction, "SwitchActionOff"));
	}
	else if (OffAction.find("script://") == 0)
	{
		//Execute possible script
		if (OffAction.find("../") != std::string::npos)
		{
			_log.Log(LOG_ERROR, "SQLHelper: Invalid script location! '%s'", OffAction.c_str());
			return false;
		}

		std::string scriptname = OffAction.substr(9);
#if !defined WIN32
		if (scriptname.find("/") != 0)
			scriptname = szUserDataFolder + "scripts/" + scriptname;
#endif
		std::string scriptparams = "";
		int pindex = scriptname.find(' ');
		if (pindex != std::string::npos)
		{
			scriptparams = scriptname.substr(pindex + 1);
			scriptname = scriptname.substr(0, pindex);
		}
		if (file_exist(scriptname.c_str()))
		{
			AddTaskItem(_tTaskItem::ExecuteScript(0.2f, scriptname, scriptparams));
		}
	}
	return true;
}

//Executed every hour
void CSQLHelper::CheckBatteryLow()
{
	int iBatteryLowLevel = 0;
	GetPreferencesVar("BatteryLowNotification", iBatteryLowLevel);
	if (iBatteryLowLevel == 0)
		return;//disabled

	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT ID,Name, BatteryLevel FROM DeviceStatus WHERE (Used!=0 AND BatteryLevel<%d AND BatteryLevel!=255)", iBatteryLowLevel);
	if (result.empty())
		return;

	time_t now = mytime(NULL);
	struct tm stoday;
	localtime_r(&now, &stoday);

	//check if last batterylow_notification is not sent today and if true, send notification
	for (const auto & itt : result)
	{
		std::vector<std::string> sd = itt;
		uint64_t ulID = std::strtoull(sd[0].c_str(), nullptr, 10);
		bool bDoSend = true;
		std::map<uint64_t, int>::const_iterator sitt;
		sitt = m_batterylowlastsend.find(ulID);
		if (sitt != m_batterylowlastsend.end())
		{
			bDoSend = (stoday.tm_mday != sitt->second);
		}
		if (bDoSend)
		{
			char szTmp[300];
			int batlevel = atoi(sd[2].c_str());
			if (batlevel == 0)
				sprintf(szTmp, "Battery Low: %s (Level: Low)", sd[1].c_str());
			else
				sprintf(szTmp, "Battery Low: %s (Level: %d %%)", sd[1].c_str(), batlevel);
			m_batterylowlastsend[ulID] = stoday.tm_mday;
		}
	}
}

//Executed every hour
void CSQLHelper::CheckDeviceTimeout()
{
	int TimeoutCheckInterval = 1;
	GetPreferencesVar("SensorTimeoutNotification", TimeoutCheckInterval);

	if (TimeoutCheckInterval == 0)
		return;
	m_sensortimeoutcounter += 1;
	if (m_sensortimeoutcounter < TimeoutCheckInterval)
		return;
	m_sensortimeoutcounter = 0;

	int SensorTimeOut = 60;
	GetPreferencesVar("SensorTimeout", SensorTimeOut);
	time_t now = mytime(NULL);
	struct tm stoday;
	localtime_r(&now, &stoday);
	now -= (SensorTimeOut * 60);
	struct tm ltime;
	localtime_r(&now, &ltime);

	std::vector<std::vector<std::string> > result;
	result = safe_query(
		"SELECT ID, Name, LastUpdate FROM DeviceStatus WHERE (Used!=0 AND LastUpdate<='%04d-%02d-%02d %02d:%02d:%02d' "
		"AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d "
		"AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d AND Type!=%d) "
		"ORDER BY Name",
		ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday, ltime.tm_hour, ltime.tm_min, ltime.tm_sec,
		pTypeLighting1,
		pTypeLighting2,
		pTypeLighting3,
		pTypeLighting4,
		pTypeLighting5,
		pTypeLighting6,
		pTypeFan,
		pTypeRadiator1,
		pTypeColorSwitch,
		pTypeSecurity1,
		pTypeCurtain,
		pTypeBlinds,
		pTypeRFY,
		pTypeChime,
		pTypeThermostat2,
		pTypeThermostat3,
		pTypeThermostat4,
		pTypeRemote,
		pTypeGeneralSwitch,
		pTypeHomeConfort,
		pTypeFS20,
		pTypeHunter
	);
	if (result.empty())
		return;

	//check if last timeout_notification is not sent today and if true, send notification
	for (const auto & itt : result)
	{
		std::vector<std::string> sd = itt;
		uint64_t ulID = std::strtoull(sd[0].c_str(), nullptr, 10);
		bool bDoSend = true;
		std::map<uint64_t, int>::const_iterator sitt;
		sitt = m_timeoutlastsend.find(ulID);
		if (sitt != m_timeoutlastsend.end())
		{
			bDoSend = (stoday.tm_mday != sitt->second);
		}
		if (bDoSend)
		{
			char szTmp[300];
			sprintf(szTmp, "Sensor Timeout: %s, Last Received: %s", sd[1].c_str(), sd[2].c_str());
			m_timeoutlastsend[ulID] = stoday.tm_mday;
		}
	}
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
								m_webservers.ReloadCustomSwitchIcons();
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
								m_webservers.ReloadCustomSwitchIcons();
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
								m_webservers.ReloadCustomSwitchIcons();
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
								m_webservers.ReloadCustomSwitchIcons();
							}
							return false;
						}
						rc = sqlite3_bind_blob(stmt, 1, pFBuf, fsize, SQLITE_STATIC);
						if (rc != SQLITE_OK) {
							ErrorMessage = "Problem inserting icon into database! " + std::string(sqlite3_errmsg(m_dbase));
							free(pFBuf);
							if (iTotalAdded > 0)
							{
								m_webservers.ReloadCustomSwitchIcons();
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
									m_webservers.ReloadCustomSwitchIcons();
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

	m_webservers.ReloadCustomSwitchIcons();
	return true;
}

std::map<std::string, std::string> CSQLHelper::BuildDeviceOptions(const std::string & options, const bool decode)
{
	std::map<std::string, std::string> optionsMap;
	if (!options.empty()) {
		//_log.Log(LOG_STATUS, "DEBUG : Build device options from '%s'...", options.c_str());
		std::vector<std::string> optionsArray;
		StringSplit(options, ";", optionsArray);
		for (const auto & itt : optionsArray)
		{
			std::string oValue = itt;
			if (oValue.empty())
				continue;
			size_t tpos = oValue.find_first_of(':');
			if ((tpos != std::string::npos) && (oValue.size() > tpos + 1))
			{
				std::string optionName = oValue.substr(0, tpos);
				oValue = oValue.substr(tpos + 1);
				std::string optionValue = decode ? base64_decode(oValue) : oValue;
				//_log.Log(LOG_STATUS, "DEBUG : Build device option ['%s': '%s'] => ['%s': '%s']", optionArray[0].c_str(), optionArray[1].c_str(), optionName.c_str(), optionValue.c_str());
				optionsMap.insert(std::pair<std::string, std::string>(optionName, optionValue));
			}
		}
	}
	//_log.Log(LOG_STATUS, "DEBUG : Build %d device(s) option(s)", optionsMap.size());
	return optionsMap;
}

std::map<std::string, std::string> CSQLHelper::GetDeviceOptions(const std::string & idx)
{
	std::map<std::string, std::string> optionsMap;

	if (idx.empty()) {
		_log.Log(LOG_ERROR, "Cannot set options on device %s", idx.c_str());
		return optionsMap;
	}

	uint64_t ulID = std::strtoull(idx.c_str(), nullptr, 10);
	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT Options FROM DeviceStatus WHERE (ID==%" PRIu64 ")", ulID);
	if (!result.empty()) {
		std::vector<std::string> sd = result[0];
		optionsMap = BuildDeviceOptions(sd[0].c_str());
	}
	return optionsMap;
}

std::string CSQLHelper::FormatDeviceOptions(const std::map<std::string, std::string> & optionsMap)
{
	std::string options;
	int count = optionsMap.size();
	if (count > 0) {
		int i = 0;
		std::stringstream ssoptions;
		for (const auto & itt : optionsMap)
		{
			i++;
			//_log.Log(LOG_STATUS, "DEBUG : Reading device option ['%s', '%s']", itt->first.c_str(), itt->second.c_str());
			std::string optionName = itt.first.c_str();
			std::string optionValue = base64_encode(itt.second);
			ssoptions << optionName << ":" << optionValue;
			if (i < count) {
				ssoptions << ";";
			}
		}
		options.assign(ssoptions.str());
	}

	return options;
}

bool CSQLHelper::SetDeviceOptions(const uint64_t idx, const std::map<std::string, std::string> & optionsMap)
{
	if (idx < 1) {
		_log.Log(LOG_ERROR, "Cannot set options on device %" PRIu64 "", idx);
		return false;
	}

	if (optionsMap.empty()) {
		//_log.Log(LOG_STATUS, "DEBUG : removing options on device %" PRIu64 "", idx);
		safe_query("UPDATE DeviceStatus SET Options = null WHERE (ID==%" PRIu64 ")", idx);
	}
	else {
		std::string options = FormatDeviceOptions(optionsMap);
		if (options.empty()) {
			_log.Log(LOG_ERROR, "Cannot parse options for device %" PRIu64 "", idx);
			return false;
		}
		//_log.Log(LOG_STATUS, "DEBUG : setting options '%s' on device %" PRIu64 "", options.c_str(), idx);
		safe_query("UPDATE DeviceStatus SET Options = '%q' WHERE (ID==%" PRIu64 ")", options.c_str(), idx);
	}
	return true;
}

float CSQLHelper::GetCounterDivider(const int metertype, const int dType, const float DefaultValue)
{
	float divider = float(DefaultValue);
	if (divider == 0)
	{
		int tValue;
		switch (metertype)
		{
		case MTYPE_ENERGY:
		case MTYPE_ENERGY_GENERATED:
			if (GetPreferencesVar("MeterDividerEnergy", tValue))
			{
				divider = float(tValue);
			}
			break;
		case MTYPE_GAS:
			if (GetPreferencesVar("MeterDividerGas", tValue))
			{
				divider = float(tValue);
			}
			break;
		case MTYPE_WATER:
			if (GetPreferencesVar("MeterDividerWater", tValue))
			{
				divider = float(tValue);
			}
			break;
		}
		if (dType == pTypeP1Gas)
			divider = 1000;
		else if ((dType == pTypeENERGY) || (dType == pTypePOWER))
			divider *= 100.0f;

		if (divider == 0) divider = 1.0f;
	}
	return divider;
}
