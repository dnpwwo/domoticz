#include "stdafx.h"
#include "WebServer.h"
#include "WebServerHelper.h"
#include <boost/bind.hpp>
#include <iostream>
#include <fstream>
#include "mainworker.h"
#include "Helper.h"
#include "localtime_r.h"
#include "EventSystem.h"
#include "HTMLSanitizer.h"
#include "../httpclient/HTTPClient.h"
#include "../hardware/hardwaretypes.h"
#ifdef WITH_OPENZWAVE
#include "../hardware/OpenZWave.h"
#endif
#include "../hardware/RFXBase.h"
#include "../webserver/Base64.h"
#include "../json/json.h"
#include "Logger.h"
#include "SQLHelper.h"
#include "../push/BasePush.h"
#include <algorithm>
#ifdef ENABLE_PYTHON
#include "../hardware/plugins/Plugins.h"
#endif

#ifndef WIN32
#include <sys/utsname.h>
#include <dirent.h>
#else
#include "../msbuild/WindowsHelper.h"
#include "dirent_windows.h"
#endif
#include "../notifications/NotificationHelper.h"
#include "../main/LuaHandler.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define round(a) ( int ) ( a + .5 )

extern std::string szUserDataFolder;
extern std::string szWWWFolder;

extern std::string szAppVersion;
extern std::string szAppHash;
extern std::string szAppDate;
extern std::string szPyVersion;

extern bool g_bUseUpdater;

extern time_t m_StartTime;

extern bool g_bDontCacheWWW;

struct _tGuiLanguage {
	const char* szShort;
	const char* szLong;
};

static const _tGuiLanguage guiLanguage[] =
{
	{ "en", "English" },
	{ "sq", "Albanian" },
	{ "ar", "Arabic" },
	{ "bs", "Bosnian" },
	{ "bg", "Bulgarian" },
	{ "ca", "Catalan" },
	{ "zh", "Chinese" },
	{ "cs", "Czech" },
	{ "da", "Danish" },
	{ "nl", "Dutch" },
	{ "et", "Estonian" },
	{ "de", "German" },
	{ "el", "Greek" },
	{ "fr", "French" },
	{ "fi", "Finnish" },
	{ "he", "Hebrew" },
	{ "hu", "Hungarian" },
	{ "is", "Icelandic" },
	{ "it", "Italian" },
	{ "lt", "Lithuanian" },
	{ "lv", "Latvian" },
	{ "mk", "Macedonian" },
	{ "no", "Norwegian" },
	{ "fa", "Persian" },
	{ "pl", "Polish" },
	{ "pt", "Portuguese" },
	{ "ro", "Romanian" },
	{ "ru", "Russian" },
	{ "sr", "Serbian" },
	{ "sk", "Slovak" },
	{ "sl", "Slovenian" },
	{ "es", "Spanish" },
	{ "sv", "Swedish" },
	{ "zh_TW", "Taiwanese" },
	{ "tr", "Turkish" },
	{ "uk", "Ukrainian" },
	{ NULL, NULL }
};

extern http::server::CWebServerHelper m_webservers;

namespace http {
	namespace server {

		CWebServer::CWebServer(void) : session_store()
		{
			m_pWebEm = NULL;
			m_bDoStop = false;
#ifdef WITH_OPENZWAVE
			m_ZW_Hwidx = -1;
#endif
		}


		CWebServer::~CWebServer(void)
		{
			// RK, we call StopServer() instead of just deleting m_pWebEm. The Do_Work thread might still be accessing that object
			StopServer();
		}

		void CWebServer::Do_Work()
		{
			bool exception_thrown = false;
			while (!m_bDoStop)
			{
				exception_thrown = false;
				try {
					if (m_pWebEm) {
						m_pWebEm->Run();
					}
				}
				catch (std::exception& e) {
					_log.Log(LOG_ERROR, "WebServer(%s) exception occurred : '%s'", m_server_alias.c_str(), e.what());
					exception_thrown = true;
				}
				catch (...) {
					_log.Log(LOG_ERROR, "WebServer(%s) unknown exception occurred", m_server_alias.c_str());
					exception_thrown = true;
				}
				if (exception_thrown) {
					_log.Log(LOG_STATUS, "WebServer(%s) restart server in 5 seconds", m_server_alias.c_str());
					sleep_milliseconds(5000); // prevents from an exception flood
					continue;
				}
				break;
			}
			_log.Log(LOG_STATUS, "WebServer(%s) stopped", m_server_alias.c_str());
		}

		void CWebServer::ReloadCustomSwitchIcons()
		{
			m_custom_light_icons.clear();
			m_custom_light_icons_lookup.clear();
			std::string sLine = "";

			//First get them from the switch_icons.txt file
			std::ifstream infile;
			std::string switchlightsfile = szWWWFolder + "/switch_icons.txt";
			infile.open(switchlightsfile.c_str());
			if (infile.is_open())
			{
				int index = 0;
				while (!infile.eof())
				{
					getline(infile, sLine);
					if (sLine.size() != 0)
					{
						std::vector<std::string> results;
						StringSplit(sLine, ";", results);
						if (results.size() == 3)
						{
							_tCustomIcon cImage;
							cImage.idx = index++;
							cImage.RootFile = results[0];
							cImage.Title = results[1];
							cImage.Description = results[2];
							m_custom_light_icons.push_back(cImage);
							m_custom_light_icons_lookup[cImage.idx] = m_custom_light_icons.size() - 1;
						}
					}
				}
				infile.close();
			}
			//Now get them from the database (idx 100+)
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT ID,Base,Name,Description FROM CustomImages");
			if (!result.empty())
			{
				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					int ID = atoi(sd[0].c_str());

					_tCustomIcon cImage;
					cImage.idx = 100 + ID;
					cImage.RootFile = sd[1];
					cImage.Title = sd[2];
					cImage.Description = sd[3];

					std::string IconFile16 = cImage.RootFile + ".png";
					std::string IconFile48On = cImage.RootFile + "48_On.png";
					std::string IconFile48Off = cImage.RootFile + "48_Off.png";

					std::map<std::string, std::string> _dbImageFiles;
					_dbImageFiles["IconSmall"] = szWWWFolder + "/images/" + IconFile16;
					_dbImageFiles["IconOn"] = szWWWFolder + "/images/" + IconFile48On;
					_dbImageFiles["IconOff"] = szWWWFolder + "/images/" + IconFile48Off;

					//Check if files are on disk, else add them
					for (const auto & iItt : _dbImageFiles)
					{
						std::string TableField = iItt.first;
						std::string IconFile = iItt.second;

						if (!file_exist(IconFile.c_str()))
						{
							//Does not exists, extract it from the database and add it
							std::vector<std::vector<std::string> > result2;
							result2 = m_sql.safe_queryBlob("SELECT %s FROM CustomImages WHERE ID=%d", TableField.c_str(), ID);
							if (!result2.empty())
							{
								std::ofstream file;
								file.open(IconFile.c_str(), std::ios::out | std::ios::binary);
								if (!file.is_open())
									return;

								file << result2[0][0];
								file.close();
							}
						}
					}

					m_custom_light_icons.push_back(cImage);
					m_custom_light_icons_lookup[cImage.idx] = m_custom_light_icons.size() - 1;
					ii++;
				}
			}
		}

