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

#define DB_VERSION 1

extern std::string szWWWFolder;

// Always create a integer primary key for tables if you can 'walk it' to another table.
// SQLite will create one anyway so it might as well have a name

const char* sqlCreateBackupLog =
		"CREATE TABLE IF NOT EXISTS [BackupLog] ("
			"[BackupLogID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT DEFAULT \"\","
			"[Number] INTEGER DEFAULT 0);";

const char* sqlCreatePreference =
		"CREATE TABLE IF NOT EXISTS [Preference] ("
			"[PreferenceID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE NOT NULL,"
			"[Value] Text)";

const char* sqlCreateInterface =
		"CREATE TABLE IF NOT EXISTS [Interface] ("
			"[InterfaceID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE DEFAULT Unknown, "
			"[Script] TEXT DEFAULT \"\","
			"[Configuration] TEXT DEFAULT \"\","
			"[Debug] INTEGER DEFAULT 0, "
			"[Notifiable] INTEGER DEFAULT 0, "
			"[Active] INTEGER DEFAULT 0);";

const char* sqlCreateInterfaceLog =
		"CREATE TABLE IF NOT EXISTS [InterfaceLog] ("
			"[InterfaceLogID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[InterfaceID] INTEGER NOT NULL, "
			"[Message] TEXT DEFAULT \"\","
			"[Timestamp] TEXT DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW')),"
		"FOREIGN KEY(InterfaceID) REFERENCES Interface(InterfaceID) ON DELETE CASCADE);";

const char* sqlInterfaceAfterInsertTrigger =
		"CREATE TRIGGER IF NOT EXISTS [InterfaceAfterInsertTrigger] AFTER INSERT ON Interface "
			"FOR EACH ROW "
			"BEGIN "
				"INSERT into InterfaceLog(InterfaceID, Message) values (NEW.InterfaceID, 'Interface \"' || New.Name || '\" created.'); "
			"END;";

const char* sqlInterfaceAfterUpdateTrigger =
		"CREATE TRIGGER IF NOT EXISTS[InterfaceAfterUpdateTrigger] AFTER UPDATE ON Interface "
			"FOR EACH ROW "
			"BEGIN "
				"INSERT into InterfaceLog(InterfaceID, Message) values(NEW.InterfaceID, "
					"CASE "
					"WHEN NEW.Name != OLD.Name THEN "
						"'Interface Name \"' || New.Name || '\" updated from \"' || OLD.Name || '\" to \"' || NEW.Name || '\".' "
					"WHEN NEW.Notifiable != OLD.Notifiable THEN "
						"'Interface Notifiable flag of \"' || New.Name || '\" updated from \"' || OLD.Notifiable || '\" to \"' || NEW.Notifiable || '\".' "
					"WHEN NEW.Active != OLD.Active THEN "
						"'Interface Active flag of \"' || New.Name || '\" updated from \"' || OLD.Active || '\" to \"' || NEW.Active || '\".' "
					"WHEN NEW.Configuration != OLD.Configuration THEN "
						"'Interface Configuration updated.' "
					"WHEN NEW.Script != OLD.Script THEN "
						"'Interface Script updated.' "
					"END "
					"); "
			"END;";

// Override cascading delete to force Devices to be deleted BEFORE Interface, make event processing logic work
const char* sqlInterfaceBeforeDeleteTrigger =
		"CREATE TRIGGER IF NOT EXISTS [InterfaceBeforeDeleteTrigger] BEFORE DELETE ON Interface "
			"FOR EACH ROW "
			"BEGIN "
				"DELETE FROM Device WHERE InterfaceID = OLD.InterfaceID; "
			"END;";

const char* sqlCreateDevice =
		"CREATE TABLE IF NOT EXISTS [Device] ("
			"[DeviceID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[InterfaceID] INTEGER NOT NULL, "
			"[Name] TEXT DEFAULT Unknown, "
			"[InternalID] TEXT DEFAULT \"\", "
			"[Address] TEXT DEFAULT \"\", "
			"[Debug] INTEGER DEFAULT 0, "
			"[Enabled] INTEGER DEFAULT 0, "
			"[Active] INTEGER DEFAULT 0, "
			"[Timestamp] TEXT DEFAULT CURRENT_TIMESTAMP,"
		"FOREIGN KEY(InterfaceID) REFERENCES Interface(InterfaceID) ON DELETE CASCADE);";

