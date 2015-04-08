#ifndef LOCK_H_
#define LOCK_H_

//////////////////////////////////////////////////////////////////////////
//���������ֹ����
//////////////////////////////////////////////////////////////////////////
//��
#include "GlobalDefine.h"
class CSlock
{
public:
	CSlock(CRITICAL_SECTION * pCs)
	{
		m_pCs = pCs;
		EnterCriticalSection(m_pCs);
	}
	~CSlock()
	{
		LeaveCriticalSection(m_pCs);
	}
private:
	CRITICAL_SECTION * m_pCs;
};
#endif