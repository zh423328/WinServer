#include "Session.h"
#include "SessionList.h"
#include "SessionPool.h"
#include "TcpServer.h"
#include "Connector.h"
#include "MsWinsockUtil.h"
#include "SendBuffer.h"
#include "RecvBuffer.h"


DWORD WINAPI cIocpServer::_WorkerThread( LPVOID lpParam )
{
	cIocpServer *pServer = (cIocpServer*)lpParam;
	if (pServer)
	{
		//
		OVERLAPPEDEX *pIoContext = NULL;
		CSession *pSession = NULL;

		xe_uint32 dwBytes = 0;

		//循环处理请求，直到收到shutdown消息
		while (WAIT_OBJECT_0 != WaitForSingleObject(pServer->m_hShutdownEvent,0))
		{
			BOOL bRet = GetQueuedCompletionStatus(pServer->m_hIOCompletionPort,(LPDWORD)&dwBytes,(PULONG_PTR)&pSession,(LPOVERLAPPED*)&pIoContext,INFINITE);
			
			if (pSession == EXIT_CODE)
				break;					//程序结束

			if (bRet == FALSE || pIoContext == NULL || pSession == NULL)
			{
				//退出
				DWORD dwErr = GetLastError();

				if (pSession != NULL)
				{
					// 显示一下提示信息,退出返回true
					if( !pServer->HandleError( pSession,dwErr ) )			
					{
						break;
					}

				}

				continue;  
			}
			else
			{
				// 判断是否有客户端断开了
				if ((dwBytes == 0) && (pIoContext->dwOperator == IOCP_REQUEST_SEND  || pIoContext->dwOperator == IOCP_REQUEST_RECV))
				{
					//客户端掉线了
					pServer->_ShowMessage("客户端 %s:%d 断开连接.",pSession->GetIP(), pSession->GetIndex() );
					
					//删除所有内存中的pool
					//SOCKETPOOL->ReleasePerSocketSession(pSession);
					pSession->Remove();
					continue;
				}
				else
				{
					switch(pIoContext->dwOperator)
					{
					case IOCP_REQUEST_ACCEPT:
						//处理accept
						pServer->_DoAccpet(pSession,pIoContext,dwBytes);
						break;
					case IOCP_REQUEST_RECV:
						//处理read
						pServer->_DoRecv(pSession,pIoContext,dwBytes);
						break;
					case IOCP_REQUEST_SEND:
						//处理recv
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
			//发送线程
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

	m_hIOCompletionPort = NULL;		//io端口
	m_hShutdownEvent = NULL;
	m_pListenSession = NULL;		//服务器监听
	
	m_hBackendThread = NULL;

	m_pSessionPool = new CSessionPool(2000);		//缓冲池

	m_pActiveSessionList = new CSessionList();

	m_pTempSessionList = new CSessionList();

	m_pConnectSuccessList = new CSessionList();		//连接成功

	m_pConnector = new CConnector(this);

	m_bRunning = false;
}

cIocpServer::~cIocpServer()
{
	//释放资源
	Stop();
	SAFE_DELETE(m_pSessionPool);
	SAFE_DELETE(m_pActiveSessionList);
	SAFE_DELETE(m_pTempSessionList);
	SAFE_DELETE(m_pTempSessionList);
}


//====================================================================================
//				    网络消息处理
//====================================================================================
void cIocpServer::_ShowMessage( char*szFormat,... )
{
	va_list ap;

	char szText[1024] = {0};
	va_start(ap,szFormat);
	vsprintf(szText,szFormat,ap);
	va_end(ap);

	//处理输出消息
	printf("%s",szText);		//暂时这样处理
}

//====================================================================================
//
//				    IOCP初始化和终止
//
//====================================================================================

bool cIocpServer::LoadSocketLib()
{
	WSADATA wd;
	int nResult;

	nResult = WSAStartup(MAKEWORD(2,2),&wd);

	if(nResult != NO_ERROR)
	{
		_ShowMessage("Init socket envirment error!");		//初始化socket环境
		return false;
	}

	return true;
}

//获取系统cpu
int cIocpServer::_GetNoOfProcessors()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);

	return info.dwNumberOfProcessors;
}

//初始化完成端口
bool cIocpServer::_InitializeIOCP()
{
	//建立完成端口
	m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,0);
	if (m_hIOCompletionPort == NULL)
	{
		_ShowMessage("call CreateIoCompletionPort failed");
	}

	//工作线程数量
	m_nThreads = 2 * _GetNoOfProcessors();	

	//工作线程句柄
	m_phWorkerThreads = new HANDLE[m_nThreads];


	//建立工作者线程
	DWORD dwThreadId;
	for (int i = 0; i < m_nThreads; ++i)
	{
		m_phWorkerThreads[i] = CreateThread(NULL,0,_WorkerThread,this,0,&dwThreadId);
	}

	_ShowMessage("iocp has success created!\n");

	return true;
}

//初始化服务器socket
bool cIocpServer::_InitializeListenSocket()
{
	//服务器socket创建
	SOCKADDR_IN serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(m_nPort);
	serAddr.sin_addr.s_addr = m_strIP.empty() ? htonl(INADDR_ANY) : inet_addr(m_strIP.c_str());


	m_pListenSession = m_pSessionPool->Alloc();

	if (m_pListenSession)
	{
		// 需要使用重叠IO，必须得使用WSASocket来建立Socket，才可以支持重叠IO操作
		m_pListenSession->SetSocket(m_pListenSession->CreateSocket());
		m_pListenSession->SetSockAddr(serAddr);


		if (m_pListenSession->GetSocket() == INVALID_SOCKET)
		{
			_ShowMessage("init listen socket error : %d!\n",WSAGetLastError());
			return false;
		}

		//// 将Listen Socket绑定至完成端口中
		if (CreateIoCompletionPort((HANDLE)m_pListenSession->GetSocket(),m_hIOCompletionPort,(xe_uint32)m_pListenSession,0) == NULL)
		{
			_ShowMessage("绑定 Listen Socket至完成端口失败！错误代码: %d\n", WSAGetLastError());  
			SAFE_DELETE( m_pListenSession);
			return false;
		}

		//bind
		int ret = bind(m_pListenSession->GetSocket(), (struct sockaddr *) &serAddr, sizeof(serAddr));
		if (ret == SOCKET_ERROR)
		{
			this->_ShowMessage("bind()函数执行错误.\n");  
			SAFE_DELETE( m_pListenSession);
			return false;
		}


		// 开始进行监听
		if (SOCKET_ERROR == listen(m_pListenSession->GetSocket(),SOMAXCONN))
		{
			this->_ShowMessage("Listen()函数执行出现错误.\n");

			SAFE_DELETE( m_pListenSession);
			return false;
		}

		_SetSocketProp(m_pListenSession->GetSocket());

		MsWinsockUtil::LoadExtensionFunction(m_pListenSession->GetSocket());


		// 为AcceptEx 准备参数，然后投递AcceptEx I/O请求，投递accept
		for( int i=0;i<MAX_POST_ACCEPT;i++ )
		{
			// 新建一个IO_CONTEXT
			CSession *pTmpSession = m_pSessionPool->Alloc();

			if (pTmpSession)
			{
				//投递
				if( false==this->_PostAccept( pTmpSession ) )
				{
					pTmpSession->Remove();
					return false;
				}
			}
			
		}

		_ShowMessage( "投递 %d 个AcceptEx请求完毕",MAX_POST_ACCEPT );

		
		return true;
	}

	return false;
}

void cIocpServer::_DeInitialize()
{
	// 关闭系统退出事件句柄
	RELEASE_HANDLE(m_hShutdownEvent);

	// 释放工作者线程句柄指针
	for( int i=0;i<m_nThreads;i++ )
	{
		RELEASE_HANDLE(m_phWorkerThreads[i]);
	}

	SAFE_DELETE(m_phWorkerThreads);

	RELEASE_HANDLE(m_hBackendThread);

	// 关闭IOCP句柄
	RELEASE_HANDLE(m_hIOCompletionPort);

	// 关闭监听Socket
	//SAFE_DELETE(m_pListenSession);
	SAFE_DELETE(m_pListenSession);

	m_pListenSession = NULL;

	_ShowMessage("释放资源完毕.\n");
}

/*******************
客户端获取端口绑定到完成端口上

*********************/
bool cIocpServer::_AssociateWithIOCP( CSession *pSession )
{
	// 将用于和客户端通信的SOCKET绑定到完成端口中
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pSession->GetSocket(), m_hIOCompletionPort, (xe_uint32)pSession, 0);

	if (NULL == hTemp)
	{
		this->_ShowMessage("执行CreateIoCompletionPort()出现错误.错误代码：%d",GetLastError());
		return false;
	}

	return true;
}

//投递请求
bool cIocpServer::_PostAccept( CSession* pAcceptIoContext )
{
	if (pAcceptIoContext)
	{
		return pAcceptIoContext->PreAccept(m_pListenSession->GetSocket());
	}

	return false;
}


////////////////////////////////////////////////////////////
// 在有客户端连入的时候，进行处理

// 总之你要知道，传入的是ListenSocket的Context，我们需要复制一份出来给新连入的Socket用
// 原来的Context还是要在上面继续投递下一个Accept请求
// 当完成端口接受到accpet消息时，GetAcceptExSockaddrs同时获取到第一次消息
bool cIocpServer::_DoAccpet(CSession *pSession,OVERLAPPEDEX *ol,xe_uint32 dwBytes)
{
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;  
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);  

	//假设把服务器端的iocontext接受到的消息buffer重置，再用m_lpfnGetAcceptExSockAddrs去获取第一次消息
	//pIoContext->ResetBuffer();

	///////////////////////////////////////////////////////////////////////////
	// 1. 首先取得连入客户端的地址信息
	// 这个 m_lpfnGetAcceptExSockAddrs 不得了啊~~~~~~
	// 不但可以取得客户端和本地端的地址信息，还能顺便取出客户端发来的第一组数据
	MsWinsockUtil::m_lpfnGetAcceptExSockAddrs(ol->pBuf->szBuf, MAX_BUFFER - ((sizeof(SOCKADDR_IN)+16)*2),  
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);  

#ifdef _DEBUG
	this->_ShowMessage( "客户端 %s:%d 连入.", inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port) );
	this->_ShowMessage( "客户额 %s:%d 信息：%s.",inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port),ol->pBuf->szBuf);
