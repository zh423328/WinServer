#include "Session.h"
#include "SessionList.h"

CSessionList::CSessionList()
{
	//init
	InitializeCriticalSection(&m_cs);
}

CSessionList::~CSessionList()
{
	Clear();

	//ɾ��
	DeleteCriticalSection(&m_cs);
}

void CSessionList::Clear()
{
	CSlock cs(&m_cs);

	CSession *pSession = NULL;

	for (std::list<CSession*>::iterator iter = begin(); iter != end(); ++iter)
	{
		pSession = *iter;
		SAFE_DELETE(pSession);
	}

	clear();			//���
}