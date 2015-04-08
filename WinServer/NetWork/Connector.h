//////////////////////////////////////////////////////////////////////////
//连接其他服务器用途
//////////////////////////////////////////////////////////////////////////
#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_

#include "GlobalDefine.h"

class cIocpServer;
class CSession;
class CSessionList;
class CConnector
{
public:
	CConnector(cIocpServer *pParent);		
	virtual ~CConnector();

	void	AddConnectList(CSession *pSession);			//加入连接链表
	//连接线程
	static DWORD WINAPI _ConnectThread(void*lpParam);

	bool Start();
	void Stop();	

private:
	bool			m_bShutDown;		//关闭
	cIocpServer*	m_pParent;			//父类
	CSessionList*	m_pConnectList;		//连接类
	HANDLE			m_hThread;
	HANDLE			m_hEvent;			//event
};
#endif

