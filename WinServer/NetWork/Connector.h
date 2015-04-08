//////////////////////////////////////////////////////////////////////////
//����������������;
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

	void	AddConnectList(CSession *pSession);			//������������
	//�����߳�
	static DWORD WINAPI _ConnectThread(void*lpParam);

	bool Start();
	void Stop();	

private:
	bool			m_bShutDown;		//�ر�
	cIocpServer*	m_pParent;			//����
	CSessionList*	m_pConnectList;		//������
	HANDLE			m_hThread;
	HANDLE			m_hEvent;			//event
};
#endif

