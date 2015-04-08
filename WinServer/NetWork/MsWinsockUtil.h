#ifndef MSWINSOCKUTIL_H_
#define MSWINSOCKUTIL_H_

//////////////////////////////////////////////////////////////////////////
//主要api函数AcceptEx和GetSocketAddr
//////////////////////////////////////////////////////////////////////////
#include "GlobalDefine.h"

class MsWinsockUtil
{
public:
	//加载函数
	static void	LoadExtensionFunction( SOCKET ActiveSocket );	

	//AcceptEx
	static LPFN_ACCEPTEX				m_lpfnAccepteEx;			
	//GetAcceptExSockAddrs函数
	static LPFN_GETACCEPTEXSOCKADDRS	m_lpfnGetAcceptExSockAddrs;

private:
	static bool LoadExtensionFunction( SOCKET ActiveSocket,	GUID FunctionID, void **ppFunc);
};

#endif