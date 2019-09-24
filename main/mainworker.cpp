#include "stdafx.h"
#include "mainworker.h"
#include "Helper.h"
#include "SunRiseSet.h"
#include "localtime_r.h"
#include "Logger.h"
#include "WebServerHelper.h"
#include "SQLHelper.h"

#include "../httpclient/HTTPClient.h"
#include "../webserver/Base64.h"
#include <boost/algorithm/string/join.hpp>

#include <boost/crc.hpp>
#include <algorithm>
#include <set>

//Hardware Devices
#include "../hardware/hardwaretypes.h"
#include "../hardware/RFXBase.h"
#include "../hardware/DomoticzTCP.h"
#include "../hardware/P1MeterBase.h"
#ifdef WITH_OPENZWAVE
#include "../hardware/OpenZWave.h"
#endif

#include "../hardware/plugins/Plugins.h"

// load notifications configuration
#include "../notifications/NotificationHelper.h"

#ifdef WIN32
#include "../msbuild/WindowsHelper.h"
#include "dirent_windows.h"
#else
#include <sys/utsname.h>
#include <dirent.h>
#endif

#include "mainstructs.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef _DEBUG
//#define PARSE_RFXCOM_DEVICE_LOG
//#define DEBUG_DOWNLOAD
//#define DEBUG_RXQUEUE
#endif

#ifdef PARSE_RFXCOM_DEVICE_LOG
#include <iostream>
#include <fstream>
#endif

#define round(a) ( int ) ( a + .5 )

extern std::string szStartupFolder;
extern std::string szUserDataFolder;
extern std::string szWWWFolder;
extern std::string szAppVersion;
extern std::string szWebRoot;
extern bool g_bUseUpdater;
extern http::server::_eWebCompressionMode g_wwwCompressMode;
extern http::server::CWebServerHelper m_webservers;

namespace tcp {
	namespace server {
		class CTCPClient;
	} //namespace server
} //namespace tcp

MainWorker::MainWorker()
{
	m_SecCountdown = -1;

	m_bStartHardware = false;
	m_hardwareStartCounter = 0;

	// Set default settings for web servers
	m_webserver_settings.listening_address = "::"; // listen to all network interfaces
	m_webserver_settings.listening_port = "8080";
#ifdef WWW_ENABLE_SSL
	m_secure_webserver_settings.listening_address = "::"; // listen to all network interfaces
	m_secure_webserver_settings.listening_port = "443";
	m_secure_webserver_settings.ssl_method = "sslv23";
	m_secure_webserver_settings.certificate_chain_file_path = "./server_cert.pem";
	m_secure_webserver_settings.ca_cert_file_path = m_secure_webserver_settings.certificate_chain_file_path; // not used
	m_secure_webserver_settings.cert_file_path = m_secure_webserver_settings.certificate_chain_file_path;
	m_secure_webserver_settings.private_key_file_path = m_secure_webserver_settings.certificate_chain_file_path;
	m_secure_webserver_settings.private_key_pass_phrase = "";
	m_secure_webserver_settings.ssl_options = "default_workarounds,no_sslv2,no_sslv3,no_tlsv1,no_tlsv1_1,single_dh_use";
	m_secure_webserver_settings.tmp_dh_file_path = m_secure_webserver_settings.certificate_chain_file_path;
	m_secure_webserver_settings.verify_peer = false;
	m_secure_webserver_settings.verify_fail_if_no_peer_cert = false;
	m_secure_webserver_settings.verify_file_path = "";
#endif
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

	m_SecStatus = SECSTATUS_DISARMED;

	m_bForceLogNotificationCheck = false;
}

MainWorker::~MainWorker()
{
	Stop();
}

