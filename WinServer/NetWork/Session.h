#ifndef SESSION_H_
#define SESSION_H_

//////////////////////////////////////////////////////////////////////////
//Session封装
//////////////////////////////////////////////////////////////////////////

#include "GlobalDefine.h"
#include "PackBuffer.h"

//overapped 操作
//////////////////////////////////////////////////////////////////
// 在完成端口上投递的I/O操作的类型
enum IOCP_REQUESTTYPE
{
	IOCP_REQUEST_NULL,
	IOCP_REQUEST_ACCEPT,						
	IOCP_REQUEST_RECV ,								 // read
	IOCP_REQUEST_SEND,								 // write
};


class CSession;
//overplapped 异步传输
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

//连接类,
class CSession
{
public:
	CSession();
	virtual ~CSession();

	void Init();

	//设置地址
	inline void SetSockAddr(SOCKADDR_IN & sockaddr) { m_sockaddr = sockaddr;}

	//获取地址
	inline SOCKADDR* GetSockAddr()	{ return (SOCKADDR*)&m_sockaddr;}

	//获取地址
	inline char*  GetIP()		{ return inet_ntoa(m_sockaddr.sin_addr);}

	//设置和获取socket
	inline void   SetSocket(SOCKET sock)		{ m_socket = sock;}

	inline SOCKET GetSocket()	{ return m_socket;}

	//关闭sock
	inline void	CloseScket() { closesocket(m_socket); m_socket = INVALID_SOCKET;}

	//获取buf
	inline CSendBuffer * GetSendBuffer()		{ return m_pSendBuffer;}
	inline CRecvBuffer * GetRecvBuffer()		{ return m_pRecvBuffer;}

	//remove
	inline void		Remove()					{ m_bRemove = true;}
	inline bool		IsWillRemove()				{ return m_bRemove;}

	//create socket
	SOCKET	CreateSocket();					
	//设置opt
	void	SetSocketBasicOpt();

	//准备socket
	bool	PreAccept(SOCKET listenSocket);
	//
	bool	PreRecv();
	//
	bool	PreSend();

	//接受accpet,send后的操作
	bool	onAccept();
	bool	onSend();

	//成功连接以后的消息
	bool	onConnect(bool bSuccess);

	//处理接收数据，其实就是抛给network
	bool	onRecv(xe_uint8 *pMsg,xe_uint16 nLen);
	bool	ProcessRecvPack();				//处理接收数据

	//发送数据
	bool	SendMsg(stPackHeader *pHeader,xe_uint8 *pMsg,xe_uint16 nLen);

	//类型
	bool	OutputLog(xe_int32 nType,char * szFormat,...);

	//失去连接
	bool	Disconnet(bool bDelete);

	inline bool	IsLimit(){ return m_bLimit;}		//是否收到限制


	//在网关的唯一标示
	inline	void	SetIndex(xe_uint32 nIndex)		{ m_nIndex = nIndex;}
	inline	xe_uint32	GetIndex() { return m_nIndex;}	


	//network
	void	BindNetWork(INetWorkObj *pObj);
	void	UnBindNetWork();

	inline INetWorkObj* GetNetWorkObj(){ return m_pNetWork;}
private:
	xe_uint32	  m_nIndex;					//标记

	SOCKET					m_socket;
	SOCKADDR_IN				m_sockaddr;

	//读写缓存去
	CRecvBuffer *			m_pRecvBuffer;
	CSendBuffer	*			m_pSendBuffer;

	//是否将要关闭,这样简单的就不需要临界区，多线程也行了
	volatile bool			m_bRemove;

	//io操作
	OVERLAPPEDEX			m_AccpetIo;
	OVERLAPPEDEX			m_SendIo;
	OVERLAPPEDEX			m_RecvIo;

	bool					m_bLimit;			//服务器单方面限制,不接受任何消息，但是可以发送消息
	INetWorkObj*			m_pNetWork;			//network
};
#endif