		bool CWebServer::StartServer(server_settings & settings, const std::string & serverpath, const bool bIgnoreUsernamePassword)
		{
			m_server_alias = (settings.is_secure() == true) ? "SSL" : "HTTP";

			if (!settings.is_enabled())
				return true;

			ReloadCustomSwitchIcons();

			int tries = 0;
			bool exception = false;

			//_log.Log(LOG_STATUS, "CWebServer::StartServer() : settings : %s", settings.to_string().c_str());
			do {
				try {
					exception = false;
					m_pWebEm = new http::server::cWebem(settings, serverpath.c_str());
				}
				catch (std::exception& e) {
					exception = true;
					switch (tries) {
					case 0:
						settings.listening_address = "::";
						break;
					case 1:
						settings.listening_address = "0.0.0.0";
						break;
					case 2:
						_log.Log(LOG_ERROR, "WebServer(%s) startup failed on address %s with port: %s: %s", m_server_alias.c_str(), settings.listening_address.c_str(), settings.listening_port.c_str(), e.what());
						if (atoi(settings.listening_port.c_str()) < 1024)
							_log.Log(LOG_ERROR, "WebServer(%s) check privileges for opening ports below 1024", m_server_alias.c_str());
						else
							_log.Log(LOG_ERROR, "WebServer(%s) check if no other application is using port: %s", m_server_alias.c_str(), settings.listening_port.c_str());
						return false;
					}
					tries++;
				}
			} while (exception);

			_log.Log(LOG_STATUS, "WebServer(%s) started on address: %s with port %s", m_server_alias.c_str(), settings.listening_address.c_str(), settings.listening_port.c_str());

			m_pWebEm->SetDigistRealm("Domoticz.com");
			m_pWebEm->SetSessionStore(this);

			if (!bIgnoreUsernamePassword)
			{
				LoadUsers();
				std::string WebLocalNetworks;
				int nValue;
				if (m_sql.GetPreferencesVar("WebLocalNetworks", nValue, WebLocalNetworks))
				{
					std::vector<std::string> strarray;
					StringSplit(WebLocalNetworks, ";", strarray);
					for (const auto & itt : strarray)
						m_pWebEm->AddLocalNetworks(itt);
					//add local hostname
					m_pWebEm->AddLocalNetworks("");
				}
			}

			std::string WebRemoteProxyIPs;
			int nValue;
			if (m_sql.GetPreferencesVar("WebRemoteProxyIPs", nValue, WebRemoteProxyIPs))
			{
				std::vector<std::string> strarray;
				StringSplit(WebRemoteProxyIPs, ";", strarray);
				for (const auto & itt : strarray)
					m_pWebEm->AddRemoteProxyIPs(itt);
			}

			//register callbacks
			m_pWebEm->RegisterIncludeCode("switchtypes", boost::bind(&CWebServer::DisplaySwitchTypesCombo, this, _1));
			m_pWebEm->RegisterIncludeCode("metertypes", boost::bind(&CWebServer::DisplayMeterTypesCombo, this, _1));
			m_pWebEm->RegisterIncludeCode("timertypes", boost::bind(&CWebServer::DisplayTimerTypesCombo, this, _1));
			m_pWebEm->RegisterIncludeCode("combolanguage", boost::bind(&CWebServer::DisplayLanguageCombo, this, _1));

			m_pWebEm->RegisterPageCode("/json.htm", boost::bind(&CWebServer::GetJSonPage, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/uploadcustomicon", boost::bind(&CWebServer::Post_UploadCustomIcon, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/html5.appcache", boost::bind(&CWebServer::GetAppCache, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/camsnapshot.jpg", boost::bind(&CWebServer::GetCameraSnapshot, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/backupdatabase.php", boost::bind(&CWebServer::GetDatabaseBackup, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/raspberry.cgi", boost::bind(&CWebServer::GetInternalCameraSnapshot, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/uvccapture.cgi", boost::bind(&CWebServer::GetInternalCameraSnapshot, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/images/floorplans/plan", boost::bind(&CWebServer::GetFloorplanImage, this, _1, _2, _3));

			m_pWebEm->RegisterActionCode("uploadfloorplanimage", boost::bind(&CWebServer::UploadFloorplanImage, this, _1, _2, _3));

			m_pWebEm->RegisterActionCode("restoredatabase", boost::bind(&CWebServer::RestoreDatabase, this, _1, _2, _3));

			m_pWebEm->RegisterActionCode("event_create", boost::bind(&CWebServer::EventCreate, this, _1, _2, _3));

			RegisterCommandCode("getlanguage", boost::bind(&CWebServer::Cmd_GetLanguage, this, _1, _2, _3), true);
			RegisterCommandCode("getthemes", boost::bind(&CWebServer::Cmd_GetThemes, this, _1, _2, _3), true);
			RegisterCommandCode("gettitle", boost::bind(&CWebServer::Cmd_GetTitle, this, _1, _2, _3), true);

			RegisterCommandCode("logincheck", boost::bind(&CWebServer::Cmd_LoginCheck, this, _1, _2, _3), true);
			RegisterCommandCode("getversion", boost::bind(&CWebServer::Cmd_GetVersion, this, _1, _2, _3), true);
			RegisterCommandCode("getlog", boost::bind(&CWebServer::Cmd_GetLog, this, _1, _2, _3));
			RegisterCommandCode("clearlog", boost::bind(&CWebServer::Cmd_ClearLog, this, _1, _2, _3));
			RegisterCommandCode("getauth", boost::bind(&CWebServer::Cmd_GetAuth, this, _1, _2, _3), true);
			RegisterCommandCode("getuptime", boost::bind(&CWebServer::Cmd_GetUptime, this, _1, _2, _3), true);


			RegisterCommandCode("gethardwaretypes", boost::bind(&CWebServer::Cmd_GetHardwareTypes, this, _1, _2, _3));
			RegisterCommandCode("addhardware", boost::bind(&CWebServer::Cmd_AddHardware, this, _1, _2, _3));
			RegisterCommandCode("updatehardware", boost::bind(&CWebServer::Cmd_UpdateHardware, this, _1, _2, _3));
			RegisterCommandCode("deletehardware", boost::bind(&CWebServer::Cmd_DeleteHardware, this, _1, _2, _3));

			RegisterCommandCode("addcamera", boost::bind(&CWebServer::Cmd_AddCamera, this, _1, _2, _3));
			RegisterCommandCode("updatecamera", boost::bind(&CWebServer::Cmd_UpdateCamera, this, _1, _2, _3));
			RegisterCommandCode("deletecamera", boost::bind(&CWebServer::Cmd_DeleteCamera, this, _1, _2, _3));

			RegisterCommandCode("getdevicevalueoptions", boost::bind(&CWebServer::Cmd_GetDeviceValueOptions, this, _1, _2, _3));
			RegisterCommandCode("getdevicevalueoptionwording", boost::bind(&CWebServer::Cmd_GetDeviceValueOptionWording, this, _1, _2, _3));

			RegisterCommandCode("allownewhardware", boost::bind(&CWebServer::Cmd_AllowNewHardware, this, _1, _2, _3));

			RegisterCommandCode("addplan", boost::bind(&CWebServer::Cmd_AddPlan, this, _1, _2, _3));
			RegisterCommandCode("updateplan", boost::bind(&CWebServer::Cmd_UpdatePlan, this, _1, _2, _3));
			RegisterCommandCode("deleteplan", boost::bind(&CWebServer::Cmd_DeletePlan, this, _1, _2, _3));
			RegisterCommandCode("getunusedplandevices", boost::bind(&CWebServer::Cmd_GetUnusedPlanDevices, this, _1, _2, _3));
			RegisterCommandCode("addplanactivedevice", boost::bind(&CWebServer::Cmd_AddPlanActiveDevice, this, _1, _2, _3));
			RegisterCommandCode("getplandevices", boost::bind(&CWebServer::Cmd_GetPlanDevices, this, _1, _2, _3));
			RegisterCommandCode("deleteplandevice", boost::bind(&CWebServer::Cmd_DeletePlanDevice, this, _1, _2, _3));
			RegisterCommandCode("setplandevicecoords", boost::bind(&CWebServer::Cmd_SetPlanDeviceCoords, this, _1, _2, _3));
			RegisterCommandCode("deleteallplandevices", boost::bind(&CWebServer::Cmd_DeleteAllPlanDevices, this, _1, _2, _3));
			RegisterCommandCode("changeplanorder", boost::bind(&CWebServer::Cmd_ChangePlanOrder, this, _1, _2, _3));
			RegisterCommandCode("changeplandeviceorder", boost::bind(&CWebServer::Cmd_ChangePlanDeviceOrder, this, _1, _2, _3));

			RegisterCommandCode("gettimerplans", boost::bind(&CWebServer::Cmd_GetTimerPlans, this, _1, _2, _3));
			RegisterCommandCode("addtimerplan", boost::bind(&CWebServer::Cmd_AddTimerPlan, this, _1, _2, _3));
			RegisterCommandCode("updatetimerplan", boost::bind(&CWebServer::Cmd_UpdateTimerPlan, this, _1, _2, _3));
			RegisterCommandCode("deletetimerplan", boost::bind(&CWebServer::Cmd_DeleteTimerPlan, this, _1, _2, _3));
			RegisterCommandCode("duplicatetimerplan", boost::bind(&CWebServer::Cmd_DuplicateTimerPlan, this, _1, _2, _3));

			RegisterCommandCode("getactualhistory", boost::bind(&CWebServer::Cmd_GetActualHistory, this, _1, _2, _3));
			RegisterCommandCode("getnewhistory", boost::bind(&CWebServer::Cmd_GetNewHistory, this, _1, _2, _3));

			RegisterCommandCode("getconfig", boost::bind(&CWebServer::Cmd_GetConfig, this, _1, _2, _3), true);
			RegisterCommandCode("udevice", boost::bind(&CWebServer::Cmd_UpdateDevice, this, _1, _2, _3));
			RegisterCommandCode("udevices", boost::bind(&CWebServer::Cmd_UpdateDevices, this, _1, _2, _3));
			RegisterCommandCode("system_shutdown", boost::bind(&CWebServer::Cmd_SystemShutdown, this, _1, _2, _3));
			RegisterCommandCode("system_reboot", boost::bind(&CWebServer::Cmd_SystemReboot, this, _1, _2, _3));
			RegisterCommandCode("execute_script", boost::bind(&CWebServer::Cmd_ExcecuteScript, this, _1, _2, _3));
			RegisterCommandCode("getcosts", boost::bind(&CWebServer::Cmd_GetCosts, this, _1, _2, _3));
			RegisterCommandCode("checkforupdate", boost::bind(&CWebServer::Cmd_CheckForUpdate, this, _1, _2, _3));
			RegisterCommandCode("downloadupdate", boost::bind(&CWebServer::Cmd_DownloadUpdate, this, _1, _2, _3));
			RegisterCommandCode("downloadready", boost::bind(&CWebServer::Cmd_DownloadReady, this, _1, _2, _3));
			RegisterCommandCode("deletedatapoint", boost::bind(&CWebServer::Cmd_DeleteDatePoint, this, _1, _2, _3));

			RegisterCommandCode("setactivetimerplan", boost::bind(&CWebServer::Cmd_SetActiveTimerPlan, this, _1, _2, _3));
			RegisterCommandCode("addtimer", boost::bind(&CWebServer::Cmd_AddTimer, this, _1, _2, _3));
			RegisterCommandCode("updatetimer", boost::bind(&CWebServer::Cmd_UpdateTimer, this, _1, _2, _3));
			RegisterCommandCode("deletetimer", boost::bind(&CWebServer::Cmd_DeleteTimer, this, _1, _2, _3));
			RegisterCommandCode("enabletimer", boost::bind(&CWebServer::Cmd_EnableTimer, this, _1, _2, _3));
			RegisterCommandCode("disabletimer", boost::bind(&CWebServer::Cmd_DisableTimer, this, _1, _2, _3));
			RegisterCommandCode("cleartimers", boost::bind(&CWebServer::Cmd_ClearTimers, this, _1, _2, _3));

			RegisterCommandCode("addscenetimer", boost::bind(&CWebServer::Cmd_AddSceneTimer, this, _1, _2, _3));
			RegisterCommandCode("updatescenetimer", boost::bind(&CWebServer::Cmd_UpdateSceneTimer, this, _1, _2, _3));
			RegisterCommandCode("deletescenetimer", boost::bind(&CWebServer::Cmd_DeleteSceneTimer, this, _1, _2, _3));
			RegisterCommandCode("enablescenetimer", boost::bind(&CWebServer::Cmd_EnableSceneTimer, this, _1, _2, _3));
			RegisterCommandCode("disablescenetimer", boost::bind(&CWebServer::Cmd_DisableSceneTimer, this, _1, _2, _3));
			RegisterCommandCode("clearscenetimers", boost::bind(&CWebServer::Cmd_ClearSceneTimers, this, _1, _2, _3));
			RegisterCommandCode("getsceneactivations", boost::bind(&CWebServer::Cmd_GetSceneActivations, this, _1, _2, _3));
			RegisterCommandCode("addscenecode", boost::bind(&CWebServer::Cmd_AddSceneCode, this, _1, _2, _3));
			RegisterCommandCode("removescenecode", boost::bind(&CWebServer::Cmd_RemoveSceneCode, this, _1, _2, _3));
			RegisterCommandCode("clearscenecodes", boost::bind(&CWebServer::Cmd_ClearSceneCodes, this, _1, _2, _3));
			RegisterCommandCode("renamescene", boost::bind(&CWebServer::Cmd_RenameScene, this, _1, _2, _3));

			RegisterCommandCode("serial_devices", boost::bind(&CWebServer::Cmd_GetSerialDevices, this, _1, _2, _3));
			RegisterCommandCode("devices_list", boost::bind(&CWebServer::Cmd_GetDevicesList, this, _1, _2, _3));
			RegisterCommandCode("devices_list_onoff", boost::bind(&CWebServer::Cmd_GetDevicesListOnOff, this, _1, _2, _3));

			RegisterCommandCode("getcustomiconset", boost::bind(&CWebServer::Cmd_GetCustomIconSet, this, _1, _2, _3));
			RegisterCommandCode("deletecustomicon", boost::bind(&CWebServer::Cmd_DeleteCustomIcon, this, _1, _2, _3));
			RegisterCommandCode("updatecustomicon", boost::bind(&CWebServer::Cmd_UpdateCustomIcon, this, _1, _2, _3));

			RegisterCommandCode("renamedevice", boost::bind(&CWebServer::Cmd_RenameDevice, this, _1, _2, _3));
			RegisterCommandCode("setunused", boost::bind(&CWebServer::Cmd_SetUnused, this, _1, _2, _3));

			RegisterCommandCode("addlogmessage", boost::bind(&CWebServer::Cmd_AddLogMessage, this, _1, _2, _3));
			RegisterCommandCode("clearshortlog", boost::bind(&CWebServer::Cmd_ClearShortLog, this, _1, _2, _3));
			RegisterCommandCode("vacuumdatabase", boost::bind(&CWebServer::Cmd_VacuumDatabase, this, _1, _2, _3));

			RegisterCommandCode("addmobiledevice", boost::bind(&CWebServer::Cmd_AddMobileDevice, this, _1, _2, _3));
			RegisterCommandCode("updatemobiledevice", boost::bind(&CWebServer::Cmd_UpdateMobileDevice, this, _1, _2, _3));
			RegisterCommandCode("deletemobiledevice", boost::bind(&CWebServer::Cmd_DeleteMobileDevice, this, _1, _2, _3));

			RegisterRType("lightlog", boost::bind(&CWebServer::RType_LightLog, this, _1, _2, _3));
			RegisterRType("textlog", boost::bind(&CWebServer::RType_TextLog, this, _1, _2, _3));
			RegisterRType("scenelog", boost::bind(&CWebServer::RType_SceneLog, this, _1, _2, _3));
			RegisterRType("settings", boost::bind(&CWebServer::RType_Settings, this, _1, _2, _3));
			RegisterRType("events", boost::bind(&CWebServer::RType_Events, this, _1, _2, _3));

			RegisterRType("hardware", boost::bind(&CWebServer::RType_Hardware, this, _1, _2, _3));
			RegisterRType("devices", boost::bind(&CWebServer::RType_Devices, this, _1, _2, _3));
			RegisterRType("deletedevice", boost::bind(&CWebServer::RType_DeleteDevice, this, _1, _2, _3));
			RegisterRType("cameras", boost::bind(&CWebServer::RType_Cameras, this, _1, _2, _3));
			RegisterRType("cameras_user", boost::bind(&CWebServer::RType_CamerasUser, this, _1, _2, _3));
			RegisterRType("users", boost::bind(&CWebServer::RType_Users, this, _1, _2, _3));
			RegisterRType("mobiles", boost::bind(&CWebServer::RType_Mobiles, this, _1, _2, _3));

			RegisterRType("timers", boost::bind(&CWebServer::RType_Timers, this, _1, _2, _3));
			RegisterRType("scenetimers", boost::bind(&CWebServer::RType_SceneTimers, this, _1, _2, _3));
			RegisterRType("setpointtimers", boost::bind(&CWebServer::RType_SetpointTimers, this, _1, _2, _3));

			RegisterRType("gettransfers", boost::bind(&CWebServer::RType_GetTransfers, this, _1, _2, _3));
			RegisterRType("transferdevice", boost::bind(&CWebServer::RType_TransferDevice, this, _1, _2, _3));
			RegisterRType("notifications", boost::bind(&CWebServer::RType_Notifications, this, _1, _2, _3));
			RegisterRType("schedules", boost::bind(&CWebServer::RType_Schedules, this, _1, _2, _3));
			RegisterRType("getshareduserdevices", boost::bind(&CWebServer::RType_GetSharedUserDevices, this, _1, _2, _3));
			RegisterRType("setshareduserdevices", boost::bind(&CWebServer::RType_SetSharedUserDevices, this, _1, _2, _3));
			RegisterRType("setused", boost::bind(&CWebServer::RType_SetUsed, this, _1, _2, _3));
			RegisterRType("scenes", boost::bind(&CWebServer::RType_Scenes, this, _1, _2, _3));
			RegisterRType("addscene", boost::bind(&CWebServer::RType_AddScene, this, _1, _2, _3));
			RegisterRType("deletescene", boost::bind(&CWebServer::RType_DeleteScene, this, _1, _2, _3));
			RegisterRType("updatescene", boost::bind(&CWebServer::RType_UpdateScene, this, _1, _2, _3));

			RegisterRType("custom_light_icons", boost::bind(&CWebServer::RType_CustomLightIcons, this, _1, _2, _3));
			RegisterRType("plans", boost::bind(&CWebServer::RType_Plans, this, _1, _2, _3));
			RegisterRType("floorplans", boost::bind(&CWebServer::RType_FloorPlans, this, _1, _2, _3));
#ifdef WITH_OPENZWAVE
			//ZWave
			RegisterCommandCode("updatezwavenode", boost::bind(&CWebServer::Cmd_ZWaveUpdateNode, this, _1, _2, _3));
			RegisterCommandCode("deletezwavenode", boost::bind(&CWebServer::Cmd_ZWaveDeleteNode, this, _1, _2, _3));
			RegisterCommandCode("zwaveinclude", boost::bind(&CWebServer::Cmd_ZWaveInclude, this, _1, _2, _3));
			RegisterCommandCode("zwaveexclude", boost::bind(&CWebServer::Cmd_ZWaveExclude, this, _1, _2, _3));

			RegisterCommandCode("zwaveisnodeincluded", boost::bind(&CWebServer::Cmd_ZWaveIsNodeIncluded, this, _1, _2, _3));
			RegisterCommandCode("zwaveisnodeexcluded", boost::bind(&CWebServer::Cmd_ZWaveIsNodeExcluded, this, _1, _2, _3));

			RegisterCommandCode("zwavesoftreset", boost::bind(&CWebServer::Cmd_ZWaveSoftReset, this, _1, _2, _3));
			RegisterCommandCode("zwavehardreset", boost::bind(&CWebServer::Cmd_ZWaveHardReset, this, _1, _2, _3));
			RegisterCommandCode("zwavenetworkheal", boost::bind(&CWebServer::Cmd_ZWaveNetworkHeal, this, _1, _2, _3));
			RegisterCommandCode("zwavenodeheal", boost::bind(&CWebServer::Cmd_ZWaveNodeHeal, this, _1, _2, _3));
			RegisterCommandCode("zwavenetworkinfo", boost::bind(&CWebServer::Cmd_ZWaveNetworkInfo, this, _1, _2, _3));
			RegisterCommandCode("zwaveremovegroupnode", boost::bind(&CWebServer::Cmd_ZWaveRemoveGroupNode, this, _1, _2, _3));
			RegisterCommandCode("zwaveaddgroupnode", boost::bind(&CWebServer::Cmd_ZWaveAddGroupNode, this, _1, _2, _3));
			RegisterCommandCode("zwavegroupinfo", boost::bind(&CWebServer::Cmd_ZWaveGroupInfo, this, _1, _2, _3));
			RegisterCommandCode("zwavecancel", boost::bind(&CWebServer::Cmd_ZWaveCancel, this, _1, _2, _3));
			RegisterCommandCode("applyzwavenodeconfig", boost::bind(&CWebServer::Cmd_ApplyZWaveNodeConfig, this, _1, _2, _3));
			RegisterCommandCode("requestzwavenodeconfig", boost::bind(&CWebServer::Cmd_ZWaveRequestNodeConfig, this, _1, _2, _3));
			RegisterCommandCode("requestzwavenodeinfo", boost::bind(&CWebServer::Cmd_ZWaveRequestNodeInfo, this, _1, _2, _3));
			RegisterCommandCode("zwavestatecheck", boost::bind(&CWebServer::Cmd_ZWaveStateCheck, this, _1, _2, _3));
			RegisterCommandCode("zwavereceiveconfigurationfromothercontroller", boost::bind(&CWebServer::Cmd_ZWaveReceiveConfigurationFromOtherController, this, _1, _2, _3));
			RegisterCommandCode("zwavesendconfigurationtosecondcontroller", boost::bind(&CWebServer::Cmd_ZWaveSendConfigurationToSecondaryController, this, _1, _2, _3));
			RegisterCommandCode("zwavetransferprimaryrole", boost::bind(&CWebServer::Cmd_ZWaveTransferPrimaryRole, this, _1, _2, _3));
			RegisterCommandCode("zwavestartusercodeenrollmentmode", boost::bind(&CWebServer::Cmd_ZWaveSetUserCodeEnrollmentMode, this, _1, _2, _3));
			RegisterCommandCode("zwavegetusercodes", boost::bind(&CWebServer::Cmd_ZWaveGetNodeUserCodes, this, _1, _2, _3));
			RegisterCommandCode("zwaveremoveusercode", boost::bind(&CWebServer::Cmd_ZWaveRemoveUserCode, this, _1, _2, _3));
			RegisterCommandCode("zwavegetbatterylevels", boost::bind(&CWebServer::Cmd_ZWaveGetBatteryLevels, this, _1, _2, _3));

			m_pWebEm->RegisterPageCode("/zwavegetconfig.php", boost::bind(&CWebServer::ZWaveGetConfigFile, this, _1, _2, _3));

			m_pWebEm->RegisterPageCode("/ozwcp/poll.xml", boost::bind(&CWebServer::ZWaveCPPollXml, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/cp.html", boost::bind(&CWebServer::ZWaveCPIndex, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/confparmpost.html", boost::bind(&CWebServer::ZWaveCPNodeGetConf, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/refreshpost.html", boost::bind(&CWebServer::ZWaveCPNodeGetValues, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/valuepost.html", boost::bind(&CWebServer::ZWaveCPNodeSetValue, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/buttonpost.html", boost::bind(&CWebServer::ZWaveCPNodeSetButton, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/admpost.html", boost::bind(&CWebServer::ZWaveCPAdminCommand, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/nodepost.html", boost::bind(&CWebServer::ZWaveCPNodeChange, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/savepost.html", boost::bind(&CWebServer::ZWaveCPSaveConfig, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/thpost.html", boost::bind(&CWebServer::ZWaveCPTestHeal, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/topopost.html", boost::bind(&CWebServer::ZWaveCPGetTopo, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/statpost.html", boost::bind(&CWebServer::ZWaveCPGetStats, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/grouppost.html", boost::bind(&CWebServer::ZWaveCPSetGroup, this, _1, _2, _3));
			m_pWebEm->RegisterPageCode("/ozwcp/scenepost.html", boost::bind(&CWebServer::ZWaveCPSceneCommand, this, _1, _2, _3));
			//
			//pollpost.html
			//scenepost.html
			//thpost.html
			RegisterRType("openzwavenodes", boost::bind(&CWebServer::RType_OpenZWaveNodes, this, _1, _2, _3));
#endif
			m_pWebEm->RegisterWhitelistURLString("/html5.appcache");
			m_pWebEm->RegisterWhitelistURLString("/images/floorplans/plan");

			//Start normal worker thread
			m_bDoStop = false;
			m_thread = std::make_shared<std::thread>(&CWebServer::Do_Work, this);
			std::string server_name = "WebServer_" + settings.listening_port;
			SetThreadName(m_thread->native_handle(), server_name.c_str());
			return (m_thread != nullptr);
		}

		void CWebServer::StopServer()
		{
			m_bDoStop = true;
			try
			{
				if (m_pWebEm == NULL)
					return;
				m_pWebEm->Stop();
				if (m_thread) {
					m_thread->join();
					m_thread.reset();
				}
				delete m_pWebEm;
				m_pWebEm = NULL;
			}
			catch (...)
			{

			}
		}

		void CWebServer::SetWebCompressionMode(const _eWebCompressionMode gzmode)
		{
			if (m_pWebEm == NULL)
				return;
			m_pWebEm->SetWebCompressionMode(gzmode);
		}

		void CWebServer::SetAuthenticationMethod(const _eAuthenticationMethod amethod)
		{
			if (m_pWebEm == NULL)
				return;
			m_pWebEm->SetAuthenticationMethod(amethod);
		}

		void CWebServer::SetWebTheme(const std::string &themename)
		{
			if (m_pWebEm == NULL)
				return;
			m_pWebEm->SetWebTheme(themename);
		}

		void CWebServer::SetWebRoot(const std::string &webRoot)
		{
			if (m_pWebEm == NULL)
				return;
			m_pWebEm->SetWebRoot(webRoot);
		}

		void CWebServer::RegisterCommandCode(const char* idname, webserver_response_function ResponseFunction, bool bypassAuthentication)
		{
			m_webcommands.insert(std::pair<std::string, webserver_response_function >(std::string(idname), ResponseFunction));
			if (bypassAuthentication)
			{
				m_pWebEm->RegisterWhitelistURLString(idname);
			}
		}

		void CWebServer::RegisterRType(const char* idname, webserver_response_function ResponseFunction)
		{
			m_webrtypes.insert(std::pair<std::string, webserver_response_function >(std::string(idname), ResponseFunction));
		}

		void CWebServer::HandleRType(const std::string &rtype, WebEmSession & session, const request& req, Json::Value &root)
		{
			std::map < std::string, webserver_response_function >::iterator pf = m_webrtypes.find(rtype);
			if (pf != m_webrtypes.end())
			{
				pf->second(session, req, root);
			}
		}

		void CWebServer::GetAppCache(WebEmSession & session, const request& req, reply & rep)
		{
			std::string response = "";
			if (g_bDontCacheWWW)
			{
				return;
			}
			//Return the appcache file (dynamically generated)
			std::string sLine;
			std::string filename = szWWWFolder + "/html5.appcache";


			std::string sWebTheme = "default";
			m_sql.GetPreferencesVar("WebTheme", sWebTheme);

			//Get Dynamic Theme Files
			std::map<std::string, int> _ThemeFiles;
			GetDirFilesRecursive(szWWWFolder + "/styles/" + sWebTheme + "/", _ThemeFiles);

			//Get Dynamic Floorplan Images from database
			std::map<std::string, int> _FloorplanFiles;
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT ID FROM Floorplans ORDER BY [Order]");
			if (!result.empty())
			{
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;
					std::string ImageURL = "images/floorplans/plan?idx=" + sd[0];
					_FloorplanFiles[ImageURL] = 1;
				}
			}

			std::ifstream is(filename.c_str());
			if (is)
			{
				while (!is.eof())
				{
					getline(is, sLine);
					if (!sLine.empty())
					{
						if (sLine.find("#BuildHash") != std::string::npos)
						{
							stdreplace(sLine, "#BuildHash", szAppHash);
						}
						else if (sLine.find("#ThemeFiles") != std::string::npos)
						{
							response += "#Theme=" + sWebTheme + '\n';
							//Add all theme files
							for (const auto & itt : _ThemeFiles)
							{
								std::string tfname = itt.first.substr(szWWWFolder.size() + 1);
								stdreplace(tfname, "styles/" + sWebTheme, "acttheme");
								response += tfname + '\n';
							}
							continue;
						}
						else if (sLine.find("#Floorplans") != std::string::npos)
						{
							//Add all floorplans
							for (const auto & itt : _FloorplanFiles)
							{
								std::string tfname = itt.first;
								response += tfname + '\n';
							}
							continue;
						}
						else if (sLine.find("#SwitchIcons") != std::string::npos)
						{
							//Add database switch icons
							for (const auto & itt : m_custom_light_icons)
							{
								if (itt.idx >= 100)
								{
									std::string IconFile16 = itt.RootFile + ".png";
									std::string IconFile48On = itt.RootFile + "48_On.png";
									std::string IconFile48Off = itt.RootFile + "48_Off.png";

									response += "images/" + CURLEncode::URLEncode(IconFile16) + '\n';
									response += "images/" + CURLEncode::URLEncode(IconFile48On) + '\n';
									response += "images/" + CURLEncode::URLEncode(IconFile48Off) + '\n';
								}
							}
						}
					}
					response += sLine + '\n';
				}
			}
			reply::set_content(&rep, response);
		}

		void CWebServer::GetJSonPage(WebEmSession & session, const request& req, reply & rep)
		{
			Json::Value root;
			root["status"] = "ERR";

			std::string rtype = request::findValue(&req, "type");
			if (rtype == "command")
			{
				std::string cparam = request::findValue(&req, "param");
				if (cparam.empty())
				{
					cparam = request::findValue(&req, "dparam");
					if (cparam.empty())
					{
						goto exitjson;
					}
				}
				if (cparam == "dologout")
				{
					session.forcelogin = true;
					root["status"] = "OK";
					root["title"] = "Logout";
					goto exitjson;

				}
				_log.Debug(DEBUG_WEBSERVER, "WEBS GetJSon :%s :%s ", cparam.c_str(), req.uri.c_str());
				HandleCommand(cparam, session, req, root);
			} //(rtype=="command")
			else {
				HandleRType(rtype, session, req, root);
			}
		exitjson:
			std::string jcallback = request::findValue(&req, "jsoncallback");
			if (jcallback.size() == 0) {
				reply::set_content(&rep, root.toStyledString());
				return;
			}
			reply::set_content(&rep, "var data=" + root.toStyledString() + '\n' + jcallback + "(data);");
		}

		void CWebServer::Cmd_GetLanguage(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string sValue;
			if (m_sql.GetPreferencesVar("Language", sValue))
			{
				root["status"] = "OK";
				root["title"] = "GetLanguage";
				root["language"] = sValue;
			}
		}

		void CWebServer::Cmd_GetThemes(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetThemes";
			m_mainworker.GetAvailableWebThemes();
			int ii = 0;
			for (const auto & itt : m_mainworker.m_webthemes)
			{
				root["result"][ii]["theme"] = itt;
				ii++;
			}
		}

		void CWebServer::Cmd_GetTitle(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string sValue;
			root["status"] = "OK";
			root["title"] = "GetTitle";
			if (m_sql.GetPreferencesVar("Title", sValue))
				root["Title"] = sValue;
			else
				root["Title"] = "Domoticz";
		}

		void CWebServer::Cmd_LoginCheck(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string tmpusrname = request::findValue(&req, "username");
			std::string tmpusrpass = request::findValue(&req, "password");
			if (
				(tmpusrname.empty()) ||
				(tmpusrpass.empty())
				)
				return;

			std::string rememberme = request::findValue(&req, "rememberme");

			std::string usrname;
			std::string usrpass;
			if (request_handler::url_decode(tmpusrname, usrname))
			{
				if (request_handler::url_decode(tmpusrpass, usrpass))
				{
					usrname = base64_decode(usrname);
					int iUser = FindUser(usrname.c_str());
					if (iUser == -1) {
						// log brute force attack
						_log.Log(LOG_ERROR, "Failed login attempt from %s for user '%s' !", session.remote_host.c_str(), usrname.c_str());
						return;
					}
					if (m_users[iUser].Password != usrpass) {
						// log brute force attack
						_log.Log(LOG_ERROR, "Failed login attempt from %s for '%s' !", session.remote_host.c_str(), m_users[iUser].Username.c_str());
						return;
					}
					_log.Log(LOG_STATUS, "Login successful from %s for user '%s'", session.remote_host.c_str(), m_users[iUser].Username.c_str());
					root["status"] = "OK";
					root["version"] = szAppVersion;
					root["title"] = "logincheck";
					session.isnew = true;
					session.username = m_users[iUser].Username;
					session.rights = m_users[iUser].userrights;
					session.rememberme = (rememberme == "true");
					root["user"] = session.username;
					root["rights"] = session.rights;
				}
			}
		}

		void CWebServer::Cmd_GetHardwareTypes(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			root["status"] = "OK";
			root["title"] = "GetHardwareTypes";
			std::map<std::string, int> _htypes;
			for (int ii = 0; ii < HTYPE_END; ii++)
			{
				bool bDoAdd = true;
#ifndef _DEBUG
#ifdef WIN32
				if (
					(ii == HTYPE_RaspberryBMP085) ||
					(ii == HTYPE_RaspberryHTU21D) ||
					(ii == HTYPE_RaspberryTSL2561) ||
					(ii == HTYPE_RaspberryPCF8574) ||
					(ii == HTYPE_RaspberryBME280) ||
					(ii == HTYPE_RaspberryMCP23017)
					)
				{
					bDoAdd = false;
				}
				else
				{
#ifndef WITH_LIBUSB
					if (
						(ii == HTYPE_VOLCRAFTCO20) ||
						(ii == HTYPE_TE923)
						)
					{
						bDoAdd = false;
					}
#endif

		}
#endif
#endif
#ifndef WITH_OPENZWAVE
				if (ii == HTYPE_OpenZWave)
					bDoAdd = false;
#endif
#ifndef WITH_GPIO
				if (ii == HTYPE_RaspberryGPIO)
				{
					bDoAdd = false;
				}

				if (ii == HTYPE_SysfsGpio)
				{
					bDoAdd = false;
				}
#endif
				if (ii == HTYPE_PythonPlugin)
					bDoAdd = false;
				if (bDoAdd)
					_htypes[Hardware_Type_Desc(ii)] = ii;
	}
			//return a sorted hardware list
			int ii = 0;
			for (const auto & itt : _htypes)
			{
				root["result"][ii]["idx"] = itt.second;
				root["result"][ii]["name"] = itt.first;
				ii++;
			}

#ifdef ENABLE_PYTHON
			// Append Plugin list as well
			PluginList(root["result"]);
#endif
}

		void CWebServer::Cmd_AddHardware(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string name = HTMLSanitizer::Sanitize(CURLEncode::URLDecode(request::findValue(&req, "name")));
			std::string senabled = request::findValue(&req, "enabled");
			std::string shtype = request::findValue(&req, "htype");
			std::string address = HTMLSanitizer::Sanitize(request::findValue(&req, "address"));
			std::string sport = request::findValue(&req, "port");
			std::string username = HTMLSanitizer::Sanitize(CURLEncode::URLDecode(request::findValue(&req, "username")));
			std::string password = HTMLSanitizer::Sanitize(CURLEncode::URLDecode(request::findValue(&req, "password")));
			std::string extra = CURLEncode::URLDecode(request::findValue(&req, "extra"));
			std::string sdatatimeout = request::findValue(&req, "datatimeout");
			if (
				(name.empty()) ||
				(senabled.empty()) ||
				(shtype.empty())
				)
				return;
			_eHardwareTypes htype = (_eHardwareTypes)atoi(shtype.c_str());

			int iDataTimeout = atoi(sdatatimeout.c_str());
			int mode1 = 0;
			int mode2 = 0;
			int mode3 = 0;
			int mode4 = 0;
			int mode5 = 0;
			int mode6 = 0;
			int port = atoi(sport.c_str());
			std::string mode1Str = request::findValue(&req, "Mode1");
			if (!mode1Str.empty()) {
				mode1 = atoi(mode1Str.c_str());
			}
			std::string mode2Str = request::findValue(&req, "Mode2");
			if (!mode2Str.empty()) {
				mode2 = atoi(mode2Str.c_str());
			}
			std::string mode3Str = request::findValue(&req, "Mode3");
			if (!mode3Str.empty()) {
				mode3 = atoi(mode3Str.c_str());
			}
			std::string mode4Str = request::findValue(&req, "Mode4");
			if (!mode4Str.empty()) {
				mode4 = atoi(mode4Str.c_str());
			}
			std::string mode5Str = request::findValue(&req, "Mode5");
			if (!mode5Str.empty()) {
				mode5 = atoi(mode5Str.c_str());
			}
			std::string mode6Str = request::findValue(&req, "Mode6");
			if (!mode6Str.empty()) {
				mode6 = atoi(mode6Str.c_str());
			}

			if (IsSerialDevice(htype))
			{
				//USB/System
				if (sport.empty())
					return; //need to have a serial port

				if (htype == HTYPE_TeleinfoMeter) {
					// Teleinfo always has decimals. Chances to have a P1 and a Teleinfo device on the same
					// Domoticz instance are very low as both are national standards (NL and FR)
					m_sql.UpdatePreferencesVar("SmartMeterType", 0);
				}
			}
			else if (IsNetworkDevice(htype))
			{
				//Lan
				if (address.empty() || port == 0)
					return;

				if (htype == HTYPE_MySensorsMQTT || htype == HTYPE_MQTT) {
					std::string modeqStr = request::findValue(&req, "mode1");
					if (!modeqStr.empty()) {
						mode1 = atoi(modeqStr.c_str());
					}
				}

				if (htype == HTYPE_ECODEVICES) {
					// EcoDevices always have decimals. Chances to have a P1 and a EcoDevice/Teleinfo device on the same
					// Domoticz instance are very low as both are national standards (NL and FR)
					m_sql.UpdatePreferencesVar("SmartMeterType", 0);
				}
			}
			else if (htype == HTYPE_DomoticzInternal) {
				// DomoticzInternal cannot be added manually
				return;
			}
			else if (htype == HTYPE_Domoticz) {
				//Remote Domoticz
				if (address.empty() || port == 0)
					return;
			}
			else if (htype == HTYPE_TE923) {
				//all fine here!
			}
			else if (htype == HTYPE_VOLCRAFTCO20) {
				//all fine here!
			}
			else if (htype == HTYPE_System) {
				//There should be only one
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT ID FROM Hardware WHERE (Type==%d)", HTYPE_System);
				if (!result.empty())
					return;
			}
			else if (htype == HTYPE_1WIRE) {
				//all fine here!
			}
			else if (htype == HTYPE_Rtl433) {
				//all fine here!
			}
			else if (htype == HTYPE_Pinger) {
				//all fine here!
			}
			else if (htype == HTYPE_Kodi) {
				//all fine here!
			}
			else if (htype == HTYPE_PanasonicTV) {
				// all fine here!
			}
			else if (htype == HTYPE_LogitechMediaServer) {
				//all fine here!
			}
			else if (htype == HTYPE_RaspberryBMP085) {
				//all fine here!
			}
			else if (htype == HTYPE_RaspberryHTU21D) {
				//all fine here!
			}
			else if (htype == HTYPE_RaspberryTSL2561) {
				//all fine here!
			}
			else if (htype == HTYPE_RaspberryBME280) {
				//all fine here!
			}
			else if (htype == HTYPE_RaspberryMCP23017) {
				//all fine here!
			}
			else if (htype == HTYPE_Dummy) {
				//all fine here!
			}
			else if (htype == HTYPE_Tellstick) {
				//all fine here!
			}
			else if (htype == HTYPE_EVOHOME_SCRIPT || htype == HTYPE_EVOHOME_SERIAL || htype == HTYPE_EVOHOME_WEB || htype == HTYPE_EVOHOME_TCP) {
				//all fine here!
			}
			else if (htype == HTYPE_PiFace) {
				//all fine here!
			}
			else if (htype == HTYPE_HTTPPOLLER) {
				//all fine here!
			}
			else if (htype == HTYPE_BleBox) {
				//all fine here!
			}
			else if (htype == HTYPE_HEOS) {
				//all fine here!
			}
			else if (htype == HTYPE_Yeelight) {
				//all fine here!
			}
			else if (htype == HTYPE_XiaomiGateway) {
				//all fine here!
			}
			else if (htype == HTYPE_Arilux) {
				//all fine here!
			}
			else if (htype == HTYPE_USBtinGateway) {
				//All fine here
			}
			else if (htype == HTYPE_BuienRadar) {
				//All fine here
			}
			else if (
				(htype == HTYPE_Wunderground) ||
				(htype == HTYPE_DarkSky) ||
				(htype == HTYPE_AccuWeather) ||
				(htype == HTYPE_OpenWeatherMap) ||
				(htype == HTYPE_ICYTHERMOSTAT) ||
				(htype == HTYPE_TOONTHERMOSTAT) ||
				(htype == HTYPE_AtagOne) ||
				(htype == HTYPE_PVOUTPUT_INPUT) ||
				(htype == HTYPE_NEST) ||
				(htype == HTYPE_ANNATHERMOSTAT) ||
				(htype == HTYPE_THERMOSMART) ||
				(htype == HTYPE_Tado) ||
				(htype == HTYPE_Netatmo)
				)
			{
				if (
					(username.empty()) ||
					(password.empty())
					)
					return;
			}
			else if (htype == HTYPE_SolarEdgeAPI)
			{
				if (
					(username.empty())
					)
					return;
			}
			else if (htype == HTYPE_Nest_OAuthAPI) {
				if (
					(username == "") &&
					(extra == "||")
					)
					return;
			}
			else if (htype == HTYPE_SBFSpot) {
				if (username.empty())
					return;
			}
			else if (htype == HTYPE_HARMONY_HUB) {
				if (
					(address.empty() || port == 0)
					)
					return;
			}
			else if (htype == HTYPE_Philips_Hue) {
				if (
					(username.empty()) ||
					(address.empty() || port == 0)
					)
					return;
				if (port == 0)
					port = 80;
			}
			else if (htype == HTYPE_WINDDELEN) {
				std::string mill_id = request::findValue(&req, "Mode1");
				if (
					(mill_id.empty()) ||
					(sport.empty())
					)

					return;
				mode1 = atoi(mill_id.c_str());
			}
			else if (htype == HTYPE_Honeywell) {
				//all fine here!
			}
			else if (htype == HTYPE_RaspberryGPIO) {
				//all fine here!
			}
			else if (htype == HTYPE_SysfsGpio) {
				//all fine here!
			}
			else if (htype == HTYPE_OpenWebNetTCP) {
				//All fine here
			}
			else if (htype == HTYPE_Daikin) {
				//All fine here
			}
			else if (htype == HTYPE_GoodweAPI) {
				if (username.empty())
					return;
			}
			else if (htype == HTYPE_PythonPlugin) {
				//All fine here
			}
			else if (htype == HTYPE_RaspberryPCF8574) {
				//All fine here
			}
			else if (htype == HTYPE_OpenWebNetUSB) {
				//All fine here
			}
			else if (htype == HTYPE_IntergasInComfortLAN2RF) {
				//All fine here
			}
			else if (htype == HTYPE_EnphaseAPI) {
				//All fine here
			}
			else if (htype == HTYPE_EcoCompteur) {
				//all fine here!
			}
			else
				return;

			root["status"] = "OK";
			root["title"] = "AddHardware";

			std::vector<std::vector<std::string> > result;

			if (htype == HTYPE_Domoticz)
			{
				if (password.size() != 32)
				{
					password = GenerateMD5Hash(password);
				}
			}
			else if ((htype == HTYPE_S0SmartMeterUSB) || (htype == HTYPE_S0SmartMeterTCP))
			{
				extra = "0;1000;0;1000;0;1000;0;1000;0;1000";
			}
			else if (htype == HTYPE_Pinger)
			{
				mode1 = 30;
				mode2 = 1000;
			}
			else if (htype == HTYPE_Kodi)
			{
				mode1 = 30;
				mode2 = 1000;
			}
			else if (htype == HTYPE_PanasonicTV)
			{
				mode1 = 30;
				mode2 = 1000;
			}
			else if (htype == HTYPE_LogitechMediaServer)
			{
				mode1 = 30;
				mode2 = 1000;
			}
			else if (htype == HTYPE_HEOS)
			{
				mode1 = 30;
				mode2 = 1000;
			}
			else if (htype == HTYPE_Tellstick)
			{
				mode1 = 4;
				mode2 = 500;
			}

			if (htype == HTYPE_HTTPPOLLER) {
				m_sql.safe_query(
					"INSERT INTO Hardware (Name, Enabled, Type, Address, Port, SerialPort, Username, Password, Extra, Mode1, Mode2, Mode3, Mode4, Mode5, Mode6, DataTimeout) VALUES ('%q',%d, %d,'%q',%d,'%q','%q','%q','%q','%q','%q', '%q', '%q', '%q', '%q', %d)",
					name.c_str(),
					(senabled == "true") ? 1 : 0,
					htype,
					address.c_str(),
					port,
					sport.c_str(),
					username.c_str(),
					password.c_str(),
					extra.c_str(),
					mode1Str.c_str(), mode2Str.c_str(), mode3Str.c_str(), mode4Str.c_str(), mode5Str.c_str(), mode6Str.c_str(),
					iDataTimeout
				);
			}
			else if (htype == HTYPE_PythonPlugin) {
				sport = request::findValue(&req, "serialport");
				m_sql.safe_query(
					"INSERT INTO Hardware (Name, Enabled, Type, Address, Port, SerialPort, Username, Password, Extra, Mode1, Mode2, Mode3, Mode4, Mode5, Mode6, DataTimeout) VALUES ('%q',%d, %d,'%q',%d,'%q','%q','%q','%q','%q','%q', '%q', '%q', '%q', '%q', %d)",
					name.c_str(),
					(senabled == "true") ? 1 : 0,
					htype,
					address.c_str(),
					port,
					sport.c_str(),
					username.c_str(),
					password.c_str(),
					extra.c_str(),
					mode1Str.c_str(), mode2Str.c_str(), mode3Str.c_str(), mode4Str.c_str(), mode5Str.c_str(), mode6Str.c_str(),
					iDataTimeout
				);
			}
			else if (
				(htype == HTYPE_RFXtrx433)||
				(htype == HTYPE_RFXtrx868)
				)
			{
				//No Extra field here, handled in CWebServer::SetRFXCOMMode
				m_sql.safe_query(
					"INSERT INTO Hardware (Name, Enabled, Type, Address, Port, SerialPort, Username, Password, Mode1, Mode2, Mode3, Mode4, Mode5, Mode6, DataTimeout) VALUES ('%q',%d, %d,'%q',%d,'%q','%q','%q',%d,%d,%d,%d,%d,%d,%d)",
					name.c_str(),
					(senabled == "true") ? 1 : 0,
					htype,
					address.c_str(),
					port,
					sport.c_str(),
					username.c_str(),
					password.c_str(),
					mode1, mode2, mode3, mode4, mode5, mode6,
					iDataTimeout
				);
				extra = "0";
			}
			else {
				m_sql.safe_query(
					"INSERT INTO Hardware (Name, Enabled, Type, Address, Port, SerialPort, Username, Password, Extra, Mode1, Mode2, Mode3, Mode4, Mode5, Mode6, DataTimeout) VALUES ('%q',%d, %d,'%q',%d,'%q','%q','%q','%q',%d,%d,%d,%d,%d,%d,%d)",
					name.c_str(),
					(senabled == "true") ? 1 : 0,
					htype,
					address.c_str(),
					port,
					sport.c_str(),
					username.c_str(),
					password.c_str(),
					extra.c_str(),
					mode1, mode2, mode3, mode4, mode5, mode6,
					iDataTimeout
				);
			}

			//add the device for real in our system
			result = m_sql.safe_query("SELECT MAX(ID) FROM Hardware");
			if (!result.empty())
			{
				std::vector<std::string> sd = result[0];
				int ID = atoi(sd[0].c_str());

				root["idx"] = sd[0].c_str(); // OTO output the created ID for easier management on the caller side (if automated)

				m_mainworker.AddHardwareFromParams(ID, name, (senabled == "true") ? true : false, htype, address, port, sport, username, password, extra, mode1, mode2, mode3, mode4, mode5, mode6, iDataTimeout, true);
			}
		}

		void CWebServer::Cmd_UpdateHardware(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			std::string name = HTMLSanitizer::Sanitize(CURLEncode::URLDecode(request::findValue(&req, "name")));
			std::string senabled = request::findValue(&req, "enabled");
			std::string shtype = request::findValue(&req, "htype");
			std::string address = HTMLSanitizer::Sanitize(request::findValue(&req, "address"));
			std::string sport = request::findValue(&req, "port");
			std::string username = HTMLSanitizer::Sanitize(CURLEncode::URLDecode(request::findValue(&req, "username")));
			std::string password = CURLEncode::URLDecode(request::findValue(&req, "password"));
			std::string extra = HTMLSanitizer::Sanitize(CURLEncode::URLDecode(request::findValue(&req, "extra")));
			std::string sdatatimeout = request::findValue(&req, "datatimeout");

			if (
				(name.empty()) ||
				(senabled.empty()) ||
				(shtype.empty())
				)
				return;

			int mode1 = atoi(request::findValue(&req, "Mode1").c_str());
			int mode2 = atoi(request::findValue(&req, "Mode2").c_str());
			int mode3 = atoi(request::findValue(&req, "Mode3").c_str());
			int mode4 = atoi(request::findValue(&req, "Mode4").c_str());
			int mode5 = atoi(request::findValue(&req, "Mode5").c_str());
			int mode6 = atoi(request::findValue(&req, "Mode6").c_str());

			bool bEnabled = (senabled == "true") ? true : false;

			_eHardwareTypes htype = (_eHardwareTypes)atoi(shtype.c_str());
			int iDataTimeout = atoi(sdatatimeout.c_str());

			int port = atoi(sport.c_str());

			bool bIsSerial = false;

			if (IsSerialDevice(htype))
			{
				//USB/System
				bIsSerial = true;
				if (bEnabled)
				{
					if (sport.empty())
						return; //need to have a serial port
				}
			}
			else if (
				(htype == HTYPE_RFXLAN) || (htype == HTYPE_P1SmartMeterLAN) ||
				(htype == HTYPE_YouLess) || (htype == HTYPE_OpenThermGatewayTCP) || (htype == HTYPE_LimitlessLights) ||
				(htype == HTYPE_SolarEdgeTCP) || (htype == HTYPE_WOL) || (htype == HTYPE_S0SmartMeterTCP) || (htype == HTYPE_ECODEVICES) || (htype == HTYPE_Mochad) ||
				(htype == HTYPE_MySensorsTCP) || (htype == HTYPE_MySensorsMQTT) || (htype == HTYPE_MQTT) || (htype == HTYPE_TTN_MQTT) || (htype == HTYPE_FRITZBOX) || (htype == HTYPE_ETH8020) || (htype == HTYPE_Sterbox) ||
				(htype == HTYPE_KMTronicTCP) || (htype == HTYPE_KMTronicUDP) || (htype == HTYPE_SOLARMAXTCP) || (htype == HTYPE_RelayNet) || (htype == HTYPE_SatelIntegra) || (htype == HTYPE_eHouseTCP) || (htype == HTYPE_RFLINKTCP) ||
				(htype == HTYPE_Comm5TCP || (htype == HTYPE_Comm5SMTCP) || (htype == HTYPE_CurrentCostMeterLAN)) ||
				(htype == HTYPE_NefitEastLAN) || (htype == HTYPE_DenkoviHTTPDevices) || (htype == HTYPE_DenkoviTCPDevices) || (htype == HTYPE_Ec3kMeterTCP) || (htype == HTYPE_MultiFun) || (htype == HTYPE_ZIBLUETCP) || (htype == HTYPE_OnkyoAVTCP)
				) {
				//Lan
				if (address.empty())
					return;
			}
			else if (htype == HTYPE_DomoticzInternal) {
				// DomoticzInternal cannot be updated
				return;
			}
			else if (htype == HTYPE_Domoticz) {
				//Remote Domoticz
				if (address.empty())
					return;
			}
			else if (htype == HTYPE_System) {
				//There should be only one, and with this ID
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT ID FROM Hardware WHERE (Type==%d)", HTYPE_System);
				if (!result.empty())
				{
					int hID = atoi(result[0][0].c_str());
					int aID = atoi(idx.c_str());
					if (hID != aID)
						return;
				}
			}
			else if (htype == HTYPE_TE923) {
				//All fine here
			}
			else if (htype == HTYPE_VOLCRAFTCO20) {
				//All fine here
			}
			else if (htype == HTYPE_1WIRE) {
				//All fine here
			}
			else if (htype == HTYPE_Pinger) {
				//All fine here
			}
			else if (htype == HTYPE_Kodi) {
				//All fine here
			}
			else if (htype == HTYPE_PanasonicTV) {
				//All fine here
			}
			else if (htype == HTYPE_LogitechMediaServer) {
				//All fine here
			}
			else if (htype == HTYPE_RaspberryBMP085) {
				//All fine here
			}
			else if (htype == HTYPE_RaspberryHTU21D) {
				//All fine here
			}
			else if (htype == HTYPE_RaspberryTSL2561) {
				//All fine here
			}
			else if (htype == HTYPE_RaspberryBME280) {
				//All fine here
			}
			else if (htype == HTYPE_RaspberryMCP23017) {
				//all fine here!
			}
			else if (htype == HTYPE_Dummy) {
				//All fine here
			}
			else if (htype == HTYPE_EVOHOME_SCRIPT || htype == HTYPE_EVOHOME_SERIAL || htype == HTYPE_EVOHOME_WEB || htype == HTYPE_EVOHOME_TCP) {
				//All fine here
			}
			else if (htype == HTYPE_PiFace) {
				//All fine here
			}
			else if (htype == HTYPE_HTTPPOLLER) {
				//all fine here!
			}
			else if (htype == HTYPE_BleBox) {
				//All fine here
			}
			else if (htype == HTYPE_HEOS) {
				//All fine here
			}
			else if (htype == HTYPE_Yeelight) {
				//All fine here
			}
			else if (htype == HTYPE_XiaomiGateway) {
				//All fine here
			}
			else if (htype == HTYPE_Arilux) {
				//All fine here
			}
			else if (htype == HTYPE_USBtinGateway) {
				//All fine here
			}
			else if (htype == HTYPE_BuienRadar) {
				//All fine here
			}
			else if (
				(htype == HTYPE_Wunderground) ||
				(htype == HTYPE_DarkSky) ||
				(htype == HTYPE_AccuWeather) ||
				(htype == HTYPE_OpenWeatherMap) ||
				(htype == HTYPE_ICYTHERMOSTAT) ||
				(htype == HTYPE_TOONTHERMOSTAT) ||
				(htype == HTYPE_AtagOne) ||
				(htype == HTYPE_PVOUTPUT_INPUT) ||
				(htype == HTYPE_NEST) ||
				(htype == HTYPE_ANNATHERMOSTAT) ||
				(htype == HTYPE_THERMOSMART) ||
				(htype == HTYPE_Tado) ||
				(htype == HTYPE_Netatmo)
				)
			{
				if (
					(username.empty()) ||
					(password.empty())
					)
					return;
			}
			else if (htype == HTYPE_SolarEdgeAPI)
			{
				if (
					(username.empty())
					)
					return;
			}
			else if (htype == HTYPE_Nest_OAuthAPI) {
				if (
					(username == "") &&
					(extra == "||")
					)
					return;
			}
			else if (htype == HTYPE_HARMONY_HUB) {
				if (
					(address.empty())
					)
					return;
			}
			else if (htype == HTYPE_Philips_Hue) {
				if (
					(username.empty()) ||
					(address.empty())
					)
					return;
				if (port == 0)
					port = 80;
			}
			else if (htype == HTYPE_RaspberryGPIO) {
				//all fine here!
			}
			else if (htype == HTYPE_SysfsGpio) {
				//all fine here!
			}
			else if (htype == HTYPE_Rtl433) {
				//all fine here!
			}
			else if (htype == HTYPE_Daikin) {
				//all fine here!
			}
			else if (htype == HTYPE_SBFSpot) {
				if (username.empty())
					return;
			}
			else if (htype == HTYPE_WINDDELEN) {
				std::string mill_id = request::findValue(&req, "Mode1");
				if (
					(mill_id.empty()) ||
					(sport.empty())
					)
					return;
			}
			else if (htype == HTYPE_Honeywell) {
				//All fine here
			}
			else if (htype == HTYPE_OpenWebNetTCP) {
				//All fine here
			}
			else if (htype == HTYPE_PythonPlugin) {
				//All fine here
			}
			else if (htype == HTYPE_GoodweAPI) {
				if (username.empty()) {
					return;
				}
			}
			else if (htype == HTYPE_RaspberryPCF8574) {
				//All fine here
			}
			else if (htype == HTYPE_OpenWebNetUSB) {
				//All fine here
			}
			else if (htype == HTYPE_IntergasInComfortLAN2RF) {
				//All fine here
			}
			else if (htype == HTYPE_EnphaseAPI) {
				//all fine here!
			}
			else
				return;

			std::string mode1Str;
			std::string mode2Str;
			std::string mode3Str;
			std::string mode4Str;
			std::string mode5Str;
			std::string mode6Str;

			root["status"] = "OK";
			root["title"] = "UpdateHardware";

			if (htype == HTYPE_Domoticz)
			{
				if (password.size() != 32)
				{
					password = GenerateMD5Hash(password);
				}
			}

			if ((bIsSerial) && (!bEnabled) && (sport.empty()))
			{
				//just disable the device
				m_sql.safe_query(
					"UPDATE Hardware SET Enabled=%d WHERE (ID == '%q')",
					(bEnabled == true) ? 1 : 0,
					idx.c_str()
				);
			}
			else
			{
				if (htype == HTYPE_HTTPPOLLER) {
					m_sql.safe_query(
						"UPDATE Hardware SET Name='%q', Enabled=%d, Type=%d, Address='%q', Port=%d, SerialPort='%q', Username='%q', Password='%q', Extra='%q', DataTimeout=%d WHERE (ID == '%q')",
						name.c_str(),
						(senabled == "true") ? 1 : 0,
						htype,
						address.c_str(),
						port,
						sport.c_str(),
						username.c_str(),
						password.c_str(),
						extra.c_str(),
						iDataTimeout,
						idx.c_str()
					);
				}
				else if (htype == HTYPE_PythonPlugin) {
					mode1Str = request::findValue(&req, "Mode1");
					mode2Str = request::findValue(&req, "Mode2");
					mode3Str = request::findValue(&req, "Mode3");
					mode4Str = request::findValue(&req, "Mode4");
					mode5Str = request::findValue(&req, "Mode5");
					mode6Str = request::findValue(&req, "Mode6");
					sport = request::findValue(&req, "serialport");
					m_sql.safe_query(
						"UPDATE Hardware SET Name='%q', Enabled=%d, Type=%d, Address='%q', Port=%d, SerialPort='%q', Username='%q', Password='%q', Extra='%q', Mode1='%q', Mode2='%q', Mode3='%q', Mode4='%q', Mode5='%q', Mode6='%q', DataTimeout=%d WHERE (ID == '%q')",
						name.c_str(),
						(senabled == "true") ? 1 : 0,
						htype,
						address.c_str(),
						port,
						sport.c_str(),
						username.c_str(),
						password.c_str(),
						extra.c_str(),
						mode1Str.c_str(), mode2Str.c_str(), mode3Str.c_str(), mode4Str.c_str(), mode5Str.c_str(), mode6Str.c_str(),
						iDataTimeout,
						idx.c_str()
					);
				}
				else if (
					(htype == HTYPE_RFXtrx433) ||
					(htype == HTYPE_RFXtrx868)
					)
				{
					//No Extra field here, handled in CWebServer::SetRFXCOMMode
					m_sql.safe_query(
						"UPDATE Hardware SET Name='%q', Enabled=%d, Type=%d, Address='%q', Port=%d, SerialPort='%q', Username='%q', Password='%q', Mode1=%d, Mode2=%d, Mode3=%d, Mode4=%d, Mode5=%d, Mode6=%d, DataTimeout=%d WHERE (ID == '%q')",
						name.c_str(),
						(bEnabled == true) ? 1 : 0,
						htype,
						address.c_str(),
						port,
						sport.c_str(),
						username.c_str(),
						password.c_str(),
						mode1, mode2, mode3, mode4, mode5, mode6,
						iDataTimeout,
						idx.c_str()
					);
					std::vector<std::vector<std::string> > result;
					result = m_sql.safe_query("SELECT Extra FROM Hardware WHERE ID=%q", idx.c_str());
					if (!result.empty())
						extra = result[0][0];
				}
				else {
					m_sql.safe_query(
						"UPDATE Hardware SET Name='%q', Enabled=%d, Type=%d, Address='%q', Port=%d, SerialPort='%q', Username='%q', Password='%q', Extra='%q', Mode1=%d, Mode2=%d, Mode3=%d, Mode4=%d, Mode5=%d, Mode6=%d, DataTimeout=%d WHERE (ID == '%q')",
						name.c_str(),
						(bEnabled == true) ? 1 : 0,
						htype,
						address.c_str(),
						port,
						sport.c_str(),
						username.c_str(),
						password.c_str(),
						extra.c_str(),
						mode1, mode2, mode3, mode4, mode5, mode6,
						iDataTimeout,
						idx.c_str()
					);
				}
			}

			//re-add the device in our system
			int ID = atoi(idx.c_str());
			m_mainworker.AddHardwareFromParams(ID, name, bEnabled, htype, address, port, sport, username, password, extra, mode1, mode2, mode3, mode4, mode5, mode6, iDataTimeout, true);
		}

		void CWebServer::Cmd_GetDeviceValueOptions(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			std::vector<std::string> result;
			result = CBasePush::DropdownOptions(atoi(idx.c_str()));
			if ((result.size() == 1) && result[0] == "Status") {
				root["result"][0]["Value"] = 0;
				root["result"][0]["Wording"] = result[0];
			}
			else {
				int ii = 0;
				for (const auto & itt : result)
				{
					std::string ddOption = itt;
					root["result"][ii]["Value"] = ii + 1;
					root["result"][ii]["Wording"] = ddOption.c_str();
					ii++;
				}

			}
			root["status"] = "OK";
			root["title"] = "GetDeviceValueOptions";
		}

		void CWebServer::Cmd_GetDeviceValueOptionWording(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
			std::string idx = request::findValue(&req, "idx");
			std::string pos = request::findValue(&req, "pos");
			if ((idx.empty()) || (pos.empty()))
				return;
			std::string wording;
			wording = CBasePush::DropdownOptionsValue(atoi(idx.c_str()), atoi(pos.c_str()));
			root["wording"] = wording;
			root["status"] = "OK";
			root["title"] = "GetDeviceValueOptions";
		}

		void CWebServer::Cmd_AllowNewHardware(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
			std::string sTimeout = request::findValue(&req, "timeout");
			if (sTimeout.empty())
				return;
			root["status"] = "OK";
			root["title"] = "AllowNewHardware";

			m_sql.AllowNewHardwareTimer(atoi(sTimeout.c_str()));
		}


		void CWebServer::Cmd_DeleteHardware(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			int hwID = atoi(idx.c_str());

			CDomoticzHardwareBase *pBaseHardware = m_mainworker.GetHardware(hwID);
			if ((pBaseHardware != NULL) && (pBaseHardware->HwdType == HTYPE_DomoticzInternal)) {
				// DomoticzInternal cannot be removed
				return;
			}

			root["status"] = "OK";
			root["title"] = "DeleteHardware";

			m_mainworker.RemoveDomoticzHardware(hwID);
			m_sql.DeleteHardware(idx);
		}

		void CWebServer::Cmd_GetLog(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetLog";

			time_t lastlogtime = 0;
			std::string slastlogtime = request::findValue(&req, "lastlogtime");
			if (slastlogtime != "")
			{
				std::stringstream s_str(slastlogtime);
				s_str >> lastlogtime;
			}

			_eLogLevel lLevel = LOG_NORM;
			std::string sloglevel = request::findValue(&req, "loglevel");
			if (!sloglevel.empty())
			{
				lLevel = (_eLogLevel)atoi(sloglevel.c_str());
			}

			std::list<CLogger::_tLogLineStruct> logmessages = _log.GetLog(lLevel);
			int ii = 0;
			for (const auto & itt : logmessages)
			{
				if (itt.logtime > lastlogtime)
				{
					std::stringstream szLogTime;
					szLogTime << itt.logtime;
					root["LastLogTime"] = szLogTime.str();
					root["result"][ii]["level"] = static_cast<int>(itt.level);
					root["result"][ii]["message"] = itt.logmessage;
					ii++;
				}
			}
		}

		void CWebServer::Cmd_ClearLog(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "ClearLog";
			_log.ClearLog();
		}

		//Plan Functions
		void CWebServer::Cmd_AddPlan(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string name = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));
			if (name.empty())
			{
				session.reply_status = reply::bad_request;
			}

			root["status"] = "OK";
			root["title"] = "AddPlan";
			m_sql.safe_query(
				"INSERT INTO Plans (Name) VALUES ('%q')",
				name.c_str()
			);
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT MAX(ID) FROM Plans");
			if (!result.empty())
			{
				std::vector<std::string> sd = result[0];
				int ID = atoi(sd[0].c_str());

				root["idx"] = sd[0].c_str(); // OTO output the created ID for easier management on the caller side (if automated)
			}
		}

		void CWebServer::Cmd_UpdatePlan(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			std::string name = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));
			if (name.empty())
			{
				session.reply_status = reply::bad_request;
				return;
			}

			root["status"] = "OK";
			root["title"] = "UpdatePlan";

			m_sql.safe_query(
				"UPDATE Plans SET Name='%q' WHERE (ID == '%q')",
				name.c_str(),
				idx.c_str()
			);
		}

		void CWebServer::Cmd_DeletePlan(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			root["status"] = "OK";
			root["title"] = "DeletePlan";
			m_sql.safe_query(
				"DELETE FROM DeviceToPlansMap WHERE (PlanID == '%q')",
				idx.c_str()
			);
			m_sql.safe_query(
				"DELETE FROM Plans WHERE (ID == '%q')",
				idx.c_str()
			);
		}

		void CWebServer::Cmd_GetUnusedPlanDevices(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetUnusedPlanDevices";
			std::string sunique = request::findValue(&req, "unique");
			if (sunique.empty())
				return;
			int iUnique = (sunique == "true") ? 1 : 0;
			int ii = 0;

			std::vector<std::vector<std::string> > result;
			std::vector<std::vector<std::string> > result2;
			result = m_sql.safe_query("SELECT T1.[ID], T1.[Name], T1.[Type], T1.[SubType], T2.[Name] AS HardwareName FROM DeviceStatus as T1, Hardware as T2 WHERE (T1.[Used]==1) AND (T2.[ID]==T1.[HardwareID]) ORDER BY T2.[Name], T1.[Name]");
			if (!result.empty())
			{
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					bool bDoAdd = true;
					if (iUnique)
					{
						result2 = m_sql.safe_query("SELECT ID FROM DeviceToPlansMap WHERE (DeviceRowID=='%q') AND (DevSceneType==0)",
							sd[0].c_str());
						bDoAdd = (result2.size() == 0);
					}
					if (bDoAdd)
					{
						int _dtype = atoi(sd[2].c_str());
						std::string Name = "[" + sd[4] + "] " + sd[1] + " (" + RFX_Type_Desc(_dtype, 1) + "/" + RFX_Type_SubType_Desc(_dtype, atoi(sd[3].c_str())) + ")";
						root["result"][ii]["type"] = 0;
						root["result"][ii]["idx"] = sd[0];
						root["result"][ii]["Name"] = Name;
						ii++;
					}
				}
			}
			//Add Scenes
			result = m_sql.safe_query("SELECT ID, Name FROM Scenes ORDER BY Name");
			if (!result.empty())
			{
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					bool bDoAdd = true;
					if (iUnique)
					{
						result2 = m_sql.safe_query("SELECT ID FROM DeviceToPlansMap WHERE (DeviceRowID=='%q') AND (DevSceneType==1)",
							sd[0].c_str());
						bDoAdd = (result2.size() == 0);
					}
					if (bDoAdd)
					{
						root["result"][ii]["type"] = 1;
						root["result"][ii]["idx"] = sd[0];
						std::string sname = "[Scene] " + sd[1];
						root["result"][ii]["Name"] = sname;
						ii++;
					}
				}
			}
		}

		void CWebServer::Cmd_AddPlanActiveDevice(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string idx = request::findValue(&req, "idx");
			std::string sactivetype = request::findValue(&req, "activetype");
			std::string activeidx = request::findValue(&req, "activeidx");
			if (
				(idx.empty()) ||
				(sactivetype.empty()) ||
				(activeidx.empty())
				)
				return;
			root["status"] = "OK";
			root["title"] = "AddPlanActiveDevice";

			int activetype = atoi(sactivetype.c_str());

			//check if it is not already there
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT ID FROM DeviceToPlansMap WHERE (DeviceRowID=='%q') AND (DevSceneType==%d) AND (PlanID=='%q')",
				activeidx.c_str(), activetype, idx.c_str());
			if (result.empty())
			{
				m_sql.safe_query(
					"INSERT INTO DeviceToPlansMap (DevSceneType,DeviceRowID, PlanID) VALUES (%d,'%q','%q')",
					activetype,
					activeidx.c_str(),
					idx.c_str()
				);
			}
		}

		void CWebServer::Cmd_GetPlanDevices(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			root["status"] = "OK";
			root["title"] = "GetPlanDevices";

			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT ID, DevSceneType, DeviceRowID, [Order] FROM DeviceToPlansMap WHERE (PlanID=='%q') ORDER BY [Order]",
				idx.c_str());
			if (!result.empty())
			{
				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					std::string ID = sd[0];
					int DevSceneType = atoi(sd[1].c_str());
					std::string DevSceneRowID = sd[2];

					std::string Name = "";
					if (DevSceneType == 0)
					{
						std::vector<std::vector<std::string> > result2;
						result2 = m_sql.safe_query("SELECT Name FROM DeviceStatus WHERE (ID=='%q')",
							DevSceneRowID.c_str());
						if (!result2.empty())
						{
							Name = result2[0][0];
						}
					}
					else
					{
						std::vector<std::vector<std::string> > result2;
						result2 = m_sql.safe_query("SELECT Name FROM Scenes WHERE (ID=='%q')",
							DevSceneRowID.c_str());
						if (!result2.empty())
						{
							Name = "[Scene] " + result2[0][0];
						}
					}
					if (Name != "")
					{
						root["result"][ii]["idx"] = ID;
						root["result"][ii]["devidx"] = DevSceneRowID;
						root["result"][ii]["type"] = DevSceneType;
						root["result"][ii]["DevSceneRowID"] = DevSceneRowID;
						root["result"][ii]["order"] = sd[3];
						root["result"][ii]["Name"] = Name;
						ii++;
					}
				}
			}
		}

		void CWebServer::Cmd_DeletePlanDevice(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			root["status"] = "OK";
			root["title"] = "DeletePlanDevice";
			m_sql.safe_query("DELETE FROM DeviceToPlansMap WHERE (ID == '%q')", idx.c_str());
		}

		void CWebServer::Cmd_SetPlanDeviceCoords(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string idx = request::findValue(&req, "idx");
			std::string planidx = request::findValue(&req, "planidx");
			std::string xoffset = request::findValue(&req, "xoffset");
			std::string yoffset = request::findValue(&req, "yoffset");
			std::string type = request::findValue(&req, "DevSceneType");
			if ((idx.empty()) || (planidx.empty()) || (xoffset.empty()) || (yoffset.empty()))
				return;
			if (type != "1") type = "0";  // 0 = Device, 1 = Scene/Group
			root["status"] = "OK";
			root["title"] = "SetPlanDeviceCoords";
			m_sql.safe_query("UPDATE DeviceToPlansMap SET [XOffset] = '%q', [YOffset] = '%q' WHERE (DeviceRowID='%q') and (PlanID='%q') and (DevSceneType='%q')",
				xoffset.c_str(), yoffset.c_str(), idx.c_str(), planidx.c_str(), type.c_str());
			_log.Log(LOG_STATUS, "(Floorplan) Device '%s' coordinates set to '%s,%s' in plan '%s'.", idx.c_str(), xoffset.c_str(), yoffset.c_str(), planidx.c_str());
		}

		void CWebServer::Cmd_DeleteAllPlanDevices(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			root["status"] = "OK";
			root["title"] = "DeleteAllPlanDevices";
			m_sql.safe_query("DELETE FROM DeviceToPlansMap WHERE (PlanID == '%q')", idx.c_str());
		}

		void CWebServer::Cmd_ChangePlanOrder(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			std::string sway = request::findValue(&req, "way");
			if (sway.empty())
				return;
			bool bGoUp = (sway == "0");

			std::string aOrder, oID, oOrder;

			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT [Order] FROM Plans WHERE (ID=='%q')",
				idx.c_str());
			if (result.empty())
				return;
			aOrder = result[0][0];

			if (!bGoUp)
			{
				//Get next device order
				result = m_sql.safe_query("SELECT ID, [Order] FROM Plans WHERE ([Order]>'%q') ORDER BY [Order] ASC",
					aOrder.c_str());
				if (result.empty())
					return;
				oID = result[0][0];
				oOrder = result[0][1];
			}
			else
			{
				//Get previous device order
				result = m_sql.safe_query("SELECT ID, [Order] FROM Plans WHERE ([Order]<'%q') ORDER BY [Order] DESC",
					aOrder.c_str());
				if (result.empty())
					return;
				oID = result[0][0];
				oOrder = result[0][1];
			}
			//Swap them
			root["status"] = "OK";
			root["title"] = "ChangePlanOrder";

			m_sql.safe_query("UPDATE Plans SET [Order] = '%q' WHERE (ID='%q')",
				oOrder.c_str(), idx.c_str());
			m_sql.safe_query("UPDATE Plans SET [Order] = '%q' WHERE (ID='%q')",
				aOrder.c_str(), oID.c_str());
		}

