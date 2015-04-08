#include "Session.h"
#include "SessionList.h"
#include "Connector.h"
#include "TcpServer.h"

DWORD WINAPI CConnector::_ConnectThread( LPVOID lpParam )
{
	CConnector *pConnector = (CConnector*)lpParam;
	if (pConnector)
	{
		while (!pConnector->m_bShutDown)
		{
			xe_uint32 dwRet = WaitForSingleObject(pConnector->m_hEvent,INFINITE);

			if(dwRet == WAIT_OBJECT_0)
			{
				while(!pConnector->m_pConnectList->empty())
				{
					//触发读取一个数据
					pConnector->m_pConnectList->Lock();
					CSession *pSession = pConnector->m_pConnectList->front();
					pConnector->m_pConnectList->pop_front();
					pConnector->m_pConnectList->UnLock();

					if (pSession == NULL)
						continue;

					xe_uint32 nRetConnect = connect(pSession->GetSocket(),(SOCKADDR*)pSession->GetSockAddr(),sizeof(SOCKADDR_IN));
					if (nRetConnect == SOCKET_ERROR)
					{
						pConnector->m_pParent->AddConnectSuccessList(pSession);
					}
					else
					{
						pConnector->AddConnectList(pSession);	//插入
					}
					
				}
				
			}
		}

		return 1;
	}

	return 0;
}



CConnector::CConnector( cIocpServer *pParent )
{
	m_pParent = pParent;
	m_pConnectList = new CSessionList();
	m_hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_hThread = INVALID_HANDLE_VALUE;
}

CConnector::~CConnector()
{
	Stop();

	CloseHandle(m_hEvent);
	CloseHandle(m_hThread);
}

void CConnector::AddConnectList( CSession *pSession )
{
	m_pConnectList->Lock();
	m_pConnectList->push_back(pSession);
	m_pConnectList->UnLock();

	SetEvent(m_hEvent);
}

bool CConnector::Start()
{
	m_bShutDown = false;

	//创建一条线程
	DWORD dwThread;
	m_hThread = CreateThread(NULL,0,_ConnectThread,this,0,&dwThread);

	return m_hThread != INVALID_HANDLE_VALUE;
}

void CConnector::Stop()
{
	m_bShutDown = true;

	SetEvent(m_hEvent);

	WaitForSingleObject(m_hThread,INFINITE);
}