#endif

	//绑定到完成端上。
	if (_AssociateWithIOCP(pSession)== false)
	{
		_ShowMessage("新接入socket 绑定完成端口失败！");
	}

	m_pActiveSessionList->Lock();
	m_pActiveSessionList->push_back(pSession);
	m_pActiveSessionList->UnLock();

	_SetSocketProp(pSession->GetSocket());

	//这里可以处理第一组数据
	if (!_RecvPost(pSession,ol->pBuf->szBuf,dwBytes))
	{
		return false;
	}

	SAFE_DELETE(ol->pBuf);

	//该socket 等待下一次数据接收
	if (!_PostRecv(pSession))
	{
		return false;
	}


	//绑定到完成端口上新的客户端连接
	CSession *pNewSession = m_pSessionPool->Alloc();	

	//继续投递
	return _PostAccept(pNewSession);
}

//tcp无延迟发送和缓存池设0，立即发送
void cIocpServer::_SetSocketProp( SOCKET socket )
{
	//tcp无延迟
	int nodelay = 1;
	setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));

	//设置缓冲池立即发送
	int zero = 0;
	setsockopt(socket,SOL_SOCKET,SO_SNDBUF,(char*)&zero,sizeof(zero));

	zero = 0;
	setsockopt(socket,SOL_SOCKET,SO_RCVBUF,(char*)&zero,sizeof(zero));
}

