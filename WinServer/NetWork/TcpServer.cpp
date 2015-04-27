#include "Session.h"
#include "SessionList.h"
#include "SessionPool.h"
#include "TcpServer.h"
#include "Connector.h"
#include "MsWinsockUtil.h"
#include "SendBuffer.h"
#include "RecvBuffer.h"
#include "Acceptor.h"
#include "Log.h"


DWORD WINAPI cIocpServer::_WorkerThread( LPVOID lpParam )
{
	cIocpServer *pServer = (cIocpServer*)lpParam;
	if (pServer)
	{
		//
		OVERLAPPEDEX *pIoContext = NULL;
		CSession *pSession = NULL;

		xe_uint32 dwBytes = 0;

		//ѭ����������ֱ���յ�shutdown��Ϣ
		while (WAIT_OBJECT_0 != WaitForSingleObject(pServer->m_hShutdownEvent,0))
		{
			BOOL bRet = GetQueuedCompletionStatus(pServer->m_hIOCompletionPort,(LPDWORD)&dwBytes,(PULONG_PTR)&pSession,(LPOVERLAPPED*)&pIoContext,INFINITE);
			
			if (pSession == EXIT_CODE)
				break;					//�������

			if (bRet == FALSE || pIoContext == NULL || pSession == NULL)
			{
				//�˳�
				DWORD dwErr = GetLastError();

				if (pSession != NULL)
				{
					// ��ʾһ����ʾ��Ϣ,�˳�����true
					if( !pServer->HandleError( pSession,dwErr ) )			
					{
						break;
					}

				}

				continue;  
			}
			else
			{
				// �ж��Ƿ��пͻ��˶Ͽ���
				if ((dwBytes == 0) && (pIoContext->dwOperator == IOCP_REQUEST_SEND  || pIoContext->dwOperator == IOCP_REQUEST_RECV))
				{
					//�ͻ��˵�����
					pServer->_ShowMessage("�ͻ��� %s:%d �Ͽ�����.",pSession->GetIP(), pSession->GetIndex() );
					
					//ɾ�������ڴ��е�pool
					//SOCKETPOOL->ReleasePerSocketSession(pSession);
					pSession->Remove();
					continue;
				}
				else
				{
					switch(pIoContext->dwOperator)
					{
					//case IOCP_REQUEST_ACCEPT:
					//	//����accept
					//	pServer->_DoAccpet(pSession,pIoContext,dwBytes);
					//	break;
					case IOCP_REQUEST_RECV:
						//����read
						pServer->_DoRecv(pSession,pIoContext,dwBytes);
						break;
					case IOCP_REQUEST_SEND:
						//����recv
						pServer->_DoSend(pSession,pIoContext);
						break;
					}
				}
			}
		}
	}

	return 0;
}

DWORD WINAPI cIocpServer::_BackEndThread( LPVOID lpParam )
{
	cIocpServer *pServer = (cIocpServer*)lpParam;
	if (pServer)
	{

		while(WAIT_OBJECT_0 != WaitForSingleObject(pServer->m_hShutdownEvent,0))
		{
			//timeout
			//�����߳�
			pServer->ProcessSendPacket();

			Sleep(10);			
		}
	}

	return 0;
}




cIocpServer::cIocpServer()
{
	m_nThreads = 0;
	m_phWorkerThreads = NULL;

	m_strIP = DEFAULT_IP;
	m_nPort = DEFAULT_PORT;

	m_hIOCompletionPort = NULL;		//io�˿�
	m_hShutdownEvent = NULL;
	m_pListenSession = NULL;		//����������
	
	m_hBackendThread = NULL;

	m_pSessionPool = new CSessionPool(2000);		//�����

	m_pActiveSessionList = new CSessionList();

	m_pTempSessionList = new CSessionList();

	m_pConnectSuccessList = new CSessionList();		//���ӳɹ�

	m_pConnector = new CConnector(this);

	m_pAcceptor = new CAcceptor(this);

	m_bRunning = false;
}