void MainWorker::AddAllDomoticzHardware()
{
	//Add Hardware devices
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query(
		"SELECT ID, Name, Enabled, Type, Address, Port, SerialPort, Username, Password, Extra, Mode1, Mode2, Mode3, Mode4, Mode5, Mode6, DataTimeout FROM Hardware ORDER BY ID ASC");
	if (!result.empty())
	{
		for (const auto & itt : result)
		{
			std::vector<std::string> sd = itt;

			int ID = atoi(sd[0].c_str());
			std::string Name = sd[1];
			std::string sEnabled = sd[2];
			bool Enabled = (sEnabled == "1") ? true : false;
			_eHardwareTypes Type = (_eHardwareTypes)atoi(sd[3].c_str());
			std::string Address = sd[4];
			uint16_t Port = (uint16_t)atoi(sd[5].c_str());
			std::string SerialPort = sd[6];
			std::string Username = sd[7];
			std::string Password = sd[8];
			std::string Extra = sd[9];
			int mode1 = atoi(sd[10].c_str());
			int mode2 = atoi(sd[11].c_str());
			int mode3 = atoi(sd[12].c_str());
			int mode4 = atoi(sd[13].c_str());
			int mode5 = atoi(sd[14].c_str());
			int mode6 = atoi(sd[15].c_str());
			int DataTimeout = atoi(sd[16].c_str());
			AddHardwareFromParams(ID, Name, Enabled, Type, Address, Port, SerialPort, Username, Password, Extra, mode1, mode2, mode3, mode4, mode5, mode6, DataTimeout, false);
		}
		m_hardwareStartCounter = 0;
		m_bStartHardware = true;
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
		m_pluginsystem.DeregisterPlugin(itt->m_HwdID);
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
	if (m_sql.GetPreferencesVar("WebTheme", sValue))
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
	int devidx = FindDomoticzHardware(pHardware->m_HwdID);
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
#ifdef ENABLE_PYTHON
	m_pluginsystem.DeregisterPlugin(HwdId);
#endif
	RemoveDomoticzHardware(m_hardwaredevices[dpos]);
}

int MainWorker::FindDomoticzHardware(int HwdId)
{
	std::lock_guard<std::mutex> l(m_devicemutex);
	int ii = 0;
	for (const auto & itt : m_hardwaredevices)
	{
		if (itt->m_HwdID == HwdId)
			return ii;
		ii++;
	}
	return -1;
}

int MainWorker::FindDomoticzHardwareByType(const _eHardwareTypes HWType)
{
	std::lock_guard<std::mutex> l(m_devicemutex);
	int ii = 0;
	for (const auto & itt : m_hardwaredevices)
	{
		if (itt->HwdType == HWType)
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
		if (itt->m_HwdID == HwdId)
			return itt;
	}
	return NULL;
}

CDomoticzHardwareBase* MainWorker::GetHardwareByIDType(const std::string &HwdId, const _eHardwareTypes HWType)
{
	if (HwdId == "")
		return NULL;
	int iHardwareID = atoi(HwdId.c_str());
	CDomoticzHardwareBase *pHardware = m_mainworker.GetHardware(iHardwareID);
	if (pHardware == NULL)
		return NULL;
	if (pHardware->HwdType != HWType)
		return NULL;
	return pHardware;
}

CDomoticzHardwareBase* MainWorker::GetHardwareByType(const _eHardwareTypes HWType)
{
	std::lock_guard<std::mutex> l(m_devicemutex);
	for (auto & itt : m_hardwaredevices)
	{
		if (itt->HwdType == HWType)
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
	int nValue;
	std::string sValue;
	std::vector<std::string> strarray;
	if (m_sql.GetPreferencesVar("Location", nValue, sValue))
		StringSplit(sValue, ";", strarray);

	if (strarray.size() != 2)
	{
		// No location entered in the settings, lets just reload our schedules and return
		// Load non sun settings timers
		m_scheduler.ReloadSchedules();
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

	m_scheduler.SetSunRiseSetTimers(sunrise, sunset, sunatsouth, civtwstart, civtwend, nauttwstart, nauttwend, asttwstart, asttwend); // Do not change the order

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

		if (!bFirstTime)
			m_eventsystem.LoadEvents();

		// FixMe: only reload schedules relative to sunset/sunrise to prevent race conditions
		// m_scheduler.ReloadSchedules(); // force reload of all schedules to adjust for changed sunrise/sunset values
	}
	return true;
}

void MainWorker::SetWebserverSettings(const http::server::server_settings & settings)
{
	m_webserver_settings.set(settings);
}

std::string MainWorker::GetWebserverAddress()
{
	return m_webserver_settings.listening_address;
}

std::string MainWorker::GetWebserverPort()
{
	return m_webserver_settings.listening_port;
}

#ifdef WWW_ENABLE_SSL
std::string MainWorker::GetSecureWebserverPort()
{
	return m_secure_webserver_settings.listening_port;
}

void MainWorker::SetSecureWebserverSettings(const http::server::ssl_server_settings & ssl_settings)
{
	m_secure_webserver_settings.set(ssl_settings);
}
#endif

bool MainWorker::RestartHardware(const std::string &idx)
{
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query(
		"SELECT Name, Enabled, Type, Address, Port, SerialPort, Username, Password, Extra, Mode1, Mode2, Mode3, Mode4, Mode5, Mode6, DataTimeout FROM Hardware WHERE (ID=='%q')",
		idx.c_str());
	if (result.empty())
		return false;
	std::vector<std::string> sd = result[0];
	std::string Name = sd[0];
	std::string senabled = (sd[1] == "1") ? "true" : "false";
	_eHardwareTypes htype = (_eHardwareTypes)atoi(sd[2].c_str());
	std::string address = sd[3];
	uint16_t port = (uint16_t)atoi(sd[4].c_str());
	std::string serialport = sd[5];
	std::string username = sd[6];
	std::string password = sd[7];
	std::string extra = sd[8];
	int Mode1 = atoi(sd[9].c_str());
	int Mode2 = atoi(sd[10].c_str());
	int Mode3 = atoi(sd[11].c_str());
	int Mode4 = atoi(sd[12].c_str());
	int Mode5 = atoi(sd[13].c_str());
	int Mode6 = atoi(sd[14].c_str());
	int DataTimeout = atoi(sd[15].c_str());
	return AddHardwareFromParams(atoi(idx.c_str()), Name, (senabled == "true") ? true : false, htype, address, port, serialport, username, password, extra, Mode1, Mode2, Mode3, Mode4, Mode5, Mode6, DataTimeout, true);
}

bool MainWorker::AddHardwareFromParams(
	const int ID,
	const std::string &Name,
	const bool Enabled,
	const _eHardwareTypes Type,
	const std::string &Address, const uint16_t Port, const std::string &SerialPort,
	const std::string &Username, const std::string &Password,
	const std::string &Extra,
	const int Mode1,
	const int Mode2,
	const int Mode3,
	const int Mode4,
	const int Mode5,
	const int Mode6,
	const int DataTimeout,
	const bool bDoStart
)
{
	RemoveDomoticzHardware(ID);

	if (!Enabled)
		return true;

	CDomoticzHardwareBase *pHardware = NULL;

	switch (Type)
	{
	case HTYPE_OpenZWave:
#ifdef WITH_OPENZWAVE
		pHardware = new COpenZWave(ID, SerialPort);
#endif
		break;
	case HTYPE_PythonPlugin:
#ifdef ENABLE_PYTHON
		pHardware = m_pluginsystem.RegisterPlugin(ID, Name, Extra);
#endif
		break;
	}

	if (pHardware)
	{
		pHardware->HwdType = Type;
		pHardware->m_Name = Name;
		pHardware->m_ShortName = Hardware_Short_Desc(Type);
		pHardware->m_DataTimeout = DataTimeout;
		AddDomoticzHardware(pHardware);

		if (bDoStart)
			pHardware->Start();
		return true;
	}
	return false;
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

	HTTPClient::SetUserAgent(GenerateUserAgent());
	m_notifications.Init();
	GetSunSettings();
	GetAvailableWebThemes();
#ifdef ENABLE_PYTHON
	m_pluginsystem.StartPluginSystem();
#endif
	AddAllDomoticzHardware();
#ifdef PARSE_RFXCOM_DEVICE_LOG
	if (m_bStartHardware == false)
		m_bStartHardware = true;
#endif
	// load notifications configuration
	m_notifications.LoadConfig();

	if (m_webserver_settings.is_enabled()
#ifdef WWW_ENABLE_SSL
		|| m_secure_webserver_settings.is_enabled()
#endif
		)
	{
		//Start WebServer
#ifdef WWW_ENABLE_SSL
		if (!m_webservers.StartServers(m_webserver_settings, m_secure_webserver_settings, szWWWFolder, m_bIgnoreUsernamePassword, &m_sharedserver))
#else
		if (!m_webservers.StartServers(m_webserver_settings, szWWWFolder, m_bIgnoreUsernamePassword, &m_sharedserver))
#endif
		{
#ifdef WIN32
			MessageBox(0, "Error starting webserver(s), check if ports are not in use!", MB_OK, MB_ICONERROR);
#endif
			return false;
		}
	}
	int nValue = 0;
	if (m_sql.GetPreferencesVar("AuthenticationMethod", nValue))
	{
		m_webservers.SetAuthenticationMethod((http::server::_eAuthenticationMethod)nValue);
	}
	std::string sValue;
	if (m_sql.GetPreferencesVar("WebTheme", sValue))
	{
		m_webservers.SetWebTheme(sValue);
	}

	m_webservers.SetWebRoot(szWebRoot);
	m_webservers.SetWebCompressionMode(g_wwwCompressMode);

	//Start Scheduler
	m_scheduler.StartScheduler();
	m_cameras.ReloadCameras();

	int rnvalue = 0;
	m_sql.GetPreferencesVar("RemoteSharedPort", rnvalue);
	if (rnvalue != 0)
	{
		char szPort[100];
		sprintf(szPort, "%d", rnvalue);
		m_sharedserver.StartServer("::", szPort);

		LoadSharedUsers();
	}

	m_thread = std::make_shared<std::thread>(&MainWorker::Do_Work, this);
	SetThreadName(m_thread->native_handle(), "MainWorker");
	return (m_thread != nullptr);
}


bool MainWorker::Stop()
{
	if (m_thread)
	{
		m_webservers.StopServers();
		m_sharedserver.StopServer();
		_log.Log(LOG_STATUS, "Stopping all hardware...");
		StopDomoticzHardware();
		m_scheduler.StopScheduler();
		m_eventsystem.StopEventSystem();
#ifdef ENABLE_PYTHON
		m_pluginsystem.StopPluginSystem();
#endif

		//    m_cameras.StopCameraGrabber();

		HTTPClient::Cleanup();

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
		int nValue = 0;
		m_sql.GetPreferencesVar("UseAutoUpdate", nValue);
		if (nValue != 1)
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

	int nValue;
	m_sql.GetPreferencesVar("ReleaseChannel", nValue);
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

	if (!HTTPClient::GET(szURL, revfile))
		return false;

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
	int nValue = 0;
	if (!m_sql.GetPreferencesVar("UseAutoBackup", nValue))
		return;
	if (nValue != 1)
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
	if (m_sql.GetPreferencesVar("Title", szVar))
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

void MainWorker::Do_Work()
{
	int second_counter = 0;
	int heartbeat_counter = 0;
	while (!IsStopRequested(500))
	{
		if (m_bDoDownloadDomoticzUpdate)
		{
			m_bDoDownloadDomoticzUpdate = false;

			_log.Log(LOG_STATUS, "Starting Upgrade progress...");
#ifdef WIN32
			std::string outfile;

			//First download the checksum file
			outfile = szStartupFolder + "update.tgz.sha256sum";
			bool bHaveDownloadedChecksum = HTTPClient::GETBinaryToFile(m_szDomoticzUpdateChecksumURL.c_str(), outfile.c_str());
			if (bHaveDownloadedChecksum)
			{
				//Next download the actual update
				outfile = szStartupFolder + "update.tgz";
				m_bHaveDownloadedDomoticzUpdateSuccessFull = HTTPClient::GETBinaryToFile(m_szDomoticzUpdateURL.c_str(), outfile.c_str());
				if (!m_bHaveDownloadedDomoticzUpdateSuccessFull)
				{
					m_UpdateStatusMessage = "Problem downloading update file!";
				}
			}
			else
				m_UpdateStatusMessage = "Problem downloading checksum file!";
#else
			int nValue;
			m_sql.GetPreferencesVar("ReleaseChannel", nValue);
			bool bIsBetaChannel = (nValue != 0);

			std::string scriptname = szUserDataFolder + "scripts/download_update.sh";
			std::string strparm = szUserDataFolder;
			if (bIsBetaChannel)
				strparm += " /beta";

			std::string lscript = scriptname + " " + strparm;
			_log.Log(LOG_STATUS, "Starting: %s", lscript.c_str());
			int ret = system(lscript.c_str());
			m_bHaveDownloadedDomoticzUpdateSuccessFull = (ret == 0);
#endif
			m_bHaveDownloadedDomoticzUpdate = true;
		}

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
#ifdef ENABLE_PYTHON
				m_pluginsystem.AllPluginsStarted();
#endif
				m_eventsystem.SetEnabled(true);
				m_eventsystem.StartEventSystem();
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
				result = m_sql.safe_query("SELECT Name FROM Hardware WHERE (ID=='%q')",
					idx.c_str());
				if (!result.empty())
				{
					std::vector<std::string> sd = result[0];
					std::string Name = sd[0];
					_log.Log(LOG_ERROR, "Restarting: %s", Name.c_str());
					RestartHardware(idx);
				}
			}
			m_devicestorestart.clear();
		}

		if (m_SecCountdown > 0)
		{
			m_SecCountdown--;
			if (m_SecCountdown == 0)
			{
				SetInternalSecStatus();
			}
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
				if (ltime.tm_min % 10 == 0)
				{
					m_sql.ScheduleShortlog();
				}
				std::string szPwdResetFile = szStartupFolder + "resetpwd";
				if (file_exist(szPwdResetFile.c_str()))
				{
					m_webservers.ClearUserPasswords();
					m_sql.UpdatePreferencesVar("WebUserName", "");
					m_sql.UpdatePreferencesVar("WebPassword", "");
					std::remove(szPwdResetFile.c_str());
				}
				m_notifications.CheckAndHandleLastUpdateNotification();
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

				m_sql.CheckDeviceTimeout();
				m_sql.CheckBatteryLow();

				//check for daily schedule
				if (ltime.tm_hour == 0)
				{
					if (atime - m_ScheduleLastDayTime > 12 * 60 * 60)
					{
						m_ScheduleLastDayTime = atime;
						m_sql.ScheduleDay();
					}
				}
#ifdef WITH_OPENZWAVE
				if (ltime.tm_hour == 4)
				{
					//Heal the OpenZWave network
					std::lock_guard<std::mutex> l(m_devicemutex);
					std::vector<CDomoticzHardwareBase*>::iterator itt;
					for (itt = m_hardwaredevices.begin(); itt != m_hardwaredevices.end(); ++itt)
					{
						CDomoticzHardwareBase *pHardware = (*itt);
						if (pHardware->HwdType == HTYPE_OpenZWave)
						{
							COpenZWave *pZWave = reinterpret_cast<COpenZWave *>(pHardware);
							pZWave->NightlyNodeHeal();
						}
					}
				}
#endif
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
	if (
		(pHardware->HwdType != HTYPE_RFXtrx315) &&
		(pHardware->HwdType != HTYPE_RFXtrx433) &&
		(pHardware->HwdType != HTYPE_RFXtrx868) &&
		(pHardware->HwdType != HTYPE_RFXLAN)
		)
	{
		//enable receive
		pHardware->m_bEnableReceive = true;
		return;
	}
	CRFXBase *pRFXBase = reinterpret_cast<CRFXBase *>(pHardware);
	pRFXBase->SendResetCommand();
}

uint64_t MainWorker::PerformRealActionFromDomoticzClient(const uint8_t *pRXCommand, CDomoticzHardwareBase **pOriginalHardware)
{
	*pOriginalHardware = NULL;
	uint8_t devType = pRXCommand[1];
	uint8_t subType = pRXCommand[2];
	std::string ID = "";
	uint8_t Unit = 0;
	const tRBUF *pResponse = reinterpret_cast<const tRBUF *>(pRXCommand);
	char szTmp[300];
	std::vector<std::vector<std::string> > result;

	switch (devType) {
	case pTypeLighting1:
		sprintf(szTmp, "%d", pResponse->LIGHTING1.housecode);
		ID = szTmp;
		Unit = pResponse->LIGHTING1.unitcode;
		break;
	case pTypeLighting2:
		sprintf(szTmp, "%X%02X%02X%02X", pResponse->LIGHTING2.id1, pResponse->LIGHTING2.id2, pResponse->LIGHTING2.id3, pResponse->LIGHTING2.id4);
		ID = szTmp;
		Unit = pResponse->LIGHTING2.unitcode;
		break;
	case pTypeLighting5:
		if (subType != sTypeEMW100)
			sprintf(szTmp, "%02X%02X%02X", pResponse->LIGHTING5.id1, pResponse->LIGHTING5.id2, pResponse->LIGHTING5.id3);
		else
			sprintf(szTmp, "%02X%02X", pResponse->LIGHTING5.id2, pResponse->LIGHTING5.id3);
		ID = szTmp;
		Unit = pResponse->LIGHTING5.unitcode;
		break;
	case pTypeLighting6:
		sprintf(szTmp, "%02X%02X%02X", pResponse->LIGHTING6.id1, pResponse->LIGHTING6.id2, pResponse->LIGHTING6.groupcode);
		ID = szTmp;
		Unit = pResponse->LIGHTING6.unitcode;
		break;
	case pTypeHomeConfort:
		sprintf(szTmp, "%02X%02X%02X%02X", pResponse->HOMECONFORT.id1, pResponse->HOMECONFORT.id2, pResponse->HOMECONFORT.id3, pResponse->HOMECONFORT.housecode);
		ID = szTmp;
		Unit = pResponse->HOMECONFORT.unitcode;
		break;
	case pTypeRadiator1:
		if (subType == sTypeSmartwaresSwitchRadiator)
		{
			sprintf(szTmp, "%X%02X%02X%02X", pResponse->RADIATOR1.id1, pResponse->RADIATOR1.id2, pResponse->RADIATOR1.id3, pResponse->RADIATOR1.id4);
			ID = szTmp;
			Unit = pResponse->RADIATOR1.unitcode;
		}
		break;
	case pTypeColorSwitch:
	{
		const _tColorSwitch *pLed = reinterpret_cast<const _tColorSwitch*>(pResponse);
		ID = "1";
		Unit = pLed->dunit;
	}
	break;
	case pTypeFS20:
		sprintf(szTmp, "%02X%02X", pResponse->FS20.hc1, pResponse->FS20.hc2);
		ID = szTmp;
		Unit = pResponse->FS20.addr;
		break;
	case pTypeCurtain:
		sprintf(szTmp, "%d", pResponse->CURTAIN1.housecode);
		ID = szTmp;
		Unit = pResponse->CURTAIN1.unitcode;
		break;
	case pTypeBlinds:
		sprintf(szTmp, "%02X%02X%02X", pResponse->BLINDS1.id1, pResponse->BLINDS1.id2, pResponse->BLINDS1.id3);
		ID = szTmp;
		Unit = pResponse->BLINDS1.unitcode;
		break;
	case pTypeRFY:
		sprintf(szTmp, "%02X%02X%02X", pResponse->RFY.id1, pResponse->RFY.id2, pResponse->RFY.id3);
		ID = szTmp;
		Unit = pResponse->RFY.unitcode;
		break;
	case pTypeSecurity1:
		sprintf(szTmp, "%02X%02X%02X", pResponse->SECURITY1.id1, pResponse->SECURITY1.id2, pResponse->SECURITY1.id3);
		ID = szTmp;
		Unit = 0;
		break;
	case pTypeSecurity2:
		sprintf(szTmp, "%02X%02X%02X%02X%02X%02X%02X%02X", pResponse->SECURITY2.id1, pResponse->SECURITY2.id2, pResponse->SECURITY2.id3, pResponse->SECURITY2.id4, pResponse->SECURITY2.id5, pResponse->SECURITY2.id6, pResponse->SECURITY2.id7, pResponse->SECURITY2.id8);
		ID = szTmp;
		Unit = 0;
		break;
	case pTypeChime:
		sprintf(szTmp, "%02X%02X", pResponse->CHIME.id1, pResponse->CHIME.id2);
		ID = szTmp;
		Unit = pResponse->CHIME.sound;
		break;
	case pTypeThermostat:
	{
		const _tThermostat *pMeter = reinterpret_cast<const _tThermostat*>(pResponse);
		sprintf(szTmp, "%X%02X%02X%02X", pMeter->id1, pMeter->id2, pMeter->id3, pMeter->id4);
		ID = szTmp;
		Unit = pMeter->dunit;
	}
	break;
	case pTypeThermostat2:
		ID = "1";
		Unit = pResponse->THERMOSTAT2.unitcode;
		break;
	case pTypeThermostat3:
		sprintf(szTmp, "%02X%02X%02X", pResponse->THERMOSTAT3.unitcode1, pResponse->THERMOSTAT3.unitcode2, pResponse->THERMOSTAT3.unitcode3);
		ID = szTmp;
		Unit = 0;
		break;
	case pTypeThermostat4:
		sprintf(szTmp, "%02X%02X%02X", pResponse->THERMOSTAT4.unitcode1, pResponse->THERMOSTAT4.unitcode2, pResponse->THERMOSTAT4.unitcode3);
		ID = szTmp;
		Unit = 0;
		break;
	case pTypeGeneralSwitch:
	{
		const _tGeneralSwitch *pSwitch = reinterpret_cast<const _tGeneralSwitch*>(pResponse);
		sprintf(szTmp, "%08X", pSwitch->id);
		ID = szTmp;
		Unit = pSwitch->unitcode;
	}
	break;
	default:
		return -1;
	}

	if (ID != "")
	{
		// find our original hardware
		// if it is not a domoticz type, perform the actual command

		result = m_sql.safe_query(
			"SELECT HardwareID,ID,Name,StrParam1,StrParam2,nValue,sValue FROM DeviceStatus WHERE (DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d)",
			ID.c_str(), Unit, devType, subType);
		if (result.size() == 1)
		{
			std::vector<std::string> sd = result[0];

			CDomoticzHardwareBase *pHardware = GetHardware(atoi(sd[0].c_str()));
			if (pHardware != NULL)
			{
				if (pHardware->HwdType != HTYPE_Domoticz)
				{
					*pOriginalHardware = pHardware;
					pHardware->WriteToHardware((const char*)pRXCommand, pRXCommand[0] + 1);
					std::stringstream s_strid;
					s_strid << std::dec << sd[1];
					uint64_t ullID;
					s_strid >> ullID;
					return ullID;
				}
			}
		}
	}
	return -1;
}

bool MainWorker::GetSensorData(const uint64_t idx, int &nValue, std::string &sValue)
{
	std::vector<std::vector<std::string> > result;
	char szTmp[100];
	result = m_sql.safe_query("SELECT [Type],[SubType],[nValue],[sValue],[SwitchType] FROM DeviceStatus WHERE (ID==%" PRIu64 ")", idx);
	if (result.empty())
		return false;
	std::vector<std::string> sd = result[0];
	int devType = atoi(sd[0].c_str());
	int subType = atoi(sd[1].c_str());
	nValue = atoi(sd[2].c_str());
	sValue = sd[3];
	_eMeterType metertype = (_eMeterType)atoi(sd[4].c_str());

	//Special cases
	if ((devType == pTypeP1Power) && (subType == sTypeP1Power))
	{
		std::vector<std::string> results;
		StringSplit(sValue, ";", results);
		if (results.size() < 6)
			return false; //invalid data
						  //Return usage or delivery
		long usagecurrent = atol(results[4].c_str());
		long delivcurrent = atol(results[5].c_str());
		std::stringstream ssvalue;
		if (delivcurrent > 0)
		{
			ssvalue << "-" << delivcurrent;
		}
		else
		{
			ssvalue << usagecurrent;
		}
		nValue = 0;
		sValue = ssvalue.str();
	}
	else if ((devType == pTypeP1Gas) && (subType == sTypeP1Gas))
	{
		float GasDivider = 1000.0f;
		//get lowest value of today
		time_t now = mytime(NULL);
		struct tm tm1;
		localtime_r(&now, &tm1);

		struct tm ltime;
		ltime.tm_isdst = tm1.tm_isdst;
		ltime.tm_hour = 0;
		ltime.tm_min = 0;
		ltime.tm_sec = 0;
		ltime.tm_year = tm1.tm_year;
		ltime.tm_mon = tm1.tm_mon;
		ltime.tm_mday = tm1.tm_mday;

		char szDate[40];
		sprintf(szDate, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

		std::vector<std::vector<std::string> > result2;
		strcpy(szTmp, "0");
		result2 = m_sql.safe_query("SELECT MIN(Value) FROM Meter WHERE (DeviceRowID='%" PRIu64 "' AND Date>='%q')", idx, szDate);
		if (!result2.empty())
		{
			std::vector<std::string> sd2 = result2[0];
			unsigned long long total_min_gas = std::strtoull(sd2[0].c_str(), nullptr, 10);
			unsigned long long gasactual = std::strtoull(sValue.c_str(), nullptr, 10);
			unsigned long long total_real_gas = gasactual - total_min_gas;
			float musage = float(total_real_gas) / GasDivider;
			sprintf(szTmp, "%.03f", musage);
		}
		else
		{
			sprintf(szTmp, "%.03f", 0.0f);
		}
		nValue = 0;
		sValue = szTmp;
	}
	else if (devType == pTypeRFXMeter)
	{
		float EnergyDivider = 1000.0f;
		float GasDivider = 100.0f;
		//float WaterDivider = 100.0f;
		int tValue;
		if (m_sql.GetPreferencesVar("MeterDividerEnergy", tValue))
		{
			EnergyDivider = float(tValue);
		}
		if (m_sql.GetPreferencesVar("MeterDividerGas", tValue))
		{
			GasDivider = float(tValue);
		}
		//if (m_sql.GetPreferencesVar("MeterDividerWater", tValue))
		//{
		//WaterDivider = float(tValue);
		//}

		//get value of today
		time_t now = mytime(NULL);
		struct tm tm1;
		localtime_r(&now, &tm1);

		struct tm ltime;
		ltime.tm_isdst = tm1.tm_isdst;
		ltime.tm_hour = 0;
		ltime.tm_min = 0;
		ltime.tm_sec = 0;
		ltime.tm_year = tm1.tm_year;
		ltime.tm_mon = tm1.tm_mon;
		ltime.tm_mday = tm1.tm_mday;

		char szDate[40];
		sprintf(szDate, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

		std::vector<std::vector<std::string> > result2;
		strcpy(szTmp, "0");
		result2 = m_sql.safe_query("SELECT MIN(Value), MAX(Value) FROM Meter WHERE (DeviceRowID='%" PRIu64 "' AND Date>='%q')", idx, szDate);
		if (!result2.empty())
		{
			std::vector<std::string> sd2 = result2[0];

			unsigned long long total_min = std::strtoull(sd2[0].c_str(), nullptr, 10);
			unsigned long long total_max = std::strtoull(sd2[1].c_str(), nullptr, 10);
			unsigned long long total_real = total_max - total_min;
			sprintf(szTmp, "%llu", total_real);

			float musage = 0;
			switch (metertype)
			{
			case MTYPE_ENERGY:
			case MTYPE_ENERGY_GENERATED:
				musage = float(total_real) / EnergyDivider;
				sprintf(szTmp, "%.03f", musage);
				break;
			case MTYPE_GAS:
				musage = float(total_real) / GasDivider;
				sprintf(szTmp, "%.03f", musage);
				break;
			case MTYPE_WATER:
				sprintf(szTmp, "%llu", total_real);
				break;
			case MTYPE_COUNTER:
				sprintf(szTmp, "%llu", total_real);
				break;
				/*
				default:
				strcpy(szTmp, "?");
				break;
				*/
			}
		}
		nValue = 0;
		sValue = szTmp;
	}
	return true;
}

bool MainWorker::SwitchLightInt(const std::vector<std::string> &sd, std::string switchcmd, int level, _tColor color, const bool IsTesting)
{
	unsigned long ID;
	std::stringstream s_strid;
	s_strid << std::hex << sd[1];
	s_strid >> ID;
	uint8_t ID1 = (uint8_t)((ID & 0xFF000000) >> 24);
	uint8_t ID2 = (uint8_t)((ID & 0x00FF0000) >> 16);
	uint8_t ID3 = (uint8_t)((ID & 0x0000FF00) >> 8);
	uint8_t ID4 = (uint8_t)((ID & 0x000000FF));

	int HardwareID = atoi(sd[0].c_str());

	_log.Debug(DEBUG_NORM, "MAIN SwitchLightInt : switchcmd:%s level:%d HWid:%d  sd:%s %s %s %s %s %s", switchcmd.c_str(), level, HardwareID,
		sd[0].c_str(), sd[1].c_str(), sd[2].c_str(), sd[3].c_str(), sd[4].c_str(), sd[5].c_str());

	int hindex = FindDomoticzHardware(HardwareID);
	if (hindex == -1)
	{
		_log.Log(LOG_ERROR, "Switch command not send!, Hardware device disabled or not found!");
		return false;
	}
	CDomoticzHardwareBase *pHardware = GetHardware(HardwareID);
	if (pHardware == NULL)
		return false;

	if (pHardware->HwdType == HTYPE_DomoticzInternal)
	{
		//Special cases
		if (ID == 0x00148702)
		{
			int iSecStatus = 2;
			if (switchcmd == "Disarm")
				iSecStatus = 0;
			else if (switchcmd == "Arm Home")
				iSecStatus = 1;
			else if (switchcmd == "Arm Away")
				iSecStatus = 2;
			else
				return false;
			UpdateDomoticzSecurityStatus(iSecStatus);
			return true;
		}
	}

	uint8_t Unit = atoi(sd[2].c_str());
	uint8_t dType = atoi(sd[3].c_str());
	uint8_t dSubType = atoi(sd[4].c_str());
	_eSwitchType switchtype = (_eSwitchType)atoi(sd[5].c_str());
	std::map<std::string, std::string> options = m_sql.BuildDeviceOptions(sd[10].c_str());

	//when asking for Toggle, just switch to the opposite value
	if (switchcmd == "Toggle") {
		//Request current state of switch
		std::string lstatus = "";
		int llevel = 0;
		bool bHaveDimmer = false;
		bool bHaveGroupCmd = false;
		int maxDimLevel = 0;

		int nValue = atoi(sd[7].c_str());
		std::string sValue = sd[8];

		GetLightStatus(dType, dSubType, switchtype, nValue, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);
		//Flip the status
		switchcmd = (IsLightSwitchOn(lstatus) == true) ? "Off" : "On";
	}

	// If dimlevel is 0 or no dimlevel, turn switch off
	if (level <= 0 && switchcmd == "Set Level")
		switchcmd = "Off";

	//when level is invalid or command is "On", replace level with "LastLevel"
	if (switchcmd == "On" || level < 0)
	{
		//Get LastLevel
		std::vector<std::vector<std::string> > result;
		result = m_sql.safe_query(
			"SELECT LastLevel FROM DeviceStatus WHERE (HardwareID=%d AND DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d)", HardwareID, sd[1].c_str(), Unit, int(dType), int(dSubType));
		if (result.size() == 1)
		{
			level = atoi(result[0][0].c_str());
		}
		_log.Debug(DEBUG_NORM, "MAIN SwitchLightInt : switchcmd==\"On\" || level < 0, new level:%d", level);
	}
	// TODO: Something smarter if level is not valid?
	level = std::max(level, 0);

	//
	//	For plugins all the specific logic below is irrelevent
	//	so just send the full details to the plugin so that it can take appropriate action
	//
	if (pHardware->HwdType == HTYPE_PythonPlugin)
	{
#ifdef ENABLE_PYTHON
		// Special case when color is passed from timer or scene
		if ((switchcmd == "On") || (switchcmd == "Set Level"))
		{
			if (color.mode != ColorModeNone)
			{
				switchcmd = "Set Color";
			}
		}
		((Plugins::CPlugin*)m_hardwaredevices[hindex])->SendCommand(Unit, switchcmd, level, color);
#endif
		return true;
	}

	switch (dType)
	{
	case pTypeLighting1:
	{
		tRBUF lcmd;
		lcmd.LIGHTING1.packetlength = sizeof(lcmd.LIGHTING1) - 1;
		lcmd.LIGHTING1.packettype = dType;
		lcmd.LIGHTING1.subtype = dSubType;
		lcmd.LIGHTING1.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.LIGHTING1.housecode = atoi(sd[1].c_str());
		lcmd.LIGHTING1.unitcode = Unit;
		lcmd.LIGHTING1.filler = 0;
		lcmd.LIGHTING1.rssi = 12;

		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.LIGHTING1.cmnd, options))
			return false;
		if (switchtype == STYPE_Doorbell)
		{
			int rnvalue = 0;
			m_sql.GetPreferencesVar("DoorbellCommand", rnvalue);
			if (rnvalue == 0)
				lcmd.LIGHTING1.cmnd = light1_sChime;
			else
				lcmd.LIGHTING1.cmnd = light1_sOn;
		}

		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.LIGHTING1)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeLighting2:
	{
		tRBUF lcmd;
		lcmd.LIGHTING2.packetlength = sizeof(lcmd.LIGHTING2) - 1;
		lcmd.LIGHTING2.packettype = dType;
		lcmd.LIGHTING2.subtype = dSubType;
		lcmd.LIGHTING2.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.LIGHTING2.id1 = ID1;
		lcmd.LIGHTING2.id2 = ID2;
		lcmd.LIGHTING2.id3 = ID3;
		lcmd.LIGHTING2.id4 = ID4;
		lcmd.LIGHTING2.unitcode = Unit;
		lcmd.LIGHTING2.filler = 0;
		lcmd.LIGHTING2.rssi = 12;

		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.LIGHTING2.cmnd, options))
			return false;
		if (switchtype == STYPE_Doorbell) {
			int rnvalue = 0;
			m_sql.GetPreferencesVar("DoorbellCommand", rnvalue);
			if (rnvalue == 0)
				lcmd.LIGHTING2.cmnd = light2_sGroupOn;
			else
				lcmd.LIGHTING2.cmnd = light2_sOn;
			level = 15;
		}
		else if (switchtype == STYPE_X10Siren) {
			level = 15;
		}
		else if ((switchtype == STYPE_BlindsPercentage) || (switchtype == STYPE_BlindsPercentageInverted)) {
			if (lcmd.LIGHTING2.cmnd == light2_sSetLevel)
			{
				if (level == 15)
				{
					lcmd.LIGHTING2.cmnd = light2_sOn;
				}
				else if (level == 0)
				{
					lcmd.LIGHTING2.cmnd = light2_sOff;
				}
			}
		}
		else if (switchtype == STYPE_Media)
		{
			if (switchcmd == "Set Volume") {
				level = (level < 0) ? 0 : level;
				level = (level > 100) ? 100 : level;
			}
		}
		else
			level = (level > 15) ? 15 : level;

		lcmd.LIGHTING2.level = (uint8_t)level;
		if (switchtype != STYPE_Motion) //dont send actual motion off command
		{
			if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.LIGHTING2)))
				return false;
		}

		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeLighting3:
		if (level > 9)
			level = 9;
		break;
	case pTypeLighting4:
	{
		tRBUF lcmd;
		lcmd.LIGHTING4.packetlength = sizeof(lcmd.LIGHTING4) - 1;
		lcmd.LIGHTING4.packettype = dType;
		lcmd.LIGHTING4.subtype = dSubType;
		lcmd.LIGHTING4.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.LIGHTING4.cmd1 = ID2;
		lcmd.LIGHTING4.cmd2 = ID3;
		lcmd.LIGHTING4.cmd3 = ID4;
		lcmd.LIGHTING4.filler = 0;
		lcmd.LIGHTING4.rssi = 12;

		//Get Pulse timing
		std::vector<std::vector<std::string> > result;
		result = m_sql.safe_query(
			"SELECT sValue FROM DeviceStatus WHERE (HardwareID=%d AND DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d)", HardwareID, sd[1].c_str(), Unit, int(dType), int(dSubType));
		if (result.size() == 1)
		{
			int pulsetimeing = atoi(result[0][0].c_str());
			lcmd.LIGHTING4.pulseHigh = pulsetimeing / 256;
			lcmd.LIGHTING4.pulseLow = pulsetimeing & 0xFF;
			if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.LIGHTING4)))
				return false;
			if (!IsTesting) {
				//send to internal for now (later we use the ACK)
				//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
			}
			return true;
		}
		return false;
	}
	break;
	case pTypeLighting5:
	{
		tRBUF lcmd;
		lcmd.LIGHTING5.packetlength = sizeof(lcmd.LIGHTING5) - 1;
		lcmd.LIGHTING5.packettype = dType;
		lcmd.LIGHTING5.subtype = dSubType;
		lcmd.LIGHTING5.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.LIGHTING5.id1 = ID2;
		lcmd.LIGHTING5.id2 = ID3;
		lcmd.LIGHTING5.id3 = ID4;
		lcmd.LIGHTING5.unitcode = Unit;
		lcmd.LIGHTING5.filler = 0;
		lcmd.LIGHTING5.rssi = 12;

		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.LIGHTING5.cmnd, options))
			return false;
		if (switchtype == STYPE_Doorbell)
		{
			int rnvalue = 0;
			m_sql.GetPreferencesVar("DoorbellCommand", rnvalue);
			if (rnvalue == 0)
				lcmd.LIGHTING5.cmnd = light5_sGroupOn;
			else
				lcmd.LIGHTING5.cmnd = light5_sOn;
			level = 31;
		}
		else if (switchtype == STYPE_X10Siren)
		{
			level = 31;
		}
		if (level > 31)
			level = 31;
		lcmd.LIGHTING5.level = (uint8_t)level;
		if (dSubType == sTypeLivolo)
		{
			if ((switchcmd == "Set Level") && (level == 0))
			{
				switchcmd = "Off";
				GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.LIGHTING5.cmnd, options);
			}
			if (switchcmd != "Off")
			{
				//Special Case, turn off first
				uint8_t oldCmd = lcmd.LIGHTING5.cmnd;
				lcmd.LIGHTING5.cmnd = light5_sLivoloAllOff;
				if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.LIGHTING5)))
					return false;
				lcmd.LIGHTING5.cmnd = oldCmd;
			}
			if (switchcmd == "Set Level")
			{
				//dim value we have to send multiple times
				for (int iDim = 0; iDim < level; iDim++)
				{
					if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.LIGHTING5)))
						return false;
				}
			}
			else
			{
				if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.LIGHTING5)))
					return false;
			}
		}
		else if ((dSubType == sTypeTRC02) || (dSubType == sTypeTRC02_2))
		{
			int oldlevel = level;
			if (switchcmd != "Off")
			{
				if (color.mode == ColorModeRGB)
				{
					switchcmd = "Set Color";
				}
			}
			if ((switchcmd == "Off") ||
				(switchcmd == "On") ||      //Special Case, turn off first to ensure light is in normal mode
				(switchcmd == "Set Color"))
			{
				uint8_t oldCmd = lcmd.LIGHTING5.cmnd;
				lcmd.LIGHTING5.cmnd = light5_sRGBoff;
				if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.LIGHTING5)))
					return false;
				lcmd.LIGHTING5.cmnd = oldCmd;
				sleep_milliseconds(100);
			}
			if ((switchcmd == "On") || (switchcmd == "Set Color"))
			{
				//turn on
				lcmd.LIGHTING5.cmnd = light5_sRGBon;
				if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.LIGHTING5)))
					return false;
				sleep_milliseconds(100);

				if (switchcmd == "Set Color")
				{
					if (color.mode == ColorModeRGB)
					{
						float hsb[3];
						rgb2hsb(color.r, color.g, color.b, hsb);
						switchcmd = "Set Color";

						float dval = 126.0f*hsb[0]; // Color Range is 0x06..0x84
						lcmd.LIGHTING5.cmnd = light5_sRGBcolormin + 1 + round(dval);
						if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.LIGHTING5)))
							return false;
					}
				}
			}
		}
		else
		{
			if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.LIGHTING5)))
				return false;
		}
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeLighting6:
	{
		tRBUF lcmd;
		lcmd.LIGHTING6.packetlength = sizeof(lcmd.LIGHTING6) - 1;
		lcmd.LIGHTING6.packettype = dType;
		lcmd.LIGHTING6.subtype = dSubType;
		lcmd.LIGHTING6.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.LIGHTING6.seqnbr2 = 0;
		lcmd.LIGHTING6.id1 = ID2;
		lcmd.LIGHTING6.id2 = ID3;
		lcmd.LIGHTING6.groupcode = ID4;
		lcmd.LIGHTING6.unitcode = Unit;
		lcmd.LIGHTING6.cmndseqnbr = m_hardwaredevices[hindex]->m_SeqNr % 4;
		lcmd.LIGHTING6.filler = 0;
		lcmd.LIGHTING6.rssi = 12;

		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.LIGHTING6.cmnd, options))
			return false;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.LIGHTING6)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeFS20:
	{
		tRBUF lcmd;
		lcmd.FS20.packetlength = sizeof(lcmd.FS20) - 1;
		lcmd.FS20.packettype = dType;
		lcmd.FS20.subtype = dSubType;
		lcmd.FS20.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.FS20.hc1 = ID3;
		lcmd.FS20.hc2 = ID4;
		lcmd.FS20.addr = Unit;
		lcmd.FS20.filler = 0;
		lcmd.FS20.rssi = 12;
		lcmd.FS20.cmd2 = 0;
		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.FS20.cmd1, options))
			return false;
		level = (level > 15) ? 15 : level;

		if (level > 0)
		{
			lcmd.FS20.cmd1 = fs20_sDimlevel_1 + level;
		}

		if (switchtype != STYPE_Motion) //dont send actual motion off command
		{
			if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.FS20)))
				return false;
		}

		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeHomeConfort:
	{
		tRBUF lcmd;
		lcmd.HOMECONFORT.packetlength = sizeof(lcmd.HOMECONFORT) - 1;
		lcmd.HOMECONFORT.packettype = dType;
		lcmd.HOMECONFORT.subtype = dSubType;
		lcmd.HOMECONFORT.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.HOMECONFORT.id1 = ID1;
		lcmd.HOMECONFORT.id2 = ID2;
		lcmd.HOMECONFORT.id3 = ID3;
		lcmd.HOMECONFORT.housecode = ID4;
		lcmd.HOMECONFORT.unitcode = Unit;
		lcmd.HOMECONFORT.filler = 0;
		lcmd.HOMECONFORT.rssi = 12;

		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.HOMECONFORT.cmnd, options))
			return false;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.HOMECONFORT)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeFan:
	{
		tRBUF lcmd;
		lcmd.FAN.packetlength = sizeof(lcmd.FAN) - 1;
		lcmd.FAN.packettype = dType;
		lcmd.FAN.subtype = dSubType;
		lcmd.FAN.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.FAN.id1 = ID2;
		lcmd.FAN.id2 = ID3;
		lcmd.FAN.id3 = ID4;
		lcmd.FAN.filler = 0;
		lcmd.FAN.rssi = 12;

		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.FAN.cmnd, options))
			return false;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.FAN)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeColorSwitch:
	{
		_tColorSwitch lcmd;
		lcmd.subtype = dSubType;
		lcmd.id = ID;
		lcmd.dunit = Unit;
		lcmd.color = color;
		level = std::min(100, level);
		level = std::max(0, level);
		lcmd.value = level;

		//Special case when color is passed from timer or scene
		if ((switchcmd == "On") || (switchcmd == "Set Level"))
		{
			if (color.mode != ColorModeNone)
			{
				switchcmd = "Set Color";
			}
		}
		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.command, options))
			return false;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(_tColorSwitch)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeSecurity1:
	{
		tRBUF lcmd;
		lcmd.SECURITY1.packetlength = sizeof(lcmd.SECURITY1) - 1;
		lcmd.SECURITY1.packettype = dType;
		lcmd.SECURITY1.subtype = dSubType;
		lcmd.SECURITY1.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.SECURITY1.battery_level = 9;
		lcmd.SECURITY1.id1 = ID2;
		lcmd.SECURITY1.id2 = ID3;
		lcmd.SECURITY1.id3 = ID4;
		lcmd.SECURITY1.rssi = 12;
		switch (dSubType)
		{
		case sTypeKD101:
		case sTypeSA30:
		{
			if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.SECURITY1.status, options))
				return false;
			//send it twice
			if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.SECURITY1)))
				return false;
			sleep_milliseconds(500);
			if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.SECURITY1)))
				return false;
			if (!IsTesting) {
				//send to internal for now (later we use the ACK)
				//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
			}
		}
		break;
		case sTypeSecX10M:
		case sTypeSecX10R:
		case sTypeSecX10:
		case sTypeMeiantech:
		{
			if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.SECURITY1.status, options))
				return false;
			if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.SECURITY1)))
				return false;
			if (!IsTesting) {
				//send to internal for now (later we use the ACK)
				//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
			}
		}
		break;
		}
		return true;
	}
	break;
	case pTypeSecurity2:
	{
		BYTE kCodes[9];
		if (sd[1].size() < 8 * 2)
		{
			return false;
		}
		for (int ii = 0; ii < 8; ii++)
		{
			std::string sHex = sd[1].substr(ii * 2, 2);
			std::stringstream s_strid;
			int iHex = 0;
			s_strid << std::hex << sHex;
			s_strid >> iHex;
			kCodes[ii] = (BYTE)iHex;

		}
		tRBUF lcmd;
		lcmd.SECURITY2.packetlength = sizeof(lcmd.SECURITY2) - 1;
		lcmd.SECURITY2.packettype = dType;
		lcmd.SECURITY2.subtype = dSubType;
		lcmd.SECURITY2.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.SECURITY2.id1 = kCodes[0];
		lcmd.SECURITY2.id2 = kCodes[1];
		lcmd.SECURITY2.id3 = kCodes[2];
		lcmd.SECURITY2.id4 = kCodes[3];
		lcmd.SECURITY2.id5 = kCodes[4];
		lcmd.SECURITY2.id6 = kCodes[5];
		lcmd.SECURITY2.id7 = kCodes[6];
		lcmd.SECURITY2.id8 = kCodes[7];

		lcmd.SECURITY2.id9 = 0;//bat full
		lcmd.SECURITY2.battery_level = 9;
		lcmd.SECURITY2.rssi = 12;

		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.SECURITY2)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeHunter:
	{
		BYTE kCodes[6];
		if (sd[1].size() < 6 * 2)
		{
			return false;
		}
		for (int ii = 0; ii < 6; ii++)
		{
			std::string sHex = sd[1].substr(ii * 2, 2);
			std::stringstream s_strid;
			int iHex = 0;
			s_strid << std::hex << sHex;
			s_strid >> iHex;
			kCodes[ii] = (BYTE)iHex;
		}
		tRBUF lcmd;
		lcmd.HUNTER.packetlength = sizeof(lcmd.HUNTER) - 1;
		lcmd.HUNTER.packettype = dType;
		lcmd.HUNTER.subtype = dSubType;
		lcmd.HUNTER.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.HUNTER.id1 = kCodes[0];
		lcmd.HUNTER.id2 = kCodes[1];
		lcmd.HUNTER.id3 = kCodes[2];
		lcmd.HUNTER.id4 = kCodes[3];
		lcmd.HUNTER.id5 = kCodes[4];
		lcmd.HUNTER.id6 = kCodes[5];
		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.HUNTER.cmnd, options))
			return false;
		lcmd.HUNTER.rssi = 12;

		if (!WriteToHardware(HardwareID, (const char*)& lcmd, sizeof(lcmd.SECURITY2)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t*)& lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeCurtain:
	{
		tRBUF lcmd;
		lcmd.CURTAIN1.packetlength = sizeof(lcmd.CURTAIN1) - 1;
		lcmd.CURTAIN1.packettype = dType;
		lcmd.CURTAIN1.subtype = dSubType;
		lcmd.CURTAIN1.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.CURTAIN1.housecode = atoi(sd[1].c_str());
		lcmd.CURTAIN1.unitcode = Unit;
		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.CURTAIN1.cmnd, options))
			return false;
		lcmd.CURTAIN1.filler = 0;

		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.CURTAIN1)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeBlinds:
	{
		tRBUF lcmd;
		lcmd.BLINDS1.packetlength = sizeof(lcmd.BLINDS1) - 1;
		lcmd.BLINDS1.packettype = dType;
		lcmd.BLINDS1.subtype = dSubType;
		lcmd.BLINDS1.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.BLINDS1.id1 = ID1;
		lcmd.BLINDS1.id2 = ID2;
		lcmd.BLINDS1.id3 = ID3;
		lcmd.BLINDS1.id4 = 0;
		if (
			(dSubType == sTypeBlindsT0) ||
			(dSubType == sTypeBlindsT1) ||
			(dSubType == sTypeBlindsT3) ||
			(dSubType == sTypeBlindsT8) ||
			(dSubType == sTypeBlindsT12) ||
			(dSubType == sTypeBlindsT13) ||
			(dSubType == sTypeBlindsT14) ||
			(dSubType == sTypeBlindsT15) ||
			(dSubType == sTypeBlindsT16)
			)
		{
			lcmd.BLINDS1.unitcode = Unit;
		}
		else if ((dSubType == sTypeBlindsT6) || (dSubType == sTypeBlindsT7) || (dSubType == sTypeBlindsT9))
		{
			lcmd.BLINDS1.unitcode = Unit;
			lcmd.BLINDS1.id4 = ID4;
		}
		else
		{
			lcmd.BLINDS1.unitcode = 0;
		}
		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.BLINDS1.cmnd, options))
			return false;
		level = 15;
		lcmd.BLINDS1.filler = 0;
		lcmd.BLINDS1.rssi = 12;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.BLINDS1)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeRFY:
	{
		tRBUF lcmd;
		lcmd.BLINDS1.packetlength = sizeof(lcmd.RFY) - 1;
		lcmd.BLINDS1.packettype = dType;
		lcmd.BLINDS1.subtype = dSubType;
		lcmd.RFY.id1 = ID2;
		lcmd.RFY.id2 = ID3;
		lcmd.RFY.id3 = ID4;
		lcmd.RFY.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.RFY.unitcode = Unit;

		if (IsTesting)
		{
			lcmd.RFY.cmnd = rfy_sProgram;
		}
		else
		{
			if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.RFY.cmnd, options))
				return false;
		}

		if (lcmd.BLINDS1.subtype == sTypeRFY2)
		{
			//Special case for protocol version 2
			lcmd.BLINDS1.subtype = sTypeRFY;
			if (lcmd.RFY.cmnd == rfy_sUp)
				lcmd.RFY.cmnd = rfy_s2SecUp;
			else if (lcmd.RFY.cmnd == rfy_sDown)
				lcmd.RFY.cmnd = rfy_s2SecDown;
		}

		level = 15;
		lcmd.RFY.filler = 0;
		lcmd.RFY.rssi = 12;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.RFY)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeChime:
	{
		tRBUF lcmd;
		lcmd.CHIME.packetlength = sizeof(lcmd.CHIME) - 1;
		lcmd.CHIME.packettype = dType;
		lcmd.CHIME.subtype = dSubType;
		lcmd.CHIME.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.CHIME.id1 = ID3;
		lcmd.CHIME.id2 = ID4;
		level = 15;
		lcmd.CHIME.sound = Unit;
		lcmd.CHIME.filler = 0;
		lcmd.CHIME.rssi = 12;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.CHIME)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeThermostat2:
	{
		tRBUF lcmd;
		lcmd.THERMOSTAT2.packetlength = sizeof(lcmd.REMOTE) - 1;
		lcmd.THERMOSTAT2.packettype = dType;
		lcmd.THERMOSTAT2.subtype = dSubType;
		lcmd.THERMOSTAT2.unitcode = Unit;
		lcmd.THERMOSTAT2.cmnd = Unit;
		lcmd.THERMOSTAT2.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;

		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.THERMOSTAT2.cmnd, options))
			return false;

		lcmd.THERMOSTAT2.filler = 0;
		lcmd.THERMOSTAT2.rssi = 12;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.THERMOSTAT2)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeThermostat3:
	{
		tRBUF lcmd;
		lcmd.THERMOSTAT3.packetlength = sizeof(lcmd.THERMOSTAT3) - 1;
		lcmd.THERMOSTAT3.packettype = dType;
		lcmd.THERMOSTAT3.subtype = dSubType;
		lcmd.THERMOSTAT3.unitcode1 = ID2;
		lcmd.THERMOSTAT3.unitcode2 = ID3;
		lcmd.THERMOSTAT3.unitcode3 = ID4;
		lcmd.THERMOSTAT3.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.THERMOSTAT3.cmnd, options))
			return false;
		level = 15;
		lcmd.THERMOSTAT3.filler = 0;
		lcmd.THERMOSTAT3.rssi = 12;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.THERMOSTAT3)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeThermostat4:
	{
		_log.Log(LOG_ERROR, "Thermostat 4 not implemented yet!");
		/*
		tRBUF lcmd;
		lcmd.THERMOSTAT4.packetlength = sizeof(lcmd.THERMOSTAT3) - 1;
		lcmd.THERMOSTAT4.packettype = dType;
		lcmd.THERMOSTAT4.subtype = dSubType;
		lcmd.THERMOSTAT4.unitcode1 = ID2;
		lcmd.THERMOSTAT4.unitcode2 = ID3;
		lcmd.THERMOSTAT4.unitcode3 = ID4;
		lcmd.THERMOSTAT4.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.THERMOSTAT4.mode, options))
		return false;
		level = 15;
		lcmd.THERMOSTAT4.filler = 0;
		lcmd.THERMOSTAT4.rssi = 12;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.THERMOSTAT4)))
		return false;
		if (!IsTesting) {
		//send to internal for now (later we use the ACK)
		PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		*/
		return true;
	}
	break;
	case pTypeRemote:
	{
		tRBUF lcmd;
		lcmd.REMOTE.packetlength = sizeof(lcmd.REMOTE) - 1;
		lcmd.REMOTE.packettype = dType;
		lcmd.REMOTE.subtype = dSubType;
		lcmd.REMOTE.id = ID4;
		lcmd.REMOTE.cmnd = Unit;
		lcmd.REMOTE.cmndtype = 0;
		lcmd.REMOTE.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.REMOTE.toggle = 0;
		lcmd.REMOTE.rssi = 12;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.REMOTE)))
			return false;
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	}
	break;
	case pTypeRadiator1:
		tRBUF lcmd;
		lcmd.RADIATOR1.packetlength = sizeof(lcmd.RADIATOR1) - 1;
		lcmd.RADIATOR1.packettype = pTypeRadiator1;
		lcmd.RADIATOR1.subtype = sTypeSmartwares;
		lcmd.RADIATOR1.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		lcmd.RADIATOR1.id1 = ID1;
		lcmd.RADIATOR1.id2 = ID2;
		lcmd.RADIATOR1.id3 = ID3;
		lcmd.RADIATOR1.id4 = ID4;
		lcmd.RADIATOR1.unitcode = Unit;
		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, lcmd.RADIATOR1.cmnd, options))
			return false;
		if (level > 15)
			level = 15;
		lcmd.RADIATOR1.temperature = 0;
		lcmd.RADIATOR1.tempPoint5 = 0;
		lcmd.RADIATOR1.filler = 0;
		lcmd.RADIATOR1.rssi = 12;
		if (!WriteToHardware(HardwareID, (const char*)&lcmd, sizeof(lcmd.RADIATOR1)))
			return false;

		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			lcmd.RADIATOR1.subtype = sTypeSmartwaresSwitchRadiator;
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&lcmd, NULL, -1);
		}
		return true;
	case pTypeGeneralSwitch:
	{

		_tGeneralSwitch gswitch;
		gswitch.type = dType;
		gswitch.subtype = dSubType;
		gswitch.seqnbr = m_hardwaredevices[hindex]->m_SeqNr++;
		gswitch.id = ID;
		gswitch.unitcode = Unit;

		if (!GetLightCommand(dType, dSubType, switchtype, switchcmd, gswitch.cmnd, options))
			return false;

		if ((switchtype != STYPE_Selector) && (dSubType != sSwitchGeneralSwitch))
		{
			level = (level > 99) ? 99 : level;
		}

		if (switchtype == STYPE_Doorbell) {
			int rnvalue = 0;
			m_sql.GetPreferencesVar("DoorbellCommand", rnvalue);
			if (rnvalue == 0)
				lcmd.LIGHTING2.cmnd = gswitch_sGroupOn;
			else
				lcmd.LIGHTING2.cmnd = gswitch_sOn;
			level = 15;
		}
		else if (switchtype == STYPE_Selector)
		{
			if ((switchcmd == "Set Level") || (switchcmd == "Set Group Level")) {
				std::map<std::string, std::string> statuses;
				GetSelectorSwitchStatuses(options, statuses);
				int maxLevel = statuses.size() * 10;

				level = (level < 0) ? 0 : level;
				level = (level > maxLevel) ? maxLevel : level;

				std::stringstream sslevel;
				sslevel << level;
				if (statuses[sslevel.str()].empty()) {
					_log.Log(LOG_ERROR, "Setting a wrong level value %d to Selector device %lu", level, ID);
				}
			}
		}
		else if (((switchtype == STYPE_BlindsPercentage) ||
			(switchtype == STYPE_BlindsPercentageInverted)) &&
			(gswitch.cmnd == gswitch_sSetLevel) && (level == 100))
			gswitch.cmnd = gswitch_sOn;

		gswitch.level = (uint8_t)level;
		gswitch.rssi = 12;
		if (switchtype != STYPE_Motion) //dont send actual motion off command
		{
			if (!WriteToHardware(HardwareID, (const char*)&gswitch, sizeof(_tGeneralSwitch)))
				return false;
		}
		if (!IsTesting) {
			//send to internal for now (later we use the ACK)
			//PushAndWaitRxMessage(m_hardwaredevices[hindex], (const uint8_t *)&gswitch, NULL, -1);
		}
	}
	return true;
	}
	return false;
}