bool cIocpServer::_PostRecv(CSession *pSession)
{
	//接受消息
	if (pSession == NULL)
		return false;
	else
	{
		return pSession->PreRecv();
	}
}

//处理接收数据
bool cIocpServer::_DoRecv(CSession *pSession,OVERLAPPEDEX *ol, xe_uint32 dwBytes)
{
	if (pSession == NULL || ol == NULL || ol->pBuf)
		return false;
	else
	{
		//接受到消息
		//这里可以处理第一组数据
		_RecvPost(pSession,ol->pBuf->szBuf,dwBytes);			//发送给后台线程

		//处理继续投递
		return _PostRecv(pSession);		//继续等待接受
	}
}

//====================================================================================
//
//				       其他辅助函数定义
//
//====================================================================================


std::string cIocpServer::GetLocalIP()
{
	// 获得本机主机名
	char hostname[MAX_PATH] = {0};
	gethostname(hostname,MAX_PATH);                
	struct hostent FAR* lpHostEnt = gethostbyname(hostname);
	if(lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}

	// 取得IP地址列表中的第一个为返回的IP(因为一台主机可能会绑定多个IP)
	char* lpAddr = lpHostEnt->h_addr_list[0];      

	// 将IP地址转化成字符串形式
	struct in_addr inAddr;
	memmove(&inAddr,lpAddr,4);

	m_strIP = inet_ntoa(inAddr);        

	return m_strIP;
}


