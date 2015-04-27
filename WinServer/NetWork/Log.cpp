#include "Log.h"
#include <io.h>
#include <direct.h>

CreateSingleton(oLog);

CLogFile::CLogFile( xe_uint32 dwFlushInterval /*= 60000*/,bool bCutByDate /*= true*/ ,LogFileDumpFunc func /*NULL*/)
	:m_dwInterval(dwFlushInterval),m_bCutByDate(bCutByDate),m_pDumpFunc(func)
{
	m_dwLastTime = 0;
	m_pFile = NULL;
}

CLogFile::~CLogFile()
{
	Close();
}

//打开文件
bool CLogFile::Open( char * szFile )
{
	CAutoLock cs(&m_cs);

	if (m_pFile)
		fclose(m_pFile);
	m_pFile = NULL;

	m_szFileName = szFile;

	GetSystemTime(&m_xDate);		//获取时间

	//访问log文件夹
	if (_access("log",0) == -1)
		mkdir("log");

	char szFileWhole[256] = {0};
	sprintf(szFileWhole,"log\\%s_%04d_%02d_%02d.txt",m_szFileName.c_str(),m_xDate.wYear,m_xDate.wMonth,m_xDate.wDay);

	m_pFile = fopen(szFileWhole,"a");

	if (m_pFile== NULL)
	{
		printf("open file:%s failed!",szFileWhole);
		return false;
	}
	m_dwLastTime = GetTickCount();

	return true;
}

//关闭文件
void CLogFile::Close()
{
	CAutoLock cs(&m_cs);
	if (m_pFile)
	{
		fflush(m_pFile);
		fclose(m_pFile);
	}
}

//刷新缓冲区
void CLogFile::FFlush()
{
	CAutoLock cs(&m_cs);

	if (m_pFile)
	{
		fflush(m_pFile);

		m_dwLastTime = GetTickCount();
	}
}

//输出
void CLogFile::OutPutString( char * str,... )
{
	CAutoLock cs(&m_cs);

	char szMsg[4096] = {0};

	va_list ap;
	va_start(ap,str);
	vsnprintf(szMsg,4096,str,ap);
	va_end(ap);

	if (m_pDumpFunc)
		m_pDumpFunc(szMsg);	//输出函数

	SYSTEMTIME xSys;
	GetSystemTime(&xSys);

	if (m_pFile)
	{
		fprintf(m_pFile,"%04d/%02d/%02d %02d:%02d:%02d %s\n",xSys.wYear,xSys.wMonth,xSys.wDay,xSys.wHour,xSys.wMinute,xSys.wSecond,szMsg);

		if (m_bCutByDate && m_xDate.wDay != xSys.wDay)
		{
			fclose(m_pFile);

			char szFileWhole[256] = {0};
			sprintf(szFileWhole,"log\\%s_%04d_%02d_%02d.txt",m_szFileName.c_str(),xSys.wYear,xSys.wMonth,xSys.wDay);
			m_pFile = fopen(szFileWhole,"a");

			m_dwLastTime = GetTickCount();
		}
		else if (GetTickCount() - m_dwLastTime > m_dwInterval)
		{
			fflush(m_pFile);
			m_dwLastTime = GetTickCount();
		}
	}
}

//详细输出
void CLogFile::LogDetail( char * szFile, char *szFunction, int nLineNumber,char *szFormat,... )
{
	char szMsg[2096] = {0};
	char szTemp[4096] = {0};

	va_list ap;
	va_start(ap,szFormat);
	vsnprintf(szMsg,2096,szFormat,ap);
	va_end(ap);

	sprintf(szTemp,"file%s,fun:%s,line:%d %s",szFile,szFunction,nLineNumber,szMsg);

	OutPutString(szTemp);
}