const char* sqlCreateDeviceLog =
		"CREATE TABLE IF NOT EXISTS [DeviceLog] ("
			"[DeviceLogID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[DeviceID] INTEGER NOT NULL, "
			"[Message] TEXT DEFAULT \"\","
			"[Timestamp] TEXT DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW')),"
		"FOREIGN KEY(DeviceID) REFERENCES Device(DeviceID) ON DELETE CASCADE);";

const char* sqlDeviceAfterInsertTrigger =
		"CREATE TRIGGER IF NOT EXISTS [DeviceAfterInsertTrigger] AFTER INSERT ON Device "
			"FOR EACH ROW "
			"BEGIN "
				"INSERT into DeviceLog(DeviceID, Message) values(NEW.DeviceID, 'Device \"' || New.Name || '\" created, ID '||New.DeviceID||', InternalID \"'||New.InternalID||'\".'); "
			"END;";

const char* sqlDeviceAfterUpdateTrigger =
		"CREATE TRIGGER IF NOT EXISTS [DeviceAfterUpdateTrigger] AFTER UPDATE ON Device "
			"FOR EACH ROW "
			"BEGIN "
				"INSERT into DeviceLog(DeviceID, Message) values(NEW.DeviceID, "
					"CASE "
						"WHEN (OLD.Name == NEW.Name) AND (OLD.InternalID == New.InternalID) AND (OLD.Active == New.Active) THEN "
							"'Device \"' || New.Name || '\" touched.' "
						"WHEN (OLD.Active != New.Active) AND (NEW.Active == 1) THEN "
							"'Device \"' || New.Name || '\" status changed: Active.' "
						"WHEN (OLD.Active != New.Active) AND (NEW.Active == 0) THEN "
							"'Device \"' || New.Name || '\" status changed: Inactive.' "
						"ELSE "
							"'Device \"' || New.Name || '\" updated, ID '||New.DeviceID||', InternalID \"'||New.InternalID||'\".' "
					"END "
				"); "
			"END;";

// Override cascading delete to force Values to be deleted BEFORE Device, make event processing logic work
const char* sqlDeviceBeforeDeleteTrigger =
		"CREATE TRIGGER IF NOT EXISTS [DeviceBeforeDeleteTrigger] BEFORE DELETE ON Device "
			"FOR EACH ROW "
			"BEGIN "
				"DELETE FROM Value WHERE DeviceID = OLD.DeviceID; "
			"END;";

const char* sqlCreateUnit =
	"CREATE TABLE IF NOT EXISTS [Unit] ("
			"[UnitID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE DEFAULT \"\", "
			"[Minimum] INTEGER DEFAULT 0, "
			"[Maximum] INTEGER DEFAULT 0, "
			"[RetentionDays] INTEGER DEFAULT 30, "
			"[RetentionInterval] INTEGER DEFAULT 900, "
			"[IconList] TEXT DEFAULT \"\", "
			"[TextLabels] TEXT DEFAULT \"\");";

const char* sqlCreateValue =
	"CREATE TABLE IF NOT EXISTS [Value] ("
			"[ValueID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT DEFAULT \"\", "
			"[InternalID] TEXT DEFAULT \"\", "
			"[DeviceID] INTEGER NOT NULL, "
			"[UnitID] INTEGER NOT NULL, "
			"[Value] TEXT DEFAULT \"\", "
			"[RetentionDays] INTEGER DEFAULT -1, "
			"[RetentionInterval] INTEGER DEFAULT 900, "
			"[Debug] INTEGER DEFAULT 0, "
			"[Timestamp] TEXT DEFAULT CURRENT_TIMESTAMP, "
		"FOREIGN KEY(UnitID) REFERENCES Unit(UnitID), "
		"FOREIGN KEY(DeviceID) REFERENCES Device(DeviceID) ON DELETE CASCADE);";

const char* sqlCreateValueLog =
	"CREATE TABLE IF NOT EXISTS [ValueLog] ("
			"[ValueLogID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[ValueID] INTEGER NOT NULL, "
			"[Message] TEXT DEFAULT \"\","
			"[Timestamp] TEXT DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW')),"
		"FOREIGN KEY(ValueID) REFERENCES Value(ValueID) ON DELETE CASCADE);";

