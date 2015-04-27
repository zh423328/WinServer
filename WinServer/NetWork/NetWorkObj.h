#ifndef NETWORK_OBJ_H_
#define NETWORK_OBJ_H_
//////////////////////////////////////////////////////////////////////////
//NetWorkObj��Ҫ����Session����ز����� ��Session�ĸ��ڵ�
//////////////////////////////////////////////////////////////////////////

#include "GlobalDefine.h"

//���Ӳ���
class CSession;	

//��������ң�Ҳ�������ӹ���������������
class INetWorkObj
{
public:
	INetWorkObj();
	virtual ~INetWorkObj();

	//���÷���������session
	inline void SetSession(CSession * pSession) { m_pSession = pSession;}

	

	//������Ϣ
	virtual bool	SendMsg(stPackHeader *pHeader,xe_uint8 *pMsg,xe_uint16 nLen);

	//�Ͽ�����
	void			Disconnect( bool bGracefulDisconnect = TRUE );

	//��������
	void			Redirect( INetWorkObj *pNetworkObject );
	char*			GetIP();


	//����ʱ����ʲô��Ϣ
	virtual void	OnAccept( xe_uint32 dwNetworkIndex )=0;
	//����ʱ������Ϣ
	virtual void	OnDisconnect()=0;
	//������Ϣ
	virtual bool	OnRecv(xe_uint8  *pMsg, xe_uint16 nSize) = 0;
	//����ʱ
	virtual void	OnConnect( bool bSuccess, xe_uint32 dwNetworkIndex )=0;
private:
	CSession * m_pSession;					//socket��װ��
};
#endif