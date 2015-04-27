#ifndef NETWORK_OBJ_H_
#define NETWORK_OBJ_H_
//////////////////////////////////////////////////////////////////////////
//NetWorkObj主要负责Session的相关操作， 是Session的父节点
//////////////////////////////////////////////////////////////////////////

#include "GlobalDefine.h"

//连接参数
class CSession;	

//可以是玩家，也可以连接过来的其他服务器
class INetWorkObj
{
public:
	INetWorkObj();
	virtual ~INetWorkObj();

	//设置服务器连接session
	inline void SetSession(CSession * pSession) { m_pSession = pSession;}

	

	//发送消息
	virtual bool	SendMsg(stPackHeader *pHeader,xe_uint8 *pMsg,xe_uint16 nLen);

	//断开连接
	void			Disconnect( bool bGracefulDisconnect = TRUE );

	//重新设置
	void			Redirect( INetWorkObj *pNetworkObject );
	char*			GetIP();


	//连接时触发什么消息
	virtual void	OnAccept( xe_uint32 dwNetworkIndex )=0;
	//连接时触发消息
	virtual void	OnDisconnect()=0;
	//处理消息
	virtual bool	OnRecv(xe_uint8  *pMsg, xe_uint16 nSize) = 0;
	//连接时
	virtual void	OnConnect( bool bSuccess, xe_uint32 dwNetworkIndex )=0;
private:
	CSession * m_pSession;					//socket封装版
};
#endif