const char* sqlCreateValueHistory =
	"CREATE TABLE IF NOT EXISTS [ValueHistory] ("
			"[ValueHistoryID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[ValueID] INTEGER NOT NULL, "
			"[Value] TEXT DEFAULT \"\", "
			"[Timestamp] TEXT DEFAULT CURRENT_TIMESTAMP, "
		"FOREIGN KEY(ValueID) REFERENCES Value(ValueID) ON DELETE CASCADE);";

const char* sqlValueAfterInsertTrigger =
	"CREATE TRIGGER IF NOT EXISTS [ValueAfterInsertTrigger] AFTER INSERT ON Value "
		"FOR EACH ROW "
		"BEGIN "
			"INSERT into ValueLog(ValueID, Message) values(NEW.ValueID, 'Value \"' || New.Name || '\" created.'); "
			"INSERT into ValueHistory(ValueID, Value) values(NEW.ValueID, NEW.Value); "
		"END;";

const char* sqlValueAfterUpdateTrigger =
	"CREATE TRIGGER IF NOT EXISTS [ValueAfterUpdateTrigger] AFTER UPDATE ON Value "
		"FOR EACH ROW "
		"WHEN (OLD.Value != NEW.Value) AND "
			" ((CAST(strftime('%s', CURRENT_TIMESTAMP) as integer) - "
			"	CAST(strftime('%s', (SELECT max(Timestamp) FROM ValueHistory WHERE ValueID = NEW.ValueID)) as integer)) "
			"	>= NEW.RetentionInterval) "
		"BEGIN "
			"INSERT into ValueLog(ValueID, Message) values(NEW.ValueID, 'Value \"' || New.Name || '\" updated from \"'||OLD.Value||'\" to \"'||NEW.Value||'\".'); "
			"INSERT into ValueHistory(ValueID, Value) VALUES (NEW.ValueID, NEW.Value); "
		"END;";

const char* sqlCreateValueScript =
	"CREATE TABLE IF NOT EXISTS [ValueScript] ("
			"[ValueScriptID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[ValueID] INTEGER NOT NULL, "
			"[Update] TEXT DEFAULT NULL, "
			"[Periodic] TEXT DEFAULT NULL, "
		"FOREIGN KEY(ValueID) REFERENCES Value(ValueID) ON DELETE CASCADE);";

const char* sqlCreateValueNotification =
	"CREATE TABLE IF NOT EXISTS [ValueNotification] ("
			"[ValueNotificationID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[ValueID] INTEGER NOT NULL, "
			"[InterfaceID] INTEGER NOT NULL, "
			"[Script] TEXT DEFAULT \"\", "
			"FOREIGN KEY(ValueID) REFERENCES Value(ValueID) ON DELETE CASCADE, "
			"FOREIGN KEY(InterfaceID) REFERENCES Interface(InterfaceID));";

const char* sqlCreateTimerPlan =
	"CREATE TABLE IF NOT EXISTS [TimerPlan] ("
			"[TimerPlanID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE DEFAULT Unknown, "
			"[Active] INTEGER DEFAULT 0);";

const char* sqlCreateValueTimer =
	"CREATE TABLE IF NOT EXISTS [ValueTimer] ("
			"[ValueTimerID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[TimerPlanID] INTEGER NOT NULL, "
			"[ValueID] INTEGER NOT NULL, "
			"[DayMask] TEXT DEFAULT \"Mon,Tue,Wed,Thu,Fri,Sat,Sun\", "
			"[Time] TEXT DEFAULT \"00:00:00\", "
			"[Sunrise] INTEGER DEFAULT 0, "
			"[Sunset] INTEGER DEFAULT 0, "
			"[Random] INTEGER DEFAULT 0, "
		"FOREIGN KEY(TimerPlanID) REFERENCES TimerPlan(TimerPlanID), "
		"FOREIGN KEY(ValueID) REFERENCES Value(ValueID) ON DELETE CASCADE);";

const char* sqlCreateScene =
	"CREATE TABLE IF NOT EXISTS [Scene] ("
			"[SceneID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE NOT NULL, "
			"[Active] INTEGER DEFAULT 0);";

