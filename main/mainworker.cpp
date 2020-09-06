#include "stdafx.h"
#include "mainworker.h"
#include "Helper.h"
#include "SunRiseSet.h"
#include "localtime_r.h"
#include "Logger.h"
#include "../web/WebServerHelper.h"
#include "SQLHelper.h"

#include <boost/algorithm/string/join.hpp>

#include <boost/crc.hpp>
#include <algorithm>
#include <set>

//Hardware Devices
#include "../hardware/plugins/Plugins.h"

#ifdef WIN32
#include "../msbuild/WindowsHelper.h"
#include "dirent_windows.h"
#else
#include <sys/utsname.h>
#include <dirent.h>
#endif

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define round(a) ( int ) ( a + .5 )

extern std::string szStartupFolder;
extern std::string szUserDataFolder;
extern std::string szWWWFolder;
extern std::string szAppVersion;
extern std::string szWebRoot;
extern bool g_bUseUpdater;

MainWorker::MainWorker()
{
	m_SecCountdown = -1;

	m_bStartHardware = false;
	m_hardwareStartCounter = 0;

	m_bIgnoreUsernamePassword = false;

	time_t atime = mytime(NULL);
	m_LastHeartbeat = atime;
	struct tm ltime;
	localtime_r(&atime, &ltime);
	m_ScheduleLastMinute = ltime.tm_min;
	m_ScheduleLastHour = ltime.tm_hour;
	m_ScheduleLastMinuteTime = 0;
	m_ScheduleLastHourTime = 0;
	m_ScheduleLastDayTime = 0;
	m_LastSunriseSet = "";
	m_DayLength = "";

	m_bHaveDownloadedDomoticzUpdate = false;
	m_bHaveDownloadedDomoticzUpdateSuccessFull = false;
	m_bDoDownloadDomoticzUpdate = false;
	m_LastUpdateCheck = 0;
	m_bHaveUpdate = false;
	m_iRevision = 0;

	m_bForceLogNotificationCheck = false;

	// default values for log entry retention
	m_sInterfaceRowsToKeep = "1000";
	m_sDeviceRowsToKeep = "500";
	m_sValueRowsToKeep = "100";
}

MainWorker::~MainWorker()
{
	Stop();
}

void MainWorker::AddAllDomoticzHardware()
{
	//Add Interfaces
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query("SELECT InterfaceID, Name FROM Interface WHERE Active = 1 ORDER BY InterfaceID ASC");
	if (!result.empty())
	{
		for (const auto & itt : result)
		{
			std::vector<std::string> sd = itt;

			int ID = atoi(sd[0].c_str());
			std::string Name = sd[1];

			CDomoticzHardwareBase* pHardware = NULL;
			pHardware = m_pluginsystem.RegisterPlugin(ID, Name);
			if (pHardware)
			{
				pHardware->m_Name = Name;
				pHardware->m_ShortName = "Python Plugin";
				pHardware->m_DataTimeout = 3600; 
				AddDomoticzHardware(pHardware);
			}
		}
		m_hardwareStartCounter = 0;
		m_bStartHardware = true;
	}
	else
	{
		// Even if there is no hardware tell the Plugin Framework it is okay to start
		m_pluginsystem.AllPluginsStarted();
	}
}

void MainWorker::StartDomoticzHardware()
{
	std::vector<CDomoticzHardwareBase*>::iterator itt;
	for (auto & itt : m_hardwaredevices)
	{
		if (!itt->IsStarted())
		{
			itt->Start();
		}
	}
	m_pluginsystem.AllPluginsStarted();
}

void MainWorker::StopDomoticzHardware()
{
	// Separate the Stop() from the device removal from the vector.
	// Some actions the hardware might take during stop (e.g updating a device) can cause deadlocks on the m_devicemutex
	std::vector<CDomoticzHardwareBase*> OrgHardwaredevices;
	{
		std::lock_guard<std::mutex> l(m_devicemutex);
		for (auto & itt : m_hardwaredevices)
		{
			OrgHardwaredevices.push_back(itt);
		}
		m_hardwaredevices.clear();
	}

	for (auto & itt : OrgHardwaredevices)
	{
#ifdef ENABLE_PYTHON
		m_pluginsystem.DeregisterPlugin(itt->m_InterfaceID);
#endif
		itt->Stop();
		delete itt;
	}
}

void MainWorker::GetAvailableWebThemes()
{
	std::string ThemeFolder = szWWWFolder + "/styles/";
	m_webthemes.clear();
	DirectoryListing(m_webthemes, ThemeFolder, true, false);

	//check if current theme is found, if not, select default
	bool bFound = false;
	std::string sValue;
	std::string	sDefault = "default";
	if (m_sql.GetPreferencesVar("WebTheme", sValue, sDefault))
	{
		for (const auto & itt : m_webthemes)
		{
			if (itt == sValue)
			{
				bFound = true;
				break;
			}
		}
	}
	if (!bFound)
	{
		m_sql.UpdatePreferencesVar("WebTheme", "default");
	}
}