		void CWebServer::Cmd_ChangePlanDeviceOrder(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string planid = request::findValue(&req, "planid");
			std::string idx = request::findValue(&req, "idx");
			std::string sway = request::findValue(&req, "way");
			if (
				(planid.empty()) ||
				(idx.empty()) ||
				(sway.empty())
				)
				return;
			bool bGoUp = (sway == "0");

			std::string aOrder, oID, oOrder;

			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT [Order] FROM DeviceToPlansMap WHERE ((ID=='%q') AND (PlanID=='%q'))",
				idx.c_str(), planid.c_str());
			if (result.empty())
				return;
			aOrder = result[0][0];

			if (!bGoUp)
			{
				//Get next device order
				result = m_sql.safe_query("SELECT ID, [Order] FROM DeviceToPlansMap WHERE (([Order]>'%q') AND (PlanID=='%q')) ORDER BY [Order] ASC",
					aOrder.c_str(), planid.c_str());
				if (result.empty())
					return;
				oID = result[0][0];
				oOrder = result[0][1];
			}
			else
			{
				//Get previous device order
				result = m_sql.safe_query("SELECT ID, [Order] FROM DeviceToPlansMap WHERE (([Order]<'%q') AND (PlanID=='%q')) ORDER BY [Order] DESC",
					aOrder.c_str(), planid.c_str());
				if (result.empty())
					return;
				oID = result[0][0];
				oOrder = result[0][1];
			}
			//Swap them
			root["status"] = "OK";
			root["title"] = "ChangePlanOrder";

			m_sql.safe_query("UPDATE DeviceToPlansMap SET [Order] = '%q' WHERE (ID='%q')",
				oOrder.c_str(), idx.c_str());
			m_sql.safe_query("UPDATE DeviceToPlansMap SET [Order] = '%q' WHERE (ID='%q')",
				aOrder.c_str(), oID.c_str());
		}

		void CWebServer::Cmd_GetVersion(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetVersion";
			root["version"] = szAppVersion;
			root["hash"] = szAppHash;
			root["build_time"] = szAppDate;
			root["python_version"] = szPyVersion;

			if (session.rights != 2)
			{
				//only admin users will receive the update notification
				root["UseUpdate"] = false;
				root["HaveUpdate"] = false;
			}
			else
			{
				root["UseUpdate"] = g_bUseUpdater;
				root["HaveUpdate"] = m_mainworker.IsUpdateAvailable(false);
				root["DomoticzUpdateURL"] = m_mainworker.m_szDomoticzUpdateURL;
				root["SystemName"] = m_mainworker.m_szSystemName;
				root["Revision"] = m_mainworker.m_iRevision;
			}
		}

		void CWebServer::Cmd_GetAuth(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetAuth";
			if (session.rights != -1)
			{
				root["version"] = szAppVersion;
			}
			root["user"] = session.username;
			root["rights"] = session.rights;
		}

		void CWebServer::Cmd_GetUptime(WebEmSession & session, const request& req, Json::Value &root)
		{
			//this is used in the about page, we are going to round the seconds a bit to display nicer
			time_t atime = mytime(NULL);
			time_t tuptime = atime - m_StartTime;
			//round to 5 seconds (nicer in about page)
			tuptime = ((tuptime / 5) * 5) + 5;
			int days, hours, minutes, seconds;
			days = (int)(tuptime / 86400);
			tuptime -= (days * 86400);
			hours = (int)(tuptime / 3600);
			tuptime -= (hours * 3600);
			minutes = (int)(tuptime / 60);
			tuptime -= (minutes * 60);
			seconds = (int)tuptime;
			root["status"] = "OK";
			root["title"] = "GetUptime";
			root["days"] = days;
			root["hours"] = hours;
			root["minutes"] = minutes;
			root["seconds"] = seconds;
		}

		void CWebServer::Cmd_GetActualHistory(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetActualHistory";

			std::string historyfile = szUserDataFolder + "History.txt";

			std::ifstream infile;
			int ii = 0;
			infile.open(historyfile.c_str());
			std::string sLine;
			if (infile.is_open())
			{
				while (!infile.eof())
				{
					getline(infile, sLine);
					root["LastLogTime"] = "";
					if (sLine.find("Version ") == 0)
						root["result"][ii]["level"] = 1;
					else
						root["result"][ii]["level"] = 0;
					root["result"][ii]["message"] = sLine;
					ii++;
				}
			}
		}

		void CWebServer::Cmd_GetNewHistory(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetNewHistory";

			std::string historyfile;
			int nValue;
			m_sql.GetPreferencesVar("ReleaseChannel", nValue);
			bool bIsBetaChannel = (nValue != 0);

			std::string szHistoryURL = "https://www.domoticz.com/download.php?channel=stable&type=history";
			if (bIsBetaChannel)
			{
				utsname my_uname;
				if (uname(&my_uname) < 0)
					return;

				std::string systemname = my_uname.sysname;
				std::string machine = my_uname.machine;
				std::transform(systemname.begin(), systemname.end(), systemname.begin(), ::tolower);

				if (machine == "armv6l")
				{
					//Seems like old arm systems can also use the new arm build
					machine = "armv7l";
				}

				if (((machine != "armv6l") && (machine != "armv7l") && (systemname != "windows") && (machine != "x86_64") && (machine != "aarch64")) || (strstr(my_uname.release, "ARCH+") != NULL))
					szHistoryURL = "https://www.domoticz.com/download.php?channel=beta&type=history";
				else
					szHistoryURL = "https://www.domoticz.com/download.php?channel=beta&type=history&system=" + systemname + "&machine=" + machine;
			}
			if (!HTTPClient::GET(szHistoryURL, historyfile))
			{
				historyfile = "Unable to get Online History document !!";
			}

			std::istringstream stream(historyfile);
			std::string sLine;
			int ii = 0;
			while (std::getline(stream, sLine))
			{
				root["LastLogTime"] = "";
				if (sLine.find("Version ") == 0)
					root["result"][ii]["level"] = 1;
				else
					root["result"][ii]["level"] = 0;
				root["result"][ii]["message"] = sLine;
				ii++;
			}
		}

		void CWebServer::Cmd_GetConfig(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights == -1)
			{
				session.reply_status = reply::forbidden;
				return;//Only auth user allowed
			}

			root["status"] = "OK";
			root["title"] = "GetConfig";

			bool bHaveUser = (session.username != "");
			int urights = 3;
			unsigned long UserID = 0;
			if (bHaveUser)
			{
				int iUser = FindUser(session.username.c_str());
				if (iUser != -1)
				{
					urights = static_cast<int>(m_users[iUser].userrights);
					UserID = m_users[iUser].ID;
				}

			}

			int nValue;
			std::string sValue;

			if (m_sql.GetPreferencesVar("Language", sValue))
			{
				root["language"] = sValue;
			}
			if (m_sql.GetPreferencesVar("DegreeDaysBaseTemperature", sValue))
			{
				root["DegreeDaysBaseTemperature"] = atof(sValue.c_str());
			}

			nValue = 0;
			int iDashboardType = 0;
			m_sql.GetPreferencesVar("DashboardType", iDashboardType);
			root["DashboardType"] = iDashboardType;
			m_sql.GetPreferencesVar("MobileType", nValue);
			root["MobileType"] = nValue;

			nValue = 1;
			m_sql.GetPreferencesVar("5MinuteHistoryDays", nValue);
			root["FiveMinuteHistoryDays"] = nValue;

			nValue = 1;
			m_sql.GetPreferencesVar("ShowUpdateEffect", nValue);
			root["result"]["ShowUpdatedEffect"] = (nValue == 1);

			root["AllowWidgetOrdering"] = m_sql.m_bAllowWidgetOrdering;

			std::string Latitude = "1";
			std::string Longitude = "1";
			if (m_sql.GetPreferencesVar("Location", nValue, sValue))
			{
				std::vector<std::string> strarray;
				StringSplit(sValue, ";", strarray);

				if (strarray.size() == 2)
				{
					Latitude = strarray[0];
					Longitude = strarray[1];
				}
			}
			root["Latitude"] = Latitude;
			root["Longitude"] = Longitude;

#ifndef NOCLOUD
			bool bEnableTabProxy = request::get_req_header(&req, "X-From-MyDomoticz") != NULL;
#else
			bool bEnableTabProxy = false;