const char* sqlCreateSceneValue =
	"CREATE TABLE IF NOT EXISTS [SceneValue] ("
			"[SceneValueID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[SceneID] INTEGER NOT NULL, "
			"[ValueID] INTEGER NOT NULL, "
			"[Value] TEXT NOT NULL, "
			"[Order] INTEGER DEFAULT 0, "
			"[Delay] INTEGER DEFAULT 0, "
			"FOREIGN KEY(SceneID) REFERENCES Scene(SceneID) ON DELETE CASCADE, "
			"FOREIGN KEY(ValueID) REFERENCES Value(ValueID) ON DELETE CASCADE);";

const char* sqlCreateRole =
	"CREATE TABLE IF NOT EXISTS [Role] ("
			"[RoleID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE NOT NULL, "
			"[RemoteAccess] INTEGER DEFAULT 0, "
			"[InternalTTL] INTEGER DEFAULT 43200, "  // 30 days (in minutes)
			"[RemoteTTL] INTEGER DEFAULT 0);";

const char* sqlCreateUser =
	"CREATE TABLE IF NOT EXISTS [User] ("
			"[UserID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[UserName] TEXT UNIQUE NOT NULL, "
			"[Password] TEXT DEFAULT \"\", "
			"[Name] TEXT NOT NULL, "
			"[RoleID] INTEGER NOT NULL, "
			"[Active] INTEGER DEFAULT 0, "
			"[ForceChange] INTEGER DEFAULT 1, "
			"[FailedAttempts] INTEGER DEFAULT 0, "
			"[EmailAddress] TEXT DEFAULT \"\", "
			"[MobileNumber] TEXT DEFAULT \"\", "
			"[Theme] TEXT DEFAULT \"\", "
			"[Timestamp] TEXT DEFAULT CURRENT_TIMESTAMP, "
		"FOREIGN KEY(RoleID) REFERENCES Role(RoleID) ON DELETE CASCADE);";

const char* sqlCreateSession =
	"CREATE TABLE IF NOT EXISTS [Session] ("
			"[SessionID] TEXT NOT NULL, "
			"[AuthToken] TEXT NOT NULL, "
			"[UserID] INTEGER NOT NULL, "
			"[RoleID] INTEGER NOT NULL, "
			"[Expiry] TEXT DEFAULT CURRENT_TIMESTAMP, "
		"FOREIGN KEY(UserID) REFERENCES User(UserID) ON DELETE CASCADE, "
		"FOREIGN KEY(RoleID) REFERENCES Role(RoleID) ON DELETE CASCADE);";

const char* sqlCreateTableAccess =
	"CREATE TABLE IF NOT EXISTS [TableAccess] ("
			"[TableAccessID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT NOT NULL, "
			"[RoleID] INTEGER NOT NULL, "
			"[CanGET] TEXT DEFAULT 0, "
			"[CanPOST] TEXT DEFAULT 0, "
			"[CanPUT] TEXT DEFAULT 0, "
			"[CanPATCH] TEXT DEFAULT 0, "
			"[CanDELETE] TEXT DEFAULT 0, "
			"[DontGETFields] TEXT DEFAULT \"\", "
			"[PUTFields] TEXT DEFAULT \"*\", "
			"[PATCHFields] TEXT DEFAULT \"\", "
		"FOREIGN KEY(RoleID) REFERENCES Role(RoleID) ON DELETE CASCADE);";

const char* sqlCreateStandardScript =
	"CREATE TABLE IF NOT EXISTS [StandardScript] ("
			"[StandardScriptID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE DEFAULT Unknown, "
			"[Script] TEXT DEFAULT \"\","
			"[Active] INTEGER DEFAULT 0);";

const char* sqlCreateLayout =
	"CREATE TABLE IF NOT EXISTS [Layout] ("
			"[LayoutID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[Name] TEXT UNIQUE DEFAULT Unknown, "
			"[MinimumWidth] INTEGER DEFAULT 0, "
			"[Active] INTEGER DEFAULT 0);";