void MainWorker::AddDomoticzHardware(CDomoticzHardwareBase *pHardware)
{
	int devidx = FindDomoticzHardware(pHardware->m_InterfaceID);
	if (devidx != -1) //it is already there!, remove it
	{
		RemoveDomoticzHardware(m_hardwaredevices[devidx]);
	}
	std::lock_guard<std::mutex> l(m_devicemutex);
	m_hardwaredevices.push_back(pHardware);
}

void MainWorker::RemoveDomoticzHardware(CDomoticzHardwareBase *pHardware)
{
	// Separate the Stop() from the device removal from the vector.
	// Some actions the hardware might take during stop (e.g updating a device) can cause deadlocks on the m_devicemutex
	CDomoticzHardwareBase *pOrgHardware = NULL;
	{
		std::lock_guard<std::mutex> l(m_devicemutex);
		std::vector<CDomoticzHardwareBase*>::iterator itt;
		for (itt = m_hardwaredevices.begin(); itt != m_hardwaredevices.end(); ++itt)
		{
			pOrgHardware = *itt;
			if (pOrgHardware == pHardware) {
				m_hardwaredevices.erase(itt);
				break;
			}
		}
	}

	if (pOrgHardware == pHardware)
	{
		try
		{
			pOrgHardware->Stop();
			delete pOrgHardware;
		}
		catch (std::exception& e)
		{
			_log.Log(LOG_ERROR, "Mainworker: Exception: %s (%s:%d)", e.what(), std::string(__func__).substr(std::string(__func__).find_last_of("/\\") + 1).c_str(), __LINE__);
		}
		catch (...)
		{
			_log.Log(LOG_ERROR, "Mainworker: Exception catched! %s:%d", std::string(__func__).substr(std::string(__func__).find_last_of("/\\") + 1).c_str(), __LINE__);
		}
	}
}

void MainWorker::RemoveDomoticzHardware(int HwdId)
{
	int dpos = FindDomoticzHardware(HwdId);
	if (dpos == -1)
		return;
	m_pluginsystem.DeregisterPlugin(HwdId);
	RemoveDomoticzHardware(m_hardwaredevices[dpos]);
}

int MainWorker::FindDomoticzHardware(int HwdId)
{
	std::lock_guard<std::mutex> l(m_devicemutex);
	int ii = 0;
	for (const auto & itt : m_hardwaredevices)
	{
		if (itt->m_InterfaceID == HwdId)
			return ii;
		ii++;
	}
	return -1;
}

CDomoticzHardwareBase* MainWorker::GetHardware(int HwdId)
{
	std::lock_guard<std::mutex> l(m_devicemutex);
	for (auto & itt : m_hardwaredevices)
	{
		if (itt->m_InterfaceID == HwdId)
			return itt;
	}
	return NULL;
}

// sunset/sunrise
// http://www.earthtools.org/sun/<latitude>/<longitude>/<day>/<month>/<timezone>/<dst>
// example:
// http://www.earthtools.org/sun/52.214268/5.171002/11/11/99/1

