#include "TcpServer.h"
#include "Acceptor.h"
#include "MsWinsockUtil.h"
#include "Log.h"
#include "Session.h"

//监听线程
DWORD WINAPI CAcceptor::accept_thread( LPVOID param )
{
	CAcceptor		*pAcceptor = (CAcceptor*)param;
	cIocpServer		*pServer = pAcceptor->m_pParent;

	BOOL			bSuccess = FALSE;
	DWORD			dwIoSize = 0;
	OVERLAPPEDEX	*pOverlappedEx = NULL;
	void			*pDummy = NULL;

	SOCKADDR		*lpLocalSockaddr = NULL;
	SOCKADDR		*lpRemoteSockaddr = NULL;
	int				nLocalSockaddrLen = 0;
	int				nRemoteSockaddrLen = 0;

	int				err;

	while(true)
	{
		bSuccess = GetQueuedCompletionStatus(pAcceptor->m_hIOCP,&dwIoSize,(LPDWORD)&pDummy,(LPOVERLAPPED*)&pOverlappedEx,INFINITE);

		if (pDummy == EXIT_CODE)
			return false;

		if (!bSuccess)
		{
			err = WSAGetLastError();

			if( err == ERROR_OPERATION_ABORTED )
			{	
				pOverlappedEx->pSession->Remove();		//删除
				continue;
			}
			else if( err != ERROR_NETNAME_DELETED )
			{
				continue;
			}

			break;
		}
		else
		{
			switch(pOverlappedEx->dwOperator)
			{
			case IOCP_REQUEST_ACCEPT:
				pServer->_DoAccpet(pOverlappedEx->pSession,pOverlappedEx,dwIoSize);
				break;
			}
		}
	}

	return 0;
}


CAcceptor::CAcceptor( cIocpServer *pParent )
{
	m_pParent = pParent;

	m_hThreads = NULL;

	m_hIOCP = INVALID_HANDLE_VALUE;

	m_listenSocket = INVALID_SOCKET;

	memset(&m_sockaddr,0,sizeof(SOCKADDR_IN));

	m_numThreads = 0;
}

CAcceptor::~CAcceptor()
{
	Shutdown();

	//关闭
	CloseHandle(m_hIOCP);

	for (xe_uint32 i = 0; i < m_numThreads; ++i)
	{
		CloseHandle(m_hThreads[i]);
	}

	SAFE_DELETE_ARRAY(m_hThreads);
}

//初始化
void CAcceptor::Init( cIocpServer *pIoHandler, xe_uint32 dwNumberOfThreads )
{
	m_pParent = pIoHandler;

	m_numThreads = dwNumberOfThreads;

	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,0);

	m_hThreads = new HANDLE[m_numThreads];

	//创建线程
	xe_uint32 dwThreadIdx;
	for (xe_uint32 i = 0; i < m_numThreads; ++i)
	{
		m_hThreads[i]= CreateThread(NULL,0,accept_thread,this,0,(LPDWORD)&dwThreadIdx);
	}
}

//添加监听
bool CAcceptor::StartListen( char *pIP, xe_uint16 wPort )
{
	if (!CreateListenSocket(pIP,wPort))
		return false;

	CreateIoCompletionPort((HANDLE)m_listenSocket,m_hIOCP,NULL,0);		//绑定完成端口

	MsWinsockUtil::LoadExtensionFunction(m_listenSocket);

	return true;
}

//关闭
void CAcceptor::Shutdown()
{
	if( m_listenSocket != INVALID_SOCKET )		
		closesocket( m_listenSocket );

	for( xe_uint32 i = 0; i < m_numThreads; ++i )	
		PostQueuedCompletionStatus( m_hIOCP, 0, EXIT_CODE, NULL );

	//等待关闭
	WaitForMultipleObjects( m_numThreads, m_hThreads, TRUE, INFINITE );
}

//暂停
void CAcceptor::SuspendListenThread()
{
	for (xe_uint32 i = 0; i < m_numThreads; ++i)
	{
		SuspendThread(m_hThreads[i]);
	}
}

//恢复
void CAcceptor::ResumeListenThread()
{
	for (xe_uint32 i = 0; i < m_numThreads; ++i)
	{
		ResumeThread(m_hThreads[i]);
	}
}

//创建
bool CAcceptor::CreateListenSocket( char *pIP, xe_uint16 wPort )
{
	if (m_listenSocket != INVALID_SOCKET)
		return false;

	m_listenSocket = WSASocket(AF_INET,SOCK_STREAM,0,0,0,WSA_FLAG_OVERLAPPED);

	if (m_listenSocket == INVALID_SOCKET)
	{
		return false;
	}

	//绑定
	m_sockaddr.sin_family		= AF_INET;
	m_sockaddr.sin_addr.s_addr	= ( pIP == NULL || strlen( pIP ) == 0 ) ? htonl(INADDR_ANY) : inet_addr(pIP);
	m_sockaddr.sin_port			= htons( wPort );

	xe_int32 err = bind(m_listenSocket,(sockaddr*)&m_sockaddr,sizeof(sockaddr));
	if (err == SOCKET_ERROR)
	{
		sLog.OutPutError("[CAccept] bind port error!");
		closesocket(m_listenSocket);
		return false;
	}

	//监听
	err = listen(m_listenSocket,SOMAXCONN);
	if ( err == SOCKET_ERROR)
	{
		sLog.OutPutError("[CAccept] listen error!");
		closesocket(m_listenSocket);
		return false;
	}

	return true;
}