const char* sqlCreateTab =
	"CREATE TABLE IF NOT EXISTS [Tab] ("
			"[TabID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[LayoutID] INTEGER NOT NULL, "
			"[Name] TEXT DEFAULT Unknown, "
			"[DisplayOrder] INTEGER DEFAULT 0, "
			"[Icon] TEXT DEFAULT '', "
			"[Background] TEXT DEFAULT '', "
			"[RowHeight] INTEGER DEFAULT 100, "
			"[MinColumns] INTEGER DEFAULT 4, "
			"[MaxColumns] INTEGER DEFAULT 8, "
			"[MinColWidth] INTEGER DEFAULT 90, "
			"[MaxColWidth] INTEGER DEFAULT 180, "
			"[GutterSize] INTEGER DEFAULT 5, "
		"FOREIGN KEY(LayoutID) REFERENCES Layout(LayoutID) ON DELETE CASCADE);";

const char* sqlCreateTile =
	"CREATE TABLE IF NOT EXISTS [Tile] ("
			"[TileID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[TabID] INTEGER NOT NULL, "
			"[Name] TEXT UNIQUE DEFAULT Unknown, "
			"[DisplayOrder] INTEGER DEFAULT 0, "
			"[RowSpan] INTEGER DEFAULT 1, "
			"[ColumnSpan] INTEGER DEFAULT 1, "
			"[BorderRadius] TEXT DEFAULT '', "
			"[Background] TEXT DEFAULT '', "
		"FOREIGN KEY(TabID) REFERENCES Tab(TabID) ON DELETE CASCADE);";

const char* sqlCreateElement =
	"CREATE TABLE IF NOT EXISTS [Element] ("
			"[ElementID] INTEGER PRIMARY KEY AUTOINCREMENT, "
			"[TileID] INTEGER NOT NULL, "
			"[Name] TEXT DEFAULT Unknown, "
			"[ValueID] INTEGER NOT NULL, "
			"[Type] TEXT DEFAULT Unknown, "
			"[Class] TEXT DEFAULT '', "
			"[ContainingElementID] INTEGER DEFAULT 0, "
			"[RelativePosition] INTEGER DEFAULT 1, "
			"[OffsetX] INTEGER DEFAULT -1, "
			"[OffsetY] INTEGER DEFAULT -1, "
			"[Height] INTEGER DEFAULT -1, "
			"[Width] INTEGER DEFAULT -1, "
			"[ScreenUnits] TEXT DEFAULT 'px', "
			"[BorderRadius] INTEGER DEFAULT 0, "
			"[Padding] TEXT DEFAULT '', "
			"[Margin] TEXT DEFAULT '', "
			"[Colour] TEXT DEFAULT '', "
			"[Transparency] INTEGER DEFAULT 0, "
			"[ZIndex] INTEGER DEFAULT 0, "
			"[URL] TEXT DEFAULT '', "
			"[Javascript] TEXT DEFAULT '', "
			"[Action] TEXT DEFAULT '', "
			"[RefreshSeconds] INTEGER DEFAULT -1, "
		"FOREIGN KEY(ContainingElementID) REFERENCES Element(ElementID) ON DELETE CASCADE,"
		"FOREIGN KEY(ValueID) REFERENCES Value(ValueID),"
		"FOREIGN KEY(TileID) REFERENCES Tile(TileID) ON DELETE CASCADE);";

extern std::string szUserDataFolder;

CSQLStatement::CSQLStatement(sqlite3* pDBase, const std::string& pSQL) : m_DBase(pDBase), m_Statement(nullptr), iNextParam(1), m_Status(SQLITE_OK)
{
	const char* pTail;
	int	iRetVal = sqlite3_prepare_v3(m_DBase, pSQL.c_str(), pSQL.length(), 0, &m_Statement, &pTail);
	if (iRetVal != SQLITE_OK)
	{
		m_Status = iRetVal;
		m_ErrorText = sqlite3_errmsg(m_DBase);
	}
}

int	CSQLStatement::AddParameter(std::string& pParam)
{
	std::string		sText = pParam;
	// Strip delimiters if supplied
	if (((sText[0] == '\'') && (sText[sText.length()-1] == '\'')) ||
		 (sText[0] == '\"') && (sText[sText.length() - 1] == '\"'))
	{
		sText = pParam.substr(1, pParam.size() - 2);;
	}

	int	iRetVal = sqlite3_bind_text(m_Statement, iNextParam++, sText.c_str(), sText.length(), SQLITE_TRANSIENT);
	if (iRetVal != SQLITE_OK)
	{
		m_Status = iRetVal;
		m_ErrorText = sqlite3_errmsg(m_DBase);
	}
	return iRetVal;
}