bool MainWorker::GetSunSettings()
{
	std::string sValue;
	std::vector<std::string> strarray;
	std::string	sBlank = "";
	if (m_sql.GetPreferencesVar("Location", sValue, sBlank))
		StringSplit(sValue, ";", strarray);

	if (strarray.size() != 2)
	{
		// No location entered in the settings, lets just reload our schedules and return
		// Load non sun settings timers
		return false;
	}

	std::string Latitude = strarray[0];
	std::string Longitude = strarray[1];

	time_t atime = mytime(NULL);
	struct tm ltime;
	localtime_r(&atime, &ltime);

	int year = ltime.tm_year + 1900;
	int month = ltime.tm_mon + 1;
	int day = ltime.tm_mday;

	double dLatitude = atof(Latitude.c_str());
	double dLongitude = atof(Longitude.c_str());

	SunRiseSet::_tSubRiseSetResults sresult;
	SunRiseSet::GetSunRiseSet(dLatitude, dLongitude, year, month, day, sresult);

	std::string sunrise;
	std::string sunset;
	std::string daylength;
	std::string sunatsouth;
	std::string civtwstart;
	std::string civtwend;
	std::string nauttwstart;
	std::string nauttwend;
	std::string asttwstart;
	std::string asttwend;

	char szRiseSet[30];
	sprintf(szRiseSet, "%02d:%02d:00", sresult.SunRiseHour, sresult.SunRiseMin);
	sunrise = szRiseSet;
	sprintf(szRiseSet, "%02d:%02d:00", sresult.SunSetHour, sresult.SunSetMin);
	sunset = szRiseSet;
	sprintf(szRiseSet, "%02d:%02d:00", sresult.DaylengthHours, sresult.DaylengthMins);
	daylength = szRiseSet;
	sprintf(szRiseSet, "%02d:%02d:00", sresult.SunAtSouthHour, sresult.SunAtSouthMin);
	sunatsouth = szRiseSet;
	sprintf(szRiseSet, "%02d:%02d:00", sresult.CivilTwilightStartHour, sresult.CivilTwilightStartMin);
	civtwstart = szRiseSet;
	sprintf(szRiseSet, "%02d:%02d:00", sresult.CivilTwilightEndHour, sresult.CivilTwilightEndMin);
	civtwend = szRiseSet;
	sprintf(szRiseSet, "%02d:%02d:00", sresult.NauticalTwilightStartHour, sresult.NauticalTwilightStartMin);
	nauttwstart = szRiseSet;
	sprintf(szRiseSet, "%02d:%02d:00", sresult.NauticalTwilightEndHour, sresult.NauticalTwilightEndMin);
	nauttwend = szRiseSet;
	sprintf(szRiseSet, "%02d:%02d:00", sresult.AstronomicalTwilightStartHour, sresult.AstronomicalTwilightStartMin);
	asttwstart = szRiseSet;
	sprintf(szRiseSet, "%02d:%02d:00", sresult.AstronomicalTwilightEndHour, sresult.AstronomicalTwilightEndMin);
	asttwend = szRiseSet;

	//m_scheduler.SetSunRiseSetTimers(sunrise, sunset, sunatsouth, civtwstart, civtwend, nauttwstart, nauttwend, asttwstart, asttwend); // Do not change the order

	bool bFirstTime = m_LastSunriseSet.empty();

	std::string riseset = sunrise.substr(0, sunrise.size() - 3) + ";" + sunset.substr(0, sunset.size() - 3) + ";" + sunatsouth.substr(0, sunatsouth.size() - 3) + ";" + civtwstart.substr(0, civtwstart.size() - 3) + ";" + civtwend.substr(0, civtwend.size() - 3) + ";" + nauttwstart.substr(0, nauttwstart.size() - 3) + ";" + nauttwend.substr(0, nauttwend.size() - 3) + ";" + asttwstart.substr(0, asttwstart.size() - 3) + ";" + asttwend.substr(0, asttwend.size() - 3) + ";" + daylength.substr(0, daylength.size() - 3); //make a short version
	if (m_LastSunriseSet != riseset)
	{
		m_DayLength = daylength;
		m_LastSunriseSet = riseset;

		// Now store all the time stamps e.g. "08:42;09:12" etc, found in m_LastSunriseSet into
		// a new vector after that we've first converted them to minutes after midnight.
		std::vector<std::string> strarray;
		std::vector<std::string> hourMinItem;
		StringSplit(m_LastSunriseSet, ";", strarray);
		m_SunRiseSetMins.clear();

		for (const auto & it : strarray)
		{
			StringSplit(it, ":", hourMinItem);
			int intMins = (atoi(hourMinItem[0].c_str()) * 60) + atoi(hourMinItem[1].c_str());
			m_SunRiseSetMins.push_back(intMins);
		}

		if (sunrise == sunset)
			if (m_DayLength == "00:00:00")
				_log.Log(LOG_NORM, "Sun below horizon in the space of 24 hours");
			else
				_log.Log(LOG_NORM, "Sun above horizon in the space of 24 hours");
		else
			_log.Log(LOG_NORM, "Sunrise: %s SunSet: %s", sunrise.c_str(), sunset.c_str());
		_log.Log(LOG_NORM, "Day length: %s Sun at south: %s", daylength.c_str(), sunatsouth.c_str());
		if (civtwstart == civtwend)
			_log.Log(LOG_NORM, "There is no civil twilight in the space of 24 hours");
		else
			_log.Log(LOG_NORM, "Civil twilight start: %s Civil twilight end: %s", civtwstart.c_str(), civtwend.c_str());
		if (nauttwstart == nauttwend)
			_log.Log(LOG_NORM, "There is no nautical twilight in the space of 24 hours");
		else
			_log.Log(LOG_NORM, "Nautical twilight start: %s Nautical twilight end: %s", nauttwstart.c_str(), nauttwend.c_str());
		if (asttwstart == asttwend)
			_log.Log(LOG_NORM, "There is no astronomical twilight in the space of 24 hours");
		else
			_log.Log(LOG_NORM, "Astronomical twilight start: %s Astronomical twilight end: %s", asttwstart.c_str(), asttwend.c_str());

		// FixMe: only reload schedules relative to sunset/sunrise to prevent race conditions
		// m_scheduler.ReloadSchedules(); // force reload of all schedules to adjust for changed sunrise/sunset values
	}
	return true;
}

bool MainWorker::RestartHardware(const std::string &idx, const std::string& Name)
{
	return m_pluginsystem.RestartPlugin(atoi(idx.c_str()));
}

