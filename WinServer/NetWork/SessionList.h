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
	inline void Lock()			{m_cs.Lock();}
	inline void UnLock()		{m_cs.UnLock();}

private:
	CLock m_cs;		//�ٽ���
};

#endif