/////////////////////////////////////////////////////////////////////
// 判断客户端Socket是否已经断开，否则在一个无效的Socket上投递WSARecv操作会出现异常
// 使用的方法是尝试向这个socket发送数据，判断这个socket调用的返回值
// 因为如果客户端网络异常断开(例如客户端崩溃或者拔掉网线等)的时候，服务器端是无法收到客户端断开的通知的
bool cIocpServer::_IsSocketAlive( SOCKET s )
{
	int nByteSent=send(s,"",0,0);
	if (-1 == nByteSent) return false;
	return true;
}


///////////////////////////////////////////////////////////////////
// 显示并处理完成端口上的错误
bool cIocpServer::HandleError( CSession *pSession,const xe_uint32& dwErr )
{
	// 如果是超时了，就再继续等吧  
	if(WAIT_TIMEOUT == dwErr)  
	{  	
		// 确认客户端是否还活着...
		if( !_IsSocketAlive( pSession->GetSocket()) )			//send校验
		{
			_ShowMessage( "检测到客户端异常退出！");
			//SOCKETPOOL->ReleasePerSocketSession(pContext);
			pSession->Remove();	//删除
			return true;
		}
		else
		{
			this->_ShowMessage( "网络操作超时！重试中...");
			return true;
		}
	}  

	// 可能是客户端异常退出了
	else if( ERROR_NETNAME_DELETED==dwErr )		
	{
		this->_ShowMessage( "检测到客户端异常退出！");			//客户端是关X直接退出
		pSession->Remove();	//删除
		return true;
	}

	else
	{
		this->_ShowMessage( "完成端口操作出现错误，线程退出。错误代码：%d",dwErr );
		return false;
	}
}


//////////////////////////////////////////////////////////////////
//	启动服务器
bool cIocpServer::Start()
{
	m_bRunning = true;

	//socket环境
	if (false == LoadSocketLib())
	{
		this->_ShowMessage("初始化socket 环境失败！\n");
		return false;
	}

	// 建立系统退出的事件通知,手动重置，刚开始无信号
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// 初始化IOCP
	if (false == _InitializeIOCP())
	{
		this->_ShowMessage("初始化IOCP失败！\n");
		return false;
	}
	else
	{
		this->_ShowMessage("IOCP初始化完毕\n.");
	}

	// 初始化Socket
	if( false==_InitializeListenSocket() )
	{
		this->_ShowMessage("Listen Socket初始化失败！\n");
		this->_DeInitialize();
		return false;
	}
	else
	{
		this->_ShowMessage("Listen Socket初始化完毕.\n");
	}

	//初始化后台线程
	if (false == _InitializeBackendThread())
	{
		this->_ShowMessage("初始化后台线程失败\n");
	}
	else
	{
		this->_ShowMessage("初始化后台线程成功\n");
	}

	if (false == StartConnect())
	{
		this->_ShowMessage("连接启动失败!\n");
	}
	else
	{
		this->_ShowMessage("连接启动成功!\n");
	}

	this->_ShowMessage("系统准备就绪，等候连接....\n");

	return true;
}