cIocpServer::~cIocpServer()
{
	//�ͷ���Դ
	Stop();
	SAFE_DELETE(m_pSessionPool);
	SAFE_DELETE(m_pActiveSessionList);
	SAFE_DELETE(m_pTempSessionList);
	SAFE_DELETE(m_pTempSessionList);
	SAFE_DELETE(m_pAcceptor);
	SAFE_DELETE(m_pConnector);
}


//====================================================================================
//				    ������Ϣ����
//====================================================================================
void cIocpServer::_ShowMessage( char*szFormat,... )
{
	va_list ap;

	char szText[1024] = {0};
	va_start(ap,szFormat);
	vsprintf(szText,szFormat,ap);
	va_end(ap);

	//���������Ϣ
	//printf("%s",szText);		//��ʱ��������
	sLog.OutPutStr(szText);
}

//====================================================================================
//
//				    IOCP��ʼ������ֹ
//
//====================================================================================

bool cIocpServer::LoadSocketLib()
{
	WSADATA wd;
	int nResult;

	nResult = WSAStartup(MAKEWORD(2,2),&wd);

	if(nResult != NO_ERROR)
	{
		_ShowMessage("Init socket envirment error!");		//��ʼ��socket����
		return false;
	}

	return true;
}

//��ȡϵͳcpu
int cIocpServer::_GetNoOfProcessors()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);

	return info.dwNumberOfProcessors;
}

//��ʼ����ɶ˿�
bool cIocpServer::_InitializeIOCP()
{
	//������ɶ˿�
	m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,0);
	if (m_hIOCompletionPort == NULL)
	{
		_ShowMessage("call CreateIoCompletionPort failed");
	}

	//�����߳�����
	m_nThreads = 2 * _GetNoOfProcessors();	

	//�����߳̾��
	m_phWorkerThreads = new HANDLE[m_nThreads];


	//�����������߳�
	DWORD dwThreadId;
	for (int i = 0; i < m_nThreads; ++i)
	{
		m_phWorkerThreads[i] = CreateThread(NULL,0,_WorkerThread,this,0,&dwThreadId);
	}

	_ShowMessage("iocp has success created!\n");

	return true;
}

//��ʼ��������socket
bool cIocpServer::_InitializeListenSocket()
{
	if (m_pAcceptor)
	{
		m_pAcceptor->Init(this,_GetNoOfProcessors()*2);

		m_pAcceptor->StartListen((char*)m_strIP.c_str(),m_nPort);

		_SetSocketProp(m_pAcceptor->GetListenSocket());


		// ΪAcceptEx ׼��������Ȼ��Ͷ��AcceptEx I/O����Ͷ��accept
		for( int i=0;i<MAX_POST_ACCEPT;i++ )
		{
			// �½�һ��IO_CONTEXT
			CSession *pTmpSession = m_pSessionPool->Alloc();

			if (pTmpSession)
			{
				//Ͷ��
				if( false==this->_PostAccept( pTmpSession ) )
				{
					pTmpSession->Remove();
					return false;
				}
			}
			
		}

		_ShowMessage( "Ͷ�� %d ��AcceptEx�������",MAX_POST_ACCEPT );

		
		return true;
	}

	return false;
}

void cIocpServer::_DeInitialize()
{
	// �ر�ϵͳ�˳��¼����
	RELEASE_HANDLE(m_hShutdownEvent);

	// �ͷŹ������߳̾��ָ��
	for( int i=0;i<m_nThreads;i++ )
	{
		RELEASE_HANDLE(m_phWorkerThreads[i]);
	}

	SAFE_DELETE(m_phWorkerThreads);

	RELEASE_HANDLE(m_hBackendThread);

	// �ر�IOCP���
	RELEASE_HANDLE(m_hIOCompletionPort);

	// �رռ���Socket
	//SAFE_DELETE(m_pListenSession);
	SAFE_DELETE(m_pListenSession);

	m_pListenSession = NULL;

	_ShowMessage("�ͷ���Դ���.\n");
}

