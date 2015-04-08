#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include "GlobalDefine.h"

class CSession;
class CSessionList;
class CSessionPool;
struct OVERLAPPEDEX;
class CConnector;
class cIocpServer
{
public:
	cIocpServer();
	virtual ~cIocpServer();


	// 启动服务器
	virtual bool Start();

	//	停止服务器
	virtual void Stop();

	// 加载Socket库
	bool LoadSocketLib();

	// 卸载Socket库，彻底完事
	void UnloadSocketLib() { WSACleanup(); }

	// 获得本机的IP地址
	std::string GetLocalIP();

	// 设置监听端口
	inline void SetPort( const int& nPort ) { m_nPort=nPort; }

	void AddConnectSuccessList(CSession *pSession);

	inline bool Running() { return m_bRunning;}


	//放入主线程
	virtual void Update();

protected:

	virtual bool StartConnect();		//启动连接

	// 初始化IOCP
	virtual bool _InitializeIOCP();

	// 初始化Socket
	virtual bool _InitializeListenSocket();

	//初始化后台处理线程
	virtual bool _InitializeBackendThread();

	// 最后释放资源
	virtual void _DeInitialize();

	// 投递Accept请求
	virtual bool _PostAccept(CSession *pSession); 

	// 在有客户端连入的时候，进行处理
	virtual bool _DoAccpet(CSession *pSession,OVERLAPPEDEX *ol,xe_uint32 dwBytes);


	// 投递接收数据请求
	virtual bool _PostRecv(CSession *pSession);
	
	// 在有接收的数据到达的时候，进行处理
	virtual bool _DoRecv(CSession *pSession,OVERLAPPEDEX *ol,xe_uint32 dwBytes);

	//发送给后台
	virtual bool _RecvPost(CSession* pSocketContext,char *szBuf,xe_uint32 dwBytes);

	// 投递接发送数据
	virtual bool _PostSend(CSession *pSession);

	//处理发送数据
	virtual bool _DoSend(CSession *pSession,OVERLAPPEDEX *ol);


	//这个才是真正发生数据消息
	virtual void _IoSend();

	//消息处理
	virtual void _IoRecv();

	// 将句柄绑定到完成端口中
	virtual bool _AssociateWithIOCP( CSession *pContext);

	// 设置socket相关属性
	virtual void _SetSocketProp(SOCKET socket);

	// 处理完成端口上的错误
	virtual bool HandleError( CSession *pContext,const xe_uint32& dwErr );

	// 线程函数，为IOCP请求服务的工作者线程
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);

	// 线程函数，为session 发送数据线程
	static DWORD WINAPI _BackEndThread(LPVOID lpParam);

	// 获得本机的处理器数量
	virtual int _GetNoOfProcessors();

	// 判断客户端Socket是否已经断开
	virtual bool _IsSocketAlive(SOCKET s);

	// 在主界面中显示信息
	virtual void _ShowMessage( char*szFormat,...);

	//发送
	virtual void ProcessSendPacket();

	//接收
	virtual void ProcessRecvPacket();

	//connecttoactive
	virtual void ConnectInActive();

	//删除无效指针
	virtual void KillDeadSession();

	//关闭socket
	virtual void _CloseSocket(SOCKET s);

	//关闭操作
	virtual void KillAllSession();


	virtual void cIocpServer::SendPacket(xe_uint32 cid,xe_uint8 bCmdGroup,xe_uint8 bCmd,char * buffer, int buflength);
private:

	bool						 m_bRunning;					//

	HANDLE                       m_hShutdownEvent;              // 用来通知线程系统退出的事件，为了能够更好的退出线程

	HANDLE						 m_hBackendThread;				//后台处理线程

	HANDLE                       m_hIOCompletionPort;           // 完成端口的句柄

	HANDLE*                      m_phWorkerThreads;             // 工作者线程的句柄指针

	int		                     m_nThreads;                    // 生成的线程数量

	string                       m_strIP;                       // 服务器端的IP地址
	int                          m_nPort;                       // 服务器端的监听端口


	CSession*					 m_pListenSession;              // 用于监听的Socket的Context信息

	CSessionList*				 m_pActiveSessionList;			//有效的
	CSessionList*				 m_pTempSessionList;			//无效

	CSessionPool*				 m_pSessionPool;				//

	CSessionList*				 m_pConnectSuccessList;			//连接成功的链表

	CConnector*					 m_pConnector;				//连接链表
};
#endif