int CSQLStatement::Execute()
{
	int	iRetVal = sqlite3_step(m_Statement);
	if (iRetVal != SQLITE_DONE)
	{
		m_Status = iRetVal;
		m_ErrorText = sqlite3_errmsg(m_DBase);
	}
	return iRetVal;
}

bool CSQLStatement::Error()
{
	return (m_Status != SQLITE_OK) && (m_Status != SQLITE_DONE);
};

CSQLStatement::~CSQLStatement()
{
	if (m_Statement)
	{
		sqlite3_finalize(m_Statement);
	}
}

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
	rc = sqlite3_busy_timeout(m_dbase, 1000);	// Required in case any plugins do database changes that generate log updates (makes connection wait 1 second on db lock)
	if (rc)
	{
		_log.Log(LOG_ERROR, "Error setting busy timeout on SQLite3 database: %s", sqlite3_errmsg(m_dbase));
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
		GetPreferencesVar("DB_Version", &dbversion, 0);
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
	query(sqlCreateBackupLog);
	query(sqlCreatePreference);
	query(sqlCreateInterface);
	query(sqlCreateInterfaceLog);
	query(sqlInterfaceAfterInsertTrigger);
	query(sqlInterfaceAfterUpdateTrigger);
	query(sqlInterfaceBeforeDeleteTrigger);
	query(sqlCreateDevice);
	query(sqlCreateDeviceLog);
	query(sqlDeviceAfterInsertTrigger);
	query(sqlDeviceAfterUpdateTrigger);
	query(sqlDeviceBeforeDeleteTrigger);
	query(sqlCreateUnit);
	query(sqlCreateValue);
	query(sqlCreateValueHistory);
	query(sqlCreateValueLog);
	query(sqlValueAfterInsertTrigger);
	query(sqlValueAfterUpdateTrigger);
	query(sqlCreateValueScript);
	query(sqlCreateValueNotification);
	query(sqlCreateStandardScript);

	query(sqlCreateTimerPlan);
	query(sqlCreateValueTimer);

	query(sqlCreateScene);
	query(sqlCreateSceneValue);

	query(sqlCreateRole);
	query(sqlCreateUser);
	query(sqlCreateSession);
	query(sqlCreateTableAccess);

	query(sqlCreateLayout);
	query(sqlCreateTab);
	query(sqlCreateTile);
	query(sqlCreateElement);

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
		query("INSERT INTO Role (Name) VALUES ('Anonymous')");
		query("INSERT INTO Role (Name, RemoteAccess, RemoteTTL) VALUES ('User', true, 15)");
		query("INSERT INTO Role (Name, RemoteAccess, InternalTTL, RemoteTTL) VALUES ('Administrator', true, 15, 5)");

		// Add a couple of users
		query("INSERT INTO User (Username, Password, Name, Theme, Active, ForceChange, RoleID) SELECT 'Anonymous','Anonymous','Anonymous User', 'indigo', true, false, RoleID FROM Role WHERE Name = 'Anonymous'");
		query("INSERT INTO User (Username, Password, Name, Theme, Active, ForceChange, RoleID) SELECT 'Admin','Admin ','Administrative User', 'deep-purple', true, false, RoleID FROM Role WHERE Name = 'Administrator'");

		// Give access to tables
		query("INSERT INTO TableAccess (Name,RoleID,CanGET) SELECT A.name, B.RoleID, true FROM sqlite_master A, Role B WHERE (A.type='table' AND A.name <> 'sqlite_sequence' AND A.name NOT LIKE 'User%' AND A.name NOT LIKE 'TableAccess%') AND (B.Name <> 'Administrator')");
		query("INSERT INTO TableAccess (Name,RoleID,CanGET,CanPOST,CanPUT,CanPATCH,CanDELETE) SELECT A.name, B.RoleID, true, true, true, false, true FROM sqlite_master A, Role B WHERE(A.type = 'table' and A.name <> 'sqlite_sequence') AND(B.Name = 'Administrator')");
		query("UPDATE TableAccess SET CanPATCH=true, PATCHFields='Value' WHERE Name='Value'");
		query("UPDATE TableAccess SET CanPATCH=true, PATCHFields='Password', DontGETFields='Password' WHERE Name='User'");
		query("UPDATE TableAccess SET CanPOST=false, CanPATCH=false, CanDELETE=false WHERE Name='TableAccess'");

		// Units that Values can be associated with
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('On/Off', 0, 1, 'Push48_Off.png,Push48_On.png', 'Off,On')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Light On/Off', 0, 1, 'Light48_Off.png,Light48_On.png', 'Off,On')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Fan On/Off', 0, 1, 'Fan48_Off.png,Fan48_On.png', 'Off,On')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Laptop On/Off', 0, 1, 'Computer48_Off.png,Computer48_On.png', 'Off,On')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Desktop On/Off', 0, 1, 'ComputerPC48_Off.png,ComputerPC48_On.png', 'Off,On')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Contact Open/Close', 0, 1, 'Contact48_Off.png,Contact48_On.png', 'Closed,Open')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Door Open/Close', 0, 1, 'Door48_Off.png,Door48_On.png', 'Closed,Open')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Dimmer', 0, 100, 'Dimmer48_Off.png,Dimmer48_On.png', 'Off,On')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Percentage', 0, 100, 'Percentage48.png', '%')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Celsius', -25, 100, 'temp48.png', '°C')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Temperature', 0, 6, 'temp-0-5.png,temp-5-10.png,temp-10-15.png,temp-15-20.png,temp-20-25.png,temp-25-30.png,temp-gt-30.png', 'Cold,Chilly,Cool,Mild,Warm,Hot,Baking')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, IconList, TextLabels) VALUES ('Wind Direction', 0, 15, 'WindN.png,WindNNE.png,WindNE.png,WindENE.png,WindE.png,WindESE.png,WindSE.png,WindSSE.png,WindS.png,WindSSW.png,WindSW.png,WindWSW.png,WindW.png,WindWNW.png,WindNW.png,WindNNW.png', 'N,NNE,NE,ENE,E,ESE,SE,SSE,S,SSW,SW,WSW,W,WNW,NW,NNW')");
		query("INSERT INTO Unit (Name, Minimum, Maximum, RetentionDays, RetentionInterval, IconList, TextLabels) VALUES ('Audible', 0, 1, 1, 300, 'Speaker48_Off.png,Speaker48_On.png', 'Muted,On')");
		query("INSERT INTO Unit (Name) VALUES ('Text')");
		query("INSERT INTO Unit (Name, IconList, TextLabels) VALUES ('Media Type', 'Media48_Off.png,Media48_On.png', 'None,Audio,Video')");
		query("INSERT INTO Unit (Name, IconList, TextLabels) VALUES ('Media Status', 'Media48_Off.png,Media48_On.png', 'None,Playing,Paused')");
		query("INSERT INTO Unit (Name, IconList, TextLabels) VALUES ('Device Power', 'Push48_Off.png,pushon48.png,Push48_On.png', 'Off,Standby,On')");

		sqlite3_wal_checkpoint(m_dbase, NULL);
	}
	UpdatePreferencesVar("DB_Version", std::to_string(DB_VERSION));

	//Make sure we have some default preferences
	int nValue = 0;
	std::string sValue;
	std::string	sTrue = "True";
	std::string	sDomoticz = "Domoticz";
	std::string	sLang = "en";
	GetPreferencesVar("Title", sValue, sDomoticz);
	GetPreferencesVar("UseAutoUpdate", sValue, sTrue);
	GetPreferencesVar("UseAutoBackup", sValue, sTrue);
	GetPreferencesVar("Language", sValue, sLang);
	GetPreferencesVar("AuthenticationMethod", &nValue, 0);
	GetPreferencesVar("AcceptNewHardware", sValue, sTrue);
	m_bAcceptNewHardware = (sValue == "True");

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
				if (m_bAcceptNewHardware)
				{
					std::string	sTrue = "True";
					UpdatePreferencesVar("AcceptNewHardware", sTrue);
				}
				else
				{
					std::string	sFalse = "False";
					UpdatePreferencesVar("AcceptNewHardware", sFalse);
				}
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