bool MainWorker::SwitchLight(const std::string &idx, const std::string &switchcmd, const std::string &level, const std::string &color, const std::string &ooc, const int ExtraDelay)
{
	uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
	int ilevel = -1;
	if (level != "")
		ilevel = atoi(level.c_str());

	return SwitchLight(ID, switchcmd, ilevel, _tColor(color), atoi(ooc.c_str()) != 0, ExtraDelay);
}

bool MainWorker::SwitchLight(const uint64_t idx, const std::string &switchcmd, const int level, _tColor color, const bool ooc, const int ExtraDelay)
{
	//Get Device details
	_log.Debug(DEBUG_NORM, "MAIN SwitchLight idx:%" PRId64 " cmd:%s lvl:%d ", idx, switchcmd.c_str(), level);
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query(
		"SELECT HardwareID,DeviceID,Unit,Type,SubType,SwitchType,AddjValue2,nValue,sValue,Name,Options FROM DeviceStatus WHERE (ID == %" PRIu64 ")",
		idx);
	if (result.empty())
		return false;

	std::vector<std::string> sd = result[0];

	//uint8_t dType = atoi(sd[3].c_str());
	//uint8_t dSubType = atoi(sd[4].c_str());
	_eSwitchType switchtype = (_eSwitchType)atoi(sd[5].c_str());
	int iOnDelay = atoi(sd[6].c_str());
	int nValue = atoi(sd[7].c_str());
	std::string sValue = sd[8].c_str();
	std::string devName = sd[9].c_str();
	//std::string sOptions = sd[10].c_str();

	bool bIsOn = IsLightSwitchOn(switchcmd);
	if (ooc)//Only on change
	{
		int nNewVal = bIsOn ? 1 : 0;//Is that everything we need here
		if ((switchtype == STYPE_Selector) && (nValue == nNewVal) && (level == atoi(sValue.c_str()))) {
			return true;
		}
		else if (nValue == nNewVal) {
			return true;//FIXME no return code for already set
		}
	}
	//Check if we have an On-Delay, if yes, add it to the tasker
	if (((bIsOn) && (iOnDelay != 0)) || ExtraDelay)
	{
		if (iOnDelay + ExtraDelay != 0)
		{
			_log.Log(LOG_NORM, "Delaying switch [%s] action (%s) for %d seconds", devName.c_str(), switchcmd.c_str(), iOnDelay + ExtraDelay);
		}
		m_sql.AddTaskItem(_tTaskItem::SwitchLightEvent(static_cast<float>(iOnDelay + ExtraDelay), idx, switchcmd, level, color, "Switch with Delay"));
		return true;
	}
	else
		return SwitchLightInt(sd, switchcmd, level, color, false);
}

