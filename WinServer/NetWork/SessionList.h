#ifndef SESSIONLIST_H_
#define SESSIONLIST_H_
//////////////////////////////////////////////////////////////////////////
//sessionlist ��������������
//////////////////////////////////////////////////////////////////////////
#include <list>
#include "lock.h"

class CSession;
class CSessionList : public std::list<CSession*>
{
public:
	CSessionList();
	~CSessionList();

	void Clear();			//�����������


	//�ٽ���
	void Lock()			{EnterCriticalSection(&m_cs);}
	void UnLock()		{LeaveCriticalSection(&m_cs);}

private:
	CRITICAL_SECTION m_cs;		//�ٽ���
};

#endif