/*******************
�ͻ��˻�ȡ�˿ڰ󶨵���ɶ˿���

*********************/
bool cIocpServer::_AssociateWithIOCP( CSession *pSession )
{
	// �����ںͿͻ���ͨ�ŵ�SOCKET�󶨵���ɶ˿���
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pSession->GetSocket(), m_hIOCompletionPort, (xe_uint32)pSession, 0);

	if (NULL == hTemp)
	{
		this->_ShowMessage("ִ��CreateIoCompletionPort()���ִ���.������룺%d",GetLastError());
		return false;
	}

	return true;
}

//Ͷ������
bool cIocpServer::_PostAccept( CSession* pAcceptIoContext )
{
	if (pAcceptIoContext)
	{
		return pAcceptIoContext->PreAccept(m_pAcceptor->GetListenSocket());
	}

	return false;
}


////////////////////////////////////////////////////////////
// ���пͻ��������ʱ�򣬽��д���

// ��֮��Ҫ֪�����������ListenSocket��Context��������Ҫ����һ�ݳ������������Socket��
// ԭ����Context����Ҫ���������Ͷ����һ��Accept����
// ����ɶ˿ڽ��ܵ�accpet��Ϣʱ��GetAcceptExSockaddrsͬʱ��ȡ����һ����Ϣ
bool cIocpServer::_DoAccpet(CSession *pSession,OVERLAPPEDEX *ol,xe_uint32 dwBytes)
{
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;  
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);  

	//����ѷ������˵�iocontext���ܵ�����Ϣbuffer���ã�����m_lpfnGetAcceptExSockAddrsȥ��ȡ��һ����Ϣ
	//pIoContext->ResetBuffer();

	///////////////////////////////////////////////////////////////////////////
	// 1. ����ȡ������ͻ��˵ĵ�ַ��Ϣ
	// ��� m_lpfnGetAcceptExSockAddrs �����˰�~~~~~~
	// ��������ȡ�ÿͻ��˺ͱ��ض˵ĵ�ַ��Ϣ������˳��ȡ���ͻ��˷����ĵ�һ������
	MsWinsockUtil::m_lpfnGetAcceptExSockAddrs(ol->pBuf->szBuf, MAX_BUFFER - ((sizeof(SOCKADDR_IN)+16)*2),  
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);  

#ifdef _DEBUG
	this->_ShowMessage( "�ͻ��� %s:%d ����.", inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port) );
	this->_ShowMessage( "�ͻ��� %s:%d ��Ϣ��%s.",inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port),ol->pBuf->szBuf);
#endif

	//�󶨵���ɶ��ϡ�
	if (_AssociateWithIOCP(pSession)== false)
	{
		_ShowMessage("�½���socket ����ɶ˿�ʧ�ܣ�");
	}

	m_pActiveSessionList->Lock();
	m_pActiveSessionList->push_back(pSession);
	m_pActiveSessionList->UnLock();

	_SetSocketProp(pSession->GetSocket());

	//������Դ����һ������
	if (!_RecvPost(pSession,ol->pBuf->szBuf,dwBytes))
	{
		return false;
	}

	SAFE_DELETE(ol->pBuf);

	//��socket �ȴ���һ�����ݽ���
	if (!_PostRecv(pSession))
	{
		return false;
	}


	//�󶨵���ɶ˿����µĿͻ�������
	CSession *pNewSession = m_pSessionPool->Alloc();	

	//����Ͷ��
	return _PostAccept(pNewSession);
}

//tcp���ӳٷ��ͺͻ������0����������
void cIocpServer::_SetSocketProp( SOCKET socket )
{
	//tcp���ӳ�
	int nodelay = 1;
	setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));

	//���û������������
	int zero = 0;
	setsockopt(socket,SOL_SOCKET,SO_SNDBUF,(char*)&zero,sizeof(zero));

	zero = 0;
	setsockopt(socket,SOL_SOCKET,SO_RCVBUF,(char*)&zero,sizeof(zero));
}

bool cIocpServer::_PostRecv(CSession *pSession)
{
	//������Ϣ
	if (pSession == NULL)
		return false;
	else
	{
		return pSession->PreRecv();
	}
}