int CSQLHelper::execute_sql(const std::string& sSQL, std::vector<std::string>* pValues, bool bLogError)
{
	CSQLStatement	sqlStatement(m_dbase, sSQL);
	std::vector<std::vector<std::string> >	result;
	for (unsigned int i = 0; (i < pValues->size()) && (!sqlStatement.Error()); i++)
	{
		sqlStatement.AddParameter((*pValues)[i]);
	}

	if (!sqlStatement.Error())
	{
		sqlStatement.Execute();
	}

	if (!sqlStatement.Error())
	{
		result = m_sql.safe_query("SELECT changes();");
	}
	else
	{
		if (bLogError)
		{
			_log.Log(LOG_ERROR, "Error performing operation: '%s'", sqlStatement.ErrorText());
		}
	}

	if (result.empty() || result[0][0] == "0")
		return 0;
	else
		return atoi(result[0][0].c_str());
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

void CSQLHelper::UpdatePreferencesVar(const std::string &Name, const std::string &Value)
{
	if (!m_dbase)
		return;

	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT ROWID FROM Preference WHERE (Name='%q')",
		Name.c_str());
	if (result.empty())
	{
		//Insert
		result = safe_query("INSERT INTO Preference (Name, Value) VALUES ('%q','%q')",
			Name.c_str(), Value.c_str());
	}
	else
	{
		//Update
		result = safe_query("UPDATE Preference SET Name='%q', Value='%q' WHERE (ROWID = '%q')",
			Name.c_str(), Value.c_str(), result[0][0].c_str());
	}
}

