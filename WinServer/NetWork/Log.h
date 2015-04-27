#ifndef _LOG_H_
#define _LOG_H_

//////////////////////////////////////////////////////////////////////////
//仿照魔兽世界写的日志系统
//////////////////////////////////////////////////////////////////////////
#include "GlobalDefine.h"
#include "Singleton.h"


typedef void (*LogFileDumpFunc)(const char* szMsg);

class CLogFile
{
public:
	//刷新时间
	CLogFile(xe_uint32 dwFlushInterval = 60000,bool bCutByDate = true,LogFileDumpFunc func =NULL );
	~CLogFile();

	bool Open(char * szFile);				//日志名字
	void Close();
	void FFlush();							//刷新
	void OutPutString(char * str,...);		//输出
	void LogDetail(char * szFile, char *szFunction, int nLineNumber,char *szFormat,...);

private:
	LogFileDumpFunc m_pDumpFunc;	//一般都是输出函数，表示是否输出到屏幕上
	CLock m_cs;						//lock
	std::string m_szFileName;		//文件名
	SYSTEMTIME  m_xDate;			//时间
	FILE*	    m_pFile;			//输出文件指针
	bool		m_bCutByDate;		//cutbydate

	xe_uint32	m_dwLastTime;		//上一次刷新时间
	xe_uint32	m_dwInterval;		//刷新时间
};


class oLog : public Singleton<oLog>
{
public:
	oLog();
	~oLog();

	void OutPutStr(char * szStr,...);
	void OutPutDBStr(char * szStr,...);
	void OutPutDBError(char *szStr,...);
	void OutPutError(char * szStr,...);

	void LogDetail(char * szFile, char *szFunction, int nLineNumber,char *szFormat,...);
	void DBLogDetail(char * szFile, char *szFunction, int nLineNumber,char *szFormat,...);
	void DBErrorDetail(char * szFile, char *szFunction, int nLineNumber,char *szFormat,...);
	void ErrorDetail(char * szFile, char *szFunction, int nLineNumber,char *szFormat,...);

	void Close();
private:
	CLogFile * m_pLogFile;				//OutPut
	CLogFile * m_pDBLogFile;			//db
	CLogFile * m_pDBErrorFile;			//dberror
	CLogFile * m_pErrorFile;
};

#define sLog oLog::getSingleton()


#define LOG_NORMAL( msg, ... ) sLog.LogDetail( __FILE__, __LINE__, __FUNCTION__, msg, ##__VA_ARGS__ )
#define LOG_DBINFO( msg, ... ) sLog.DBLogDetail( __FILE__, __LINE__, __FUNCTION__, msg, ##__VA_ARGS__ )
#define LOG_DBERROR( msg, ... ) sLog.DBErrorDetail( __FILE__, __LINE__, __FUNCTION__, msg, ##__VA_ARGS__ )
#define LOG_RROR( msg, ... ) sLog.ErrorDetail( __FILE__, __LINE__, __FUNCTION__, msg, ##__VA_ARGS__ )

#endif