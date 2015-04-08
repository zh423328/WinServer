#ifndef SESSION_H_
#define SESSION_H_

//////////////////////////////////////////////////////////////////////////
//Session��װ
//////////////////////////////////////////////////////////////////////////

#include "GlobalDefine.h"
#include "PackBuffer.h"

//overapped ����
//////////////////////////////////////////////////////////////////
// ����ɶ˿���Ͷ�ݵ�I/O����������
enum IOCP_REQUESTTYPE
{
	IOCP_REQUEST_NULL,
	IOCP_REQUEST_ACCEPT,						
	IOCP_REQUEST_RECV ,								 // read
	IOCP_REQUEST_SEND,								 // write
};


class CSession;
//overplapped �첽����
struct OVERLAPPEDEX : public OVERLAPPED 
{
	xe_uint32 dwIOSize;
	xe_uint32 dwFlag;
	xe_uint8  dwOperator;
	CSession* pSession;
	stPackBuffer *pBuf;
};

class CRecvBuffer;
class CSendBuffer;

//������
class CSession
{
public:
	CSession();
	virtual ~CSession();

	void Init();

	//���õ�ַ
	inline void SetSockAddr(SOCKADDR_IN & sockaddr) { m_sockaddr = sockaddr;}

	//��ȡ��ַ
	inline SOCKADDR* GetSockAddr()	{ return (SOCKADDR*)&m_sockaddr;}

	//��ȡ��ַ
	inline char*  GetIP()		{ return inet_ntoa(m_sockaddr.sin_addr);}

	//���úͻ�ȡsocket
	inline void   SetSocket(SOCKET sock)		{ m_socket = sock;}

	inline SOCKET GetSocket()	{ return m_socket;}

	//�ر�sock
	inline void	CloseScket() { closesocket(m_socket); m_socket = INVALID_SOCKET;}

	//��ȡbuf
	inline CSendBuffer * GetSendBuffer()		{ return m_pSendBuffer;}
	inline CRecvBuffer * GetRecvBuffer()		{ return m_pRecvBuffer;}

	//remove
	inline void		Remove()					{ m_bRemove = true;}
	inline bool		IsWillRemove()				{ return m_bRemove;}

	//create socket
	SOCKET	CreateSocket();					
	//����opt
	void	SetSocketBasicOpt();

	//׼��socket
	bool	PreAccept(SOCKET listenSocket);
	//
	bool	PreRecv();
	//
	bool	PreSend();

	//����accpet,send��Ĳ���
	bool	onAccept();
	bool	onSend();

	//�ɹ������Ժ����Ϣ
	bool	onConnect(bool bSuccess);

	//����������ݣ���ʵ�����׸�network
	bool	onRecv(xe_uint8 *pMsg,xe_uint16 nLen);
	bool	ProcessRecvPack();				//�����������

	//��������
	bool	SendMsg(stPackHeader *pHeader,xe_uint8 *pMsg,xe_uint16 nLen);

	//����
	bool	OutputLog(xe_int32 nType,char * szFormat,...);

	//ʧȥ����
	bool	Disconnet(bool bDelete);

	inline bool	IsLimit(){ return m_bLimit;}		//�Ƿ��յ�����


	//�����ص�Ψһ��ʾ
	inline	void	SetIndex(xe_uint32 nIndex)		{ m_nIndex = nIndex;}
	inline	xe_uint32	GetIndex() { return m_nIndex;}	
private:
	xe_uint32	  m_nIndex;					//���

	SOCKET					m_socket;
	SOCKADDR_IN				m_sockaddr;

	//��д����ȥ
	CRecvBuffer *			m_pRecvBuffer;
	CSendBuffer	*			m_pSendBuffer;

	//�Ƿ�Ҫ�ر�,�����򵥵ľͲ���Ҫ�ٽ��������߳�Ҳ����
	volatile bool			m_bRemove;

	//io����
	OVERLAPPEDEX			m_AccpetIo;
	OVERLAPPEDEX			m_SendIo;
	OVERLAPPEDEX			m_RecvIo;

	bool					m_bLimit;			//����������������,�������κ���Ϣ�����ǿ��Է�����Ϣ
};
#endif