// #ifndef __MONITOR_LOG_H__
// #define __MONITOR_LOG_H__
#pragma once
#include "../common/log/logengine.h"
#include "../common/singleton.h"




	enum
	{
		log_mask_everything		= 0xffffffff,
		open_min_socket_count   = 2,
#if defined(DEBUG) || defined(_DEBUG) || defined(_DEBUG_) || (__DEBUG__)
		open_max_socket_count   = 5000,
#else
		open_max_socket_count   = 20480,
#endif
	};

class Log
{
public:
	Log();
	~Log();

	int WriteServerLog(int iLogLevel,
		const char* szContent, ...);

	int WriteServerLog(
		const char* szContent, ...);

	int PrintServerBin(int iLogLevel,		//日志级别
		char *pcBuffer,
		int iLength);

	void SetLogLevel(int iLogLevel);

protected:

private:

	CLogEngine m_stLogEngine;

	//文件限制
	int m_iMaxSystemLogFileSize;
	int m_iMaxSystemLogFileCount;


private:
};

typedef Singleton<Log> ProcessLog;

#define TRACE_WARN(format, ...) \
    ProcessLog::Instance().WriteServerLog(log_mask_system_warning, format, ##__VA_ARGS__)

#define TRACE_ERROR(format, ...) \
    ProcessLog::Instance().WriteServerLog(log_mask_system_error, format, ##__VA_ARGS__)

#define TRACE_DETAIL(format, ...) \
    ProcessLog::Instance().WriteServerLog(log_mask_detail, format, ##__VA_ARGS__)

#define TRACEBIN ProcessLog::Instance().PrintServerBin

#define TRACE_SERVER ProcessLog::Instance().WriteServerLog

#define SETTRACELEVEL ProcessLog::Instance().SetLogLevel


// #endif