bool MainWorker::SetClockInt(const std::vector<std::string> &sd, const std::string &clockstr)
{
#ifdef WITH_OPENZWAVE
	int HardwareID = atoi(sd[0].c_str());
	int hindex = FindDomoticzHardware(HardwareID);
	if (hindex == -1)
		return false;

	unsigned long ID;
	std::stringstream s_strid;
	s_strid << std::hex << sd[1];
	s_strid >> ID;
	CDomoticzHardwareBase *pHardware = GetHardware(HardwareID);
	if (pHardware == NULL)
		return false;
	if (pHardware->HwdType == HTYPE_OpenZWave)
	{
		std::vector<std::string> splitresults;
		StringSplit(clockstr, ";", splitresults);
		if (splitresults.size() != 3)
			return false;
		int day = atoi(splitresults[0].c_str());
		int hour = atoi(splitresults[1].c_str());
		int minute = atoi(splitresults[2].c_str());

		_tGeneralDevice tmeter;
		tmeter.subtype = sTypeZWaveClock;
		tmeter.intval1 = ID;
		tmeter.intval2 = (day*(24 * 60)) + (hour * 60) + minute;
		if (!WriteToHardware(HardwareID, (const char*)&tmeter, sizeof(_tGeneralDevice)))
			return false;
	}
#endif
	return true;
}