bool MainWorker::Start()
{
	utsname my_uname;
	if (uname(&my_uname) == 0)
	{
		m_szSystemName = my_uname.sysname;
		std::transform(m_szSystemName.begin(), m_szSystemName.end(), m_szSystemName.begin(), ::tolower);
	}

	if (!m_sql.OpenDatabase())
	{
		return false;
	}

	// Add database hook to communicate updates
	m_UpdateManager.Start(m_sql.GetDatabase());

	GetSunSettings();
	GetAvailableWebThemes();

	// Start interfaces
	m_pluginsystem.StartPluginSystem();
	AddAllDomoticzHardware();

	// Start Web Server(s)
	http::server::CWebServerHelper	WebServers;
	if (!WebServers.StartServers(&m_UpdateManager.Publisher))
	{
		_log.Log(LOG_ERROR, "Error starting webserver(s)");
#ifdef WIN32
		MessageBox(0, "Error starting webserver(s), check if ports are not in use!", MB_OK, MB_ICONERROR);
#endif
		return false;
	}
	WebServers.SetWebCompressionMode(http::server::WWW_FORCE_NO_GZIP_SUPPORT);

	// Start do work thread
	m_thread = std::make_shared<std::thread>(&MainWorker::Do_Work, this);
	SetThreadName(m_thread->native_handle(), "MainWorker");
	return (m_thread != nullptr);
}


bool MainWorker::Stop()
{
	if (m_thread)
	{
		_log.Log(LOG_STATUS, "Stopping all webservers...");
		http::server::CWebServerHelper	WebServers;
		WebServers.StopServers();

		_log.Log(LOG_STATUS, "Stopping all hardware...");
		StopDomoticzHardware();
#ifdef ENABLE_PYTHON
		m_pluginsystem.StopPluginSystem();
#endif
		m_UpdateManager.Stop();

		RequestStop();
		m_thread->join();
		m_thread.reset();
	}
	return true;
}

#define HEX( x ) \
	std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (int)( x )

bool MainWorker::IsUpdateAvailable(const bool bIsForced)
{
	if (!g_bUseUpdater)
		return false;

	if (!bIsForced)
	{
		std::string	Value;
		std::string	sTrue = "True";
		m_sql.GetPreferencesVar("UseAutoUpdate", Value, sTrue);
		if (Value != "True")
		{
			return false;
		}
	}

	utsname my_uname;
	if (uname(&my_uname) < 0)
		return false;

	std::string machine = my_uname.machine;
	if (machine == "armv6l")
	{
		//Seems like old arm systems can also use the new arm build
		machine = "armv7l";
	}

#ifdef DEBUG_DOWNLOAD
	m_szSystemName = "linux";
	machine = "armv7l";
#endif

	if ((m_szSystemName != "windows") && (machine != "armv6l") && (machine != "armv7l") && (machine != "x86_64") && (machine != "aarch64"))
	{
		//Only Raspberry Pi (Wheezy)/Ubuntu/windows/osx for now!
		return false;
	}
	time_t atime = mytime(NULL);
	if (!bIsForced)
	{
		if (atime - m_LastUpdateCheck < 12 * 3600)
		{
			return m_bHaveUpdate;
		}
	}
	m_LastUpdateCheck = atime;

	int nValue = 0;
	bool bIsBetaChannel = (nValue != 0);

	std::string szURL;
	if (!bIsBetaChannel)
	{
		szURL = "https://www.domoticz.com/download.php?channel=stable&type=version&system=" + m_szSystemName + "&machine=" + machine;
		m_szDomoticzUpdateURL = "https://www.domoticz.com/download.php?channel=stable&type=release&system=" + m_szSystemName + "&machine=" + machine;
		m_szDomoticzUpdateChecksumURL = "https://www.domoticz.com/download.php?channel=stable&type=checksum&system=" + m_szSystemName + "&machine=" + machine;
	}
	else
	{
		szURL = "https://www.domoticz.com/download.php?channel=beta&type=version&system=" + m_szSystemName + "&machine=" + machine;
		m_szDomoticzUpdateURL = "https://www.domoticz.com/download.php?channel=beta&type=release&system=" + m_szSystemName + "&machine=" + machine;
		m_szDomoticzUpdateChecksumURL = "https://www.domoticz.com/download.php?channel=beta&type=checksum&system=" + m_szSystemName + "&machine=" + machine;
	}

	std::string revfile;

	//if (!HTTPClient::GET(szURL, revfile))
	//	return false;

	stdreplace(revfile, "\r\n", "\n");
	std::vector<std::string> strarray;
	StringSplit(revfile, "\n", strarray);
	if (strarray.size() < 1)
		return false;
	StringSplit(strarray[0], " ", strarray);
	if (strarray.size() != 3)
		return false;

	int version = atoi(szAppVersion.substr(szAppVersion.find(".") + 1).c_str());
	m_iRevision = atoi(strarray[2].c_str());
#ifdef DEBUG_DOWNLOAD
	m_bHaveUpdate = true;
#else
	m_bHaveUpdate = ((version != m_iRevision) && (version < m_iRevision));
#endif
	return m_bHaveUpdate;
}

bool MainWorker::StartDownloadUpdate()
{
#ifndef DEBUG_DOWNLOAD
#ifdef WIN32
	return false; //managed by web gui
#endif
#endif

	if (!IsUpdateAvailable(true))
		return false; //no new version available

	m_bHaveDownloadedDomoticzUpdate = false;
	m_bHaveDownloadedDomoticzUpdateSuccessFull = false;
	m_bDoDownloadDomoticzUpdate = true;
	return true;
}

