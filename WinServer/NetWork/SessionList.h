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
	inline void Lock()			{m_cs.Lock();}
	inline void UnLock()		{m_cs.UnLock();}

private:
	CLock m_cs;		//临界区
};

#endif