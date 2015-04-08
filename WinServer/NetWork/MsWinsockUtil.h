#ifndef MSWINSOCKUTIL_H_
#define MSWINSOCKUTIL_H_

//////////////////////////////////////////////////////////////////////////
//��Ҫapi����AcceptEx��GetSocketAddr
//////////////////////////////////////////////////////////////////////////
#include "GlobalDefine.h"

class MsWinsockUtil
{
public:
	//���غ���
	static void	LoadExtensionFunction( SOCKET ActiveSocket );	

	//AcceptEx
	static LPFN_ACCEPTEX				m_lpfnAccepteEx;			
	//GetAcceptExSockAddrs����
	static LPFN_GETACCEPTEXSOCKADDRS	m_lpfnGetAcceptExSockAddrs;

private:
	static bool LoadExtensionFunction( SOCKET ActiveSocket,	GUID FunctionID, void **ppFunc);
};

#endif