void MainWorker::HandleAutomaticBackups()
{
	std::string Value;
	std::string	sTrue = "True";
	if (!m_sql.GetPreferencesVar("UseAutoBackup", Value, sTrue))
		return;
	if (Value != "True")
		return;

	_log.Log(LOG_STATUS, "Starting automatic database backup procedure...");

	std::stringstream backup_DirH;
	std::stringstream backup_DirD;
	std::stringstream backup_DirM;

#ifdef WIN32
	std::string sbackup_DirH = szUserDataFolder + "backups\\hourly\\";
	std::string sbackup_DirD = szUserDataFolder + "backups\\daily\\";
	std::string sbackup_DirM = szUserDataFolder + "backups\\monthly\\";
#else
	std::string sbackup_DirH = szUserDataFolder + "backups/hourly/";
	std::string sbackup_DirD = szUserDataFolder + "backups/daily/";
	std::string sbackup_DirM = szUserDataFolder + "backups/monthly/";
#endif


	//create folders if they not exists
	mkdir_deep(sbackup_DirH.c_str(), 0755);
	mkdir_deep(sbackup_DirD.c_str(), 0755);
	mkdir_deep(sbackup_DirM.c_str(), 0755);

	time_t now = mytime(NULL);
	struct tm tm1;
	localtime_r(&now, &tm1);
	int hour = tm1.tm_hour;
	int day = tm1.tm_mday;
	int month = tm1.tm_mon;

	int lastHourBackup = -1;
	int lastDayBackup = -1;
	int lastMonthBackup = -1;

	m_sql.GetLastBackupNo("Hour", lastHourBackup);
	m_sql.GetLastBackupNo("Day", lastDayBackup);
	m_sql.GetLastBackupNo("Month", lastMonthBackup);

	std::string szInstanceName = "domoticz";
	std::string szVar;
	std::string	sDomoticz = "domoticz";
	if (m_sql.GetPreferencesVar("Title", szVar, sDomoticz))
	{
		stdreplace(szVar, " ", "_");
		stdreplace(szVar, "/", "_");
		stdreplace(szVar, "\\", "_");
		if (!szVar.empty()) {
			szInstanceName = szVar;
		}
	}

	DIR *lDir;
	//struct dirent *ent;
	if ((lastHourBackup == -1) || (lastHourBackup != hour)) {

		if ((lDir = opendir(sbackup_DirH.c_str())) != NULL)
		{
			std::stringstream sTmp;
			sTmp << "backup-hour-" << std::setw(2) << std::setfill('0') << hour << "-" << szInstanceName << ".db";

			std::string OutputFileName = sbackup_DirH + sTmp.str();
			if (m_sql.BackupDatabase(OutputFileName)) {
				m_sql.SetLastBackupNo("Hour", hour);
			}
			else {
				_log.Log(LOG_ERROR, "Error writing automatic hourly backup file");
			}
			closedir(lDir);
		}
		else {
			_log.Log(LOG_ERROR, "Error accessing automatic backup directories");
		}
	}
	if ((lastDayBackup == -1) || (lastDayBackup != day)) {

		if ((lDir = opendir(sbackup_DirD.c_str())) != NULL)
		{
			std::stringstream sTmp;
			sTmp << "backup-day-" << std::setw(2) << std::setfill('0') << day << "-" << szInstanceName << ".db";

			std::string OutputFileName = sbackup_DirD + sTmp.str();
			if (m_sql.BackupDatabase(OutputFileName)) {
				m_sql.SetLastBackupNo("Day", day);
			}
			else {
				_log.Log(LOG_ERROR, "Error writing automatic daily backup file");
			}
			closedir(lDir);
		}
		else {
			_log.Log(LOG_ERROR, "Error accessing automatic backup directories");
		}
	}
	if ((lastMonthBackup == -1) || (lastMonthBackup != month)) {
		if ((lDir = opendir(sbackup_DirM.c_str())) != NULL)
		{
			std::stringstream sTmp;
			sTmp << "backup-month-" << std::setw(2) << std::setfill('0') << month + 1 << "-" << szInstanceName << ".db";

			std::string OutputFileName = sbackup_DirM + sTmp.str();
			if (m_sql.BackupDatabase(OutputFileName)) {
				m_sql.SetLastBackupNo("Month", month);
			}
			else {
				_log.Log(LOG_ERROR, "Error writing automatic monthly backup file");
			}
			closedir(lDir);
		}
		else {
			_log.Log(LOG_ERROR, "Error accessing automatic backup directories");
		}
	}
	_log.Log(LOG_STATUS, "Ending automatic database backup procedure...");
}