oLog::oLog()
{
	m_pLogFile = new CLogFile();
	m_pLogFile->Open("OutPut");
	m_pDBLogFile = new CLogFile();
	m_pDBLogFile->Open("DBLog");
	m_pDBErrorFile = new CLogFile();
	m_pDBErrorFile->Open("DBError");

	m_pErrorFile = new CLogFile();
	m_pErrorFile->Open("Error");
}

oLog::~oLog()
{
	SAFE_DELETE(m_pLogFile);
	SAFE_DELETE(m_pDBLogFile);
	SAFE_DELETE(m_pDBErrorFile);
	SAFE_DELETE(m_pErrorFile);
}

void oLog::OutPutStr( char * szStr,... )
{
	char szMsg[4096] = {0};

	va_list ap;
	va_start(ap,szStr);
	vsnprintf(szMsg,4096,szStr,ap);
	va_end(ap);

	if (m_pLogFile)
		m_pLogFile->OutPutString(szMsg);
}

void oLog::OutPutDBStr( char * szStr,... )
{
	char szMsg[4096] = {0};

	va_list ap;
	va_start(ap,szStr);
	vsnprintf(szMsg,4096,szStr,ap);
	va_end(ap);

	if (m_pDBLogFile)
		m_pDBLogFile->OutPutString(szMsg);
}

void oLog::OutPutDBError( char *szStr,... )
{
	char szMsg[4096] = {0};

	va_list ap;
	va_start(ap,szStr);
	vsnprintf(szMsg,4096,szStr,ap);
	va_end(ap);

	if (m_pDBErrorFile)
		m_pDBErrorFile->OutPutString(szMsg);
}

void oLog::LogDetail( char * szFile, char *szFunction, int nLineNumber,char *szFormat,... )
{
	char szMsg[4096] = {0};

	va_list ap;
	va_start(ap,szFormat);
	vsnprintf(szMsg,4096,szFormat,ap);
	va_end(ap);

	if (m_pLogFile)
		m_pLogFile->LogDetail(szFile,szFunction,nLineNumber,szMsg);
}

void oLog::DBLogDetail( char * szFile, char *szFunction, int nLineNumber,char *szFormat,... )
{
	char szMsg[4096] = {0};

	va_list ap;
	va_start(ap,szFormat);
	vsnprintf(szMsg,4096,szFormat,ap);
	va_end(ap);

	if (m_pDBLogFile)
		m_pDBLogFile->LogDetail(szFile,szFunction,nLineNumber,szMsg);
}

void oLog::DBErrorDetail( char * szFile, char *szFunction, int nLineNumber,char *szFormat,... )
{
	char szMsg[4096] = {0};

	va_list ap;
	va_start(ap,szFormat);
	vsnprintf(szMsg,4096,szFormat,ap);
	va_end(ap);

	if (m_pDBErrorFile)
		m_pDBErrorFile->LogDetail(szFile,szFunction,nLineNumber,szMsg);
}

void oLog::OutPutError( char * szStr,... )
{
	char szMsg[4096] = {0};

	va_list ap;
	va_start(ap,szStr);
	vsnprintf(szMsg,4096,szStr,ap);
	va_end(ap);

	if (m_pErrorFile)
		m_pErrorFile->OutPutString(szMsg);
}

void oLog::ErrorDetail( char * szFile, char *szFunction, int nLineNumber,char *szFormat,... )
{
	char szMsg[4096] = {0};

	va_list ap;
	va_start(ap,szFormat);
	vsnprintf(szMsg,4096,szFormat,ap);
	va_end(ap);

	if (m_pErrorFile)
		m_pErrorFile->LogDetail(szFile,szFunction,nLineNumber,szMsg);
}

void oLog::Close()
{
	if (m_pErrorFile)
		m_pErrorFile->Close();

	if (m_pDBLogFile)
		m_pDBLogFile->Close();

	if (m_pDBErrorFile)
		m_pDBErrorFile->Close();

	if (m_pLogFile)
		m_pLogFile->Close();
}

