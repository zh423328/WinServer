#ifndef NETWORK_OBJ_H_
#define NETWORK_OBJ_H_
//////////////////////////////////////////////////////////////////////////
//NetWorkObj��Ҫ����Session����ز����� ��Session�ĸ��ڵ�
//////////////////////////////////////////////////////////////////////////

#include "GlobalDefine.h"

////���Ӳ���
//class CSession;	
//class INetWorkObj
//{
//public:
//	INetWorkObj();
//	virtual ~INetWorkObj();
//
//	//���÷���������session
//	void SetSession(CSession * pSession) { m_pSession = pSession;}
//
//	//������Ϣ
//	bool onRecv(xe_uint8  *pMsg, xe_uint16 nSize) = 0;
//
//	virtual bool	SendMsg(stPackHeader *pHeader,xe_uint8 *pMsg,xe_uint16 nLen);
//private:
//	CSession * m_pSession;					//socket��װ��
//};
#endif