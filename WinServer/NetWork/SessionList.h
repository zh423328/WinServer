#ifndef SESSIONLIST_H_
#define SESSIONLIST_H_
//////////////////////////////////////////////////////////////////////////
//sessionlist 链表保存连接数据
//////////////////////////////////////////////////////////////////////////
#include <list>
#include "lock.h"

class CSession;
class CSessionList : public std::list<CSession*>
{
public:
	CSessionList();
	~CSessionList();

	void Clear();			//清楚所有数据


	//临界区
	void Lock()			{EnterCriticalSection(&m_cs);}
	void UnLock()		{LeaveCriticalSection(&m_cs);}

private:
	CRITICAL_SECTION m_cs;		//临界区
};

#endif