bool MainWorker::SetClock(const std::string &idx, const std::string &clockstr)
{
	//Get Device details
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query(
		"SELECT HardwareID, DeviceID,Unit,Type,SubType,SwitchType FROM DeviceStatus WHERE (ID == '%q')",
		idx.c_str());
	if (result.empty())
		return false;

	std::vector<std::string> sd = result[0];
	return SetClockInt(sd, clockstr);
}

bool MainWorker::SetZWaveThermostatModeInt(const std::vector<std::string> &sd, const int tMode)
{
#ifdef WITH_OPENZWAVE
	int HardwareID = atoi(sd[0].c_str());
	int hindex = FindDomoticzHardware(HardwareID);
	if (hindex == -1)
		return false;

	unsigned long ID;
	std::stringstream s_strid;
	s_strid << std::hex << sd[1];
	s_strid >> ID;
	CDomoticzHardwareBase *pHardware = GetHardware(HardwareID);
	if (pHardware == NULL)
		return false;
	if (pHardware->HwdType == HTYPE_OpenZWave)
	{
		_tGeneralDevice tmeter;
		tmeter.subtype = sTypeZWaveThermostatMode;
		tmeter.intval1 = ID;
		tmeter.intval2 = tMode;
		if (!WriteToHardware(HardwareID, (const char*)&tmeter, sizeof(_tGeneralDevice)))
			return false;
	}
#endif
	return true;
}