//�����������
bool cIocpServer::_DoRecv(CSession *pSession,OVERLAPPEDEX *ol, xe_uint32 dwBytes)
{
	if (pSession == NULL || ol == NULL || ol->pBuf)
		return false;
	else
	{
		//���ܵ���Ϣ
		//������Դ����һ������
		_RecvPost(pSession,ol->pBuf->szBuf,dwBytes);			//���͸���̨�߳�

		//�������Ͷ��
		return _PostRecv(pSession);		//�����ȴ�����
	}
}

//====================================================================================
//
//				       ����������������
//
//====================================================================================


std::string cIocpServer::GetLocalIP()
{
	// ��ñ���������
	char hostname[MAX_PATH] = {0};
	gethostname(hostname,MAX_PATH);                
	struct hostent FAR* lpHostEnt = gethostbyname(hostname);
	if(lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}

	// ȡ��IP��ַ�б��еĵ�һ��Ϊ���ص�IP(��Ϊһ̨�������ܻ�󶨶��IP)
	char* lpAddr = lpHostEnt->h_addr_list[0];      

	// ��IP��ַת�����ַ�����ʽ
	struct in_addr inAddr;
	memmove(&inAddr,lpAddr,4);

	m_strIP = inet_ntoa(inAddr);        

	return m_strIP;
}


/////////////////////////////////////////////////////////////////////
// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ���������һ����Ч��Socket��Ͷ��WSARecv����������쳣
// ʹ�õķ����ǳ��������socket�������ݣ��ж����socket���õķ���ֵ
// ��Ϊ����ͻ��������쳣�Ͽ�(����ͻ��˱������߰ε����ߵ�)��ʱ�򣬷����������޷��յ��ͻ��˶Ͽ���֪ͨ��
bool cIocpServer::_IsSocketAlive( SOCKET s )
{
	int nByteSent=send(s,"",0,0);
	if (-1 == nByteSent) return false;
	return true;
}


///////////////////////////////////////////////////////////////////
// ��ʾ��������ɶ˿��ϵĴ���
bool cIocpServer::HandleError( CSession *pSession,const xe_uint32& dwErr )
{
	// ����ǳ�ʱ�ˣ����ټ����Ȱ�  
	if(WAIT_TIMEOUT == dwErr)  
	{  	
		// ȷ�Ͽͻ����Ƿ񻹻���...
		if( !_IsSocketAlive( pSession->GetSocket()) )			//sendУ��
		{
			_ShowMessage( "��⵽�ͻ����쳣�˳���");
			//SOCKETPOOL->ReleasePerSocketSession(pContext);
			pSession->Remove();	//ɾ��
			return true;
		}
		else
		{
			this->_ShowMessage( "���������ʱ��������...");
			return true;
		}
	}  

	// �����ǿͻ����쳣�˳���
	else if( ERROR_NETNAME_DELETED==dwErr )		
	{
		this->_ShowMessage( "��⵽�ͻ����쳣�˳���");			//�ͻ����ǹ�Xֱ���˳�
		pSession->Remove();	//ɾ��
		return true;
	}

	else
	{
		this->_ShowMessage( "��ɶ˿ڲ������ִ����߳��˳���������룺%d",dwErr );
		return false;
	}
}


//////////////////////////////////////////////////////////////////
//	����������
bool cIocpServer::Start()
{
	m_bRunning = true;

	//socket����
	if (false == LoadSocketLib())
	{
		this->_ShowMessage("��ʼ��socket ����ʧ�ܣ�\n");
		return false;
	}

	// ����ϵͳ�˳����¼�֪ͨ,�ֶ����ã��տ�ʼ���ź�
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// ��ʼ��IOCP
	if (false == _InitializeIOCP())
	{
		this->_ShowMessage("��ʼ��IOCPʧ�ܣ�\n");
		return false;
	}
	else
	{
		this->_ShowMessage("IOCP��ʼ�����\n.");
	}

	// ��ʼ��Socket
	if( false==_InitializeListenSocket() )
	{
		this->_ShowMessage("Listen Socket��ʼ��ʧ�ܣ�\n");
		this->_DeInitialize();
		return false;
	}
	else
	{
		this->_ShowMessage("Listen Socket��ʼ�����.\n");
	}

	//��ʼ����̨�߳�
	if (false == _InitializeBackendThread())
	{
		this->_ShowMessage("��ʼ����̨�߳�ʧ��\n");
	}
	else
	{
		this->_ShowMessage("��ʼ����̨�̳߳ɹ�\n");
	}

	if (false == StartConnect())
	{
		this->_ShowMessage("��������ʧ��!\n");
	}
	else
	{
		this->_ShowMessage("���������ɹ�!\n");
	}

	this->_ShowMessage("ϵͳ׼���������Ⱥ�����....\n");

	return true;
}

