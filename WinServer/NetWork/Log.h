#ifndef _LOG_H_
#define _LOG_H_

//////////////////////////////////////////////////////////////////////////
//����ħ������д����־ϵͳ
//////////////////////////////////////////////////////////////////////////
#include "GlobalDefine.h"
#include "Singleton.h"


typedef void (*LogFileDumpFunc)(const char* szMsg);

class CLogFile
{
public:
	//ˢ��ʱ��
	CLogFile(xe_uint32 dwFlushInterval = 60000,bool bCutByDate = true,LogFileDumpFunc func =NULL );
	~CLogFile();

	bool Open(char * szFile);				//��־����
	void Close();
	void FFlush();							//ˢ��
	void OutPutString(char * str,...);		//���
	void LogDetail(char * szFile, char *szFunction, int nLineNumber,char *szFormat,...);

private:
	LogFileDumpFunc m_pDumpFunc;	//һ�㶼�������������ʾ�Ƿ��������Ļ��
	CLock m_cs;						//lock
	std::string m_szFileName;		//�ļ���
	SYSTEMTIME  m_xDate;			//ʱ��
	FILE*	    m_pFile;			//����ļ�ָ��
	bool		m_bCutByDate;		//cutbydate

	xe_uint32	m_dwLastTime;		//��һ��ˢ��ʱ��
	xe_uint32	m_dwInterval;		//ˢ��ʱ��
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