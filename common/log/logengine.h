
#pragma once
#include "base.h"





static int32_t get_string_datetime(const time_t time, char* pszBuffer)
{
	if (NULL == pszBuffer)
	{
		return fail;
	}

	struct tm stTime;
	struct tm* pstTime = NULL;
	pstTime = localtime_r(&time, &stTime);
	if (NULL == pstTime)
	{
		return fail;
	}
	sprintf(pszBuffer, "%02d:%02d:%02d",
			stTime.tm_hour, stTime.tm_min, stTime.tm_sec);

	return success;
}

static int get_current_string_datetime(char* pszBuffer)
{
	time_t now;
	time(&now);

	return get_string_datetime((const time_t) now, pszBuffer);
}

#define STRNCPY(dst, src, length)					      \
	do								      \
	{								      \
		strncpy(dst, src, (length-1));			      \
		dst[(length-1)] = '\0';					      \
	}								      \
	while(0)




#define CHECK_FMT(a, b)	__attribute__((format(printf, a, b)))


//���1Byte��ϵͳ���������룬����Ӧ�ý����Զ������־��Ϣ���Ӧ��ʹ�ø�3Byte����
enum enmBasicLogMaskBit
{
	log_mask_none				= int(0x00000000),				//
	log_mask_system_error		= int(0x00000001),				//
	log_mask_system_warning		= int(0x00000002),				//
    log_mask_info               = int(0x00000004),              //��Ҫ��Ϣ
	log_mask_system_config		= int(0x00000008),				//������Ϣ
	log_mask_system_performance = int(0x00000010),				//ϵͳ������Ϣ
	log_mask_detail				= int(0x00000020),				//��ϸ��Ϣ
	log_mask_statics			= int(0x00000040),				//ͳ����Ϣ
	log_mask_all				= int(0xffffffff),				//ǿ�����

};

enum
{
	max_log_level_title_len = 32,
};

struct stLogTitleEntry
{
	int m_iLogLevel;
	char m_szTitle[max_log_level_title_len];
};



class CLogEngine
{
public:
	enum
	{
		max_dump_info_size = 0x400, //
		max_dump_line_size = 0x10,  //
	};

	CLogEngine(int filesize, int filenumber, int logmask, char* logfile):m_iMaxLogFileSize(filesize),
		m_iMaxLogFileNumber(filenumber),m_iLogMask(logmask)
	{
		if (NULL != logfile)
		{
			STRNCPY(m_szFileName, logfile, sizeof(m_szFileName));
		}
	}

	CLogEngine():m_iMaxLogFileSize(0),
		m_iMaxLogFileNumber(0),m_iLogMask(0)
	{
		m_szFileName[0] = '\0';
	}

	virtual ~CLogEngine()
	{
		//do nothing
	}

	int initialize(int filesize, int filenumber, int logmask, char* logfile);

	bool whether_do_log(int mask);

	void set_logmask(int mask);

	int log(const char* pszFileName, int iMaxFileSize, int iMaxFileNumber, char* title, const char* pszLogInfo, va_list vaList);
	int log(FILE* pstFile, char* pszFileName, int iMaxFileSize, int iMaxFileNumber, char* title, const char* pszLogInfo, va_list vaList);

	int log(const char* pszFileName, int iMaxFileSize, int iMaxFileNumber, char* title, const char *szSrcFileName , const char* szFuncion, int iLine, const char* pszLogInfo, va_list vaList);

	int log(const char* pszFileName, int iMaxFileSize, int iMaxFileNumber, char* title, const char* pszLogInfo, ...);
    int log(FILE* pstFile, char* pszFileName, int iMaxFileSize, int iMaxFileNumber, char* title, const char* pszLogInfo, ...);

	int log(char* title, const char* pszLogInfo, va_list vaList);

	int log(char* title, const char* pszLogInfo, ...);

	int dumpbin(const char* pszFileName, const char* pszBuffer, int iSize,const char* pTitle=NULL);
    int dumpbin(FILE* pstFile, char* pszFileName, const char* pszBuffer, int iSize,const char* pTitle=NULL);

	int shiftfile(const char* pszFileName, int iMaxFileSize, int iMaxFileNumber);

	//
	char* get_log_level_title(int logLevel);

private:
	char    m_szFileName[max_file_name_length];
	int m_iMaxLogFileSize;
	int m_iMaxLogFileNumber;

	int m_iLogMask;

protected:
private:
};


