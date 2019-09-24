#pragma once

#include "RFXtrx.h"
#include "../hardware/DomoticzHardware.h"
#include "Scheduler.h"
#include "EventSystem.h"
#include "Camera.h"
#include <deque>
#include "WindCalculation.h"
#include "TrendCalculator.h"
#include "StoppableTask.h"
#include "../tcpserver/TCPServer.h"
#include "concurrent_queue.h"
#include "../webserver/server_settings.hpp"
#ifdef ENABLE_PYTHON
#	include "../hardware/plugins/PluginManager.h"
#endif

class MainWorker : public StoppableTask
{
public:
	MainWorker();
	~MainWorker();

	bool Start();
	bool Stop();

	void AddAllDomoticzHardware();
	void StopDomoticzHardware();
	void StartDomoticzHardware();
	void AddDomoticzHardware(CDomoticzHardwareBase *pHardware);
	void RemoveDomoticzHardware(CDomoticzHardwareBase *pHardware);
	void RemoveDomoticzHardware(int HwdId);
	int FindDomoticzHardware(int HwdId);
	int FindDomoticzHardwareByType(const _eHardwareTypes HWType);
	CDomoticzHardwareBase* GetHardware(int HwdId);
	CDomoticzHardwareBase* GetHardwareByIDType(const std::string &HwdId, const _eHardwareTypes HWType);
	CDomoticzHardwareBase* GetHardwareByType(const _eHardwareTypes HWType);

	void HeartbeatUpdate(const std::string &component, bool critical = true);
	void HeartbeatRemove(const std::string &component);
	void HeartbeatCheck();

	void SetWebserverSettings(const http::server::server_settings & settings);
	std::string GetWebserverAddress();
	std::string GetWebserverPort();
#ifdef WWW_ENABLE_SSL
	void SetSecureWebserverSettings(const http::server::ssl_server_settings & ssl_settings);
	std::string GetSecureWebserverPort();
#endif

	bool SwitchLight(const std::string &idx, const std::string &switchcmd, const std::string &level, const std::string &color, const std::string &ooc, const int ExtraDelay);
	bool SwitchLight(const uint64_t idx, const std::string &switchcmd, const int level, const _tColor color, const bool ooc, const int ExtraDelay);
	bool SwitchLightInt(const std::vector<std::string> &sd, std::string switchcmd, int level, _tColor color, const bool IsTesting);

	bool SwitchScene(const std::string &idx, const std::string &switchcmd);
	bool SwitchScene(const uint64_t idx, std::string switchcmd);
	void CheckSceneCode(const uint64_t DevRowIdx, const uint8_t dType, const uint8_t dSubType, const int nValue, const char* sValue);
	bool DoesDeviceActiveAScene(const uint64_t DevRowIdx, const int Cmnd);

	bool SetClock(const std::string &idx, const std::string &clockstr);
	bool SetClockInt(const std::vector<std::string> &sd, const std::string &clockstr);
	bool SetZWaveThermostatMode(const std::string &idx, const int tMode);
	bool SetZWaveThermostatFanMode(const std::string &idx, const int fMode);
	bool SetZWaveThermostatModeInt(const std::vector<std::string> &sd, const int tMode);
	bool SetZWaveThermostatFanModeInt(const std::vector<std::string> &sd, const int fMode);

	bool GetSunSettings();
	void LoadSharedUsers();

	void ForceLogNotificationCheck();

	bool RestartHardware(const std::string &idx);

	bool AddHardwareFromParams(
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
	);

	void UpdateDomoticzSecurityStatus(const int iSecStatus);
	void SetInternalSecStatus();
	bool GetSensorData(const uint64_t idx, int &nValue, std::string &sValue);

	bool UpdateDevice(const int DevIdx, int nValue, std::string& sValue, const int signallevel = 12, const int batterylevel = 255, const bool parseTrigger = true);
	bool UpdateDevice(const int HardwareID, const std::string &DeviceID, const int unit, const int devType, const int subType, int nValue, std::string &sValue, const int signallevel = 12, const int batterylevel = 255, const bool parseTrigger = true);

	boost::signals2::signal<void(const int m_HwdID, const uint64_t DeviceRowIdx, const std::string &DeviceName, const uint8_t *pRXCommand)> sOnDeviceReceived;
	boost::signals2::signal<void(const uint64_t SceneIdx, const std::string &SceneName)> sOnSwitchScene;

	CScheduler m_scheduler;
	CEventSystem m_eventsystem;
#ifdef ENABLE_PYTHON
	Plugins::CPluginSystem m_pluginsystem;
#endif
	CCameraHandler m_cameras;
	bool m_bIgnoreUsernamePassword;
	bool m_bHaveUpdate;
	int m_iRevision;

	std::string m_szSystemName;
	std::string m_szDomoticzUpdateURL;

	bool IsUpdateAvailable(const bool bIsForced = false);
	bool StartDownloadUpdate();
	bool m_bHaveDownloadedDomoticzUpdate;
	bool m_bHaveDownloadedDomoticzUpdateSuccessFull;
	std::string m_UpdateStatusMessage;

	void GetAvailableWebThemes();

	tcp::server::CTCPServer m_sharedserver;
	std::string m_LastSunriseSet;
	std::vector<int> m_SunRiseSetMins;
	std::string m_DayLength;
	std::vector<std::string> m_webthemes;
	std::map<uint16_t, _tWindCalculator> m_wind_calculator;
	std::map<uint64_t, _tTrendCalculator> m_trend_calculator;

	time_t m_LastHeartbeat = 0;
private:
	void HandleAutomaticBackups();
	uint64_t PerformRealActionFromDomoticzClient(const uint8_t *pRXCommand, CDomoticzHardwareBase **pOriginalHardware);
	void HandleLogNotifications();
	std::map<std::string, std::pair<time_t, bool> > m_componentheartbeats;
	std::mutex m_heartbeatmutex;

	std::vector<int> m_devicestorestart;

	bool m_bForceLogNotificationCheck;

	int m_SecCountdown;
	int m_SecStatus;

	int m_ScheduleLastMinute;
	int m_ScheduleLastHour;
	//fix for hardware clock that sets time back/ford
	time_t m_ScheduleLastMinuteTime;
	time_t m_ScheduleLastHourTime;
	time_t m_ScheduleLastDayTime;


	std::mutex m_devicemutex;

	std::string m_szDomoticzUpdateChecksumURL;
	bool m_bDoDownloadDomoticzUpdate;
	bool m_bStartHardware;
	uint8_t m_hardwareStartCounter;

	std::vector<CDomoticzHardwareBase*> m_hardwaredevices;
	http::server::server_settings m_webserver_settings;
#ifdef WWW_ENABLE_SSL
	http::server::ssl_server_settings m_secure_webserver_settings;
#endif
	std::shared_ptr<std::thread> m_thread;
	std::mutex m_mutex;

	time_t m_LastUpdateCheck;

	void Do_Work();
	void Heartbeat();
	bool WriteToHardware(const int HwdID, const char *pdata, const uint8_t length);

	void OnHardwareConnected(CDomoticzHardwareBase *pHardware);

	void WriteMessageStart();
	void WriteMessage(const char *szMessage);
	void WriteMessage(const char *szMessage, bool linefeed);
	void WriteMessageEnd();

	//message decoders
	void decode_BateryLevel(bool bIsInPercentage, uint8_t level);
	uint8_t get_BateryLevel(const _eHardwareTypes HwdType, bool bIsInPercentage, uint8_t level);

};

extern MainWorker m_mainworker;