bool MainWorker::SetZWaveThermostatFanModeInt(const std::vector<std::string> &sd, const int fMode)
{
#ifdef WITH_OPENZWAVE
	int HardwareID = atoi(sd[0].c_str());
	int hindex = FindDomoticzHardware(HardwareID);
	if (hindex == -1)
		return false;

	unsigned long ID;
	std::stringstream s_strid;
	s_strid << std::hex << sd[1];
	s_strid >> ID;
	CDomoticzHardwareBase *pHardware = GetHardware(HardwareID);
	if (pHardware == NULL)
		return false;
	if (pHardware->HwdType == HTYPE_OpenZWave)
	{
		_tGeneralDevice tmeter;
		tmeter.subtype = sTypeZWaveThermostatFanMode;
		tmeter.intval1 = ID;
		tmeter.intval2 = fMode;
		if (!WriteToHardware(HardwareID, (const char*)&tmeter, sizeof(_tGeneralDevice)))
			return false;
	}
#endif
	return true;
}

bool MainWorker::SetZWaveThermostatMode(const std::string &idx, const int tMode)
{
	//Get Device details
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query(
		"SELECT HardwareID, DeviceID,Unit,Type,SubType,SwitchType FROM DeviceStatus WHERE (ID == '%q')",
		idx.c_str());
	if (result.empty())
		return false;

	std::vector<std::string> sd = result[0];
	return SetZWaveThermostatModeInt(sd, tMode);
}