void MainWorker::RemoveExpiredData()
{
	std::vector<std::vector<std::string> > result;

	// if the remove expired data queue is empty refill it
	if (!m_redList.size()) {
		// Interfaces
		result = m_sql.safe_query("SELECT InterfaceID,Name FROM Interface");
		if (!result.empty())
		{
			for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
			{
				std::vector<std::string> sd = *itt;
				m_redList.push(new redEntry(RED_INTERFACE_LOG, sd[0], sd[1]));
			}
		}

		result = m_sql.safe_query("SELECT Value FROM Preference WHERE Name = 'InterfaceLogRows'");
		if (!result.empty())
		{
			for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
			{
				std::vector<std::string> sd = *itt;
				m_sInterfaceRowsToKeep = sd[0];
			}
		}
		else
		{
			std::string sSQL = "INSERT INTO Preference (Name, Value) VALUES ('InterfaceLogRows', '" + m_sInterfaceRowsToKeep + "')";
			result = m_sql.safe_query(sSQL.c_str());
		}

		// Devices
		result = m_sql.safe_query("SELECT DeviceID,Name FROM Device");
		if (!result.empty())
		{
			for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
			{
				std::vector<std::string> sd = *itt;
				m_redList.push(new redEntry(RED_DEVICE_LOG, sd[0], sd[1]));
			}
		}

		result = m_sql.safe_query("SELECT Value FROM Preference WHERE Name = 'DeviceLogRows'");
		if (!result.empty())
		{
			for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
			{
				std::vector<std::string> sd = *itt;
				m_sDeviceRowsToKeep = sd[0];
			}
		}
		else
		{
			std::string sSQL = "INSERT INTO Preference (Name, Value) VALUES ('DeviceLogRows', '" + m_sDeviceRowsToKeep + "')";
			result = m_sql.safe_query(sSQL.c_str());
		}

		// Values
		result = m_sql.safe_query("SELECT ValueID, Name, RetentionDays FROM Value");
		if (!result.empty())
		{
			for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
			{
				std::vector<std::string> sd = *itt;
				m_redList.push(new redEntry(RED_VALUE_LOG, sd[0], sd[1]));
				m_redList.push(new redEntry(RED_VALUE_HISTORY, sd[0], sd[1], sd[2]));
			}
		}

		result = m_sql.safe_query("SELECT Value FROM Preference WHERE Name = 'ValueLogRows'");
		if (!result.empty())
		{
			for (std::vector<std::vector<std::string> >::const_iterator itt = result.begin(); itt != result.end(); ++itt)
			{
				std::vector<std::string> sd = *itt;
				m_sValueRowsToKeep = sd[0];
			}
		}
		else
		{
			std::string sSQL = "INSERT INTO Preference (Name, Value) VALUES ('ValueLogRows', '" + m_sValueRowsToKeep + "')";
			result = m_sql.safe_query(sSQL.c_str());
		}
	}
	else {
		std::vector<std::string> vValues;
		int		iRowCount = 0;
		std::string	sSQL;
		std::string	sTable;

		redEntry* pEntry = m_redList.front();
		m_redList.pop();

		switch (pEntry->type) {
		case RED_INTERFACE_LOG:
			sSQL = "DELETE FROM InterfaceLog WHERE InterfaceLogID IN (SELECT InterfaceLogID FROM InterfaceLog WHERE InterfaceID = ? ORDER BY InterfaceLogID DESC LIMIT -1 OFFSET ?)";
			vValues.push_back(pEntry->ID);
			vValues.push_back(m_sInterfaceRowsToKeep);
			iRowCount = m_sql.execute_sql(sSQL, &vValues, true);
			sTable =  "Interface Log";
			break;
		case RED_DEVICE_LOG:
			sSQL = "DELETE FROM DeviceLog WHERE DeviceID IN (SELECT DeviceLogID FROM DeviceLog WHERE DeviceID = ? ORDER BY DeviceLogID DESC LIMIT -1 OFFSET ?)";
			vValues.push_back(pEntry->ID);
			vValues.push_back(m_sDeviceRowsToKeep);
			iRowCount = m_sql.execute_sql(sSQL, &vValues, true);
			sTable = "Device Log";
			break;
		case RED_VALUE_LOG:
			sSQL = "DELETE FROM ValueLog WHERE ValueLogID IN (SELECT ValueLogID FROM ValueLog WHERE ValueID = ? ORDER BY ValueLogID DESC LIMIT -1 OFFSET ?)";
			vValues.push_back(pEntry->ID);
			vValues.push_back(m_sValueRowsToKeep);
			iRowCount = m_sql.execute_sql(sSQL, &vValues, true);
			sTable = "Value Log";
			break;
		case RED_VALUE_HISTORY:
			sSQL = "DELETE FROM ValueHistory WHERE ValueID = ? AND Timestamp < datetime('now', '-" + pEntry->Extra + " days')";
			vValues.push_back(pEntry->ID);
			iRowCount = m_sql.execute_sql(sSQL, &vValues, true);
			sTable = "Value History";
			break;
		}

		// Log outcome
		if (iRowCount)
			_log.Log(LOG_NORM, "%s '%s' purged, %d records removed.", sTable.c_str(), pEntry->Name.c_str(), iRowCount);

		delete pEntry;
	}
}