void cIocpServer::Stop()
{
	if (m_pAcceptor != NULL && m_pAcceptor->GetListenSocket() != INVALID_SOCKET && m_pConnector)
	{
		m_pAcceptor->Shutdown();
		m_pConnector->Stop();			//ֹͣ

		//�ȵ����Ŀǰ�Ĳ�������
		ProcessSendPacket();
		ProcessRecvPacket();

		KillAllSession();
		KillDeadSession();

		//����ر��߳�
		SetEvent(m_hShutdownEvent); //

		//֪ͨ����workthread�ر�
		for (int i = 0; i < m_nThreads;++i)
		{
			PostQueuedCompletionStatus(m_hIOCompletionPort,0,(DWORD)EXIT_CODE,NULL);
		}

		//�ȴ������߳��˳�
		WaitForMultipleObjects(m_nThreads,m_phWorkerThreads,TRUE,INFINITE);

		//��̨�̳߳ɹ�
		WaitForSingleObject(m_hBackendThread,INFINITE);

		//�����shutdown
		//m_pSessionPool-

		_DeInitialize();

		this->_ShowMessage("ֹͣ����\n");
	}

	UnloadSocketLib();
}

//��������
bool cIocpServer::_PostSend(CSession *pSession)
{
	if (pSession == NULL)
		return false;
	else
	{
		return pSession->PreSend();
	}
}


//�ͷ���Դ
bool cIocpServer::_DoSend( CSession *pSession,OVERLAPPEDEX *ol )
{
	if (pSession == NULL || ol == NULL)
		return false;
	else
	{
		SAFE_DELETE(ol->pBuf);
	}
}


void cIocpServer::SendPacket( xe_uint32 cid,xe_uint8 bCmdGroup,xe_uint8 bCmd,char * buffer, int buflength )
{

}


void cIocpServer::_IoSend()
{

}

bool cIocpServer::_InitializeBackendThread()
{
	DWORD dwThread;
	m_hBackendThread = CreateThread(NULL,0,_BackEndThread,this,0,&dwThread);

	if (m_hBackendThread)
	{
		return true;
	}
	else
		return false;
}

bool cIocpServer::_RecvPost(CSession *pSession,char * szBuf,xe_uint32 dwBytes)
{
	if (pSession == NULL || szBuf == NULL)
		return false;
	else
	{

		if (!pSession->onRecv((xe_uint8*)szBuf,dwBytes))
		{
			//��Ч����ɾ��
			pSession->Remove();
			
			return false;
		}

		return true;
	}
}

void cIocpServer::_IoRecv()
{

}



void cIocpServer::_CloseSocket( SOCKET s )
{
	if (s != INVALID_SOCKET)
	{
		closesocket(s);
	}
}

void cIocpServer::ProcessSendPacket()
{
	m_pActiveSessionList->Lock();
	/*m_pActiveSessionList->push_back(pSession);*/
	CSession * pSession = NULL;
	for (CSessionList::iterator iter = m_pActiveSessionList->begin(); iter != m_pActiveSessionList->end(); ++iter)
	{
		//s������Ϣ
		pSession = *iter;
		if (pSession  == NULL || pSession->IsWillRemove())
			continue;

		if (pSession->PreSend() == false)
		{
			pSession->Remove();	//ɾ��
		}
	}
	m_pActiveSessionList->UnLock();
}