void cIocpServer::Stop()
{
	if (m_pListenSession != NULL && m_pListenSession->GetSocket() != INVALID_SOCKET && m_pConnector)
	{
		m_pConnector->Stop();			//停止

		//先等玩家目前的操作结束
		ProcessSendPacket();
		ProcessRecvPacket();

		KillAllSession();
		KillDeadSession();

		//激活关闭线程
		SetEvent(m_hShutdownEvent); //

		//通知所有workthread关闭
		for (int i = 0; i < m_nThreads;++i)
		{
			PostQueuedCompletionStatus(m_hIOCompletionPort,0,(DWORD)EXIT_CODE,NULL);
		}

		//等待所有线程退出
		WaitForMultipleObjects(m_nThreads,m_phWorkerThreads,TRUE,INFINITE);

		//后台线程成功
		WaitForSingleObject(m_hBackendThread,INFINITE);

		//缓冲池shutdown
		//m_pSessionPool-

		_DeInitialize();

		this->_ShowMessage("停止监听\n");
	}

	UnloadSocketLib();
}

//发送数据
bool cIocpServer::_PostSend(CSession *pSession)
{
	if (pSession == NULL)
		return false;
	else
	{
		return pSession->PreSend();
	}
}


//释放资源
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
			//无效包，删除
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
		//s发送消息
		pSession = *iter;
		if (pSession  == NULL || pSession->IsWillRemove())
			continue;

		if (pSession->PreSend() == false)
		{
			pSession->Remove();	//删除
		}
	}
	m_pActiveSessionList->UnLock();
}

//处理收到的包
void cIocpServer::ProcessRecvPacket()
{
	m_pActiveSessionList->Lock();

	CSession *pSession = NULL;
	for (CSessionList::iterator iter = m_pActiveSessionList->begin(); iter != m_pActiveSessionList->end(); ++iter)
	{
		//s发送消息
		pSession = *iter;
		if (pSession  == NULL || pSession->IsWillRemove())
			continue;

		if (pSession->IsLimit())
		{
			if (pSession->GetSendBuffer()->IsEmpty())
			{
				pSession->Remove();	//删除事件
			}
		}
		else
		{
			if (pSession->ProcessRecvPack() == false)
			{
				pSession->Remove();	//删除
			}
		}
	}
	m_pActiveSessionList->UnLock();
}

//删除无效指针
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
			//关闭socket和初始化
			pSession->CloseScket();
			pSession->Init();

			m_pSessionPool->Free(pSession);
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

//把connect编程Active
void cIocpServer::ConnectInActive()
{
	if (m_pConnectSuccessList->empty())
		return;

	//CSessionList *pTmp = new CSessionList();

	m_pConnectSuccessList->Lock();
	m_pTempSessionList->splice( m_pTempSessionList->end(), *m_pConnectSuccessList);
	m_pConnectSuccessList->UnLock();

	//链表
	CSession *pSession = NULL;
	for (CSessionList::iterator iter = m_pTempSessionList->begin(); iter != m_pTempSessionList->end(); ++iter)
	{
		pSession = *iter;
		
		if(pSession != NULL)
		{
			if (_AssociateWithIOCP(pSession))
			{
				//连接的是其他服务器
				if (pSession->PreRecv())
				{
					//接收成功
					pSession->onConnect(true);
				}
				else
				{
					//接收失败
					pSession->onConnect(false);

					m_pTempSessionList->erase(iter--);
					m_pConnector->AddConnectList(pSession);
				}
			}
			else
			{
				//按连接失败处理
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
	//启动连接

	return m_pConnector->Start();
}

void cIocpServer::AddConnectSuccessList(CSession *pSession)
{
	m_pConnectSuccessList->Lock();
	m_pConnectSuccessList->push_back(pSession);
	m_pConnectSuccessList->UnLock();
}