bool MainWorker::SetZWaveThermostatFanMode(const std::string &idx, const int fMode)
{
	//Get Device details
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query(
		"SELECT HardwareID, DeviceID,Unit,Type,SubType,SwitchType FROM DeviceStatus WHERE (ID == '%q')",
		idx.c_str());
	if (result.empty())
		return false;

	std::vector<std::string> sd = result[0];
	return SetZWaveThermostatFanModeInt(sd, fMode);
}

bool MainWorker::SwitchScene(const std::string &idx, const std::string &switchcmd)
{
	uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);

	return SwitchScene(ID, switchcmd);
}

//returns if a device activates a scene
bool MainWorker::DoesDeviceActiveAScene(const uint64_t DevRowIdx, const int Cmnd)
{
	//check for scene code
	std::vector<std::vector<std::string> > result;

	result = m_sql.safe_query("SELECT Activators, SceneType FROM Scenes WHERE (Activators!='')");
	if (!result.empty())
	{
		for (const auto & itt : result)
		{
			std::vector<std::string> sd = itt;

			int SceneType = atoi(sd[1].c_str());

			std::vector<std::string> arrayActivators;
			StringSplit(sd[0], ";", arrayActivators);
			for (const auto & ittAct : arrayActivators)
			{
				std::string sCodeCmd = ittAct;

				std::vector<std::string> arrayCode;
				StringSplit(sCodeCmd, ":", arrayCode);

				std::string sID = arrayCode[0];
				std::string sCode = "";
				if (arrayCode.size() == 2)
				{
					sCode = arrayCode[1];
				}

				uint64_t aID = std::strtoull(sID.c_str(), nullptr, 10);
				if (aID == DevRowIdx)
				{
					if ((SceneType == SGTYPE_GROUP) || (sCode.empty()))
						return true;
					int iCode = atoi(sCode.c_str());
					if (iCode == Cmnd)
						return true;
				}
			}
		}
	}
	return false;
}

bool MainWorker::SwitchScene(const uint64_t idx, std::string switchcmd)
{
	std::vector<std::vector<std::string> > result;
	int nValue = (switchcmd == "On") ? 1 : 0;

	std::string Name = "Unknown?";
	_eSceneGroupType scenetype = SGTYPE_SCENE;
	std::string onaction = "";
	std::string offaction = "";
	std::string status = "";

	//Get Scene details
	result = m_sql.safe_query("SELECT Name, SceneType, OnAction, OffAction, nValue FROM Scenes WHERE (ID == %" PRIu64 ")", idx);
	if (!result.empty())
	{
		std::vector<std::string> sds = result[0];
		Name = sds[0];
		scenetype = (_eSceneGroupType)atoi(sds[1].c_str());
		onaction = sds[2];
		offaction = sds[3];
		status = sds[4];

		if (scenetype == SGTYPE_GROUP)
		{
			//when asking for Toggle, just switch to the opposite value
			if (switchcmd == "Toggle") {
				nValue = (atoi(status.c_str()) == 0 ? 1 : 0);
				switchcmd = (nValue == 1 ? "On" : "Off");
			}
		}
		m_sql.HandleOnOffAction((nValue == 1), onaction, offaction);
	}

	m_sql.safe_query("INSERT INTO SceneLog (SceneRowID, nValue) VALUES ('%" PRIu64 "', '%d')", idx, nValue);

	std::string szLastUpdate = TimeToString(NULL, TF_DateTime);
	m_sql.safe_query("UPDATE Scenes SET nValue=%d, LastUpdate='%q' WHERE (ID == %" PRIu64 ")",
		nValue,
		szLastUpdate.c_str(),
		idx);

	_log.Log(LOG_NORM, "Activating Scene/Group: [%s]", Name.c_str());

	bool bEventTrigger = true;
	bEventTrigger = m_eventsystem.UpdateSceneGroup(idx, nValue, szLastUpdate);

	// Notify listeners
	sOnSwitchScene(idx, Name);

	//now switch all attached devices, and only the onces that do not trigger a scene
	result = m_sql.safe_query(
		"SELECT DeviceRowID, Cmd, Level, Color, OnDelay, OffDelay FROM SceneDevices WHERE (SceneRowID == %" PRIu64 ") ORDER BY [Order] ASC", idx);
	if (result.empty())
		return true; //no devices in the scene

	for (const auto & itt : result)
	{
		std::vector<std::string> sd = itt;

		int cmd = atoi(sd[1].c_str());
		int level = atoi(sd[2].c_str());
		_tColor color(sd[3]);
		int ondelay = atoi(sd[4].c_str());
		int offdelay = atoi(sd[5].c_str());
		std::vector<std::vector<std::string> > result2;
		result2 = m_sql.safe_query(
			"SELECT HardwareID, DeviceID,Unit,Type,SubType,SwitchType, nValue, sValue, Name FROM DeviceStatus WHERE (ID == '%q')", sd[0].c_str());
		if (!result2.empty())
		{
			std::vector<std::string> sd2 = result2[0];
			uint8_t rnValue = atoi(sd2[6].c_str());
			std::string sValue = sd2[7];
			uint8_t Unit = atoi(sd2[2].c_str());
			uint8_t dType = atoi(sd2[3].c_str());
			uint8_t dSubType = atoi(sd2[4].c_str());
			std::string DeviceName = sd2[8];
			_eSwitchType switchtype = (_eSwitchType)atoi(sd2[5].c_str());

			//Check if this device will not activate a scene
			uint64_t dID = std::strtoull(sd[0].c_str(), nullptr, 10);
			if (DoesDeviceActiveAScene(dID, cmd))
			{
				_log.Log(LOG_ERROR, "Skipping sensor '%s' because this triggers another scene!", DeviceName.c_str());
				continue;
			}

			std::string lstatus = switchcmd;
			int llevel = 0;
			bool bHaveDimmer = false;
			bool bHaveGroupCmd = false;
			int maxDimLevel = 0;

			GetLightStatus(dType, dSubType, switchtype, cmd, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);

			if (scenetype == SGTYPE_GROUP)
			{
				lstatus = ((switchcmd == "On") || (switchcmd == "Group On") || (switchcmd == "Chime") || (switchcmd == "All On")) ? "On" : "Off";
			}
			_log.Log(LOG_NORM, "Activating Scene/Group Device: %s (%s)", DeviceName.c_str(), lstatus.c_str());


			int ilevel = maxDimLevel - 1; // Why -1?

			if (
				((switchtype == STYPE_Dimmer) ||
				(switchtype == STYPE_BlindsPercentage) ||
					(switchtype == STYPE_BlindsPercentageInverted) ||
					(switchtype == STYPE_Selector)
					) && (maxDimLevel != 0))
			{
				if (lstatus == "On")
				{
					lstatus = "Set Level";
					float fLevel = (maxDimLevel / 100.0f)*level;
					if (fLevel > 100)
						fLevel = 100;
					ilevel = round(fLevel);
				}
				if (switchtype == STYPE_Selector) {
					if (lstatus != "Set Level") {
						ilevel = 0;
					}
					ilevel = round(ilevel / 10.0f) * 10; // select only multiples of 10
					if (ilevel == 0) {
						lstatus = "Off";
					}
				}
			}

			int idx = atoi(sd[0].c_str());
			if (switchtype != STYPE_PushOn)
			{
				int delay = (lstatus == "Off") ? offdelay : ondelay;
				if (!bEventTrigger)
					m_eventsystem.SetEventTrigger(idx, m_eventsystem.REASON_DEVICE, static_cast<float>(delay));
				SwitchLight(idx, lstatus, ilevel, color, false, delay);
				if (scenetype == SGTYPE_SCENE)
				{
					if ((lstatus != "Off") && (offdelay > 0))
					{
						//switch with on delay, and off delay
						if (!bEventTrigger)
							m_eventsystem.SetEventTrigger(idx, m_eventsystem.REASON_DEVICE, static_cast<float>(ondelay + offdelay));
						SwitchLight(idx, "Off", ilevel, color, false, ondelay + offdelay);
					}
				}
			}
			else
			{
				if (!bEventTrigger)
					m_eventsystem.SetEventTrigger(idx, m_eventsystem.REASON_DEVICE, static_cast<float>(ondelay));
				SwitchLight(idx, "On", ilevel, color, false, ondelay);
			}
			sleep_milliseconds(50);
		}
	}
	return true;
}

void MainWorker::CheckSceneCode(const uint64_t DevRowIdx, const uint8_t dType, const uint8_t dSubType, const int nValue, const char* sValue)
{
	//check for scene code
	std::vector<std::vector<std::string> > result;

	result = m_sql.safe_query("SELECT ID, Activators, SceneType FROM Scenes WHERE (Activators!='')");
	if (!result.empty())
	{
		for (const auto & itt : result)
		{
			std::vector<std::string> sd = itt;

			std::vector<std::string> arrayActivators;
			StringSplit(sd[1], ";", arrayActivators);
			for (const auto & ittAct : arrayActivators)
			{
				std::string sCodeCmd = ittAct;

				std::vector<std::string> arrayCode;
				StringSplit(sCodeCmd, ":", arrayCode);

				std::string sID = arrayCode[0];
				std::string sCode = "";
				if (arrayCode.size() == 2)
				{
					sCode = arrayCode[1];
				}

				uint64_t aID = std::strtoull(sID.c_str(), nullptr, 10);
				if (aID == DevRowIdx)
				{
					uint64_t ID = std::strtoull(sd[0].c_str(), nullptr, 10);
					int scenetype = atoi(sd[2].c_str());
					int rnValue = nValue;

					if ((scenetype == SGTYPE_SCENE) && (!sCode.empty()))
					{
						//Also check code
						int iCode = atoi(sCode.c_str());
						if (iCode != nValue)
							continue;
						rnValue = 1; //A Scene can only be activated (On)
					}

					std::string lstatus = "";
					int llevel = 0;
					bool bHaveDimmer = false;
					bool bHaveGroupCmd = false;
					int maxDimLevel = 0;

					GetLightStatus(dType, dSubType, STYPE_OnOff, rnValue, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);
					std::string switchcmd = (IsLightSwitchOn(lstatus) == true) ? "On" : "Off";

					m_sql.AddTaskItem(_tTaskItem::SwitchSceneEvent(0.2f, ID, switchcmd, "SceneTrigger"));
				}
			}
		}
	}
}