void MainWorker::Do_Work()
{
	int second_counter = 0;
	int heartbeat_counter = 0;
	while (!IsStopRequested(500))
	{
		second_counter++;
		if (second_counter < 2)
			continue;
		second_counter = 0;

		if (m_bStartHardware)
		{
			m_hardwareStartCounter++;
			if (m_hardwareStartCounter >= 2)
			{
				m_bStartHardware = false;
				StartDomoticzHardware();
			}
		}
		if (m_devicestorestart.size() > 0)
		{
			for (const auto & itt : m_devicestorestart)
			{
				int hwid = itt;
				std::stringstream sstr;
				sstr << hwid;
				std::string idx = sstr.str();

				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT Name FROM Interface WHERE (InterfaceID=='%q')",
					idx.c_str());
				if (!result.empty())
				{
					std::vector<std::string> sd = result[0];
					std::string Name = sd[0];
					_log.Log(LOG_ERROR, "Restarting: %s", Name.c_str());
					RestartHardware(idx, Name);
				}
			}
			m_devicestorestart.clear();
		}

		time_t atime = mytime(NULL);
		struct tm ltime;
		localtime_r(&atime, &ltime);

		if (ltime.tm_min != m_ScheduleLastMinute)
		{
			if (difftime(atime, m_ScheduleLastMinuteTime) > 30) //avoid RTC/NTP clock drifts
			{
				m_ScheduleLastMinuteTime = atime;
				m_ScheduleLastMinute = ltime.tm_min;

				tzset(); //this because localtime_r/localtime_s does not update for DST

				//check for 5 minute schedule
				if (ltime.tm_min % 3 == 0)
				{
					RemoveExpiredData();
				}
				//check for 5 minute schedule
				if (ltime.tm_min % 10 == 0)
				{
					m_sql.ScheduleShortlog();
				}
				std::string szPwdResetFile = szStartupFolder + "resetpwd";
				if (file_exist(szPwdResetFile.c_str()))
				{
					m_sql.UpdatePreferencesVar("WebUserName", "");
					m_sql.UpdatePreferencesVar("WebPassword", "");
					std::remove(szPwdResetFile.c_str());
				}
			}
			if (_log.NotificationLogsEnabled())
			{
				if ((ltime.tm_min % 5 == 0) || (m_bForceLogNotificationCheck))
				{
					m_bForceLogNotificationCheck = false;
					HandleLogNotifications();
				}
			}
		}
		if (ltime.tm_hour != m_ScheduleLastHour)
		{
			if (difftime(atime, m_ScheduleLastHourTime) > 30 * 60) //avoid RTC/NTP clock drifts
			{
				m_ScheduleLastHourTime = atime;
				m_ScheduleLastHour = ltime.tm_hour;
				GetSunSettings();

				//check for daily schedule
				if (ltime.tm_hour == 0)
				{
					if (atime - m_ScheduleLastDayTime > 12 * 60 * 60)
					{
						m_ScheduleLastDayTime = atime;
						m_sql.ScheduleDay();
					}
				}
				if ((ltime.tm_hour == 5) || (ltime.tm_hour == 17))
				{
					IsUpdateAvailable(true);//check for update
				}
				HandleAutomaticBackups();
			}
		}
		if (heartbeat_counter++ > 12)
		{
			heartbeat_counter = 0;
			m_LastHeartbeat = mytime(NULL);
			HeartbeatCheck();
		}
	}
	_log.Log(LOG_STATUS, "Mainworker Stopped...");
}

bool MainWorker::WriteToHardware(const int HwdID, const char *pdata, const uint8_t length)
{
	int hindex = FindDomoticzHardware(HwdID);

	if (hindex == -1)
		return false;

	return m_hardwaredevices[hindex]->WriteToHardware(pdata, length);
}

void MainWorker::WriteMessageStart()
{
	_log.LogSequenceStart();
}

void MainWorker::WriteMessageEnd()
{
	_log.LogSequenceEnd(LOG_NORM);
}

void MainWorker::WriteMessage(const char *szMessage)
{
	_log.LogSequenceAdd(szMessage);
}

void MainWorker::WriteMessage(const char *szMessage, bool linefeed)
{
	if (linefeed)
		_log.LogSequenceAdd(szMessage);
	else
		_log.LogSequenceAddNoLF(szMessage);
}

void MainWorker::OnHardwareConnected(CDomoticzHardwareBase *pHardware)
{
	//enable receive
	pHardware->m_bEnableReceive = true;
	return;
}

void MainWorker::ForceLogNotificationCheck()
{
	m_bForceLogNotificationCheck = true;
}

void MainWorker::HandleLogNotifications()
{
	std::list<CLogger::_tLogLineStruct> _loglines = _log.GetNotificationLogs();
	if (_loglines.empty())
		return;
	//Assemble notification message

	std::stringstream sstr;
	std::list<CLogger::_tLogLineStruct>::const_iterator itt;
	std::string sTopic;

	if (_loglines.size() > 1)
	{
		sTopic = "Domoticz: Multiple errors received in the last 5 minutes";
		sstr << "Multiple errors received in the last 5 minutes:<br><br>";
	}
	else
	{
		itt = _loglines.begin();
		sTopic = "Domoticz: " + itt->logmessage;
	}

	for (itt = _loglines.begin(); itt != _loglines.end(); ++itt)
	{
		sstr << itt->logmessage << "<br>";
	}
}

