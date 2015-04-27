#include "Session.h"
#include "NetWorkObj.h"


bool INetWorkObj::SendMsg( stPackHeader *pHeader,xe_uint8 *pMsg,xe_uint16 nLen )
{
	if (m_pSession && m_pSession->IsLimit() == false)
		return m_pSession->SendMsg(pHeader,pMsg,nLen);

	return false;
}

INetWorkObj::INetWorkObj()
{
	m_pSession = NULL;
}

INetWorkObj::~INetWorkObj()
{
	if (m_pSession)
	{
		m_pSession->UnBindNetWork();
	}
}

void INetWorkObj::Disconnect( bool bGracefulDisconnect /*= TRUE */ )
{
	if (m_pSession)
	{
		m_pSession->Disconnet(bGracefulDisconnect);
	}
}

void INetWorkObj::Redirect( INetWorkObj *pNetworkObject )
{
	if (pNetworkObject && m_pSession)
	{
		m_pSession->BindNetWork(pNetworkObject);
	}
}

char* INetWorkObj::GetIP()
{
	if (m_pSession)
	{
		return m_pSession->GetIP();
	}
	else
		return NULL;
}