bool CSQLHelper::GetPreferencesVar(const std::string &Name, std::string& Value, std::string &Default)
{
	if (!m_dbase)
		return false;

	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT Value FROM Preference WHERE (Name='%q')",
		Name.c_str());
	if (result.empty())
	{
		Value = Default;
		UpdatePreferencesVar(Name, Value);
		return true;
	}
	std::vector<std::string> sd = result[0];
	Value = sd[0];
	return true;
}

bool CSQLHelper::GetPreferencesVar(const std::string& Name, int* Value, int Default)
{
	if (!m_dbase)
		return false;

	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT Value FROM Preference WHERE (Name='%q')",
		Name.c_str());
	if (result.empty())
	{
		*Value = Default;
		UpdatePreferencesVar(Name, std::to_string(*Value));
		return true;
	}
	std::vector<std::string> sd = result[0];
	*Value = atoi(sd[0].c_str());
	return true;
}

void CSQLHelper::DeletePreferencesVar(const std::string &Name)
{
	safe_query("DELETE FROM Preference WHERE (Name='%q')", Name.c_str());
}

int CSQLHelper::GetLastBackupNo(const char *Name, int &nValue)
{
	if (!m_dbase)
		return false;

	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT Number FROM BackupLog WHERE (Name='%q')", Name);
	if (result.empty())
		return -1;
	std::vector<std::string> sd = result[0];
	nValue = atoi(sd[0].c_str());
	return nValue;
}

void CSQLHelper::SetLastBackupNo(const char *Name, const int nValue)
{
	if (!m_dbase)
		return;

	std::vector<std::vector<std::string> > result;
	result = safe_query("SELECT BackupLogID FROM BackupLog WHERE (Name='%q')", Name);
	if (result.empty())
	{
		//Insert
		safe_query(
			"INSERT INTO BackupLog (Name, Number) "
			"VALUES ('%q','%d')",
			Name,
			nValue);
	}
	else
	{
		//Update
		uint64_t ID = std::strtoull(result[0][0].c_str(), nullptr, 10);

		safe_query(
			"UPDATE BackupLog SET Name='%q', Number=%d "
			"WHERE (ROWID = %" PRIu64 ")",
			Name,
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
	if (GetPreferencesVar("5MinuteHistoryDays", &n5MinuteHistoryDays, 1))
	{
		// If the history days is zero then all data in the short logs is deleted!
		if (n5MinuteHistoryDays == 0)
		{
			_log.Log(LOG_ERROR, "CleanupShortLog(): MinuteHistoryDays is zero!");
			return;
		}
		std::string szQueryFilter = "strftime('%s',datetime('now','localtime')) - strftime('%s',Date) > (SELECT p.nValue * 86400 From Preferences AS p WHERE p.Name='5MinuteHistoryDays')";
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
	GetPreferencesVar("LightHistoryDays", &nMaxDays, 30);

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
	ss << "SELECT sValue FROM Preferences WHERE (Name='DB_Version')";
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