void MainWorker::HeartbeatUpdate(const std::string &component, bool critical /*= true*/)
{
	std::lock_guard<std::mutex> l(m_heartbeatmutex);
	time_t now = time(0);
	auto itt = m_componentheartbeats.find(component);
	if (itt != m_componentheartbeats.end()) {
		itt->second.first = now;
	}
	else {
		m_componentheartbeats[component] = std::make_pair(now, critical);
	}
}

void MainWorker::HeartbeatRemove(const std::string &component)
{
	std::lock_guard<std::mutex> l(m_heartbeatmutex);
	auto itt = m_componentheartbeats.find(component);
	if (itt != m_componentheartbeats.end()) {
		m_componentheartbeats.erase(itt);
	}
}

void MainWorker::HeartbeatCheck()
{
	std::lock_guard<std::mutex> l(m_heartbeatmutex);
	std::lock_guard<std::mutex> l2(m_devicemutex);


	m_devicestorestart.clear();

	time_t now;
	mytime(&now);

	for (const auto & itt : m_componentheartbeats)
	{
		double diff = difftime(now, itt.second.first);
		if (diff > 60)
		{
			_log.Log(LOG_ERROR, "%s thread seems to have ended unexpectedly (last update %f seconds ago)", itt.first.c_str(), diff);
			/* GizMoCuz: This causes long operations to crash (Like Issue #3011)
						if (itt.second.second) // If the stalled component is marked as critical, call abort / raise signal
						{
							if (!IsDebuggerPresent())
							{
			#ifdef WIN32
								abort();
			#else
								raise(SIGUSR1);
			#endif
							}
						}
			*/
		}
	}

	//Check hardware heartbeats
	std::map<int, CDomoticzHardwareBase*>* pPlugins = m_pluginsystem.GetHardware();
	std::map<int, CDomoticzHardwareBase*>::const_iterator itt;
	for (itt = pPlugins->begin(); itt != pPlugins->end(); ++itt)
	{
		CDomoticzHardwareBase* pHardware = itt->second;
		if (!pHardware->m_bSkipReceiveCheck)
		{
			//Skip Dummy Hardware
			bool bDoCheck = true;
			if (bDoCheck)
			{
				//Check Thread Timeout
				double diff = difftime(now, pHardware->m_LastHeartbeat);
				//_log.Log(LOG_STATUS, "%d last checking  %.2lf seconds ago", iterator->first, dif);
				if (diff > 60)
				{
					std::vector<std::vector<std::string> > result;
					result = m_sql.safe_query("SELECT Name FROM Interface WHERE (InterfaceID='%d')", pHardware->m_InterfaceID);
					if (result.size() == 1)
					{
						std::vector<std::string> sd = result[0];
						_log.Log(LOG_ERROR, "%s hardware (%d) thread seems to have ended unexpectedly", sd[0].c_str(), pHardware->m_InterfaceID);
					}
				}
			}

			if (pHardware->m_DataTimeout > 0)
			{
				//Check Receive Timeout
				double diff = difftime(now, pHardware->m_LastHeartbeatReceive);
				if (diff > pHardware->m_DataTimeout)
				{
					std::vector<std::vector<std::string> > result;
					result = m_sql.safe_query("SELECT Name FROM Interface WHERE (InterfaceID='%d')", pHardware->m_InterfaceID);
					if (result.size() == 1)
					{
						std::vector<std::string> sd = result[0];

						std::string sDataTimeout = "";
						int totNum = 0;
						if (pHardware->m_DataTimeout < 60) {
							totNum = pHardware->m_DataTimeout;
							sDataTimeout = "Seconds";
						}
						else if (pHardware->m_DataTimeout < 3600) {
							totNum = pHardware->m_DataTimeout / 60;
							if (totNum == 1) {
								sDataTimeout = "Minute";
							}
							else {
								sDataTimeout = "Minutes";
							}
						}
						else if (pHardware->m_DataTimeout < 86400) {
							totNum = pHardware->m_DataTimeout / 3600;
							if (totNum == 1) {
								sDataTimeout = "Hour";
							}
							else {
								sDataTimeout = "Hours";
							}
						}
						else {
							totNum = pHardware->m_DataTimeout / 60;
							if (totNum == 1) {
								sDataTimeout = "Day";
							}
							else {
								sDataTimeout = "Days";
							}
						}

						_log.Log(LOG_ERROR, "%s hardware (%d) nothing received for more than %d %s!....", sd[0].c_str(), pHardware->m_InterfaceID, totNum, sDataTimeout.c_str());
						m_devicestorestart.push_back(pHardware->m_InterfaceID);
					}
				}
			}

		}
	}
}