//�����յ��İ�
void cIocpServer::ProcessRecvPacket()
{
	m_pActiveSessionList->Lock();

	CSession *pSession = NULL;
	for (CSessionList::iterator iter = m_pActiveSessionList->begin(); iter != m_pActiveSessionList->end(); ++iter)
	{
		//s������Ϣ
		pSession = *iter;
		if (pSession  == NULL || pSession->IsWillRemove())
			continue;

		if (pSession->IsLimit())
		{
			if (pSession->GetSendBuffer()->IsEmpty())
			{
				pSession->Remove();	//ɾ���¼�
			}
		}
		else
		{
			if (pSession->ProcessRecvPack() == false)
			{
				pSession->Remove();	//ɾ��
			}
		}
	}
	m_pActiveSessionList->UnLock();
}

//ɾ����Чָ��
void cIocpServer::KillDeadSession()
{
	CSession *pSession = NULL;

	m_pTempSessionList->clear();

	m_pActiveSessionList->Lock();
	for (CSessionList::iterator iter = m_pActiveSessionList->begin();iter != m_pActiveSessionList->end();++iter)
	{
		pSession = *iter;
		if (pSession&& pSession->IsWillRemove())
		{
			m_pActiveSessionList->erase(iter--);
			m_pTempSessionList->push_back(pSession);
		}
		else if (pSession == NULL)
		{
			m_pActiveSessionList->erase(iter--);
		}
	}
	m_pActiveSessionList->UnLock();


	//SHANCHU 
	for (CSessionList::iterator iter2 = m_pTempSessionList->begin(); iter2 != m_pTempSessionList->end(); ++iter2)
	{
		//
		pSession = *iter2;
		if (pSession)
		{
			//���
			INetWorkObj *pNetWork = pSession->GetNetWorkObj();

			pSession->UnBindNetWork();

			//�����¼�
			//�ر�socket�ͳ�ʼ��
			pSession->CloseScket();
			pSession->Init();

			m_pSessionPool->Free(pSession);

			if (pNetWork)
			{
				pNetWork->OnDisconnect();		//�����¼�
			}
		}
	}

	m_pTempSessionList->clear();
}

void cIocpServer::KillAllSession()
{
	CSession *pSession = NULL;

	m_pActiveSessionList->Lock();
	for (CSessionList::iterator iter = m_pActiveSessionList->begin();iter != m_pActiveSessionList->end();++iter)
	{
		pSession = *iter;
		if (pSession)
			pSession->Remove();
	}
	m_pActiveSessionList->UnLock();
}

void cIocpServer::Update()
{
	ProcessRecvPacket();
	ConnectInActive();
	KillDeadSession();
}

//��connect���Active
void cIocpServer::ConnectInActive()
{
	if (m_pConnectSuccessList->empty())
		return;

	//CSessionList *pTmp = new CSessionList();

	m_pConnectSuccessList->Lock();
	m_pTempSessionList->splice( m_pTempSessionList->end(), *m_pConnectSuccessList);
	m_pConnectSuccessList->UnLock();

	//����
	CSession *pSession = NULL;
	for (CSessionList::iterator iter = m_pTempSessionList->begin(); iter != m_pTempSessionList->end(); ++iter)
	{
		pSession = *iter;
		
		if(pSession != NULL)
		{
			if (_AssociateWithIOCP(pSession))
			{
				//���ӵ�������������
				if (pSession->PreRecv())
				{
					//���ճɹ�
					pSession->onConnect(true);
				}
				else
				{
					//����ʧ��
					pSession->onConnect(false);

					m_pTempSessionList->erase(iter--);
					m_pConnector->AddConnectList(pSession);
				}
			}
			else
			{
				//������ʧ�ܴ���
				m_pTempSessionList->erase(iter--);
				m_pConnector->AddConnectList(pSession);
			}
		}
	}

	if (!m_pTempSessionList->empty())
	{
		m_pActiveSessionList->Lock();
		m_pActiveSessionList->splice(m_pActiveSessionList->end(),*m_pTempSessionList);
		m_pActiveSessionList->UnLock();
		m_pTempSessionList->Clear();
	}
}

bool cIocpServer::StartConnect()
{
	//��������

	return m_pConnector->Start();
}

void cIocpServer::AddConnectSuccessList(CSession *pSession)
{
	m_pConnectSuccessList->Lock();
	m_pConnectSuccessList->push_back(pSession);
	m_pConnectSuccessList->UnLock();
}