#endif
			int bEnableTabDashboard = 1;
			int bEnableTabFloorplans = 1;
			int bEnableTabLight = 1;
			int bEnableTabScenes = 1;
			int bEnableTabTemp = 1;
			int bEnableTabWeather = 1;
			int bEnableTabUtility = 1;
			int bEnableTabCustom = 1;

			std::vector<std::vector<std::string> > result;

			if ((UserID != 0) && (UserID != 10000))
			{
				result = m_sql.safe_query("SELECT TabsEnabled FROM Users WHERE (ID==%lu)",
					UserID);
				if (!result.empty())
				{
					int TabsEnabled = atoi(result[0][0].c_str());
					bEnableTabLight = (TabsEnabled&(1 << 0));
					bEnableTabScenes = (TabsEnabled&(1 << 1));
					bEnableTabTemp = (TabsEnabled&(1 << 2));
					bEnableTabWeather = (TabsEnabled&(1 << 3));
					bEnableTabUtility = (TabsEnabled&(1 << 4));
					bEnableTabCustom = (TabsEnabled&(1 << 5));
					bEnableTabFloorplans = (TabsEnabled&(1 << 6));
				}
			}
			else
			{
				m_sql.GetPreferencesVar("EnableTabFloorplans", bEnableTabFloorplans);
				m_sql.GetPreferencesVar("EnableTabLights", bEnableTabLight);
				m_sql.GetPreferencesVar("EnableTabScenes", bEnableTabScenes);
				m_sql.GetPreferencesVar("EnableTabTemp", bEnableTabTemp);
				m_sql.GetPreferencesVar("EnableTabWeather", bEnableTabWeather);
				m_sql.GetPreferencesVar("EnableTabUtility", bEnableTabUtility);
				m_sql.GetPreferencesVar("EnableTabCustom", bEnableTabCustom);
			}
			if (iDashboardType == 3)
			{
				//Floorplan , no need to show a tab floorplan
				bEnableTabFloorplans = 0;
			}
			root["result"]["EnableTabProxy"] = bEnableTabProxy;
			root["result"]["EnableTabDashboard"] = bEnableTabDashboard != 0;
			root["result"]["EnableTabFloorplans"] = bEnableTabFloorplans != 0;
			root["result"]["EnableTabLights"] = bEnableTabLight != 0;
			root["result"]["EnableTabScenes"] = bEnableTabScenes != 0;
			root["result"]["EnableTabTemp"] = bEnableTabTemp != 0;
			root["result"]["EnableTabWeather"] = bEnableTabWeather != 0;
			root["result"]["EnableTabUtility"] = bEnableTabUtility != 0;
			root["result"]["EnableTabCustom"] = bEnableTabCustom != 0;

			if (bEnableTabCustom)
			{
				//Add custom templates
				DIR *lDir;
				struct dirent *ent;
				std::string templatesFolder = szWWWFolder + "/templates";
				int iFile = 0;
				if ((lDir = opendir(templatesFolder.c_str())) != NULL)
				{
					while ((ent = readdir(lDir)) != NULL)
					{
						std::string filename = ent->d_name;
						size_t pos = filename.find(".htm");
						if (pos != std::string::npos)
						{
							std::string shortfile = filename.substr(0, pos);
							root["result"]["templates"][iFile++] = shortfile;
						}
					}
					closedir(lDir);
				}
			}
		}

		void CWebServer::Cmd_SendNotification(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string subject = request::findValue(&req, "subject");
			std::string body = request::findValue(&req, "body");
			std::string subsystem = request::findValue(&req, "subsystem");
			if (
				(subject.empty()) ||
				(body.empty())
				)
				return;
			if (subsystem.empty()) subsystem = NOTIFYALL;
			//Add to queue
			if (m_notifications.SendMessage(0, std::string(""), subsystem, subject, body, std::string(""), 1, std::string(""), false)) {
				root["status"] = "OK";
			}
			root["title"] = "SendNotification";
		}

		void CWebServer::Cmd_UpdateDevice(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights < 1)
			{
				session.reply_status = reply::forbidden;
				return; //only user or higher allowed
			}

			std::string idx = request::findValue(&req, "idx");

			if (!IsIdxForUser(&session, atoi(idx.c_str())))
			{
				_log.Log(LOG_ERROR, "User: %s tried to update an Unauthorized device!", session.username.c_str());
				session.reply_status = reply::forbidden;
				return;
			}

			std::string hid = request::findValue(&req, "hid");
			std::string did = request::findValue(&req, "did");
			std::string dunit = request::findValue(&req, "dunit");
			std::string dtype = request::findValue(&req, "dtype");
			std::string dsubtype = request::findValue(&req, "dsubtype");

			std::string nvalue = request::findValue(&req, "nvalue");
			std::string svalue = request::findValue(&req, "svalue");

			if ((nvalue.empty() && svalue.empty()))
			{
				return;
			}

			int signallevel = 12;
			int batterylevel = 255;

			if (idx.empty())
			{
				//No index supplied, check if raw parameters where supplied
				if (
					(hid.empty()) ||
					(did.empty()) ||
					(dunit.empty()) ||
					(dtype.empty()) ||
					(dsubtype.empty())
					)
					return;
			}
			else
			{
				//Get the raw device parameters
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT HardwareID, DeviceID, Unit, Type, SubType FROM DeviceStatus WHERE (ID=='%q')",
					idx.c_str());
				if (result.empty())
					return;
				hid = result[0][0];
				did = result[0][1];
				dunit = result[0][2];
				dtype = result[0][3];
				dsubtype = result[0][4];
			}

			int HardwareID = atoi(hid.c_str());
			std::string DeviceID = did;
			int unit = atoi(dunit.c_str());
			int devType = atoi(dtype.c_str());
			int subType = atoi(dsubtype.c_str());

			uint64_t ulIdx = std::strtoull(idx.c_str(), nullptr, 10);

			int invalue = atoi(nvalue.c_str());

			std::string sSignalLevel = request::findValue(&req, "rssi");
			if (sSignalLevel != "")
			{
				signallevel = atoi(sSignalLevel.c_str());
			}
			std::string sBatteryLevel = request::findValue(&req, "battery");
			if (sBatteryLevel != "")
			{
				batterylevel = atoi(sBatteryLevel.c_str());
			}
			if (m_mainworker.UpdateDevice(HardwareID, DeviceID, unit, devType, subType, invalue, svalue, signallevel, batterylevel))
			{
				root["status"] = "OK";
				root["title"] = "Update Device";
			}
		}

		void CWebServer::Cmd_UpdateDevices(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string script = request::findValue(&req, "script");
			if (script.empty())
			{
				return;
			}
			std::string content = req.content;

			std::vector<std::string> allParameters;

			// Keep the url content on the right of the '?'
			std::vector<std::string> allParts;
			StringSplit(req.uri, "?", allParts);
			if (!allParts.empty())
			{
				// Split all url parts separated by a '&'
				StringSplit(allParts[1], "&", allParameters);
			}

			CLuaHandler luaScript;
			bool ret = luaScript.executeLuaScript(script, content, allParameters);
			if (ret)
			{
				root["status"] = "OK";
				root["title"] = "Update Device";
			}
		}

		void CWebServer::Cmd_SystemShutdown(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
#ifdef WIN32
			int ret = system("shutdown -s -f -t 1 -d up:125:1");
#else
			int ret = system("sudo shutdown -h now");
#endif
			if (ret != 0)
			{
				_log.Log(LOG_ERROR, "Error executing shutdown command. returned: %d", ret);
				return;
			}
			root["title"] = "SystemShutdown";
			root["status"] = "OK";
		}

		void CWebServer::Cmd_SystemReboot(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
#ifdef WIN32
			int ret = system("shutdown -r -f -t 1 -d up:125:1");
#else
			int ret = system("sudo shutdown -r now");
#endif
			if (ret != 0)
			{
				_log.Log(LOG_ERROR, "Error executing reboot command. returned: %d", ret);
				return;
			}
			root["title"] = "SystemReboot";
			root["status"] = "OK";
		}

		void CWebServer::Cmd_ExcecuteScript(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string scriptname = request::findValue(&req, "scriptname");
			if (scriptname.empty())
				return;
			if (scriptname.find("..") != std::string::npos)
				return;
#ifdef WIN32
			scriptname = szUserDataFolder + "scripts\\" + scriptname;
#else
			scriptname = szUserDataFolder + "scripts/" + scriptname;
#endif
			if (!file_exist(scriptname.c_str()))
				return;
			std::string script_params = request::findValue(&req, "scriptparams");
			std::string strparm = szUserDataFolder;
			if (!script_params.empty())
			{
				if (strparm.size() > 0)
					strparm += " " + script_params;
				else
					strparm = script_params;
			}
			std::string sdirect = request::findValue(&req, "direct");
			if (sdirect == "true")
			{
				_log.Log(LOG_STATUS, "Executing script: %s", scriptname.c_str());
#ifdef WIN32
				ShellExecute(NULL, "open", scriptname.c_str(), strparm.c_str(), NULL, SW_SHOWNORMAL);
#else
				std::string lscript = scriptname + " " + strparm;
				int ret = system(lscript.c_str());
				if (ret != 0)
				{
					_log.Log(LOG_ERROR, "Error executing script command (%s). returned: %d", lscript.c_str(), ret);
					return;
			}
#endif
		}
			else
			{
				//add script to background worker
				m_sql.AddTaskItem(_tTaskItem::ExecuteScript(0.2f, scriptname, strparm));
			}
			root["title"] = "ExecuteScript";
			root["status"] = "OK";
		}

		void CWebServer::Cmd_GetCosts(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			char szTmp[100];
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT Type, SubType, nValue, sValue FROM DeviceStatus WHERE (ID=='%q')",
				idx.c_str());
			if (!result.empty())
			{
				std::vector<std::string> sd = result[0];

				int nValue = 0;
				root["status"] = "OK";
				root["title"] = "GetElectraCosts";
				m_sql.GetPreferencesVar("CostEnergy", nValue);
				root["CostEnergy"] = nValue;
				m_sql.GetPreferencesVar("CostEnergyT2", nValue);
				root["CostEnergyT2"] = nValue;
				m_sql.GetPreferencesVar("CostEnergyR1", nValue);
				root["CostEnergyR1"] = nValue;
				m_sql.GetPreferencesVar("CostEnergyR2", nValue);
				root["CostEnergyR2"] = nValue;
				m_sql.GetPreferencesVar("CostGas", nValue);
				root["CostGas"] = nValue;
				m_sql.GetPreferencesVar("CostWater", nValue);
				root["CostWater"] = nValue;

				int tValue = 1000;
				if (m_sql.GetPreferencesVar("MeterDividerWater", tValue))
				{
					root["DividerWater"] = float(tValue);
				}

				unsigned char dType = atoi(sd[0].c_str());
				//unsigned char subType = atoi(sd[1].c_str());
				//nValue = (unsigned char)atoi(sd[2].c_str());
				std::string sValue = sd[3];

				if (dType == pTypeP1Power)
				{
					//also provide the counter values

					std::vector<std::string> splitresults;
					StringSplit(sValue, ";", splitresults);
					if (splitresults.size() != 6)
						return;

					float EnergyDivider = 1000.0f;
					if (m_sql.GetPreferencesVar("MeterDividerEnergy", tValue))
					{
						EnergyDivider = float(tValue);
					}

					unsigned long long powerusage1 = std::strtoull(splitresults[0].c_str(), nullptr, 10);
					unsigned long long powerusage2 = std::strtoull(splitresults[1].c_str(), nullptr, 10);
					unsigned long long powerdeliv1 = std::strtoull(splitresults[2].c_str(), nullptr, 10);
					unsigned long long powerdeliv2 = std::strtoull(splitresults[3].c_str(), nullptr, 10);
					unsigned long long usagecurrent = std::strtoull(splitresults[4].c_str(), nullptr, 10);
					unsigned long long delivcurrent = std::strtoull(splitresults[5].c_str(), nullptr, 10);

					powerdeliv1 = (powerdeliv1 < 10) ? 0 : powerdeliv1;
					powerdeliv2 = (powerdeliv2 < 10) ? 0 : powerdeliv2;

					sprintf(szTmp, "%.03f", float(powerusage1) / EnergyDivider);
					root["CounterT1"] = szTmp;
					sprintf(szTmp, "%.03f", float(powerusage2) / EnergyDivider);
					root["CounterT2"] = szTmp;
					sprintf(szTmp, "%.03f", float(powerdeliv1) / EnergyDivider);
					root["CounterR1"] = szTmp;
					sprintf(szTmp, "%.03f", float(powerdeliv2) / EnergyDivider);
					root["CounterR2"] = szTmp;
				}
			}
		}

		void CWebServer::Cmd_CheckForUpdate(WebEmSession & session, const request& req, Json::Value &root)
		{
			bool bHaveUser = (session.username != "");
			int urights = 3;
			if (bHaveUser)
			{
				int iUser = FindUser(session.username.c_str());
				if (iUser != -1)
					urights = static_cast<int>(m_users[iUser].userrights);
			}
			root["statuscode"] = urights;

			root["status"] = "OK";
			root["title"] = "CheckForUpdate";
			root["HaveUpdate"] = false;
			root["Revision"] = m_mainworker.m_iRevision;

			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin users may update
			}

			bool bIsForced = (request::findValue(&req, "forced") == "true");

			if (!bIsForced)
			{
				int nValue = 0;
				m_sql.GetPreferencesVar("UseAutoUpdate", nValue);
				if (nValue != 1)
				{
					return;
				}
			}

			root["HaveUpdate"] = m_mainworker.IsUpdateAvailable(bIsForced);
			root["DomoticzUpdateURL"] = m_mainworker.m_szDomoticzUpdateURL;
			root["SystemName"] = m_mainworker.m_szSystemName;
			root["Revision"] = m_mainworker.m_iRevision;
		}

		void CWebServer::Cmd_DownloadUpdate(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (!m_mainworker.StartDownloadUpdate())
				return;
			root["status"] = "OK";
			root["title"] = "DownloadUpdate";
		}

		void CWebServer::Cmd_DownloadReady(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (!m_mainworker.m_bHaveDownloadedDomoticzUpdate)
				return;
			root["status"] = "OK";
			root["title"] = "DownloadReady";
			root["downloadok"] = (m_mainworker.m_bHaveDownloadedDomoticzUpdateSuccessFull) ? true : false;
		}

		void CWebServer::Cmd_DeleteDatePoint(WebEmSession & session, const request& req, Json::Value &root)
		{
			const std::string idx = request::findValue(&req, "idx");
			const std::string Date = request::findValue(&req, "date");
			if (
				(idx.empty()) ||
				(Date.empty())
				)
				return;
			root["status"] = "OK";
			root["title"] = "deletedatapoint";
			m_sql.DeleteDataPoint(idx.c_str(), Date);
		}

		bool CWebServer::IsIdxForUser(const WebEmSession *pSession, const int Idx)
		{
			if (pSession->rights == 2)
				return true;
			if (pSession->rights == 0)
				return false; //viewer
			//User
			int iUser = FindUser(pSession->username.c_str());
			if ((iUser < 0) || (iUser >= (int)m_users.size()))
				return false;

			if (m_users[iUser].TotSensors == 0)
				return true; // all sensors

			std::vector<std::vector<std::string> > result = m_sql.safe_query("SELECT DeviceRowID FROM SharedDevices WHERE (SharedUserID == '%d') AND (DeviceRowID == '%d')", m_users[iUser].ID, Idx);
			return (!result.empty());
		}


		void CWebServer::HandleCommand(const std::string &cparam, WebEmSession & session, const request& req, Json::Value &root)
		{
			std::map < std::string, webserver_response_function >::iterator pf = m_webcommands.find(cparam);
			if (pf != m_webcommands.end())
			{
				pf->second(session, req, root);
				return;
			}

			std::vector<std::vector<std::string> > result;
			char szTmp[300];

			bool bHaveUser = (session.username != "");
			int iUser = -1;
			if (bHaveUser)
			{
				iUser = FindUser(session.username.c_str());
			}

			if (cparam == "deleteallsubdevices")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "DeleteAllSubDevices";
				result = m_sql.safe_query("DELETE FROM LightSubDevices WHERE (ParentID == '%q')", idx.c_str());
			}
			else if (cparam == "deletesubdevice")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "DeleteSubDevice";
				result = m_sql.safe_query("DELETE FROM LightSubDevices WHERE (ID == '%q')", idx.c_str());
			}
			else if (cparam == "addsubdevice")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				std::string subidx = request::findValue(&req, "subidx");
				if ((idx.empty()) || (subidx.empty()))
					return;
				if (idx == subidx)
					return;

				//first check if it is not already a sub device
				result = m_sql.safe_query("SELECT ID FROM LightSubDevices WHERE (DeviceRowID=='%q') AND (ParentID =='%q')",
					subidx.c_str(), idx.c_str());
				if (result.empty())
				{
					root["status"] = "OK";
					root["title"] = "AddSubDevice";
					//no it is not, add it
					result = m_sql.safe_query(
						"INSERT INTO LightSubDevices (DeviceRowID, ParentID) VALUES ('%q','%q')",
						subidx.c_str(),
						idx.c_str()
					);
				}
			}
			else if (cparam == "addscenedevice")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				std::string devidx = request::findValue(&req, "devidx");
				std::string isscene = request::findValue(&req, "isscene");
				std::string scommand = request::findValue(&req, "command");
				int ondelay = atoi(request::findValue(&req, "ondelay").c_str());
				int offdelay = atoi(request::findValue(&req, "offdelay").c_str());

				if (
					(idx.empty()) ||
					(devidx.empty()) ||
					(isscene.empty())
					)
					return;
				int level = -1;
				if (request::hasValue(&req, "level"))
					level = atoi(request::findValue(&req, "level").c_str());
				std::string color = _tColor(request::findValue(&req, "color")).toJSONString(); //Parse the color to detect incorrectly formatted color data

				unsigned char command = 0;
				result = m_sql.safe_query("SELECT HardwareID, DeviceID, Unit, Type, SubType, SwitchType, Options FROM DeviceStatus WHERE (ID=='%q')",
					devidx.c_str());
				if (!result.empty())
				{
					int dType = atoi(result[0][3].c_str());
					int sType = atoi(result[0][4].c_str());
					_eSwitchType switchtype = (_eSwitchType)atoi(result[0][5].c_str());
					std::map<std::string, std::string> options = m_sql.BuildDeviceOptions(result[0][6].c_str());
					GetLightCommand(dType, sType, switchtype, scommand, command, options);
				}

				//first check if this device is not the scene code!
				result = m_sql.safe_query("SELECT Activators, SceneType FROM Scenes WHERE (ID=='%q')", idx.c_str());
				if (!result.empty())
				{
					int SceneType = atoi(result[0][1].c_str());

					std::vector<std::string> arrayActivators;
					StringSplit(result[0][0], ";", arrayActivators);
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

						if (sID == devidx)
						{
							return; //Group does not work with separate codes, so already there
						}
					}
				}
				//first check if it is not already a part of this scene/group (with the same OnDelay)
				if (isscene == "true") {
					result = m_sql.safe_query("SELECT ID FROM SceneDevices WHERE (DeviceRowID=='%q') AND (SceneRowID =='%q') AND (OnDelay == %d) AND (OffDelay == %d) AND (Cmd == %d)",
						devidx.c_str(), idx.c_str(), ondelay, offdelay, command);
				}
				else {
					result = m_sql.safe_query("SELECT ID FROM SceneDevices WHERE (DeviceRowID=='%q') AND (SceneRowID =='%q') AND (OnDelay == %d)",
						devidx.c_str(), idx.c_str(), ondelay);
				}
				if (result.empty())
				{
					root["status"] = "OK";
					root["title"] = "AddSceneDevice";
					//no it is not, add it
					if (isscene == "true")
					{
						m_sql.safe_query(
							"INSERT INTO SceneDevices (DeviceRowID, SceneRowID, Cmd, Level, Color, OnDelay, OffDelay) VALUES ('%q','%q',%d,%d,'%q',%d,%d)",
							devidx.c_str(),
							idx.c_str(),
							command,
							level,
							color.c_str(),
							ondelay,
							offdelay
						);
					}
					else
					{
						m_sql.safe_query(
							"INSERT INTO SceneDevices (DeviceRowID, SceneRowID, Level, Color, OnDelay, OffDelay) VALUES ('%q','%q',%d,'%q',%d,%d)",
							devidx.c_str(),
							idx.c_str(),
							level,
							color.c_str(),
							ondelay,
							offdelay
						);
					}
					m_mainworker.m_eventsystem.GetCurrentScenesGroups();
				}
			}
			else if (cparam == "updatescenedevice")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				std::string devidx = request::findValue(&req, "devidx");
				std::string scommand = request::findValue(&req, "command");
				int ondelay = atoi(request::findValue(&req, "ondelay").c_str());
				int offdelay = atoi(request::findValue(&req, "offdelay").c_str());

				if (
					(idx.empty()) ||
					(devidx.empty())
					)
					return;

				unsigned char command = 0;

				result = m_sql.safe_query("SELECT HardwareID, DeviceID, Unit, Type, SubType, SwitchType, Options FROM DeviceStatus WHERE (ID=='%q')",
					devidx.c_str());
				if (!result.empty())
				{
					int dType = atoi(result[0][3].c_str());
					int sType = atoi(result[0][4].c_str());
					_eSwitchType switchtype = (_eSwitchType)atoi(result[0][5].c_str());
					std::map<std::string, std::string> options = m_sql.BuildDeviceOptions(result[0][6].c_str());
					GetLightCommand(dType, sType, switchtype, scommand, command, options);
				}
				int level = -1;
				if (request::hasValue(&req, "level"))
					level = atoi(request::findValue(&req, "level").c_str());
				std::string color = _tColor(request::findValue(&req, "color")).toJSONString(); //Parse the color to detect incorrectly formatted color data
				root["status"] = "OK";
				root["title"] = "UpdateSceneDevice";
				result = m_sql.safe_query(
					"UPDATE SceneDevices SET Cmd=%d, Level=%d, Color='%q', OnDelay=%d, OffDelay=%d  WHERE (ID == '%q')",
					command, level, color.c_str(), ondelay, offdelay, idx.c_str());
			}
			else if (cparam == "deletescenedevice")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "DeleteSceneDevice";
				m_sql.safe_query("DELETE FROM SceneDevices WHERE (ID == '%q')", idx.c_str());
				m_sql.safe_query("DELETE FROM CamerasActiveDevices WHERE (DevSceneType==1) AND (DevSceneRowID == '%q')", idx.c_str());
				m_mainworker.m_eventsystem.GetCurrentScenesGroups();
			}
			else if (cparam == "getsubdevices")
			{
				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;

				root["status"] = "OK";
				root["title"] = "GetSubDevices";
				result = m_sql.safe_query("SELECT a.ID, b.Name FROM LightSubDevices a, DeviceStatus b WHERE (a.ParentID=='%q') AND (b.ID == a.DeviceRowID)",
					idx.c_str());
				if (!result.empty())
				{
					int ii = 0;
					for (const auto & itt : result)
					{
						std::vector<std::string> sd = itt;

						root["result"][ii]["ID"] = sd[0];
						root["result"][ii]["Name"] = sd[1];
						ii++;
					}
				}
			}
			else if (cparam == "getscenedevices")
			{
				std::string idx = request::findValue(&req, "idx");
				std::string isscene = request::findValue(&req, "isscene");

				if (
					(idx.empty()) ||
					(isscene.empty())
					)
					return;

				root["status"] = "OK";
				root["title"] = "GetSceneDevices";

				result = m_sql.safe_query("SELECT a.ID, b.Name, a.DeviceRowID, b.Type, b.SubType, b.nValue, b.sValue, a.Cmd, a.Level, b.ID, a.[Order], a.Color, a.OnDelay, a.OffDelay, b.SwitchType FROM SceneDevices a, DeviceStatus b WHERE (a.SceneRowID=='%q') AND (b.ID == a.DeviceRowID) ORDER BY a.[Order]",
					idx.c_str());
				if (!result.empty())
				{
					int ii = 0;
					for (const auto & itt : result)
					{
						std::vector<std::string> sd = itt;

						root["result"][ii]["ID"] = sd[0];
						root["result"][ii]["Name"] = sd[1];
						root["result"][ii]["DevID"] = sd[2];
						root["result"][ii]["DevRealIdx"] = sd[9];
						root["result"][ii]["Order"] = atoi(sd[10].c_str());
						root["result"][ii]["OnDelay"] = atoi(sd[12].c_str());
						root["result"][ii]["OffDelay"] = atoi(sd[13].c_str());

						_eSwitchType switchtype = (_eSwitchType)atoi(sd[14].c_str());

						unsigned char devType = atoi(sd[3].c_str());

						//switchtype seemed not to be used down with the GetLightStatus command,
						//causing RFY to go wrong, fixing here
						if (devType != pTypeRFY)
							switchtype = STYPE_OnOff;

						unsigned char subType = atoi(sd[4].c_str());
						unsigned char nValue = (unsigned char)atoi(sd[5].c_str());
						std::string sValue = sd[6];
						int command = atoi(sd[7].c_str());
						int level = atoi(sd[8].c_str());

						std::string lstatus = "";
						int llevel = 0;
						bool bHaveDimmer = false;
						bool bHaveGroupCmd = false;
						int maxDimLevel = 0;
						GetLightStatus(devType, subType, switchtype, command, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);
						root["result"][ii]["Command"] = lstatus;
						root["result"][ii]["Level"] = level;
						root["result"][ii]["Color"] = _tColor(sd[11]).toJSONString();
						root["result"][ii]["Type"] = RFX_Type_Desc(devType, 1);
						root["result"][ii]["SubType"] = RFX_Type_SubType_Desc(devType, subType);
						ii++;
					}
				}
			}
			else if (cparam == "changescenedeviceorder")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				std::string sway = request::findValue(&req, "way");
				if (sway.empty())
					return;
				bool bGoUp = (sway == "0");

				std::string aScene, aOrder, oID, oOrder;

				//Get actual device order
				result = m_sql.safe_query("SELECT SceneRowID, [Order] FROM SceneDevices WHERE (ID=='%q')",
					idx.c_str());
				if (result.empty())
					return;
				aScene = result[0][0];
				aOrder = result[0][1];

				if (!bGoUp)
				{
					//Get next device order
					result = m_sql.safe_query("SELECT ID, [Order] FROM SceneDevices WHERE (SceneRowID=='%q' AND [Order]>'%q') ORDER BY [Order] ASC",
						aScene.c_str(), aOrder.c_str());
					if (result.empty())
						return;
					oID = result[0][0];
					oOrder = result[0][1];
				}
				else
				{
					//Get previous device order
					result = m_sql.safe_query("SELECT ID, [Order] FROM SceneDevices WHERE (SceneRowID=='%q' AND [Order]<'%q') ORDER BY [Order] DESC",
						aScene.c_str(), aOrder.c_str());
					if (result.empty())
						return;
					oID = result[0][0];
					oOrder = result[0][1];
				}
				//Swap them
				root["status"] = "OK";
				root["title"] = "ChangeSceneDeviceOrder";

				result = m_sql.safe_query("UPDATE SceneDevices SET [Order] = '%q' WHERE (ID='%q')",
					oOrder.c_str(), idx.c_str());
				result = m_sql.safe_query("UPDATE SceneDevices SET [Order] = '%q' WHERE (ID='%q')",
					aOrder.c_str(), oID.c_str());
			}
			else if (cparam == "deleteallscenedevices")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "DeleteAllSceneDevices";
				result = m_sql.safe_query("DELETE FROM SceneDevices WHERE (SceneRowID == %q)", idx.c_str());
			}
			else if (cparam == "getmanualhardware")
			{
				//used by Add Manual Light/Switch dialog
				root["status"] = "OK";
				root["title"] = "GetHardware";
				result = m_sql.safe_query("SELECT ID, Name, Type FROM Hardware ORDER BY ID ASC");
				if (!result.empty())
				{
					int ii = 0;
					for (const auto & itt : result)
					{
						std::vector<std::string> sd = itt;

						int ID = atoi(sd[0].c_str());
						std::string Name = sd[1];
						_eHardwareTypes Type = (_eHardwareTypes)atoi(sd[2].c_str());

						if (
							(Type == HTYPE_RFXLAN) ||
							(Type == HTYPE_RFXtrx315) ||
							(Type == HTYPE_RFXtrx433) ||
							(Type == HTYPE_RFXtrx868) ||
							(Type == HTYPE_EnOceanESP2) ||
							(Type == HTYPE_EnOceanESP3) ||
							(Type == HTYPE_Dummy) ||
							(Type == HTYPE_Tellstick) ||
							(Type == HTYPE_EVOHOME_SCRIPT) ||
							(Type == HTYPE_EVOHOME_SERIAL) ||
							(Type == HTYPE_EVOHOME_WEB) ||
							(Type == HTYPE_EVOHOME_TCP) ||
							(Type == HTYPE_RaspberryGPIO) ||
							(Type == HTYPE_RFLINKUSB) ||
							(Type == HTYPE_RFLINKTCP) ||
							(Type == HTYPE_ZIBLUEUSB) ||
							(Type == HTYPE_ZIBLUETCP) ||
							(Type == HTYPE_OpenWebNetTCP) ||
							(Type == HTYPE_OpenWebNetUSB) ||
							(Type == HTYPE_SysfsGpio) ||
							(Type == HTYPE_USBtinGateway)
							)
						{
							root["result"][ii]["idx"] = ID;
							root["result"][ii]["Name"] = Name;
							ii++;
						}
					}
				}
			}
			else if (cparam == "getgpio")
			{
				//used by Add Manual Light/Switch dialog
				root["title"] = "GetGpio";
#ifdef WITH_GPIO
				std::vector<CGpioPin> pins = CGpio::GetPinList();
				if (pins.size() == 0) {
					root["status"] = "ERROR";
					root["result"][0]["idx"] = 0;
					root["result"][0]["Name"] = "GPIO INIT ERROR";
				}
				else {
					int ii = 0;
					for (const auto & it : pins)
					{
						CGpioPin pin = it;
						root["status"] = "OK";
						root["result"][ii]["idx"] = pin.GetPin();
						root["result"][ii]["Name"] = pin.ToString();
						ii++;
					}
				}
#else
				root["status"] = "OK";
				root["result"][0]["idx"] = 0;
				root["result"][0]["Name"] = "N/A";
#endif
			}
			else if (cparam == "getsysfsgpio")
			{
				//used by Add Manual Light/Switch dialog
				root["title"] = "GetSysfsGpio";
#ifdef WITH_GPIO
				std::vector<int> gpio_ids = CSysfsGpio::GetGpioIds();
				std::vector<std::string> gpio_names = CSysfsGpio::GetGpioNames();

				if (gpio_ids.size() == 0) {
					root["status"] = "ERROR";
					root["result"][0]["idx"] = 0;
					root["result"][0]["Name"] = "No sysfs-gpio exports";
				}
				else {
					for (int ii = 0; ii < gpio_ids.size(); ii++)
					{
						root["status"] = "OK";
						root["result"][ii]["idx"] = gpio_ids[ii];
						root["result"][ii]["Name"] = gpio_names[ii];
			}
				}
#else
				root["status"] = "OK";
				root["result"][0]["idx"] = 0;
				root["result"][0]["Name"] = "N/A";
#endif
			}
			else if (cparam == "getlightswitches")
			{
				root["status"] = "OK";
				root["title"] = "GetLightSwitches";
				result = m_sql.safe_query("SELECT ID, Name, Type, SubType, Used, SwitchType, Options FROM DeviceStatus ORDER BY Name");
				if (!result.empty())
				{
					int ii = 0;
					for (const auto & itt : result)
					{
						std::vector<std::string> sd = itt;

						std::string ID = sd[0];
						std::string Name = sd[1];
						int Type = atoi(sd[2].c_str());
						int SubType = atoi(sd[3].c_str());
						int used = atoi(sd[4].c_str());
						_eSwitchType switchtype = (_eSwitchType)atoi(sd[5].c_str());
						std::map<std::string, std::string> options = m_sql.BuildDeviceOptions(sd[6]);
						bool bdoAdd = false;
						switch (Type)
						{
						case pTypeLighting1:
						case pTypeLighting2:
						case pTypeLighting3:
						case pTypeLighting4:
						case pTypeLighting5:
						case pTypeLighting6:
						case pTypeFan:
						case pTypeColorSwitch:
						case pTypeSecurity1:
						case pTypeSecurity2:
						case pTypeEvohome:
						case pTypeEvohomeRelay:
						case pTypeCurtain:
						case pTypeBlinds:
						case pTypeRFY:
						case pTypeChime:
						case pTypeThermostat2:
						case pTypeThermostat3:
						case pTypeThermostat4:
						case pTypeRemote:
						case pTypeRadiator1:
						case pTypeGeneralSwitch:
						case pTypeHomeConfort:
						case pTypeFS20:
						case pTypeHunter:
							bdoAdd = true;
							if (!used)
							{
								bdoAdd = false;
								bool bIsSubDevice = false;
								std::vector<std::vector<std::string> > resultSD;
								resultSD = m_sql.safe_query("SELECT ID FROM LightSubDevices WHERE (DeviceRowID=='%q')",
									sd[0].c_str());
								if (resultSD.size() > 0)
									bdoAdd = true;
							}
							if ((Type == pTypeRadiator1) && (SubType != sTypeSmartwaresSwitchRadiator))
								bdoAdd = false;
							if (bdoAdd)
							{
								int idx = atoi(ID.c_str());
								if (!IsIdxForUser(&session, idx))
									continue;
								root["result"][ii]["idx"] = ID;
								root["result"][ii]["Name"] = Name;
								root["result"][ii]["Type"] = RFX_Type_Desc(Type, 1);
								root["result"][ii]["SubType"] = RFX_Type_SubType_Desc(Type, SubType);
								bool bIsDimmer = (
									(switchtype == STYPE_Dimmer) ||
									(switchtype == STYPE_BlindsPercentage) ||
									(switchtype == STYPE_BlindsPercentageInverted) ||
									(switchtype == STYPE_Selector)
									);
								root["result"][ii]["IsDimmer"] = bIsDimmer;

								std::string dimmerLevels = "none";

								if (bIsDimmer)
								{
									std::stringstream ss;

									if (switchtype == STYPE_Selector) {
										std::map<std::string, std::string> selectorStatuses;
										GetSelectorSwitchStatuses(options, selectorStatuses);
										bool levelOffHidden = (options["LevelOffHidden"] == "true");
										for (int i = 0; i < (int)selectorStatuses.size(); i++) {
											if (levelOffHidden && (i == 0)) {
												continue;
											}
											if ((levelOffHidden && (i > 1)) || (i > 0)) {
												ss << ",";
											}
											ss << i * 10;
										}
									}
									else
									{
										int nValue = 0;
										std::string sValue = "";
										std::string lstatus = "";
										int llevel = 0;
										bool bHaveDimmer = false;
										int maxDimLevel = 0;
										bool bHaveGroupCmd = false;

										GetLightStatus(Type, SubType, switchtype, nValue, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);

										for (int i = 0; i <= maxDimLevel; i++)
										{
											if (i != 0)
											{
												ss << ",";
											}
											ss << (int)float((100.0f / float(maxDimLevel))*i);
										}
									}
									dimmerLevels = ss.str();
								}
								root["result"][ii]["DimmerLevels"] = dimmerLevels;
								ii++;
							}
							break;
						}
					}
				}
			}
			else if (cparam == "getlightswitchesscenes")
			{
				root["status"] = "OK";
				root["title"] = "GetLightSwitchesScenes";
				int ii = 0;

				//First List/Switch Devices
				result = m_sql.safe_query("SELECT ID, Name, Type, SubType, Used FROM DeviceStatus ORDER BY Name");
				if (!result.empty())
				{
					for (const auto & itt : result)
					{
						std::vector<std::string> sd = itt;

						std::string ID = sd[0];
						std::string Name = sd[1];
						int Type = atoi(sd[2].c_str());
						int SubType = atoi(sd[3].c_str());
						int used = atoi(sd[4].c_str());
						if (used)
						{
							switch (Type)
							{
							case pTypeLighting1:
							case pTypeLighting2:
							case pTypeLighting3:
							case pTypeLighting4:
							case pTypeLighting5:
							case pTypeLighting6:
							case pTypeFan:
							case pTypeColorSwitch:
							case pTypeSecurity1:
							case pTypeSecurity2:
							case pTypeEvohome:
							case pTypeEvohomeRelay:
							case pTypeCurtain:
							case pTypeBlinds:
							case pTypeRFY:
							case pTypeChime:
							case pTypeThermostat2:
							case pTypeThermostat3:
							case pTypeThermostat4:
							case pTypeRemote:
							case pTypeGeneralSwitch:
							case pTypeHomeConfort:
							case pTypeFS20:
							case pTypeHunter:
								root["result"][ii]["type"] = 0;
								root["result"][ii]["idx"] = ID;
								root["result"][ii]["Name"] = "[Light/Switch] " + Name;
								ii++;
								break;
							case pTypeRadiator1:
								if (SubType == sTypeSmartwaresSwitchRadiator)
								{
									root["result"][ii]["type"] = 0;
									root["result"][ii]["idx"] = ID;
									root["result"][ii]["Name"] = "[Light/Switch] " + Name;
									ii++;
								}
								break;
							}
						}
					}
				}//end light/switches

				//Add Scenes
				result = m_sql.safe_query("SELECT ID, Name FROM Scenes ORDER BY Name");
				if (!result.empty())
				{
					for (const auto & itt : result)
					{
						std::vector<std::string> sd = itt;

						std::string ID = sd[0];
						std::string Name = sd[1];

						root["result"][ii]["type"] = 1;
						root["result"][ii]["idx"] = ID;
						root["result"][ii]["Name"] = "[Scene] " + Name;
						ii++;
					}
				}//end light/switches
			}
			else if (cparam == "getcamactivedevices")
			{
				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "GetCameraActiveDevices";
				//First List/Switch Devices
				result = m_sql.safe_query("SELECT ID, DevSceneType, DevSceneRowID, DevSceneWhen, DevSceneDelay FROM CamerasActiveDevices WHERE (CameraRowID=='%q') ORDER BY ID",
					idx.c_str());
				if (!result.empty())
				{
					int ii = 0;
					for (const auto & itt : result)
					{
						std::vector<std::string> sd = itt;

						std::string ID = sd[0];
						int DevSceneType = atoi(sd[1].c_str());
						std::string DevSceneRowID = sd[2];
						int DevSceneWhen = atoi(sd[3].c_str());
						int DevSceneDelay = atoi(sd[4].c_str());

						std::string Name = "";
						if (DevSceneType == 0)
						{
							std::vector<std::vector<std::string> > result2;
							result2 = m_sql.safe_query("SELECT Name FROM DeviceStatus WHERE (ID=='%q')",
								DevSceneRowID.c_str());
							if (!result2.empty())
							{
								Name = "[Light/Switches] " + result2[0][0];
							}
						}
						else
						{
							std::vector<std::vector<std::string> > result2;
							result2 = m_sql.safe_query("SELECT Name FROM Scenes WHERE (ID=='%q')",
								DevSceneRowID.c_str());
							if (!result2.empty())
							{
								Name = "[Scene] " + result2[0][0];
							}
						}
						if (Name != "")
						{
							root["result"][ii]["idx"] = ID;
							root["result"][ii]["type"] = DevSceneType;
							root["result"][ii]["DevSceneRowID"] = DevSceneRowID;
							root["result"][ii]["when"] = DevSceneWhen;
							root["result"][ii]["delay"] = DevSceneDelay;
							root["result"][ii]["Name"] = Name;
							ii++;
						}
					}
				}
			}
			else if (cparam == "addcamactivedevice")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				std::string activeidx = request::findValue(&req, "activeidx");
				std::string sactivetype = request::findValue(&req, "activetype");
				std::string sactivewhen = request::findValue(&req, "activewhen");
				std::string sactivedelay = request::findValue(&req, "activedelay");

				if (
					(idx.empty()) ||
					(activeidx.empty()) ||
					(sactivetype.empty()) ||
					(sactivewhen.empty()) ||
					(sactivedelay.empty())
					)
				{
					return;
				}

				int activetype = atoi(sactivetype.c_str());
				int activewhen = atoi(sactivewhen.c_str());
				int activedelay = atoi(sactivedelay.c_str());

				//first check if it is not already a Active Device
				result = m_sql.safe_query(
					"SELECT ID FROM CamerasActiveDevices WHERE (CameraRowID=='%q')"
					" AND (DevSceneType==%d) AND (DevSceneRowID=='%q')"
					" AND (DevSceneWhen==%d)",
					idx.c_str(), activetype, activeidx.c_str(), activewhen);
				if (result.empty())
				{
					root["status"] = "OK";
					root["title"] = "AddCameraActiveDevice";
					//no it is not, add it
					result = m_sql.safe_query(
						"INSERT INTO CamerasActiveDevices (CameraRowID, DevSceneType, DevSceneRowID, DevSceneWhen, DevSceneDelay) VALUES ('%q',%d,'%q',%d,%d)",
						idx.c_str(),
						activetype,
						activeidx.c_str(),
						activewhen,
						activedelay
					);
					m_mainworker.m_cameras.ReloadCameras();
				}
			}
			else if (cparam == "deleteamactivedevice")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "DeleteCameraActiveDevice";
				result = m_sql.safe_query("DELETE FROM CamerasActiveDevices WHERE (ID == '%q')", idx.c_str());
				m_mainworker.m_cameras.ReloadCameras();
			}
			else if (cparam == "deleteallactivecamdevices")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "DeleteAllCameraActiveDevices";
				result = m_sql.safe_query("DELETE FROM CamerasActiveDevices WHERE (CameraRowID == '%q')", idx.c_str());
				m_mainworker.m_cameras.ReloadCameras();
			}
			else if (cparam == "testnotification")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string notification_Title = "Domoticz test";
				std::string notification_Message = "Domoticz test message!";
				std::string subsystem = request::findValue(&req, "subsystem");

				std::string extraData = request::findValue(&req, "extradata");

				m_notifications.ConfigFromGetvars(req, false);
				if (m_notifications.SendMessage(0, std::string(""), subsystem, notification_Title, notification_Message, extraData, 1, std::string(""), false)) {
					root["status"] = "OK";
				}
				/* we need to reload the config, because the values that were set were only for testing */
				m_notifications.LoadConfig();
			}
			else if (cparam == "testswitch")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string hwdid = request::findValue(&req, "hwdid");
				std::string sswitchtype = request::findValue(&req, "switchtype");
				std::string slighttype = request::findValue(&req, "lighttype");

				if (
					(hwdid.empty()) ||
					(sswitchtype.empty()) ||
					(slighttype.empty())
					)
					return;
				_eSwitchType switchtype = (_eSwitchType)atoi(sswitchtype.c_str());
				int lighttype = atoi(slighttype.c_str());
				int dtype;
				int subtype = 0;
				std::string sunitcode;
				std::string devid;

				if (lighttype == 70)
				{
					//EnOcean (Lighting2 with Base_ID offset)
					dtype = pTypeLighting2;
					subtype = sTypeAC;
					std::string sgroupcode = request::findValue(&req, "groupcode");
					sunitcode = request::findValue(&req, "unitcode");
					int iUnitTest = atoi(sunitcode.c_str());	//only First Rocker_ID at the moment, gives us 128 devices we can control, should be enough!
					if (
						(sunitcode.empty()) ||
						(sgroupcode.empty()) ||
						((iUnitTest < 1) || (iUnitTest > 128))
						)
						return;
					sunitcode = sgroupcode;//Button A or B
					CDomoticzHardwareBase *pBaseHardware = reinterpret_cast<CDomoticzHardwareBase*>(m_mainworker.GetHardware(atoi(hwdid.c_str())));
					if (pBaseHardware == NULL)
						return;
					if ((pBaseHardware->HwdType != HTYPE_EnOceanESP2) && (pBaseHardware->HwdType != HTYPE_EnOceanESP3)
						&& (pBaseHardware->HwdType != HTYPE_USBtinGateway) )
						return;
					unsigned long rID = 0;
					//convert to hex, and we have our ID
					std::stringstream s_strid;
					s_strid << std::hex << std::uppercase << rID;
					devid = s_strid.str();
				}
				else if (lighttype == 68)
				{
				}
				else if (lighttype == 69)
				{
#ifdef WITH_GPIO

					sunitcode = request::findValue(&req, "unitcode"); // sysfs-gpio number
					int unitcode = atoi(sunitcode.c_str());
					dtype = pTypeLighting2;
					subtype = sTypeAC;
					std::string sswitchtype = request::findValue(&req, "switchtype");
					_eSwitchType switchtype = (_eSwitchType)atoi(sswitchtype.c_str());

					std::string id = request::findValue(&req, "id");
					if ((id.empty()) || (sunitcode.empty()))
					{
						return;
					}
					devid = id;

					if (sunitcode.empty())
					{
						root["status"] = "ERROR";
						root["message"] = "No GPIO number given";
						return;
					}

					CSysfsGpio *pSysfsGpio = reinterpret_cast<CSysfsGpio *>(m_mainworker.GetHardware(atoi(hwdid.c_str())));

					if (pSysfsGpio == NULL) {
						root["status"] = "ERROR";
						root["message"] = "Could not retrieve SysfsGpio hardware pointer";
						return;
					}

					if (pSysfsGpio->HwdType != HTYPE_SysfsGpio) {
						root["status"] = "ERROR";
						root["message"] = "Given hardware is not SysfsGpio";
						return;
					}
#else
					root["status"] = "ERROR";
					root["message"] = "GPIO support is disabled";
					return;
#endif
				}
				else if (lighttype < 20)
				{
					dtype = pTypeLighting1;
					subtype = lighttype;
					std::string shousecode = request::findValue(&req, "housecode");
					sunitcode = request::findValue(&req, "unitcode");
					if (
						(shousecode.empty()) ||
						(sunitcode.empty())
						)
						return;
					devid = shousecode;
				}
				else if (lighttype < 30)
				{
					dtype = pTypeLighting2;
					subtype = lighttype - 20;
					std::string id = request::findValue(&req, "id");
					sunitcode = request::findValue(&req, "unitcode");
					if (
						(id.empty()) ||
						(sunitcode.empty())
						)
						return;
					devid = id;
				}
				else if (lighttype < 70)
				{
					dtype = pTypeLighting5;
					subtype = lighttype - 50;
					if (lighttype == 59)
						subtype = sTypeIT;
					std::string id = request::findValue(&req, "id");
					sunitcode = request::findValue(&req, "unitcode");
					if (
						(id.empty()) ||
						(sunitcode.empty())
						)
						return;
					if ((subtype != sTypeEMW100) && (subtype != sTypeLivolo) && (subtype != sTypeLivolo1to10) && (subtype != sTypeRGB432W) && (subtype != sTypeIT))
						devid = "00" + id;
					else
						devid = id;
				}
				else
				{
					if (lighttype == 100)
					{
						//Chime/ByronSX
						dtype = pTypeChime;
						subtype = sTypeByronSX;
						std::string id = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(sunitcode.empty())
							)
							return;
						int iUnitCode = atoi(sunitcode.c_str()) - 1;
						switch (iUnitCode)
						{
						case 0:
							iUnitCode = chime_sound0;
							break;
						case 1:
							iUnitCode = chime_sound1;
							break;
						case 2:
							iUnitCode = chime_sound2;
							break;
						case 3:
							iUnitCode = chime_sound3;
							break;
						case 4:
							iUnitCode = chime_sound4;
							break;
						case 5:
							iUnitCode = chime_sound5;
							break;
						case 6:
							iUnitCode = chime_sound6;
							break;
						case 7:
							iUnitCode = chime_sound7;
							break;
						}
						sprintf(szTmp, "%d", iUnitCode);
						sunitcode = szTmp;
						devid = id;
					}
					else if (lighttype == 101)
					{
						//Curtain Harrison
						dtype = pTypeCurtain;
						subtype = sTypeHarrison;
						std::string shousecode = request::findValue(&req, "housecode");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(shousecode.empty()) ||
							(sunitcode.empty())
							)
							return;
						devid = shousecode;
					}
					else if (lighttype == 102)
					{
						//RFY
						dtype = pTypeRFY;
						subtype = sTypeRFY;
						std::string id = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(sunitcode.empty())
							)
							return;
						devid = id;
					}
					else if (lighttype == 103)
					{
						//Meiantech
						dtype = pTypeSecurity1;
						subtype = sTypeMeiantech;
						std::string id = request::findValue(&req, "id");
						if (
							(id.empty())
							)
							return;
						devid = id;
						sunitcode = "0";
					}
					else if (lighttype == 104)
					{
						//HE105
						dtype = pTypeThermostat2;
						subtype = sTypeHE105;
						sunitcode = request::findValue(&req, "unitcode");
						if (sunitcode.empty())
							return;
						//convert to hex, and we have our Unit Code
						std::stringstream s_strid;
						s_strid << std::hex << std::uppercase << sunitcode;
						int iUnitCode;
						s_strid >> iUnitCode;
						sprintf(szTmp, "%d", iUnitCode);
						sunitcode = szTmp;
						devid = "1";
					}
					else if (lighttype == 105)
					{
						//ASA
						dtype = pTypeRFY;
						subtype = sTypeASA;
						std::string id = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(sunitcode.empty())
							)
							return;
						devid = id;
					}
					else if (lighttype == 106)
					{
						//Blyss
						dtype = pTypeLighting6;
						subtype = sTypeBlyss;
						std::string sgroupcode = request::findValue(&req, "groupcode");
						sunitcode = request::findValue(&req, "unitcode");
						std::string id = request::findValue(&req, "id");
						if (
							(sgroupcode.empty()) ||
							(sunitcode.empty()) ||
							(id.empty())
							)
							return;
						devid = id + sgroupcode;
					}
					else if (lighttype == 107)
					{
						//RFY2
						dtype = pTypeRFY;
						subtype = sTypeRFY2;
						std::string id = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(sunitcode.empty())
							)
							return;
						devid = id;
					}
					else if ((lighttype >= 200) && (lighttype < 300))
					{
						dtype = pTypeBlinds;
						subtype = lighttype - 200;
						std::string id = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(sunitcode.empty())
							)
							return;
						int iUnitCode = atoi(sunitcode.c_str());
						sprintf(szTmp, "%d", iUnitCode);
						sunitcode = szTmp;
						devid = id;
					}
					else if (lighttype == 301)
					{
						//Smartwares Radiator
						dtype = pTypeRadiator1;
						subtype = sTypeSmartwaresSwitchRadiator;
						std::string id = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(sunitcode.empty())
							)
							return;
						devid = id;
					}
					else if (lighttype == 302)
					{
						//Home Confort
						dtype = pTypeHomeConfort;
						subtype = sTypeHomeConfortTEL010;
						std::string id = request::findValue(&req, "id");

						std::string shousecode = request::findValue(&req, "housecode");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(shousecode.empty()) ||
							(sunitcode.empty())
							)
							return;

						int iUnitCode = atoi(sunitcode.c_str());
						sprintf(szTmp, "%d", iUnitCode);
						sunitcode = szTmp;
						sprintf(szTmp, "%02X", atoi(shousecode.c_str()));
						shousecode = szTmp;
						devid = id + shousecode;
					}
					else if (lighttype == 303)
					{
						//Selector Switch
						dtype = pTypeGeneralSwitch;
						subtype = sSwitchTypeSelector;
						std::string id = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(sunitcode.empty())
							)
							return;
						devid = id;
					}
					else if (lighttype == 304)
					{
						//Itho CVE RFT
						dtype = pTypeFan;
						subtype = sTypeItho;
						std::string id = request::findValue(&req, "id");
						if (id.empty())
							return;
						devid = id;
						sunitcode = "0";
					}
					else if (lighttype == 305)
					{
						//Lucci Air/DC
						dtype = pTypeFan;
						subtype = sTypeLucciAir;
						std::string id = request::findValue(&req, "id");
						if (id.empty())
							return;
						devid = id;
						sunitcode = "0";
					}
					else if (lighttype == 306)
					{
						//Lucci Air DC
						dtype = pTypeFan;
						subtype = sTypeLucciAirDC;
						std::string id = request::findValue(&req, "id");
						if (id.empty())
							return;
						devid = id;
						sunitcode = "0";
					}
					else if (lighttype == 307)
					{
						//Westinghouse
						dtype = pTypeFan;
						subtype = sTypeWestinghouse;
						std::string id = request::findValue(&req, "id");
						if (id.empty())
							return;
						devid = id;
						sunitcode = "0";
					}
					else if (lighttype == 400) {
						//Openwebnet Bus Blinds
						dtype = pTypeLighting2;
						subtype = sTypeAC;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if (lighttype == 401) {
						//Openwebnet Bus Lights
						dtype = pTypeLighting2;
						subtype = sTypeAC;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if (lighttype == 402)
					{
						//Openwebnet Bus Auxiliary
						dtype = pTypeLighting2;
						subtype = sTypeAC;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if (lighttype == 403) {
						//Openwebnet Zigbee Blinds
						dtype = pTypeGeneralSwitch;
						subtype = sSwitchBlindsT2;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if (lighttype == 404) {
						//Light Openwebnet Zigbee
						dtype = pTypeGeneralSwitch;
						subtype = sSwitchLightT2;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if ((lighttype == 405) || (lighttype == 406)) {
						// Openwebnet Dry Contact / IR Detection
						dtype = pTypeGeneralSwitch;
						subtype = sSwitchContactT1;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if (lighttype == 407) {
						//Openwebnet Bus Custom
						dtype = pTypeLighting2;
						subtype = sTypeAC;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						std::string StrParam1 = request::findValue(&req, "StrParam1");
						if (
							(devid.empty()) ||
							(sunitcode.empty()) ||
							(StrParam1.empty())
							)
						{
							root["message"] = "Some field empty or not valid.";
							return;
						}
					}
				}
				// ----------- If needed convert to GeneralSwitch type (for o.a. RFlink) -----------
				CDomoticzHardwareBase *pBaseHardware = reinterpret_cast<CDomoticzHardwareBase*>(m_mainworker.GetHardware(atoi(hwdid.c_str())));
				if (pBaseHardware != NULL)
				{
					if ((pBaseHardware->HwdType == HTYPE_RFLINKUSB) || (pBaseHardware->HwdType == HTYPE_RFLINKTCP)) {
						ConvertToGeneralSwitchType(devid, dtype, subtype);
					}
				}
				// -----------------------------------------------

				root["status"] = "OK";
				root["message"] = "OK";
				root["title"] = "TestSwitch";
				std::vector<std::string> sd;

				sd.push_back(hwdid);
				sd.push_back(devid);
				sd.push_back(sunitcode);
				sprintf(szTmp, "%d", dtype);
				sd.push_back(szTmp);
				sprintf(szTmp, "%d", subtype);
				sd.push_back(szTmp);
				sprintf(szTmp, "%d", switchtype);
				sd.push_back(szTmp);
				sd.push_back(""); //AddjValue2
				sd.push_back(""); //nValue
				sd.push_back(""); //sValue
				sd.push_back(""); //Name
				sd.push_back(""); //Options

				std::string switchcmd = "On";
				int level = 0;
				if (lighttype == 70)
				{
					//Special EnOcean case, if it is a dimmer, set a dim value
					if (switchtype == STYPE_Dimmer)
						level = 5;
				}
				m_mainworker.SwitchLightInt(sd, switchcmd, level, NoColor, true);
			}
			else if (cparam == "addswitch")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string hwdid = request::findValue(&req, "hwdid");
				std::string name = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));
				std::string sswitchtype = request::findValue(&req, "switchtype");
				std::string slighttype = request::findValue(&req, "lighttype");
				std::string maindeviceidx = request::findValue(&req, "maindeviceidx");
				std::string deviceoptions;

				if (
					(hwdid.empty()) ||
					(sswitchtype.empty()) ||
					(slighttype.empty()) ||
					(name.empty())
					)
					return;
				_eSwitchType switchtype = (_eSwitchType)atoi(sswitchtype.c_str());
				int lighttype = atoi(slighttype.c_str());
				int dtype = 0;
				int subtype = 0;
				std::string sunitcode;
				std::string devid;
				std::string StrParam1;

#ifdef ENABLE_PYTHON
				//check if HW is plugin
				{
					result = m_sql.safe_query("SELECT Type FROM Hardware WHERE (ID == '%q')", hwdid.c_str());
					if (!result.empty())
					{
						std::vector<std::string> sd = result[0];
						_eHardwareTypes Type = (_eHardwareTypes)atoi(sd[0].c_str());
						if (Type == HTYPE_PythonPlugin)
						{
							// Not allowed to add device to plugin HW (plugin framework does not use key column "ID" but instead uses column "unit" as key)
							_log.Log(LOG_ERROR, "CWebServer::HandleCommand addswitch: Not allowed to add device owned by plugin %u!", atoi(hwdid.c_str()));
							root["message"] = "Not allowed to add switch to plugin HW!";
							return;
						}
					}
				}
#endif

				if (lighttype == 70)
				{
					//EnOcean (Lighting2 with Base_ID offset)
					dtype = pTypeLighting2;
					subtype = sTypeAC;
					sunitcode = request::findValue(&req, "unitcode");
					std::string sgroupcode = request::findValue(&req, "groupcode");
					int iUnitTest = atoi(sunitcode.c_str());	//gives us 128 devices we can control, should be enough!
					if (
						(sunitcode.empty()) ||
						(sgroupcode.empty()) ||
						((iUnitTest < 1) || (iUnitTest > 128))
						)
						return;
					sunitcode = sgroupcode;//Button A/B
					CDomoticzHardwareBase *pBaseHardware = reinterpret_cast<CDomoticzHardwareBase*>(m_mainworker.GetHardware(atoi(hwdid.c_str())));
					if (pBaseHardware == NULL)
						return;
					if ((pBaseHardware->HwdType != HTYPE_EnOceanESP2) && (pBaseHardware->HwdType != HTYPE_EnOceanESP3)
						&& (pBaseHardware->HwdType != HTYPE_USBtinGateway) )
						return;
					unsigned long rID = 0;
					//convert to hex, and we have our ID
					std::stringstream s_strid;
					s_strid << std::hex << std::uppercase << rID;
					devid = s_strid.str();
				}
				else if (lighttype == 68)
				{
#ifdef WITH_GPIO
					dtype = pTypeLighting1;
					subtype = sTypeIMPULS;
					devid = "0";
					sunitcode = request::findValue(&req, "unitcode"); //Unit code = GPIO number

					if (sunitcode.empty()) {
						return;
					}
					CGpio *pGpio = reinterpret_cast<CGpio *>(m_mainworker.GetHardware(atoi(hwdid.c_str())));
					if (pGpio == NULL) {
						return;
					}
					if (pGpio->HwdType != HTYPE_RaspberryGPIO) {
						return;
					}
					CGpioPin *pPin = CGpio::GetPPinById(atoi(sunitcode.c_str()));
					if (pPin == NULL) {
						return;
			}
#else
					return;
#endif
				}
				else if (lighttype == 69)
				{
#ifdef WITH_GPIO
					dtype = pTypeLighting2;
					subtype = sTypeAC;
					devid = "0";
					sunitcode = request::findValue(&req, "unitcode"); // sysfs-gpio number
					int unitcode = atoi(sunitcode.c_str());
					std::string sswitchtype = request::findValue(&req, "switchtype");
					_eSwitchType switchtype = (_eSwitchType)atoi(sswitchtype.c_str());
					std::string id = request::findValue(&req, "id");
					CSysfsGpio::RequestDbUpdate(unitcode);

					if ((id.empty()) || (sunitcode.empty()))
					{
						return;
					}
					devid = id;

					CSysfsGpio *pSysfsGpio = reinterpret_cast<CSysfsGpio *>(m_mainworker.GetHardware(atoi(hwdid.c_str())));

					if ((pSysfsGpio == NULL) || (pSysfsGpio->HwdType != HTYPE_SysfsGpio))
					{
						return;
					}
#else
					return;
#endif
				}
				else if (lighttype < 20)
				{
					dtype = pTypeLighting1;
					subtype = lighttype;
					std::string shousecode = request::findValue(&req, "housecode");
					sunitcode = request::findValue(&req, "unitcode");
					if (
						(shousecode.empty()) ||
						(sunitcode.empty())
						)
						return;
					devid = shousecode;
				}
				else if (lighttype < 30)
				{
					dtype = pTypeLighting2;
					subtype = lighttype - 20;
					std::string id = request::findValue(&req, "id");
					sunitcode = request::findValue(&req, "unitcode");
					if (
						(id.empty()) ||
						(sunitcode.empty())
						)
						return;
					devid = id;
				}
				else if (lighttype < 70)
				{
					dtype = pTypeLighting5;
					subtype = lighttype - 50;
					if (lighttype == 59)
						subtype = sTypeIT;
					std::string id = request::findValue(&req, "id");
					sunitcode = request::findValue(&req, "unitcode");
					if (
						(id.empty()) ||
						(sunitcode.empty())
						)
						return;
					if ((subtype != sTypeEMW100) && (subtype != sTypeLivolo) && (subtype != sTypeLivolo1to10) && (subtype != sTypeRGB432W) && (subtype != sTypeLightwaveRF) && (subtype != sTypeIT))
						devid = "00" + id;
					else
						devid = id;
				}
				else if (lighttype == 101)
				{
					//Curtain Harrison
					dtype = pTypeCurtain;
					subtype = sTypeHarrison;
					std::string shousecode = request::findValue(&req, "housecode");
					sunitcode = request::findValue(&req, "unitcode");
					if (
						(shousecode.empty()) ||
						(sunitcode.empty())
						)
						return;
					devid = shousecode;
				}
				else if (lighttype == 102)
				{
					//RFY
					dtype = pTypeRFY;
					subtype = sTypeRFY;
					std::string id = request::findValue(&req, "id");
					sunitcode = request::findValue(&req, "unitcode");
					if (
						(id.empty()) ||
						(sunitcode.empty())
						)
						return;
					devid = id;
				}
				else if (lighttype == 103)
				{
					//Meiantech
					dtype = pTypeSecurity1;
					subtype = sTypeMeiantech;
					std::string id = request::findValue(&req, "id");
					if (
						(id.empty())
						)
						return;
					devid = id;
					sunitcode = "0";
				}
				else if (lighttype == 104)
				{
					//HE105
					dtype = pTypeThermostat2;
					subtype = sTypeHE105;
					sunitcode = request::findValue(&req, "unitcode");
					if (sunitcode.empty())
						return;
					//convert to hex, and we have our Unit Code
					std::stringstream s_strid;
					s_strid << std::hex << std::uppercase << sunitcode;
					int iUnitCode;
					s_strid >> iUnitCode;
					sprintf(szTmp, "%d", iUnitCode);
					sunitcode = szTmp;
					devid = "1";
				}
				else if (lighttype == 105)
				{
					//ASA
					dtype = pTypeRFY;
					subtype = sTypeASA;
					std::string id = request::findValue(&req, "id");
					sunitcode = request::findValue(&req, "unitcode");
					if (
						(id.empty()) ||
						(sunitcode.empty())
						)
						return;
					devid = id;
				}
				else if (lighttype == 106)
				{
					//Blyss
					dtype = pTypeLighting6;
					subtype = sTypeBlyss;
					std::string sgroupcode = request::findValue(&req, "groupcode");
					sunitcode = request::findValue(&req, "unitcode");
					std::string id = request::findValue(&req, "id");
					if (
						(sgroupcode.empty()) ||
						(sunitcode.empty()) ||
						(id.empty())
						)
						return;
					devid = id + sgroupcode;
				}
				else if (lighttype == 107)
				{
					//RFY2
					dtype = pTypeRFY;
					subtype = sTypeRFY2;
					std::string id = request::findValue(&req, "id");
					sunitcode = request::findValue(&req, "unitcode");
					if (
						(id.empty()) ||
						(sunitcode.empty())
						)
						return;
					devid = id;
				}
				else
				{
					if (lighttype == 100)
					{
						//Chime/ByronSX
						dtype = pTypeChime;
						subtype = sTypeByronSX;
						std::string id = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(sunitcode.empty())
							)
							return;
						int iUnitCode = atoi(sunitcode.c_str()) - 1;
						switch (iUnitCode)
						{
						case 0:
							iUnitCode = chime_sound0;
							break;
						case 1:
							iUnitCode = chime_sound1;
							break;
						case 2:
							iUnitCode = chime_sound2;
							break;
						case 3:
							iUnitCode = chime_sound3;
							break;
						case 4:
							iUnitCode = chime_sound4;
							break;
						case 5:
							iUnitCode = chime_sound5;
							break;
						case 6:
							iUnitCode = chime_sound6;
							break;
						case 7:
							iUnitCode = chime_sound7;
							break;
						}
						sprintf(szTmp, "%d", iUnitCode);
						sunitcode = szTmp;
						devid = id;
					}
					else if ((lighttype >= 200) && (lighttype < 300))
					{
						dtype = pTypeBlinds;
						subtype = lighttype - 200;
						std::string id = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(sunitcode.empty())
							)
							return;
						int iUnitCode = atoi(sunitcode.c_str());
						sprintf(szTmp, "%d", iUnitCode);
						sunitcode = szTmp;
						devid = id;
					}
					else if (lighttype == 301)
					{
						//Smartwares Radiator
						std::string id = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(sunitcode.empty())
							)
							return;
						devid = id;

						//For this device, we will also need to add a Radiator type, do that first
						dtype = pTypeRadiator1;
						subtype = sTypeSmartwares;

						//check if switch is unique
						result = m_sql.safe_query(
							"SELECT Name FROM DeviceStatus WHERE (HardwareID=='%q' AND DeviceID=='%q' AND Unit=='%q' AND Type==%d AND SubType==%d)",
							hwdid.c_str(), devid.c_str(), sunitcode.c_str(), dtype, subtype);
						if (!result.empty())
						{
							root["message"] = "Switch already exists!";
							return;
						}
						bool bActEnabledState = m_sql.m_bAcceptNewHardware;
						m_sql.m_bAcceptNewHardware = true;
						std::string devname;
						//m_sql.UpdateValue(atoi(hwdid.c_str()), devid.c_str(), atoi(sunitcode.c_str()), dtype, subtype, 0, -1, 0, "20.5", devname);
						m_sql.m_bAcceptNewHardware = bActEnabledState;

						//set name and switchtype
						result = m_sql.safe_query(
							"SELECT ID FROM DeviceStatus WHERE (HardwareID=='%q' AND DeviceID=='%q' AND Unit=='%q' AND Type==%d AND SubType==%d)",
							hwdid.c_str(), devid.c_str(), sunitcode.c_str(), dtype, subtype);
						if (result.empty())
						{
							root["message"] = "Error finding switch in Database!?!?";
							return;
						}
						std::string ID = result[0][0];

						m_sql.safe_query(
							"UPDATE DeviceStatus SET Used=1, Name='%q', SwitchType=%d WHERE (ID == '%q')",
							name.c_str(), switchtype, ID.c_str());

						//Now continue to insert the switch
						dtype = pTypeRadiator1;
						subtype = sTypeSmartwaresSwitchRadiator;
					}
					else if (lighttype == 302)
					{
						//Home Confort
						dtype = pTypeHomeConfort;
						subtype = sTypeHomeConfortTEL010;
						std::string id = request::findValue(&req, "id");

						std::string shousecode = request::findValue(&req, "housecode");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(shousecode.empty()) ||
							(sunitcode.empty())
							)
							return;

						int iUnitCode = atoi(sunitcode.c_str());
						sprintf(szTmp, "%d", iUnitCode);
						sunitcode = szTmp;
						sprintf(szTmp, "%02X", atoi(shousecode.c_str()));
						shousecode = szTmp;
						devid = id + shousecode;
					}
					else if (lighttype == 303)
					{
						//Selector Switch
						dtype = pTypeGeneralSwitch;
						subtype = sSwitchTypeSelector;
						std::string id = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(id.empty()) ||
							(sunitcode.empty())
							)
							return;
						devid = "0" + id;
						switchtype = STYPE_Selector;
						if (!deviceoptions.empty()) {
							deviceoptions.append(";");
						}
						deviceoptions.append("SelectorStyle:0;LevelNames:Off|Level1|Level2|Level3");
					}
					else if (lighttype == 304)
					{
						//Itho CVE RFT
						dtype = pTypeFan;
						subtype = sTypeItho;
						std::string id = request::findValue(&req, "id");
						if (id.empty())
							return;
						devid = id;
						sunitcode = "0";
					}
					else if (lighttype == 305)
					{
						//Lucci Air
						dtype = pTypeFan;
						subtype = sTypeLucciAir;
						std::string id = request::findValue(&req, "id");
						if (id.empty())
							return;
						devid = id;
						sunitcode = "0";
					}
					else if (lighttype == 306)
					{
						//Lucci Air DC
						dtype = pTypeFan;
						subtype = sTypeLucciAirDC;
						std::string id = request::findValue(&req, "id");
						if (id.empty())
							return;
						devid = id;
						sunitcode = "0";
					}
					else if (lighttype == 307)
					{
						//Westinghouse
						dtype = pTypeFan;
						subtype = sTypeWestinghouse;
						std::string id = request::findValue(&req, "id");
						if (id.empty())
							return;
						devid = id;
						sunitcode = "0";
					}
					else if (lighttype == 400)
					{
						//Openwebnet Bus Blinds
						dtype = pTypeLighting2;
						subtype = sTypeAC;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if (lighttype == 401)
					{
						//Openwebnet Bus Lights
						dtype = pTypeLighting2;
						subtype = sTypeAC;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if (lighttype == 402)
					{
						//Openwebnet Bus Auxiliary
						dtype = pTypeLighting2;
						subtype = sTypeAC;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if (lighttype == 403)
					{
						//Openwebnet Zigbee Blinds
						dtype = pTypeGeneralSwitch;
						subtype = sSwitchBlindsT2;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if (lighttype == 404)
					{
						//Openwebnet Zigbee Lights
						dtype = pTypeGeneralSwitch;
						subtype = sSwitchLightT2;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if ((lighttype == 405) || (lighttype == 406))
					{
						//Openwebnet Dry Contact / IR Detection
						dtype = pTypeGeneralSwitch;
						subtype = sSwitchContactT1;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						if (
							(devid.empty()) ||
							(sunitcode.empty())
							)
							return;
					}
					else if (lighttype == 407) {
						//Openwebnet Bus Custom
						dtype = pTypeLighting2;
						subtype = sTypeAC;
						devid = request::findValue(&req, "id");
						sunitcode = request::findValue(&req, "unitcode");
						StrParam1 = request::findValue(&req, "StrParam1");
						_log.Log(LOG_STATUS, "COpenWebNetTCP: custom command: '%s'", StrParam1.c_str());
						if (
							(devid.empty()) ||
							(sunitcode.empty()) ||
							(StrParam1.empty())
							)
						{
							root["message"] = "Some field empty or not valid.";
							return;
						}
					}
				}

				//check if switch is unique
				result = m_sql.safe_query(
					"SELECT Name FROM DeviceStatus WHERE (HardwareID=='%q' AND DeviceID=='%q' AND Unit=='%q' AND Type==%d AND SubType==%d)",
					hwdid.c_str(), devid.c_str(), sunitcode.c_str(), dtype, subtype);
				if (!result.empty())
				{
					root["message"] = "Switch already exists!";
					return;
				}

				// ----------- If needed convert to GeneralSwitch type (for o.a. RFlink) -----------
				CDomoticzHardwareBase *pBaseHardware = m_mainworker.GetHardware(atoi(hwdid.c_str()));
				if (pBaseHardware != NULL)
				{
					if ((pBaseHardware->HwdType == HTYPE_RFLINKUSB) || (pBaseHardware->HwdType == HTYPE_RFLINKTCP)) {
						ConvertToGeneralSwitchType(devid, dtype, subtype);
					}
				}
				// -----------------------------------------------

				bool bActEnabledState = m_sql.m_bAcceptNewHardware;
				m_sql.m_bAcceptNewHardware = true;
				std::string devname;
				//m_sql.UpdateValue(atoi(hwdid.c_str()), devid.c_str(), atoi(sunitcode.c_str()), dtype, subtype, 0, -1, 0, devname);
				m_sql.m_bAcceptNewHardware = bActEnabledState;

				//set name and switchtype
				result = m_sql.safe_query(
					"SELECT ID FROM DeviceStatus WHERE (HardwareID=='%q' AND DeviceID=='%q' AND Unit=='%q' AND Type==%d AND SubType==%d)",
					hwdid.c_str(), devid.c_str(), sunitcode.c_str(), dtype, subtype);
				if (result.empty())
				{
					root["message"] = "Error finding switch in Database!?!?";
					return;
				}
				std::string ID = result[0][0];

				m_sql.safe_query(
					"UPDATE DeviceStatus SET Used=1, Name='%q', SwitchType=%d WHERE (ID == '%q')",
					name.c_str(), switchtype, ID.c_str());

				if (lighttype == 407) {
					//Openwebnet Bus Custom
					m_sql.safe_query(
						"UPDATE DeviceStatus SET StrParam1='%s' WHERE (ID == '%q')", 
						StrParam1.c_str(), ID.c_str());
				}

				m_mainworker.m_eventsystem.GetCurrentStates();

				//Set device options
				m_sql.SetDeviceOptions(atoi(ID.c_str()), m_sql.BuildDeviceOptions(deviceoptions, false));

				if (maindeviceidx != "")
				{
					if (maindeviceidx != ID)
					{
						//this is a sub device for another light/switch
						//first check if it is not already a sub device
						result = m_sql.safe_query(
							"SELECT ID FROM LightSubDevices WHERE (DeviceRowID=='%q') AND (ParentID =='%q')",
							ID.c_str(), maindeviceidx.c_str());
						if (result.empty())
						{
							//no it is not, add it
							result = m_sql.safe_query(
								"INSERT INTO LightSubDevices (DeviceRowID, ParentID) VALUES ('%q','%q')",
								ID.c_str(),
								maindeviceidx.c_str()
							);
						}
					}
				}

				root["status"] = "OK";
				root["title"] = "AddSwitch";
			}
			else if (cparam == "getnotificationtypes")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				//First get Device Type/SubType
				result = m_sql.safe_query("SELECT Type, SubType, SwitchType FROM DeviceStatus WHERE (ID == '%q')",
					idx.c_str());
				if (result.empty())
					return;

				root["status"] = "OK";
				root["title"] = "GetNotificationTypes";
				unsigned char dType = atoi(result[0][0].c_str());
				unsigned char dSubType = atoi(result[0][1].c_str());
				unsigned char switchtype = atoi(result[0][2].c_str());

				int ii = 0;
				if (
					(dType == pTypeLighting1) ||
					(dType == pTypeLighting2) ||
					(dType == pTypeLighting3) ||
					(dType == pTypeLighting4) ||
					(dType == pTypeLighting5) ||
					(dType == pTypeLighting6) ||
					(dType == pTypeColorSwitch) ||
					(dType == pTypeSecurity1) ||
					(dType == pTypeSecurity2) ||
					(dType == pTypeEvohome) ||
					(dType == pTypeEvohomeRelay) ||
					(dType == pTypeCurtain) ||
					(dType == pTypeBlinds) ||
					(dType == pTypeRFY) ||
					(dType == pTypeChime) ||
					(dType == pTypeThermostat2) ||
					(dType == pTypeThermostat3) ||
					(dType == pTypeThermostat4) ||
					(dType == pTypeRemote) ||
					(dType == pTypeGeneralSwitch) ||
					(dType == pTypeHomeConfort) ||
					(dType == pTypeFS20) ||
					((dType == pTypeRadiator1) && (dSubType == sTypeSmartwaresSwitchRadiator))
					)
				{
					if (switchtype != STYPE_PushOff)
					{
						root["result"][ii]["val"] = NTYPE_SWITCH_ON;
						root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_SWITCH_ON, 0);
						root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_SWITCH_ON, 1);
						ii++;
					}
					if (switchtype != STYPE_PushOn)
					{
						root["result"][ii]["val"] = NTYPE_SWITCH_OFF;
						root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_SWITCH_OFF, 0);
						root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_SWITCH_OFF, 1);
						ii++;
					}
					if (switchtype == STYPE_Media)
					{
						std::string idx = request::findValue(&req, "idx");

						result = m_sql.safe_query("SELECT HardwareID FROM DeviceStatus WHERE (ID=='%q')", idx.c_str());
						if (!result.empty())
						{
							std::string hdwid = result[0][0];
							CDomoticzHardwareBase *pBaseHardware = reinterpret_cast<CDomoticzHardwareBase*>(m_mainworker.GetHardware(atoi(hdwid.c_str())));
							if (pBaseHardware != NULL) {
								_eHardwareTypes type = pBaseHardware->HwdType;
								root["result"][ii]["val"] = NTYPE_PAUSED;
								root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_PAUSED, 0);
								root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_PAUSED, 1);
								ii++;
								if (type == HTYPE_Kodi) {
									root["result"][ii]["val"] = NTYPE_AUDIO;
									root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_AUDIO, 0);
									root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_AUDIO, 1);
									ii++;
									root["result"][ii]["val"] = NTYPE_VIDEO;
									root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_VIDEO, 0);
									root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_VIDEO, 1);
									ii++;
									root["result"][ii]["val"] = NTYPE_PHOTO;
									root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_PHOTO, 0);
									root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_PHOTO, 1);
									ii++;
								}
								if (type == HTYPE_LogitechMediaServer) {
									root["result"][ii]["val"] = NTYPE_PLAYING;
									root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_PLAYING, 0);
									root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_PLAYING, 1);
									ii++;
									root["result"][ii]["val"] = NTYPE_STOPPED;
									root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_STOPPED, 0);
									root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_STOPPED, 1);
									ii++;
								}
								if (type == HTYPE_HEOS) {
									root["result"][ii]["val"] = NTYPE_PLAYING;
									root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_PLAYING, 0);
									root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_PLAYING, 1);
									ii++;
									root["result"][ii]["val"] = NTYPE_STOPPED;
									root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_STOPPED, 0);
									root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_STOPPED, 1);
									ii++;
								}
							}
						}
					}
				}
				if (
					(
					(dType == pTypeTEMP) ||
						(dType == pTypeTEMP_HUM) ||
						(dType == pTypeTEMP_HUM_BARO) ||
						(dType == pTypeTEMP_BARO) ||
						(dType == pTypeEvohomeZone) ||
						(dType == pTypeEvohomeWater) ||
						(dType == pTypeThermostat1) ||
						(dType == pTypeRego6XXTemp) ||
						((dType == pTypeRFXSensor) && (dSubType == sTypeRFXSensorTemp))
						) ||
						((dType == pTypeUV) && (dSubType == sTypeUV3)) ||
					((dType == pTypeWIND) && (dSubType == sTypeWIND4)) ||
					((dType == pTypeWIND) && (dSubType == sTypeWINDNoTemp)) ||
					((dType == pTypeGeneral) && (dSubType == sTypeSystemTemp))
					)
				{
					root["result"][ii]["val"] = NTYPE_TEMPERATURE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_TEMPERATURE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_TEMPERATURE, 1);
					ii++;
				}
				if (
					(dType == pTypeHUM) ||
					(dType == pTypeTEMP_HUM) ||
					(dType == pTypeTEMP_HUM_BARO)
					)
				{
					root["result"][ii]["val"] = NTYPE_HUMIDITY;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_HUMIDITY, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_HUMIDITY, 1);
					ii++;
				}
				if (
					(dType == pTypeTEMP_HUM) ||
					(dType == pTypeTEMP_HUM_BARO)
					)
				{
					root["result"][ii]["val"] = NTYPE_DEWPOINT;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_DEWPOINT, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_DEWPOINT, 1);
					ii++;
				}
				if (dType == pTypeRAIN)
				{
					root["result"][ii]["val"] = NTYPE_RAIN;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_RAIN, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_RAIN, 1);
					ii++;
				}
				if (dType == pTypeWIND)
				{
					root["result"][ii]["val"] = NTYPE_WIND;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_WIND, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_WIND, 1);
					ii++;
				}
				if (dType == pTypeUV)
				{
					root["result"][ii]["val"] = NTYPE_UV;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_UV, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_UV, 1);
					ii++;
				}
				if (
					(dType == pTypeTEMP_HUM_BARO) ||
					(dType == pTypeBARO) ||
					(dType == pTypeTEMP_BARO)
					)
				{
					root["result"][ii]["val"] = NTYPE_BARO;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_BARO, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_BARO, 1);
					ii++;
				}
				if (
					((dType == pTypeRFXMeter) && (dSubType == sTypeRFXMeterCount)) ||
					((dType == pTypeGeneral) && (dSubType == sTypeCounterIncremental)) ||
					(dType == pTypeYouLess) ||
					((dType == pTypeRego6XXValue) && (dSubType == sTypeRego6XXCounter))
					)
				{
					if ((switchtype == MTYPE_ENERGY) || (switchtype == MTYPE_ENERGY_GENERATED))
					{
						root["result"][ii]["val"] = NTYPE_TODAYENERGY;
						root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_TODAYENERGY, 0);
						root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_TODAYENERGY, 1);
					}
					else if (switchtype == MTYPE_GAS)
					{
						root["result"][ii]["val"] = NTYPE_TODAYGAS;
						root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_TODAYGAS, 0);
						root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_TODAYGAS, 1);
					}
					else if (switchtype == MTYPE_COUNTER)
					{
						root["result"][ii]["val"] = NTYPE_TODAYCOUNTER;
						root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_TODAYCOUNTER, 0);
						root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_TODAYCOUNTER, 1);
					}
					else
					{
						//water (same as gas)
						root["result"][ii]["val"] = NTYPE_TODAYGAS;
						root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_TODAYGAS, 0);
						root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_TODAYGAS, 1);
					}
					ii++;
				}
				if (dType == pTypeYouLess)
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if (dType == pTypeAirQuality)
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				else if ((dType == pTypeGeneral) && ((dSubType == sTypeSoilMoisture) || (dSubType == sTypeLeafWetness)))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeVisibility))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeDistance))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeSolarRadiation))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeVoltage))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeCurrent))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypePressure))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeBaro))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if (dType == pTypeLux)
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeSoundLevel))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if (dType == pTypeWEIGHT)
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if (dType == pTypeUsage)
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if (
					(dType == pTypeENERGY) ||
					((dType == pTypeGeneral) && (dSubType == sTypeKwh))
					)
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if (dType == pTypePOWER)
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeCURRENT) && (dSubType == sTypeELEC1))
				{
					root["result"][ii]["val"] = NTYPE_AMPERE1;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_AMPERE1, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_AMPERE1, 1);
					ii++;
					root["result"][ii]["val"] = NTYPE_AMPERE2;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_AMPERE2, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_AMPERE2, 1);
					ii++;
					root["result"][ii]["val"] = NTYPE_AMPERE3;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_AMPERE3, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_AMPERE3, 1);
					ii++;
				}
				if ((dType == pTypeCURRENTENERGY) && (dSubType == sTypeELEC4))
				{
					root["result"][ii]["val"] = NTYPE_AMPERE1;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_AMPERE1, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_AMPERE1, 1);
					ii++;
					root["result"][ii]["val"] = NTYPE_AMPERE2;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_AMPERE2, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_AMPERE2, 1);
					ii++;
					root["result"][ii]["val"] = NTYPE_AMPERE3;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_AMPERE3, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_AMPERE3, 1);
					ii++;
				}
				if (dType == pTypeP1Power)
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
					root["result"][ii]["val"] = NTYPE_TODAYENERGY;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_TODAYENERGY, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_TODAYENERGY, 1);
					ii++;
				}
				if (dType == pTypeP1Gas)
				{
					root["result"][ii]["val"] = NTYPE_TODAYGAS;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_TODAYGAS, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_TODAYGAS, 1);
					ii++;
				}
				if ((dType == pTypeThermostat) && (dSubType == sTypeThermSetpoint))
				{
					root["result"][ii]["val"] = NTYPE_TEMPERATURE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_TEMPERATURE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_TEMPERATURE, 1);
					ii++;
				}
				if (dType == pTypeEvohomeZone)
				{
					root["result"][ii]["val"] = NTYPE_TEMPERATURE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_SETPOINT, 0); //FIXME NTYPE_SETPOINT implementation?
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_SETPOINT, 1);
					ii++;
				}
				if ((dType == pTypeRFXSensor) && ((dSubType == sTypeRFXSensorAD) || (dSubType == sTypeRFXSensorVolt)))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypePercentage))
				{
					root["result"][ii]["val"] = NTYPE_PERCENTAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_PERCENTAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_PERCENTAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeWaterflow))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeCustom))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeFan))
				{
					root["result"][ii]["val"] = NTYPE_RPM;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_RPM, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_RPM, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeAlert))
				{
					root["result"][ii]["val"] = NTYPE_USAGE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_USAGE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_USAGE, 1);
					ii++;
				}
				if ((dType == pTypeGeneral) && (dSubType == sTypeZWaveAlarm))
				{
					root["result"][ii]["val"] = NTYPE_VALUE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_VALUE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_VALUE, 1);
					ii++;
				}
				if ((dType == pTypeRego6XXValue) && (dSubType == sTypeRego6XXStatus))
				{
					root["result"][ii]["val"] = NTYPE_SWITCH_ON;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_SWITCH_ON, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_SWITCH_ON, 1);
					ii++;
					root["result"][ii]["val"] = NTYPE_SWITCH_OFF;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_SWITCH_OFF, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_SWITCH_OFF, 1);
					ii++;
				}
				if (!IsLightOrSwitch(dType, dSubType))
				{
					root["result"][ii]["val"] = NTYPE_LASTUPDATE;
					root["result"][ii]["text"] = Notification_Type_Desc(NTYPE_LASTUPDATE, 0);
					root["result"][ii]["ptag"] = Notification_Type_Desc(NTYPE_LASTUPDATE, 1);
					ii++;
				}
			}
			else if (cparam == "addnotification")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;

				std::string stype = request::findValue(&req, "ttype");
				std::string swhen = request::findValue(&req, "twhen");
				std::string svalue = request::findValue(&req, "tvalue");
				std::string scustommessage = request::findValue(&req, "tmsg");
				std::string sactivesystems = request::findValue(&req, "tsystems");
				std::string spriority = request::findValue(&req, "tpriority");
				std::string ssendalways = request::findValue(&req, "tsendalways");
				std::string srecovery = (request::findValue(&req, "trecovery") == "true") ? "1" : "0";

				if ((stype.empty()) || (swhen.empty()) || (svalue.empty()) || (spriority.empty()) || (ssendalways.empty()) || (srecovery.empty()))
					return;

				_eNotificationTypes ntype = (_eNotificationTypes)atoi(stype.c_str());
				std::string ttype = Notification_Type_Desc(ntype, 1);
				if (
					(ntype == NTYPE_SWITCH_ON) ||
					(ntype == NTYPE_SWITCH_OFF) ||
					(ntype == NTYPE_DEWPOINT)
					)
				{
					if ((ntype == NTYPE_SWITCH_ON) && (swhen == "2")) { // '='
						unsigned char twhen = '=';
						sprintf(szTmp, "%s;%c;%s", ttype.c_str(), twhen, svalue.c_str());
					}
					else
						strcpy(szTmp, ttype.c_str());
				}
				else
				{
					std::string twhen;
					if (swhen == "0")
						twhen = ">";
					else if (swhen == "1")
						twhen = ">=";
					else if (swhen == "2")
						twhen = "=";
					else if (swhen == "3")
						twhen = "!=";
					else if (swhen == "4")
						twhen = "<=";
					else
						twhen = "<";
					sprintf(szTmp, "%s;%s;%s;%s", ttype.c_str(), twhen.c_str(), svalue.c_str(), srecovery.c_str());
				}
				int priority = atoi(spriority.c_str());
				bool bOK = m_notifications.AddNotification(idx, szTmp, scustommessage, sactivesystems, priority, (ssendalways == "true") ? true : false);
				if (bOK) {
					root["status"] = "OK";
					root["title"] = "AddNotification";
				}
			}
			else if (cparam == "updatenotification")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				std::string devidx = request::findValue(&req, "devidx");
				if ((idx.empty()) || (devidx.empty()))
					return;

				std::string stype = request::findValue(&req, "ttype");
				std::string swhen = request::findValue(&req, "twhen");
				std::string svalue = request::findValue(&req, "tvalue");
				std::string scustommessage = request::findValue(&req, "tmsg");
				std::string sactivesystems = request::findValue(&req, "tsystems");
				std::string spriority = request::findValue(&req, "tpriority");
				std::string ssendalways = request::findValue(&req, "tsendalways");
				std::string srecovery = (request::findValue(&req, "trecovery") == "true") ? "1" : "0";

				if ((stype.empty()) || (swhen.empty()) || (svalue.empty()) || (spriority.empty()) || (ssendalways.empty()) || srecovery.empty())
					return;
				root["status"] = "OK";
				root["title"] = "UpdateNotification";

				std::string recoverymsg;
				if ((srecovery == "1") && (m_notifications.CustomRecoveryMessage(strtoull(idx.c_str(), NULL, 0), recoverymsg, true)))
				{
					scustommessage.append(";;");
					scustommessage.append(recoverymsg);
				}
				//delete old record
				m_notifications.RemoveNotification(idx);

				_eNotificationTypes ntype = (_eNotificationTypes)atoi(stype.c_str());
				std::string ttype = Notification_Type_Desc(ntype, 1);
				if (
					(ntype == NTYPE_SWITCH_ON) ||
					(ntype == NTYPE_SWITCH_OFF) ||
					(ntype == NTYPE_DEWPOINT)
					)
				{
					if ((ntype == NTYPE_SWITCH_ON) && (swhen == "2")) { // '='
						unsigned char twhen = '=';
						sprintf(szTmp, "%s;%c;%s", ttype.c_str(), twhen, svalue.c_str());
					}
					else
						strcpy(szTmp, ttype.c_str());
				}
				else
				{
					std::string twhen;
					if (swhen == "0")
						twhen = ">";
					else if (swhen == "1")
						twhen = ">=";
					else if (swhen == "2")
						twhen = "=";
					else if (swhen == "3")
						twhen = "!=";
					else if (swhen == "4")
						twhen = "<=";
					else
						twhen = "<";
					sprintf(szTmp, "%s;%s;%s;%s", ttype.c_str(), twhen.c_str(), svalue.c_str(), srecovery.c_str());
				}
				int priority = atoi(spriority.c_str());
				m_notifications.AddNotification(devidx, szTmp, scustommessage, sactivesystems, priority, (ssendalways == "true") ? true : false);
			}
			else if (cparam == "deletenotification")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;

				root["status"] = "OK";
				root["title"] = "DeleteNotification";

				m_notifications.RemoveNotification(idx);
			}
			else if (cparam == "switchdeviceorder")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx1 = request::findValue(&req, "idx1");
				std::string idx2 = request::findValue(&req, "idx2");
				if ((idx1.empty()) || (idx2.empty()))
					return;
				std::string sroomid = request::findValue(&req, "roomid");
				int roomid = atoi(sroomid.c_str());

				std::string Order1, Order2;
				if (roomid == 0)
				{
					//get device order 1
					result = m_sql.safe_query("SELECT [Order] FROM DeviceStatus WHERE (ID == '%q')",
						idx1.c_str());
					if (result.empty())
						return;
					Order1 = result[0][0];

					//get device order 2
					result = m_sql.safe_query("SELECT [Order] FROM DeviceStatus WHERE (ID == '%q')",
						idx2.c_str());
					if (result.empty())
						return;
					Order2 = result[0][0];

					root["status"] = "OK";
					root["title"] = "SwitchDeviceOrder";

					if (atoi(Order1.c_str()) < atoi(Order2.c_str()))
					{
						m_sql.safe_query(
							"UPDATE DeviceStatus SET [Order] = [Order]+1 WHERE ([Order] >= '%q' AND [Order] < '%q')",
							Order1.c_str(), Order2.c_str());
					}
					else
					{
						m_sql.safe_query(
							"UPDATE DeviceStatus SET [Order] = [Order]-1 WHERE ([Order] > '%q' AND [Order] <= '%q')",
							Order2.c_str(), Order1.c_str());
					}

					m_sql.safe_query("UPDATE DeviceStatus SET [Order] = '%q' WHERE (ID == '%q')",
						Order1.c_str(), idx2.c_str());
				}
				else
				{
					//change order in a room
					//get device order 1
					result = m_sql.safe_query("SELECT [Order] FROM DeviceToPlansMap WHERE (DeviceRowID == '%q') AND (PlanID==%d)",
						idx1.c_str(), roomid);
					if (result.empty())
						return;
					Order1 = result[0][0];

					//get device order 2
					result = m_sql.safe_query("SELECT [Order] FROM DeviceToPlansMap WHERE (DeviceRowID == '%q') AND (PlanID==%d)",
						idx2.c_str(), roomid);
					if (result.empty())
						return;
					Order2 = result[0][0];

					root["status"] = "OK";
					root["title"] = "SwitchDeviceOrder";

					if (atoi(Order1.c_str()) < atoi(Order2.c_str()))
					{
						m_sql.safe_query(
							"UPDATE DeviceToPlansMap SET [Order] = [Order]+1 WHERE ([Order] >= '%q' AND [Order] < '%q') AND (PlanID==%d)",
							Order1.c_str(), Order2.c_str(), roomid);
					}
					else
					{
						m_sql.safe_query(
							"UPDATE DeviceToPlansMap SET [Order] = [Order]-1 WHERE ([Order] > '%q' AND [Order] <= '%q') AND (PlanID==%d)",
							Order2.c_str(), Order1.c_str(), roomid);
					}

					m_sql.safe_query("UPDATE DeviceToPlansMap SET [Order] = '%q' WHERE (DeviceRowID == '%q') AND (PlanID==%d)",
						Order1.c_str(), idx2.c_str(), roomid);
				}
			}
			else if (cparam == "switchsceneorder")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx1 = request::findValue(&req, "idx1");
				std::string idx2 = request::findValue(&req, "idx2");
				if ((idx1.empty()) || (idx2.empty()))
					return;

				std::string Order1, Order2;
				//get device order 1
				result = m_sql.safe_query("SELECT [Order] FROM Scenes WHERE (ID == '%q')",
					idx1.c_str());
				if (result.empty())
					return;
				Order1 = result[0][0];

				//get device order 2
				result = m_sql.safe_query("SELECT [Order] FROM Scenes WHERE (ID == '%q')",
					idx2.c_str());
				if (result.empty())
					return;
				Order2 = result[0][0];

				root["status"] = "OK";
				root["title"] = "SwitchSceneOrder";

				if (atoi(Order1.c_str()) < atoi(Order2.c_str()))
				{
					m_sql.safe_query(
						"UPDATE Scenes SET [Order] = [Order]+1 WHERE ([Order] >= '%q' AND [Order] < '%q')",
						Order1.c_str(), Order2.c_str());
				}
				else
				{
					m_sql.safe_query(
						"UPDATE Scenes SET [Order] = [Order]-1 WHERE ([Order] > '%q' AND [Order] <= '%q')",
						Order2.c_str(), Order1.c_str());
				}

				m_sql.safe_query("UPDATE Scenes SET [Order] = '%q' WHERE (ID == '%q')",
					Order1.c_str(), idx2.c_str());
			}
			else if (cparam == "clearnotifications")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;

				root["status"] = "OK";
				root["title"] = "ClearNotification";

				m_notifications.RemoveDeviceNotifications(idx);
			}
			else if (cparam == "adduser")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string senabled = request::findValue(&req, "enabled");
				std::string username = request::findValue(&req, "username");
				std::string password = request::findValue(&req, "password");
				std::string srights = request::findValue(&req, "rights");
				std::string sRemoteSharing = request::findValue(&req, "RemoteSharing");
				std::string sTabsEnabled = request::findValue(&req, "TabsEnabled");
				if (
					(senabled.empty()) ||
					(username.empty()) ||
					(password.empty()) ||
					(srights.empty()) ||
					(sRemoteSharing.empty()) ||
					(sTabsEnabled.empty())
					)
					return;
				int rights = atoi(srights.c_str());
				if (rights != 2)
				{
					if (!FindAdminUser())
					{
						root["message"] = "Add a Admin user first! (Or enable Settings/Website Protection)";
						return;
					}
				}
				root["status"] = "OK";
				root["title"] = "AddUser";
				m_sql.safe_query(
					"INSERT INTO Users (Active, Username, Password, Rights, RemoteSharing, TabsEnabled) VALUES (%d,'%q','%q','%d','%d','%d')",
					(senabled == "true") ? 1 : 0,
					base64_encode(username).c_str(),
					password.c_str(),
					rights,
					(sRemoteSharing == "true") ? 1 : 0,
					atoi(sTabsEnabled.c_str())
				);
				LoadUsers();
			}
			else if (cparam == "updateuser")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				std::string senabled = request::findValue(&req, "enabled");
				std::string username = request::findValue(&req, "username");
				std::string password = request::findValue(&req, "password");
				std::string srights = request::findValue(&req, "rights");
				std::string sRemoteSharing = request::findValue(&req, "RemoteSharing");
				std::string sTabsEnabled = request::findValue(&req, "TabsEnabled");
				if (
					(senabled.empty()) ||
					(username.empty()) ||
					(password.empty()) ||
					(srights.empty()) ||
					(sRemoteSharing.empty()) ||
					(sTabsEnabled.empty())
					)
					return;
				int rights = atoi(srights.c_str());
				if (rights != 2)
				{
					if (!FindAdminUser())
					{
						root["message"] = "Add a Admin user first! (Or enable Settings/Website Protection)";
						return;
					}
				}
				std::string sHashedUsername = base64_encode(username);

				// Invalid user's sessions if username or password has changed
				std::string sOldUsername;
				std::string sOldPassword;
				result = m_sql.safe_query("SELECT Username, Password FROM Users WHERE (ID == '%q')", idx.c_str());
				if (result.size() == 1)
				{
					sOldUsername = result[0][0];
					sOldPassword = result[0][1];
				}
				if ((sHashedUsername != sOldUsername) || (password != sOldPassword))
					RemoveUsersSessions(sOldUsername, session);

				root["status"] = "OK";
				root["title"] = "UpdateUser";
				m_sql.safe_query(
					"UPDATE Users SET Active=%d, Username='%q', Password='%q', Rights=%d, RemoteSharing=%d, TabsEnabled=%d WHERE (ID == '%q')",
					(senabled == "true") ? 1 : 0,
					sHashedUsername.c_str(),
					password.c_str(),
					rights,
					(sRemoteSharing == "true") ? 1 : 0,
					atoi(sTabsEnabled.c_str()),
					idx.c_str()
				);
				LoadUsers();


			}
			else if (cparam == "deleteuser")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;

				root["status"] = "OK";
				root["title"] = "DeleteUser";

				// Remove user's sessions
				result = m_sql.safe_query("SELECT Username FROM Users WHERE (ID == '%q')", idx.c_str());
				if (result.size() == 1)
				{
					RemoveUsersSessions(result[0][0], session);
				}

				m_sql.safe_query("DELETE FROM Users WHERE (ID == '%q')", idx.c_str());

				m_sql.safe_query("DELETE FROM SharedDevices WHERE (SharedUserID == '%q')", idx.c_str());

				LoadUsers();
			}
			else if (cparam == "clearlightlog")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				//First get Device Type/SubType
				result = m_sql.safe_query("SELECT Type, SubType FROM DeviceStatus WHERE (ID == '%q')",
					idx.c_str());
				if (result.empty())
					return;

				unsigned char dType = atoi(result[0][0].c_str());
				unsigned char dSubType = atoi(result[0][1].c_str());

				if (
					(dType != pTypeLighting1) &&
					(dType != pTypeLighting2) &&
					(dType != pTypeLighting3) &&
					(dType != pTypeLighting4) &&
					(dType != pTypeLighting5) &&
					(dType != pTypeLighting6) &&
					(dType != pTypeFan) &&
					(dType != pTypeColorSwitch) &&
					(dType != pTypeSecurity1) &&
					(dType != pTypeSecurity2) &&
					(dType != pTypeEvohome) &&
					(dType != pTypeEvohomeRelay) &&
					(dType != pTypeCurtain) &&
					(dType != pTypeBlinds) &&
					(dType != pTypeRFY) &&
					(dType != pTypeChime) &&
					(dType != pTypeThermostat2) &&
					(dType != pTypeThermostat3) &&
					(dType != pTypeThermostat4) &&
					(dType != pTypeRemote) &&
					(dType != pTypeGeneralSwitch) &&
					(dType != pTypeHomeConfort) &&
					(dType != pTypeFS20) &&
					(!((dType == pTypeRadiator1) && (dSubType == sTypeSmartwaresSwitchRadiator))) &&
					(!((dType == pTypeGeneral) && (dSubType == sTypeTextStatus))) &&
					(!((dType == pTypeGeneral) && (dSubType == sTypeAlert))) &&
					(dType != pTypeHunter)
					)
					return; //no light device! we should not be here!

				root["status"] = "OK";
				root["title"] = "ClearLightLog";

				result = m_sql.safe_query("DELETE FROM LightingLog WHERE (DeviceRowID=='%q')", idx.c_str());
			}
			else if (cparam == "clearscenelog")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "ClearSceneLog";

				result = m_sql.safe_query("DELETE FROM SceneLog WHERE (SceneRowID=='%q')", idx.c_str());
			}
			else if (cparam == "makefavorite")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				std::string sisfavorite = request::findValue(&req, "isfavorite");
				if ((idx.empty()) || (sisfavorite.empty()))
					return;
				int isfavorite = atoi(sisfavorite.c_str());
				m_sql.safe_query("UPDATE DeviceStatus SET Favorite=%d WHERE (ID == '%q')",
					isfavorite, idx.c_str());
				root["status"] = "OK";
				root["title"] = "MakeFavorite";
			} //makefavorite
			else if (cparam == "makescenefavorite")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				std::string sisfavorite = request::findValue(&req, "isfavorite");
				if ((idx.empty()) || (sisfavorite.empty()))
					return;
				int isfavorite = atoi(sisfavorite.c_str());
				m_sql.safe_query("UPDATE Scenes SET Favorite=%d WHERE (ID == '%q')",
					isfavorite, idx.c_str());
				root["status"] = "OK";
				root["title"] = "MakeSceneFavorite";
			} //makescenefavorite
			else if (cparam == "resetsecuritystatus")
			{
				std::string idx = request::findValue(&req, "idx");
				std::string switchcmd = request::findValue(&req, "switchcmd");

				if ((idx.empty()) || (switchcmd.empty()))
					return;

				root["status"] = "OK";
				root["title"] = "ResetSecurityStatus";

				int nValue = -1;

				// Change to generic *Security_Status_Desc lookup...

				if (switchcmd == "Panic End") {
					nValue = 7;
				}
				else if (switchcmd == "Normal") {
					nValue = 0;
				}

				if (nValue >= 0)
				{
					m_sql.safe_query("UPDATE DeviceStatus SET nValue=%d WHERE (ID == '%q')",
						nValue, idx.c_str());
					root["status"] = "OK";
					root["title"] = "SwitchLight";
				}
			}
			else if (cparam == "verifypasscode")
			{
				std::string passcode = request::findValue(&req, "passcode");
				if (passcode.empty())
					return;
				//Check if passcode is correct
				passcode = GenerateMD5Hash(passcode);
				std::string rpassword;
				int nValue = 1;
				m_sql.GetPreferencesVar("ProtectionPassword", nValue, rpassword);
				if (passcode == rpassword)
				{
					root["title"] = "VerifyPasscode";
					root["status"] = "OK";
					return;
				}
			}
			else if (cparam == "switchlight")
			{
				if (session.rights < 1)
				{
					session.reply_status = reply::forbidden;
					return; //Only user/admin allowed
				}
				std::string Username = "Admin";
				if (!session.username.empty())
					Username = session.username;

				std::string idx = request::findValue(&req, "idx");

				std::string switchcmd = request::findValue(&req, "switchcmd");
				std::string level = "-1";
				if (switchcmd == "Set Level")
					level = request::findValue(&req, "level");
				std::string onlyonchange = request::findValue(&req, "ooc");//No update unless the value changed (check if updated)
				_log.Debug(DEBUG_WEBSERVER, "WEBS switchlight idx:%s switchcmd:%s level:%s", idx.c_str(), switchcmd.c_str(), level.c_str());
				std::string passcode = request::findValue(&req, "passcode");
				if ((idx.empty()) || (switchcmd.empty()) || ((switchcmd == "Set Level") && (level.empty())) )
					return;

				result = m_sql.safe_query(
					"SELECT [Protected],[Name] FROM DeviceStatus WHERE (ID = '%q')", idx.c_str());
				if (result.empty())
				{
					//Switch not found!
					return;
				}
				bool bIsProtected = atoi(result[0][0].c_str()) != 0;
				std::string sSwitchName = result[0][1];
				if (session.rights == 1)
				{
					if (!IsIdxForUser(&session, atoi(idx.c_str())))
					{
						_log.Log(LOG_ERROR, "User: %s initiated a Unauthorized switch command!", Username.c_str());
						session.reply_status = reply::forbidden;
						return;
					}
				}

				if (bIsProtected)
				{
					if (passcode.empty())
					{
						//Switch is protected, but no passcode has been
						root["title"] = "SwitchLight";
						root["status"] = "ERROR";
						root["message"] = "WRONG CODE";
						return;
					}
					//Check if passcode is correct
					passcode = GenerateMD5Hash(passcode);
					std::string rpassword;
					int nValue = 1;
					m_sql.GetPreferencesVar("ProtectionPassword", nValue, rpassword);
					if (passcode != rpassword)
					{
						_log.Log(LOG_ERROR, "User: %s initiated a switch command (Wrong code!)", Username.c_str());
						root["title"] = "SwitchLight";
						root["status"] = "ERROR";
						root["message"] = "WRONG CODE";
						return;
					}
				}

				_log.Log(LOG_STATUS, "User: %s initiated a switch command (%s/%s/%s)", Username.c_str(), idx.c_str(), sSwitchName.c_str(), switchcmd.c_str());

				root["title"] = "SwitchLight";
				if (m_mainworker.SwitchLight(idx, switchcmd, level, "-1", onlyonchange, 0) == true)
				{
					root["status"] = "OK";
				}
				else
				{
					root["status"] = "ERROR";
					root["message"] = "Error sending switch command, check device/hardware !";
				}
			} //(rtype=="switchlight")
			else if (cparam == "switchscene")
			{
				if (session.rights < 1)
				{
					session.reply_status = reply::forbidden;
					return; //Only user/admin allowed
				}
				std::string Username = "Admin";
				if (!session.username.empty())
					Username = session.username;

				std::string idx = request::findValue(&req, "idx");
				std::string switchcmd = request::findValue(&req, "switchcmd");
				std::string passcode = request::findValue(&req, "passcode");
				if ((idx.empty()) || (switchcmd.empty()))
					return;

				result = m_sql.safe_query(
					"SELECT [Protected] FROM Scenes WHERE (ID = '%q')", idx.c_str());
				if (result.empty())
				{
					//Scene/Group not found!
					return;
				}
				bool bIsProtected = atoi(result[0][0].c_str()) != 0;
				if (bIsProtected)
				{
					if (passcode.empty())
					{
						root["title"] = "SwitchScene";
						root["status"] = "ERROR";
						root["message"] = "WRONG CODE";
						return;
					}
					//Check if passcode is correct
					passcode = GenerateMD5Hash(passcode);
					std::string rpassword;
					int nValue = 1;
					m_sql.GetPreferencesVar("ProtectionPassword", nValue, rpassword);
					if (passcode != rpassword)
					{
						root["title"] = "SwitchScene";
						root["status"] = "ERROR";
						root["message"] = "WRONG CODE";
						_log.Log(LOG_ERROR, "User: %s initiated a scene/group command (Wrong code!)", Username.c_str());
						return;
					}
				}
				_log.Log(LOG_STATUS, "User: %s initiated a scene/group command", Username.c_str());

				if (m_mainworker.SwitchScene(idx, switchcmd) == true)
				{
					root["status"] = "OK";
					root["title"] = "SwitchScene";
				}
			} //(rtype=="switchscene")
			else if (cparam == "getSunRiseSet") {
				if (!m_mainworker.m_LastSunriseSet.empty())
				{
					std::vector<std::string> strarray;
					StringSplit(m_mainworker.m_LastSunriseSet, ";", strarray);
					if (strarray.size() == 10)
					{
						struct tm loctime;
						time_t now = mytime(NULL);

						localtime_r(&now, &loctime);
						//strftime(szTmp, 80, "%b %d %Y %X", &loctime);
						strftime(szTmp, 80, "%Y-%m-%d %X", &loctime);

						root["status"] = "OK";
						root["title"] = "getSunRiseSet";
						root["ServerTime"] = szTmp;
						root["Sunrise"] = strarray[0];
						root["Sunset"] = strarray[1];
						root["SunAtSouth"] = strarray[2];
						root["CivTwilightStart"] = strarray[3];
						root["CivTwilightEnd"] = strarray[4];
						root["NautTwilightStart"] = strarray[5];
						root["NautTwilightEnd"] = strarray[6];
						root["AstrTwilightStart"] = strarray[7];
						root["AstrTwilightEnd"] = strarray[8];
						root["DayLength"] = strarray[9];
					}
				}
			}
			else if (cparam == "getServerTime") {

				struct tm loctime;
				time_t now = mytime(NULL);

				localtime_r(&now, &loctime);
				//strftime(szTmp, 80, "%b %d %Y %X", &loctime);
				strftime(szTmp, 80, "%Y-%m-%d %X", &loctime);

				root["status"] = "OK";
				root["title"] = "getServerTime";
				root["ServerTime"] = szTmp;
			}
			else if (cparam == "getsecstatus")
			{
				root["status"] = "OK";
				root["title"] = "GetSecStatus";

				int secstatus = 0;
				m_sql.GetPreferencesVar("SecStatus", secstatus);
				root["secstatus"] = secstatus;

				int secondelay = 30;
				m_sql.GetPreferencesVar("SecOnDelay", secondelay);
				root["secondelay"] = secondelay;
			}
			else if (cparam == "setsecstatus")
			{
				std::string ssecstatus = request::findValue(&req, "secstatus");
				std::string seccode = request::findValue(&req, "seccode");
				if ((ssecstatus.empty()) || (seccode.empty()))
				{
					root["message"] = "WRONG CODE";
					return;
				}
				root["title"] = "SetSecStatus";
				std::string rpassword;
				int nValue = 1;
				m_sql.GetPreferencesVar("SecPassword", nValue, rpassword);
				if (seccode != rpassword)
				{
					root["status"] = "ERROR";
					root["message"] = "WRONG CODE";
					return;
				}
				root["status"] = "OK";
				int iSecStatus = atoi(ssecstatus.c_str());
				m_mainworker.UpdateDomoticzSecurityStatus(iSecStatus);
			}
			else if (cparam == "setcolbrightnessvalue")
			{
				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}
				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				_tColor color;

				std::string json = request::findValue(&req, "color");
				std::string hex = request::findValue(&req, "hex");
				std::string hue = request::findValue(&req, "hue");
				std::string sat = request::findValue(&req, "sat");
				std::string brightness = request::findValue(&req, "brightness");
				std::string iswhite = request::findValue(&req, "iswhite");

				int ival = 100;
				float brightnessAdj = 1.0f;

				if (!json.empty())
				{
					color = _tColor(json);
					if (color.mode == ColorModeRGB)
					{
						// Normalize RGB to full brightness
						float hsb[3];
						int r, g, b;
						rgb2hsb(color.r, color.g, color.b, hsb);
						hsb2rgb(hsb[0]*360.0f, hsb[1], 1.0f, r, g, b, 255);
						color.r = r;
						color.g = g;
						color.b = b;
						brightnessAdj = hsb[2];
					}

					//_log.Debug(DEBUG_WEBSERVER, "setcolbrightnessvalue: json: %s, color: '%s', bri: '%s'", json.c_str(), color.toString().c_str(), brightness.c_str());
				}
				else if (!hex.empty())
				{
					uint64_t ihex = hexstrtoui64(hex);
					//_log.Debug(DEBUG_WEBSERVER, "setcolbrightnessvalue: hex: '%s', ihex: %" PRIx64 ", bri: '%s', iswhite: '%s'", hex.c_str(), ihex, brightness.c_str(), iswhite.c_str());
					uint8_t r = 0;
					uint8_t g = 0;
					uint8_t b = 0;
					uint8_t cw = 0;
					uint8_t ww = 0;
					switch (hex.length())
					{
						case 6: //RGB
							r = (uint8_t)((ihex & 0x0000FF0000) >> 16);
							g = (uint8_t)((ihex & 0x000000FF00) >> 8);
							b = (uint8_t)ihex & 0xFF;
							float hsb[3];
							int tr, tg, tb; // tmp of 'int' type so can be passed as references to hsb2rgb
							rgb2hsb(r, g, b, hsb);
							// Normalize RGB to full brightness
							hsb2rgb(hsb[0]*360.0f, hsb[1], 1.0f, tr, tg, tb, 255);
							r = tr;
							g = tg;
							b = tb;
							brightnessAdj = hsb[2];
							// Backwards compatibility: set iswhite for unsaturated colors
							iswhite = (hsb[1] < (20.0 / 255.0)) ? "true" : "false";
							color = _tColor(r, g, b, cw, ww, ColorModeRGB);
							break;
						case 8: //RGB_WW
							r = (uint8_t)((ihex & 0x00FF000000) >> 24);
							g = (uint8_t)((ihex & 0x0000FF0000) >> 16);
							b = (uint8_t)((ihex & 0x000000FF00) >> 8);
							ww = (uint8_t)ihex & 0xFF;
							color = _tColor(r, g, b, cw, ww, ColorModeCustom);
							break;
						case 10: //RGB_CW_WW
							r = (uint8_t)((ihex & 0xFF00000000) >> 32);
							g = (uint8_t)((ihex & 0x00FF000000) >> 24);
							b = (uint8_t)((ihex & 0x0000FF0000) >> 16);
							cw = (uint8_t)((ihex & 0x000000FF00) >> 8);
							ww = (uint8_t)ihex & 0xFF;
							color = _tColor(r, g, b, cw, ww, ColorModeCustom);
							break;
					}
					if (iswhite == "true") color.mode = ColorModeWhite;
					//_log.Debug(DEBUG_WEBSERVER, "setcolbrightnessvalue: rgbww: %02x%02x%02x%02x%02x, color: '%s'", r, g, b, cw, ww, color.toString().c_str());
				}
				else if (!hue.empty())
				{
					int r, g, b;

					//convert hue to RGB
					float iHue = float(atof(hue.c_str()));
					float iSat = 100.0f;
					if (!sat.empty()) iSat = float(atof(sat.c_str()));
					hsb2rgb(iHue, iSat/100.0f, 1.0f, r, g, b, 255);

					color = _tColor(r, g, b, 0, 0, ColorModeRGB);
					if (iswhite == "true") color.mode = ColorModeWhite;
					//_log.Debug(DEBUG_WEBSERVER, "setcolbrightnessvalue2: hue: %f, rgb: %02x%02x%02x, color: '%s'", iHue, r, g, b, color.toString().c_str());
				}

				if (color.mode == ColorModeNone)
				{
					return;
				}

				if (!brightness.empty())
					ival = atoi(brightness.c_str());
				ival = int(ival * brightnessAdj);
				ival = std::max(ival, 0);
				ival = std::min(ival, 100);

				_log.Log(LOG_STATUS, "setcolbrightnessvalue: ID: %" PRIx64 ", bri: %d, color: '%s'", ID, ival, color.toString().c_str());
				m_mainworker.SwitchLight(ID, "Set Color", (unsigned char)ival, color, false, 0);

				root["status"] = "OK";
				root["title"] = "SetColBrightnessValue";
			}
			else if (cparam.find("setkelvinlevel") == 0)
			{
				root["status"] = "OK";
				root["title"] = "Set Kelvin Level";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);

				std::string kelvin = request::findValue(&req, "kelvin");
				double ival = atof(kelvin.c_str());
				ival = std::max(ival, 0.0);
				ival = std::min(ival, 100.0);
				_tColor color = _tColor(round(ival*255.0f/100.0f), ColorModeTemp);
				_log.Log(LOG_STATUS, "setkelvinlevel: t: %f, color: '%s'", ival, color.toString().c_str());

				m_mainworker.SwitchLight(ID, "Set Color", -1, color, false, 0);
			}
			else if (cparam == "brightnessup")
			{
				root["status"] = "OK";
				root["title"] = "Set brightness up!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Bright Up", 0, NoColor, false, 0);
			}
			else if (cparam == "brightnessdown")
			{
				root["status"] = "OK";
				root["title"] = "Set brightness down!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Bright Down", 0, NoColor, false, 0);
			}
			else if (cparam == "discomode")
			{
				root["status"] = "OK";
				root["title"] = "Set to last known disco mode!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Disco Mode", 0, NoColor, false, 0);
			}
			else if (cparam.find("discomodenum") == 0 && cparam != "discomode" && cparam.size() == 13)
			{
				root["status"] = "OK";
				root["title"] = "Set to disco mode!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				char szTmp[40];
				sprintf(szTmp, "Disco Mode %s", cparam.substr(12).c_str());
				m_mainworker.SwitchLight(ID, szTmp, 0, NoColor, false, 0);
			}
			else if (cparam == "discoup")
			{
				root["status"] = "OK";
				root["title"] = "Set to next disco mode!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Disco Up", 0, NoColor, false, 0);
			}
			else if (cparam == "discodown")
			{
				root["status"] = "OK";
				root["title"] = "Set to previous disco mode!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Disco Down", 0, NoColor, false, 0);
			}
			else if (cparam == "speedup")
			{
				root["status"] = "OK";
				root["title"] = "Set disco speed up!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Speed Up", 0, NoColor, false, 0);
			}
			else if (cparam == "speeduplong")
			{

				root["status"] = "OK";
				root["title"] = "Set speed long!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Speed Up Long", 0, NoColor, false, 0);
			}
			else if (cparam == "speeddown")
			{
				root["status"] = "OK";
				root["title"] = "Set disco speed down!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Speed Down", 0, NoColor, false, 0);
			}
			else if (cparam == "speedmin")
			{
				root["status"] = "OK";
				root["title"] = "Set disco speed minimal!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Speed Minimal", 0, NoColor, false, 0);
			}
			else if (cparam == "speedmax")
			{
				root["status"] = "OK";
				root["title"] = "Set disco speed maximal!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Speed Maximal", 0, NoColor, false, 0);
			}
			else if (cparam == "warmer")
			{
				root["status"] = "OK";
				root["title"] = "Set Kelvin up!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Warmer", 0, NoColor, false, 0);
			}
			else if (cparam == "cooler")
			{
				root["status"] = "OK";
				root["title"] = "Set Kelvin down!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Cooler", 0, NoColor, false, 0);
			}
			else if (cparam == "fulllight")
			{
				root["status"] = "OK";
				root["title"] = "Set Full!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Set Full", 0, NoColor, false, 0);
			}
			else if (cparam == "nightlight")
			{
				root["status"] = "OK";
				root["title"] = "Set to nightlight!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				m_mainworker.SwitchLight(ID, "Set Night", 0, NoColor, false, 0);
			}
			else if (cparam == "whitelight")
			{
				root["status"] = "OK";
				root["title"] = "Set to clear white!";

				std::string idx = request::findValue(&req, "idx");

				if (idx.empty())
				{
					return;
				}

				uint64_t ID = std::strtoull(idx.c_str(), nullptr, 10);
				//TODO: Change to color with mode=ColorModeWhite and level=100?
				m_mainworker.SwitchLight(ID, "Set White", 0, NoColor, false, 0);
			}
			else if (cparam == "getfloorplanimages")
			{
				root["status"] = "OK";
				root["title"] = "GetFloorplanImages";

				bool bReturnUnused = atoi(request::findValue(&req, "unused").c_str()) != 0;

				if (!bReturnUnused)
					result = m_sql.safe_query("SELECT ID, Name, ScaleFactor FROM Floorplans ORDER BY [Name]");
				else
					result = m_sql.safe_query("SELECT ID, Name, ScaleFactor FROM Floorplans WHERE ID NOT IN(SELECT FloorplanID FROM Plans)");
				if (!result.empty())
				{
					int ii = 0;
					for (const auto & itt : result)
					{
						std::vector<std::string> sd = itt;

						root["result"][ii]["idx"] = sd[0];
						root["result"][ii]["name"] = sd[1];
						root["result"][ii]["scalefactor"] = sd[2];
						ii++;
					}
				}
			}
			else if (cparam == "updatefloorplan")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				std::string name = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));
				std::string scalefactor = request::findValue(&req, "scalefactor");
				if (
					(name.empty())
					||(scalefactor.empty())
					)
					return;

				root["status"] = "OK";
				root["title"] = "UpdateFloorplan";

				m_sql.safe_query(
					"UPDATE Floorplans SET Name='%q',ScaleFactor='%q' WHERE (ID == '%q')",
					name.c_str(),
					scalefactor.c_str(),
					idx.c_str()
				);
			}
			else if (cparam == "deletefloorplan")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "DeleteFloorplan";
				m_sql.safe_query("UPDATE DeviceToPlansMap SET XOffset=0,YOffset=0 WHERE (PlanID IN (SELECT ID from Plans WHERE (FloorplanID == '%q')))", idx.c_str());
				m_sql.safe_query("UPDATE Plans SET FloorplanID=0,Area='' WHERE (FloorplanID == '%q')", idx.c_str());
				m_sql.safe_query("DELETE FROM Floorplans WHERE (ID == '%q')", idx.c_str());
			}
			else if (cparam == "changefloorplanorder")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				std::string sway = request::findValue(&req, "way");
				if (sway.empty())
					return;
				bool bGoUp = (sway == "0");

				std::string aOrder, oID, oOrder;

				result = m_sql.safe_query("SELECT [Order] FROM Floorplans WHERE (ID=='%q')",
					idx.c_str());
				if (result.empty())
					return;
				aOrder = result[0][0];

				if (!bGoUp)
				{
					//Get next device order
					result = m_sql.safe_query("SELECT ID, [Order] FROM Floorplans WHERE ([Order]>'%q') ORDER BY [Order] ASC",
						aOrder.c_str());
					if (result.empty())
						return;
					oID = result[0][0];
					oOrder = result[0][1];
				}
				else
				{
					//Get previous device order
					result = m_sql.safe_query("SELECT ID, [Order] FROM Floorplans WHERE ([Order]<'%q') ORDER BY [Order] DESC",
						aOrder.c_str());
					if (result.empty())
						return;
					oID = result[0][0];
					oOrder = result[0][1];
				}
				//Swap them
				root["status"] = "OK";
				root["title"] = "ChangeFloorPlanOrder";

				m_sql.safe_query("UPDATE Floorplans SET [Order] = '%q' WHERE (ID='%q')",
					oOrder.c_str(), idx.c_str());
				m_sql.safe_query("UPDATE Floorplans SET [Order] = '%q' WHERE (ID='%q')",
					aOrder.c_str(), oID.c_str());
			}
			else if (cparam == "getunusedfloorplanplans")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				root["status"] = "OK";
				root["title"] = "GetUnusedFloorplanPlans";
				int ii = 0;

				result = m_sql.safe_query("SELECT ID, Name FROM Plans WHERE (FloorplanID==0) ORDER BY Name");
				if (!result.empty())
				{
					for (const auto & itt : result)
					{
						std::vector<std::string> sd = itt;

						root["result"][ii]["type"] = 0;
						root["result"][ii]["idx"] = sd[0];
						root["result"][ii]["Name"] = sd[1];
						ii++;
					}
				}
			}
			else if (cparam == "getfloorplanplans")
			{
				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "GetFloorplanPlans";
				int ii = 0;
				result = m_sql.safe_query("SELECT ID, Name, Area FROM Plans WHERE (FloorplanID=='%q') ORDER BY Name",
					idx.c_str());
				if (!result.empty())
				{
					for (const auto & itt : result)
					{
						std::vector<std::string> sd = itt;

						root["result"][ii]["idx"] = sd[0];
						root["result"][ii]["Name"] = sd[1];
						root["result"][ii]["Area"] = sd[2];
						ii++;
					}
				}
			}
			else if (cparam == "addfloorplanplan")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				std::string planidx = request::findValue(&req, "planidx");
				if (
					(idx.empty()) ||
					(planidx.empty())
					)
					return;
				root["status"] = "OK";
				root["title"] = "AddFloorplanPlan";

				m_sql.safe_query(
					"UPDATE Plans SET FloorplanID='%q' WHERE (ID == '%q')",
					idx.c_str(),
					planidx.c_str()
				);
				_log.Log(LOG_STATUS, "(Floorplan) Plan '%s' added to floorplan '%s'.", planidx.c_str(), idx.c_str());
			}
			else if (cparam == "updatefloorplanplan")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string planidx = request::findValue(&req, "planidx");
				std::string planarea = request::findValue(&req, "area");
				if (planidx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "UpdateFloorplanPlan";

				m_sql.safe_query(
					"UPDATE Plans SET Area='%q' WHERE (ID == '%q')",
					planarea.c_str(),
					planidx.c_str()
				);
				_log.Log(LOG_STATUS, "(Floorplan) Plan '%s' floor area updated to '%s'.", planidx.c_str(), planarea.c_str());
			}
			else if (cparam == "deletefloorplanplan")
			{
				if (session.rights < 2)
				{
					session.reply_status = reply::forbidden;
					return; //Only admin user allowed
				}

				std::string idx = request::findValue(&req, "idx");
				if (idx.empty())
					return;
				root["status"] = "OK";
				root["title"] = "DeleteFloorplanPlan";
				m_sql.safe_query(
					"UPDATE DeviceToPlansMap SET XOffset=0,YOffset=0 WHERE (PlanID == '%q')",
					idx.c_str()
				);
				_log.Log(LOG_STATUS, "(Floorplan) Device coordinates reset for plan '%s'.", idx.c_str());
				m_sql.safe_query(
					"UPDATE Plans SET FloorplanID=0,Area='' WHERE (ID == '%q')",
					idx.c_str()
				);
				_log.Log(LOG_STATUS, "(Floorplan) Plan '%s' floorplan data reset.", idx.c_str());
			}
		}

		void CWebServer::DisplaySwitchTypesCombo(std::string & content_part)
		{
			char szTmp[200];

			std::map<std::string, int> _switchtypes;

			for (int ii = 0; ii < STYPE_END; ii++)
			{
				_switchtypes[Switch_Type_Desc((_eSwitchType)ii)] = ii;
			}
			//return a sorted list
			for (const auto & itt : _switchtypes)
			{
				sprintf(szTmp, "<option value=\"%d\">%s</option>\n", itt.second, itt.first.c_str());
				content_part += szTmp;
			}
		}

		void CWebServer::DisplayMeterTypesCombo(std::string & content_part)
		{
			char szTmp[200];
			for (int ii = 0; ii < MTYPE_END; ii++)
			{
				sprintf(szTmp, "<option value=\"%d\">%s</option>\n", ii, Meter_Type_Desc((_eMeterType)ii));
				content_part += szTmp;
			}
		}

		void CWebServer::DisplayLanguageCombo(std::string & content_part)
		{
			//return a sorted list
			std::map<std::string, std::string> _ltypes;
			char szTmp[200];
			int ii = 0;
			while (guiLanguage[ii].szShort != NULL)
			{
				_ltypes[guiLanguage[ii].szLong] = guiLanguage[ii].szShort;
				ii++;
			}
			for (const auto & itt : _ltypes)
			{
				sprintf(szTmp, "<option value=\"%s\">%s</option>\n", itt.second.c_str(), itt.first.c_str());
				content_part += szTmp;
			}
		}

		void CWebServer::DisplayTimerTypesCombo(std::string & content_part)
		{
			char szTmp[200];
			for (int ii = 0; ii < TTYPE_END; ii++)
			{
				sprintf(szTmp, "<option data-i18n=\"%s\" value=\"%d\">%s</option>\n", Timer_Type_Desc(ii), ii, Timer_Type_Desc(ii));
				content_part += szTmp;
			}
		}

		void CWebServer::LoadUsers()
		{
			ClearUserPasswords();
			std::string WebUserName, WebPassword;
			int nValue = 0;
			if (m_sql.GetPreferencesVar("WebUserName", nValue, WebUserName))
			{
				if (m_sql.GetPreferencesVar("WebPassword", nValue, WebPassword))
				{
					if ((WebUserName != "") && (WebPassword != ""))
					{
						WebUserName = base64_decode(WebUserName);
						//WebPassword = WebPassword;
						AddUser(10000, WebUserName, WebPassword, URIGHTS_ADMIN, 0xFFFF);

						std::vector<std::vector<std::string> > result;
						result = m_sql.safe_query("SELECT ID, Active, Username, Password, Rights, TabsEnabled FROM Users");
						if (!result.empty())
						{
							int ii = 0;
							for (const auto & itt : result)
							{
								std::vector<std::string> sd = itt;

								int bIsActive = static_cast<int>(atoi(sd[1].c_str()));
								if (bIsActive)
								{
									unsigned long ID = (unsigned long)atol(sd[0].c_str());

									std::string username = base64_decode(sd[2]);
									std::string password = sd[3];

									_eUserRights rights = (_eUserRights)atoi(sd[4].c_str());
									int activetabs = atoi(sd[5].c_str());

									AddUser(ID, username, password, rights, activetabs);
								}
							}
						}
					}
				}
			}
			m_mainworker.LoadSharedUsers();
		}

		void CWebServer::AddUser(const unsigned long ID, const std::string &username, const std::string &password, const int userrights, const int activetabs)
		{
			std::vector<std::vector<std::string> > result = m_sql.safe_query("SELECT COUNT(*) FROM SharedDevices WHERE (SharedUserID == '%d')", ID);
			if (result.empty())
				return;

			_tWebUserPassword wtmp;
			wtmp.ID = ID;
			wtmp.Username = username;
			wtmp.Password = password;
			wtmp.userrights = (_eUserRights)userrights;
			wtmp.ActiveTabs = activetabs;
			wtmp.TotSensors = atoi(result[0][0].c_str());
			m_users.push_back(wtmp);

			m_pWebEm->AddUserPassword(ID, username, password, (_eUserRights)userrights, activetabs);
		}

		void CWebServer::ClearUserPasswords()
		{
			m_users.clear();
			m_pWebEm->ClearUserPasswords();
		}

		int CWebServer::FindUser(const char* szUserName)
		{
			int iUser = 0;
			for (const auto & itt : m_users)
			{
				if (itt.Username == szUserName)
					return iUser;
				iUser++;
			}
			return -1;
		}

		bool CWebServer::FindAdminUser()
		{
			for (const auto & itt : m_users)
			{
				if (itt.userrights == URIGHTS_ADMIN)
					return true;
			}
			return false;
		}

		void CWebServer::RestoreDatabase(WebEmSession & session, const request& req, std::string & redirect_uri)
		{
			redirect_uri = "/index.html";
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string dbasefile = request::findValue(&req, "dbasefile");
			if (dbasefile.empty()) {
				return;
			}

			m_mainworker.StopDomoticzHardware();

			m_sql.RestoreDatabase(dbasefile);
			m_mainworker.AddAllDomoticzHardware();
		}

		struct _tHardwareListInt {
			std::string Name;
			int HardwareTypeVal;
			std::string HardwareType;
			bool Enabled;
			std::string Mode1; // Used to flag DimmerType as relative for some old LimitLessLight type bulbs
			std::string Mode2; // Used to flag DimmerType as relative for some old LimitLessLight type bulbs
		} tHardwareList;

		void CWebServer::UploadFloorplanImage(WebEmSession & session, const request& req, std::string & redirect_uri)
		{
			redirect_uri = "/index.html";
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string planname = request::findValue(&req, "planname");
			std::string scalefactor = request::findValue(&req, "scalefactor");
			std::string imagefile = request::findValue(&req, "imagefile");

			std::vector<std::vector<std::string> > result;
			m_sql.safe_query("INSERT INTO Floorplans ([Name],[ScaleFactor]) VALUES('%s','%s')", planname.c_str(),scalefactor.c_str());
			result = m_sql.safe_query("SELECT MAX(ID) FROM Floorplans");
			if (!result.empty())
			{
				if (!m_sql.safe_UpdateBlobInTableWithID("Floorplans", "Image", result[0][0], imagefile))
					_log.Log(LOG_ERROR, "SQL: Problem inserting floorplan image into database! ");
			}
		}

		void CWebServer::GetFloorplanImage(WebEmSession & session, const request& req, reply & rep)
		{
			std::string idx = request::findValue(&req, "idx");
			if (idx == "") {
				return;
			}
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_queryBlob("SELECT Image FROM Floorplans WHERE ID=%d", atol(idx.c_str()));
			if (result.empty())
				return;
			reply::set_content(&rep, result[0][0].begin(), result[0][0].end());
			std::string oname = "floorplan";
			if (result[0][0].size() > 10)
			{
				if (result[0][0][0] == 'P')
					oname += ".png";
				else if (result[0][0][0] == -1)
					oname += ".jpg";
				else if (result[0][0][0] == 'B')
					oname += ".bmp";
				else if (result[0][0][0] == 'G')
					oname += ".gif";
				else if ((result[0][0][0] == '<') && (result[0][0][1] == 's') && (result[0][0][2] == 'v') && (result[0][0][3] == 'g'))
					oname += ".svg";
				else if (result[0][0].find("<svg") != std::string::npos) //some SVG's start with <xml
					oname += ".svg";
			}
			reply::add_header_attachment(&rep, oname);
		}

		void CWebServer::GetDatabaseBackup(WebEmSession & session, const request& req, reply & rep)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
