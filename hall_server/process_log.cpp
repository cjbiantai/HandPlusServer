
#include "process_log.h"
#include "process_define.h"

int get_current_string_date(char* strDate)
{
	struct tm *pstTm = NULL;
	time_t now  =0;

	if(NULL == strDate)
	{
		return fail;
	}

	time(&now);
	strDate[0] = '\0';

	pstTm = localtime(&now);
	if(NULL == pstTm)
	{
		return fail;
	}

	sprintf(strDate, "%04d-%02d-%02d", pstTm->tm_year + 1900, pstTm->tm_mon + 1, pstTm->tm_mday);
	return success;
}

	Log::Log()
	{
		//文件限制
		m_iMaxSystemLogFileSize = 0x2000000;
		m_iMaxSystemLogFileCount = 5;
	}

	Log::~Log()
	{

	}

	int Log::WriteServerLog( int iLogLevel, const char* szContent, ... )
	{
		int iRet = success;
		char szFileName[256];
		char szCurDate[32];

		if(!m_stLogEngine.whether_do_log(iLogLevel))
		{
			printf("no log!!loglevel too small\n");
			return fail;
		}

		get_current_string_date((char *)szCurDate);
		sprintf(szFileName, "./log/%s%s", PROCESS_LOGFILE_PREFIX,szCurDate);

		char* title = m_stLogEngine.get_log_level_title(iLogLevel);

		va_list ap;

		va_start(ap, szContent);
		iRet = m_stLogEngine.log(szFileName,
			m_iMaxSystemLogFileSize, m_iMaxSystemLogFileCount, title, szContent,ap);
		va_end(ap);

		return iRet;
	}

	int Log::WriteServerLog( const char* szContent, ... )
	{
		int iRet = success;
		char szFileName[256];
		char szCurDate[32];
		//char szErrInfo[256];

		get_current_string_date((char *)szCurDate);
		sprintf(szFileName, "./log/%s%s", PROCESS_LOGFILE_PREFIX, szCurDate);

		const char* title = "detail";

		va_list ap;

		va_start(ap, szContent);
		iRet = m_stLogEngine.log(szFileName,
			m_iMaxSystemLogFileSize, m_iMaxSystemLogFileCount, (char *)title, szContent,ap);
		va_end(ap);

		return iRet;
	}

	int Log::PrintServerBin( int iLogLevel, /*日志级别 */ char *pcBuffer, int iLength )
	{
		int iRet = success;
		char szFileName[256];
		char szCurDate[32];
		//char szErrInfo[256];


		if(!m_stLogEngine.whether_do_log(iLogLevel))
		{
			return fail;
		}

		get_current_string_date((char *)szCurDate);
		sprintf(szFileName, "./log/%s%s", PROCESS_LOGFILE_PREFIX, szCurDate);

		iRet = m_stLogEngine.dumpbin(szFileName,pcBuffer,iLength);

		return iRet;
	}

	void Log::SetLogLevel( int iLogLevel )
	{
		return m_stLogEngine.set_logmask(iLogLevel);
	}


