#pragma once

#include "../hardware/DomoticzHardware.h"
#include <deque>
#include "StoppableTask.h"
#include "concurrent_queue.h"
#ifdef ENABLE_PYTHON
#	include "../hardware/plugins/PluginManager.h"
#endif
#include "../main/UpdatePublisher.h"

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
	CDomoticzHardwareBase* GetHardware(int HwdId);

	void HeartbeatUpdate(const std::string &component, bool critical = true);
	void HeartbeatRemove(const std::string &component);
	void HeartbeatCheck();

	bool GetSunSettings();

	void ForceLogNotificationCheck();

	bool RestartHardware(const std::string &idx, const std::string& Name);

	boost::signals2::signal<void(const int m_HwdID, const uint64_t DeviceRowIdx, const std::string &DeviceName, const uint8_t *pRXCommand)> sOnDeviceReceived;
	boost::signals2::signal<void(const uint64_t SceneIdx, const std::string &SceneName)> sOnSwitchScene;

	Plugins::CPluginSystem m_pluginsystem;
	CUpdateManager	m_UpdateManager;

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

	std::string m_LastSunriseSet;
	std::vector<int> m_SunRiseSetMins;
	std::string m_DayLength;
	std::vector<std::string> m_webthemes;

	time_t m_LastHeartbeat = 0;
private:
	void HandleAutomaticBackups();
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
};

extern MainWorker m_mainworker;