#ifdef WIN32
			std::string OutputFileName = szUserDataFolder + "backup.db";
#else
			std::string OutputFileName = "/tmp/backup.db";
#endif
			if (m_sql.BackupDatabase(OutputFileName))
			{
				std::string szAttachmentName = "domoticz.db";
				std::string szVar;
				if (m_sql.GetPreferencesVar("Title", szVar))
				{
					stdreplace(szVar, " ", "_");
					stdreplace(szVar, "/", "_");
					stdreplace(szVar, "\\", "_");
					if (!szVar.empty()) {
						szAttachmentName = szVar + ".db";
					}
				}
				reply::set_download_file(&rep, OutputFileName, szAttachmentName);
			}
		}

		void CWebServer::RType_DeleteDevice(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;

			root["status"] = "OK";
			root["title"] = "DeleteDevice";
			m_sql.DeleteDevices(idx);
			m_mainworker.m_scheduler.ReloadSchedules();
		}

		void CWebServer::RType_AddScene(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string name = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));
			name = HTMLSanitizer::Sanitize(name);
			if (name.empty())
			{
				root["status"] = "ERR";
				root["message"] = "No Scene Name specified!";
				return;
			}
			std::string stype = request::findValue(&req, "scenetype");
			if (stype.empty())
			{
				root["status"] = "ERR";
				root["message"] = "No Scene Type specified!";
				return;
			}
			if (m_sql.DoesSceneByNameExits(name) == true)
			{
				root["status"] = "ERR";
				root["message"] = "A Scene with this Name already Exits!";
				return;
			}
			root["status"] = "OK";
			root["title"] = "AddScene";
			m_sql.safe_query(
				"INSERT INTO Scenes (Name,SceneType) VALUES ('%q',%d)",
				name.c_str(),
				atoi(stype.c_str())
			);
			m_mainworker.m_eventsystem.GetCurrentScenesGroups();
		}

		void CWebServer::RType_DeleteScene(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			root["status"] = "OK";
			root["title"] = "DeleteScene";
			m_sql.safe_query("DELETE FROM Scenes WHERE (ID == '%q')", idx.c_str());
			m_sql.safe_query("DELETE FROM SceneDevices WHERE (SceneRowID == '%q')", idx.c_str());
			m_sql.safe_query("DELETE FROM SceneTimers WHERE (SceneRowID == '%q')", idx.c_str());
			m_sql.safe_query("DELETE FROM SceneLog WHERE (SceneRowID=='%q')", idx.c_str());
			uint64_t ullidx = std::strtoull(idx.c_str(), nullptr, 10);
			m_mainworker.m_eventsystem.RemoveSingleState(ullidx, m_mainworker.m_eventsystem.REASON_SCENEGROUP);
		}

		void CWebServer::RType_UpdateScene(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string idx = request::findValue(&req, "idx");
			std::string name = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));
			std::string description = HTMLSanitizer::Sanitize(request::findValue(&req, "description"));

			name = HTMLSanitizer::Sanitize(name);
			description = HTMLSanitizer::Sanitize(description);

			if ((idx.empty()) || (name.empty()))
				return;
			std::string stype = request::findValue(&req, "scenetype");
			if (stype.empty())
			{
				root["status"] = "ERR";
				root["message"] = "No Scene Type specified!";
				return;
			}
			std::string tmpstr = request::findValue(&req, "protected");
			int iProtected = (tmpstr == "true") ? 1 : 0;

			std::string onaction = base64_decode(request::findValue(&req, "onaction"));
			std::string offaction = base64_decode(request::findValue(&req, "offaction"));


			root["status"] = "OK";
			root["title"] = "UpdateScene";
			m_sql.safe_query("UPDATE Scenes SET Name='%q', Description='%q', SceneType=%d, Protected=%d, OnAction='%q', OffAction='%q' WHERE (ID == '%q')",
				name.c_str(),
				description.c_str(),
				atoi(stype.c_str()),
				iProtected,
				onaction.c_str(),
				offaction.c_str(),
				idx.c_str()
			);
			uint64_t ullidx = std::strtoull(idx.c_str(), nullptr, 10);
			m_mainworker.m_eventsystem.WWWUpdateSingleState(ullidx, name, m_mainworker.m_eventsystem.REASON_SCENEGROUP);
		}

		bool compareIconsByName(const http::server::CWebServer::_tCustomIcon &a, const http::server::CWebServer::_tCustomIcon &b)
		{
			return a.Title < b.Title;
		}

		void CWebServer::RType_CustomLightIcons(WebEmSession & session, const request& req, Json::Value &root)
		{
			int ii = 0;

			std::vector<_tCustomIcon> temp_custom_light_icons = m_custom_light_icons;
			//Sort by name
			std::sort(temp_custom_light_icons.begin(), temp_custom_light_icons.end(), compareIconsByName);

			for (const auto & itt : temp_custom_light_icons)
			{
				root["result"][ii]["idx"] = itt.idx;
				root["result"][ii]["imageSrc"] = itt.RootFile;
				root["result"][ii]["text"] = itt.Title;
				root["result"][ii]["description"] = itt.Description;
				ii++;
			}
			root["status"] = "OK";
		}

		void CWebServer::RType_Plans(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "Plans";

			std::string sDisplayHidden = request::findValue(&req, "displayhidden");
			bool bDisplayHidden = (sDisplayHidden == "1");

			std::vector<std::vector<std::string> > result, result2;
			result = m_sql.safe_query("SELECT ID, Name, [Order] FROM Plans ORDER BY [Order]");
			if (!result.empty())
			{
				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					std::string Name = sd[1];
					bool bIsHidden = (Name[0] == '$');

					if ((bDisplayHidden) || (!bIsHidden))
					{
						root["result"][ii]["idx"] = sd[0];
						root["result"][ii]["Name"] = Name;
						root["result"][ii]["Order"] = sd[2];

						unsigned int totDevices = 0;

						result2 = m_sql.safe_query("SELECT COUNT(*) FROM DeviceToPlansMap WHERE (PlanID=='%q')",
							sd[0].c_str());
						if (!result2.empty())
						{
							totDevices = (unsigned int)atoi(result2[0][0].c_str());
						}
						root["result"][ii]["Devices"] = totDevices;

						ii++;
					}
				}
			}
		}

		void CWebServer::RType_FloorPlans(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "Floorplans";

			std::vector<std::vector<std::string> > result, result2, result3;

			result = m_sql.safe_query("SELECT Key, nValue, sValue FROM Preferences WHERE Key LIKE 'Floorplan%%'");
			if (result.empty())
				return;

			for (const auto & itt : result)
			{
				std::vector<std::string> sd = itt;
				std::string Key = sd[0];
				int nValue = atoi(sd[1].c_str());
				std::string sValue = sd[2];

				if (Key == "FloorplanPopupDelay")
				{
					root["PopupDelay"] = nValue;
				}
				if (Key == "FloorplanFullscreenMode")
				{
					root["FullscreenMode"] = nValue;
				}
				if (Key == "FloorplanAnimateZoom")
				{
					root["AnimateZoom"] = nValue;
				}
				if (Key == "FloorplanShowSensorValues")
				{
					root["ShowSensorValues"] = nValue;
				}
				if (Key == "FloorplanShowSwitchValues")
				{
					root["ShowSwitchValues"] = nValue;
				}
				if (Key == "FloorplanShowSceneNames")
				{
					root["ShowSceneNames"] = nValue;
				}
				if (Key == "FloorplanRoomColour")
				{
					root["RoomColour"] = sValue;
				}
				if (Key == "FloorplanActiveOpacity")
				{
					root["ActiveRoomOpacity"] = nValue;
				}
				if (Key == "FloorplanInactiveOpacity")
				{
					root["InactiveRoomOpacity"] = nValue;
				}
			}

			result2 = m_sql.safe_query("SELECT ID, Name, ScaleFactor, [Order] FROM Floorplans ORDER BY [Order]");
			if (!result2.empty())
			{
				int ii = 0;
				for (const auto & itt : result2)
				{
					std::vector<std::string> sd = itt;

					root["result"][ii]["idx"] = sd[0];
					root["result"][ii]["Name"] = sd[1];
					std::string ImageURL = "images/floorplans/plan?idx=" + sd[0];
					root["result"][ii]["Image"] = ImageURL;
					root["result"][ii]["ScaleFactor"] = sd[2];
					root["result"][ii]["Order"] = sd[3];

					unsigned int totPlans = 0;

					result3 = m_sql.safe_query("SELECT COUNT(*) FROM Plans WHERE (FloorplanID=='%q')", sd[0].c_str());
					if (!result3.empty())
					{
						totPlans = (unsigned int)atoi(result3[0][0].c_str());
					}
					root["result"][ii]["Plans"] = totPlans;

					ii++;
				}
			}
		}

		void CWebServer::RType_Scenes(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "Scenes";
			root["AllowWidgetOrdering"] = m_sql.m_bAllowWidgetOrdering;

			std::string sDisplayHidden = request::findValue(&req, "displayhidden");
			bool bDisplayHidden = (sDisplayHidden == "1");

			std::string sLastUpdate = request::findValue(&req, "lastupdate");

			std::string rid = request::findValue(&req, "rid");

			time_t LastUpdate = 0;
			if (sLastUpdate != "")
			{
				std::stringstream sstr;
				sstr << sLastUpdate;
				sstr >> LastUpdate;
			}

			time_t now = mytime(NULL);
			struct tm tm1;
			localtime_r(&now, &tm1);
			struct tm tLastUpdate;
			localtime_r(&now, &tLastUpdate);

			root["ActTime"] = static_cast<int>(now);

			std::vector<std::vector<std::string> > result, result2;
			std::string szQuery = "SELECT ID, Name, Activators, Favorite, nValue, SceneType, LastUpdate, Protected, OnAction, OffAction, Description FROM Scenes";
			if (!rid.empty())
				szQuery += " WHERE (ID == " + rid + ")";
			szQuery += " ORDER BY [Order]";
			result = m_sql.safe_query(szQuery.c_str());
			if (!result.empty())
			{
				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					std::string sName = sd[1];
					if ((bDisplayHidden == false) && (sName[0] == '$'))
						continue;

					std::string sLastUpdate = sd[6].c_str();
					if (LastUpdate != 0)
					{
						time_t cLastUpdate;
						ParseSQLdatetime(cLastUpdate, tLastUpdate, sLastUpdate, tm1.tm_isdst);
						if (cLastUpdate <= LastUpdate)
							continue;
					}

					unsigned char nValue = atoi(sd[4].c_str());
					unsigned char scenetype = atoi(sd[5].c_str());
					int iProtected = atoi(sd[7].c_str());

					std::string onaction = base64_encode(sd[8]);
					std::string offaction = base64_encode(sd[9]);

					root["result"][ii]["idx"] = sd[0];
					root["result"][ii]["Name"] = sName;
					root["result"][ii]["Description"] = sd[10];
					root["result"][ii]["Favorite"] = atoi(sd[3].c_str());
					root["result"][ii]["Protected"] = (iProtected != 0);
					root["result"][ii]["OnAction"] = onaction;
					root["result"][ii]["OffAction"] = offaction;

					if (scenetype == 0)
					{
						root["result"][ii]["Type"] = "Scene";
					}
					else
					{
						root["result"][ii]["Type"] = "Group";
					}

					root["result"][ii]["LastUpdate"] = sLastUpdate;

					if (nValue == 0)
						root["result"][ii]["Status"] = "Off";
					else if (nValue == 1)
						root["result"][ii]["Status"] = "On";
					else
						root["result"][ii]["Status"] = "Mixed";
					root["result"][ii]["Timers"] = (m_sql.HasSceneTimers(sd[0]) == true) ? "true" : "false";
					uint64_t camIDX = m_mainworker.m_cameras.IsDevSceneInCamera(1, sd[0]);
					root["result"][ii]["UsedByCamera"] = (camIDX != 0) ? true : false;
					if (camIDX != 0) {
						std::stringstream scidx;
						scidx << camIDX;
						root["result"][ii]["CameraIdx"] = scidx.str();
					}
					ii++;
				}
			}
			if (!m_mainworker.m_LastSunriseSet.empty())
			{
				std::vector<std::string> strarray;
				StringSplit(m_mainworker.m_LastSunriseSet, ";", strarray);
				if (strarray.size() == 10)
				{
					char szTmp[100];
					//strftime(szTmp, 80, "%b %d %Y %X", &tm1);
					strftime(szTmp, 80, "%Y-%m-%d %X", &tm1);
					root["ServerTime"] = szTmp;
					root["Sunrise"] = strarray[0];
					root["Sunset"] = strarray[1];
					root["SunAtSouth"] = strarray[2];
					root["CivTwilightStart"] = strarray[3];
					root["CivTwilightEnd"] = strarray[4];
					root["NautTwilightStart"] = strarray[5];
					root["NautTwilightEnd"] = strarray[6];
					root["AstrTwilightStart"] = strarray[7];
					root["AstrTwilightEnd"] = strarray[8];
					root["DayLength"] = strarray[9];
				}
			}
		}

		void CWebServer::RType_Hardware(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "Hardware";

#ifdef WITH_OPENZWAVE
			m_ZW_Hwidx = -1;
#endif

			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT ID, Name, Enabled, Type, Address, Port, SerialPort, Username, Password, Extra, Mode1, Mode2, Mode3, Mode4, Mode5, Mode6, DataTimeout FROM Hardware ORDER BY ID ASC");
			if (!result.empty())
			{
				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					_eHardwareTypes hType = (_eHardwareTypes)atoi(sd[3].c_str());
					if (hType == HTYPE_DomoticzInternal)
						continue;
					root["result"][ii]["idx"] = sd[0];
					root["result"][ii]["Name"] = sd[1];
					root["result"][ii]["Enabled"] = (sd[2] == "1") ? "true" : "false";
					root["result"][ii]["Type"] = hType;
					root["result"][ii]["Address"] = sd[4];
					root["result"][ii]["Port"] = atoi(sd[5].c_str());
					root["result"][ii]["SerialPort"] = sd[6];
					root["result"][ii]["Username"] = sd[7];
					root["result"][ii]["Password"] = sd[8];
					root["result"][ii]["Extra"] = sd[9];

					if (hType == HTYPE_PythonPlugin) {
						root["result"][ii]["Mode1"] = sd[10];  // Plugins can have non-numeric values in the Mode fields
						root["result"][ii]["Mode2"] = sd[11];
						root["result"][ii]["Mode3"] = sd[12];
						root["result"][ii]["Mode4"] = sd[13];
						root["result"][ii]["Mode5"] = sd[14];
						root["result"][ii]["Mode6"] = sd[15];
					}
					else {
						root["result"][ii]["Mode1"] = atoi(sd[10].c_str());
						root["result"][ii]["Mode2"] = atoi(sd[11].c_str());
						root["result"][ii]["Mode3"] = atoi(sd[12].c_str());
						root["result"][ii]["Mode4"] = atoi(sd[13].c_str());
						root["result"][ii]["Mode5"] = atoi(sd[14].c_str());
						root["result"][ii]["Mode6"] = atoi(sd[15].c_str());
					}
					root["result"][ii]["DataTimeout"] = atoi(sd[16].c_str());

					//Special case for openzwave (status for nodes queried)
					CDomoticzHardwareBase *pHardware = m_mainworker.GetHardware(atoi(sd[0].c_str()));
					if (pHardware != NULL)
					{
						if (
							(pHardware->HwdType == HTYPE_RFXtrx315) ||
							(pHardware->HwdType == HTYPE_RFXtrx433) ||
							(pHardware->HwdType == HTYPE_RFXtrx868) ||
							(pHardware->HwdType == HTYPE_RFXLAN)
							)
						{
							CRFXBase *pMyHardware = reinterpret_cast<CRFXBase*>(pHardware);
							if (!pMyHardware->m_Version.empty())
								root["result"][ii]["version"] = pMyHardware->m_Version;
							else
								root["result"][ii]["version"] = sd[11];
							root["result"][ii]["noiselvl"] = pMyHardware->m_NoiseLevel;
						}
						else
						{
#ifdef WITH_OPENZWAVE
							if (pHardware->HwdType == HTYPE_OpenZWave)
							{
								COpenZWave *pOZWHardware = reinterpret_cast<COpenZWave*>(pHardware);
								root["result"][ii]["version"] = pOZWHardware->GetVersionLong();
								root["result"][ii]["NodesQueried"] = (pOZWHardware->m_awakeNodesQueried || pOZWHardware->m_allNodesQueried);
							}
#endif
						}
					}
					ii++;
				}
			}
		}

		void CWebServer::RType_Devices(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string rfilter = request::findValue(&req, "filter");
			std::string order = request::findValue(&req, "order");
			std::string rused = request::findValue(&req, "used");
			std::string rid = request::findValue(&req, "rid");
			std::string planid = request::findValue(&req, "plan");
			std::string floorid = request::findValue(&req, "floor");
			std::string sDisplayHidden = request::findValue(&req, "displayhidden");
			std::string sFetchFavorites = request::findValue(&req, "favorite");
			std::string sDisplayDisabled = request::findValue(&req, "displaydisabled");
			bool bDisplayHidden = (sDisplayHidden == "1");
			bool bFetchFavorites = (sFetchFavorites == "1");

			int HideDisabledHardwareSensors = 0;
			m_sql.GetPreferencesVar("HideDisabledHardwareSensors", HideDisabledHardwareSensors);
			bool bDisabledDisabled = (HideDisabledHardwareSensors == 0);
			if (sDisplayDisabled == "1")
				bDisabledDisabled = true;

			std::string sLastUpdate = request::findValue(&req, "lastupdate");
			std::string hwidx = request::findValue(&req, "hwidx"); // OTO

			time_t LastUpdate = 0;
			if (sLastUpdate != "")
			{
				std::stringstream sstr;
				sstr << sLastUpdate;
				sstr >> LastUpdate;
			}

			root["status"] = "OK";
			root["title"] = "Devices";
			root["app_version"] = szAppVersion;
			//GetJSonDevices(root, rused, rfilter, order, rid, planid, floorid, bDisplayHidden, bDisabledDisabled, bFetchFavorites, LastUpdate, session.username, hwidx);
		}

		void CWebServer::RType_Users(WebEmSession & session, const request& req, Json::Value &root)
		{
			bool bHaveUser = (session.username != "");
			int urights = 3;
			if (bHaveUser)
			{
				int iUser = FindUser(session.username.c_str());
				if (iUser != -1)
					urights = static_cast<int>(m_users[iUser].userrights);
			}
			if (urights < 2)
				return;

			root["status"] = "OK";
			root["title"] = "Users";

			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT ID, Active, Username, Password, Rights, RemoteSharing, TabsEnabled FROM USERS ORDER BY ID ASC");
			if (!result.empty())
			{
				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					root["result"][ii]["idx"] = sd[0];
					root["result"][ii]["Enabled"] = (sd[1] == "1") ? "true" : "false";
					root["result"][ii]["Username"] = base64_decode(sd[2]);
					root["result"][ii]["Password"] = sd[3];
					root["result"][ii]["Rights"] = atoi(sd[4].c_str());
					root["result"][ii]["RemoteSharing"] = atoi(sd[5].c_str());
					root["result"][ii]["TabsEnabled"] = atoi(sd[6].c_str());
					ii++;
				}
			}
		}

		void CWebServer::RType_Mobiles(WebEmSession & session, const request& req, Json::Value &root)
		{
			bool bHaveUser = (session.username != "");
			int urights = 3;
			if (bHaveUser)
			{
				int iUser = FindUser(session.username.c_str());
				if (iUser != -1)
					urights = static_cast<int>(m_users[iUser].userrights);
			}
			if (urights < 2)
				return;

			root["status"] = "OK";
			root["title"] = "Mobiles";

			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT ID, Active, Name, UUID, LastUpdate, DeviceType FROM MobileDevices ORDER BY Name ASC");
			if (!result.empty())
			{
				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					root["result"][ii]["idx"] = sd[0];
					root["result"][ii]["Enabled"] = (sd[1] == "1") ? "true" : "false";
					root["result"][ii]["Name"] = sd[2];
					root["result"][ii]["UUID"] = sd[3];
					root["result"][ii]["LastUpdate"] = sd[4];
					root["result"][ii]["DeviceType"] = sd[5];
					ii++;
				}
			}
		}

		void CWebServer::Cmd_GetSceneActivations(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;

			root["status"] = "OK";
			root["title"] = "GetSceneActivations";

			std::vector<std::vector<std::string> > result, result2;
			result = m_sql.safe_query("SELECT Activators, SceneType FROM Scenes WHERE (ID==%q)", idx.c_str());
			if (result.empty())
				return;
			int ii = 0;
			std::string Activators = result[0][0];
			int SceneType = atoi(result[0][1].c_str());
			if (!Activators.empty())
			{
				//Get Activator device names
				std::vector<std::string> arrayActivators;
				StringSplit(Activators, ";", arrayActivators);
				for (const auto & ittAct : arrayActivators)
				{
					std::string sCodeCmd = ittAct;

					std::vector<std::string> arrayCode;
					StringSplit(sCodeCmd, ":", arrayCode);

					std::string sID = arrayCode[0];
					int sCode = 0;
					if (arrayCode.size() == 2)
					{
						sCode = atoi(arrayCode[1].c_str());
					}


					result2 = m_sql.safe_query("SELECT Name, [Type], SubType, SwitchType FROM DeviceStatus WHERE (ID==%q)", sID.c_str());
					if (!result2.empty())
					{
						std::vector<std::string> sd = result2[0];
						std::string lstatus = "-";
						if ((SceneType == 0) && (arrayCode.size() == 2))
						{
							unsigned char devType = (unsigned char)atoi(sd[1].c_str());
							unsigned char subType = (unsigned char)atoi(sd[2].c_str());
							_eSwitchType switchtype = (_eSwitchType)atoi(sd[3].c_str());
							int nValue = sCode;
							std::string sValue = "";
							int llevel = 0;
							bool bHaveDimmer = false;
							bool bHaveGroupCmd = false;
							int maxDimLevel = 0;
							GetLightStatus(devType, subType, switchtype, nValue, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);
						}
						uint64_t dID = std::strtoull(sID.c_str(), nullptr, 10);
						root["result"][ii]["idx"] = dID;
						root["result"][ii]["name"] = sd[0];
						root["result"][ii]["code"] = sCode;
						root["result"][ii]["codestr"] = lstatus;
						ii++;
					}
				}
			}
		}

		void CWebServer::Cmd_AddSceneCode(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string sceneidx = request::findValue(&req, "sceneidx");
			std::string idx = request::findValue(&req, "idx");
			std::string cmnd = request::findValue(&req, "cmnd");
			if (
				(sceneidx.empty()) ||
				(idx.empty()) ||
				(cmnd.empty())
				)
				return;
			root["status"] = "OK";
			root["title"] = "AddSceneCode";

			//First check if we do not already have this device as activation code
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT Activators, SceneType FROM Scenes WHERE (ID==%q)", sceneidx.c_str());
			if (result.empty())
				return;
			std::string Activators = result[0][0];
			unsigned char scenetype = atoi(result[0][1].c_str());

			if (!Activators.empty())
			{
				//Get Activator device names
				std::vector<std::string> arrayActivators;
				StringSplit(Activators, ";", arrayActivators);
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

					if (sID == idx)
					{
						if (scenetype == 1)
							return; //Group does not work with separate codes, so already there
						if (sCode == cmnd)
							return; //same code, already there!
					}
				}
			}
			if (!Activators.empty())
				Activators += ";";
			Activators += idx;
			if (scenetype == 0)
			{
				Activators += ":" + cmnd;
			}
			m_sql.safe_query("UPDATE Scenes SET Activators='%q' WHERE (ID==%q)", Activators.c_str(), sceneidx.c_str());
		}

		void CWebServer::Cmd_RemoveSceneCode(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string sceneidx = request::findValue(&req, "sceneidx");
			std::string idx = request::findValue(&req, "idx");
			std::string code = request::findValue(&req, "code");
			if (
				(idx.empty()) ||
				(sceneidx.empty()) ||
				(code.empty())
				)
				return;
			root["status"] = "OK";
			root["title"] = "RemoveSceneCode";

			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT Activators, SceneType FROM Scenes WHERE (ID==%q)", sceneidx.c_str());
			if (result.empty())
				return;
			std::string Activators = result[0][0];
			int SceneType = atoi(result[0][1].c_str());
			if (!Activators.empty())
			{
				//Get Activator device names
				std::vector<std::string> arrayActivators;
				StringSplit(Activators, ";", arrayActivators);
				std::string newActivation = "";
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
					bool bFound = false;
					if (sID == idx)
					{
						if ((SceneType == 1) || (sCode.empty()))
						{
							bFound = true;
						}
						else
						{
							//Also check the code
							bFound = (sCode == code);
						}
					}
					if (!bFound)
					{
						if (!newActivation.empty())
							newActivation += ";";
						newActivation += sID;
						if ((SceneType == 0) && (!sCode.empty()))
						{
							newActivation += ":" + sCode;
						}
					}
				}
				if (Activators != newActivation)
				{
					m_sql.safe_query("UPDATE Scenes SET Activators='%q' WHERE (ID==%q)", newActivation.c_str(), sceneidx.c_str());
				}
			}
		}

		void CWebServer::Cmd_ClearSceneCodes(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string sceneidx = request::findValue(&req, "sceneidx");
			if (sceneidx.empty())
				return;
			root["status"] = "OK";
			root["title"] = "ClearSceneCode";

			m_sql.safe_query("UPDATE Scenes SET Activators='' WHERE (ID==%q)", sceneidx.c_str());
		}

		void CWebServer::Cmd_GetSerialDevices(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetSerialDevices";

			bool bUseDirectPath = false;
			std::vector<std::string> serialports = GetSerialPorts(bUseDirectPath);
			int ii = 0;
			for (const auto & itt : serialports)
			{
				root["result"][ii]["name"] = itt;
				root["result"][ii]["value"] = ii;
				ii++;
			}
		}

		void CWebServer::Cmd_GetDevicesList(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetDevicesList";
			int ii = 0;
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT ID, Name FROM DeviceStatus WHERE (Used == 1) ORDER BY Name");
			if (!result.empty())
			{
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;
					root["result"][ii]["name"] = sd[1];
					root["result"][ii]["value"] = sd[0];
					ii++;
				}
			}
		}

		void CWebServer::Post_UploadCustomIcon(WebEmSession & session, const request& req, reply & rep)
		{
			Json::Value root;
			root["title"] = "UploadCustomIcon";
			root["status"] = "ERROR";
			root["error"] = "Invalid";
			//Only admin user allowed
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
			std::string zipfile = request::findValue(&req, "file");
			if (zipfile != "")
			{
				std::string ErrorMessage;
				bool bOK = m_sql.InsertCustomIconFromZip(zipfile, ErrorMessage);
				if (bOK)
				{
					root["status"] = "OK";
				}
				else
				{
					root["status"] = "ERROR";
					root["error"] = ErrorMessage;
				}
			}
			std::string jcallback = request::findValue(&req, "jsoncallback");
			if (jcallback.size() == 0) {
				reply::set_content(&rep, root.toStyledString());
				return;
			}
			reply::set_content(&rep, "var data=" + root.toStyledString() + '\n' + jcallback + "(data);");
		}

		void CWebServer::Cmd_GetCustomIconSet(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetCustomIconSet";
			int ii = 0;
			for (const auto & itt : m_custom_light_icons)
			{
				if (itt.idx >= 100)
				{
					std::string IconFile16 = "images/" + itt.RootFile + ".png";
					std::string IconFile48On = "images/" + itt.RootFile + "48_On.png";
					std::string IconFile48Off = "images/" + itt.RootFile + "48_Off.png";

					root["result"][ii]["idx"] = itt.idx - 100;
					root["result"][ii]["Title"] = itt.Title;
					root["result"][ii]["Description"] = itt.Description;
					root["result"][ii]["IconFile16"] = IconFile16;
					root["result"][ii]["IconFile48On"] = IconFile48On;
					root["result"][ii]["IconFile48Off"] = IconFile48Off;
					ii++;
				}
			}
		}

		void CWebServer::Cmd_DeleteCustomIcon(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string sidx = request::findValue(&req, "idx");
			if (sidx.empty())
				return;
			int idx = atoi(sidx.c_str());
			root["status"] = "OK";
			root["title"] = "DeleteCustomIcon";

			m_sql.safe_query("DELETE FROM CustomImages WHERE (ID == %d)", idx);

			//Delete icons file from disk
			for (const auto & itt : m_custom_light_icons)
			{
				if (itt.idx == idx + 100)
				{
					std::string IconFile16 = szWWWFolder + "/images/" + itt.RootFile + ".png";
					std::string IconFile48On = szWWWFolder + "/images/" + itt.RootFile + "48_On.png";
					std::string IconFile48Off = szWWWFolder + "/images/" + itt.RootFile + "48_Off.png";
					std::remove(IconFile16.c_str());
					std::remove(IconFile48On.c_str());
					std::remove(IconFile48Off.c_str());
					break;
				}
			}
			ReloadCustomSwitchIcons();
		}

		void CWebServer::Cmd_UpdateCustomIcon(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string sidx = request::findValue(&req, "idx");
			std::string sname = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));
			std::string sdescription = HTMLSanitizer::Sanitize(request::findValue(&req, "description"));
			if (
				(sidx.empty()) ||
				(sname.empty()) ||
				(sdescription.empty())
				)
				return;

			int idx = atoi(sidx.c_str());
			root["status"] = "OK";
			root["title"] = "UpdateCustomIcon";

			m_sql.safe_query("UPDATE CustomImages SET Name='%q', Description='%q' WHERE (ID == %d)", sname.c_str(), sdescription.c_str(), idx);
			ReloadCustomSwitchIcons();
		}

		void CWebServer::Cmd_RenameDevice(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string sidx = request::findValue(&req, "idx");
			std::string sname = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));
			if (
				(sidx.empty()) ||
				(sname.empty())
				)
				return;
			int idx = atoi(sidx.c_str());
			root["status"] = "OK";
			root["title"] = "RenameDevice";

			m_sql.safe_query("UPDATE DeviceStatus SET Name='%q' WHERE (ID == %d)", sname.c_str(), idx);
			uint64_t ullidx = std::strtoull(sidx.c_str(), nullptr, 10);
			m_mainworker.m_eventsystem.WWWUpdateSingleState(ullidx, sname, m_mainworker.m_eventsystem.REASON_DEVICE);

