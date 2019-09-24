#include "stdafx.h"
#include <iostream>
#include "DomoticzHardware.h"
#include "../main/Logger.h"
#include "../main/localtime_r.h"
#include "../main/Helper.h"
#include "../main/RFXtrx.h"
#include "../main/SQLHelper.h"
#include "../main/mainworker.h"
#include "hardwaretypes.h"

#define round(a) ( int ) ( a + .5 )

CDomoticzHardwareBase::CDomoticzHardwareBase()
{
	mytime(&m_LastHeartbeat);
	mytime(&m_LastHeartbeatReceive);
};

CDomoticzHardwareBase::~CDomoticzHardwareBase()
{
}

bool CDomoticzHardwareBase::CustomCommand(const uint64_t /*idx*/, const std::string& /*sCommand*/)
{
	return false;
}

bool CDomoticzHardwareBase::Start()
{
	m_iHBCounter = 0;
	m_bIsStarted = StartHardware();
	return m_bIsStarted;
}

bool CDomoticzHardwareBase::Stop()
{
	m_bIsStarted = (!StopHardware());
	return (!m_bIsStarted);
}

bool CDomoticzHardwareBase::Restart()
{
	if (StopHardware())
	{
		m_bIsStarted = StartHardware();
		return m_bIsStarted;
	}
	return false;
}

bool CDomoticzHardwareBase::RestartWithDelay(const long seconds)
{
	if (StopHardware())
	{
		sleep_seconds(seconds);
		m_bIsStarted = StartHardware();
		return m_bIsStarted;
	}
	return false;
}

void CDomoticzHardwareBase::EnableOutputLog(const bool bEnableLog)
{
	m_bOutputLog = bEnableLog;
}

void CDomoticzHardwareBase::StartHeartbeatThread()
{
	StartHeartbeatThread("Domoticz_HBWork");
}

void CDomoticzHardwareBase::StartHeartbeatThread(const char* ThreadName)
{
	m_Heartbeatthread = std::make_shared<std::thread>(&CDomoticzHardwareBase::Do_Heartbeat_Work, this);
	SetThreadName(m_Heartbeatthread->native_handle(), ThreadName);
}


void CDomoticzHardwareBase::StopHeartbeatThread()
{
	if (m_Heartbeatthread)
	{
		RequestStop();
		m_Heartbeatthread->join();
		// Wait a while. The read thread might be reading. Adding this prevents a pointer error in the async serial class.
		sleep_milliseconds(10);
		m_Heartbeatthread.reset();
	}
}

void CDomoticzHardwareBase::Do_Heartbeat_Work()
{
	int secCounter = 0;
	int hbCounter = 0;
	while (!IsStopRequested(200))
	{
		secCounter++;
		if (secCounter == 5)
		{
			secCounter = 0;
			hbCounter++;
			if (hbCounter % 12 == 0) {
				mytime(&m_LastHeartbeat);
			}
		}
	}
}

void CDomoticzHardwareBase::SetHeartbeatReceived()
{
	mytime(&m_LastHeartbeatReceive);
}

void CDomoticzHardwareBase::HandleHBCounter(const int iInterval)
{
	m_iHBCounter++;
	if (m_iHBCounter % iInterval == 0)
	{
		SetHeartbeatReceived();
	}
}

int CDomoticzHardwareBase::SetThreadNameInt(const std::thread::native_handle_type& thread)
{
	return SetThreadName(thread, m_ShortName.c_str());
}

//Log Helper functions
#define MAX_LOG_LINE_LENGTH (2048*3)
void CDomoticzHardwareBase::Log(const _eLogLevel level, const std::string& sLogline)
{
	_log.Log(level, "%s: %s", m_ShortName.c_str(), sLogline.c_str());
}

void CDomoticzHardwareBase::Log(const _eLogLevel level, const char* logline, ...)
{
	va_list argList;
	char cbuffer[MAX_LOG_LINE_LENGTH];
	va_start(argList, logline);
	vsnprintf(cbuffer, sizeof(cbuffer), logline, argList);
	va_end(argList);
	_log.Log(level, "%s: %s", m_ShortName.c_str(), cbuffer);
}

void CDomoticzHardwareBase::Debug(const _eDebugLevel level, const std::string& sLogline)
{
	_log.Debug(level, "%s: %s", m_ShortName.c_str(), sLogline.c_str());
}

void CDomoticzHardwareBase::Debug(const _eDebugLevel level, const char* logline, ...)
{
	va_list argList;
	char cbuffer[MAX_LOG_LINE_LENGTH];
	va_start(argList, logline);
	vsnprintf(cbuffer, sizeof(cbuffer), logline, argList);
	va_end(argList);
	_log.Debug(level, "%s: %s", m_ShortName.c_str(), cbuffer);
}
