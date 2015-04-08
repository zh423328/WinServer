#include <winsock2.h>
#include <mswsock.h>
#include "MsWinsockUtil.h"


LPFN_ACCEPTEX				MsWinsockUtil::m_lpfnAccepteEx				= NULL;
LPFN_GETACCEPTEXSOCKADDRS	MsWinsockUtil::m_lpfnGetAcceptExSockAddrs	= NULL;

void MsWinsockUtil::LoadExtensionFunction( SOCKET ActiveSocket )
{
	//GetAcceptEx
	GUID acceptex_guid = WSAID_ACCEPTEX;
	LoadExtensionFunction( ActiveSocket, acceptex_guid, (void**) &m_lpfnAccepteEx);


	//GetAcceptExSockaddrs
	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	LoadExtensionFunction( ActiveSocket, guidGetAcceptExSockaddrs, (void**) &m_lpfnGetAcceptExSockAddrs);
}

bool MsWinsockUtil::LoadExtensionFunction( SOCKET ActiveSocket,	GUID FunctionID, void **ppFunc )
{
	DWORD	dwBytes = 0;

	if (0 != WSAIoctl(
		ActiveSocket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&FunctionID,
		sizeof(GUID),
		ppFunc,
		sizeof(void *),
		&dwBytes,
		0,
		0))
	{
		return false;
	}

	return true;
}

