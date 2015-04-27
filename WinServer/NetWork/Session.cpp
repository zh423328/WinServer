#include "NetWorkObj.h"
#include "Gen.h"
#include "MsWinsockUtil.h"
#include "Log.h"
#include "Session.h"
#include "PackBuffer.h"
#include "SendBuffer.h"
#include "RecvBuffer.h"

CSession::CSession()
{
	m_nIndex = 0;//CGenManager::shareGenManager()->GenSessionIdx();

	m_socket = INVALID_SOCKET;

	memset(&m_sockaddr,0,sizeof(SOCKADDR_IN));		//���û���


	//��д�ַ���
	m_pRecvBuffer = new CRecvBuffer();
	m_pSendBuffer = new CSendBuffer();

	m_bRemove = false;
}

CSession::~CSession()
{
	SAFE_DELETE(m_pSendBuffer);
	SAFE_DELETE(m_pRecvBuffer);
}

void CSession::Init()
{
	//����
	m_pRecvBuffer->Clear();
	m_pSendBuffer->Clear();

	m_bRemove = false;

	m_bLimit = false;
}

SOCKET CSession::CreateSocket()
{
	SOCKET newSocket = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED);

	if (newSocket != INVALID_SOCKET)
	{
		SetSocketBasicOpt();
	}
	return newSocket;
}

//���û���opt
void CSession::SetSocketBasicOpt()
{
	//��������
	int nodelay = 1;
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));

	//���û������������
	int zero = 0;
	setsockopt(m_socket,SOL_SOCKET,SO_SNDBUF,(char*)&zero,sizeof(zero));

	zero = 0;
	setsockopt(m_socket,SOL_SOCKET,SO_RCVBUF,(char*)&zero,sizeof(zero));
}

bool CSession::PreAccept( SOCKET listenSocket )
{
	//���ж�socket �Ƿ���Ч
	if (m_socket == INVALID_SOCKET)
	{
		m_socket = CreateSocket();
	}

	//��ʼ��
	Init();

	WSABUF wsaBuf;
	stPackBuffer *pBuf = new stPackBuffer();
	wsaBuf.buf = pBuf->szBuf;
	wsaBuf.len = pBuf->nLen;

	//����
	memset(&m_AccpetIo,0,sizeof(OVERLAPPEDEX));
	m_AccpetIo.dwOperator = IOCP_REQUEST_ACCEPT;
	m_AccpetIo.pSession = this;
	m_AccpetIo.pBuf = NULL;


	DWORD dwRecvBytes = 0;
	//����
	//AcceptEx(listenSocket,m_socket,szBuffer,0,sizeof(SOCKADDR_IN) + 16,sizeof(SOCKADDR_IN)+16,&dwRecvBytes,&m_AccpetIo);
	int nRet = MsWinsockUtil::m_lpfnAccepteEx(listenSocket,m_socket,wsaBuf.buf,wsaBuf.len,sizeof(SOCKADDR_IN) + 16,sizeof(SOCKADDR_IN)+16,&dwRecvBytes,&m_AccpetIo);

	if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		//
		Remove();
		SAFE_DELETE(pBuf);
		return false;
	}

	return true;
}

//Ͷ�ݺ������ptPackBuffer
bool CSession::PreRecv()
{
	//����
	WSABUF wsaBuf;

	stPackBuffer *pBuf = new stPackBuffer();
	wsaBuf.buf = pBuf->szBuf;
	wsaBuf.len = pBuf->nLen;

	memset(&m_RecvIo,0,sizeof(OVERLAPPEDEX));
	m_RecvIo.dwOperator = IOCP_REQUEST_RECV;
	m_RecvIo.pSession = this;
	m_RecvIo.pBuf = pBuf;

	int nRet = WSARecv(m_socket,&wsaBuf,1,(LPDWORD)&m_RecvIo.dwIOSize,(LPDWORD)&m_RecvIo.dwFlag,&m_RecvIo,NULL);

	if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		SAFE_DELETE(pBuf);
		Remove();
		return false;
	}

	return true;
}

