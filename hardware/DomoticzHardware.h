#pragma once

#include <boost/signals2.hpp>
#include "../main/StoppableTask.h"

enum _eLogLevel : uint32_t;
enum _eDebugLevel : uint32_t;

//Base class with functions all notification systems should have
class CDomoticzHardwareBase : public StoppableTask
{
	friend class MainWorker;
public:
	CDomoticzHardwareBase();
	virtual ~CDomoticzHardwareBase();

	bool Start();
	bool Stop();
	bool Restart();
	bool RestartWithDelay(const long seconds);
	virtual bool WriteToHardware(const char *pdata, const unsigned char length) = 0;
	virtual bool CustomCommand(const uint64_t idx, const std::string &sCommand);

	void EnableOutputLog(const bool bEnableLog);

	bool IsStarted() { return m_bIsStarted; }

	void SetHeartbeatReceived();

	time_t m_LastHeartbeat = { 0 };
	time_t m_LastHeartbeatReceive = { 0 };

	int m_HwdID = { 0 }; //must be uniquely assigned
	bool m_bSkipReceiveCheck = { false };
	unsigned long m_DataTimeout = { 0 };
	std::string m_Name;
	std::string m_ShortName;
	unsigned char m_SeqNr = { 0 };
	bool m_bEnableReceive = { false };
	boost::signals2::signal<void(CDomoticzHardwareBase *pHardware, const unsigned char *pRXCommand, const char *defaultName, const int BatteryLevel)> sDecodeRXMessage;
	boost::signals2::signal<void(CDomoticzHardwareBase *pDevice)> sOnConnected;
	void *m_pUserData = { NULL };
	bool m_bOutputLog = { true };

	int SetThreadNameInt(const std::thread::native_handle_type &thread);

	//Log Helper functions
	void Log(const _eLogLevel level, const std::string& sLogline);
	void Log(const _eLogLevel level, const char* logline, ...)
#ifdef __GNUC__
		__attribute__((format(printf, 3, 4)))
#endif
		;
	void Debug(const _eDebugLevel level, const std::string& sLogline);
	void Debug(const _eDebugLevel level, const char* logline, ...)
#ifdef __GNUC__
		__attribute__((format(printf, 3, 4)))
#endif
		;

protected:
	virtual bool StartHardware()=0;
	virtual bool StopHardware()=0;

    //Heartbeat thread for classes that can not provide this themselves
	void StartHeartbeatThread();
	void StartHeartbeatThread(const char* ThreadName);
	void StopHeartbeatThread();
	void HandleHBCounter(const int iInterval);


	int m_iHBCounter = { 0 };
	bool m_bIsStarted = { false };
private:
    void Do_Heartbeat_Work();

	volatile bool m_stopHeartbeatrequested = { false };
	std::shared_ptr<std::thread> m_Heartbeatthread = { nullptr };
};

