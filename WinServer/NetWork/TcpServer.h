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


	// ����������
	virtual bool Start();

	//	ֹͣ������
	virtual void Stop();

	// ����Socket��
	bool LoadSocketLib();

	// ж��Socket�⣬��������
	void UnloadSocketLib() { WSACleanup(); }

	// ��ñ�����IP��ַ
	std::string GetLocalIP();

	// ���ü����˿�
	inline void SetPort( const int& nPort ) { m_nPort=nPort; }

	void AddConnectSuccessList(CSession *pSession);

	inline bool Running() { return m_bRunning;}


	//�������߳�
	virtual void Update();

protected:

	virtual bool StartConnect();		//��������

	// ��ʼ��IOCP
	virtual bool _InitializeIOCP();

	// ��ʼ��Socket
	virtual bool _InitializeListenSocket();

	//��ʼ����̨�����߳�
	virtual bool _InitializeBackendThread();

	// ����ͷ���Դ
	virtual void _DeInitialize();

	// Ͷ��Accept����
	virtual bool _PostAccept(CSession *pSession); 

	// ���пͻ��������ʱ�򣬽��д���
	virtual bool _DoAccpet(CSession *pSession,OVERLAPPEDEX *ol,xe_uint32 dwBytes);


	// Ͷ�ݽ�����������
	virtual bool _PostRecv(CSession *pSession);
	
	// ���н��յ����ݵ����ʱ�򣬽��д���
	virtual bool _DoRecv(CSession *pSession,OVERLAPPEDEX *ol,xe_uint32 dwBytes);

	//���͸���̨
	virtual bool _RecvPost(CSession* pSocketContext,char *szBuf,xe_uint32 dwBytes);

	// Ͷ�ݽӷ�������
	virtual bool _PostSend(CSession *pSession);

	//����������
	virtual bool _DoSend(CSession *pSession,OVERLAPPEDEX *ol);


	//���������������������Ϣ
	virtual void _IoSend();

	//��Ϣ����
	virtual void _IoRecv();

	// ������󶨵���ɶ˿���
	virtual bool _AssociateWithIOCP( CSession *pContext);

	// ����socket�������
	virtual void _SetSocketProp(SOCKET socket);

	// ������ɶ˿��ϵĴ���
	virtual bool HandleError( CSession *pContext,const xe_uint32& dwErr );

	// �̺߳�����ΪIOCP�������Ĺ������߳�
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);

	// �̺߳�����Ϊsession ���������߳�
	static DWORD WINAPI _BackEndThread(LPVOID lpParam);

	// ��ñ����Ĵ���������
	virtual int _GetNoOfProcessors();

	// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ�
	virtual bool _IsSocketAlive(SOCKET s);

	// ������������ʾ��Ϣ
	virtual void _ShowMessage( char*szFormat,...);

	//����
	virtual void ProcessSendPacket();

	//����
	virtual void ProcessRecvPacket();

	//connecttoactive
	virtual void ConnectInActive();

	//ɾ����Чָ��
	virtual void KillDeadSession();

	//�ر�socket
	virtual void _CloseSocket(SOCKET s);

	//�رղ���
	virtual void KillAllSession();


	virtual void cIocpServer::SendPacket(xe_uint32 cid,xe_uint8 bCmdGroup,xe_uint8 bCmd,char * buffer, int buflength);
private:

	bool						 m_bRunning;					//

	HANDLE                       m_hShutdownEvent;              // ����֪ͨ�߳�ϵͳ�˳����¼���Ϊ���ܹ����õ��˳��߳�

	HANDLE						 m_hBackendThread;				//��̨�����߳�

	HANDLE                       m_hIOCompletionPort;           // ��ɶ˿ڵľ��

	HANDLE*                      m_phWorkerThreads;             // �������̵߳ľ��ָ��

	int		                     m_nThreads;                    // ���ɵ��߳�����

	string                       m_strIP;                       // �������˵�IP��ַ
	int                          m_nPort;                       // �������˵ļ����˿�


	CSession*					 m_pListenSession;              // ���ڼ�����Socket��Context��Ϣ

	CSessionList*				 m_pActiveSessionList;			//��Ч��
	CSessionList*				 m_pTempSessionList;			//��Ч

	CSessionPool*				 m_pSessionPool;				//

	CSessionList*				 m_pConnectSuccessList;			//���ӳɹ�������

	CConnector*					 m_pConnector;				//��������
};
#endif