//Ͷ��������ͷ���Դ
bool CSession::PreSend()
{
	//����
	WSABUF wsaBuf;

	stPackBuffer *pBuffer = m_pSendBuffer->PeerPacket();

	while(pBuffer)
	{
		wsaBuf.buf = pBuffer->szBuf;
		wsaBuf.len = pBuffer->nLen;

		memset(&m_SendIo,0,sizeof(OVERLAPPEDEX));
		m_SendIo.dwOperator = IOCP_REQUEST_SEND;
		m_SendIo.pBuf = pBuffer;
		m_SendIo.pSession = this;

		int nRet = WSASend(m_socket,&wsaBuf,1,(LPDWORD)&m_SendIo.dwIOSize,m_SendIo.dwFlag,&m_SendIo,NULL);

		if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			Remove();
			SAFE_DELETE(pBuffer);
			return false;
		}

		pBuffer = m_pSendBuffer->PeerPacket();
	}

	return true;
}

bool CSession::onAccept()
{
	//����ʱ������Ϣ
	m_bRemove = false;

	if (m_pNetWork)
		m_pNetWork->OnAccept(GetIndex());

	return true;
}

bool CSession::onSend()
{
	return true;
}

bool CSession::onRecv(xe_uint8 *pMsg,xe_uint16 nLen)
{
	if(m_pRecvBuffer)
		m_pRecvBuffer->Write(pMsg,nLen);

	return true;
}

bool CSession::ProcessRecvPack()
{
	stPackBuffer *pPacket = m_pRecvBuffer->PeerPacket();		//��ȡ��

	while (pPacket)
	{
		//������
		if(pPacket->nLen <= sizeof(stPackHeader))
		{
			SAFE_DELETE(pPacket);
			return false;
		}

		//���ݳ���
		if (pPacket->nLen >= MAX_BUFFER)
		{
			SAFE_DELETE(pPacket);
			return false;
		}


		//�����
		if (m_pNetWork)
		{
			m_pNetWork->OnRecv((xe_uint8*)pPacket->szBuf,pPacket->nLen);
		}

		SAFE_DELETE(pPacket);

		pPacket = m_pRecvBuffer->PeerPacket();
	}
	return true;
}

bool CSession::SendMsg( stPackHeader *pHeader,xe_uint8 *pMsg,xe_uint16 nLen )
{
	if (pHeader == NULL || pMsg == NULL)
		return false;

	bool bRet =  m_pSendBuffer->Write(pHeader,pMsg,nLen);

	if (bRet == false)
	{
		sLog.OutPutStr("ERROR[CMDGROUP:%d][CMD:%d][SIZE:%d]",pHeader->bCmdGroup,pHeader->bCmd,nLen);
	}
	return bRet;
}

bool CSession::OutputLog( xe_int32 nType,char * szFormat,... )
{
	char szMsg[1024] = {0};

	va_list ap;
	va_start(ap,szFormat);
	vsprintf(szMsg,szFormat,ap);
	va_end(ap);

	return true;
}

bool CSession::Disconnet( bool bDelete )
{
	if (bDelete)
	{
		m_bRemove = true;
	}
	else
	{
		m_bLimit = true;		//����������
	}

	return true;
}

bool CSession::onConnect( bool bSuccess )
{
	//���ӳɹ���ʲô��ʧ������ʲô��..

	Init();

	INetWorkObj *pNetworkObject = m_pNetWork;

	if( !bSuccess )
	{		
		UnBindNetWork();		//ʧ��
	}
	if (m_pNetWork)
		pNetworkObject->OnConnect( bSuccess, GetIndex() );

	return true;
}

void CSession::BindNetWork( INetWorkObj *pObj )
{
	m_pNetWork = pObj;
	m_pNetWork->SetSession(this);
}

void CSession::UnBindNetWork()
{
	if (m_pNetWork == NULL)
		return;

	m_pNetWork->SetSession(NULL);

	m_pNetWork = NULL;
}








