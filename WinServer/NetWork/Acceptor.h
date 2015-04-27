//////////////////////////////////////////////////////////////////////////
//接收器，接收连接
//////////////////////////////////////////////////////////////////////////

#include "GlobalDefine.h"

class cIocpServer;			//完成端口

//接收器
class CAcceptor
{
public:
	CAcceptor(cIocpServer *pParent);
	virtual ~CAcceptor();

	void				Init( cIocpServer *pIoHandler, xe_uint32 dwNumberOfThreads );
	bool				StartListen( char *pIP, xe_uint16 wPort );
	void				Shutdown();
	
	void				SuspendListenThread();
	void				ResumeListenThread();

	inline SOCKET		GetListenSocket() { return m_listenSocket; }
	inline bool			IsListening() { return m_listenSocket != INVALID_SOCKET; }

	static DWORD WINAPI accept_thread( LPVOID param );

private:
	bool				CreateListenSocket( char *pIP, xe_uint16 wPort );

	cIocpServer			*m_pParent;
	HANDLE*				m_hThreads;
	HANDLE				m_hIOCP;
	SOCKET				m_listenSocket;
	SOCKADDR_IN			m_sockaddr;
	xe_uint32			m_numThreads;
};