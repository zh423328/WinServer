#ifndef NETWORK_OBJ_H_
#define NETWORK_OBJ_H_
//////////////////////////////////////////////////////////////////////////
//NetWorkObj主要负责Session的相关操作， 是Session的父节点
//////////////////////////////////////////////////////////////////////////

#include "GlobalDefine.h"

////连接参数
//class CSession;	
//class INetWorkObj
//{
//public:
//	INetWorkObj();
//	virtual ~INetWorkObj();
//
//	//设置服务器连接session
//	void SetSession(CSession * pSession) { m_pSession = pSession;}
//
//	//处理消息
//	bool onRecv(xe_uint8  *pMsg, xe_uint16 nSize) = 0;
//
//	virtual bool	SendMsg(stPackHeader *pHeader,xe_uint8 *pMsg,xe_uint16 nLen);
//private:
//	CSession * m_pSession;					//socket封装版
//};
#endif