void MainWorker::LoadSharedUsers()
{
	std::vector<tcp::server::_tRemoteShareUser> users;

	std::vector<std::vector<std::string> > result;
	std::vector<std::vector<std::string> > result2;

	result = m_sql.safe_query("SELECT ID, Username, Password FROM USERS WHERE ((RemoteSharing==1) AND (Active==1))");
	if (!result.empty())
	{
		for (const auto & itt : result)
		{
			std::vector<std::string> sd = itt;
			tcp::server::_tRemoteShareUser suser;
			suser.Username = base64_decode(sd[1]);
			suser.Password = sd[2];

			//Get User Devices
			result2 = m_sql.safe_query("SELECT DeviceRowID FROM SharedDevices WHERE (SharedUserID == '%q')", sd[0].c_str());
			if (!result2.empty())
			{
				for (const auto & itt2 : result2)
				{
					std::vector<std::string> sd2 = itt2;
					uint64_t ID = std::strtoull(sd2[0].c_str(), nullptr, 10);
					suser.Devices.push_back(ID);
				}
			}
			users.push_back(suser);
		}
	}
	m_sharedserver.SetRemoteUsers(users);
	m_sharedserver.stopAllClients();
}

void MainWorker::SetInternalSecStatus()
{
	m_eventsystem.WWWUpdateSecurityState(m_SecStatus);

	//Update Domoticz Security Device
	RBUF tsen;
	memset(&tsen, 0, sizeof(RBUF));
	tsen.SECURITY1.packetlength = sizeof(tsen.TEMP) - 1;
	tsen.SECURITY1.packettype = pTypeSecurity1;
	tsen.SECURITY1.subtype = sTypeDomoticzSecurity;
	tsen.SECURITY1.battery_level = 9;
	tsen.SECURITY1.rssi = 12;
	tsen.SECURITY1.id1 = 0x14;
	tsen.SECURITY1.id2 = 0x87;
	tsen.SECURITY1.id3 = 0x02;
	tsen.SECURITY1.seqnbr = 1;
	if (m_SecStatus == SECSTATUS_DISARMED)
		tsen.SECURITY1.status = sStatusNormal;
	else if (m_SecStatus == SECSTATUS_ARMEDHOME)
		tsen.SECURITY1.status = sStatusArmHome;
	else
		tsen.SECURITY1.status = sStatusArmAway;

	if (_log.IsDebugLevelEnabled(DEBUG_RECEIVED))
	{
		_log.Log(LOG_NORM, "(System) Domoticz Security Status");
	}

	CDomoticzHardwareBase *pHardware = GetHardwareByType(HTYPE_DomoticzInternal);
	//PushAndWaitRxMessage(pHardware, (const uint8_t *)&tsen, "Domoticz Security Panel", -1);
}

void MainWorker::UpdateDomoticzSecurityStatus(const int iSecStatus)
{
	m_SecCountdown = -1; //cancel possible previous delay
	m_SecStatus = iSecStatus;

	m_sql.UpdatePreferencesVar("SecStatus", iSecStatus);

	int nValue = 0;
	m_sql.GetPreferencesVar("SecOnDelay", nValue);

	if ((nValue == 0) || (iSecStatus == SECSTATUS_DISARMED))
	{
		//Do it Directly
		SetInternalSecStatus();
	}
	else
	{
		//Schedule It
		m_SecCountdown = nValue;
	}
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
	m_sql.AddTaskItem(_tTaskItem::SendEmail(1, sTopic, sstr.str()));
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
	std::vector<CDomoticzHardwareBase*>::const_iterator itt;
	for (itt = m_hardwaredevices.begin(); itt != m_hardwaredevices.end(); ++itt)
	{
		CDomoticzHardwareBase *pHardware = (CDomoticzHardwareBase *)(*itt);
		if (!pHardware->m_bSkipReceiveCheck)
		{
			//Skip Dummy Hardware
			bool bDoCheck = (pHardware->HwdType != HTYPE_Dummy) && (pHardware->HwdType != HTYPE_EVOHOME_SCRIPT);
			if (bDoCheck)
			{
				//Check Thread Timeout
				double diff = difftime(now, pHardware->m_LastHeartbeat);
				//_log.Log(LOG_STATUS, "%d last checking  %.2lf seconds ago", iterator->first, dif);
				if (diff > 60)
				{
					std::vector<std::vector<std::string> > result;
					result = m_sql.safe_query("SELECT Name FROM Hardware WHERE (ID='%d')", pHardware->m_HwdID);
					if (result.size() == 1)
					{
						std::vector<std::string> sd = result[0];
						_log.Log(LOG_ERROR, "%s hardware (%d) thread seems to have ended unexpectedly", sd[0].c_str(), pHardware->m_HwdID);
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
					result = m_sql.safe_query("SELECT Name FROM Hardware WHERE (ID='%d')", pHardware->m_HwdID);
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

						_log.Log(LOG_ERROR, "%s hardware (%d) nothing received for more than %d %s!....", sd[0].c_str(), pHardware->m_HwdID, totNum, sDataTimeout.c_str());
						m_devicestorestart.push_back(pHardware->m_HwdID);
					}
				}
			}

		}
	}
}

bool MainWorker::UpdateDevice(const int DevIdx, int nValue, std::string& sValue, const int signallevel, const int batterylevel, const bool parseTrigger)
{
	// Get the raw device parameters
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query("SELECT HardwareID, DeviceID, Unit, Type, SubType FROM DeviceStatus WHERE (ID==%d)", DevIdx);
	if (result.empty())
		return false;

	int HardwareID = std::stoi(result[0][0]);
	std::string DeviceID = result[0][1];
	int unit = std::stoi(result[0][2]);
	int devType = std::stoi(result[0][3]);
	int subType = std::stoi(result[0][4]);

	return UpdateDevice(HardwareID, DeviceID, unit, devType, subType, nValue, sValue, signallevel, batterylevel, parseTrigger);
}

bool MainWorker::UpdateDevice(const int HardwareID, const std::string &DeviceID, const int unit, const int devType, const int subType, int nValue, std::string &sValue, const int signallevel, const int batterylevel, const bool parseTrigger)
{
	CDomoticzHardwareBase *pHardware = GetHardware(HardwareID);

	// Prevent hazardous modification of DB from JSON calls
	if (!m_sql.DoesDeviceExist(HardwareID, DeviceID.c_str(), unit, devType, subType))
		return false;

	unsigned long ID = 0;
	std::stringstream s_strid;
	s_strid << std::hex << DeviceID;
	s_strid >> ID;

	float temp = 12345.0f;

	if (pHardware)
	{
		if (devType == pTypeLighting2)
		{
			//Update as Lighting 2
			unsigned long ID;
			std::stringstream s_strid;
			s_strid << std::hex << DeviceID;
			s_strid >> ID;
			uint8_t ID1 = (uint8_t)((ID & 0xFF000000) >> 24);
			uint8_t ID2 = (uint8_t)((ID & 0x00FF0000) >> 16);
			uint8_t ID3 = (uint8_t)((ID & 0x0000FF00) >> 8);
			uint8_t ID4 = (uint8_t)((ID & 0x000000FF));

			tRBUF lcmd;
			memset(&lcmd, 0, sizeof(RBUF));
			lcmd.LIGHTING2.packetlength = sizeof(lcmd.LIGHTING2) - 1;
			lcmd.LIGHTING2.packettype = pTypeLighting2;
			lcmd.LIGHTING2.subtype = subType;
			lcmd.LIGHTING2.id1 = ID1;
			lcmd.LIGHTING2.id2 = ID2;
			lcmd.LIGHTING2.id3 = ID3;
			lcmd.LIGHTING2.id4 = ID4;
			lcmd.LIGHTING2.unitcode = (uint8_t)unit;
			lcmd.LIGHTING2.cmnd = (uint8_t)nValue;
			lcmd.LIGHTING2.level = (uint8_t)atoi(sValue.c_str());
			lcmd.LIGHTING2.filler = 0;
			lcmd.LIGHTING2.rssi = signallevel;
			//DecodeRXMessage(pHardware, (const uint8_t *)&lcmd.LIGHTING2, NULL, batterylevel);
			return true;
		}

		if (
			(devType == pTypeTEMP)
			|| (devType == pTypeTEMP_HUM)
			|| (devType == pTypeTEMP_HUM_BARO)
			|| (devType == pTypeBARO)
			)
		{
			//Adjustment value
			float AddjValue = 0.0f;
			float AddjMulti = 1.0f;

			char szTmp[100];
			std::vector<std::string> strarray;

			if (devType == pTypeTEMP)
			{
				temp = static_cast<float>(atof(sValue.c_str()));
				temp += AddjValue;
				sprintf(szTmp, "%.2f", temp);
				sValue = szTmp;
			}
			else if (devType == pTypeTEMP_HUM)
			{
				StringSplit(sValue, ";", strarray);
				if (strarray.size() == 3)
				{
					temp = static_cast<float>(atof(strarray[0].c_str()));
					temp += AddjValue;
					sprintf(szTmp, "%.2f;%s;%s", temp, strarray[1].c_str(), strarray[2].c_str());
					sValue = szTmp;
				}
			}
			else if (devType == pTypeTEMP_HUM_BARO)
			{
				StringSplit(sValue, ";", strarray);
				if (strarray.size() == 5)
				{
					temp = static_cast<float>(atof(strarray[0].c_str()));
					float fbarometer = static_cast<float>(atof(strarray[3].c_str()));
					temp += AddjValue;

					AddjValue = 0.0f;
					AddjMulti = 1.0f;
					fbarometer += AddjValue;

					if (subType == sTypeTHBFloat)
					{
						sprintf(szTmp, "%.2f;%s;%s;%.1f;%s", temp, strarray[1].c_str(), strarray[2].c_str(), fbarometer, strarray[4].c_str());
					}
					else
					{
						sprintf(szTmp, "%.2f;%s;%s;%d;%s", temp, strarray[1].c_str(), strarray[2].c_str(), (int)rint(fbarometer), strarray[4].c_str());
					}
					sValue = szTmp;
				}
			}
		}
	}

	std::string devname = "Unknown";

#ifdef ENABLE_PYTHON
	// notify plugin
	//m_pluginsystem.DeviceModified(devidx);
#endif

	// signal connected devices (MQTT, fibaro, http push ... ) about the web update
	if (parseTrigger)
	{
		//sOnDeviceReceived(HardwareID, devidx, devname, NULL);
	}

	std::stringstream sidx;
	//sidx << devidx;

	if ((devType == pTypeGeneral) && (subType == sTypeZWaveThermostatMode))
	{
		_log.Log(LOG_NORM, "Sending Thermostat Mode to device....");
		SetZWaveThermostatMode(sidx.str(), nValue);
	}
	else if ((devType == pTypeGeneral) && (subType == sTypeZWaveThermostatFanMode))
	{
		_log.Log(LOG_NORM, "Sending Thermostat Fan Mode to device....");
		SetZWaveThermostatFanMode(sidx.str(), nValue);
	}
	else if (pHardware) {
		//Handle Notification
		//m_notifications.CheckAndHandleNotification(devidx, HardwareID, DeviceID, devname, unit, devType, subType, nValue, sValue);
	}
	return true;
}
