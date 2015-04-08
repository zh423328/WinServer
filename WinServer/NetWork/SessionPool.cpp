#include "Session.h"
#include "SessionList.h"
#include "SessionPool.h"


CSessionPool::CSessionPool( xe_uint32 dwSize)
{
	m_pList = new CSessionList();

	Create(dwSize);
}

CSessionPool::~CSessionPool()
{
	SAFE_DELETE(m_pList);
}

void CSessionPool::Create(xe_uint32 dwSize)
{
	m_nSize = dwSize;			//预先分配 
	m_nStepSize = dwSize/10;	//

	for(xe_uint32 i = 0; i < m_nSize; ++i)
	{
		CSession *pSession = new CSession();			//分配 

		m_pList->push_back(pSession);
	}
}

CSession* CSessionPool::Alloc()
{
	//分配一个,已经全部分配
	if (m_pList->empty())
	{
		CreateStepSize();
	}

	m_pList->Lock();

	CSession *pSession = m_pList->front();

	m_pList->pop_front();

	m_pList->UnLock();

	return pSession;
}

void CSessionPool::Free(CSession *pSession)
{
	if (pSession == NULL)
		return;

	pSession->CloseScket();
	pSession->Init();

	m_pList->Lock();
	m_pList->push_back(pSession);
	m_pList->UnLock();
}

xe_uint32 CSessionPool::GetLength()
{
	m_pList->Lock();
	return m_pList->size();
	m_pList->UnLock();
}

void CSessionPool::CreateStepSize()
{
	m_pList->Lock();

	for(xe_uint32 i = 0; i < m_nSize; ++i)
	{
		CSession *pSession = new CSession();			//分配 

		m_pList->push_back(pSession);
	}

	m_pList->UnLock();
}