#ifdef ENABLE_PYTHON
			// Notify plugin framework about the change
			m_mainworker.m_pluginsystem.DeviceModified(idx);
#endif
		}

		void CWebServer::Cmd_RenameScene(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string sidx = request::findValue(&req, "idx");
			std::string sname = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));
			if (
				(sidx.empty()) ||
				(sname.empty())
				)
				return;
			int idx = atoi(sidx.c_str());
			root["status"] = "OK";
			root["title"] = "RenameScene";

			m_sql.safe_query("UPDATE Scenes SET Name='%q' WHERE (ID == %d)", sname.c_str(), idx);
			uint64_t ullidx = std::strtoull(sidx.c_str(), nullptr, 10);
			m_mainworker.m_eventsystem.WWWUpdateSingleState(ullidx, sname, m_mainworker.m_eventsystem.REASON_SCENEGROUP);
		}

		void CWebServer::Cmd_SetUnused(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string sidx = request::findValue(&req, "idx");
			if (sidx.empty())
				return;
			int idx = atoi(sidx.c_str());
			root["status"] = "OK";
			root["title"] = "SetUnused";
			m_sql.safe_query("UPDATE DeviceStatus SET Used=0 WHERE (ID == %d)", idx);
			m_mainworker.m_eventsystem.RemoveSingleState(idx, m_mainworker.m_eventsystem.REASON_DEVICE);

#ifdef ENABLE_PYTHON
			// Notify plugin framework about the change
			m_mainworker.m_pluginsystem.DeviceModified(idx);
#endif
		}

		void CWebServer::Cmd_AddLogMessage(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string smessage = request::findValue(&req, "message");
			if (smessage.empty())
				return;
			root["status"] = "OK";
			root["title"] = "AddLogMessage";

			_log.Log(LOG_STATUS, "%s", smessage.c_str());
		}

		void CWebServer::Cmd_ClearShortLog(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
			root["status"] = "OK";
			root["title"] = "ClearShortLog";

			_log.Log(LOG_STATUS, "Clearing Short Log...");

			m_sql.ClearShortLog();

			_log.Log(LOG_STATUS, "Short Log Cleared!");
		}

		void CWebServer::Cmd_VacuumDatabase(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
			root["status"] = "OK";
			root["title"] = "VacuumDatabase";

			m_sql.VacuumDatabase();
		}

		void CWebServer::Cmd_AddMobileDevice(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string suuid = HTMLSanitizer::Sanitize(request::findValue(&req, "uuid"));
			std::string ssenderid = HTMLSanitizer::Sanitize(request::findValue(&req, "senderid"));
			std::string sname = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));
			std::string sdevtype = HTMLSanitizer::Sanitize(request::findValue(&req, "devicetype"));
			std::string sactive = request::findValue(&req, "active");
			if (
				(suuid.empty()) ||
				(ssenderid.empty())
				)
				return;
			root["status"] = "OK";
			root["title"] = "AddMobileDevice";

			if (sactive.empty())
				sactive = "1";
			int iActive = (sactive == "1") ? 1 : 0;

			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT ID, Name, DeviceType FROM MobileDevices WHERE (UUID=='%q')", suuid.c_str());
			if (result.empty())
			{
				//New
				m_sql.safe_query("INSERT INTO MobileDevices (Active,UUID,SenderID,Name,DeviceType) VALUES (%d,'%q','%q','%q','%q')",
					iActive,
					suuid.c_str(),
					ssenderid.c_str(),
					sname.c_str(),
					sdevtype.c_str());
			}
			else
			{
				//Update
				time_t now = mytime(NULL);
				struct tm ltime;
				localtime_r(&now, &ltime);
				m_sql.safe_query("UPDATE MobileDevices SET Active=%d, SenderID='%q', LastUpdate='%04d-%02d-%02d %02d:%02d:%02d' WHERE (UUID == '%q')",
					iActive,
					ssenderid.c_str(),
					ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday, ltime.tm_hour, ltime.tm_min, ltime.tm_sec,
					suuid.c_str()
				);

				std::string dname = result[0][1];
				std::string ddevtype = result[0][2];
				if (dname.empty() || ddevtype.empty())
				{
					m_sql.safe_query("UPDATE MobileDevices SET Name='%q', DeviceType='%q' WHERE (UUID == '%q')",
						sname.c_str(), sdevtype.c_str(),
						suuid.c_str()
					);
				}
			}
		}

		void CWebServer::Cmd_UpdateMobileDevice(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
			std::string sidx = request::findValue(&req, "idx");
			std::string enabled = request::findValue(&req, "enabled");
			std::string name = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));

			if (
				(sidx.empty()) ||
				(enabled.empty()) ||
				(name.empty())
				)
				return;
			uint64_t idx = std::strtoull(sidx.c_str(), nullptr, 10);

			m_sql.safe_query("UPDATE MobileDevices SET Name='%q', Active=%d WHERE (ID==%" PRIu64 ")",
				name.c_str(), (enabled == "true") ? 1 : 0, idx);

			root["status"] = "OK";
			root["title"] = "UpdateMobile";
		}

		void CWebServer::Cmd_DeleteMobileDevice(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}
			std::string suuid = request::findValue(&req, "uuid");
			if (suuid.empty())
				return;
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT ID FROM MobileDevices WHERE (UUID=='%q')", suuid.c_str());
			if (result.empty())
				return;
			m_sql.safe_query("DELETE FROM MobileDevices WHERE (UUID == '%q')", suuid.c_str());
			root["status"] = "OK";
			root["title"] = "DeleteMobileDevice";
		}


		void CWebServer::RType_GetTransfers(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "GetTransfers";

			uint64_t idx = 0;
			if (request::findValue(&req, "idx") != "")
			{
				idx = std::strtoull(request::findValue(&req, "idx").c_str(), nullptr, 10);
			}

			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT Type, SubType FROM DeviceStatus WHERE (ID==%" PRIu64 ")",
				idx);
			if (!result.empty())
			{
				int dType = atoi(result[0][0].c_str());
				if (
					(dType == pTypeTEMP) ||
					(dType == pTypeTEMP_HUM)
					)
				{
					result = m_sql.safe_query(
						"SELECT ID, Name FROM DeviceStatus WHERE (Type=='%q') AND (ID!=%" PRIu64 ")",
						result[0][0].c_str(), idx);
				}
				else
				{
					result = m_sql.safe_query(
						"SELECT ID, Name FROM DeviceStatus WHERE (Type=='%q') AND (SubType=='%q') AND (ID!=%" PRIu64 ")",
						result[0][0].c_str(), result[0][1].c_str(), idx);
				}

				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					root["result"][ii]["idx"] = sd[0];
					root["result"][ii]["Name"] = sd[1];
					ii++;
				}
			}
		}

		//Will transfer Newest sensor log to OLD sensor,
		//then set the HardwareID/DeviceID/Unit/Name/Type/Subtype/Unit for the OLD sensor to the NEW sensor ID/Type/Subtype/Unit
		//then delete the NEW sensor
		void CWebServer::RType_TransferDevice(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string sidx = request::findValue(&req, "idx");
			if (sidx.empty())
				return;

			std::string newidx = request::findValue(&req, "newidx");
			if (newidx.empty())
				return;

			std::vector<std::vector<std::string> > result;

			//Check which device is newer

			time_t now = mytime(NULL);
			struct tm tm1;
			localtime_r(&now, &tm1);
			struct tm LastUpdateTime_A;
			struct tm LastUpdateTime_B;

			result = m_sql.safe_query(
				"SELECT A.LastUpdate, B.LastUpdate FROM DeviceStatus as A, DeviceStatus as B WHERE (A.ID == '%q') AND (B.ID == '%q')",
				sidx.c_str(), newidx.c_str());
			if (result.empty())
				return;

			std::string sLastUpdate_A = result[0][0];
			std::string sLastUpdate_B = result[0][1];

			time_t timeA, timeB;
			ParseSQLdatetime(timeA, LastUpdateTime_A, sLastUpdate_A, tm1.tm_isdst);
			ParseSQLdatetime(timeB, LastUpdateTime_B, sLastUpdate_B, tm1.tm_isdst);

			if (timeA < timeB)
			{
				//Swap idx with newidx
				sidx.swap(newidx);
			}

			result = m_sql.safe_query(
				"SELECT HardwareID, DeviceID, Unit, Name, Type, SubType, SignalLevel, BatteryLevel, nValue, sValue FROM DeviceStatus WHERE (ID == '%q')",
				newidx.c_str());
			if (result.empty())
				return;

			root["status"] = "OK";
			root["title"] = "TransferDevice";

			//transfer device logs (new to old)
			m_sql.TransferDevice(newidx, sidx);

			//now delete the NEW device
			m_sql.DeleteDevices(newidx);

			m_mainworker.m_scheduler.ReloadSchedules();
		}

		void CWebServer::RType_Notifications(WebEmSession & session, const request& req, Json::Value &root)
		{
			root["status"] = "OK";
			root["title"] = "Notifications";

			int ii = 0;

			//Add known notification systems
			for (const auto & ittNotifiers : m_notifications.m_notifiers)
			{
				root["notifiers"][ii]["name"] = ittNotifiers.first;
				root["notifiers"][ii]["description"] = ittNotifiers.first;
				ii++;
			}

			uint64_t idx = 0;
			if (request::findValue(&req, "idx") != "")
			{
				idx = std::strtoull(request::findValue(&req, "idx").c_str(), nullptr, 10);
			}
			std::vector<_tNotification> notifications = m_notifications.GetNotifications(idx);
			if (notifications.size() > 0)
			{
				ii = 0;
				for (const auto & itt : notifications)
				{
					root["result"][ii]["idx"] = itt.ID;
					std::string sParams = itt.Params;
					if (sParams.empty()) {
						sParams = "S";
					}
					root["result"][ii]["Params"] = sParams;
					root["result"][ii]["Priority"] = itt.Priority;
					root["result"][ii]["SendAlways"] = itt.SendAlways;
					root["result"][ii]["CustomMessage"] = itt.CustomMessage;
					root["result"][ii]["ActiveSystems"] = itt.ActiveSystems;
					ii++;
				}
			}
		}

		void CWebServer::RType_GetSharedUserDevices(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string idx = request::findValue(&req, "idx");
			if (idx.empty())
				return;
			root["status"] = "OK";
			root["title"] = "GetSharedUserDevices";

			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT DeviceRowID FROM SharedDevices WHERE (SharedUserID == '%q')", idx.c_str());
			if (!result.empty())
			{
				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;
					root["result"][ii]["DeviceRowIdx"] = sd[0];
					ii++;
				}
			}
		}

		void CWebServer::RType_SetSharedUserDevices(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::string idx = request::findValue(&req, "idx");
			std::string userdevices = request::findValue(&req, "devices");
			if (idx.empty())
				return;
			root["status"] = "OK";
			root["title"] = "SetSharedUserDevices";
			std::vector<std::string> strarray;
			StringSplit(userdevices, ";", strarray);

			//First delete all devices for this user, then add the (new) onces
			m_sql.safe_query("DELETE FROM SharedDevices WHERE (SharedUserID == '%q')", idx.c_str());

			int nDevices = static_cast<int>(strarray.size());
			for (int ii = 0; ii < nDevices; ii++)
			{
				m_sql.safe_query("INSERT INTO SharedDevices (SharedUserID,DeviceRowID) VALUES ('%q','%q')", idx.c_str(), strarray[ii].c_str());
			}
			LoadUsers();
		}

		void CWebServer::RType_SetUsed(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string idx = request::findValue(&req, "idx");
			std::string deviceid = request::findValue(&req, "deviceid");
			std::string name = HTMLSanitizer::Sanitize(request::findValue(&req, "name"));
			std::string description = HTMLSanitizer::Sanitize(request::findValue(&req, "description"));
			std::string sused = request::findValue(&req, "used");
			std::string sswitchtype = request::findValue(&req, "switchtype");
			std::string maindeviceidx = request::findValue(&req, "maindeviceidx");
			std::string addjvalue = request::findValue(&req, "addjvalue");
			std::string addjmulti = request::findValue(&req, "addjmulti");
			std::string addjvalue2 = request::findValue(&req, "addjvalue2");
			std::string addjmulti2 = request::findValue(&req, "addjmulti2");
			std::string setPoint = request::findValue(&req, "setpoint");
			std::string state = request::findValue(&req, "state");
			std::string mode = request::findValue(&req, "mode");
			std::string until = request::findValue(&req, "until");
			std::string clock = request::findValue(&req, "clock");
			std::string tmode = request::findValue(&req, "tmode");
			std::string fmode = request::findValue(&req, "fmode");
			std::string sCustomImage = request::findValue(&req, "customimage");

			std::string strunit = request::findValue(&req, "unit");
			std::string strParam1 = HTMLSanitizer::Sanitize(base64_decode(request::findValue(&req, "strparam1")));
			std::string strParam2 = HTMLSanitizer::Sanitize(base64_decode(request::findValue(&req, "strparam2")));
			std::string tmpstr = request::findValue(&req, "protected");
			bool bHasstrParam1 = request::hasValue(&req, "strparam1");
			int iProtected = (tmpstr == "true") ? 1 : 0;

			std::string sOptions = HTMLSanitizer::Sanitize(base64_decode(request::findValue(&req, "options")));
			std::string devoptions = HTMLSanitizer::Sanitize(CURLEncode::URLDecode(request::findValue(&req, "devoptions")));
			std::string EnergyMeterMode = CURLEncode::URLDecode(request::findValue(&req, "EnergyMeterMode"));

			char szTmp[200];

			bool bHaveUser = (session.username != "");
			int iUser = -1;
			if (bHaveUser)
			{
				iUser = FindUser(session.username.c_str());
			}

			int switchtype = -1;
			if (sswitchtype != "")
				switchtype = atoi(sswitchtype.c_str());

			if ((idx.empty()) || (sused.empty()))
				return;
			int used = (sused == "true") ? 1 : 0;
			if (maindeviceidx != "")
				used = 0;

			int CustomImage = 0;
			if (sCustomImage != "")
				CustomImage = atoi(sCustomImage.c_str());

			//Strip trailing spaces in 'name'
			name = stdstring_trim(name);

			//Strip trailing spaces in 'description'
			description = stdstring_trim(description);

			std::vector<std::vector<std::string> > result;

			result = m_sql.safe_query("SELECT Type,SubType,HardwareID FROM DeviceStatus WHERE (ID == '%q')", idx.c_str());
			if (result.empty())
				return;
			std::vector<std::string> sd = result[0];

			unsigned char dType = atoi(sd[0].c_str());
			//unsigned char dSubType=atoi(sd[1].c_str());
			int HwdID = atoi(sd[2].c_str());
			std::string sHwdID = sd[2];

			if (setPoint != "" || state != "")
			{
				double tempcelcius = atof(setPoint.c_str());
				sprintf(szTmp, "%.2f", tempcelcius);

				if (dType != pTypeEvohomeZone && dType != pTypeEvohomeWater)//sql update now done in setsetpoint for evohome devices
				{
					m_sql.safe_query("UPDATE DeviceStatus SET Used=%d, sValue='%q' WHERE (ID == '%q')",
						used, szTmp, idx.c_str());
				}
			}
			if (name.empty())
			{
				m_sql.safe_query("UPDATE DeviceStatus SET Used=%d WHERE (ID == '%q')",
					used, idx.c_str());
			}
			else
			{
				if (switchtype == -1)
				{
					m_sql.safe_query("UPDATE DeviceStatus SET Used=%d, Name='%q', Description='%q' WHERE (ID == '%q')",
						used, name.c_str(), description.c_str(), idx.c_str());
				}
				else
				{
					m_sql.safe_query(
						"UPDATE DeviceStatus SET Used=%d, Name='%q', Description='%q', SwitchType=%d, CustomImage=%d WHERE (ID == '%q')",
						used, name.c_str(), description.c_str(), switchtype, CustomImage, idx.c_str());
				}
			}

			if (bHasstrParam1)
			{
				m_sql.safe_query("UPDATE DeviceStatus SET StrParam1='%q', StrParam2='%q' WHERE (ID == '%q')",
					strParam1.c_str(), strParam2.c_str(), idx.c_str());
			}

			m_sql.safe_query("UPDATE DeviceStatus SET Protected=%d WHERE (ID == '%q')", iProtected, idx.c_str());

			if (!setPoint.empty() || !state.empty())
			{
				int urights = 3;
				if (bHaveUser)
				{
					int iUser = FindUser(session.username.c_str());
					if (iUser != -1)
					{
						urights = static_cast<int>(m_users[iUser].userrights);
						_log.Log(LOG_STATUS, "User: %s initiated a SetPoint command", m_users[iUser].Username.c_str());
					}
				}
				if (urights < 1)
					return;
			}
			else if (!clock.empty())
			{
				int urights = 3;
				if (bHaveUser)
				{
					int iUser = FindUser(session.username.c_str());
					if (iUser != -1)
					{
						urights = static_cast<int>(m_users[iUser].userrights);
						_log.Log(LOG_STATUS, "User: %s initiated a SetClock command", m_users[iUser].Username.c_str());
					}
				}
				if (urights < 1)
					return;
				m_mainworker.SetClock(idx, clock);
			}
			else if (!tmode.empty())
			{
				int urights = 3;
				if (bHaveUser)
				{
					int iUser = FindUser(session.username.c_str());
					if (iUser != -1)
					{
						urights = static_cast<int>(m_users[iUser].userrights);
						_log.Log(LOG_STATUS, "User: %s initiated a Thermostat Mode command", m_users[iUser].Username.c_str());
					}
				}
				if (urights < 1)
					return;
				m_mainworker.SetZWaveThermostatMode(idx, atoi(tmode.c_str()));
			}
			else if (!fmode.empty())
			{
				int urights = 3;
				if (bHaveUser)
				{
					int iUser = FindUser(session.username.c_str());
					if (iUser != -1)
					{
						urights = static_cast<int>(m_users[iUser].userrights);
						_log.Log(LOG_STATUS, "User: %s initiated a Thermostat Fan Mode command", m_users[iUser].Username.c_str());
					}
				}
				if (urights < 1)
					return;
				m_mainworker.SetZWaveThermostatFanMode(idx, atoi(fmode.c_str()));
			}

			if (!strunit.empty())
			{
				bool bUpdateUnit = true;
#ifdef ENABLE_PYTHON
				//check if HW is plugin
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT Type FROM Hardware WHERE (ID == %d)", HwdID);
				if (!result.empty())
				{
					std::vector<std::string> sd = result[0];
					_eHardwareTypes Type = (_eHardwareTypes)atoi(sd[0].c_str());
					if (Type == HTYPE_PythonPlugin)
					{
						bUpdateUnit = false;
						_log.Log(LOG_ERROR, "CWebServer::RType_SetUsed: Not allowed to change unit of device owned by plugin %u!", HwdID);
					}
				}
#endif
				if (bUpdateUnit)
				{
					m_sql.safe_query("UPDATE DeviceStatus SET Unit='%q' WHERE (ID == '%q')",
						strunit.c_str(), idx.c_str());
				}
			}
			//FIXME evohome ...we need the zone id to update the correct zone...but this should be ok as a generic call?
			if (!deviceid.empty())
			{
				m_sql.safe_query("UPDATE DeviceStatus SET DeviceID='%q' WHERE (ID == '%q')",
					deviceid.c_str(), idx.c_str());
			}
			if (!addjvalue.empty())
			{
				double faddjvalue = atof(addjvalue.c_str());
				m_sql.safe_query("UPDATE DeviceStatus SET AddjValue=%f WHERE (ID == '%q')",
					faddjvalue, idx.c_str());
			}
			if (!addjmulti.empty())
			{
				double faddjmulti = atof(addjmulti.c_str());
				if (faddjmulti == 0)
					faddjmulti = 1;
				m_sql.safe_query("UPDATE DeviceStatus SET AddjMulti=%f WHERE (ID == '%q')",
					faddjmulti, idx.c_str());
			}
			if (!addjvalue2.empty())
			{
				double faddjvalue2 = atof(addjvalue2.c_str());
				m_sql.safe_query("UPDATE DeviceStatus SET AddjValue2=%f WHERE (ID == '%q')",
					faddjvalue2, idx.c_str());
			}
			if (!addjmulti2.empty())
			{
				double faddjmulti2 = atof(addjmulti2.c_str());
				if (faddjmulti2 == 0)
					faddjmulti2 = 1;
				m_sql.safe_query("UPDATE DeviceStatus SET AddjMulti2=%f WHERE (ID == '%q')",
					faddjmulti2, idx.c_str());
			}
			if (!EnergyMeterMode.empty())
			{
				auto options = m_sql.GetDeviceOptions(idx);
				options["EnergyMeterMode"] = EnergyMeterMode;
				uint64_t ullidx = std::strtoull(idx.c_str(), nullptr, 10);
				m_sql.SetDeviceOptions(ullidx, options);
			}

			if (!devoptions.empty())
			{
				m_sql.safe_query("UPDATE DeviceStatus SET Options='%q' WHERE (ID == '%q')", devoptions.c_str(), idx.c_str());
			}

			if (used == 0)
			{
				bool bRemoveSubDevices = (request::findValue(&req, "RemoveSubDevices") == "true");

				if (bRemoveSubDevices)
				{
					//if this device was a slave device, remove it
					m_sql.safe_query("DELETE FROM LightSubDevices WHERE (DeviceRowID == '%q')", idx.c_str());
				}
				m_sql.safe_query("DELETE FROM LightSubDevices WHERE (ParentID == '%q')", idx.c_str());

				m_sql.safe_query("DELETE FROM Timers WHERE (DeviceRowID == '%q')", idx.c_str());
			}

			// Save device options
			if (!sOptions.empty())
			{
				uint64_t ullidx = std::strtoull(idx.c_str(), nullptr, 10);
				m_sql.SetDeviceOptions(ullidx, m_sql.BuildDeviceOptions(sOptions, false));
			}

			if (maindeviceidx != "")
			{
				if (maindeviceidx != idx)
				{
					//this is a sub device for another light/switch
					//first check if it is not already a sub device
					result = m_sql.safe_query("SELECT ID FROM LightSubDevices WHERE (DeviceRowID=='%q') AND (ParentID =='%q')",
						idx.c_str(), maindeviceidx.c_str());
					if (result.empty())
					{
						//no it is not, add it
						m_sql.safe_query(
							"INSERT INTO LightSubDevices (DeviceRowID, ParentID) VALUES ('%q','%q')",
							idx.c_str(),
							maindeviceidx.c_str()
						);
					}
				}
			}
			if ((used == 0) && (maindeviceidx.empty()))
			{
				//really remove it, including log etc
				m_sql.DeleteDevices(idx);
			}
			else
			{
#ifdef ENABLE_PYTHON
				// Notify plugin framework about the change
				m_mainworker.m_pluginsystem.DeviceModified(atoi(idx.c_str()));
#endif
			}
			if (!result.empty())
			{
				root["status"] = "OK";
				root["title"] = "SetUsed";
			}
			m_mainworker.m_eventsystem.GetCurrentStates();
		}

		void CWebServer::RType_Settings(WebEmSession & session, const request& req, Json::Value &root)
		{
			std::vector<std::vector<std::string> > result;
			char szTmp[100];

			result = m_sql.safe_query("SELECT Key, nValue, sValue FROM Preferences");
			if (result.empty())
				return;
			root["status"] = "OK";
			root["title"] = "settings";
#ifndef NOCLOUD
			root["cloudenabled"] = true;
#else
			root["cloudenabled"] = false;
#endif

			for (const auto & itt : result)
			{
				std::vector<std::string> sd = itt;
				std::string Key = sd[0];
				int nValue = atoi(sd[1].c_str());
				std::string sValue = sd[2];

				if (Key == "Location")
				{
					std::vector<std::string> strarray;
					StringSplit(sValue, ";", strarray);

					if (strarray.size() == 2)
					{
						root["Location"]["Latitude"] = strarray[0];
						root["Location"]["Longitude"] = strarray[1];
					}
				}
				/* RK: notification settings */
				if (m_notifications.IsInConfig(Key)) {
					if (sValue.empty() && nValue > 0) {
						root[Key] = nValue;
					}
					else {
						root[Key] = sValue;
					}
				}
				else if (Key == "DashboardType")
				{
					root["DashboardType"] = nValue;
				}
				else if (Key == "MobileType")
				{
					root["MobileType"] = nValue;
				}
				else if (Key == "LightHistoryDays")
				{
					root["LightHistoryDays"] = nValue;
				}
				else if (Key == "5MinuteHistoryDays")
				{
					root["ShortLogDays"] = nValue;
				}
				else if (Key == "ShortLogInterval")
				{
					root["ShortLogInterval"] = nValue;
				}
				else if (Key == "WebUserName")
				{
					root["WebUserName"] = base64_decode(sValue);
				}
				//else if (Key == "WebPassword")
				//{
				//	root["WebPassword"] = sValue;
				//}
				else if (Key == "SecPassword")
				{
					root["SecPassword"] = sValue;
				}
				else if (Key == "ProtectionPassword")
				{
					root["ProtectionPassword"] = sValue;
				}
				else if (Key == "WebLocalNetworks")
				{
					root["WebLocalNetworks"] = sValue;
				}
				else if (Key == "WebRemoteProxyIPs")
				{
					root["WebRemoteProxyIPs"] = sValue;
				}
				else if (Key == "RandomTimerFrame")
				{
					root["RandomTimerFrame"] = nValue;
				}
				else if (Key == "MeterDividerEnergy")
				{
					root["EnergyDivider"] = nValue;
				}
				else if (Key == "MeterDividerGas")
				{
					root["GasDivider"] = nValue;
				}
				else if (Key == "MeterDividerWater")
				{
					root["WaterDivider"] = nValue;
				}
				else if (Key == "ElectricVoltage")
				{
					root["ElectricVoltage"] = nValue;
				}
				else if (Key == "CM113DisplayType")
				{
					root["CM113DisplayType"] = nValue;
				}
				else if (Key == "UseAutoUpdate")
				{
					root["UseAutoUpdate"] = nValue;
				}
				else if (Key == "UseAutoBackup")
				{
					root["UseAutoBackup"] = nValue;
				}
				else if (Key == "Rego6XXType")
				{
					root["Rego6XXType"] = nValue;
				}
				else if (Key == "CostEnergy")
				{
					sprintf(szTmp, "%.4f", (float)(nValue) / 10000.0f);
					root["CostEnergy"] = szTmp;
				}
				else if (Key == "CostEnergyT2")
				{
					sprintf(szTmp, "%.4f", (float)(nValue) / 10000.0f);
					root["CostEnergyT2"] = szTmp;
				}
				else if (Key == "CostEnergyR1")
				{
					sprintf(szTmp, "%.4f", (float)(nValue) / 10000.0f);
					root["CostEnergyR1"] = szTmp;
				}
				else if (Key == "CostEnergyR2")
				{
					sprintf(szTmp, "%.4f", (float)(nValue) / 10000.0f);
					root["CostEnergyR2"] = szTmp;
				}
				else if (Key == "CostGas")
				{
					sprintf(szTmp, "%.4f", (float)(nValue) / 10000.0f);
					root["CostGas"] = szTmp;
				}
				else if (Key == "CostWater")
				{
					sprintf(szTmp, "%.4f", (float)(nValue) / 10000.0f);
					root["CostWater"] = szTmp;
				}
				else if (Key == "ActiveTimerPlan")
				{
					root["ActiveTimerPlan"] = nValue;
				}
				else if (Key == "DoorbellCommand")
				{
					root["DoorbellCommand"] = nValue;
				}
				else if (Key == "SmartMeterType")
				{
					root["SmartMeterType"] = nValue;
				}
				else if (Key == "EnableTabFloorplans")
				{
					root["EnableTabFloorplans"] = nValue;
				}
				else if (Key == "EnableTabLights")
				{
					root["EnableTabLights"] = nValue;
				}
				else if (Key == "EnableTabTemp")
				{
					root["EnableTabTemp"] = nValue;
				}
				else if (Key == "EnableTabWeather")
				{
					root["EnableTabWeather"] = nValue;
				}
				else if (Key == "EnableTabUtility")
				{
					root["EnableTabUtility"] = nValue;
				}
				else if (Key == "EnableTabScenes")
				{
					root["EnableTabScenes"] = nValue;
				}
				else if (Key == "EnableTabCustom")
				{
					root["EnableTabCustom"] = nValue;
				}
				else if (Key == "NotificationSensorInterval")
				{
					root["NotificationSensorInterval"] = nValue;
				}
				else if (Key == "NotificationSwitchInterval")
				{
					root["NotificationSwitchInterval"] = nValue;
				}
				else if (Key == "RemoteSharedPort")
				{
					root["RemoteSharedPort"] = nValue;
				}
				else if (Key == "Language")
				{
					root["Language"] = sValue;
				}
				else if (Key == "Title")
				{
					root["Title"] = sValue;
				}
				else if (Key == "WindUnit")
				{
					root["WindUnit"] = nValue;
				}
				else if (Key == "TempUnit")
				{
					root["TempUnit"] = nValue;
				}
				else if (Key == "WeightUnit")
				{
					root["WeightUnit"] = nValue;
				}
				else if (Key == "AuthenticationMethod")
				{
					root["AuthenticationMethod"] = nValue;
				}
				else if (Key == "ReleaseChannel")
				{
					root["ReleaseChannel"] = nValue;
				}
				else if (Key == "RaspCamParams")
				{
					root["RaspCamParams"] = sValue;
				}
				else if (Key == "UVCParams")
				{
					root["UVCParams"] = sValue;
				}
				else if (Key == "AcceptNewHardware")
				{
					root["AcceptNewHardware"] = nValue;
				}
				else if (Key == "HideDisabledHardwareSensors")
				{
					root["HideDisabledHardwareSensors"] = nValue;
				}
				else if (Key == "ShowUpdateEffect")
				{
					root["ShowUpdateEffect"] = nValue;
				}
				else if (Key == "DegreeDaysBaseTemperature")
				{
					root["DegreeDaysBaseTemperature"] = sValue;
				}
				else if (Key == "EnableEventScriptSystem")
				{
					root["EnableEventScriptSystem"] = nValue;
				}
				else if (Key == "EventSystemLogFullURL")
				{
					root["EventSystemLogFullURL"] = nValue;
				}
				else if (Key == "LogEventScriptTrigger")
				{
					root["LogEventScriptTrigger"] = nValue;
				}
				else if (Key == "(1WireSensorPollPeriod")
				{
					root["1WireSensorPollPeriod"] = nValue;
				}
				else if (Key == "(1WireSwitchPollPeriod")
				{
					root["1WireSwitchPollPeriod"] = nValue;
				}
				else if (Key == "SecOnDelay")
				{
					root["SecOnDelay"] = nValue;
				}
				else if (Key == "AllowWidgetOrdering")
				{
					root["AllowWidgetOrdering"] = nValue;
				}
				else if (Key == "FloorplanPopupDelay")
				{
					root["FloorplanPopupDelay"] = nValue;
				}
				else if (Key == "FloorplanFullscreenMode")
				{
					root["FloorplanFullscreenMode"] = nValue;
				}
				else if (Key == "FloorplanAnimateZoom")
				{
					root["FloorplanAnimateZoom"] = nValue;
				}
				else if (Key == "FloorplanShowSensorValues")
				{
					root["FloorplanShowSensorValues"] = nValue;
				}
				else if (Key == "FloorplanShowSwitchValues")
				{
					root["FloorplanShowSwitchValues"] = nValue;
				}
				else if (Key == "FloorplanShowSceneNames")
				{
					root["FloorplanShowSceneNames"] = nValue;
				}
				else if (Key == "FloorplanRoomColour")
				{
					root["FloorplanRoomColour"] = sValue;
				}
				else if (Key == "FloorplanActiveOpacity")
				{
					root["FloorplanActiveOpacity"] = nValue;
				}
				else if (Key == "FloorplanInactiveOpacity")
				{
					root["FloorplanInactiveOpacity"] = nValue;
				}
				else if (Key == "SensorTimeout")
				{
					root["SensorTimeout"] = nValue;
				}
				else if (Key == "BatteryLowNotification")
				{
					root["BatterLowLevel"] = nValue;
				}
				else if (Key == "WebTheme")
				{
					root["WebTheme"] = sValue;
				}
#ifndef NOCLOUD
				else if (Key == "MyDomoticzInstanceId") {
					root["MyDomoticzInstanceId"] = sValue;
				}
				else if (Key == "MyDomoticzUserId") {
					root["MyDomoticzUserId"] = sValue;
				}
				else if (Key == "MyDomoticzPassword") {
					root["MyDomoticzPassword"] = sValue;
				}
				else if (Key == "MyDomoticzSubsystems") {
					root["MyDomoticzSubsystems"] = nValue;
				}
#endif
				else if (Key == "MyDomoticzSubsystems") {
					root["MyDomoticzSubsystems"] = nValue;
				}
				else if (Key == "SendErrorsAsNotification") {
					root["SendErrorsAsNotification"] = nValue;
				}
				else if (Key == "DeltaTemperatureLog") {
					root[Key] = sValue;
				}
			}
		}

		void CWebServer::RType_LightLog(WebEmSession & session, const request& req, Json::Value &root)
		{
			uint64_t idx = 0;
			if (request::findValue(&req, "idx") != "")
			{
				idx = std::strtoull(request::findValue(&req, "idx").c_str(), nullptr, 10);
			}
			std::vector<std::vector<std::string> > result;
			//First get Device Type/SubType
			result = m_sql.safe_query("SELECT Type, SubType, SwitchType, Options FROM DeviceStatus WHERE (ID == %" PRIu64 ")",
				idx);
			if (result.empty())
				return;

			unsigned char dType = atoi(result[0][0].c_str());
			unsigned char dSubType = atoi(result[0][1].c_str());
			_eSwitchType switchtype = (_eSwitchType)atoi(result[0][2].c_str());
			std::map<std::string, std::string> options = m_sql.BuildDeviceOptions(result[0][3].c_str());

			if (
				(dType != pTypeLighting1) &&
				(dType != pTypeLighting2) &&
				(dType != pTypeLighting3) &&
				(dType != pTypeLighting4) &&
				(dType != pTypeLighting5) &&
				(dType != pTypeLighting6) &&
				(dType != pTypeFan) &&
				(dType != pTypeColorSwitch) &&
				(dType != pTypeSecurity1) &&
				(dType != pTypeSecurity2) &&
				(dType != pTypeEvohome) &&
				(dType != pTypeEvohomeRelay) &&
				(dType != pTypeCurtain) &&
				(dType != pTypeBlinds) &&
				(dType != pTypeRFY) &&
				(dType != pTypeRego6XXValue) &&
				(dType != pTypeChime) &&
				(dType != pTypeThermostat2) &&
				(dType != pTypeThermostat3) &&
				(dType != pTypeThermostat4) &&
				(dType != pTypeRemote) &&
				(dType != pTypeGeneralSwitch) &&
				(dType != pTypeHomeConfort) &&
				(dType != pTypeFS20) &&
				(!((dType == pTypeRadiator1) && (dSubType == sTypeSmartwaresSwitchRadiator))) &&
				(dType != pTypeHunter)
				)
				return; //no light device! we should not be here!

			root["status"] = "OK";
			root["title"] = "LightLog";

			result = m_sql.safe_query("SELECT ROWID, nValue, sValue, Date FROM LightingLog WHERE (DeviceRowID==%" PRIu64 ") ORDER BY Date DESC", idx);
			if (!result.empty())
			{
				std::map<std::string, std::string> selectorStatuses;
				if (switchtype == STYPE_Selector) {
					GetSelectorSwitchStatuses(options, selectorStatuses);
				}

				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					std::string lidx = sd.at(0);
					int nValue = atoi(sd.at(1).c_str());
					std::string sValue = sd.at(2);
					std::string ldate = sd.at(3);

					//add light details
					std::string lstatus;
					std::string ldata;
					int llevel = 0;
					bool bHaveDimmer = false;
					bool bHaveSelector = false;
					bool bHaveGroupCmd = false;
					int maxDimLevel = 0;

					if (switchtype == STYPE_Media) {
						if (sValue == "0") continue; //skip 0-values in log for MediaPlayers
							lstatus = sValue;
						ldata = lstatus;
					}
					else if (switchtype == STYPE_Selector)
					{
						if (ii == 0) {
							bHaveSelector = true;
							maxDimLevel = selectorStatuses.size();
						}
						if (!selectorStatuses.empty()) {

							std::string sLevel = selectorStatuses[sValue];
							ldata = sLevel;
							lstatus = "Set Level: " + sLevel;
							llevel = atoi(sValue.c_str());
						}
					}
					else {
						GetLightStatus(dType, dSubType, switchtype, nValue, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);
						ldata = lstatus;
					}

					if (ii == 0)
					{
						//Log these parameters once
						root["HaveDimmer"] = bHaveDimmer;
						root["result"][ii]["MaxDimLevel"] = maxDimLevel;
						root["HaveGroupCmd"] = bHaveGroupCmd;
						root["HaveSelector"] = bHaveSelector;
					}

					//Corrent names for certain switch types
					switch (switchtype)
					{
					case STYPE_Contact:
						ldata = (ldata == "On") ? "Open" : "Closed";
						break;
					case STYPE_DoorContact:
						ldata = (ldata == "On") ? "Open" : "Closed";
						break;
					case STYPE_DoorLock:
						ldata = (ldata == "On") ? "Locked" : "Unlocked";
						break;
					case STYPE_DoorLockInverted:
						ldata = (ldata == "On") ? "Unlocked" : "Locked";
						break;
					case STYPE_Blinds:
					case STYPE_VenetianBlindsEU:
					case STYPE_VenetianBlindsUS:
						ldata = (ldata == "On") ? "Closed" : "Open";
						break;
					case STYPE_BlindsInverted:
						ldata = (ldata == "On") ? "Open" : "Closed";
						break;
					case STYPE_BlindsPercentage:
						if ((ldata == "On") || (ldata == "Off"))
						{
							ldata = (ldata == "On") ? "Closed" : "Open";
						}
						break;
					case STYPE_BlindsPercentageInverted:
						if ((ldata == "On") || (ldata == "Off"))
						{
							ldata = (ldata == "On") ? "Open" : "Closed";
						}
						break;
					}

					root["result"][ii]["idx"] = lidx;
					root["result"][ii]["Date"] = ldate;
					root["result"][ii]["Data"] = ldata;
					root["result"][ii]["Status"] = lstatus;
					root["result"][ii]["Level"] = llevel;

					ii++;
				}
			}
		}

		void CWebServer::RType_TextLog(WebEmSession & session, const request& req, Json::Value &root)
		{
			uint64_t idx = 0;
			if (request::findValue(&req, "idx") != "")
			{
				idx = std::strtoull(request::findValue(&req, "idx").c_str(), nullptr, 10);
			}
			std::vector<std::vector<std::string> > result;

			root["status"] = "OK";
			root["title"] = "TextLog";

			result = m_sql.safe_query("SELECT ROWID, sValue, Date FROM LightingLog WHERE (DeviceRowID==%" PRIu64 ") ORDER BY Date DESC",
				idx);
			if (!result.empty())
			{
				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					root["result"][ii]["idx"] = sd[0];
					root["result"][ii]["Date"] = sd[2];
					root["result"][ii]["Data"] = sd[1];
					ii++;
				}
			}
		}

		void CWebServer::RType_SceneLog(WebEmSession & session, const request& req, Json::Value &root)
		{
			uint64_t idx = 0;
			if (request::findValue(&req, "idx") != "")
			{
				idx = std::strtoull(request::findValue(&req, "idx").c_str(), nullptr, 10);
			}
			std::vector<std::vector<std::string> > result;

			root["status"] = "OK";
			root["title"] = "SceneLog";

			result = m_sql.safe_query("SELECT ROWID, nValue, Date FROM SceneLog WHERE (SceneRowID==%" PRIu64 ") ORDER BY Date DESC", idx);
			if (!result.empty())
			{
				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					int nValue = atoi(sd[1].c_str());
					root["result"][ii]["idx"] = sd[0];
					root["result"][ii]["Date"] = sd[2];
					root["result"][ii]["Data"] = (nValue == 0) ? "Off" : "On";
					ii++;
				}
			}
		}

		/**
		 * Retrieve user session from store, without remote host.
		 */
		const WebEmStoredSession CWebServer::GetSession(const std::string & sessionId) {
			//_log.Log(LOG_STATUS, "SessionStore : get...");
			WebEmStoredSession session;

			if (sessionId.empty()) {
				_log.Log(LOG_ERROR, "SessionStore : cannot get session without id.");
			}
			else {
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT SessionID, Username, AuthToken, ExpirationDate FROM UserSessions WHERE SessionID = '%q'",
					sessionId.c_str());
				if (!result.empty()) {
					session.id = result[0][0].c_str();
					session.username = base64_decode(result[0][1]);
					session.auth_token = result[0][2].c_str();

					std::string sExpirationDate = result[0][3];
					time_t now = mytime(NULL);
					struct tm tExpirationDate;
					ParseSQLdatetime(session.expires, tExpirationDate, sExpirationDate);
					// RemoteHost is not used to restore the session
					// LastUpdate is not used to restore the session
				}
			}

			return session;
		}

		/**
		 * Save user session.
		 */
		void CWebServer::StoreSession(const WebEmStoredSession & session) {
			//_log.Log(LOG_STATUS, "SessionStore : store...");
			if (session.id.empty()) {
				_log.Log(LOG_ERROR, "SessionStore : cannot store session without id.");
				return;
			}

			char szExpires[30];
			struct tm ltime;
			localtime_r(&session.expires, &ltime);
			strftime(szExpires, sizeof(szExpires), "%Y-%m-%d %H:%M:%S", &ltime);

			std::string remote_host = (session.remote_host.size() <= 50) ? // IPv4 : 15, IPv6 : (39|45)
				session.remote_host : session.remote_host.substr(0, 50);

			WebEmStoredSession storedSession = GetSession(session.id);
			if (storedSession.id.empty()) {
				m_sql.safe_query(
					"INSERT INTO UserSessions (SessionID, Username, AuthToken, ExpirationDate, RemoteHost) VALUES ('%q', '%q', '%q', '%q', '%q')",
					session.id.c_str(),
					base64_encode(session.username).c_str(),
					session.auth_token.c_str(),
					szExpires,
					remote_host.c_str());
			}
			else {
				m_sql.safe_query(
					"UPDATE UserSessions set AuthToken = '%q', ExpirationDate = '%q', RemoteHost = '%q', LastUpdate = datetime('now', 'localtime') WHERE SessionID = '%q'",
					session.auth_token.c_str(),
					szExpires,
					remote_host.c_str(),
					session.id.c_str());
			}
		}

		/**
		 * Remove user session and expired sessions.
		 */
		void CWebServer::RemoveSession(const std::string & sessionId) {
			//_log.Log(LOG_STATUS, "SessionStore : remove...");
			if (sessionId.empty()) {
				return;
			}
			m_sql.safe_query(
				"DELETE FROM UserSessions WHERE SessionID = '%q'",
				sessionId.c_str());
		}

		/**
		 * Remove all expired user sessions.
		 */
		void CWebServer::CleanSessions() {
			//_log.Log(LOG_STATUS, "SessionStore : clean...");
			m_sql.safe_query(
				"DELETE FROM UserSessions WHERE ExpirationDate < datetime('now', 'localtime')");
		}

		/**
		 * Delete all user's session, except the session used to modify the username or password.
		 * username must have been hashed
		 *
		 * Note : on the WebUserName modification, this method will not delete the session, but the session will be deleted anyway
		 * because the username will be unknown (see cWebemRequestHandler::checkAuthToken).
		 */
		void CWebServer::RemoveUsersSessions(const std::string& username, const WebEmSession & exceptSession) {
			m_sql.safe_query("DELETE FROM UserSessions WHERE (Username=='%q') and (SessionID!='%q')", username.c_str(), exceptSession.id.c_str());
		}

	} //server
}//http
